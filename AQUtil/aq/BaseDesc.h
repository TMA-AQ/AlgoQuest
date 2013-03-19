#ifndef __AQ_BASE_DESC_H__
#define __AQ_BASE_DESC_H__

#include "symbole.h"
#include <cstdio>

namespace aq
{

// Les structures de description de la base
// ------------------------------------------------------------------------------
typedef struct s_col_double 
{
	int nb_decimale_max; // if Float, double : precision
	char decimale_separator ; // if Float, double 
} s_col_double;

typedef struct s_col_struct_v2
{
	char *nom;
	int num;
	symbole type; // Chaine, entier, réel, date etc
	char cadrage[1+1];// G(auche) ou D(roite)
	int taille; // taille max du champs en caractère (pour sizeof)
	// s_col_double double_info; // float or double
} s_col_struct_v2;

typedef struct s_table_struct_v2
{
	char *nom;
	int num;
	int nb_enreg; // nombre 'enreg dans la table au loading
	int nb_cols;
	s_col_struct_v2 *colonne; // une colonne par enregistrement
} s_table_struct_v2;

typedef struct s_base_v2
{
	char *nom;
	int num;
	int nb_tables;
	s_table_struct_v2 *table; //une table par enregistrement
} s_base_v2;

typedef struct s_prefixe
{
	char *base  ;      // nbre de bases max 999
	char *table   ;      // nbre de tables max  9999
	char *colonne ;      // nbre de colonnes max 9999
	char *niveau ;      // nbre de niveau max max 99
	char *paquet  ;      // nbre de paquets  max 999....999
} s_prefixe;

// --------  s_list_int -----------
typedef struct s_list_int
{
	// symbole status_sort;
	// symbole status_shrink;
	int max_size;
	int used_size;
	int *part ;
} s_list_int;

/// Lit le fichier de description d'une base et le place dans une structure s_base_v2
/// Argument : le nom du fichier de description
void construis_base ( FILE* fp, s_base_v2 *base );

}

#endif