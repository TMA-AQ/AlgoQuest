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
#include <boost/tuple/tuple.hpp>

namespace aq {

void solveAQMatrix(boost::shared_ptr<aq::AQMatrix> aqMatrix, 
                   const std::vector<Column::Ptr>& columnTypes, 
                   const std::vector<aq::tnode*> columnGroup,
                   const TProjectSettings& settings, 
                   const Base& BaseDesc, 
                   boost::shared_ptr<aq::RowProcess_Intf> rowProcess,
                   uint64_t nbThread,
                   bool aggregate = false);

}

#endif