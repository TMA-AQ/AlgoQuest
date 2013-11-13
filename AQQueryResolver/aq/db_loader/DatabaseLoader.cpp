#include "DatabaseLoader.h"

#include <aq/Utilities.h>
#include <aq/Logger.h>
#include <aq/Exceptions.h>

#include <cstring>
#include <cstdlib>
#include <cerrno>
#include <ctime>
#include <algorithm>
#include <string>

#if defined(WIN32)
# include <windows.h>
# define atoll _atoi64
#elif defined(__FreeBSD__)
# define atoll atol
#endif

#include <boost/lexical_cast.hpp>
#include <boost/algorithm/string/trim.hpp>
#include <boost/thread.hpp>
#include <boost/bind.hpp>
#include <boost/type_traits.hpp>

#define k_record_size_max    8192 ///< max record's size in a file
#define k_field_size_max     4096 ///< max field's size in a file  
#define k_file_name_size_max 512  ///< max filename's size

namespace aq
{

  namespace helper
  {
    
    template <typename T>
    struct pointer_type_handler
    {
      typedef T type;
      static T cast(const char * field)
      {
        return (T)field;
      }
      static size_t write(T& value, size_t size, FILE * fd)
      {
        return fwrite(value, sizeof(boost::remove_pointer<T>::type), size, fd);
      }
    };
    
    template <typename T>
    struct raw_type_handler
    {
      typedef T type;
      static T cast(const char * field)
      {
        return boost::lexical_cast<T>(field);
      }
      static size_t write(T& value, size_t size, FILE * fd)
      {
        return fwrite(&value, sizeof(T), size, fd);
      }
    };

    template <typename T>
    void write_record(const char * field, size_t size, aq::column_info_t& ci)
    {
      typedef typename pointer_type_handler<T> ptr_handler;
      typedef typename typename raw_type_handler<T> raw_handler;
      typedef typename boost::mpl::if_<boost::is_pointer<T>, ptr_handler, raw_handler>::type handler_type_t;
      typename handler_type_t::type value;
      
      if (strcmp(field, "NULL") == 0)
      {
        value = 0; // FIXME : handle NULL properly
      }
      else
      {
        value = handler_type_t::cast(field);
      }
      if (ci.fd)
      {
        handler_type_t::write(value, size, ci.fd);
      }
      else if (ci.thesaurus && ci.prm)
      {
        auto it = ci.thesaurus->insert(new T(value));
        uint32_t pos = static_cast<uint32_t>(std::distance(ci.thesaurus->begin(), it.first));
        if (it.second)
        {
          for (auto& p : *ci.prm)
          {
            if (p >= pos)
            {
              p += 1;
            }
          }
        }
        ci.prm->push_back(pos);
      }
    }

  }

// ---------------------------------------------------------------------------------------------
DatabaseLoader::DatabaseLoader(const aq::base_t bd, const std::string& _path, const size_t _packet_size, const char _end_of_field_c, bool _csv_format)
  : 
  my_base(bd),
  k_rep_racine(_path),
  k_batch_loader(""),
  k_packet_size(_packet_size),
  k_double_separator(','),
  end_of_field_c(_end_of_field_c),
  csv_format(_csv_format),
  format_file_name("%sB%03dT%04dC%04dP%012d")
{
  base_desc_file = this->k_rep_racine + "base_desc/base.aqb";
	rep_source = k_rep_racine + "data_orga/tables/";
	rep_cible = k_rep_racine + "data_orga/vdg/data/";
  ini_filename = this->k_rep_racine + "loader.ini";
}

// ---------------------------------------------------------------------------------------------
DatabaseLoader::DatabaseLoader(const aq::base_t bd, const std::string& _loader, const std::string& _path, 
                               const size_t _packet_size, const char _end_of_field_c, bool _csv_format)
  : 
  my_base(bd),
  k_rep_racine(_path),
  k_batch_loader(_loader),
  k_packet_size(_packet_size),
  k_double_separator(','),
  end_of_field_c(_end_of_field_c),
  csv_format(_csv_format),
  format_file_name("%sB%03dT%04dC%04dP%012d")
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
	  throw aq::generic_error(aq::generic_error::INVALID_FILE, "cannot create file %s\n", ini_filename.c_str());
	}

	fwrite("export.filename.final=", 1, 22, fini);
	fwrite(k_rep_racine.c_str(), 1, k_rep_racine.size(), fini);
	fwrite("base_struct/base.aqb", 1, 20, fini);
	fwrite("\n", 1, 1, fini);
	fwrite("step1.field.separator=", 1, 22, fini);
	fwrite(&end_of_field_c, 1, 1, fini);
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
void DatabaseLoader::load() 
{	
	time_t t = time (NULL);
	struct tm * tb = localtime(&t);
	aq::Logger::getInstance().log(AQ_INFO, "Start loading database [%s] on  %d/%02d/%02d H %02d:%02d:%02d\n",
    this->my_base.name.c_str(),
    tb->tm_year +1900, tb->tm_mon +1 ,tb->tm_mday, tb->tm_hour, tb->tm_min, tb->tm_sec);
  
  boost::thread_group grp;
	for (auto& table : my_base.table)
	{
    std::string filename = table.name;
    boost::trim_if(filename, boost::is_any_of(" \""));
    filename = rep_source + filename + ".txt";
    grp.create_thread(boost::bind(&DatabaseLoader::loadTable, this, boost::cref(table), filename));
	}
  grp.join_all();

	// end time
	t = time ( NULL );
	tb = localtime( &t );
	aq::Logger::getInstance().log(AQ_INFO, "End on %d/%02d/%02d H %02d:%02d:%02d\n", 
    tb->tm_year +1900, tb->tm_mon +1 ,tb->tm_mday, tb->tm_hour, tb->tm_min, tb->tm_sec);
}

//-------------------------------------------------------------------------------
void DatabaseLoader::load(const size_t table_id) 
{
	for (auto& table : my_base.table)
	{
    if (table.id != table_id)
      continue;
    
    std::string filename = table.name;
    boost::trim_if(filename, boost::is_any_of(" \""));
    filename = rep_source + filename + ".txt";
    this->loadTable(table, filename);
	}
}

// --------------------------------------------------------------------------------------------
void DatabaseLoader::loadTable(const aq::base_t::table_t& table, const std::string& filename) const
{
  aq::Logger::getInstance().log(AQ_INFO, "Table : %u\n", table.name.c_str());
  
	FILE * fd_table;
  FileCloser fcloser(fd_table);

	int total_nb_enreg =0;
	char my_record[k_record_size_max]; 
	char my_col[k_file_name_size_max];

	// initialisation des composants de nom de fichiers
	int n_base = 1;
	int n_paquet = 0;

  // open source file and check if opened
  if( (fd_table = aq::fopenUTF8(filename.c_str(), "r")) == NULL )
  {
    throw aq::generic_error(aq::generic_error::COULD_NOT_OPEN_FILE, "error opening file %s\n", filename.c_str());
  }

  // prepare columns infos
  std::vector<struct aq::column_info_t> columns_infos;
  for (auto& col : table.colonne)
  {
    struct aq::column_info_t infos = { col, "", NULL, NULL, NULL };
    columns_infos.push_back(infos);
  }

  // import records
  int write_n_enreg = 0;
  while (fgets(my_record, aq::packet_size, fd_table)) // read record
  {
    if (feof(fd_table))  
      break;

    if (ferror(fd_table))
    {
      throw aq::generic_error(aq::generic_error::INVALID_FILE, "error reading record  %d, loading aborted\n", total_nb_enreg);
    }

    if ((write_n_enreg % aq::packet_size) == 0)
    {
      // next packet
      for (auto& ci : columns_infos)
      {    
        if (k_batch_loader == "")
        {
          sprintf(my_col, format_file_name.c_str(), rep_cible.c_str(), n_base, table.id, ci.col.id, n_paquet);
          ci.filename = my_col;
          if ((ci.fd = fopen (my_col ,"w+b")) == NULL)
          {
            throw aq::generic_error(aq::generic_error::COULD_NOT_OPEN_FILE, "error opening file %s\n", my_col);
          }
        }
        else
        {
          ci.prm = new prm_t();
          ci.thesaurus = new thesaurus_t(item_cmp_t(ci.col.getSize()));
        }
      }
      n_paquet++;
    }

    write_n_enreg++;
    total_nb_enreg++;

    if (write_n_enreg > aq::packet_size)
    {
      // write packet
      for (auto& ci : columns_infos)
      {
        this->runLoader(table.id, ci, n_paquet);
      }
      write_n_enreg = 1; 
    }

    this->writeRecord(columns_infos, my_record);

  }

  // write last packet
  for (auto& ci : columns_infos)
  {
    this->runLoader(table.id, ci, n_paquet);
  }

  aq::Logger::getInstance().log(AQ_INFO, "%u rows recorded in table [%s]\n", total_nb_enreg, filename.c_str());
}

// --------------------------------------------------------------------------------------------
void DatabaseLoader::writeRecord(std::vector<struct aq::column_info_t>& columns_infos, const char * record) const
{
  
	size_t len_rec = strlen(record);
	int debut_lecture = 0;
	int cur_col = 0;
  bool end_of_field = false;
  
	int indice_car;
	char field[k_field_size_max]; 

  // for each field in record
  size_t i = 0;
  while (i < len_rec)
  {
    if (csv_format && (record[i] == '"'))
    {
      i++;
    }

    indice_car = 0 ;
    end_of_field = false;
    while (!end_of_field) 
    {
      const char& c = record[i];

      if (csv_format)
      {
        if ((c == '"') && (record[i - 1] != '\\'))
        {
          i++;
          end_of_field = true;
        }
      }
      else if (endOfField(c))
      {
        end_of_field = true;
      }

      if (end_of_field)
      {
        if (indice_car == 0)  
          strcpy(field, "NULL");
        else
          field[indice_car] = '\0';
        
        auto& ci = columns_infos[cur_col];
        this->FileWriteEnreg(ci, field);
        cur_col++;
      }
      else
      {
        field[indice_car] = c;
        indice_car++;
      }

      i++;
    }
  }
}

// --------------------------------------------------------------------------------------------
void DatabaseLoader::buildPrmThesaurus(const aq::column_info_t& ci, size_t table_id, size_t packet) const
{
  // prm
  std::string prmFilename = aq::getPrmFileName(this->rep_cible.c_str(), table_id, ci.col.id, packet);
  FILE * prm = fopen(prmFilename.c_str(), "w");
  if (prm == NULL)
  {
    throw aq::generic_error(aq::generic_error::COULD_NOT_OPEN_FILE, "can't open file [%s]", prmFilename.c_str());
  }
  for (auto& p : *ci.prm)
  {
    fwrite(&p, sizeof(p), 1, prm);
  }
  fclose(prm);
  
  // thesaurus
  std::string theFilename = aq::getThesaurusFileName(this->rep_cible.c_str(), table_id, ci.col.id, packet);
  FILE * the = fopen(theFilename.c_str(), "w");
  if (the == NULL)
  {
    throw aq::generic_error(aq::generic_error::COULD_NOT_OPEN_FILE, "can't open file [%s]", theFilename.c_str());
  }
  for (auto& t : *ci.thesaurus)
  {
    fwrite(t, 1, ci.col.getSize(), the);
  }
  fclose(the);
}

// --------------------------------------------------------------------------------------------
void DatabaseLoader::runLoader(size_t table, column_info_t& ci, size_t packet) const
{
  if (k_batch_loader == "")
  {
    this->buildPrmThesaurus(ci, table, packet);
  }
  else
  {
    fflush(ci.fd);
    fclose(ci.fd);
    int rc;
    char exec_cmd[1024];
    sprintf(exec_cmd, "%s %s %d %d %d", k_batch_loader.c_str(), ini_filename.c_str(), table , ci.col.id, packet);
    aq::Logger::getInstance().log(AQ_INFO, exec_cmd);
    rc = system(exec_cmd);
    if (rc != 0)
    {
      aq::Logger::getInstance().log(AQ_ERROR, "loader error [%s] return exit code [%d]\n", exec_cmd, rc);
    }
  }
}

//-------------------------------------------------------------------------------
void DatabaseLoader::FileWriteEnreg(aq::column_info_t& ci, char * field) const
{
	switch (ci.col.type)
	{
	case aq::t_int:
    helper::write_record<int>(field, ci.col.size, ci);
		break;

	case aq::t_long_long:
    helper::write_record<long long>(field, ci.col.size, ci);
		break;

	case aq::t_double:
    ChangeCommaToDot(field);
    helper::write_record<double>(field, ci.col.size, ci);
		break;

	case aq::t_date1:
	case aq::t_date2:
  case aq::t_date3:
    if ((strcmp(field, "NULL") != 0) || (strcmp(field, "") != 0))  
    {
      dateConverter.dateToBigInt(field);
    }
    helper::write_record<long long>(field, 1, ci);
    break;

	case aq::t_char:
		if ((int)strlen(field) >= ci.col.size) 
    {
      aq::Logger::getInstance().log(AQ_WARNING, "column size is too small\n");
      field[ci.col.size] = 0 ;
    }
    aq::cleanSpaceAtEnd(field);
    helper::write_record<char*>(field, strlen(field) + 1, ci);
		break;

	default:
    throw aq::generic_error(aq::generic_error::TYPE_MISMATCH, "type de colonne non traite [%s]\n", aq::symbole_to_char(ci.col.type));
		break;
	}
}

//-------------------------------------------------------------------------------
bool DatabaseLoader::endOfField(unsigned char c) const
{
  return (c == end_of_field_c) || (c == '\t') || (c == '\n') || (c == '\r');
}

}
