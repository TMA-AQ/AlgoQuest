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

using namespace std;

namespace aq
{

//------------------------------------------------------------------------------
/* Particular cases : 
	1.) pszStr == NULL, *pncbBuf == 0, ncbInc == 0 :
	A new buffer is allocated with the default size of STR_BUF_SIZE_ROUND_UP
	2.) pszStr != NULL, *pncbBuf == 0, ncbInc == whatever :
	The buffer will be freed !
*/
char* realloc_string_buffer( char* pszStr, unsigned int *pncbBuf, unsigned int ncbInc ) {
	unsigned int nLen;
	char *pszStrNew;

	if ( ncbInc == 0 )
		ncbInc = STR_BUF_SIZE_ROUND_UP;

	if ( pszStr == NULL ) {
		/* Round up the size to 4096 multiple - avoid and on bits */
		nLen = *pncbBuf + ncbInc + STR_BUF_SIZE_ROUND_UP - 1;
		nLen = nLen - nLen % STR_BUF_SIZE_ROUND_UP;

		pszStr = (char*)malloc( sizeof( char ) * nLen );
		if ( pszStr == NULL ) {
			report_error( "Not enough memory", EXIT_ON_MEM_ERROR );
			return NULL;
		}
		/* memset( pszStr, 0, sizeof( char ) * nLen ); */
		pszStr[ 0 ] = '\0';
		*pncbBuf = nLen;				/* Set Buffer Len */

		return pszStr;
	}

	if ( pszStr != NULL ) {
		pszStrNew = realloc_string_buffer( NULL, pncbBuf, 0 );
		strcpy( pszStrNew, pszStr );
	}

	free( pszStr );

	return pszStrNew;
}

//------------------------------------------------------------------------------
// Retuned string must be freed with C-RTL's free() !
// Returns NULL on error !
void show_node( aq::tnode *pNode, std::string& str, char* szTmpBuf, char* szBuffer )
{
	const char * pszToAdd;
	
	memset(szBuffer, 0, STR_BUF_SIZE);

	pszToAdd = NULL;
	switch ( pNode->tag ) {
		case K_INTEGER:
			sprintf( szTmpBuf, "K_VALUE %lld", pNode->getData().val_int );
			pszToAdd = szTmpBuf;
			break;
		case K_REAL:
			doubleToString( szBuffer, pNode->getData().val_number );
			sprintf( szTmpBuf, "K_VALUE %s", szBuffer );
			//sprintf( szTmpBuf, "K_VALUE %.2lf", pNode->data.val_number );
			pszToAdd = szTmpBuf;
			break;
		case K_STRING:
			sprintf( szTmpBuf, "K_VALUE '%s'", pNode->getData().val_str );
			pszToAdd = szTmpBuf;
			break;
		case K_DATE_VALUE:
			bigIntToDate( pNode->getData().val_int, DDMMYYYY_HHMMSS, szTmpBuf );
			pszToAdd = szTmpBuf;
			break;
		case K_IDENT:
		case K_COLUMN:
			pszToAdd = pNode->getData().val_str;
			break;
		default:
			pszToAdd = id_to_string( pNode->tag );
			break;
	}

	if ( pszToAdd == NULL ) 
		return;
	
	//add extra spaces
	if ( str.length() > 0 ) {
		if (	pNode->tag == K_SELECT || pNode->tag == K_FROM 
				|| pNode->tag == K_WHERE || pNode->tag == K_GROUP
				|| pNode->tag == K_HAVING || pNode->tag == K_ORDER )
			str += "\n";
		else
			str += " ";
	}

	str += pszToAdd;
}

//------------------------------------------------------------------------------
/* Return : 0 false, !=0 true */
int IsColumnReference( aq::tnode *pNode ) {
	if ( pNode != NULL ) {
		if ( pNode->tag == K_COLUMN )
			return 1;
		if ( pNode->tag == K_PERIOD ) {
			if ( pNode->left != NULL && pNode->right != NULL ) {
				if ( pNode->left->tag == K_IDENT ) {
					if ( pNode->right->tag == K_IDENT || pNode->right->tag == K_COLUMN )
						return 1;
				}
			}
		}
	}
	return 0;
}

//------------------------------------------------------------------------------
void getTableAndColumnName(aq::tnode * n, std::string& table, std::string& column) 
{
	if (n == NULL) return;
	
	if (n->tag == K_COLUMN)
	{
		column = n->getData().val_str;
	}
	else if (( n->tag == K_PERIOD ) &&
					 ( (n->left != NULL) && (n->right != NULL) ) &&
					 ( n->left->tag == K_IDENT ) &&
					 ( (n->right->tag == K_IDENT) || (n->right->tag == K_COLUMN) ) )
	{
		table = n->left->getData().val_str;
		column = n->right->getData().val_str;
	}
}

//------------------------------------------------------------------------------
bool SameTableAndColumn(aq::tnode * l, aq::tnode * r, const std::map<std::string, std::string>& tablesAlias)
{
	std::string lt, lc, rt, rc;
	getTableAndColumnName(l, lt, lc);
	getTableAndColumnName(r, rt, rc);
	std::map<std::string, std::string>::const_iterator lt_it = tablesAlias.find(lt);
	std::map<std::string, std::string>::const_iterator rt_it = tablesAlias.find(rt);
	if (lt_it != tablesAlias.end()) lt = lt_it->second;
	if (rt_it != tablesAlias.end()) rt = rt_it->second;
	return (lt == rt) && (lc == rc);
}

void getTableAlias(aq::tnode *pNode, std::map<std::string, std::string>& tablesAlias)
{
	if (pNode != NULL)
	{
		if ((pNode->tag == K_AS) &&
				(pNode->left != NULL) && (pNode->left->tag == K_IDENT) &&
				(pNode->right != NULL) && (pNode->right->tag == K_IDENT))
		{
			tablesAlias.insert(std::make_pair(pNode->right->getData().val_str, pNode->left->getData().val_str));
		}
		else
		{
			if (pNode->left != NULL) getTableAlias(pNode->left, tablesAlias);
			if (pNode->right != NULL) getTableAlias(pNode->right, tablesAlias);
		}
	}
}

//------------------------------------------------------------------------------
std::string syntax_tree_to_prefix_form(aq::tnode * pNode)
{
  std::string query;
  syntax_tree_to_prefix_form(pNode, query);
  return query;
}

//------------------------------------------------------------------------------
std::string& syntax_tree_to_prefix_form( aq::tnode *sqlStatement, std::string& str )
{
	if ( sqlStatement == NULL )
		return str;
  
  aq::tnode * pNode = aq::clone_subtree(sqlStatement);

  if (pNode->tag == K_SELECT)
    aq::setOneColumnByTableOnSelect(pNode);

	// str.clear();
	char szTmpBuf[1000];
	char szBuffer[STR_BUF_SIZE];
	stack<aq::tnode*> nodes;
	nodes.push( pNode );
	while( nodes.size() > 0 )
	{
		aq::tnode* pTop = nodes.top();
		assert(pTop != NULL);
		nodes.pop();
		// Enforce K_JEQ instead of K_EQ !
		if ( pTop->tag == K_EQ ) {
			if ( IsColumnReference( pTop->left ) && IsColumnReference( pTop->right ) )
				pTop->tag = K_JEQ;
		}
#if defined (K_JAUTO_ACTIVATED)
		if ( pTop->tag == K_JEQ ) {
			if ( IsColumnReference( pTop->left ) && IsColumnReference( pTop->right ) ) {
				if ( SameTableAndColumn(pTop->left, pTop->right, tablesAlias) ) {
					pTop->tag = K_JAUTO;
				}
			}
		}
#endif
		if( pTop->tag == K_SELECT && !pTop->left ) {
			//do not show node
		}
		else {
			show_node( pTop, str, szTmpBuf, szBuffer );
#if defined (K_JAUTO_ACTIVATED)
			if (pTop->tag == K_FROM) {
				getTableAlias(pTop, tablesAlias);
			}
#endif
		}
		if( pTop->next )
			nodes.push( pTop->next );
		if( pTop->right )
			nodes.push( pTop->right );
		if( pTop->left )
			nodes.push( pTop->left );
	}

  return str;
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
	if ( pNode == NULL ) return query;

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
			char tmp[128];
			bigIntToDate( pNode->getData().val_int, DDMMYYYY_HHMMSS, tmp );
			stmp << tmp;
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
      && pNode->tag != K_REAL && pNode->tag != K_INTEGER && pNode->tag != K_DATE_VALUE )
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
	if (pNode == NULL) return query;

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
			char tmp[128];
			bigIntToDate( pNode->getData().val_int, DDMMYYYY_HHMMSS, tmp );
			stmp << tmp;
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

		query += " " + stmp.str() + " ";
		aq::syntax_tree_to_sql_form_nonext(pNode->right, query, ++level);
	}
  return query;
}

std::string& multiline_query(std::string& query)
{
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
