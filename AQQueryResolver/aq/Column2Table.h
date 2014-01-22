#ifndef __AQ_COLUMN2TABLE_H__
#define __AQ_COLUMN2TABLE_H__

#include "parser/SQLParser.h"
#include <aq/Base.h>

/// algoquest namespace
namespace aq
{

/// \brief add table for each column reference and check ambiguity
/// \deprectated
/// If * occur, replace it by all column
/// \param pNode
/// \param baseDesc
/// \return
int enforce_qualified_column_reference(aq::tnode * pNode, aq::Base & baseDesc);

}

#endif