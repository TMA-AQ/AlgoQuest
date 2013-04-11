/*
*  base_desc.c
*  cut_in_col_v10
*
*  Created by Ouzi on 23/09/09.
*  Copyright 2009 __MyCompanyName__. All rights reserved.
*
*/

#include "BaseDesc.h"
#include "Utilities.h"
#include "Exceptions.h"
#include "Logger.h"
#include <cstring>
#include <cstdlib>
#include <algorithm>
#include <boost/property_tree/xml_parser.hpp>

#define k_size_max 1024

using namespace aq;

//-------------------------------------------------------------------------------
namespace // anonymous namespace
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

void remove_double_quote(std::string& s)
{
  if (s[0] = '\"') s.erase(0, 1);
  if (s[s.size() - 1] = '\"') s.erase(s.size() - 1, 1);
}

void clean(base_t& base)
{
  base.nb_tables = 0;
  base.nom = "";
  base.num = -1;
  base.table.clear();
}

const char * type_to_string(symbole type)
{
  switch (type)
  {
  case t_int: return "INT"; break;
  case t_long_long: return "BIG_INT"; break;
  case t_date1: return "DATE1"; break;
  case t_date2: return "DATE2"; break;
  case t_date3: return "DATE3"; break;
  case t_char: return "VARCHAR"; break;
  case t_double: return "REAL"; break;
  case t_raw: return "RAW"; break;
  default:
    return "unknown";
    break;
  }
}

void fill_column_type(base_t::table_t::col_t * colonne, const char * type_aux)
{
	// Type connus
	// serie numbers 
	if ((strcmp ( type_aux ,"INT" ) == 0) || (strcmp(type_aux,"NUMBER") == 0))
	{ 
		colonne->type = t_int;
		strcpy(colonne->cadrage,"D"); 
	}
	// long or long_long
	else if ((strcmp(type_aux,"BIG_INT") == 0) || (strcmp(type_aux,"LONG" ) == 0))
	{ 
		colonne->type = t_long_long;
		strcpy(colonne->cadrage,"D"); 
	}
	else if ( strcmp(type_aux,"DATE1") == 0 )
	{
		colonne->type=t_date1;
		strcpy(colonne->cadrage,"D"); 
	}
	else if ( strcmp(type_aux,"DATE2") == 0 )
	{
		colonne->type=t_date2;
		strcpy(colonne->cadrage,"D"); 
	}
	else if ( strcmp(type_aux,"DATE3") == 0 )
	{
		colonne->type=t_date3;
		strcpy(colonne->cadrage,"D"); 
	}
	// serie chars
	else if ((strcmp(type_aux, "CHAR") == 0)  ||
    (strcmp(type_aux, "VARCHAR") == 0) ||
    (strcmp(type_aux, "VARCHAR2") == 0))
	{
		colonne->type=t_char;
		strcpy(colonne->cadrage,"G"); 
	}

	//  2008 06 24
	// serie float, real, double
	else if ((strcmp(type_aux, "FLOAT") == 0) ||
		(strcmp(type_aux, "REAL") == 0) ||
		(strcmp(type_aux, "DOUBLE") == 0))
	{
		colonne->type = t_double;
		strcpy(colonne->cadrage,"D"); 
		/*
		// to fill from : file descriptor or  ini.properties 2009/09/01
		colonne->float_info.nb_decimale_max = k_float_nb_decimal_max; 
		colonne->float_info.decimale_separator =  k_double_separator; 
		*/
	}
	// 2008/06/24

	// serie raws
	else if ( strcmp ( type_aux , "RAW" ) == 0 )
	{
		colonne->type = t_raw;
		strcpy(colonne->cadrage,"G"); 
	}
  else
  {

    // types inconnus 
    aq::Logger::getInstance().log(AQ_CRITICAL, "le Types %s est inconnu\n", type_aux); 
    aq::Logger::getInstance().log(AQ_CRITICAL, "Les types implementes sont INT, NUMBER, LONG, BIG_INT , FLOAT, DOUBLE, REAL, CHAR, VARCHAR2, VARCHAR2, DATE1, DATE2 et DATE3 \n");
    aq::Logger::getInstance().log(AQ_CRITICAL, "Les types non implementes mais declares : RAW  \n");
    
    throw aq::generic_error(aq::generic_error::INVALID_BASE_FILE, "");
  }
}

// -----------------------------------------------------------------------------------------------------------
// lecture du base_desc
// -----------------------------------------------------------------------------------------------------------
// Lit le fichier de description d'une base et le place dans une structure s_base_v2
// Argument : le nom du fichier de description
void construis_colonne ( FILE* fp, base_t::table_t::col_t *colonne )
{
	// Lecture des informations au niveau de la colonne
	// Nom de la colonne, numero, taille en char, type 
	// la cadrage est deduis du type de la colonne

	char nom[k_size_max], type_aux[k_size_max];
	fscanf(fp,"%s %d %d %s", nom, &(colonne->num), &(colonne->taille), type_aux);
	colonne->nom = nom;
  remove_double_quote(colonne->nom);
  fill_column_type(colonne, type_aux);
}

//-----------------------------------------------------------
void construis_table ( FILE* fp, base_t::table_t * table )
{
	// Lecture des informations au niveau de la table
	// Nom de la table, numero, nombre de lignes et nombre de colonnes 
	char nom[k_size_max];
	fscanf(fp,"%s %d %d %d", nom, &(table->num), &(table->nb_enreg), &(table->nb_cols));
	table->nom = nom;
  remove_double_quote(table->nom);
	// Lecture des colonnes
	for (int i = 0 ; i < table->nb_cols ; i++)
	{
    base_t::table_t::col_t c;
		construis_colonne(fp, &c);
    table->colonne.push_back(c);
	}
}

} // end anonynous namespace

//-----------------------------------------------------------
namespace aq
{
  
void build_base_from_raw ( const char * fname, base_t& base )
{
  FILE * fp = fopenUTF8(fname, "r");
  if (fp != NULL)
  {
    build_base_from_raw(fp, base);
  }
  else // leave base empty
  {
    clean(base);
  }
}

void build_base_from_raw ( FILE* fp, base_t& base )
{
   clean(base);
	// Lecture des informations au niveau de la base
	// Nom de la base et nombre de tables
	char nom[k_size_max];
	fscanf(fp,"%s %d",nom ,&(base.nb_tables));
	base.nom = nom;
	base.num = 1; // **** non renseigne dans base pour l'instant
	// Lecture des tables 
	for (int i = 0 ; i < base.nb_tables ; i++)
	{
    base_t::table_t table;
		construis_table(fp, &table);
    base.table.push_back(table);
	}
}

void dump_base(std::ostream& oss, const base_t& base)
{
  oss << base.nom << std::endl;
  std::for_each(base.table.begin(), base.table.end(), [&] (const base_t::table_t& table) {
    oss << "  " << table.nom << std::endl;
    std::for_each(table.colonne.begin(), table.colonne.end(), [&] (const base_t::table_t::col_t& column) {
      oss << "  " << "  " << column.nom << std::endl;
    });
  });
}

void build_base_from_xml ( std::istream& is, base_t& base )
{
  clean(base);
  boost::property_tree::ptree parser;
  boost::property_tree::xml_parser::read_xml(is, parser);
  base.nom = parser.get<std::string>("Database.<xmlattr>.Name");
  boost::property_tree::ptree tables = parser.get_child("Database.Tables");
  std::for_each(tables.begin(), tables.end(), [&] (const boost::property_tree::ptree::value_type& ptTable) {
    base_t::table_t table;
    table.nom = ptTable.second.get<std::string>("<xmlattr>.Name");
    table.num = ptTable.second.get<int>("<xmlattr>.ID");
    table.nb_enreg = ptTable.second.get<int>("<xmlattr>.NbRows");
    boost::property_tree::ptree columns = ptTable.second.get_child("Columns");
    std::for_each(columns.begin(), columns.end(), [&] (const boost::property_tree::ptree::value_type& ptColumn) {
      base_t::table_t::col_t col;
      col.nom = ptColumn.second.get<std::string>("<xmlattr>.Name");
      col.num = ptColumn.second.get<int>("<xmlattr>.ID");
      col.taille = ptColumn.second.get<int>("<xmlattr>.Size");
      std::string type = ptColumn.second.get<std::string>("<xmlattr>.Type");
      fill_column_type(&col, type.c_str());
      table.colonne.push_back(col);
    });
    table.nb_cols = static_cast<int>(table.colonne.size());
    base.table.push_back(table);
  });
  base.nb_tables = static_cast<int>(base.table.size());
}

void dump_xml_base(std::ostream& oss, const base_t& base)
{
  oss << "<Database Name=\"" << base.nom << "\">" << std::endl;
  oss << "<Tables>" << std::endl;
  std::for_each(base.table.begin(), base.table.end(), [&] (const base_t::table_t& table) {
    oss << "<Table Name=\"" << table.nom << "\" ID=\"" << table.num << "\" NbRows=\"" << table.nb_enreg << "\">" << std::endl;
    oss << "<Columns>" << std::endl;
    std::for_each(table.colonne.begin(), table.colonne.end(), [&] (const base_t::table_t::col_t& column) {
      oss << "<Column Name=\"" << column.nom << "\" ID=\""<< column.num << "\" Size=\"" << column.taille << "\" Type=\"" << type_to_string(column.type) << "\"/>" << std::endl;
    });
    oss << "</Columns>" << std::endl;
    oss << "</Table>" << std::endl;
  });
  oss << "</Tables>" << std::endl;
  oss << "</Database>" << std::endl;
}

}
