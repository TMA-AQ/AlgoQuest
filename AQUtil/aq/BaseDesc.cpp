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
  if (s[0] == '\"') s.erase(0, 1);
  if (s[s.size() - 1] == '\"') s.erase(s.size() - 1, 1);
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
  default: break;
  }
  return "unknown";
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
	else if ( strcmp(type_aux,"DATE1") == 0 || strcmp(type_aux,"DATE") == 0)
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
void construis_colonne ( std::istream& iss, base_t::table_t::col_t *colonne )
{
  std::string type_aux;
	iss >> colonne->nom;
  iss >> colonne->num;
  iss >> colonne->taille;
  iss >> type_aux;
  remove_double_quote(colonne->nom);
  fill_column_type(colonne, type_aux.c_str());
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

//-----------------------------------------------------------
void construis_table ( std::istream& iss, base_t::table_t * table )
{
  iss >> table->nom;
  iss >> table->num;
  iss >> table->nb_enreg;
  iss >> table->nb_cols;
  remove_double_quote(table->nom);
	for (int i = 0 ; i < table->nb_cols ; i++)
	{
    base_t::table_t::col_t c;
		construis_colonne(iss, &c);
    table->colonne.push_back(c);
	}
}

} // end anonynous namespace

//-----------------------------------------------------------
namespace aq
{
  
int build_base_from_raw ( const char * fname, base_t& base )
{
  int rc = 0;
  FILE * fp = fopenUTF8(fname, "r");
  if (fp != NULL)
  {
    build_base_from_raw(fp, base);
  }
  else // leave base empty
  {
    clean(base);
    rc = -1;
  }
  return rc;
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

void build_base_from_raw ( std::istream& iss, base_t& base )
{
  clean(base);
	iss >> base.nom;
  iss >> base.nb_tables;
	base.num = 1;
	for (int i = 0 ; i < base.nb_tables ; i++)
	{
    base_t::table_t table;
		construis_table(iss, &table);
    base.table.push_back(table);
	}
}


void dump_raw_base(std::ostream& oss, const base_t& base)
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
  for (auto& ptTable : tables) 
  {
    base_t::table_t table;
    table.nom = ptTable.second.get<std::string>("<xmlattr>.Name");
    table.num = ptTable.second.get<int>("<xmlattr>.ID");
    table.nb_enreg = ptTable.second.get<int>("<xmlattr>.NbRows");
    boost::property_tree::ptree columns = ptTable.second.get_child("Columns");
    for (auto& ptColumn : columns) 
    {
      base_t::table_t::col_t col;
      col.nom = ptColumn.second.get<std::string>("<xmlattr>.Name");
      col.num = ptColumn.second.get<int>("<xmlattr>.ID");
      col.taille = ptColumn.second.get<int>("<xmlattr>.Size");
      std::string type = ptColumn.second.get<std::string>("<xmlattr>.Type");
      fill_column_type(&col, type.c_str());
      table.colonne.push_back(col);
    }
    table.nb_cols = static_cast<int>(table.colonne.size());
    base.table.push_back(table);
  }
  base.nb_tables = static_cast<int>(base.table.size());
}

void dump_xml_base(std::ostream& oss, const base_t& base)
{
  oss << "<Database Name=\"" << base.nom << "\">" << std::endl;
  oss << "<Tables>" << std::endl;
  for (auto& table : base.table) 
  {
    oss << "<Table Name=\"" << table.nom << "\" ID=\"" << table.num << "\" NbRows=\"" << table.nb_enreg << "\">" << std::endl;
    oss << "<Columns>" << std::endl;
    for (auto& column : table.colonne) 
    {
      oss << "<Column Name=\"" << column.nom << "\" ID=\""<< column.num << "\" Size=\"" << column.taille << "\" Type=\"" << type_to_string(column.type) << "\"/>" << std::endl;
    }
    oss << "</Columns>" << std::endl;
    oss << "</Table>" << std::endl;
  }
  oss << "</Tables>" << std::endl;
  oss << "</Database>" << std::endl;
}

const char * symbole_to_char(aq::symbole sid)
{
  switch (sid)
  {
  case faux: return "faux"; break; 
  case vrai: return "vrai"; break; 
  case vide: return "vide"; break; 
  case unaire: return "unaire"; break; 
  case binaire: return "binaire"; break; 
  case scalaire: return "scalaire"; break; 
  case vecteur: return "vecteur"; break; 
  case liste: return "liste"; break; 
  case r_et: return "r_et"; break; 
  case r_ou: return "r_ou"; break; 
  case r_feuille: return "r_feuille"; break; 
  case r_frere: return "r_frere"; break; 
  case mini_mot: return "mini_mot"; break; 
  case maxi_mot: return "maxi_mot"; break; 
  case liste_mot: return "liste_mot"; break;
  case r_liste: return "r_liste"; break; 
  case inf_egal: return "inf_egal"; break; 
  case egal: return "egal"; break; 
  case sup_egal: return "sup_egal"; break;
  case hp: return "hp"; break;
  case tuples: return "tuples"; break; 
  case r_tag: return "r_tag"; break; 
  case fils_gauche: return "fils_gauche"; break; 
  case fils_droit: return "fils_droit"; break;
  case t_int: return "t_int"; break; 
  case t_double: return "t_double"; break; 
  case t_date1: return "t_date1"; break; 
  case t_date2: return "t_date2"; break; 
  case t_date3: return "t_date3"; break;
  case t_char : return "t_char "; break; 
  case t_long_long: return "t_long_long"; break; 
  case t_raw : return "t_raw "; break;
  case m_up: return "m_up"; break; 
  case m_down: return "m_down"; break;
  case n_contenu: return "n_contenu"; break; 
  case n_table: return "n_table"; break;
  case t_continue : return "t_continue "; break;  
  case t_done : return "t_done "; break; 
  case t_eof: return "t_eof"; break; 
  case t_file_read_error: return "t_file_read_error"; break; 
  case r_jeq: return "r_jeq"; break;
  case r_between: return "r_between"; break; 
  case r_sup: return "r_sup"; break; 
  case r_inf: return "r_inf"; break; 
  case r_leq: return "r_leq"; break; 
  case r_seq: return "r_seq"; break; 
  case r_in: return "r_in"; break; 
  case r_equal : return "r_equal "; break;
  case l_source: return "l_source"; break;  
  case l_pivot: return "l_pivot"; break; 
  case l_cible: return "l_cible"; break;
  case ec_requete: return "ec_requete"; break; 
  case ec_jointure: return "ec_jointure"; break; 
  case ec_etape_1: return "ec_etape_1"; break; 
  case ec_etape_2: return "ec_etape_2"; break; 
  case ec_etape_3 : return "ec_etape_3 "; break;
  case ec_etape_4: return "ec_etape_4"; break; 
  case ec_hp: return "ec_hp"; break; 
  case ec_tuple: return "ec_tuple"; break;
  case c_neutre: return "c_neutre"; break; 
  case c_calcul: return "c_calcul"; break;
  case string: return "string"; break; 
  case integer: return "integer"; break; 
  case d_nulle: return "d_nulle"; break; 
  case comma: return "comma"; break;
  case my_eof: return "my_eof"; break; 
  case une_table: return "une_table"; break; 
  case column: return "column"; break; 
  case copy: return "copy"; break; 
  case vdg: return "vdg"; break; 
  case troncat: return "troncat"; break; 
  case name: return "name"; break;
  case file: return "file"; break; 
  case t_row_id: return "t_row_id"; break; 
  case precision: return "precision"; break; 
  case t_star: return "t_star"; break; 
  case last_symbole: return "last_symbole"; break;
  }
}

}
