#include "RowSolver.h"
#include "TemporaryColumnMapper.h"
#include <aq/Exceptions.h>
#include <aq/Timer.h>
#include <aq/Logger.h>
#include <boost/thread.hpp>
#include <boost/thread/mutex.hpp>

boost::mutex mutex;

namespace aq 
{
  
void prepareColumnAndColumnMapper(const TProjectSettings& settings,
                                  const Base& BaseDesc,
                                  const std::vector<Column::Ptr>& columnTypes, 
                                  const std::vector<aq::tnode*>& columnGroup,
                                  std::vector<Column::Ptr>& columns,
                                  std::vector<aq::ColumnMapper_Intf::Ptr>& columnsMapper)
{ 
  // order must be the same that columnsType
  for (size_t i = 0; i < columnTypes.size(); ++i)
  {
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

    columns.push_back(c);

    columnTypes[i]->TableID = BaseDesc.getTable(columnTypes[i]->getTableName())->ID;

    ColumnMapper_Intf::Ptr cm;
    if (columnTypes[i]->Temporary)
    {
      cm.reset(new aq::TemporaryColumnMapper(settings.szTempPath1, columnTypes[i]->TableID, columnTypes[i]->ID, columnTypes[i]->Type, columnTypes[i]->Size, settings.packSize));
    }
    else
    {
      const aq::Column::Ptr c = columnTypes[i];
      switch(c->Type)
      {
      case aq::ColumnType::COL_TYPE_BIG_INT:
      case aq::ColumnType::COL_TYPE_DATE1:
      case aq::ColumnType::COL_TYPE_DATE2:
      case aq::ColumnType::COL_TYPE_DATE3:
      case aq::ColumnType::COL_TYPE_DATE4:
        // assert(c->Size == 1);
        cm.reset(new aq::ColumnMapper<int64_t>(settings.szThesaurusPath, c->TableID, c->ID, 1/*c->Size*/, settings.packSize));
        break;
      case aq::ColumnType::COL_TYPE_INT:
        // assert(c->Size == 1);
        cm.reset(new aq::ColumnMapper<int32_t>(settings.szThesaurusPath, c->TableID, c->ID, 1/*c->Size*/, settings.packSize));
        break;
      case aq::ColumnType::COL_TYPE_DOUBLE:
        // assert(c->Size == 1);
        cm.reset(new aq::ColumnMapper<double>(settings.szThesaurusPath, c->TableID, c->ID, 1/*c->Size*/, settings.packSize));
        break;
      case aq::ColumnType::COL_TYPE_VARCHAR:
        cm.reset(new aq::ColumnMapper<char>(settings.szThesaurusPath, c->TableID, c->ID, c->Size, settings.packSize));
        break;
      }
    }
    columnsMapper.push_back(cm);
  }
}

void addGroupColumn(const TProjectSettings& settings,
                    const Base& BaseDesc,
                    const std::vector<Column::Ptr>& columnTypes, 
                    const std::vector<aq::tnode*>& columnGroup,
                    std::vector<Column::Ptr>& columns,
                    std::vector<aq::ColumnMapper_Intf::Ptr>& columnsMapper)
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
          Column::Ptr column = new Column(*table.Columns[idx]);
          column->setTableName(table.getName());
          column->TableID = BaseDesc.getTable(column->getTableName())->ID;
          column->GroupBy = true;
          columns.push_back(column);
          ColumnMapper_Intf::Ptr cm;
          if (column->Temporary)
          {
            cm.reset(new aq::TemporaryColumnMapper(settings.szTempPath1, column->TableID, column->ID, column->Type, column->Size, settings.packSize));
          }
          else
          {
            switch(column->Type)
            {
            case aq::ColumnType::COL_TYPE_BIG_INT:
            case aq::ColumnType::COL_TYPE_DATE1:
            case aq::ColumnType::COL_TYPE_DATE2:
            case aq::ColumnType::COL_TYPE_DATE3:
            case aq::ColumnType::COL_TYPE_DATE4:
              // assert(column->Size == 1);
              cm.reset(new aq::ColumnMapper<int64_t>(settings.szThesaurusPath, column->TableID, column->ID, 1/*column->Size*/, settings.packSize));
              break;
            case aq::ColumnType::COL_TYPE_INT:
              // assert(column->Size == 1);
              cm.reset(new aq::ColumnMapper<int32_t>(settings.szThesaurusPath, column->TableID, column->ID, 1/*column->Size*/, settings.packSize));
              break;
            case aq::ColumnType::COL_TYPE_DOUBLE:
              // assert(column->Size == 1);
              cm.reset(new aq::ColumnMapper<double>(settings.szThesaurusPath, column->TableID, column->ID, 1/*column->Size*/, settings.packSize));
              break;
            case aq::ColumnType::COL_TYPE_VARCHAR:
              cm.reset(new aq::ColumnMapper<char>(settings.szThesaurusPath, column->TableID, column->ID, column->Size, settings.packSize));
              break;
            }
          }
          columnsMapper.push_back(cm);
          found = true;
          break;
        }
      }

    }
  }
}

ThreadResolver::ThreadResolver(
  const std::vector<aq::ColumnMapper_Intf::Ptr>& _columnsMapper,
  const boost::shared_ptr<aq::RowProcess_Intf> _rowProcess,
  const aq::AQMatrix& _aqMatrix,
  const std::vector<size_t>& _count,
  const size_t _begin,
  const size_t _end,
  const std::vector<Column::Ptr>& _columns,
  const std::vector<size_t>& _columnToAQMatrixColumn,
  const std::vector<bool>& _isGroupedColumn,
  const bool _aggregate)
  :
columnsMapper(_columnsMapper),
  aqMatrix(_aqMatrix),
  count(_count),
  begin(_begin),
  end(_end),
  columns(_columns),
  columnToAQMatrixColumn(_columnToAQMatrixColumn),
  isGroupedColumn(_isGroupedColumn),
  aggregate(_aggregate)
{
  // this->rowProcess.reset(_rowProcess->clone());
  this->rowProcess = _rowProcess; // FIXME
}

void ThreadResolver::solve()
{  
  aq::Timer timer;
  std::vector<aq::Row> rows(1);
  aq::Row& row = rows[0];
  row.initialRow.resize(columns.size());
  size_t nrow = 0;
  size_t groupByIndex = 0;
  size_t groupByCount = 0;
  for (size_t i = begin; i < end; ++i)
  {
    row.completed = !aggregate;
    row.flush = false;
    row.reinit = (groupByCount == 0);

    for (size_t c = 0; c < columns.size(); ++c) // FIXME : optimisable
    {

      if ((groupByCount > 0) && isGroupedColumn[c]) continue;

      // initial row
      aq::row_item_t& item_tmp = row.initialRow[c];
      boost::lock_guard<boost::mutex> lock(mutex);
      if (c < columnToAQMatrixColumn.size()) // FIXME
      {
        columnsMapper[c]->loadValue(aqMatrix.getColumn(columnToAQMatrixColumn[c])[i], *item_tmp.item);
        if (item_tmp.columnName == "")
        {
          item_tmp.type = columns[c]->Type;
          item_tmp.size = static_cast<unsigned int>(columns[c]->Size);
          item_tmp.tableName = columns[c]->getTableName();
          item_tmp.columnName = columns[c]->getName();
          item_tmp.grouped = columns[c]->GroupBy;
        }
      }
    }

    row.count = static_cast<unsigned>(count[i]);

    if (((i + 1) % 1000000) == 0)
    {
      aq::Logger::getInstance().log(AQ_INFO, "%uM rows proceed in %s\n", (i + 1) / 1000000, aq::Timer::getString(timer.getTimeElapsed()).c_str());
      timer.start();
    }

    groupByCount += count[i];
    if (aqMatrix.getGroupBy()[groupByIndex].first == groupByCount)
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

void solveAQMatrix_V2(aq::AQMatrix& aqMatrix, 
                      const std::vector<llong>& tableIDs, 
                      const std::vector<Column::Ptr>& columnTypes, 
                      const std::vector<aq::tnode*> columnGroup,
                      const TProjectSettings& settings, 
                      const Base& BaseDesc, 
                      boost::shared_ptr<aq::RowProcess_Intf> rowProcess,
                      std::vector<aq::Row>& rows,
                      uint64_t processThread,
                      bool aggregate)
{
  aq::Timer timer;

  if ((aqMatrix.getTotalCount() == 0) || (aqMatrix.getNbColumn() == 0))
    return;
  
  //
  // Prepare Columns and Column Mapper
  std::vector<Column::Ptr> columns;
  std::vector<aq::ColumnMapper_Intf::Ptr> columnsMapper; // order must be the same that columnsType
  prepareColumnAndColumnMapper(settings, BaseDesc, columnTypes, columnGroup, columns, columnsMapper);

  //
  // add group columns whose are not in select
  addGroupColumn(settings, BaseDesc, columnTypes, columnGroup, columns, columnsMapper);

  //
  // TODO : special case for Count only
  assert (columns.size() > 0);

  size_t row_size = columns.size();

  // compute match between column and column table in aq matrix
  std::vector<size_t> columnToAQMatrixColumn;
  for (auto& c : columns) 
  {
    for (size_t j = 0; j < aqMatrix.getNbColumn(); ++j) 
    {
      if (c->TableID == tableIDs[j])
      {
        columnToAQMatrixColumn.push_back(j);
      }
    }
  }

  // NOTE : EACH GROUP CAN BE PROCESS INDEPENDENTLY (THEREFORE MULTITHREAD CAN BE USED)
  std::vector<bool> isGroupedColumn;
  for (size_t c = 0; c < columns.size(); ++c)
  {
    bool isGrouped = false;
    for (auto& node : columnGroup)
    {
      if ((strcmp(columns[c]->getTableName().c_str(), node->left->getData().val_str) == 0) && 
        (strcmp(columns[c]->getName().c_str(), node->right->getData().val_str) == 0))
      {
        isGrouped = true;
      }
    }
    isGroupedColumn.push_back(isGrouped);
  }
  
  if (processThread == 1)
  {
    ThreadResolver solver(columnsMapper, rowProcess, aqMatrix, aqMatrix.getCount(), 0, aqMatrix.getSize(),
      columns, columnToAQMatrixColumn, isGroupedColumn, aggregate);
    solver.solve();
  }
  else
  {

    // dispatch on several thread
    const std::vector<std::pair<size_t, size_t> >& grp = aqMatrix.getGroupBy();
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
      uint64_t step = aqMatrix.getSize() / processThread;
      threadIndices.resize(processThread, std::make_pair(0, 0));
      std::vector<std::pair<size_t, size_t> >::const_iterator itGrp = grp.begin();
      uint64_t pos = 0;
      for (auto it = threadIndices.begin(); (it != threadIndices.end()) && (itGrp != grp.end()); ++it)
      {
        it->first = pos;
        if ((it + 1) == threadIndices.end())
        {
          it->second = aqMatrix.getSize();
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

      boost::shared_ptr<ThreadResolver> thr(new ThreadResolver(
        columnsMapper,
        rowProcess,
        aqMatrix,
        aqMatrix.getCount(),
        it->first,
        it->second,
        columns,
        columnToAQMatrixColumn,
        isGroupedColumn,
        aggregate));
      threads.create_thread(boost::bind(&ThreadResolver::solve, thr));
    }
    threads.join_all();
  }
}

}
