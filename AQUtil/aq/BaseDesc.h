#ifndef __AQ_BASE_DESC_H__
#define __AQ_BASE_DESC_H__

#include "Symbole.h"
#include <cstdio>
#include <vector>
#include <sstream>

namespace aq
{

static const unsigned int packet_size = 1048576;

// -------------------------------------------------------------
struct base_t
{
  struct table_t
  {
    struct col_t
    {
      std::string name;
      int id; // FIXME
      symbole type;
      int size; ///< number of elements
      int getSize() const;
    };
    typedef std::vector<col_t> cols_t;

    std::string name;
    int id; // FIXME
    int nb_record; // FIXME
    cols_t colonne;
  };
  typedef std::vector<table_t> tables_t;

	std::string name;
	int id; // FIXME
	tables_t table;
};

/// Lit le fichier de description d'une base et le place dans une structure s_base_v2
/// Argument : le nom du fichier de description
int build_base_from_raw ( const char * fname, base_t& base );
void build_base_from_raw ( FILE* fp, base_t& base );
void build_base_from_raw ( std::istream& is, base_t& base );
void build_base_from_xml ( std::istream& is, base_t& base );

void dump_raw_base(std::ostream& oss, const base_t& base);
void dump_xml_base(std::ostream& oss, const base_t& base);

}

#endif
