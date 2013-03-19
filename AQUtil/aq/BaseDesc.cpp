/*
*  base_desc.c
*  cut_in_col_v10
*
*  Created by Ouzi on 23/09/09.
*  Copyright 2009 __MyCompanyName__. All rights reserved.
*
*/

#include "BaseDesc.h"
#include <cstring>
#include <cstdlib>

#define k_size_max 1024

using namespace aq;

//-------------------------------------------------------------------------------
namespace
{
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
}

// -----------------------------------------------------------------------------------------------------------
// lecture du base_desc
// -----------------------------------------------------------------------------------------------------------
// Lit le fichier de description d'une base et le place dans une structure s_base_v2
// Argument : le nom du fichier de description
void construis_colonne ( FILE* fp, s_col_struct_v2 *colonne )
{
	// Lecture des informations au niveau de la colonne
	// Nom de la colonne, numero, taille en char, type 
	// la cadrage est deduis du type de la colonne

	char nom[k_size_max], type_aux[k_size_max];
	size_t len;

	fscanf(fp,"%s %d %d %s", nom, &(colonne->num), &(colonne->taille), type_aux);
	// Type connus
	// serie numbers 
	if ( strcmp ( type_aux ,"INT" ) == 0        ||
		strcmp(type_aux,"NUMBER") == 0
		)
	{ 
		colonne->type = t_int;
		strcpy(colonne->cadrage,"D"); 
		goto fin_controle_type;
	}
	// long or long_long
	if ( strcmp(type_aux,"BIG_INT") == 0||
		strcmp(type_aux,"LONG" ) == 0
		)
	{ 
		colonne->type = t_long_long;
		strcpy(colonne->cadrage,"D"); 
		goto fin_controle_type;
	}
	if ( strcmp(type_aux,"DATE1") == 0 )
	{
		colonne->type=t_date1;
		strcpy(colonne->cadrage,"D"); 
		goto fin_controle_type;
	}
	if ( strcmp(type_aux,"DATE2") == 0 )
	{
		colonne->type=t_date2;
		strcpy(colonne->cadrage,"D"); 
		goto fin_controle_type;
	}
	if ( strcmp(type_aux,"DATE3") == 0 )
	{
		colonne->type=t_date3;
		strcpy(colonne->cadrage,"D"); 
		goto fin_controle_type;
	}
	// serie chars
	if ( strcmp ( type_aux , "CHAR" ) == 0  ||
		strcmp(type_aux,"VARCHAR") == 0 ||
		strcmp(type_aux,"VARCHAR2") == 0 )
	{
		colonne->type=t_char;
		strcpy(colonne->cadrage,"G"); 
		goto fin_controle_type;
	}

	//  2008 06 24
	// serie float, real, double
	if ( strcmp ( type_aux , "FLOAT" ) == 0 ||
		strcmp ( type_aux , "REAL"  ) == 0  ||
		strcmp ( type_aux , "DOUBLE"  ) == 0  )
	{
		colonne->type = t_double;
		strcpy(colonne->cadrage,"D"); 
		/*
		// to fill from : file descriptor or  ini.properties 2009/09/01
		colonne->float_info.nb_decimale_max = k_float_nb_decimal_max; 
		colonne->float_info.decimale_separator =  k_double_separator; 
		*/
		goto fin_controle_type;
	}
	// 2008/06/24

	// serie raws
	if ( strcmp ( type_aux , "RAW" ) == 0 )
	{
		colonne->type = t_raw;
		strcpy(colonne->cadrage,"G"); 
		goto fin_controle_type;
	}

	// types inconnus 
	printf("le Types %s est inconnu\n", type_aux ); 
	printf("Les types implementes sont INT, NUMBER, LONG, BIG_INT , FLOAT, DOUBLE, REAL, CHAR, VARCHAR2, VARCHAR2, DATE1, DATE2 et DATE3 \n");
	printf("Les types non implementes  mais declares  :   RAW  \n");
	exit(1);


fin_controle_type:

	// charge le nom de la colonne                                       	
	len = strlen(nom) + 1;
	colonne->nom=(char *) safecalloc(len,sizeof(char));
	strcpy(colonne->nom,nom);
}
//-----------------------------------------------------------
void construis_table ( FILE* fp, s_table_struct_v2 *table )
{
	// Lecture des informations au niveau de la table
	// Nom de la table, numero, nombre de lignes et nombre de colonnes 
	char nom[k_size_max];
	size_t len;
	int i;
	fscanf(fp,"%s %d %d %d",nom,&(table->num), &(table->nb_enreg)  ,&(table->nb_cols));
	len = strlen(nom)+1;
	table->nom=(char *) safecalloc(len,sizeof(char));
	strcpy(table->nom,nom);
	// Lecture des colonnes
	table->colonne = (s_col_struct_v2 *) safecalloc(table->nb_cols,sizeof(s_col_struct_v2));
	for ( i = 0 ; i < table->nb_cols ; i++ )
	{
		construis_colonne(fp,table->colonne+i);
	}
}
//-----------------------------------------------------------
namespace aq
{

void construis_base ( FILE* fp, s_base_v2 *base )
{
	// Lecture des informations au niveau de la base
	// Nom de la base et nombre de tables
	char nom[k_size_max];
	size_t len;
	int i;
	fscanf(fp,"%s %d",nom ,&(base->nb_tables));

	len = strlen(nom)+1;
	base->nom=(char *) safecalloc(len,sizeof(char));
	strcpy(base->nom,nom);

	base->num = 1; // **** non renseigne dans base pour l'instant


	// Lecture des tables 
	base->table = (s_table_struct_v2 *) safecalloc(base->nb_tables,sizeof(s_table_struct_v2));
	for ( i = 0 ; i < base->nb_tables ; i++ )
	{
		construis_table(fp,base->table+i);
	}
}

}
