#ifndef __FIAN_SQLPREFIX_H__
#define __FIAN_SQLPREFIX_H__

#include "parser/SQLParser.h"

namespace aq
{

std::string& syntax_tree_to_prefix_form( tnode *pNode, std::string& str );
std::string& syntax_tree_to_sql_form(tnode * pNode, std::string& query, unsigned int level = 0);

}

#endif /* __FIAN_SQLPREFIX_H__ */