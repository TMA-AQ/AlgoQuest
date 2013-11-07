#ifndef __DATABASE_LOADER_H__
#define __DATABASE_LOADER_H__

#include <cstddef>
#include <string>
#include <aq/BaseDesc.h>
#include <aq/DateConversion.h>

namespace aq
{
  
// ---------------------------------------------------------------------------------------------
struct column_info_t
{
  const aq::base_t::table_t::col_t col;
  std::string filename;
  FILE * fd;
};

// ---------------------------------------------------------------------------------------------
class DatabaseLoader
{
public:
  DatabaseLoader(const aq::base_t bd, const std::string& _path, const size_t _packet_size, const char _end_of_field_c, bool _csv_format);
  DatabaseLoader(const aq::base_t bd, const std::string& _loader, const std::string& _path, const size_t _packet_size, const char _end_of_field, bool _csv_format);
  void generate_ini();
  void load();
  void load(const size_t table_id);

protected:
  void runLoader(size_t table, size_t column, size_t packet) const;
  void loadTable(const aq::base_t::table_t& table, const std::string& filename) const;
  void writeRecord(std::vector<aq::column_info_t>& columns_infos, const char * record) const;
  void FileWriteEnreg(aq::symbole col_type, int col_size, char *my_field, FILE *fcol) const;
  bool endOfField(unsigned char c) const;

private:
	const aq::base_t my_base;
  mutable aq::DateConversion dateConverter;
  std::string k_rep_racine;
  std::string k_rep_param;
  std::string k_batch_loader;
  std::string ini_filename;
  std::string rep_source;
  std::string rep_cible;
  std::string base_desc_file;
  std::string format_file_name;
  size_t k_packet_size;
  char k_double_separator;
  unsigned char end_of_field_c;   
  bool csv_format;

};

}

#endif
