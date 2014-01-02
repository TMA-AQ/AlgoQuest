#ifndef __AQ_COLUMN2TABLE_H__
#define __AQ_COLUMN2TABLE_H__

#include "parser/SQLParser.h"
#include <aq/Base.h>

namespace aq
{

///
int enforce_qualified_column_reference(aq::tnode * pNode, 
                                       aq::Base  & baseDesc);

}

#endif