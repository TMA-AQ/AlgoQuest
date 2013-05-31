#include "RowSolver.h"
#include "TemporaryColumnMapper.h"
#include <aq/Exceptions.h>
#include <aq/Timer.h>
#include <aq/Logger.h>

namespace aq 
{
  
void solveAQMatrix(aq::AQMatrix& aqMatrix, 
                   const std::vector<llong>& tableIDs, 
                   const std::vector<Column::Ptr>& columnTypes, 
                   const std::vector<aq::tnode**> columnGroup,
                   const TProjectSettings& settings, 
                   const Base& BaseDesc, 
                   boost::shared_ptr<aq::RowProcess_Intf> rowProcess,
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
    for (size_t i = 0; i < columnTypes.size(); ++i)
    {
      Column::Ptr c(new Column(*columnTypes[i]));

      c->TableID = BaseDesc.getTable(c->getTableName())->ID;

      for (std::vector<aq::tnode**>::const_iterator it = columnGroup.begin(); it != columnGroup.end(); ++it)
      {
        const aq::tnode * node = **it;
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

    //
    // add group columns whose are not in select
    for (std::vector<aq::tnode**>::const_iterator it = columnGroup.begin(); it != columnGroup.end(); ++it)
    {
      const aq::tnode * node = **it;
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

    //
    // Get the last column
    const std::vector<size_t>& count = aqMatrix.getCount();

    //
    // Special case for Count only
    if (columns.size() == 0)
    {
      ColumnItem::Ptr item(new ColumnItem((double)aqMatrix.getTotalCount()));
      aq::Row row;
      // TODO
      // row.com.push_back(aq::Row_item_t(item, COL_TYPE_INT, 4, "", "Count"));
      rowProcess->process(row);
      return;
    }

    size_t row_size = columns.size();

    // For each Row
    timer.start();
    aq::Row row;
    row.initialRow.resize(row_size, aq::row_item_t(ColumnItem::Ptr(), COL_TYPE_BIG_INT, 8, "", ""));
    for (size_t i = 0; i < aqMatrix.getSize(); ++i)
    {
      row.computedRow.clear();
      row.completed = true;
      row.flush = false;
      for (size_t j = 0; j < mapToUniqueIndex.size(); ++j) 
      {
        for (size_t c = 0; c < columns.size(); ++c)
        {
          assert(mapToUniqueIndex[j][i] < uniqueIndex[j].size());
          if (columns[c]->TableID == tableIDs[j])
          {
            row.initialRow[c] = aq::row_item_t(
              columnsMapper[c]->loadValue(uniqueIndex[j][mapToUniqueIndex[j][i]]), 
              columns[c]->Type,
              static_cast<unsigned>(columns[c]->Size),
              columns[c]->getTableName(),
              columns[c]->getName());
            row.initialRow[c].grouped = columns[c]->GroupBy;
          }
        }
      }

      if (aqMatrix.hasCountColumn())
      {
        row.count = static_cast<unsigned>(count[i]);
      }

      if (((i + 1) % 1000000) == 0)
      {
        aq::Logger::getInstance().log(AQ_DEBUG, "%uM rows processed in %u ms\n", i / 1000000, timer.getTimeElapsed().total_milliseconds());
        timer.start();
      }

      rowProcess->process(row);

    }
    
    // the flush must be done only when an aggregate operation occur
    if (aggregate)
      rowProcess->flush();

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
