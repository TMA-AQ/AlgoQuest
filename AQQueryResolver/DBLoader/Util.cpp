/*
*  toolbox.c
*  cut_in_col_v10
*
*  Created by Ouzi on 23/09/09.
*  Copyright 2009 __MyCompanyName__. All rights reserved.
*
*/
// --------------------------------------------------------------------------------------------
#include "Util.h"
#include <cstdlib>
#include <cerrno>
#include <cstring>
#include <aq/DateConversion.h>

using namespace aq;

// --------------------------------------------------------------------------------------------
/*symbole ReadRecord ( FILE *fp2 , char *my_fic2 , char *my_record ,  int max_read )
{
symbole answer;
char * read_status;
// if read is OK, return after reading
if ( fgets ( my_record, max_read, fp2) != NULL ) 
return t_done;

// read not OK check if eof
int error;
error = ferror (fp2 );

if ( error == EOF ) answer = t_eof;
else answer = t_file_read_error;

return answer; 
}*/

// ---------------------------------------------------------------------------------------------
void die_with_error ( char *errorMessage )
{
	printf ( "\nMy Error message  :  %s\n", errorMessage );
	printf ( "Error : %s \n", strerror (errno) ); // Num erreur du systeme 
	fflush (stdout);
	exit ( 0 );
}
// --------------------------------------------------------------------------------------------
symbole  NbFieldInRecord ( char *my_fic2 , char *my_record ,  int max_read, int *nb_fields  , int *nb_pack_col , int *nb_col_in_last_pack )
{
	FILE *fp2;

	int len_rec , i ;
	// open source file
	if (( fp2 = fopenUTF8( my_fic2 , "r" )) == NULL)
	{
		printf("Table  %s  absente\n",my_fic2);
		fflush (stdout);
		return t_continue ; // passe à la table suivante
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

	return  t_done;
}
// --------------------------------------------------------------------------------------------
void redirect_std_out ( char *name_out, FILE *f_out )
{ 
	f_out = freopen (name_out, "wt", stdout);
	if (f_out != stdout)
	{
		fprintf(stderr,"cannot open output file %s \n",name_out);
		exit (2);
	}
}
// --------------------------------------------------------------------------------------------
// void my_copie_secure ( char * source, char *cible )
// {
//  // copie sécurisé : 
//  // sizeof( cible ) = max_size;
//  // si strlen(source) > max_size alors source[max_size] = 0  puis strcpy
//  int max_size = sizeof (cible);
//  
//  if ( strlen (source) >= max_size)   source[max_size - 1] = 0;
//  
//  strcpy ( cible, source );
// }
// -----------------------------------------------------------------------------------------------------------
void nettoie_nom (char *nom)
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
void *safecalloc(size_t nb, size_t size)
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
void allocate_and_init_prefixe ( s_prefixe *prefixe )
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
void init_format_column ( char *format , s_prefixe *prefixe )
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
void init_format_column_short ( char *format , s_prefixe *prefixe )
{
	// noat : strcat add \0 automatically
	strcpy  ( format , "T%"  );
	strcat  ( format , prefixe->table );
	strcat  ( format , "dC%"  );
	strcat  ( format , prefixe->colonne );
	strcat  ( format , "d"  ); 
}
//-------------------------------------------------------------------------------
void   CleanSpaceAtEnd ( char *my_field )
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
/*
double my_atof (const char *nptr)
{
double res;
//  locale_t new_locale, prev_locale;
lconv new_locale, prev_locale;

new_locale = newlocale (LC_NUMERIC_MASK, "C", NULL);
prev_locale = uselocale (new_locale);
res = atof (nptr);
uselocale (prev_locale);
freelocale (new_locale);

return res;

}*/
//-------------------------------------------------------------------------------
void ChangeCommaToDot (  char *string )
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
void check_args (const char * iniFilename, size_t table, size_t column)
{
	FILE *fp = NULL;
	char line[1024];
	char lside[1024], rside[1024];
	char comment_char = '#';
	char* posChr = NULL;
	csv_format = 0;

	if ((fp = fopenUTF8(iniFilename, "r")) == NULL)
	{
		fprintf(stderr, "Error opening property file: %s\n", iniFilename);
		exit(0);
	}

	// scan ini.properties
	while (fgets(line, sizeof(line), fp) != NULL) {

		if (line[0] != comment_char && strlen(line) >3)
		{
			//sscanf(line, "%[0-9a-zA-Z. _~\\/+-*?!.,;:\(\\)@'\"#\[ ]=%[0-9a-zA-Z. _~\\/+-*?!.,;:\(\\)@'\"#\[ ]", lside, rside);
			posChr=strchr(line,'=');
			if( posChr == NULL )
				continue;
			*posChr = '\0';
			strcpy( lside, line );
			strcpy( rside, posChr + 1 );

			while (lside[0]==' ') strcpy(lside, lside+1);
			size_t l = strlen(lside);
			if (l > 0)
				while (lside[l-1]==' ') lside[l---1]='\0';

			while (rside[0]==' ') strcpy(rside, rside+1);
			l = strlen(rside);
			if (l > 0)
				while (rside[l-1]<=' ') rside[l---1]='\0';

			// printf("%s = %s\n", lside, rside);

			if (strcmp(lside, "k_file_name_size_max") == 0)
			{
				sscanf(rside, "%d", k_file_name_size_max);
			}
			else if (strcmp(lside, "k_taille_paquet") == 0)
			{
				// sscanf(rside, "%d", k_taille_paquet);
				// sscanf(rside, "%d", 1048576);
			}
			else if ( strcmp(lside, "loader") == 0 ) // loader batch
			{
				strcpy( k_batch_loader , rside);
				size_t len = strlen( k_batch_loader );
				if (len > 0)
					for( int idx = 1; idx < len - 1; ++idx )
						if( k_batch_loader[idx-1] == '"' && 
							k_batch_loader[idx] == ',' &&
							k_batch_loader[idx+1] == '"' )
							k_batch_loader[idx] = ' ';
			}
			else if (strcmp(lside, "root-folder") == 0)
			{
				strcpy(k_rep_racine, rside);
			}
			else if (strcmp(lside, "field-separator") == 0)
			{
				end_of_field = rside[0];
			}
			else if (strcmp(lside, "csv-format") == 0)
			{
				csv_format = atoi(rside);
			}
		}
	}
	fclose(fp);
	
	sprintf(base_desc_file, "%sbase_struct/base", k_rep_racine);
	if( csv_format  )  end_of_field = ',';
}
//-------------------------------------------------------------------------------
FILE* fopenUTF8(const char* filename, const char* mode)
{
	FILE	*fp = NULL;
	fp = fopen(filename, mode);
	if( fp == NULL )
		return NULL;

	/* Skip UTF-8 BOM if present */
	unsigned char bom[3];
	if( fread( bom, 3, sizeof(char), fp ) != 1 )
	{
		fclose( fp );
		return NULL;
	}
	if( !((bom[0] == 0xEF) && (bom[1] == 0xBB) && (bom[2] == 0xBF)) )
	{
		/* BOM not present, return to the beginning */
		if ( fseek( fp, 0, SEEK_SET ) != 0 ) {
			fclose( fp );
			return NULL;
		}
	}
	return fp;
}
//-------------------------------------------------------------------------------
void FileWriteEnreg(symbole col_type, int col_size, char *my_field, FILE *fcol, int rowNr)
{
	int dum_int;
	int * my_int = & dum_int;

	double dum_double; // 2009/09/01 
	double * my_double = &dum_double; // 2009/09/01 

	long long int dum_long_long;
	long long int *my_long_long = &dum_long_long;

	if ( (int) strlen ( my_field ) >= col_size ) my_field[ col_size ] = 0 ;

	switch (  col_type )
	{
	case t_int :
		if ( strcmp ( my_field, "NULL")  ==   0 )  *my_int = 'NULL'; // ****
		else  *my_int = atoi ( my_field );
		fwrite( my_int , sizeof(int), 1, fcol  );
		break;

	case t_long_long :
		if ( strcmp ( my_field, "NULL")  ==   0 )  *my_long_long  = 'NULL'; // ****  
#ifdef WIN32
		else  *my_long_long  = _atoi64 (my_field );   
#else
		else  *my_long_long  = atoll (my_field );   
#endif
		fwrite( my_long_long , sizeof(long long), 1, fcol );
		break;

	case t_double :
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

	case t_date1 :
	case t_date2 :
	case t_date3 :
		{
			char *dateBuf;
			DateType dateType;
			switch( col_type )
			{
			case t_date1 :
				dateBuf = "DD/MM/YYYY HH:MM:SS";
				dateType = DDMMYYYY_HHMMSS;
				break;
			case t_date2 :
				dateBuf = "DD/MM/YYYY";
				dateType = DDMMYYYY;
				break;
			case t_date3 :
				dateBuf = "DD/MM/YY";
				dateType = DDMMYY;
				break;
			default:
				printf ("type de colonne  non traite \n");
			}
			if ( (strcmp ( my_field, "NULL") == 0) || (strcmp( my_field, "" ) == 0) )  
				*my_long_long  = 'NULL'; // ****
			else
			{
				if ( dateToBigInt(my_field, dateType, my_long_long) )
				{
					fwrite( my_long_long , sizeof(long long), 1, fcol );
				}
				else
				{
					sprintf ( a_message, "Champ DATE invalide. "
						"Format attendu: %s. Champ: %s. Row: %d", dateBuf, my_field, rowNr );
					die_with_error ( a_message );
				}
			}
		}
		break;

	case t_char :
		// check my_field size
		if ( (int) strlen ( my_field ) >= col_size ) my_field[ col_size ] = 0 ;
		// clean all space at the end
		CleanSpaceAtEnd (my_field );
		// write string record and go to next
		fwrite(my_field, sizeof(char), strlen( my_field ) , fcol );
		fwrite("\0",sizeof(char),1, fcol );
		break;

	default:
		sprintf( a_message, "type de colonne non traite. Row: %d \n", rowNr );
		die_with_error( a_message );
		break;
	}
}
//-------------------------------------------------------------------------------
char* nettoie_nom_vite( char* strval )
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
//-------------------------------------------------------------------------------



