#ifndef __DATABASE_LOADER_H__
#define __DATABASE_LOADER_H__

#include <cstddef>
#include <string>
#include <set>
#include <vector>
#include <aq/BaseDesc.h>
#include <aq/DateConversion.h>

namespace aq
{
  
class item_cmp_t
{
public:
  item_cmp_t(size_t _size) : size(_size) {}
  bool operator()(const void * buf1, const void * buf2) const
  {
    return ::memcmp(buf1, buf2, size) < 0;
  }
private:
  size_t size;
};

typedef std::set<void*, item_cmp_t> thesaurus_t;
typedef std::vector<uint32_t>       prm_t;

// ---------------------------------------------------------------------------------------------
struct column_info_t
{
  aq::base_t::table_t::col_t   col;
  std::string                  filename;
  FILE                       * fd;
  prm_t                      * prm;
  thesaurus_t                * thesaurus;
};

// ---------------------------------------------------------------------------------------------
class DatabaseLoader
{
public:
  DatabaseLoader(const aq::base_t bd, const std::string& _path, const size_t _packet_size, const char _end_of_field_c, bool _csv_format);
  DatabaseLoader(const aq::base_t bd, const std::string& _loader, const std::string& _path, const size_t _packet_size, const char _end_of_field, bool _csv_format);
  void generate_ini();
  void load(); ///< load from table_name.txt file (can be a csv file)
  void load(const size_t table_id); ///< load from table_name.txt file (can be a csv file)
  void loadAllColumns() const; ///< load from column prepared files
  void loadColumn(const size_t table_id, const size_t column_id) const; ///< load from column prepared file

protected:
  void buildPrmThesaurus(const column_info_t& ci, size_t table_id, size_t packet) const;
  void runLoader(size_t table, column_info_t& ci, size_t packet) const;
  void loadTable(const aq::base_t::table_t& table, const std::string& filename) const;
  void writeRecord(std::vector<aq::column_info_t>& columns_infos, const char * record) const;
  void FileWriteEnreg(aq::column_info_t& ci, char *my_field) const;
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
  size_t packet_size;
  unsigned char end_of_field_c;   
  bool csv_format;

};

}

#endif
