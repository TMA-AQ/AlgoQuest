#ifndef __AQ_SQLPREFIX_H__
#define __AQ_SQLPREFIX_H__

#include "parser/SQLParser.h"

namespace aq
{

std::string& syntax_tree_to_prefix_form( aq::tnode *pNode, std::string& str );
std::string& syntax_tree_to_sql_form(aq::tnode * pNode, std::string& query, unsigned int level = 0);
std::string& syntax_tree_to_sql_form_nonext(aq::tnode * pNode, std::string& query, unsigned int level = 0);
std::string& multiline_query(std::string& query);

}

#endif /* __AQ_SQLPREFIX_H__ */