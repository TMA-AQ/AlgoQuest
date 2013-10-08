#include "Util.h"
#include <aq/FileMapper.h>
#include <aq/Base.h>
#include <aq/Timer.h>
#include <aq/Exceptions.h>
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
  
  // ----------------------------------------------------------------------------
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
    std::cerr << p << " already exist : STOP" << std::endl;
    exit(-1);
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

  aq::TProjectSettings settings;
  aq::Base baseDesc;
  baseDesc.loadFromRawFile(baseFilename.c_str());

  // Set the baseDesc for the Where Conditions
  // whereValidator.setBaseDesc(baseDesc);


  aq::AQMatrix matrix(settings, baseDesc);
  
  std::vector<long long> tableIDs;
  matrix.load(answerPath.c_str(), tableIDs);

  aq::Logger::getInstance().log(AQ_LOG_INFO, "AQMatrix: \n");
  std::stringstream ss;
  ss << tableIDs.size() << " tables: [ ";
  std::for_each(tableIDs.begin(), tableIDs.end(), [&] (long long id) { ss << id << " "; });
  ss << "]";
  aq::Logger::getInstance().log(AQ_LOG_INFO, "\t%s", ss.str().c_str());
  aq::Logger::getInstance().log(AQ_LOG_INFO, "\t%u results", matrix.getNbRows());
  aq::Logger::getInstance().log(AQ_LOG_INFO, "\t%u groups", matrix.getGroupBy().size());
  
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
      std::cerr << "FATAL ERROR: indexes size of table differs" << std::endl;
      exit(-1);
    }
    
    std::map<size_t, boost::shared_ptr<aq::ColumnMapper_Intf> > tableColumnMappers;
    const aq::AQMatrix::matrix_t::value_type t = *it;
    aq::Table::Ptr table = baseDesc.getTable(t.table_id);
    for (auto itCol = table->Columns.begin(); itCol != table->Columns.end(); ++itCol)
    {
      boost::shared_ptr<aq::ColumnMapper_Intf> cm;
      switch((*itCol)->Type)
      {
      case aq::ColumnType::COL_TYPE_INT:
        cm.reset(new aq::ColumnMapper<int32_t, FileMapper>(vdgPath.c_str(), t.table_id, (*itCol)->ID, 1/*(*itCol)->Size*/, o.packetSize));
        break;
      case aq::ColumnType::COL_TYPE_BIG_INT:
      case aq::ColumnType::COL_TYPE_DATE:
        cm.reset(new aq::ColumnMapper<int64_t, FileMapper>(vdgPath.c_str(), t.table_id, (*itCol)->ID, 1/*(*itCol)->Size*/, o.packetSize));
        break;
      case aq::ColumnType::COL_TYPE_DOUBLE:
        cm.reset(new aq::ColumnMapper<double, FileMapper>(vdgPath.c_str(), t.table_id, (*itCol)->ID, 1/*(*itCol)->Size*/, o.packetSize));
        break;
      case aq::ColumnType::COL_TYPE_VARCHAR:
        cm.reset(new aq::ColumnMapper<char, FileMapper>(vdgPath.c_str(), t.table_id, (*itCol)->ID, (*itCol)->Size, o.packetSize));
        break;
      }
      tableColumnMappers[(*itCol)->ID] = cm;
      bool isSelecting = std::find(selectedColumns.begin(), selectedColumns.end(), std::string(table->getName() + "." + (*itCol)->getName())) != selectedColumns.end();
      isSelected.push_back(isSelecting);
      bool isGrouping = std::find(groupedColumns.begin(), groupedColumns.end(), std::string(table->getName() + "." + (*itCol)->getName())) != groupedColumns.end();
      isGrouped.push_back(std::make_pair(new aq::ColumnItem, isGrouping));
      bool isOrdering = std::find(orderedColumns.begin(), orderedColumns.end(), std::string(table->getName() + "." + (*itCol)->getName())) != orderedColumns.end();
      isOrdered.push_back(std::make_pair(new aq::ColumnItem, isOrdering));

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
  char buf[128];
  size_t groupIndex = 0;
  std::set<v_item_t, grp_cmp> groups;
  std::vector<size_t> groupCount(matrix.getGroupBy().size(), 0);
  for (size_t i = 0; i < size && ((o.limit == 0) || (i < o.limit)); ++i)
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
      if (o.display && o.withIndex)
        os << t.table_id << "[" << t.indexes[i] << "] => ";
      for (auto& cm : columnMappers[t.table_id])
      {
        aq::ColumnItem::Ptr item(new aq::ColumnItem);
        cm.second->loadValue(t.indexes[i] - 1, *item);
        
        if (o.display && *itSelected)
        {
          if (t.indexes[i] > 0)
          {
            item->toString(buf, cm.second->getType());
            os << buf << " ; ";
          }
          else
          {
            os << "NULL ; ";
          }
        }

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
            aq::Logger::getInstance().log(AQ_ERROR, "BAD GROUPING: get '%s', expect '%s'\n", item->toString(cm.second->getType()), itGrouped->first->toString(cm.second->getType()));
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
            aq::Logger::getInstance().log(AQ_ERROR, "BAD ORDERING: get '%s', expect '%s'\n", item->toString(cm.second->getType()), itGrouped->first->toString(cm.second->getType()));
            return -1;
          }
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
        aq::Logger::getInstance().log(AQ_ERROR, "BAD GROUPING: group %s already insert\n", group);
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

// ------------------------------------------------------------------------------
class print_data
{
public:
  print_data(
    const struct opt& _o,
    display_cb * _cb, 
    std::vector<std::pair<size_t, boost::shared_ptr<aq::ColumnMapper_Intf> > >& _display_order) 
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

    for (auto& c : display_order)
    {
      auto& tindex = c.first;
      auto& cm = c.second;
      auto index = rows[tindex];
      if (index == 0)
      {
          cb->push("NULL");
      }
      else
      {
        aq::ColumnItem::Ptr item(new aq::ColumnItem);
        cm->loadValue(index - 1, *item);
        item->toString(buf, cm->getType());
        cb->push(buf);
      }
    }

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
  std::vector<std::pair<size_t, boost::shared_ptr<aq::ColumnMapper_Intf> > >& display_order;
};

// ------------------------------------------------------------------------------
int display(display_cb * cb,
            const std::string& answerPath,
            const struct opt& o,
            const std::vector<std::string>& selectedColumns)
{
  std::stringstream ss;
  std::string baseFilename(o.dbPath);
  baseFilename += "/base_struct/base.aqb";
  std::string vdgPath(o.dbPath);
  vdgPath += "/data_orga/vdg/data/";

  aq::TProjectSettings settings;
  aq::Base baseDesc;
  baseDesc.loadFromRawFile(baseFilename.c_str());

  aq::AQMatrix aqMatrix(settings, baseDesc);
  
  std::vector<long long> tableIDs;
  //matrix.load(answerPath.c_str(), tableIDs);
  aqMatrix.loadHeader(answerPath.c_str(), tableIDs);
  aqMatrix.prepareData(answerPath.c_str());
  //matrix.loadNextPacket();

  aq::Logger::getInstance().log(AQ_LOG_INFO, "AQMatrix: \n");
  ss << tableIDs.size() << " tables: [ ";
  std::for_each(tableIDs.begin(), tableIDs.end(), [&] (long long id) { ss << id << " "; });
  ss << "]";
  aq::Logger::getInstance().log(AQ_LOG_INFO, "\t%s", ss.str().c_str());
  aq::Logger::getInstance().log(AQ_LOG_INFO, "\t%u results", aqMatrix.getNbRows());
  aq::Logger::getInstance().log(AQ_LOG_INFO, "\t%u groups", aqMatrix.getGroupBy().size());
  
  const aq::AQMatrix::matrix_t& matrix = aqMatrix.getMatrix();

  // check size, print column name and prepare column mapping
  size_t size = 0;
  std::vector<std::pair<size_t, boost::shared_ptr<aq::ColumnMapper_Intf> > > display_order(selectedColumns.size());
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
        boost::shared_ptr<aq::ColumnMapper_Intf> cm;
        switch(col->Type)
        {
        case aq::ColumnType::COL_TYPE_INT:
          cm.reset(new aq::ColumnMapper<int32_t, FileMapper>(vdgPath.c_str(), t.table_id, col->ID, 1/*(*itCol)->Size*/, o.packetSize));
          break;
        case aq::ColumnType::COL_TYPE_BIG_INT:
        case aq::ColumnType::COL_TYPE_DATE:
          cm.reset(new aq::ColumnMapper<int64_t, FileMapper>(vdgPath.c_str(), t.table_id, col->ID, 1/*(*itCol)->Size*/, o.packetSize));
          break;
        case aq::ColumnType::COL_TYPE_DOUBLE:
          cm.reset(new aq::ColumnMapper<double, FileMapper>(vdgPath.c_str(), t.table_id, col->ID, 1/*(*itCol)->Size*/, o.packetSize));
          break;
        case aq::ColumnType::COL_TYPE_VARCHAR:
          cm.reset(new aq::ColumnMapper<char, FileMapper>(vdgPath.c_str(), t.table_id, col->ID, col->Size, o.packetSize));
          break;
        }
        display_order[std::distance(selectedColumns.begin(), it)] = std::make_pair(tindex, cm);
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
  if (size == 0)
  {
    print_data pd_cb(o, cb, display_order);
    aqMatrix.readData<print_data>(pd_cb);
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
        auto& tindex = c.first;
        auto& cm = c.second;
        auto index = matrix[tindex].indexes[i];
        if (index == 0)
        {
          cb->push("NULL");
        }
        else
        {
          aq::ColumnItem::Ptr item(new aq::ColumnItem);
          cm->loadValue(index - 1, *item);
          item->toString(buf, cm->getType());
          cb->push(buf);
        }
      }

      if (o.withCount)
      {
        ss.str("");
        ss << aqMatrix.getCount()[i];
        cb->push(ss.str());
      }
    }
  }
  return 0;
}

}
