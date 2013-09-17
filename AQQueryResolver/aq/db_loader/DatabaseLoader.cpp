// include files 

#include "DatabaseLoader.h"
#include "Util.h"

#include <cstring>
#include <cstdlib>
#include <ctime>
#include <algorithm>

// Version 1.0 : Decoupe en colonne les tables origine (OK)
// Version 1.1 : Remplace les end_of_record ( 'CR' et/ou 'LF') par '0'  (OK)
// Version 1.2 : Creation automatique de la colonne pivot pour chaque table (en test)
// Version 1.3 : Ajout du client : Demo (en test)
// Version 1.4  : lecture du base_desc a partir duquel charge le nom des tables et des colonnes
// version 1.5 : lecture du fichier des parametres
// version 1.6 : limite la longueur de chaque champs à k_max_field_size
// version 1.7 : decoupe les tables par paquet de 1048576 lignes
// version 1.8 : limite la longueur de chaque champs à la taille déclarée dans base->table->colonne->taille
// version 1.9 : convertit le fichier en fonction de la nature du contenu : si int alors string->int
// version 1.9.1 : lecture des parametres depuis un fichier ini.properties compatible Java et passe en argument
// version 1.9.2 : change le test de taille paquet pour bien avoir des paquet de 1048576 lignes
// version 2.0 : teste le type de colonne avant d'accepter de decouper
// version 2.1 :structure nom fichier : BxxTxxxxCxxxxPxxxxxx 
// version 2.2 :pas de limite de taille de champs
// 2008 06 24 / Integration floats
// 2009/09/02 : les float sont toujours vus comme des double
// 2009/10/31 : les float et les doubles sont vus commes des int avec le decalage de virgule qui va bien

//2012/09/10 : parametres : ini.properties num_ta able  num_col : quand un paquet est construit, lance le loader
//     avec les paramètres : ini.properties num_table num_col num_paquet pour lier la construction d'un paquet et son loading

// -----------------------------------------------------------------------------------------------------------
//  Compilation : gcc -o cut_in_col main.c
// -----------------------------------------------------------------------------------------------------------
// declarations des fonctions
// -----------------------------------------------------------------------------------------------------------
//-------------------------------------------------------------------------------
// variables globales
//-------------------------------------------------------------------------------
char a_message [ k_taille_message ];
char k_rep_racine[k_file_name_size_max] = ".";          // a passer en argument
char k_rep_param[k_file_name_size_max] = "parameters/"; //  a passer en argument
char k_batch_loader [ k_batch_size_max ]; // from ini.properties
char ini_filename [ k_batch_size_max ];
// int k_packet_size = 1048576;
// parameter for vdg construction
// FLOAT INFOS to get from file descriptor or ini.properties
char k_double_separator = ',';
int csv_format;
// int k_float_nb_decimal_max = 2 ;

char *rep_source;
char *rep_cible;
char *base_desc_file;
unsigned char end_of_field; //field separator  

int rc;

char name_log [ k_taille_message ];
//-------------------------------------------------------------------------------
int cut_in_col (const char * iniFilename, size_t num_table, size_t num_column) 
{
	// parametres des emplacement des fichiers 
	rep_source =(char *) safecalloc ( k_file_name_size_max, sizeof(char) );
	rep_cible =(char *) safecalloc ( k_file_name_size_max , sizeof(char) );
	base_desc_file =  (char *) safecalloc ( k_file_name_size_max, sizeof(char) );

	check_args(iniFilename, num_table, num_column);
	// ----------- declaration des variables ----------------

	FILE *fp2; // fichier source
	FILE *fcol; // col destination du decoupage + binaire
	int  len_rec ;
	int i ;
	int nb_pack_col;
	int nb_col_in_last_pack;
	int debut_lecture;
	int  cur_col ;

	int total_nb_enreg =0;
	char *my_record; 
	my_record  = (char *) safecalloc ( k_record_size_max, sizeof(char)); 
	char *my_field; 
	my_field  = (char *) safecalloc ( k_field_size_max, sizeof(char)); 
	char *my_col_dir;
	my_col_dir  = (char *) safecalloc (k_file_name_size_max, sizeof(char));   
	char *my_dir_source;
	my_dir_source  = (char *) safecalloc (k_file_name_size_max, sizeof(char));
	char *my_dir_cible;
	my_dir_cible  = (char *) safecalloc (k_file_name_size_max, sizeof(char)); 

	// init
	aq::s_prefixe prefixe;
	allocate_and_init_prefixe( &prefixe);

	// info on curent column in parameters
	aq::symbole col_type;
	int  col_size ;
	char *my_col;
	my_col  = (char*) safecalloc(k_file_name_size_max, sizeof(char));

	char *my_fic2;
	my_fic2  = (char *) safecalloc(k_file_name_size_max, sizeof(char)); 

	int max_nb_table; // nbre de fichier en fonction du client

	char my_string [1+1];

	// format for sprintf 
	char *format_file_name ;
	format_file_name = (char*) safecalloc(4096 , sizeof (char));
	init_format_column(format_file_name, &prefixe);

	// initialisation des composants de nom de fichiers
	int n_base = 1;
	int n_table = 1;
	int i_table = 0 ; // loop table index   
	int n_paquet = 0;

	// Declaration de la description de la base
	aq::base_t my_base;
	// ------------------------------------------------------------------------------------------------------------
	// Ouverture du fichier parametres et remplissage de source et cible
	// ------------------------------------------------------------------------------------------------------------
	// char *jointure_desc_file;
	FILE *fp;

	strcpy(rep_source, k_rep_racine);
	strcat(rep_source, "data_orga/tables/");
	strcpy(rep_cible, k_rep_racine);
	strcat(rep_cible, "data_orga/columns/");

	// -------------------------------------------------------------------------------------
	// Generate temporary loader ini file
	// -------------------------------------------------------------------------------------
	strcpy(ini_filename, k_rep_racine);
	strcat(ini_filename, "loader.ini");
	FILE * fini = fopen(ini_filename, "w");
	if (!fini)
	{
		fprintf(stderr, "cannot create file %s\n", ini_filename);
		exit(-1);
	}

	fwrite("export.filename.final=", 1, 22, fini);
	fwrite(k_rep_racine, 1, strlen(k_rep_racine), fini);
	fwrite("base_struct/base.", 1, 17, fini);
	fwrite("\n", 1, 1, fini);
	fwrite("step1.field.separator=", 1, 22, fini);
	fwrite(&end_of_field, 1, 1, fini);
	fwrite("\n", 1, 1, fini);
	fwrite("k_rep_racine=", 1, 13, fini);
	fwrite(k_rep_racine, 1, strlen(k_rep_racine), fini);
	fwrite("\n", 1, 1, fini);
	fwrite("k_rep_racine_tmp=", 1, 17, fini);
	fwrite(k_rep_racine, 1, strlen(k_rep_racine), fini);
	fwrite("\n", 1, 1, fini);
  fwrite("k_taille_nom_fichier=1024", 1, 25, fini);
	fwrite("\n", 1, 1, fini);
	fclose(fini);

	// ----------------------
	// declaration de la date et l'heure
	// --------------------------
	time_t t;
	struct tm *tb; 
	// -----------------------------------
	// déclaration du log file
	// -----------------------------------
	char format_file_name_short[k_taille_message];
	init_format_column_short( format_file_name_short, &prefixe );
	char name_log_file[k_taille_message];
	sprintf( name_log_file, format_file_name_short, num_table, num_column);
	sprintf( name_log, "%sdata_orga/Cut_in_col_log_%s.txt", k_rep_racine, name_log_file );

	fclose( stdout );
	FILE    *f_out = NULL;   // output file
	
	redirect_std_out  ( name_log,  f_out );
	// answer_size : nb of lines in each file
	// display exec date and hour
	t = time ( NULL );
	tb = localtime( &t );
	printf(" Start on  %d/%02d/%02d H %02d:%02d:%02d\n" , tb->tm_year +1900, tb->tm_mon +1 ,tb->tm_mday, tb->tm_hour, tb->tm_min, tb->tm_sec );





	// --------------------------------------------------------------------------------------
	// Ouverture du fichier et remplissage du s_base_v2 b
	// --------------------------------------------------------------------------------------
	// chargement de la description de la base
	if ((fp=fopenUTF8(base_desc_file,"r"))==NULL) 
	{
		sprintf ( a_message, "error opening file %s\n",base_desc_file );
		die_with_error ( a_message );
	}
	aq::build_base_from_raw ( fp , my_base );
	fclose (fp); // fermeture du fichier base_desc

	// -------------------------------------------------------------------------------------- 
	// declaration des fichier a decouper
	// -------------------------------------------------------------------------------------- 
	printf ("\n");
	// printf ("step 1 \n");
	max_nb_table =  my_base.nb_tables;
	char *cust_name;
	cust_name = (char *) safecalloc ( k_file_name_size_max, sizeof(char));
	// alloc cust_file_name
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
  
	// printf ("step 2 \n");
	// declaration des repertoire source et cible
	strcpy (my_dir_source ,rep_source);
	strcpy (my_dir_cible ,rep_cible);

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

		//----------------------
		// check n_table
		//----------------------
		if ( n_table != num_table )
			continue;
		//----------------------

		printf("\n Table : %u \n", n_table );
		// curent file to cut in col
		strcpy ( my_col_dir , my_dir_cible ); // toutes les colonnes directement dans la bon repertoire  
		sprintf ( my_fic2,"%s%s",  my_dir_source, cust_file_name[i_table]);

		len_rec = 0 ;
		nb_fields = 1; // one field at least !
		// initialisation du num_paquet
		n_paquet = 0;


		// calculate the number of fields in a record
		if(  NbFieldInRecord ( my_fic2 , my_record ,  max_read,  &nb_fields  , &nb_pack_col , &nb_col_in_last_pack ) == aq::symbole::t_continue )  continue;
		
		// open source file and check if opened
		if( (fp2 = fopenUTF8(my_fic2, "r")) == NULL )
		{
			sprintf(a_message, "error opening file %s\n", my_fic2);
			die_with_error(a_message);
		}

		// table and colonne : OK
		// creation des noms de fichiers : nouvelle structure
		sprintf ( my_col, format_file_name , my_col_dir,  n_base, n_table, num_column, n_paquet);
		// init col infos
		col_type = my_base.table[i_table].colonne[num_column - 1].type;
		col_size = my_base.table[i_table].colonne[num_column - 1].taille;
		printf( "%s max_size: %d\n", my_col, col_size );
		if( (fcol = fopen ( my_col ,"w+b")) == NULL )  //  '+ ' : erase former file if exist 2009/10/27
		{
			sprintf(a_message, "error opening file %s\n", my_col);
			die_with_error(a_message);
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
				sprintf( a_message, "error reading record  %d, loading aborted\n", total_nb_enreg );
				die_with_error(a_message);
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
				sprintf( a_message, "\"%s %s %d %d %d\"\n", k_batch_loader, ini_filename, num_table , num_column , n_paquet );
				// loader 
				system( a_message );

				// next packet
				n_paquet++;
				// ré-initiaisation du write_n_enreg
				write_n_enreg = 1; 
				//remove( my_col );

				// ouverture de la colonne pour le nouveau packet
				sprintf ( my_col, format_file_name , my_col_dir,  n_base, n_table, num_column, n_paquet);
				if( (fcol = fopen(my_col, "w+b")) == NULL ) //  '+ ' : erase former file if exist 2009/10/27
				{
					sprintf(a_message, "error opening file %s\n", my_col);
					die_with_error(a_message);
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
				// NULL value in Pages jaunes marketing services 2011/02/23
				check_empty_1 =  my_record [ i ];
				check_empty_2 =  my_record [ i  + 1 ];


				my_char =    my_record[ i ];       
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
							sprintf( a_message, "Format CSV mais champ sans \"\". Row: %d.\n", 
								total_nb_enreg );
							die_with_error( a_message );
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
		sprintf( a_message, "\"%s %s %d %d %d\"\n", k_batch_loader, ini_filename, num_table , num_column , n_paquet );
		// loader 

		rc = system( a_message );
		if (rc != 0)
		{
			fprintf(stderr, "loader error '%s'", a_message);
		}

		// changement de pack
		// close the table source file
		fclose(fp2);
		printf("nbre de ligne dans la table %s : %u \n", my_fic2, total_nb_enreg);
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
	printf ( "\nDone ! \n" ); 
	// end time
	t = time ( NULL );
	tb = localtime( &t );
	printf(" End on  %d/%02d/%02d H %02d:%02d:%02d\n" , tb->tm_year +1900, tb->tm_mon +1 ,tb->tm_mday, tb->tm_hour, tb->tm_min, tb->tm_sec );

	return 0;
}
//------------------------------
