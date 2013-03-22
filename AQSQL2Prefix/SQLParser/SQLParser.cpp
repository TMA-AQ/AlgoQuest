#include "SQLParser.h"
#include "Table.h"
#include "sql92_grm_tab.h"
#include "ID2Str.h"

#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <cassert>
#include <cerrno>
#include <stack>

#define STR_BUF_SIZE_ROUND_UP	4096
#define EXIT_ON_MEM_ERROR		1

extern int yyerror ( const char *pszMsg );

using namespace std;

//------------------------------------------------------------------------------
/* bExit == 1 - exit the app. - ex. for memmory allocation failed */
void report_error( char* pszMsg, int bExit ) {
	int errnum;
	char szBuf[ 1000 ];

	errnum = errno;	/* Save it for later use */

	if ( pszMsg != NULL ) {
		sprintf( szBuf, "Error = %s : %s\n", pszMsg, strerror( errno ) );
		yyerror( szBuf );
	} else {
		sprintf( szBuf, "Error : %s\n", strerror( errno ) );
		yyerror( szBuf );
	}

	if ( bExit == 1 ) {
		if ( errnum != 0 )
			exit( -errnum );
		else
			exit( -1 );
	}
}

//------------------------------------------------------------------------------
tnode* new_node( short tag ) {
	tnode* pNode;
	
	pNode = (tnode*)malloc( sizeof( tnode ) );
	if ( pNode == NULL ) {
		report_error( "Not enough memory", EXIT_ON_MEM_ERROR );
		return NULL;
	}
	memset( pNode, 0, sizeof( tnode ) );
	pNode->eNodeDataType = NODE_DATA_INT;
	pNode->tag = tag;
	return pNode;
}

//------------------------------------------------------------------------------
tnode* new_node( tnode* pNode )
{
	if( !pNode )
		return NULL;
	tnode* pNew;

	pNew = (tnode*)malloc( sizeof( tnode ) );
	memset( pNew, 0, sizeof( tnode ) );
	if ( pNew == NULL ) {
		report_error( "Not enough memory", EXIT_ON_MEM_ERROR );
		return NULL;
	}
	pNew->inf = pNode->inf;
	pNew->tag = pNode->tag;
	switch( pNode->eNodeDataType )
	{
	case NODE_DATA_STRING:
		set_string_data( pNew, pNode->data.val_str );
		break;
	case NODE_DATA_INT:
		set_int_data( pNew, pNode->data.val_int );
		break;
	case NODE_DATA_NUMBER:
		set_double_data( pNew, pNode->data.val_number );
		break;
	default:
		assert( 0 );
	}
	return pNew;
}

//------------------------------------------------------------------------------
tnode* set_string_data( tnode* pNode, const char* pszStr ) {
	if ( pNode != NULL ) {
		size_t nLen;

		if ( pszStr == NULL ) {
			/* Call free */
			if (	pNode->eNodeDataType == NODE_DATA_STRING 
					&& pNode->nStrBufCb != 0 
					&& pNode->data.val_str != NULL ) {
				free( pNode->data.val_str );
				pNode->data.val_str = NULL;
				pNode->nStrBufCb	= 0;
			}
			return pNode;
		}

		nLen = strlen( pszStr ) + 1;		/* Include terminating NULL char */

		if ( pNode->eNodeDataType != NODE_DATA_STRING || pNode->nStrBufCb < nLen ) {
			/* Need to allocate or reallocate the data buffer */

			if ( pNode->nStrBufCb > 0 )
				set_string_data( pNode, NULL );	/* Free Old Buffer - avoid realloc */

			/* Round up the size to 4096 multiple - avoid and on bits */
			nLen = nLen + STR_BUF_SIZE_ROUND_UP - 1;
			nLen = nLen - nLen % STR_BUF_SIZE_ROUND_UP;

			pNode->data.val_str = (char*)malloc( sizeof( char ) * nLen );
			if ( pNode->data.val_str == NULL ) {
				report_error( "Not enough memory", EXIT_ON_MEM_ERROR );
				return NULL;
			}
			pNode->nStrBufCb		= nLen;				/* Set Buffer Len */
			pNode->eNodeDataType	= NODE_DATA_STRING; /* Set Correct Data Type */
		}

		strcpy( pNode->data.val_str, pszStr );
	}

	return pNode;
}

//------------------------------------------------------------------------------
tnode* append_string_data( tnode* pNode, char* pszStr ) {
	if ( pNode != NULL ) {
		unsigned int nLen, nLen1;

		if ( pNode->eNodeDataType != NODE_DATA_STRING || pszStr == NULL )
			return set_string_data( pNode, pszStr );

		nLen = strlen( pNode->data.val_str );	
		if ( nLen == 0 )	/* Empty String */
			return set_string_data( pNode, pszStr );

		nLen1 = strlen( pszStr );
		if ( nLen1 == 0 )	/* Empty String */
			return pNode;	/* Nothing to do ! */
	
		nLen += nLen1 + 1;	/* Include terminating NULL char */

		if ( pNode->nStrBufCb < nLen ) {
			/* Need to allocate or reallocate the data buffer */
			char *pszBuf;

			/* Round up the size to 4096 multiple - avoid and on bits */
			nLen = nLen + STR_BUF_SIZE_ROUND_UP - 1;
			nLen = nLen - nLen % STR_BUF_SIZE_ROUND_UP;

			pszBuf = (char*)malloc( sizeof( char ) * nLen );
			if ( pszBuf == NULL ) {
				report_error( "Not enough memory", EXIT_ON_MEM_ERROR );
				return NULL;
			}

			strcpy( pszBuf, pNode->data.val_str );	/* Keep the old string */
			set_string_data( pNode, NULL );	/* Free Old Buffer - avoid realloc */
			pNode->data.val_str = pszBuf;	/* Set the new buffer */

			pNode->nStrBufCb		= nLen;				/* Set Buffer Len */
			pNode->eNodeDataType	= NODE_DATA_STRING; /* Set Correct Data Type */
		}

		/* Append the new string */
		strcat( pNode->data.val_str, pszStr );
	}

	return pNode;
}

//------------------------------------------------------------------------------
tnode* set_int_data( tnode* pNode, llong nVal ) {
	if ( pNode != NULL ) {
		if ( pNode->eNodeDataType == NODE_DATA_STRING )
			set_string_data( pNode, NULL );		/* Free the String Buffer */

		pNode->data.val_int	 = nVal;
		pNode->eNodeDataType = NODE_DATA_INT;
	}

	return pNode;
}

//------------------------------------------------------------------------------
tnode* set_double_data( tnode* pNode, double dVal ) {
	if ( pNode != NULL ) {
		if ( pNode->eNodeDataType == NODE_DATA_STRING )
			set_string_data( pNode, NULL );		/* Free the String Buffer */

		pNode->data.val_number	= dVal;
		pNode->eNodeDataType	= NODE_DATA_NUMBER;
	}

	return pNode;
}

//------------------------------------------------------------------------------
tnode* delete_node( tnode* pNode ) {
	if ( pNode != NULL ) {
		free( pNode );
	}
	return NULL;
}

//------------------------------------------------------------------------------
void delete_subtree( tnode* pNode ) {
	if( !pNode )
		return;

	deque<tnode*> nodes;
	nodes.push_back( pNode );
	size_t idx = 0;
	while( idx < nodes.size() )
	{
		pNode = nodes[idx];
		if ( pNode->left )
			nodes.push_back( pNode->left );
		if ( pNode->right )
			nodes.push_back( pNode->right );
		if ( pNode->next )
			nodes.push_back( pNode->next );
		++idx;
	}
	for( idx = 0; idx < nodes.size(); ++idx )
	{
		set_string_data( nodes[idx], NULL );	/* Delete String Buffer if any */
		delete_node( nodes[idx] );
	}
}

//------------------------------------------------------------------------------
tnode* get_leftmost_child( tnode *pNode ) {
	if ( pNode == NULL )
		return NULL;
	if ( pNode->left != NULL )
		return get_leftmost_child( pNode->left );
	else if ( pNode->right != NULL )
		return get_leftmost_child( pNode->right );
	return pNode;
}

//------------------------------------------------------------------------------
tnode* set_data( tnode* pNode, const Scalar& scalar )
{
	if ( pNode == NULL ) 
		return NULL;

	switch( scalar.Type )
	{
	case COL_TYPE_DOUBLE:
		pNode->tag = K_REAL;
		set_double_data( pNode, scalar.Item.numval );
		break;
	case COL_TYPE_INT:
	case COL_TYPE_BIG_INT:
	case COL_TYPE_DATE1:
	case COL_TYPE_DATE2:
	case COL_TYPE_DATE3:
		pNode->tag = K_INTEGER;
		set_int_data( pNode, (llong) scalar.Item.numval );
		break;
	case COL_TYPE_VARCHAR:
		pNode->tag = K_STRING;
		set_string_data( pNode, scalar.Item.strval.c_str() );
		break;
	default:
		assert( 0 );
	}
	return pNode;
}

//------------------------------------------------------------------------------
std::string to_string(const tnode* const pNode )
{
	char szBuffer[STR_BUF_SIZE]; // tma: FIXME
	memset(szBuffer, 0, STR_BUF_SIZE);
	if( !pNode )
		return "";
	switch( pNode->eNodeDataType )
	{
	case NODE_DATA_INT: 
		sprintf(szBuffer, "%lld", pNode->data.val_int);
		return szBuffer;
		break;
	case NODE_DATA_NUMBER: 
		aq::doubleToString( szBuffer, pNode->data.val_number );
		//sprintf(szBuffer, "%.2lf", pNode->data.val_number);
		return szBuffer;
		break;
	case NODE_DATA_STRING:
		return pNode->data.val_str;
		break;
	default:
		assert( 0 );
	}
	return "";
}

//------------------------------------------------------------------------------
tnode* clone_subtree( tnode* pNode )
{
	stack<tnode*> nodes;
	nodes.push( pNode );
	tnode* pClone = new_node( pNode );
	stack<tnode*> clones;
	clones.push( pClone );
	while( !nodes.empty() )
	{
		tnode* nodeIdx = nodes.top();
		tnode* cloneIdx = clones.top();
		nodes.pop();
		clones.pop();
		if( nodeIdx->next )
		{
			cloneIdx->next = new_node( nodeIdx->next );
			nodes.push( nodeIdx->next );
			clones.push( cloneIdx->next );
		}
		if( nodeIdx->right )
		{
			cloneIdx->right = new_node( nodeIdx->right );
			nodes.push( nodeIdx->right );
			clones.push( cloneIdx->right );
		}
		if( nodeIdx->left )
		{
			cloneIdx->left = new_node( nodeIdx->left );
			nodes.push( nodeIdx->left );
			clones.push( cloneIdx->left );
		}
	}
	return pClone;
}

//------------------------------------------------------------------------------
void treeListToNodeArray( tnode* pNode, std::vector<tnode*>& nodes, int tag )
{
	if( pNode->tag == tag )
	{
		// nodes.push_back( pNode->right );
		treeListToNodeArray( pNode->right, nodes, tag );
		treeListToNodeArray( pNode->left, nodes, tag );
	}
	else
		nodes.push_back( pNode );
}

//------------------------------------------------------------------------------
tnode* nodeArrayToTreeList( const std::vector<tnode*>& nodes, int tag )
{
	if( nodes.size() < 1 )
		return NULL;
	if( nodes.size() == 1 )
		return nodes[0];

	tnode* pNode = new_node( tag );
	tnode* pStart = pNode;
	for( size_t idx = nodes.size() - 1; idx > 1; --idx )
	{
		pNode->left = new_node( tag );
		pNode->right = nodes[idx];
		pNode = pNode->left;
	}
	pNode->left = nodes[0];
	pNode->right = nodes[1];
	return pStart;
}

//------------------------------------------------------------------------------
void commaListToNodeArray( tnode* pNode, std::vector<tnode*>& nodes )
{
	treeListToNodeArray( pNode, nodes, K_COMMA );
}
tnode* nodeArrayToCommaList( const std::vector<tnode*>& nodes )
{
	return nodeArrayToTreeList( nodes, K_COMMA );
}
void andListToNodeArray( tnode* pNode, std::vector<tnode*>& nodes )
{
	treeListToNodeArray( pNode, nodes, K_AND );
}
tnode* nodeArrayToAndList( const std::vector<tnode*>& nodes )
{
	return nodeArrayToTreeList( nodes, K_AND );
}

//------------------------------------------------------------------------------
tnode* find_main_node(tnode * pNode, int tag ) {
	tnode *pNodeFound;

	if ( pNode == NULL )
		return NULL;
	if ( pNode->tag == tag )
		return pNode;
	if ( ( pNodeFound = find_main_node( pNode->next, tag ) ) != NULL )
		return pNodeFound;
// 	if ( ( pNodeFound = find_main_node( pNode->left, tag ) ) != NULL )
// 		return pNodeFound;
// 	if ( ( pNodeFound = find_main_node( pNode->right, tag ) ) != NULL )
// 		return pNodeFound;
	return NULL;
}

//------------------------------------------------------------------------------
tnode* find_deeper_node(tnode * pNode, int tag, bool with_next ) {
	tnode *pNodeFound;

	if ( pNode == NULL )
		return NULL;

	//if ( with_next && (( pNodeFound = find_deeper_node( pNode->next, tag ) ) != NULL ))
	//	return pNodeFound;

 	if ( ( pNodeFound = find_deeper_node( pNode->left, tag ) ) != NULL )
 		return pNodeFound;

 	if ( ( pNodeFound = find_deeper_node( pNode->right, tag ) ) != NULL )
		return pNodeFound;

	if ( pNode->tag == tag )
		return pNode;

	return NULL;
}


void dump(const tnode * const pNode, std::ostream& os, std::string indent)
{
	os << indent << "'" << id_to_string(pNode->tag) << "' [" << pNode->tag << ", " << pNode->inf << "] : " << to_string(pNode) << std::endl;

	if (pNode->left) dump(pNode->left, os, indent + "  ");
	// else os << indent << "NULL [empty left]" << std::endl;

	if (pNode->right) dump(pNode->right, os, indent + "  ");
	// else os << indent << "NULL [empty right]" << std::endl;
	
	if (pNode->next) 
	{
		os << indent << "next ->" << std::endl;
		dump(pNode->next, os, indent);
	}
}

std::ostream& operator<<(std::ostream& os, const tnode& pNode)
{
	dump(&pNode, os);
	return os;
}