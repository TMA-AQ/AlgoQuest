#ifndef __QUERY_ANALYZER_H__
#define __QUERY_ANALYZER_H__

#include "parser/SQLParser.h"

namespace aq {
namespace analyze {

  enum type_t
  {
    REGULAR,
    TEMPORARY_COLUMN,
    TEMPORARY_TABLE,
    FOLD_UP_QUERY,
  };
  
  type_t analyze_query(const tnode * pNode)
  {
    // TODO
    return TEMPORARY_TABLE;
  }

}
}

#endif