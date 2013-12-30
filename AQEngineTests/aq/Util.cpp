#include "Util.h"
#include <aq/FileMapper.h>
#include <aq/Base.h>
#include <aq/Timer.h>
#include <aq/Exceptions.h>
#include <aq/ColumnItem.h>
#include <aq/ColumnMapper.h>
#include <aq/FileMapper.h>
#include <algorithm>
#include <fstream>
#include <boost/tuple/tuple.hpp>
#include <boost/filesystem.hpp>
#include <boost/program_options.hpp>
#include <boost/algorithm/string.hpp>
#include <boost/variant.hpp>

namespace aq
{
  
  // ----------------------------------------------------------------------------
  typedef boost::variant<
    aq::ColumnItem<int32_t>::Ptr, 
    aq::ColumnItem<int64_t>::Ptr, 
    aq::ColumnItem<double>::Ptr, 
    aq::ColumnItem<char*>::Ptr 
  > item_t;
  typedef std::vector<std::pair<item_t, aq::ColumnType> > v_item_t;
  typedef boost::variant<
    aq::ColumnMapper_Intf<int32_t>::Ptr, 
    aq::ColumnMapper_Intf<int64_t>::Ptr, 
    aq::ColumnMapper_Intf<double>::Ptr, 
    aq::ColumnMapper_Intf<char>::Ptr
  > column_mapper_t;

  // ----------------------------------------------------------------------------
  template <typename T>
  bool cmp_item(const typename aq::ColumnItem<T>::Ptr& i1, const typename aq::ColumnItem<T>::Ptr& i2)
  {
    if (aq::ColumnItem<T>::lessThan(*i1, *i2))
      return true;
    else if (!aq::ColumnItem<T>::equal(*i1, *i2))
      return false;
    return false;
  }

  // ----------------------------------------------------------------------------
  struct grp_cmp
  {
    bool operator()(const v_item_t& v1, const v_item_t& v2)
    {
      assert(v1.size() == v2.size());
      for (size_t i = 0; i < v1.size(); ++i)
      {
        assert(v1[i].second == v2[i].second);
        switch (v1[i].second)
        {
        case aq::ColumnType::COL_TYPE_BIG_INT:
        case aq::ColumnType::COL_TYPE_DATE:
          return cmp_item<int64_t>(boost::get<aq::ColumnItem<int64_t>::Ptr>(v1[i].first), boost::get<aq::ColumnItem<int64_t>::Ptr>(v2[i].first));
        break;
        case aq::ColumnType::COL_TYPE_DOUBLE:
          return cmp_item<double>(boost::get<aq::ColumnItem<double>::Ptr>(v1[i].first), boost::get<aq::ColumnItem<double>::Ptr>(v2[i].first));
        break;
        case aq::ColumnType::COL_TYPE_INT:
          return cmp_item<int32_t>(boost::get<aq::ColumnItem<int32_t>::Ptr>(v1[i].first), boost::get<aq::ColumnItem<int32_t>::Ptr>(v2[i].first));
        break;
        case aq::ColumnType::COL_TYPE_VARCHAR:
          return cmp_item<char*>(boost::get<aq::ColumnItem<char*>::Ptr>(v1[i].first), boost::get<aq::ColumnItem<char*>::Ptr>(v2[i].first));
        break;
        }
      }
      return false;
    }
  };

// ----------------------------------------------------------------------------
int generate_database(const char * path, const char * name)
{  
  // generate empty DB
  std::string dbPath_str = std::string(path) + std::string(name) + "/";
  boost::filesystem::path dbPath(dbPath_str);
  if (boost::filesystem::exists(dbPath))
  {
    std::cerr << "database " << name << " already exist" << std::endl;
    return -1;
  }
  boost::filesystem::create_directory(dbPath);

  boost::filesystem::path p(dbPath.string() + "base_struct");
  boost::filesystem::create_directory(p);
  p = boost::filesystem::path(dbPath.string() + "calculus");
  boost::filesystem::create_directory(p);
  p = boost::filesystem::path(dbPath.string() + "data_orga");
  boost::filesystem::create_directory(p);
  p = boost::filesystem::path(dbPath.string() + "data_orga/tmp");
  boost::filesystem::create_directory(p);
  p = boost::filesystem::path(dbPath.string() + "data_orga/tmp");
  boost::filesystem::create_directory(p);
  p = boost::filesystem::path(dbPath.string() + "data_orga/vdg");
  boost::filesystem::create_directory(p);
  
  // generate empty base struct
  std::ofstream baseStruct(std::string(dbPath.string() + "base_struct/base.aqb").c_str());
  baseStruct << "EMPTY_DB" << std::endl;
  baseStruct << "1" << std::endl;
  baseStruct << std::endl;
  baseStruct << "\"table_1\" 1 1 1" << std::endl;
  baseStruct << "\"id\" 1 19 INT" << std::endl;
  baseStruct.close();

  return 0;
}

// ------------------------------------------------------------------------------
int generate_working_directories(const struct opt& o, std::string& iniFilename)
{
  boost::filesystem::path p;
  p = boost::filesystem::path(o.dbPath + "calculus/" + o.queryIdent);
  boost::filesystem::create_directory(p);
  p = boost::filesystem::path(o.workingPath + "data_orga/tmp/" + o.queryIdent);
  if (boost::filesystem::exists(p))
  {
    if (o.force)
    {
      boost::filesystem::remove_all(p);
    }
    else
    {
      throw aq::generic_error(aq::generic_error::INVALID_FILE, std::string(p.string() + " already exist").c_str());
    }
  }
  boost::filesystem::create_directory(p);
  p = boost::filesystem::path(o.workingPath + "data_orga/tmp/" + o.queryIdent + "/dpy");
  boost::filesystem::create_directory(p);
  p = boost::filesystem::path(o.dbPath + "calculus/" + o.queryIdent);
  boost::filesystem::create_directory(p);
  
  iniFilename = o.dbPath + "calculus/" + o.queryIdent + "/aq_engine.ini";
  std::ofstream ini(iniFilename.c_str());
  ini << "export.filename.final=" << o.dbPath << "base_struct/base.aqb" << std::endl;
  ini << "step1.field.separator=;" << std::endl;
  ini << "k_rep_racine=" << o.dbPath << std::endl;
  ini << "k_rep_racine_tmp=" << o.workingPath << std::endl;
  ini.close();

  return 0;
}

// ------------------------------------------------------------------------------
int run_aq_engine(const std::string& aq_engine, const std::string& iniFilename, const std::string& ident)
{
  std::string args = iniFilename + " " + ident + " Dpy";
  int rc = system((aq_engine + " " + args).c_str());
  if (rc != 0)
  {
    aq::Logger::getInstance().log(AQ_ERROR, "error running: '%s %s'\n", aq_engine.c_str(), args.c_str());
  }
  return rc;
}

// ------------------------------------------------------------------------------
int check_answer_validity(const struct opt& o, aq::AQMatrix& matrix, const uint64_t count, const uint64_t nbRows, const uint64_t nbGroups)
{
  int rc = 0;
  try
  {
    std::string answerFile(o.dbPath);
    answerFile += "/data_orga/tmp/" + std::string(o.queryIdent) + "/dpy/";
    std::string iniFilename(o.dbPath);
    iniFilename += "/calculus/" + std::string(o.queryIdent) + "/aq_engine.ini";
    std::vector<long long> tablesIds;
    matrix.load(answerFile.c_str(), tablesIds);
    if (count != 0)
    {
      if (matrix.getTotalCount() != count)
      {
        aq::Logger::getInstance().log(AQ_ERROR, "ERROR: expected %u count, get %u\n", count, matrix.getTotalCount());
        rc = -1;
      }
    }
    if (nbRows != 0)
    {
      if (matrix.getNbRows() != nbRows)
      {
        aq::Logger::getInstance().log(AQ_ERROR, "ERROR: expected %u rows, get %u\n", nbRows, matrix.getNbRows());
        rc = -1;
      }
      else
      {
        aq::Logger::getInstance().log(AQ_INFO, "rows match [%u]\n", matrix.getNbRows());
      }
    }
    if (nbGroups != 0)
    {
      if (matrix.getGroupBy().size() != nbGroups)
      {
        aq::Logger::getInstance().log(AQ_INFO, "ERROR: expected %u groups, get %u\n", nbGroups, matrix.getGroupBy().size());
        rc = -1;
      }
      else
      {
        aq::Logger::getInstance().log(AQ_INFO, "groups match [%u]\n", matrix.getGroupBy().size());
      }
    }
  } 
  catch (const aq::generic_error& ge)
  {
    aq::Logger::getInstance().log(AQ_ERROR, "ERROR: %s", ge.what());
    rc = -1;
  }
  return rc;
}

// ------------------------------------------------------------------------------
void get_columns(std::vector<std::string>& columns, const std::string& query, const std::string& key)
{
  std::string::size_type pos = query.find(key);
  if (pos != std::string::npos)
  {
    // assume Columns are on the same line
    std::string::size_type end = query.find("\n", pos);
    std::string line = query.substr(pos + key.size(), end - pos - key.size());
    // skip ','
    pos = line.find(",");
    while (pos != std::string::npos)
    {
      line = line.substr(pos + 1);
      pos = line.find(",");
    }
    boost::replace_all(line, "\n", "");
    boost::trim(line);
    boost::split(columns, line, boost::is_any_of("."));
    for (auto& c : columns)
    {
      boost::trim(c);
      boost::replace_all(c, " ", ".");
    }
    auto new_end = std::remove(columns.begin(), columns.end(), std::string(""));
    columns.erase(new_end, columns.end());
  }
}


typedef std::vector<bool> selected_t;
typedef std::vector<std::pair<item_t, bool> > grouped_t;
typedef std::vector<std::pair<item_t, bool> > ordered_t;
typedef std::map<size_t, std::map<size_t, std::pair<aq::ColumnType, column_mapper_t> > > column_mappers_t;

template <typename T>
int check_item(column_mapper_t mapper, aq::ColumnType type, size_t pos, std::ostream& os, v_item_t& grp, 
               grouped_t::value_type::first_type& itGrouped,
               ordered_t::value_type::first_type& itOrdered,
               bool display, bool isSelected, bool isGrouped, bool isOrdered, bool new_group, size_t i)
{
  T value;
  typename aq::ColumnItem<T>::Ptr item(new aq::ColumnItem<T>);
  auto m = boost::get<typename aq::ColumnMapper_Intf<T>::Ptr>(mapper);
  m->loadValue(pos - 1, &value);
  item->setValue(value);

  if (display && isSelected)
  {
    if (pos > 0)
    {
      os << item->toString() << " ; ";
    }
    else
    {
      os << "NULL ; ";
    }
  }

  auto& itemGroup = boost::get<aq::ColumnItem<T> >(itGrouped);
  auto& itemOrder = boost::get<aq::ColumnItem<T> >(itOrdered);

  if (isGrouped)
  {
    if (new_group || (i == 0))
    {
      itemGroup = *item;
      grp.push_back(std::make_pair(item_t(item), type));
    }
    else if (!aq::ColumnItem<T>::equal(*item, itemGroup))
    {
      aq::Logger::getInstance().log(AQ_ERROR, "BAD GROUPING: get '%s', expect '%s'\n", item->toString().c_str(), itemGroup.toString().c_str());
      return -1;
    }
  }

  if (isOrdered)
  {
    if (new_group || (i == 0))
    {
      itemOrder = *item;
    }
    else if (!aq::ColumnItem<T>::equal(*item, itemOrder) && !aq::ColumnItem<T>::lessThan(itemOrder, *item.get()))
    {
      aq::Logger::getInstance().log(AQ_ERROR, "BAD ORDERING: get '%s', expect '%s'\n", item->toString().c_str(), itemGroup.toString().c_str());
      return -1;
    }
  }

  return 0;
}

template <typename T>
void register_item(const std::string& vdgPath,
                   const std::string& columnName,
                   size_t tableId,
                   size_t colId,
                   size_t size,
                   size_t packetSize,
                   const std::vector<std::string>& groupedColumns,
                   const std::vector<std::string>& orderedColumns,
                   grouped_t& isGrouped, 
                   ordered_t& isOrdered,
                   column_mapper_t& cm)
{
  typename aq::ColumnMapper_Intf<T>::Ptr m(new aq::ColumnMapper<T, FileMapper>(vdgPath.c_str(), tableId, colId, size, packetSize));
  cm = m;
  bool isGrouping = std::find(groupedColumns.begin(), groupedColumns.end(), std::string(columnName)) != groupedColumns.end();
  isGrouped.push_back(std::make_pair(new aq::ColumnItem<T>, isGrouping));
  bool isOrdering = std::find(orderedColumns.begin(), orderedColumns.end(), std::string(columnName)) != orderedColumns.end();
  isOrdered.push_back(std::make_pair(new aq::ColumnItem<T>, isOrdering));
}

template <> 
inline void register_item<char>(const std::string& vdgPath,
                                const std::string& columnName,
                                size_t tableId,
                                size_t colId,
                                size_t size,
                                size_t packetSize,
                                const std::vector<std::string>& groupedColumns, 
                                const std::vector<std::string>& orderedColumns,
                                grouped_t& isGrouped, 
                                ordered_t& isOrdered,
                                column_mapper_t& cm)
{
  aq::ColumnMapper_Intf<char>::Ptr m(new aq::ColumnMapper<char, FileMapper>(vdgPath.c_str(), tableId, colId, size, packetSize));
  cm = m;
  bool isGrouping = std::find(groupedColumns.begin(), groupedColumns.end(), std::string(columnName)) != groupedColumns.end();
  isGrouped.push_back(std::make_pair(new aq::ColumnItem<char*>, isGrouping));
  bool isOrdering = std::find(orderedColumns.begin(), orderedColumns.end(), std::string(columnName)) != orderedColumns.end();
  isOrdered.push_back(std::make_pair(new aq::ColumnItem<char*>, isOrdering));
}

template <typename T>
std::string get_string_value(const item_t& item)
{
  const auto& i = boost::get<typename aq::ColumnItem<T>::Ptr>(item);
  return i->toString();
}

std::string get_string_value(const item_t& item, aq::ColumnType type)
{
  std::string v;
  switch (type)
  {
  case aq::ColumnType::COL_TYPE_BIG_INT:
  case aq::ColumnType::COL_TYPE_DATE:
    return get_string_value<int64_t>(item);
    break;
  case aq::ColumnType::COL_TYPE_DOUBLE:
    return get_string_value<double>(item);
    break;
  case aq::ColumnType::COL_TYPE_INT:
    return get_string_value<int32_t>(item);
    break;
  case aq::ColumnType::COL_TYPE_VARCHAR:
    return get_string_value<char*>(item);
    break;
  }
  return v;
}

// ------------------------------------------------------------------------------
int check_answer_data(std::ostream& os,
                      const std::string& answerPath,
                      const struct opt& o,
                      const std::vector<std::string>& selectedColumns,
                      const std::vector<std::string>& groupedColumns, 
                      const std::vector<std::string>& orderedColumns
                      // WhereValidator& whereValidator
                      )
{
  std::string baseFilename(o.dbPath);
  baseFilename += "/base_struct/base.aqb";
  std::string vdgPath(o.dbPath);
  vdgPath += "/data_orga/vdg/data/";

  aq::Settings settings;
  aq::Base baseDesc(baseFilename);

  // Set the baseDesc for the Where Conditions
  // whereValidator.setBaseDesc(baseDesc);

  aq::AQMatrix matrix(settings, baseDesc);
  
  std::vector<long long> tableIDs;
  matrix.load(answerPath.c_str(), tableIDs);

  const aq::AQMatrix::matrix_t& m = matrix.getMatrix();

  // check size, print column name and prepare column mapping
  size_t size = 0;
  selected_t isSelected;
  grouped_t isGrouped;
  ordered_t isOrdered;
  column_mappers_t columnMappers;
  for (auto it = m.begin(); it != m.end(); ++it)
  {
    if (size == 0)
    {
      size = (*it).indexes.size();
    }
    else if (size != (*it).indexes.size())
    {
      std::cerr << "FATAL ERROR: indexes size of table differs" << std::endl;
      exit(-1);
    }
    
    std::map<size_t, std::pair<aq::ColumnType, column_mapper_t> > tableColumnMappers;
    const aq::AQMatrix::matrix_t::value_type t = *it;
    aq::Table::Ptr table = baseDesc.getTable(t.table_id);
    for (auto itCol = table->Columns.begin(); itCol != table->Columns.end(); ++itCol)
    {
      column_mapper_t cm;
      std::string columnName = table->getName() + "." + (*itCol)->getName();
      switch((*itCol)->Type)
      {
      case aq::ColumnType::COL_TYPE_INT:
        register_item<int32_t>(vdgPath, columnName, t.table_id, (*itCol)->ID, 1, o.packetSize, groupedColumns, orderedColumns, isGrouped, isOrdered, cm);
        break;
      case aq::ColumnType::COL_TYPE_BIG_INT:
      case aq::ColumnType::COL_TYPE_DATE:
        register_item<int64_t>(vdgPath, columnName, t.table_id, (*itCol)->ID, 1, o.packetSize, groupedColumns, orderedColumns, isGrouped, isOrdered, cm);
        break;
      case aq::ColumnType::COL_TYPE_DOUBLE:
        register_item<double>(vdgPath, columnName, t.table_id, (*itCol)->ID, 1, o.packetSize, groupedColumns, orderedColumns, isGrouped, isOrdered, cm);
        break;
      case aq::ColumnType::COL_TYPE_VARCHAR:
        register_item<char>(vdgPath, columnName, t.table_id, (*itCol)->ID, 1, o.packetSize, groupedColumns, orderedColumns, isGrouped, isOrdered, cm);
        break;
      }
      tableColumnMappers[(*itCol)->ID] = std::make_pair((*itCol)->Type, cm);
      bool isSelecting = std::find(selectedColumns.begin(), selectedColumns.end(), std::string(table->getName() + "." + (*itCol)->getName())) != selectedColumns.end();
      isSelected.push_back(isSelecting);

      if (o.display && isSelecting)
        os << table->getName() << "." << (*itCol)->getName() << " ; ";
    }
    
    columnMappers.insert(std::make_pair(t.table_id, tableColumnMappers));
  }
  if (o.display)
  {
    if (o.withCount)
      os << "COUNT" << std::endl;
    else
      os << std::endl;
  }

  // print data and check group
  size_t groupIndex = 0;
  std::set<v_item_t, grp_cmp> groups;
  std::vector<size_t> groupCount(matrix.getGroupBy().size(), 0);
  for (size_t i = 0; i < size && ((o.limit == 0) || (i < o.limit)); ++i)
  {
    v_item_t grp;
    bool new_group = false;
    assert(groupCount[groupIndex] <= matrix.getGroupBy()[groupIndex].first);
    if (groupCount[groupIndex] == matrix.getGroupBy()[groupIndex].first)
    {
      new_group = true;
      groupIndex++;
    }
    assert(groupIndex < groupCount.size());
    groupCount[groupIndex]++;
    auto itSelected = isSelected.begin();
    auto itGrouped = isGrouped.begin();
    auto itOrdered = isOrdered.begin();
    for (auto& t : m)
    {
      if (o.display && o.withIndex)
        os << t.table_id << "[" << t.indexes[i] << "] => ";
      for (auto& cm : columnMappers[t.table_id])
      {
        auto& type = cm.second.first;
        auto& mapper = cm.second.second;

        switch (type)
        {
        case aq::ColumnType::COL_TYPE_BIG_INT:
        case aq::ColumnType::COL_TYPE_DATE:
          check_item<int64_t>(mapper, type, t.indexes[i], os, grp, itGrouped->first, itOrdered->first, o.display, *itSelected, itGrouped->second, itOrdered->second, new_group, i);
          break;
        case aq::ColumnType::COL_TYPE_DOUBLE:
          check_item<double>(mapper, type, t.indexes[i], os, grp, itGrouped->first, itOrdered->first, o.display, *itSelected, itGrouped->second, itOrdered->second, new_group, i);
          break;
        case aq::ColumnType::COL_TYPE_INT:
          check_item<int32_t>(mapper, type, t.indexes[i], os, grp, itGrouped->first, itOrdered->first, o.display, *itSelected, itGrouped->second, itOrdered->second, new_group, i);
          break;
        case aq::ColumnType::COL_TYPE_VARCHAR:
          check_item<char*>(mapper, type, t.indexes[i], os, grp, itGrouped->first, itOrdered->first, o.display, *itSelected, itGrouped->second, itOrdered->second, new_group, i);
          break;
        }

        ++itSelected;
        ++itGrouped;
        ++itOrdered;
      }
    }

    if (o.display && o.withCount)
      os << matrix.getCount()[i];

    if (o.display)
      os << std::endl;

    if (!grp.empty())
    {
      if (groups.size() == 0 || groups.find(grp) == groups.end())
        groups.insert(grp);
      else
      {
        std::string group;
        group = "[ ";
        for (auto& g : grp)
        {
          group += get_string_value(g.first, g.second);
          group += " ";
        }
        group += "]" ;
        aq::Logger::getInstance().log(AQ_ERROR, "BAD GROUPING: group %s already insert\n", group.c_str());
        return -1;
      }
    }

    //if (whereValidator.check(matrix, columnMappers, i) == false)
    //{
    //  aq::Logger::getInstance().log(AQ_ERROR, "WHERE VALIDATOR FAILED\n");
    //  return -1;
    //}

  }

  return 0;
}

template <typename T>
void push_to_cb(char * buf, display_cb * cb, size_t index, column_mapper_t& cm)
{
  typename aq::ColumnItem<T>::Ptr item(new aq::ColumnItem<T>);
  auto m = boost::get<typename aq::ColumnMapper_Intf<T>::Ptr>(cm);
  T item_value;
  m->loadValue(index - 1, &item_value);
  item->setValue(item_value);
  item->toString(buf);
  cb->push(buf);
}

template <> inline
void push_to_cb<char*>(char * buf, display_cb * cb, size_t index, column_mapper_t& cm)
{
  aq::ColumnItem<char*>::Ptr item(new aq::ColumnItem<char*>);
  auto m = boost::get<aq::ColumnMapper_Intf<char>::Ptr>(cm);
  char * item_value = new char[128];
  m->loadValue(index - 1, item_value);
  item->setValue(item_value);
  item->toString(buf);
  cb->push(buf);
}

// ------------------------------------------------------------------------------
class print_data
{
public:
  typedef std::vector<boost::tuple<size_t, aq::ColumnType, column_mapper_t> > display_order_t;

public:
  print_data(
    const struct opt& _o,
    display_cb * _cb, 
    display_order_t& _display_order) 
    : o(_o), cb(_cb), display_order(_display_order)
  {
  }
  void handle(std::vector<size_t>& rows)
  {
    cb->next();

    if (o.withIndex)
    {
      for (size_t i = 0; i < rows.size() - 1; i++)
      {
        ss.str("");
        ss << rows[i];
        cb->push(ss.str());
      }
    }

    size_t i = o.withCount ? 1 : *(rows.rbegin());
    do
    {
      for (auto& c : display_order)
      {
        auto& tindex = boost::get<0>(c);
        auto& type = boost::get<1>(c);
        auto& cm = boost::get<2>(c);
        auto index = rows[tindex];
        if (index == 0)
        {
          cb->push("NULL");
        }
        else
        {
          switch (type)
          {
          case aq::ColumnType::COL_TYPE_BIG_INT:
          case aq::ColumnType::COL_TYPE_DATE:
            push_to_cb<int64_t>(buf, cb, index, cm);
            break;
          case aq::ColumnType::COL_TYPE_DOUBLE:
            push_to_cb<double>(buf, cb, index, cm);
            break;
          case aq::ColumnType::COL_TYPE_INT:
            push_to_cb<int32_t>(buf, cb, index, cm);
            break;
          case aq::ColumnType::COL_TYPE_VARCHAR:
            push_to_cb<char*>(buf, cb, index, cm);
            break;
          }
        }
      }
    } while (--i > 0);

    if (o.withCount)
    {
      ss.str("");
      ss << *(rows.rbegin());
      cb->push(ss.str());
    }

  }
private:
  char buf[128];
  std::stringstream ss;
  const struct opt& o;
  display_cb * cb;
  std::vector<boost::tuple<size_t, aq::ColumnType, column_mapper_t> >& display_order;
};

// ------------------------------------------------------------------------------
int display(display_cb * cb,
            // const std::string& answerPath,
            aq::AQMatrix& aqMatrix,
            const aq::Base& baseDesc,
            const aq::Settings& settings,
            const struct opt& o,
            const std::vector<std::string>& selectedColumns)
{
  std::stringstream ss;
  const auto& matrix = aqMatrix.getMatrix();

  // check size, print column name and prepare column mapping
  size_t size = 0;
  std::vector<boost::tuple<size_t, aq::ColumnType, column_mapper_t> > display_order(selectedColumns.size());
  for (size_t tindex = 0; tindex < matrix.size(); tindex++)
  {
    auto& t = matrix[tindex];
    if (size == 0)
    {
      size = t.indexes.size();
    }
    else if (size != t.indexes.size())
    {
      std::cerr << "FATAL ERROR: indexes size of table differs" << std::endl;
      exit(-1);
    }
    
    aq::Table::Ptr table = baseDesc.getTable(t.table_id);
    for (auto& col : table->Columns)
    {
      auto it = std::find(selectedColumns.begin(), selectedColumns.end(), std::string(table->getName() + "." + col->getName()));
      if (it != selectedColumns.end())
      {
        column_mapper_t cm;
        switch(col->Type)
        {
        case aq::ColumnType::COL_TYPE_INT:
          {
            aq::ColumnMapper_Intf<int32_t>::Ptr m(new aq::ColumnMapper<int32_t, FileMapper>(settings.dataPath.c_str(), t.table_id, col->ID, 1/*(*itCol)->Size*/, o.packetSize));
            cm = m;
          }
          break;
        case aq::ColumnType::COL_TYPE_BIG_INT:
        case aq::ColumnType::COL_TYPE_DATE:
          {
            aq::ColumnMapper_Intf<int64_t>::Ptr m(new aq::ColumnMapper<int64_t, FileMapper>(settings.dataPath.c_str(), t.table_id, col->ID, 1/*(*itCol)->Size*/, o.packetSize));
            cm = m;
          }
          break;
        case aq::ColumnType::COL_TYPE_DOUBLE:
          {
            aq::ColumnMapper_Intf<double>::Ptr m(new aq::ColumnMapper<double, FileMapper>(settings.dataPath.c_str(), t.table_id, col->ID, 1/*(*itCol)->Size*/, o.packetSize));
            cm = m;
          }
          break;
        case aq::ColumnType::COL_TYPE_VARCHAR:
          {
            aq::ColumnMapper_Intf<char>::Ptr m(new aq::ColumnMapper<char, FileMapper>(settings.dataPath.c_str(), t.table_id, col->ID, col->Size, o.packetSize));
            cm = m;
          }
          break;
        }
        display_order[std::distance(selectedColumns.begin(), it)] = boost::make_tuple(tindex, col->Type, cm);
      }
    }
  }

  if (o.withIndex)
  {
    for (auto& t : matrix)
    {
      ss.str("");
      ss << "TABLE " << t.table_id; // FIXME : put table's name
      cb->push(ss.str());
    }
  }

  for (auto& sc : selectedColumns)
    cb->push(sc);
  if (o.withCount)
    cb->push("COUNT");
  
  // print data
  if (size == 0) // FIXME : size can be 0 if there is no result
  {
    print_data pd_cb(o, cb, display_order);
    aqMatrix.readData<print_data>(pd_cb);
    cb->next();
  }
  else
  {
    char buf[128];
    for (size_t i = 0; i < size && ((o.limit == 0) || (i < o.limit)); ++i)
    {

      cb->next();

      if (o.withIndex)
      {
        for (auto& t : matrix)
        {
          ss.str("");
          ss << t.indexes[i];
          cb->push(ss.str());
        }
      }

      for (auto& c : display_order)
      {
        auto& tindex = boost::get<0>(c);
        auto& type = boost::get<1>(c);
        auto& cm = boost::get<2>(c);
        auto index = matrix[tindex].indexes[i];
        if (index == 0)
        {
          cb->push("NULL");
        }
        else
        {
          switch (type)
          {
          case aq::ColumnType::COL_TYPE_BIG_INT:
          case aq::ColumnType::COL_TYPE_DATE:
            push_to_cb<int64_t>(buf, cb, index, cm);
            break;
          case aq::ColumnType::COL_TYPE_DOUBLE:
            push_to_cb<double>(buf, cb, index, cm);
            break;
          case aq::ColumnType::COL_TYPE_INT:
            push_to_cb<int32_t>(buf, cb, index, cm);
            break;
          case aq::ColumnType::COL_TYPE_VARCHAR:
            push_to_cb<char*>(buf, cb, index, cm);
            break;
          }
        }
      }

      if (o.withCount)
      {
        ss.str("");
        ss << aqMatrix.getCount()[i];
        cb->push(ss.str());
      }
    }
    
    // cb->next();
  }
  return 0;
}

}
