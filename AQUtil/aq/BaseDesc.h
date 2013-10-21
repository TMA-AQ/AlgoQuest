#ifndef __AQ_BASE_DESC_H__
#define __AQ_BASE_DESC_H__

#include "Symbole.h"
#include <cstdio>
#include <vector>
#include <sstream>

namespace aq
{

static const unsigned int packet_size = 1048576;

// Les structures de description de la base
// ------------------------------------------------------------------------------
typedef struct s_col_double 
{
	int nb_decimale_max; // if Float, double : precision
	char decimale_separator ; // if Float, double 
} s_col_double;

typedef struct s_prefixe
{
	char *base  ;      // nbre de bases max 999
	char *table   ;      // nbre de tables max  9999
	char *colonne ;      // nbre de colonnes max 9999
	char *niveau ;      // nbre de niveau max max 99
	char *paquet  ;      // nbre de paquets  max 999....999
} s_prefixe;

typedef struct s_list_int
{
	// symbole status_sort;
	// symbole status_shrink;
	int max_size;
	int used_size;
	int *part ;
} s_list_int;

// -------------------------------------------------------------
// BaseDesc
struct base_t
{
  struct table_t
  {
    struct col_t
    {
      std::string nom;
      int num;
      symbole type; // Chaine, entier, réel, date etc
      char cadrage[2];// G(auche) ou D(roite)
      int taille; // taille max du champs en caractère (pour sizeof)
    };
    typedef std::vector<col_t> cols_t;

    std::string nom;
    int num;
    int nb_enreg; // nombre 'enreg dans la table au loading
    int nb_cols;
    cols_t colonne; // une colonne par enregistrement
  };
  typedef std::vector<table_t> tables_t;

	std::string nom;
	int num;
	int nb_tables;
	tables_t table; //une table par enregistrement
};

/// Lit le fichier de description d'une base et le place dans une structure s_base_v2
/// Argument : le nom du fichier de description
int build_base_from_raw ( const char * fname, base_t& base );
void build_base_from_raw ( FILE* fp, base_t& base );
void build_base_from_xml ( std::istream& is, base_t& base );

void dump_raw_base(std::ostream& oss, const base_t& base);
void dump_xml_base(std::ostream& oss, const base_t& base);

}

#endif
