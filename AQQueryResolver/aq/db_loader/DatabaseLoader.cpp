// include files 

#include "DatabaseLoader.h"

#include <aq/Utilities.h>
#include <aq/DateConversion.h>
#include <aq/Logger.h>
#include <aq/Exceptions.h>

#include <cstring>
#include <cstdlib>
#include <cerrno>
#include <ctime>
#include <algorithm>
#include <string>

#define k_batch_size_max 8192   // batch_name
#define k_record_size_max 40960   // record in a file
#define k_field_size_max 4096   // record in a file  
#define k_file_name_size_max 4096    // nom de fichier 
#define k_taille_message  4096 

namespace aq
{

// ---------------------------------------------------------------------------------------------
DatabaseLoader::DatabaseLoader(const aq::base_t bd, const std::string& _loader, const std::string& _path, const size_t _packet_size, const char _end_of_field, bool _csv_format)
  : 
  my_base(bd),
  k_rep_racine(_path),
  k_batch_loader(_loader),
  k_packet_size(_packet_size),
  k_double_separator(','),
  end_of_field(_end_of_field),
  csv_format(_csv_format)
{
  base_desc_file = this->k_rep_racine + "base_desc/base.aqb";
	rep_source = k_rep_racine + "data_orga/tables/";
	rep_cible = k_rep_racine + "data_orga/columns/";
  ini_filename = this->k_rep_racine + "loader.ini";
}

//-------------------------------------------------------------------------------
void DatabaseLoader::generate_ini()
{
	FILE * fini = fopen(ini_filename.c_str(), "w");
	if (!fini)
	{
    throw aq::generic_error(aq::generic_error::INVALID_FILE, "cannot create file %s\n", ini_filename);
	}

	fwrite("export.filename.final=", 1, 22, fini);
	fwrite(k_rep_racine.c_str(), 1, k_rep_racine.size(), fini);
	fwrite("base_struct/base.aqb", 1, 20, fini);
	fwrite("\n", 1, 1, fini);
	fwrite("step1.field.separator=", 1, 22, fini);
	fwrite(&end_of_field, 1, 1, fini);
	fwrite("\n", 1, 1, fini);
	fwrite("k_rep_racine=", 1, 13, fini);
	fwrite(k_rep_racine.c_str(), 1, k_rep_racine.size(), fini);
	fwrite("\n", 1, 1, fini);
	fwrite("k_rep_racine_tmp=", 1, 17, fini);
	fwrite(k_rep_racine.c_str(), 1, k_rep_racine.size(), fini);
	fwrite("\n", 1, 1, fini);
  fwrite("k_taille_nom_fichier=1024", 1, 25, fini);
	fwrite("\n", 1, 1, fini);
	fclose(fini);
}

//-------------------------------------------------------------------------------
void DatabaseLoader::run(size_t num_table, size_t num_column) 
{
  int rc;
  std::string name_log;
  char exec_cmd[1024];
  
	FILE * fp2; // fichier source
	FILE * fcol; // col destination du decoupage + binaire
	int len_rec ;
	int i ;
	int nb_pack_col;
	int nb_col_in_last_pack;
	int debut_lecture;
	int cur_col ;

	int total_nb_enreg =0;
	char * my_record = (char *) safecalloc ( k_record_size_max, sizeof(char)); 
	char * my_field = (char *) safecalloc ( k_field_size_max, sizeof(char)); 
	std::string my_col_dir;   
	std::string my_dir_source;
	std::string my_dir_cible; 

	// init
	aq::s_prefixe prefixe;
	allocate_and_init_prefixe( &prefixe);

	// info on curent column in parameters
	aq::symbole col_type;
	int col_size ;
	char * my_col = (char*) safecalloc(k_file_name_size_max, sizeof(char));
	std::string my_fic2; 
	int max_nb_table; // nbre de fichier en fonction du client
	char my_string[2];

	// format for sprintf 
	char * format_file_name = (char*) safecalloc(4096 , sizeof (char));
	init_format_column(format_file_name, &prefixe);

	// initialisation des composants de nom de fichiers
	int n_base = 1;
	int n_table = 1;
	int i_table = 0 ; // loop table index   
	int n_paquet = 0;

	// declaration de la date et l'heure
	time_t t = time (NULL);
	struct tm * tb = localtime(&t);
	aq::Logger::getInstance().log(AQ_INFO, "Start on  %d/%02d/%02d H %02d:%02d:%02d\n" , tb->tm_year +1900, tb->tm_mon +1 ,tb->tm_mday, tb->tm_hour, tb->tm_min, tb->tm_sec);

	// declaration des fichier a decouper
	max_nb_table =  my_base.nb_tables;
	char *cust_name;
	cust_name = (char *) safecalloc ( k_file_name_size_max, sizeof(char));
	char ** cust_file_name = (char **) safecalloc ( max_nb_table , sizeof(char *));
	for ( i = 0; i < max_nb_table; i++) 
	{
		cust_file_name[ i ] = (char *) safecalloc ( k_file_name_size_max, sizeof(char));
	}

	// preparation des noms de fichiers
  i = 0;
  for (auto& table : my_base.table) 
  {
		strcpy ( cust_name, table.nom.c_str() );
		nettoie_nom (cust_name); // ote les " " 
		strcat ( cust_name,".txt");  
		strcpy ( cust_file_name[ i ], cust_name );
    ++i;
	}
  
	// load num_table from base_struct
	aq::s_list_int  l_n_table;
	// allocate 
	l_n_table.max_size = max_nb_table;
	l_n_table.used_size = max_nb_table;
	l_n_table.part = ( int * ) safecalloc( max_nb_table, sizeof( int ) );       
	// load n_table as declared in base_struct   
  i = 0;
  for (auto& table : my_base.table) 
  {
    *( l_n_table.part + i ) = table.num ; 
    ++i;
	}
  
	// declaration des repertoire source et cible
	my_dir_source = rep_source;
	my_dir_cible = rep_cible;

	int max_read = aq::packet_size; // old value 4096; 
	int nb_fields;
	int indice_car;
	unsigned char  my_char;

	int openQuote = 0; //if csv, skip end_of_field chars within quotes

	// int nb_enreg; // size in nb records of file

	for ( i_table = 0 ; i_table < max_nb_table; i_table++)
	{
		// store current n_table
		n_table =   *( l_n_table.part + i_table );

		// check n_table
		if ( n_table != num_table )
			continue;

		aq::Logger::getInstance().log(AQ_INFO, "Table : %u\n", n_table);

		// curent file to cut in col
		my_col_dir = my_dir_cible; // toutes les colonnes directement dans la bon repertoire  
		my_fic2 = my_dir_source + cust_file_name[i_table];

		len_rec = 0 ;
		nb_fields = 1; // one field at least !
		n_paquet = 0;

		// calculate the number of fields in a record
		if(  NbFieldInRecord ( my_fic2.c_str() , my_record ,  max_read,  &nb_fields  , &nb_pack_col , &nb_col_in_last_pack ) == aq::symbole::t_continue )  continue;
		
		// open source file and check if opened
		if( (fp2 = aq::fopenUTF8(my_fic2.c_str(), "r")) == NULL )
		{
      throw aq::generic_error(aq::generic_error::COULD_NOT_OPEN_FILE, "error opening file %s\n", my_fic2.c_str());
		}

		// table and colonne : OK
		// creation des noms de fichiers : nouvelle structure
		sprintf(my_col, format_file_name, my_col_dir.c_str(), n_base, n_table, num_column, n_paquet);
		// init col infos
		col_type = my_base.table[i_table].colonne[num_column - 1].type;
		col_size = my_base.table[i_table].colonne[num_column - 1].taille;
		aq::Logger::getInstance().log(AQ_INFO, "%s max_size: %d\n", my_col, col_size);
		if( (fcol = fopen ( my_col ,"w+b")) == NULL )  //  '+ ' : erase former file if exist 2009/10/27
		{
      throw aq::generic_error(aq::generic_error::COULD_NOT_OPEN_FILE, "error opening file %s\n", my_col);
		}

		int write_n_enreg = 0;
		unsigned char check_empty_1, check_empty_2;

		while ( fgets ( my_record, max_read, fp2 )  ) //read record
		{
			// check if EOF
			if ( feof ( fp2  ) )  
				break;
			// check if error on reading
			if ( ferror ( fp2  ) )
			{
        throw aq::generic_error(aq::generic_error::INVALID_FILE, "error reading record  %d, loading aborted\n", total_nb_enreg);
			}

			// record is loaded, update total nb_enreg
			write_n_enreg++;
			total_nb_enreg++;
			// changement de paquet ?
			if ( write_n_enreg > aq::packet_size )
			{
				// fermeture de la  colonne du paquet en cours
				fflush( fcol );
				fclose( fcol );
				// ------------------
				// loader part
				// ------------------
				// construct loader parameters
				sprintf(exec_cmd, "%s %s %d %d %d", k_batch_loader.c_str(), ini_filename.c_str(), num_table , num_column , n_paquet);
				// loader 
        aq::Logger::getInstance().log(AQ_INFO, exec_cmd);
				rc = system(exec_cmd);
        if (rc != 0)
        {
          aq::Logger::getInstance().log(AQ_ERROR, "loader error '%s'\n", exec_cmd);
        }

				// next packet
				n_paquet++;
				// ré-initiaisation du write_n_enreg
				write_n_enreg = 1; 
				//remove( my_col );

				// ouverture de la colonne pour le nouveau packet
				sprintf(my_col, format_file_name, my_col_dir.c_str(), n_base, n_table, num_column, n_paquet);
				if( (fcol = fopen(my_col, "w+b")) == NULL ) //  '+ ' : erase former file if exist 2009/10/27
				{
          throw aq::generic_error(aq::generic_error::COULD_NOT_OPEN_FILE, "error opening file %s\n", my_col);
				}
			}

			// cut & store record in n_col files

			// my_record a decouper en tranches
			len_rec = (int) strlen(my_record);
			debut_lecture = 0;
			cur_col = 1;

			// je me cale sur  num_colonne
			if ( cur_col  !=  num_column ) // premiere colonne ?
			{  
				for ( i = 0 ; i < len_rec; i++ )
				{
					if ( csv_format && my_record[ i ] == '"' )
						openQuote = openQuote ? 0 : 1;
					if ( !openQuote && (my_record[ i ] == end_of_field || my_record[ i ] == '\t') )
					{
						cur_col++;
						if ( cur_col == num_column ) 
						{
							debut_lecture = i +1;// premier caractere du premier champ du pack
							break;
						} 
					}
				}
			}
			indice_car = 0 ;
			// start reading num_colonne
			for ( i = debut_lecture ; i < len_rec ; i++ ) 
			{
				check_empty_1 =  my_record [ i ];
				check_empty_2 =  my_record [ i  + 1 ];


				my_char = my_record[ i ];       
				my_string[0] = my_char;
				my_string[1] = 0;

				if ( csv_format && my_char == '"' )
					openQuote = openQuote ? 0 : 1;

				// if end_of_field, or end_of record, write and next enreg
				if (  !openQuote && (my_char == end_of_field || 
					my_char == '\t'|| my_char == '\n'  || 
					my_char == '\r')  ||
					( check_empty_1 == '\\' && check_empty_2 =='N' ) )
				{
					char* field = my_field;
					if ( csv_format )
					{
						field = nettoie_nom_vite( field );
						if( !field )
						{
              throw aq::generic_error(aq::generic_error::INVALID_FILE, "Format CSV mais champ sans \"\". Row: %d.\n", total_nb_enreg);
						}
					}
					// empty column   in pages jaunes marketing services 2011/02/23
					if(  check_empty_1 == '\\' && check_empty_2 =='N' )  strcpy (my_field,"NULL");//2011/02/22
					// end of column 
					else if( indice_car > 0 )  field[indice_car ] = 0; // end of string
					// indice car = 0 : no sting in field
					else  
						strcpy (field,"NULL");//2011/02/22

					// write column value in file
					FileWriteEnreg(col_type, col_size, field, fcol, total_nb_enreg);
					break;
				}
				else //  start or in column : to add the current char  
				{
					if ( indice_car == 0 ) strcpy (my_field, my_string);
					else strcat (my_field, my_string);  
					indice_car ++;
				}
			}
    }

		// last packet
		// close all the column files
		fflush( fcol );
		fclose( fcol );

		// ------------------
		// loader part
		// ------------------
		// construct loader parameters
		sprintf(exec_cmd, "%s %s %d %d %d", k_batch_loader.c_str(), ini_filename.c_str(), num_table , num_column , n_paquet);
		// loader 

    aq::Logger::getInstance().log(AQ_INFO, exec_cmd);
		rc = system(exec_cmd);
		if (rc != 0)
		{
			aq::Logger::getInstance().log(AQ_ERROR, "loader error '%s'\n", exec_cmd);
		}

		// changement de pack
		// close the table source file
		fclose(fp2);
		aq::Logger::getInstance().log(AQ_INFO, "nbre de ligne dans la table %s : %u \n", my_fic2.c_str(), total_nb_enreg);
		total_nb_enreg = 0; // changement de table 
		// **** a faire  fonction de controle, avec la valeur déclarée dans base. Si different, alors modifie base
		// non je laisse en l'etat on verra si c'est utile
	}

	//  2008 2 15
	for ( i = 0; i < max_nb_table; i++) 
	{
		free ( cust_file_name [ i ] );
	}
	free ( cust_file_name );
	aq::Logger::getInstance().log(AQ_INFO, "Done !\n"); 
	// end time
	t = time ( NULL );
	tb = localtime( &t );
	aq::Logger::getInstance().log(AQ_INFO, "End on %d/%02d/%02d H %02d:%02d:%02d\n" , tb->tm_year +1900, tb->tm_mon +1 ,tb->tm_mday, tb->tm_hour, tb->tm_min, tb->tm_sec);
}

// --------------------------------------------------------------------------------------------
aq::symbole DatabaseLoader::NbFieldInRecord ( const char *my_fic2 , char *my_record ,  int max_read, int *nb_fields  , int *nb_pack_col , int *nb_col_in_last_pack )
{
	FILE *fp2;

	int len_rec , i ;
	// open source file
	if (( fp2 = aq::fopenUTF8( my_fic2 , "r" )) == NULL)
	{
		aq::Logger::getInstance().log(AQ_INFO, "Table  %s  absente\n", my_fic2);
		fflush (stdout);
		return aq::t_continue ; // passe à la table suivante
	}

	// printf ("step 3 \n");
	// count the number of fields in record
	if ( fgets ( my_record, max_read, fp2 ) != NULL )
	{
		// my_record a decouper en tranches
		len_rec = ( int ) strlen( my_record);

		// count the number of fields
		int openQuote = 0;

		for ( i = 0 ; i < len_rec; i++ )
		{
			if( csv_format && (my_record[i] == '"') ) 	openQuote = openQuote ? 0 : 1;
			if ( ((my_record[ i ] == end_of_field) && !openQuote) || my_record[ i ] == '\t' )  (*nb_fields) ++;
		}
		/*
		(*nb_pack_col) =  (*nb_fields) / k_max_open_file;
		(*nb_col_in_last_pack) = (*nb_fields) - ((*nb_pack_col ) *k_max_open_file);
		if ( (*nb_col_in_last_pack) > 0 ) (*nb_pack_col) ++; // add the last col serie
		}
		else
		{
		(*nb_pack_col) = 0;
		(*nb_col_in_last_pack) = 0;
		*/
	}
	// close the table source file
	fclose(fp2); 

	return  aq::t_done;
}

// -----------------------------------------------------------------------------------------------------------
void DatabaseLoader::nettoie_nom (char *nom)
{ 
	// retire les guillements qui encadre nom
	char *new_nom;
	new_nom = (char *) safecalloc (512, sizeof (char));
	int i, len;
	len = (int)  strlen (nom);
	if ( nom[0] != '"') return;
	len -- ; // pour ne pas lire le " en fin de chaine
	for ( i = 1; i < len ; i++)
	{
		new_nom[ i - 1 ] = nom[ i ];  
	}
	new_nom[i] = 0;
	strcpy ( nom, new_nom);
	free (new_nom);
	return;
}

//-------------------------------------------------------------------------------
void* DatabaseLoader::safecalloc(size_t nb, size_t size)
{
	void *p = (void *) calloc(nb, size);
	if (!p)
	{
		fprintf(stderr, "Pas assez de memoire\n");
		exit(0);
	}
	return p;
}

//-------------------------------------------------------------------------------
void DatabaseLoader::allocate_and_init_prefixe ( aq::s_prefixe *prefixe )
{
	// allocate
	prefixe->base = (char *) safecalloc( 3, sizeof (char) );
	prefixe->table = (char *) safecalloc( 3, sizeof (char) );
	prefixe->colonne = (char *) safecalloc( 3, sizeof (char) ); 
	prefixe->niveau = (char *) safecalloc( 3, sizeof (char) );
	prefixe->paquet = (char *) safecalloc( 4, sizeof (char) );
	// init
	strcpy ( prefixe->base , "03" ) ;      // nbre de bases max 999
	strcpy ( prefixe->table  , "04" ) ;      // nbre de tables max 9999
	strcpy (  prefixe->colonne   , "04" ) ;      // nbre de colonnes  max 9999
	strcpy (  prefixe->niveau   , "02" ) ;      // nbre de colonnes  max 99
	strcpy (  prefixe->paquet   , "012" ) ;      // nbre de paquet max 999...999
}

//-------------------------------------------------------------------------------
void DatabaseLoader::init_format_column ( char *format , aq::s_prefixe *prefixe )
{
	// noat : strcat add \0 automatically
	strcpy ( format , "%sB%");
	strcat  ( format , prefixe->base );
	strcat  ( format , "dT%"  );
	strcat  ( format , prefixe->table );
	strcat  ( format , "dC%"  );
	strcat  ( format , prefixe->colonne );
	strcat  ( format , "dP%"  );
	strcat  ( format , prefixe->paquet );
	strcat  ( format , "d"  ); 
}

//-------------------------------------------------------------------------------
void DatabaseLoader::init_format_column_short ( char *format , aq::s_prefixe *prefixe )
{
	// noat : strcat add \0 automatically
	strcpy  ( format , "T%"  );
	strcat  ( format , prefixe->table );
	strcat  ( format , "dC%"  );
	strcat  ( format , prefixe->colonne );
	strcat  ( format , "d"  ); 
}

//-------------------------------------------------------------------------------
void DatabaseLoader::CleanSpaceAtEnd ( char *my_field )
{
	// discard all space at the end
	size_t max_size = strlen( my_field);
	if (max_size > 0)
	{
		// beware >0, must have at least one char
		for ( size_t i = max_size - 1; i > 0 ; i -- )
		{
			if ( my_field [ i ] == ' ' ) my_field [ i ] = '\0';
			else return;
		}
	}
	//at this point  my_field is empty 
	//need this ;   strcpy ( my_field, "NULL" ); ?
}

//-------------------------------------------------------------------------------
void DatabaseLoader::ChangeCommaToDot (  char *string )
{
	// assume  : input is to be converted in double
	// change  ',' in '.'
	char *p;
	// seach first  ',' in string
	p = strchr(string, ',' );
	// modify string ',' become '.'  
	if (p != NULL )  *p = '.' ;
}

//-------------------------------------------------------------------------------
void DatabaseLoader::FileWriteEnreg(aq::symbole col_type, int col_size, char *my_field, FILE *fcol, int rowNr)
{
	int dum_int;
	int * my_int = & dum_int;

	double dum_double; // 2009/09/01 
	double * my_double = &dum_double; // 2009/09/01 

	long long int dum_long_long;
	long long int *my_long_long = &dum_long_long;

  aq::DateConversion dateConverter;

	switch (  col_type )
	{
	case aq::t_int :
		if ( strcmp ( my_field, "NULL")  ==   0 )  *my_int = 'NULL'; // ****
		else  *my_int = atoi ( my_field );
		fwrite( my_int , sizeof(int), 1, fcol  );
		break;

	case aq::t_long_long :
		if ( strcmp ( my_field, "NULL")  ==   0 )  *my_long_long  = 'NULL'; // ****  
#ifdef WIN32
		else  *my_long_long  = _atoi64 (my_field );   
#else
		else  *my_long_long  = atoll (my_field );   
#endif
		fwrite( my_long_long , sizeof(long long), 1, fcol );
		break;

	case aq::t_double :
		if (  strcmp ( my_field, "NULL")  ==   0 )  *my_double = 'NULL'; // ****  
		else
		{
			// step 1 convert ',' in '.'
			ChangeCommaToDot (  my_field );
			// step 2 : use strtod
			*my_double =     strtod ( my_field, NULL );  // atof  ( field );
		}
		fwrite( my_double, sizeof(double), 1, fcol );
		break;

	case aq::t_date1 :
	case aq::t_date2 :
	case aq::t_date3 :
		{
			if ( (strcmp ( my_field, "NULL") == 0) || (strcmp( my_field, "" ) == 0) )  
      {
				*my_long_long  = 'NULL'; // ****
      }
			else
			{
				dateConverter.dateToBigInt(my_field);
        fwrite( my_long_long , sizeof(long long), 1, fcol );
			}
		}
		break;

	case aq::t_char :
		// check my_field size
		if ( (int) strlen ( my_field ) >= col_size ) my_field[ col_size ] = 0 ;
		// clean all space at the end
		CleanSpaceAtEnd (my_field );
		// write string record and go to next
		fwrite(my_field, sizeof(char), strlen( my_field ) , fcol );
		fwrite("\0",sizeof(char),1, fcol );
		break;

	default:
    throw aq::generic_error(aq::generic_error::TYPE_MISMATCH, "type de colonne non traite. Row: %d \n", rowNr);
		break;
	}
}

//-------------------------------------------------------------------------------
char* DatabaseLoader::nettoie_nom_vite( char* strval )
{
	if( !strval )
		return NULL;
	size_t len = strlen( strval );
	if( len < 2 )
		return NULL;
	if( strval[0] != '"' || strval[len - 1] != '"' )
		return NULL;
	strval[len - 1] = '\0';
	return strval + 1;
}

}
