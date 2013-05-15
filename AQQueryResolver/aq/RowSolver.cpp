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
                   boost::shared_ptr<aq::RowProcess_Intf> rowProcess)
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

      size_t tableIdx = BaseDesc.getTableIdx(c->getTableName());
      c->TableID = BaseDesc.Tables[tableIdx]->ID;

      for (std::vector<aq::tnode**>::const_iterator it = columnGroup.begin(); it != columnGroup.end(); ++it)
      {
        const aq::tnode * node = **it;
        if ((strcmp(c->getTableName().c_str(), node->left->data.val_str) == 0) && (strcmp(c->getName().c_str(), node->right->data.val_str) == 0))
        {
          c->GroupBy = true;
          break;
        }
      }
      columns.push_back(c);

      columnTypes[i]->TableID = BaseDesc.getTableIdx(columnTypes[i]->getTableName());
      columnTypes[i]->TableID = BaseDesc.Tables[columnTypes[i]->TableID]->ID;
      
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
    // Get the last column
    const std::vector<size_t>& count = aqMatrix.getCount();

    //
    // Special case for Count only
    if (columns.size() == 0)
    {
      ColumnItem::Ptr item(new ColumnItem((double)aqMatrix.getTotalCount()));
      // aq::RowProcess_Intf::row_t row(1, aq::RowProcess_Intf::row_item_t(item, COL_TYPE_INT, "", "Count"));
      aq::RowProcess_Intf::Row row;
      row.row.push_back(aq::RowProcess_Intf::row_item_t(item, COL_TYPE_INT, 4, "", "Count"));
      rowProcess->process(row);
      return;
    }

    size_t row_size = aqMatrix.hasCountColumn() ? columns.size() + 1 : columns.size();

    // For each Row
    for (size_t i = 0; i < aqMatrix.getSize(); ++i)
    {

      aq::RowProcess_Intf::Row row;
      row.row.resize(row_size, RowProcess_Intf::row_item_t(ColumnItem::Ptr(), COL_TYPE_BIG_INT, 8, "", ""));
      for (size_t j = 0; j < mapToUniqueIndex.size(); ++j) 
      {
        for (size_t c = 0; c < columns.size(); ++c)
        {
          assert(mapToUniqueIndex[j][i] < uniqueIndex[j].size());
          if (columns[c]->TableID == tableIDs[j])
          {
            row.row[c+1] = RowProcess_Intf::row_item_t(
              columnsMapper[c]->loadValue(uniqueIndex[j][mapToUniqueIndex[j][i]]), 
              columnTypes[c]->Type,
              columnTypes[c]->Size,
              columns[c]->getTableName(),
              columns[c]->getName());
            row.row[c+1].grouped = columns[c]->GroupBy;
          }
        }
      }

      if (aqMatrix.hasCountColumn())
      {
        ColumnItem::Ptr item(new ColumnItem((double)count[i]));
        row.row[0] = RowProcess_Intf::row_item_t(item, COL_TYPE_BIG_INT, 8, "", "Count", true);
        // row.row[0].displayed = true;
      }

      rowProcess->process(row);

      if ((i % 10000) == 0)
      {
        std::cout << i << std::endl;
      }

    }
    
    // the flush must be done only when an aggregate operation occur
    if (!columnGroup.empty())
      rowProcess->flush();

  }

  void solveAQMatrix(
    aq::AQMatrix& aqMatrix, 
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
