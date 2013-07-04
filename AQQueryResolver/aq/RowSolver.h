#ifndef __ROW_SOLVER_H__
#define __ROW_SOLVER_H__

#include "Base.h"
#include "Table.h"
#include "Column.h"
#include "AQMatrix.h"
#include "RowProcess_Intf.h"
#include "parser/SQLParser.h"
#include <aq/BaseDesc.h>
#include <vector>

namespace aq {
  
class ThreadResolver
{
public:
  ThreadResolver(
    const std::vector<aq::ColumnMapper_Intf::Ptr>& _columnsMapper,
    const boost::shared_ptr<aq::RowProcess_Intf> _rowProcess,
    const aq::AQMatrix& _aqMatrix,
    const std::vector<size_t>& _count,
    const size_t _begin,
    const size_t _end,
    const std::vector<Column::Ptr>& _columns,
    const std::vector<size_t>& _columnToAQMatrixColumn,
    const std::vector<bool>& _isGroupedColumn,
    const bool _aggregate);

  void solve();

private:
  boost::shared_ptr<aq::RowProcess_Intf> rowProcess;
  const std::vector<aq::ColumnMapper_Intf::Ptr> columnsMapper;
  const aq::AQMatrix& aqMatrix;
  const std::vector<size_t>& count;
  const size_t begin;
  const size_t end;
  const std::vector<Column::Ptr>& columns;
  const std::vector<size_t>& columnToAQMatrixColumn;
  const std::vector<bool>& isGroupedColumn;
  const bool aggregate;
};

void solveAQMatrix(aq::AQMatrix& aqMatrix, 
                   const std::vector<llong>& tableIDs, 
                   const std::vector<Column::Ptr>& columnTypes, 
                   const std::vector<aq::tnode*> columnGroup,
                   const TProjectSettings& settings, 
                   const Base& BaseDesc, 
                   boost::shared_ptr<aq::RowProcess_Intf> rowProcess,
                   std::vector<aq::Row>& rows,
                   bool aggregate = false);

void solveAQMatrix_V2(aq::AQMatrix& aqMatrix, 
                      const std::vector<llong>& tableIDs, 
                      const std::vector<Column::Ptr>& columnTypes, 
                      const std::vector<aq::tnode*> columnGroup,
                      const TProjectSettings& settings, 
                      const Base& BaseDesc, 
                      boost::shared_ptr<aq::RowProcess_Intf> rowProcess,
                      std::vector<aq::Row>& rows,
                      uint64_t nbThread,
                      bool aggregate = false);
  
void solveAQMatrix(aq::AQMatrix& aqMatrix, 
                   const std::vector<uint64_t>& tableIDs, 
                   const std::vector<Column::Ptr>& columnTypes, 
                   const TProjectSettings& settings, 
                   const aq::base_t& baseDesc, 
                   boost::shared_ptr<aq::RowProcess_Intf> rowProcess);

}

#endif