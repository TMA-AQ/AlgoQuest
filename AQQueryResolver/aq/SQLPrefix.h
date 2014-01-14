#ifndef __AQ_SQLPREFIX_H__
#define __AQ_SQLPREFIX_H__

#include "parser/SQLParser.h"

namespace aq
{
  
/// \defgroup tnode_dump tnode dump processing
/// output of tnode structure into SQL or AQL string query
/// \param pNode[in] input node to dump
/// \param str[out] string to dump query node into
/// \return string containing query
/// \{
std::string syntax_tree_to_aql_form(const aq::tnode * const pNode);
std::string& syntax_tree_to_aql_form(const aq::tnode * const pNode, std::string& str);
std::string syntax_tree_to_sql_form(const aq::tnode * const pNode, unsigned int level = 0);
std::string& syntax_tree_to_sql_form(const aq::tnode * const pNode, std::string& query, unsigned int level = 0);
std::string& syntax_tree_to_sql_form_nonext(const aq::tnode * const pNode, std::string& query, unsigned int level = 0);
std::string& multiline_query(std::string& query);
/// \}

}

#endif /* __AQ_SQLPREFIX_H__ */