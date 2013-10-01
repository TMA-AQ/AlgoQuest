#include "RowSolver.h"
#include "TemporaryColumnMapper.h"
#include <aq/FileMapper.h>
#include <aq/Exceptions.h>
#include <aq/Timer.h>
#include <aq/Logger.h>
#include <boost/thread.hpp>
#include <boost/thread/mutex.hpp>

#if defined(WIN32)
# include <aq/WIN32FileMapper.h>
typedef aq::WIN32FileMapper FileMapper;
#else
# include <aq/FileMapper.h>
typedef aq::FileMapper FileMapper;
#endif

boost::mutex mutex;

namespace aq 
{

struct column_infos_t
{
  column_infos_t() : table_index(0), grouped(false) {}
  Column::Ptr column; 
  aq::ColumnMapper_Intf::Ptr mapper; 
  size_t table_index; 
  bool grouped;
};
typedef std::vector<column_infos_t> columns_infos_t;

void matched_index(const boost::shared_ptr<aq::AQMatrix> aqMatrix, const Base& BaseDesc, column_infos_t& infos)
{
  Table::Ptr table = BaseDesc.getTable(infos.column->TableID);
  while (table->isTemporary())
  {
    table = BaseDesc.getTable(table->getReferenceTable());
  }
  for (size_t j = 0; j < aqMatrix->getNbColumn(); ++j) 
  {
    if (table->ID == aqMatrix->getMatrix()[j].table_id)
    {
      infos.table_index = j;
      return;
    }
  }
  throw aq::generic_error(aq::generic_error::INVALID_FILE, "");
}

void set_grouped(const std::vector<aq::tnode*>& columnGroup, column_infos_t& infos)
{
  for (auto& node : columnGroup)
  {
    assert(node->left);
    assert(node->right);
    if ((strcmp(infos.column->getTableName().c_str(), node->left->getData().val_str) == 0) && 
      (strcmp(infos.column->getName().c_str(), node->right->getData().val_str) == 0))
    {
      infos.grouped = true;
      return;
    }
  }
}

boost::shared_ptr<ColumnMapper_Intf> new_column_mapper(const aq::ColumnType type, const char * path, const size_t tableId, 
                                                       const size_t columnId, const size_t size, const size_t packetSize)
{      
  boost::shared_ptr<ColumnMapper_Intf> cm;
  switch(type)
  {
  case aq::ColumnType::COL_TYPE_BIG_INT:
  case aq::ColumnType::COL_TYPE_DATE:
    cm.reset(new aq::ColumnMapper<int64_t, FileMapper>(path, tableId, columnId, size, packetSize));
    break;
  case aq::ColumnType::COL_TYPE_INT:
    cm.reset(new aq::ColumnMapper<int32_t, FileMapper>(path, tableId, columnId, size, packetSize));
    break;
  case aq::ColumnType::COL_TYPE_DOUBLE:
    cm.reset(new aq::ColumnMapper<double, FileMapper>(path, tableId, columnId, size, packetSize));
    break;
  case aq::ColumnType::COL_TYPE_VARCHAR:
    cm.reset(new aq::ColumnMapper<char, FileMapper>(path, tableId, columnId, size, packetSize));
    break;
  }
  return cm;
}

void prepareColumnAndColumnMapper(const boost::shared_ptr<aq::AQMatrix> aqMatrix,
                                  const TProjectSettings& settings,
                                  const Base& BaseDesc,
                                  const std::vector<Column::Ptr>& columnTypes, 
                                  const std::vector<aq::tnode*>& columnGroup,
                                  columns_infos_t& columns_infos)
{ 
  // order must be the same that columnsType
  for (size_t i = 0; i < columnTypes.size(); ++i)
  {
    column_infos_t infos;
    
    // COLUMN
    Column::Ptr c(new Column(*columnTypes[i]));
    c->TableID = BaseDesc.getTable(c->getTableName())->ID;
    for (auto it = columnGroup.begin(); it != columnGroup.end(); ++it)
    {
      const aq::tnode * node = *it;
      if ((strcmp(c->getTableName().c_str(), node->left->getData().val_str) == 0) && (strcmp(c->getName().c_str(), node->right->getData().val_str) == 0))
      {
        c->GroupBy = true;
        break;
      }
    }
    infos.column = c;

    columnTypes[i]->TableID = BaseDesc.getTable(columnTypes[i]->getTableName())->ID;

    // MAPPER
    ColumnMapper_Intf::Ptr cm;
    if (columnTypes[i]->Temporary)
    {
      cm.reset(new aq::TemporaryColumnMapper(settings.szTempPath1, columnTypes[i]->TableID, columnTypes[i]->ID, columnTypes[i]->Type, columnTypes[i]->Size, settings.packSize));
    }
    else
    {
      const aq::Column::Ptr& c = columnTypes[i];

      Table::Ptr table = BaseDesc.getTable(c->TableID);
      while (table->isTemporary()) 
      {
        table = BaseDesc.getTable(table->getReferenceTable());
      }

      cm = new_column_mapper(c->Type, settings.szThesaurusPath, table->ID, c->ID, c->Size, settings.packSize);
    }
    infos.mapper = cm;
    
    // TABLE_INDEX
    matched_index(aqMatrix, BaseDesc, infos);

    // GROUPED
    set_grouped(columnGroup, infos);

    columns_infos.push_back(infos);
  }
}

void addGroupColumn(const boost::shared_ptr<aq::AQMatrix> aqMatrix,
                    const TProjectSettings& settings,
                    const Base& BaseDesc,
                    const std::vector<Column::Ptr>& columnTypes, 
                    const std::vector<aq::tnode*>& columnGroup,
                    columns_infos_t& columns_infos)
{  
  for (auto it = columnGroup.begin(); it != columnGroup.end(); ++it)
  {
    const aq::tnode * node = *it;
    bool inSelect = false;
    for (size_t i = 0; !inSelect && (i < columnTypes.size()); ++i)
    {
      if ((strcmp(columnTypes[i]->getTableName().c_str(), node->left->getData().val_str) == 0) && (strcmp(columnTypes[i]->getName().c_str(), node->right->getData().val_str) == 0))
      {
        inSelect = true;
      }
    }
    if (!inSelect)
    {
      // add to columns
      Table& table = *BaseDesc.getTable( node->left->getData().val_str );

      bool found = false;
      Column auxCol;
      auxCol.setName(node->right->getData().val_str);
      for( size_t idx = 0; idx < table.Columns.size(); ++idx )
      {
        if( table.Columns[idx]->getName() == auxCol.getName() )
        {

          // COLUMN
          Column::Ptr column = new Column(*table.Columns[idx]);
          column->setTableName(table.getName());
          column->TableID = BaseDesc.getTable(column->getTableName())->ID;
          column->GroupBy = true;

          // MAPPER
          ColumnMapper_Intf::Ptr cm;
          if (column->Temporary)
          {
            cm.reset(new aq::TemporaryColumnMapper(settings.szTempPath1, column->TableID, column->ID, column->Type, column->Size, settings.packSize));
          }
          else
          {
            cm = new_column_mapper(column->Type, settings.szThesaurusPath, column->TableID, column->ID, column->Size, settings.packSize);
          }

          column_infos_t infos;
          infos.column = column;
          infos.mapper = cm;

          // TABLE_INDEX
          matched_index(aqMatrix, BaseDesc, infos);

          // GROUPED
          set_grouped(columnGroup, infos);

          columns_infos.push_back(infos);

          found = true;
          break;
        }
      }

    }
  }
}

void solve(boost::shared_ptr<aq::AQMatrix> aqMatrix,
           boost::shared_ptr<aq::RowProcess_Intf> rowProcess,
           const std::pair<size_t, size_t> indexes,
           const columns_infos_t columns,
           const bool aggregate)
{  
  aq::Timer timer;
  std::vector<aq::Row> rows(1);
  aq::Row& row = rows[0];
  row.initialRow.resize(columns.size());
  size_t nrow = 0;
  size_t groupByIndex = 0;
  size_t groupByCount = 0;

  // FIXME
  // load aqMatrix packet if needed
  // aqMatrix->loadNextPacket();

  size_t index;
  for (size_t i = indexes.first; i < indexes.second; ++i)
  {
    row.completed = !aggregate;
    row.flush = false;
    row.reinit = (groupByCount == 0);

    for (size_t c = 0; c < columns.size(); ++c) // FIXME : optimisable
    {

      if ((groupByCount > 0) && columns[c].grouped) continue;

      // initial row
      aq::row_item_t& item_tmp = row.initialRow[c];
      boost::lock_guard<boost::mutex> lock(mutex); // FIXME
      index = aqMatrix->getColumn(columns[c].table_index)[i];
      // assert(index > 0); // FIXME : when outer is used, index can be 0
      index -= 1;
      columns[c].mapper->loadValue(index, *item_tmp.item);
      if (item_tmp.columnName == "")
      {
        item_tmp.type = columns[c].column->Type;
        item_tmp.size = static_cast<unsigned int>(columns[c].column->Size);
        item_tmp.tableName = columns[c].column->getTableName();
        item_tmp.columnName = columns[c].column->getName();
        item_tmp.grouped = columns[c].column->GroupBy;
      }
    }

    row.count = static_cast<unsigned>(aqMatrix->getCount()[i]);

    if (((i + 1) % 1000000) == 0)
    {
      aq::Logger::getInstance().log(AQ_INFO, "%uM rows proceed in %s\n", (i + 1) / 1000000, aq::Timer::getString(timer.getTimeElapsed()).c_str());
      timer.start();
    }

    groupByCount += aqMatrix->getCount()[i];
    if (aqMatrix->getGroupBy()[groupByIndex].first == groupByCount)
    {
      ++groupByIndex;
      groupByCount = 0;
      row.completed = true;
    }
    
    //groupByCount += 1;
    //if (aqMatrix.getGroupBy()[groupByIndex].second == groupByCount)
    //{
    //  ++groupByIndex;
    //  groupByCount = 0;
    //  row.completed = true;
    //}

    if ((nrow + 1) == rows.size())
    {
      rowProcess->process(rows);
      nrow = 0;
    }
    else
    {
      ++nrow;
    }

  }

}

void solveAQMatrix(boost::shared_ptr<aq::AQMatrix> aqMatrix, 
                   const std::vector<Column::Ptr>& columnTypes, 
                   const std::vector<aq::tnode*> columnGroup,
                   const TProjectSettings& settings, 
                   const Base& BaseDesc, 
                   boost::shared_ptr<aq::RowProcess_Intf> rowProcess,
                   uint64_t processThread,
                   bool aggregate)
{
  aq::Timer timer;

  if ((aqMatrix->getTotalCount() == 0) || (aqMatrix->getNbColumn() == 0))
    return;

  columns_infos_t columns_infos;
  
  //
  // Prepare columns infos
  prepareColumnAndColumnMapper(aqMatrix, settings, BaseDesc, columnTypes, columnGroup, columns_infos);

  //
  // add group columns whose are not in select
  addGroupColumn(aqMatrix, settings, BaseDesc, columnTypes, columnGroup, columns_infos);

  //
  // TODO : special case for Count only
  assert(columns_infos.size() > 0);

  if (processThread == 1)
  {
    solve(aqMatrix, rowProcess, std::make_pair((size_t)0, (size_t)aqMatrix->getNbRows()), columns_infos, aggregate);
  }
  else
  {

    // dispatch on several thread
    const std::vector<std::pair<uint64_t, uint64_t> >& grp = aqMatrix->getGroupBy();
    std::vector<std::pair<size_t, size_t> > threadIndices;
    if (processThread > grp.size())
    {
      processThread = grp.size();
      size_t pos = 0;
      for (auto& p : grp) 
      { 
        threadIndices.push_back(std::make_pair(pos, pos + p.first)); 
        pos += p.first;
      }
    }
    else
    {
      uint64_t step = aqMatrix->getNbRows() / processThread;
      threadIndices.resize(processThread, std::make_pair(0, 0));
      std::vector<std::pair<uint64_t, uint64_t> >::const_iterator itGrp = grp.begin();
      uint64_t pos = 0;
      for (auto it = threadIndices.begin(); (it != threadIndices.end()) && (itGrp != grp.end()); ++it)
      {
        it->first = pos;
        if ((it + 1) == threadIndices.end())
        {
          it->second = aqMatrix->getNbRows();
        }
        else
        {
          do
          {
            pos += itGrp->first;
            ++itGrp;
          } while (!((itGrp == grp.end()) || ((pos - it->first) > step)));
          it->second = pos;
        }
      }
    }

    boost::thread_group threads;
    for (auto it = threadIndices.begin(); it != threadIndices.end(); ++it)
    {
      if (it->first == it->second) 
        continue;
      threads.create_thread(boost::bind(&solve, aqMatrix, rowProcess, *it, columns_infos, aggregate));
    }
    threads.join_all();
  }
}

}
