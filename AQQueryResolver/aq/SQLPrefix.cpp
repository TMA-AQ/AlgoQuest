#include "SQLPrefix.h"
#include "TreeUtilities.h"
#include "parser/ID2Str.h"
#include <cstdio>
#include <cassert>
#include <cstdlib>
#include <cstring>
#include <stack>
#include <map>
#include <aq/DateConversion.h>

#define STR_BUF_SIZE_ROUND_UP	4096
#define EXIT_ON_MEM_ERROR		1

namespace aq
{

//------------------------------------------------------------------------------
bool IsColumnReference( aq::tnode *pNode ) {
	if ( pNode != nullptr ) {
		if ( pNode->tag == K_COLUMN )
			return true;
		if ( pNode->tag == K_PERIOD ) {
			if ( pNode->left != nullptr && pNode->right != nullptr ) {
				if ( pNode->left->tag == K_IDENT ) {
					if ( pNode->right->tag == K_IDENT || pNode->right->tag == K_COLUMN )
						return true;
				}
			}
		}
	}
	return false;
}

//------------------------------------------------------------------------------
void getTableAndColumnName(aq::tnode * n, std::string& table, std::string& column) 
{
	if (n == nullptr) return;
	
	if (n->tag == K_COLUMN)
	{
		column = n->getData().val_str;
	}
	else if (( n->tag == K_PERIOD ) &&
					 ( (n->left != nullptr) && (n->right != nullptr) ) &&
					 ( n->left->tag == K_IDENT ) &&
					 ( (n->right->tag == K_IDENT) || (n->right->tag == K_COLUMN) ) )
	{
		table = n->left->getData().val_str;
		column = n->right->getData().val_str;
	}
}

//------------------------------------------------------------------------------
std::string syntax_tree_to_aql_form(aq::tnode * pNode)
{
  std::string query;
  syntax_tree_to_aql_form(pNode, query);
  return query;
}

//------------------------------------------------------------------------------
std::string& syntax_tree_to_aql_form( aq::tnode *pNode, std::string& query )
{
  if ( pNode == nullptr ) return query;

	if (	pNode->tag == K_SELECT || pNode->tag == K_FROM 
		 || pNode->tag == K_WHERE  || pNode->tag == K_GROUP
		 || pNode->tag == K_HAVING || pNode->tag == K_ORDER )
	{
    if (pNode->tag != K_SELECT)
      query += "\n";
    query += std::string(id_to_string(pNode->tag));
		aq::syntax_tree_to_aql_form(pNode->left, query);
		aq::syntax_tree_to_aql_form(pNode->right, query);
	  aq::syntax_tree_to_aql_form(pNode->next, query);
	}
	else
	{
		std::ostringstream stmp;

    if ((pNode->tag == K_EQ) && IsColumnReference(pNode->left) && IsColumnReference(pNode->right))
    {
      pNode->tag = K_JEQ;
    }

    if (pNode->tag == K_JNO) 
    {
      assert(pNode->left);
      query += " " + std::string(id_to_string(pNode->tag));
      aq::syntax_tree_to_aql_form(pNode->left->left, query);
    }
    else if ((pNode->tag == K_JEQ) || (pNode->tag == K_JINF) || (pNode->tag == K_JIEQ) || (pNode->tag == K_JSEQ) || (pNode->tag == K_JSUP))
    {
      assert(pNode->left);
      assert(pNode->right);
      query += " " + std::string(id_to_string(pNode->tag));
      aq::syntax_tree_to_aql_form(pNode->right, query);
      aq::syntax_tree_to_aql_form(pNode->left, query);
    }
    else
    {
      switch ( pNode->tag ) {
      case K_INTEGER:
        stmp << "K_VALUE "  << pNode->getData().val_int;
        break;
      case K_REAL:
        stmp << "K_VALUE "  << pNode->getData().val_number;
        break;
      case K_DATE_VALUE:
        {
          DateConversion dateConverter;
          stmp << "K_VALUE " << dateConverter.bigIntToDate(pNode->getData().val_int);
        }
        break;
      case K_STRING:
        stmp << "K_VALUE '" <<  pNode->getData().val_str << "'";
        break;
      case K_IDENT:
      case K_COLUMN:
        stmp << pNode->getData().val_str;
        break;
      default:
        stmp << id_to_string( pNode->tag );
        break;
      }
      query += " " + stmp.str();
      aq::syntax_tree_to_aql_form(pNode->left, query);
      aq::syntax_tree_to_aql_form(pNode->right, query);
    }
	}
  return query;
}

//------------------------------------------------------------------------------
std::string syntax_tree_to_sql_form(aq::tnode * pNode, unsigned int level)
{
  std::string query;
  syntax_tree_to_sql_form(pNode, query, level);
  return query;
}

//------------------------------------------------------------------------------
std::string& syntax_tree_to_sql_form(aq::tnode * pNode, std::string& query, unsigned int level)
{
	if ( pNode == nullptr ) return query;

	if ( level > 100 )
	{
		query += " ... ";
		return query;
	}
	
  bool bill = false;

	if (	pNode->tag == K_SELECT || pNode->tag == K_FROM 
		 || pNode->tag == K_WHERE  || pNode->tag == K_GROUP
		 || pNode->tag == K_HAVING || pNode->tag == K_ORDER )
	{
    if ( pNode->tag == K_SELECT && pNode->parent )
      query += " (";
    if ( pNode->tag == K_SELECT && pNode->parent || pNode->tag != K_SELECT )
		  query += " ";
    query += std::string(id_to_string(pNode->tag));
		aq::syntax_tree_to_sql_form(pNode->left, query, ++level);
		aq::syntax_tree_to_sql_form(pNode->right, query, ++level);
	  aq::syntax_tree_to_sql_form(pNode->next, query, ++level);
    if ( pNode->tag == K_SELECT && pNode->parent )
      query += " )";
	}
	else
	{
    if ( pNode->right )
		  aq::syntax_tree_to_sql_form(pNode->left, query, ++level);
    else
      bill = true;

		std::ostringstream stmp;
		switch ( pNode->tag ) {
		case K_INTEGER:
			stmp << pNode->getData().val_int;
			break;
		case K_REAL:
			stmp << pNode->getData().val_number;
			break;
		case K_DATE_VALUE:
		{
      DateConversion dateConverter;
			stmp << dateConverter.bigIntToDate(pNode->getData().val_int);
		}
			break;
		case K_STRING:
		case K_IDENT:
		case K_COLUMN:
			stmp << pNode->getData().val_str;
			break;
		default:
			stmp << id_to_sql_string( pNode->tag );
			break;
		}
		query += " " + stmp.str();
    if ( bill == true && pNode->tag != K_IDENT && pNode->tag != K_STRING && pNode->tag != K_COLUMN
      && pNode->tag != K_REAL && pNode->tag != K_INTEGER && pNode->tag != K_DATE_VALUE && pNode->tag != K_STAR )
    {
      query += " (";
      aq::syntax_tree_to_sql_form(pNode->left, query, ++level);
      query += " )";
    }
    else if ( bill == true )
      aq::syntax_tree_to_sql_form(pNode->left, query, ++level);
    bill = false;

		aq::syntax_tree_to_sql_form(pNode->right, query, ++level);
	  aq::syntax_tree_to_sql_form(pNode->next, query, ++level); 
	}
  return query;
}

//------------------------------------------------------------------------------
std::string& syntax_tree_to_sql_form_nonext(aq::tnode * pNode, std::string& query, unsigned int level)
{
	if (pNode == nullptr) return query;

	if (level > 100)
	{
		query += " ... ";
		return query;
	}
	
	if (	pNode->tag == K_SELECT || pNode->tag == K_FROM 
		 || pNode->tag == K_WHERE  || pNode->tag == K_GROUP
		 || pNode->tag == K_HAVING || pNode->tag == K_ORDER )
	{
		query += " " + std::string(id_to_string(pNode->tag)) + " ";
		aq::syntax_tree_to_sql_form_nonext(pNode->left, query, ++level);
		aq::syntax_tree_to_sql_form_nonext(pNode->right, query, ++level);
	}
	else
	{
		aq::syntax_tree_to_sql_form_nonext(pNode->left, query, ++level);

		std::ostringstream stmp;
		switch ( pNode->tag ) {
		case K_INTEGER:
			stmp << pNode->getData().val_int;
			break;
		case K_REAL:
			stmp << pNode->getData().val_number;
			break;
		case K_DATE_VALUE:
		{
      DateConversion dateConverter;
			stmp << dateConverter.bigIntToDate(pNode->getData().val_int);
		}
			break;
		case K_STRING:
      stmp <<  pNode->getData().val_str;
      break;
		case K_IDENT:
		case K_COLUMN:
			stmp << pNode->getData().val_str;
			break;
		default:
			stmp << id_to_sql_string( pNode->tag );
			break;
		}

		query += " " + stmp.str() + " ";
		aq::syntax_tree_to_sql_form_nonext(pNode->right, query, ++level);
	}
  return query;
}

//------------------------------------------------------------------------------
std::string& multiline_query(std::string& query)
{
  boost::erase_all(query, "\n");
  std::string::size_type pos = std::string::npos;
  pos = query.find("FROM");
  if (pos != std::string::npos)
    query.insert(pos, "\n");
  pos = query.find("WHERE");
  if (pos != std::string::npos)
    query.insert(pos, "\n");
  std::string::size_type prvAnd = 0;
  while ((pos = query.find("AND", prvAnd)) != std::string::npos)
  {
    query.insert(pos, "\n  ");
    prvAnd = pos + 6;
  }
  pos = query.find("GROUP");
  if (pos != std::string::npos)
    query.insert(pos, "\n");
  pos = query.find("ORDER");
  if (pos != std::string::npos)
    query.insert(pos, "\n");
  return query;
}

}
