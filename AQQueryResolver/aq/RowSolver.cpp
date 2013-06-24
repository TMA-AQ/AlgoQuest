#include "RowSolver.h"
#include "TemporaryColumnMapper.h"
#include <aq/Exceptions.h>
#include <aq/Timer.h>
#include <aq/Logger.h>

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
      cm.reset(new aq::ColumnMapper(settings.szThesaurusPath, columnTypes[i]->TableID, columnTypes[i]->ID, columnTypes[i]->Type, columnTypes[i]->Size, settings.packSize));
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
            cm.reset(new aq::ColumnMapper(settings.szThesaurusPath, column->TableID, column->ID, column->Type, column->Size, settings.packSize));
          }
          columnsMapper.push_back(cm);
          found = true;
          break;
        }
      }

    }
  }
}

void solveAQMatrix(aq::AQMatrix& aqMatrix, 
                   const std::vector<llong>& tableIDs, 
                   const std::vector<Column::Ptr>& columnTypes, 
                   const std::vector<aq::tnode*> columnGroup,
                   const TProjectSettings& settings, 
                   const Base& BaseDesc, 
                   boost::shared_ptr<aq::RowProcess_Intf> rowProcess,
                   std::vector<aq::Row>& rows,
                   bool aggregate)
{
  aq::Timer timer;

  if ((aqMatrix.getTotalCount() == 0) || (aqMatrix.getNbColumn() == 0))
    return;

  std::vector<std::vector<size_t> > mapToUniqueIndex;
  std::vector<std::vector<size_t> > uniqueIndex;

  timer.start();
  aqMatrix.computeUniqueRow(mapToUniqueIndex, uniqueIndex);
  aq::Logger::getInstance().log(AQ_INFO, "Sorted and Unique Index: Time Elapsed = %s\n", aq::Timer::getString(timer.getTimeElapsed()).c_str());

  //
  // Prepare Columns and Column Mapper
  std::vector<Column::Ptr> columns;
  std::vector<aq::ColumnMapper_Intf::Ptr> columnsMapper; // order must be the same that columnsType
  prepareColumnAndColumnMapper(settings, BaseDesc, columnTypes, columnGroup, columns, columnsMapper);

  //
  // add group columns whose are not in select
  addGroupColumn(settings, BaseDesc, columnTypes, columnGroup, columns, columnsMapper);

  //
  // Get the last column
  const std::vector<size_t>& count = aqMatrix.getCount();

  //
  // Special case for Count only
  if (columns.size() == 0)
  {
    ColumnItem::Ptr item(new ColumnItem((double)aqMatrix.getTotalCount()));
    std::vector<aq::Row> rows;
    // TODO
    // row.com.push_back(aq::Row_item_t(item, COL_TYPE_INT, 4, "", "Count"));
    rowProcess->process(rows);
    return;
  }

  size_t row_size = columns.size();

  // For each Row
  timer.start();
  for (auto it = rows.begin(); it != rows.end(); ++it)
  {
    aq::Row& row = *it;
    row.initialRow.resize(row_size);
    row.computedRow; // .resize((std::max)(row_size, (size_t)32)); // fixme
  }
  size_t nrow = 0;
  for (size_t i = 0; i < aqMatrix.getSize(); ++i)
  {
    // row.computedRow.clear();
    rows[nrow].completed = true;
    rows[nrow].flush = false;
    for (size_t j = 0; j < mapToUniqueIndex.size(); ++j) 
    {
      for (size_t c = 0; c < columns.size(); ++c)
      {
        assert(mapToUniqueIndex[j][i] < uniqueIndex[j].size());
        if (columns[c]->TableID == tableIDs[j])
        {

          // initial row
          aq::row_item_t& item_tmp = rows[nrow].initialRow[c];
          columnsMapper[c]->loadValue(uniqueIndex[j][mapToUniqueIndex[j][i]], *item_tmp.item);
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
    }

    if (aqMatrix.hasCountColumn())
    {
      rows[nrow].count = static_cast<unsigned>(count[i]);
    }

    if (((i + 1) % 1000000) == 0)
    {
      aq::Logger::getInstance().log(AQ_INFO, "%uM rows proceed in %u\n", (i + 1) / 1000000, aq::Timer::getString(timer.getTimeElapsed()).c_str());
      timer.start();
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

  // the flush must be done only when an aggregate operation occur
  if (aggregate)
    rowProcess->flush(rows);

}
    
void solveAQMatrix_V2(aq::AQMatrix& aqMatrix, 
                      const std::vector<llong>& tableIDs, 
                      const std::vector<Column::Ptr>& columnTypes, 
                      const std::vector<aq::tnode*> columnGroup,
                      const TProjectSettings& settings, 
                      const Base& BaseDesc, 
                      boost::shared_ptr<aq::RowProcess_Intf> rowProcess,
                      std::vector<aq::Row>& rows,
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
  // Get the last column
  const std::vector<size_t>& count = aqMatrix.getCount();

  //
  // Special case for Count only
  if (columns.size() == 0)
  {
    ColumnItem::Ptr item(new ColumnItem((double)aqMatrix.getTotalCount()));
    std::vector<aq::Row> rows;
    // TODO
    // row.com.push_back(aq::Row_item_t(item, COL_TYPE_INT, 4, "", "Count"));
    rowProcess->process(rows);
    return;
  }

  size_t row_size = columns.size();

  // compute match between column and column table in aq matrix
  std::vector<size_t> columnToAQMatrixColumn;
  std::for_each(columns.begin(), columns.end(), [&] (Column::Ptr c) {
    for (size_t j = 0; j < aqMatrix.getNbColumn(); ++j) 
    {
      if (c->TableID == tableIDs[j])
      {
        columnToAQMatrixColumn.push_back(j);
      }
    }
  });

  // For each Row
  timer.start();
  for (auto it = rows.begin(); it != rows.end(); ++it)
  {
    aq::Row& row = *it;
    row.initialRow.resize(row_size);
    row.computedRow;
  }
  size_t nrow = 0;
  size_t groupByIndex = 0;
  size_t groupByCount = 0;

  // NOTE : EACH GROUP CAN BE PROCESS INDEPENDENTLY (THEREFORE MULTITHREAD CAN BE USED)
  std::vector<bool> isGroupedColumn;
  for (size_t c = 0; c < columns.size(); ++c)
  {
    bool isGrouped = false;
    for (auto it = columnGroup.begin(); !isGrouped && (it != columnGroup.end()); ++it)
    {
      const aq::tnode * node = *it;
      if ((strcmp(columns[c]->getTableName().c_str(), node->left->getData().val_str) == 0) && 
        (strcmp(columns[c]->getName().c_str(), node->right->getData().val_str) == 0))
      {
        isGrouped = true;
      }
    }
    isGroupedColumn.push_back(isGrouped);
  }

  for (size_t i = 0; i < aqMatrix.getSize(); ++i)
  {
    // row.computedRow.clear();
    rows[nrow].completed = !aggregate;
    rows[nrow].flush = false;

    for (size_t c = 0; c < columns.size(); ++c) // FIXME : optimisable
    {

      if ((groupByCount > 0) && isGroupedColumn[c]) continue;

      // initial row
      aq::row_item_t& item_tmp = rows[nrow].initialRow[c];
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

    if (aqMatrix.hasCountColumn())
    {
      rows[nrow].count = static_cast<unsigned>(count[i]);
    }

    if (((i + 1) % 1000000) == 0)
    {
      aq::Logger::getInstance().log(AQ_INFO, "%uM rows proceed in %s\n", (i + 1) / 1000000, aq::Timer::getString(timer.getTimeElapsed()).c_str());
      timer.start();
    }
    
    ++groupByCount;
    if (aqMatrix.getGroupBy()[groupByIndex].second == groupByCount)
    {
      ++groupByIndex;
      groupByCount = 0;
      rows[0].completed = true;
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

  // the flush must be done only when an aggregate operation occur
  //if (aggregate)
  //  rowProcess->flush(rows);

}

void solveAQMatrix(aq::AQMatrix& aqMatrix, 
                   const std::vector<uint64_t>& tableIDs, 
                   const std::vector<Column::Ptr>& columnTypes, 
                   const TProjectSettings& settings, 
                   const aq::base_t& baseDesc, 
                   boost::shared_ptr<aq::RowProcess_Intf> rowProcess)
{
  // TODO
  assert(false);
  throw aq::generic_error(aq::generic_error::NOT_IMPLEMENED, "__FUNCTION__");
}

}
