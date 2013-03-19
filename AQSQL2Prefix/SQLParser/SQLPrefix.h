#ifndef __FIAN_SQLPREFIX_H__
#define __FIAN_SQLPREFIX_H__

#include "SQLParser.h"

//------------------------------------------------------------------------------
void syntax_tree_to_prefix_form( tnode *pNode, std::string& str );

void syntax_tree_to_sql_form(tnode * pNode, std::string& query, unsigned int level = 0);

#endif /* __FIAN_SQLPREFIX_H__ */