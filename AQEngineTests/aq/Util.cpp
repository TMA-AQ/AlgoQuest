#include "Util.h"
#include <aq/FileMapper.h>
#include <aq/Base.h>
#include <aq/Timer.h>
#include <aq/Exceptions.h>
#include <iostream>
#include <algorithm>
#include <fstream>
#include <boost/filesystem.hpp>
#include <boost/program_options.hpp>
#include <boost/algorithm/string.hpp>

#if defined(WIN32)
# include <aq/WIN32FileMapper.h>
typedef aq::WIN32FileMapper FileMapper;
#else
# include <aq/FileMapper.h>
typedef aq::FileMapper FileMapper;
#endif

namespace aq
{

  typedef std::vector<std::pair<aq::ColumnItem::Ptr, aq::ColumnType> > v_item_t;
  struct grp_cmp
  {
    bool operator()(const v_item_t& v1, const v_item_t& v2)
    {
      assert(v1.size() == v2.size());
      for (size_t i = 0; i < v1.size(); ++i)
      {
        assert(v1[i].second == v2[i].second);
        if (aq::ColumnItem::lessThan(v1[i].first.get(), v2[i].first.get(), v1[i].second))
          return true;
        else if (!aq::ColumnItem::equal(v1[i].first.get(), v2[i].first.get(), v1[i].second))
          return false;
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
  std::ofstream baseStruct(std::string(dbPath.string() + "base_struct/base.").c_str());
  baseStruct << "EMPTY_DB" << std::endl;
  baseStruct << "1" << std::endl;
  baseStruct << std::endl;
  baseStruct << "\"table_1\" 1 1 1" << std::endl;
  baseStruct << "\"id\" 1 19 INT" << std::endl;
  baseStruct.close();

  return 0;
}

// ------------------------------------------------------------------------------
int generate_working_directories(const std::string& dbPath, std::string& queryIdent, std::string& iniFilename)
{
  boost::filesystem::path p;
  p = boost::filesystem::path(dbPath + "calculus/" + queryIdent);
  boost::filesystem::create_directory(p);
  p = boost::filesystem::path(dbPath + "data_orga/tmp/" + queryIdent);
  std::string tmp = dbPath + "data_orga/tmp/";
  std::string tmp2 = queryIdent;
  if (boost::filesystem::exists(p))
  {
    std::cerr << p << " already exist : STOP" << std::endl;
    exit(-1);
  }
  boost::filesystem::create_directory(p);
  p = boost::filesystem::path(dbPath + "data_orga/tmp/" + queryIdent + "/dpy");
  boost::filesystem::create_directory(p);
  p = boost::filesystem::path(dbPath + "calculus/" + queryIdent);
  boost::filesystem::create_directory(p);
  
  iniFilename = dbPath + "calculus/" + queryIdent + "/aq_engine.ini";
  std::ofstream ini(iniFilename.c_str());
  ini << "export.filename.final=" << dbPath << "base_struct/base." << std::endl;
  ini << "step1.field.separator=;" << std::endl;
  ini << "k_rep_racine=" << dbPath << std::endl;
  ini << "k_rep_racine_tmp=" << dbPath << std::endl;
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
    std::cerr << "error running: '" << aq_engine << " " << args << "'" << std::endl;
  }
  return rc;
}

// ------------------------------------------------------------------------------
int check_answer_validity(const char * dbPath, const char * queryIdent, aq::AQMatrix& matrix, const uint64_t nbRows, const uint64_t nbGroups)
{
  int rc = 0;
  try
  {
    std::string answerFile(dbPath);
    answerFile += "/data_orga/tmp/" + std::string(queryIdent) + "/dpy/";
    std::string iniFilename(dbPath);
    iniFilename += "/calculus/" + std::string(queryIdent) + "/aq_engine.ini";
    std::vector<long long> tablesIds;
    matrix.load(answerFile.c_str(), tablesIds);
    if (nbRows != 0)
    {
      if (matrix.getNbRows() != nbRows)
      {
        std::cerr << "ERROR: expected " << nbRows << " rows, get " << matrix.getNbRows() << std::endl;
        rc = -1;
      }
      else
      {
        std::cout << "\t" << "rows match [" << matrix.getNbRows() << "]" << std::endl;
      }
    }
    if (nbGroups != 0)
    {
      if (matrix.getGroupBy().size() != nbGroups)
      {
        std::cerr << "ERROR: expected " << nbGroups << " groups, get " << matrix.getGroupBy().size() << std::endl;
        rc = -1;
      }
      else
      {
        std::cout << "\t" << "groups match [" << matrix.getGroupBy().size() << "]" << std::endl;
      }
    }
  } 
  catch (const aq::generic_error& ge)
  {
    std::cerr << "ERROR: " << ge.what() << std::endl;
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

int check_answer_data(const std::string& answerPath, const std::string& dbPath, const size_t limit, const size_t packetSize, 
                      const std::vector<std::string>& selectedColumns,
                      const std::vector<std::string>& groupedColumns, 
                      const std::vector<std::string>& orderedColumns,
                      WhereValidator& whereValidator)
{
  std::string baseFilename(dbPath);
  baseFilename += "/base_struct/base";
  std::string vdgPath(dbPath);
  vdgPath += "/data_orga/vdg/data/";

  aq::TProjectSettings settings;
  aq::Base baseDesc;
  baseDesc.loadFromRawFile(baseFilename.c_str());

  // Set the baseDesc for the Where Conditions
  whereValidator.setBaseDesc(baseDesc);


  aq::AQMatrix matrix(settings, baseDesc);
  
  std::vector<long long> tableIDs;
  matrix.load(answerPath.c_str(), tableIDs);

  std::cout << "AQMatrix: " << std::endl;
  std::cout << "\t" << tableIDs.size() << " tables: [ ";
  std::for_each(tableIDs.begin(), tableIDs.end(), [&] (long long id) { std::cout << id << " "; });
  std::cout << "]" << std::endl;
  std::cout << "\t" << matrix.getNbRows() << " results" << std::endl;
  std::cout << "\t" << matrix.getGroupBy().size() << " groups" << std::endl;
  
  const aq::AQMatrix::matrix_t& m = matrix.getMatrix();

  // check size, print column name and prepare column mapping
  size_t size = 0;
  std::vector<bool> isSelected;
  std::vector<std::pair<aq::ColumnItem::Ptr, bool> > isGrouped;
  std::vector<std::pair<aq::ColumnItem::Ptr, bool> > isOrdered;
  std::map<size_t, std::map<size_t, boost::shared_ptr<aq::ColumnMapper_Intf> > > columnMappers;
  for (auto it = m.begin(); it != m.end(); ++it)
  {
    if (size == 0)
    {
      size = (*it).indexes.size();
    }
    else if (size != (*it).indexes.size())
    {
      std::cerr << "ERROR: indexes size of table differs" << std::endl;
      exit(-1);
    }
    
    std::map<size_t, boost::shared_ptr<aq::ColumnMapper_Intf> > tableColumnMappers;
    const aq::AQMatrix::matrix_t::value_type t = *it;
    aq::Table::Ptr table = baseDesc.getTable(t.table_id);
    if (aq::verbose)
      std::cout << table->getName() << ".index ; ";
    for (auto itCol = table->Columns.begin(); itCol != table->Columns.end(); ++itCol)
    {
      boost::shared_ptr<aq::ColumnMapper_Intf> cm;
      switch((*itCol)->Type)
      {
      case aq::ColumnType::COL_TYPE_INT:
        cm.reset(new aq::ColumnMapper<int32_t, FileMapper>(vdgPath.c_str(), t.table_id, (*itCol)->ID, 1/*(*itCol)->Size*/, packetSize));
        break;
      case aq::ColumnType::COL_TYPE_BIG_INT:
      case aq::ColumnType::COL_TYPE_DATE:
        cm.reset(new aq::ColumnMapper<int64_t, FileMapper>(vdgPath.c_str(), t.table_id, (*itCol)->ID, 1/*(*itCol)->Size*/, packetSize));
        break;
      case aq::ColumnType::COL_TYPE_DOUBLE:
        cm.reset(new aq::ColumnMapper<double, FileMapper>(vdgPath.c_str(), t.table_id, (*itCol)->ID, 1/*(*itCol)->Size*/, packetSize));
        break;
      case aq::ColumnType::COL_TYPE_VARCHAR:
        cm.reset(new aq::ColumnMapper<char, FileMapper>(vdgPath.c_str(), t.table_id, (*itCol)->ID, (*itCol)->Size, packetSize));
        break;
      }
      tableColumnMappers[(*itCol)->ID] = cm;
      bool isSelecting = std::find(selectedColumns.begin(), selectedColumns.end(), std::string(table->getName() + "." + (*itCol)->getName())) != selectedColumns.end();
      isSelected.push_back(isSelecting);
      bool isGrouping = std::find(groupedColumns.begin(), groupedColumns.end(), std::string(table->getName() + "." + (*itCol)->getName())) != groupedColumns.end();
      isGrouped.push_back(std::make_pair(new aq::ColumnItem, isGrouping));
      bool isOrdering = std::find(orderedColumns.begin(), orderedColumns.end(), std::string(table->getName() + "." + (*itCol)->getName())) != orderedColumns.end();
      isOrdered.push_back(std::make_pair(new aq::ColumnItem, isOrdering));

      if (aq::verbose && isSelecting)
        std::cout << table->getName() << "." << (*itCol)->getName() << " ; ";
    }
    
    columnMappers.insert(std::make_pair(t.table_id, tableColumnMappers));
  }
  if (aq::verbose)
    std::cout << std::endl;

  // print data and check group
  char buf[128];
  size_t groupIndex = 0;
  std::set<v_item_t, grp_cmp> groups;
  std::vector<size_t> groupCount(matrix.getGroupBy().size(), 0);
  for (size_t i = 0; i < size && ((limit == 0) || (i < limit)); ++i)
  {
    v_item_t v;
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
      if (aq::verbose)
        std::cout << t.table_id << "[" << t.indexes[i] << "] => ";
      for (auto& cm : columnMappers[t.table_id])
      {
        aq::ColumnItem::Ptr item(new aq::ColumnItem);
        cm.second->loadValue(t.indexes[i] - 1, *item);
        item->toString(buf, cm.second->getType());
        if (aq::verbose && *itSelected)
          std::cout << buf << " ; ";

        assert(itGrouped != isGrouped.end());
        if (itGrouped->second)
        {
          if (new_group || (i == 0))
          {
            *itGrouped->first = *item;
            v.push_back(std::make_pair(item, cm.second->getType()));
          }
          else if (!aq::ColumnItem::equal(item.get(), itGrouped->first.get(), cm.second->getType()))
          {
            std::cerr << std::endl << "BAD GROUPING: get '" << item->toString(cm.second->getType()) << "', expect '" << itGrouped->first->toString(cm.second->getType()) << "'" << std::endl;
            return -1;
          }
        }

        assert(itOrdered != isOrdered.end());
        if (itOrdered->second)
        {
          if (new_group || (i == 0))
          {
            *itOrdered->first = *item;
          }
          else if (
            !aq::ColumnItem::equal(item.get(), itGrouped->first.get(), cm.second->getType()) && 
            !aq::ColumnItem::lessThan(itGrouped->first.get(), item.get(), cm.second->getType()))
          {
            std::cerr << std::endl << "BAD ORDERING: get '" << item->toString(cm.second->getType()) << "', expect '" << itGrouped->first->toString(cm.second->getType()) << "'" << std::endl;
            return -1;
          }
        }

        ++itSelected;
        ++itGrouped;
        ++itOrdered;
      }

      if (aq::verbose)
        std::cout << " | ";
    }
    
    if (aq::verbose)
      std::cout << std::endl;

    if (!v.empty())
    {
      if (groups.size() == 0 || groups.find(v) == groups.end())
        groups.insert(v);
      else
      {
        std::string group = "[ ";
        for (auto& g : v)
        {
          group += g.first->toString(g.second) + " ";
        }
        group += "]" ;
        std::cerr << std::endl << "BAD GROUPING: group " << group << " already insert" << std::endl;
        return -1;
      }
    }

    if (whereValidator.check(matrix, columnMappers, i) == false)
    {
      std::cerr << std::endl << "WHERE VALIDATOR FAILED" << std::endl;
      return -1;
    }

  }

  return 0;
}

}
