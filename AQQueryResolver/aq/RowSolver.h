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

void solveAQMatrix(aq::AQMatrix& aqMatrix, 
                   const std::vector<llong>& tableIDs, 
                   const std::vector<Column::Ptr>& columnTypes, 
                   const std::vector<aq::tnode**> columnGroup,
                   const TProjectSettings& settings, 
                   const Base& BaseDesc, 
                   boost::shared_ptr<aq::RowProcess_Intf> rowProcess,
                   bool aggregate = false);
  
void solveAQMatrix(aq::AQMatrix& aqMatrix, 
                   const std::vector<uint64_t>& tableIDs, 
                   const std::vector<Column::Ptr>& columnTypes, 
                   const TProjectSettings& settings, 
                   const aq::base_t& baseDesc, 
                   boost::shared_ptr<aq::RowProcess_Intf> rowProcess);

}

#endif