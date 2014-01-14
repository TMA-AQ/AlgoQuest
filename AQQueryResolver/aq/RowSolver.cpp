#include "RowSolver.h"
#include "TemporaryColumnMapper.h"
#include "ColumnMapper.h"
#include <aq/FileMapper.h>
#include <aq/Exceptions.h>
#include <aq/Timer.h>
#include <aq/Logger.h>
#include <aq/FileMapper.h>
#include <boost/thread.hpp>
#include <boost/thread/mutex.hpp>

namespace  
{

void set_grouped(const std::vector<aq::tnode*>& columnGroup, aq::RowSolver::column_infos_t& infos)
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

aq::RowSolver::column_infos_t::mapper_type_t new_column_mapper(const aq::ColumnType type, const char * path, const size_t tableId, 
                                                               const size_t columnId, const size_t size, const size_t packetSize)
{      
  aq::RowSolver::column_infos_t::mapper_type_t cm;
  switch(type)
  {
  case aq::ColumnType::COL_TYPE_BIG_INT:
  case aq::ColumnType::COL_TYPE_DATE:
    cm = aq::ColumnMapper_Intf<int64_t>::Ptr(new aq::ColumnMapper<int64_t, aq::FileMapper>(path, tableId, columnId, size, packetSize));
    break;
  case aq::ColumnType::COL_TYPE_INT:
    cm = aq::ColumnMapper_Intf<int32_t>::Ptr(new aq::ColumnMapper<int32_t, aq::FileMapper>(path, tableId, columnId, size, packetSize));
    break;
  case aq::ColumnType::COL_TYPE_DOUBLE:
    cm = aq::ColumnMapper_Intf<double>::Ptr(new aq::ColumnMapper<double, aq::FileMapper>(path, tableId, columnId, size, packetSize));
    break;
  case aq::ColumnType::COL_TYPE_VARCHAR:
    cm = aq::ColumnMapper_Intf<char>::Ptr(new aq::ColumnMapper<char, aq::FileMapper>(path, tableId, columnId, size, packetSize));
    break;
  }
  return cm;
}

}

namespace aq 
{
  
RowSolver::RowSolver(boost::shared_ptr<aq::AQMatrix> _aqMatrix, const std::vector<Column::Ptr>& _columnTypes, 
                     const std::vector<aq::tnode*> _columnGroup, const Settings& _settings, const Base& _BaseDesc)
                     : aqMatrix(_aqMatrix), columnTypes(_columnTypes), columnGroup(_columnGroup), settings(_settings), BaseDesc(_BaseDesc)
{
}

void RowSolver::matched_index(column_infos_t& infos)
{
  auto joinPath = aqMatrix->getJoinPath();
  Table::Ptr table = BaseDesc.getTable(infos.column->getTableID());
  for (size_t j = 0; j < aqMatrix->getNbColumn(); ++j) 
  {
    if (table->getName() == joinPath[j])
    {
      infos.table_index = j;
      return;
    }
  }
  throw aq::generic_error(aq::generic_error::AQ_ENGINE, "cannot find table [%u] in join Path", infos.column->getTableID());
}

void RowSolver::prepareColumnAndColumnMapper(const std::vector<Column::Ptr>& columnTypes, 
                                             const std::vector<aq::tnode*>& columnGroup,
                                             columns_infos_t& columns_infos)
{ 
  // order must be the same that columnsType
  for (size_t i = 0; i < columnTypes.size(); ++i)
  {
    columns_infos.push_back(column_infos_t());
    auto& infos = *columns_infos.rbegin();
    
    // COLUMN
    Column::Ptr c(new Column(*columnTypes[i]));
    c->setTableID(BaseDesc.getTable(c->getTableName())->getID());
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

    columnTypes[i]->setTableID(BaseDesc.getTable(columnTypes[i]->getTableName())->getID());

    // MAPPER
    auto& cm = infos.mapper;
    if (columnTypes[i]->Temporary)
    {
      // TODO !!!!!
      assert(false);
      // cm.reset(new aq::TemporaryColumnMapper(settings.tmpPath.c_str(), columnTypes[i]->TableID, columnTypes[i]->ID, columnTypes[i]->Type, columnTypes[i]->Size, settings.packSize));
    }
    else
    {
      const aq::Column::Ptr& c = columnTypes[i];

      Table::Ptr table = BaseDesc.getTable(c->getTableID());
      while (table->isTemporary()) 
      {
        table = BaseDesc.getTable(table->getReferenceTable());
      }

      cm = new_column_mapper(c->getType(), settings.dataPath.c_str(), table->getID(), c->getID(), c->getSize(), settings.packSize);
    }
    infos.mapper = cm;
    
    // TABLE_INDEX
    matched_index(infos);

    // GROUPED
    set_grouped(columnGroup, infos);
  }
}

void RowSolver::addGroupColumn(const std::vector<Column::Ptr>& columnTypes, 
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

      Column auxCol;
      auxCol.setName(node->right->getData().val_str);
      for( size_t idx = 0; idx < table.Columns.size(); ++idx )
      {
        if( table.Columns[idx]->getName() == auxCol.getName() )
        {
          
          columns_infos.push_back(column_infos_t());
          auto& infos = *columns_infos.rbegin();

          // COLUMN
          infos.column.reset(new Column(*table.Columns[idx]));
          auto& column = infos.column;
          column->setTableName(table.getName());
          column->setTableID(BaseDesc.getTable(column->getTableName())->getID());
          column->GroupBy = true;

          // MAPPER
          auto& cm = infos.mapper;
          if (column->Temporary)
          {
            // TODO !!!
            assert(false);
            // cm.reset(new aq::TemporaryColumnMapper(settings.tmpPath.c_str(), column->TableID, column->ID, column->Type, column->Size, settings.packSize));
          }
          else
          {
            cm = new_column_mapper(column->getType(), settings.dataPath.c_str(), column->getTableID(), column->getID(), column->getSize(), settings.packSize);
          }

          // TABLE_INDEX
          matched_index(infos);

          // GROUPED
          set_grouped(columnGroup, infos);


          break;
        }
      }

    }
  }
}

template <typename T>
void set_value(const RowSolver::column_infos_t& infos, aq::row_item_t& item, size_t index)
{
  T value;
  typename aq::ColumnMapper_Intf<T>::Ptr mapper = boost::get<typename aq::ColumnMapper_Intf<T>::Ptr>(infos.mapper);
  mapper->loadValue(index, &value);
  boost::get<typename aq::ColumnItem<T> >(item.item).setValue(value);
}

template <> inline
void set_value<char*>(const RowSolver::column_infos_t& infos, aq::row_item_t& item, size_t index)
{
  char * value = new char[128]; // FIXME
  memset(value, 0, 128); // FIXME
  boost::get<aq::ColumnMapper_Intf<char>::Ptr>(infos.mapper)->loadValue(index, value);
  boost::get<aq::ColumnItem<char*> >(item.item).setValue(value);
}

void RowSolver::solve_thread(boost::shared_ptr<aq::RowProcess_Intf> rowProcess,
                             const std::pair<size_t, size_t> indexes,
                             const columns_infos_t columns,
                             const bool aggregate)
{  
  aq::Timer timer;
  std::vector<aq::Row> rows(1);
  aq::Row& row = rows[0];
  row.indexes.resize(aqMatrix->getNbColumn());
  row.initialRow.resize(columns.size());
  size_t nrow = 0;
  size_t groupByIndex = 0;
  size_t groupByCount = 0;

  // FIXME
  // load aqMatrix packet if needed
  // aqMatrix->loadNextPacket();

  for (size_t c = 0; c < columns.size(); ++c)
  {
    switch (columns[c].column->getType())
    {
    case aq::ColumnType::COL_TYPE_INT:
      row.initialRow[c].item = aq::ColumnItem<int32_t>();
      break;
    case aq::ColumnType::COL_TYPE_BIG_INT:
    case aq::ColumnType::COL_TYPE_DATE:
      row.initialRow[c].item = aq::ColumnItem<int64_t>();
      break;
    case aq::ColumnType::COL_TYPE_DOUBLE:
      row.initialRow[c].item = aq::ColumnItem<double>();
      break;
    case aq::ColumnType::COL_TYPE_VARCHAR:
      row.initialRow[c].item = aq::ColumnItem<char*>();
      break;
    }
  }

  size_t index;
  for (size_t i = indexes.first; i < indexes.second; ++i)
  {
    row.completed = !aggregate;
    row.flush = false;
    row.reinit = (groupByCount == 0);

    for (size_t n = 0; n < aqMatrix->getNbColumn(); ++n)
    {
      row.indexes[n] = aqMatrix->getColumn(n)[i];
    }

    for (size_t c = 0; c < columns.size(); ++c) // FIXME : optimisable
    {

      if ((groupByCount > 0) && columns[c].grouped) continue;

      // initial row
      aq::row_item_t& item_tmp = row.initialRow[c];
      boost::lock_guard<boost::mutex> lock(mutex); // FIXME
      index = aqMatrix->getColumn(columns[c].table_index)[i];
      // assert(index > 0); // FIXME : when outer is used, index can be 0
      if (index > 0)
      {
        index -= 1;
        switch (columns[c].column->getType())
        {
        case aq::ColumnType::COL_TYPE_INT:
          set_value<int32_t>(columns[c], item_tmp, index);
          break;
        case aq::ColumnType::COL_TYPE_BIG_INT:
          set_value<int64_t>(columns[c], item_tmp, index);
          break;
        case aq::ColumnType::COL_TYPE_DOUBLE:
          set_value<double>(columns[c], item_tmp, index);
          break;
        case aq::ColumnType::COL_TYPE_VARCHAR:
          set_value<char*>(columns[c], item_tmp, index);
          break;
        default:
          // TODO !!!
          assert(false);
          throw aq::generic_error(aq::generic_error::NOT_IMPLEMENTED, "type not supported");
          break;
        }
        item_tmp.null = false;
      }
      else
      {
        item_tmp.null = true;
      }
      if (item_tmp.columnName == "")
      {
        item_tmp.type = columns[c].column->getType();
        item_tmp.size = static_cast<unsigned int>(columns[c].column->getSize());
        item_tmp.tableName = columns[c].column->getTableName();
        item_tmp.columnName = columns[c].column->getName();
        item_tmp.grouped = columns[c].column->GroupBy;
      }
    }

    row.count = static_cast<unsigned>(aqMatrix->getCount()[i]);

    if (((i + 1) % aq::packet_size) == 0)
    {
      aq::Logger::getInstance().log(AQ_INFO, "%uM rows proceed in %s\n", (i + 1) / aq::packet_size, aq::Timer::getString(timer.getTimeElapsed()).c_str());
      timer.start();
    }

    groupByCount += aqMatrix->getCount()[i];
    if (aqMatrix->getGroupBy()[groupByIndex].first == groupByCount)
    {
      ++groupByIndex;
      groupByCount = 0;
      row.completed = true;
    }
    
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

void RowSolver::solve(boost::shared_ptr<aq::RowProcess_Intf> rowProcess,
                      uint64_t processThread,
                      bool aggregate)
{
  aq::Timer timer;

  if ((aqMatrix->getTotalCount() == 0) || (aqMatrix->getNbColumn() == 0))
    return;

  columns_infos_t columns_infos;
  
  //
  // Prepare columns infos
  prepareColumnAndColumnMapper(columnTypes, columnGroup, columns_infos);

  //
  // add group columns whose are not in select
  addGroupColumn(columnTypes, columnGroup, columns_infos);

  //
  // TODO : special case for Count only
  assert(columns_infos.size() > 0);

  if (processThread == 1)
  {
    solve_thread(rowProcess, std::make_pair((size_t)0, (size_t)aqMatrix->getNbRows()), columns_infos, aggregate);
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
      threads.create_thread(boost::bind(&RowSolver::solve_thread, this, rowProcess, *it, columns_infos, aggregate));
    }
    threads.join_all();
  }
}

}
