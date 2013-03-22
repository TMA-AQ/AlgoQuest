#include "SQLPrefix.h"
#include "ID2Str.h"
#include <stdio.h>
#include <stdlib.h>
#include <malloc.h>
#include <string.h>
#include <stack>
#include <map>
#include "DateConversion.h"

#define STR_BUF_SIZE_ROUND_UP	4096
#define EXIT_ON_MEM_ERROR		1

using namespace std;

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
void show_node( tnode *pNode, string& str, char* szTmpBuf, char* szBuffer )
{
	size_t nLen;
	char *pszToAdd;
	
	memset(szBuffer, 0, STR_BUF_SIZE);

	pszToAdd = NULL;
	switch ( pNode->tag ) {
		case K_INTEGER:
			sprintf( szTmpBuf, "K_VALUE %lld", pNode->data.val_int );
			pszToAdd = szTmpBuf;
			break;
		case K_REAL:
			doubleToString( szBuffer, pNode->data.val_number );
			sprintf( szTmpBuf, "K_VALUE %s", szBuffer );
			//sprintf( szTmpBuf, "K_VALUE %.2lf", pNode->data.val_number );
			pszToAdd = szTmpBuf;
			break;
		case K_STRING:
			sprintf( szTmpBuf, "K_VALUE '%s'", pNode->data.val_str );
			pszToAdd = szTmpBuf;
			break;
		case K_DATE_VALUE:
			bigIntToDate( pNode->data.val_int, DDMMYYYY_HHMMSS, szTmpBuf );
			pszToAdd = szTmpBuf;
			break;
		case K_IDENT:
		case K_COLUMN:
			pszToAdd = pNode->data.val_str;
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
int IsColumnReference( tnode *pNode ) {
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
void getTableAndColumnName(tnode * n, std::string& table, std::string& column) 
{
	if (n == NULL) return;
	
	if (n->tag == K_COLUMN)
	{
		column = n->data.val_str;
	}
	else if (( n->tag == K_PERIOD ) &&
					 ( (n->left != NULL) && (n->right != NULL) ) &&
					 ( n->left->tag == K_IDENT ) &&
					 ( (n->right->tag == K_IDENT) || (n->right->tag == K_COLUMN) ) )
	{
		table = n->left->data.val_str;
		column = n->right->data.val_str;
	}
}

//------------------------------------------------------------------------------
bool SameTableAndColumn(tnode * l, tnode * r, const std::map<std::string, std::string>& tablesAlias)
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

void getTableAlias(tnode *pNode, std::map<std::string, std::string>& tablesAlias)
{
	if (pNode != NULL)
	{
		if ((pNode->tag == K_AS) &&
				(pNode->left != NULL) && (pNode->left->tag == K_IDENT) &&
				(pNode->right != NULL) && (pNode->right->tag == K_IDENT))
		{
			tablesAlias.insert(std::make_pair(pNode->right->data.val_str, pNode->left->data.val_str));
		}
		else
		{
			if (pNode->left != NULL) getTableAlias(pNode->left, tablesAlias);
			if (pNode->right != NULL) getTableAlias(pNode->right, tablesAlias);
		}
	}
}

//------------------------------------------------------------------------------
void syntax_tree_to_prefix_form( tnode *pNode, string& str )
{
	if ( pNode == NULL )
		return;

	std::map<std::string, std::string> tablesAlias;

	str.clear();
	char szTmpBuf[1000];
	char szBuffer[STR_BUF_SIZE];
	stack<tnode*> nodes;
	nodes.push( pNode );
	while( nodes.size() > 0 )
	{
		tnode* pTop = nodes.top();
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
}

//------------------------------------------------------------------------------
void syntax_tree_to_sql_form(tnode * pNode, std::string& query, unsigned int level)
{
	if (pNode == NULL) return;

	if (level > 100) 
	{
		query += " ... ";
		return;
	}
	
	if (	pNode->tag == K_SELECT || pNode->tag == K_FROM 
		 || pNode->tag == K_WHERE  || pNode->tag == K_GROUP
		 || pNode->tag == K_HAVING || pNode->tag == K_ORDER )
	{
		query += " " + std::string(id_to_string(pNode->tag)) + " ";
		syntax_tree_to_sql_form(pNode->left, query, ++level);
		syntax_tree_to_sql_form(pNode->right, query, ++level);
	}
	else
	{
		syntax_tree_to_sql_form(pNode->left, query, ++level);

		std::ostringstream stmp;
		switch ( pNode->tag ) {
		case K_INTEGER:
			stmp << pNode->data.val_int;
			break;
		case K_REAL:
			stmp << pNode->data.val_number;
			break;
		case K_DATE_VALUE:
		{
			char tmp[128];
			bigIntToDate( pNode->data.val_int, DDMMYYYY_HHMMSS, tmp );
			stmp << tmp;
		}
			break;
		case K_STRING:
		case K_IDENT:
		case K_COLUMN:
			stmp << pNode->data.val_str;
			break;
		default:
			stmp << id_to_sql_string( pNode->tag );
			break;
		}

		query += " " + stmp.str() + " ";
		syntax_tree_to_sql_form(pNode->right, query, ++level);
	}
	syntax_tree_to_sql_form(pNode->next, query, ++level);
}
