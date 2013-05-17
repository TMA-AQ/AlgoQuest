#include "SQLParser.h"
#include <aq/DBTypes.h>
#include <aq/Exceptions.h>
#include "sql92_grm_tab.hpp"
#include "ID2Str.h"

#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <cassert>
#include <cerrno>
#include <stack>
#include <boost/algorithm/string.hpp>

#define STR_BUF_SIZE_ROUND_UP	4096
#define EXIT_ON_MEM_ERROR		1

extern int yyerror ( const char *pszMsg );

using namespace std;

namespace aq
{

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

tnode::tnode(short _tag)
  : 
  left(NULL),
  right(NULL),
  next(NULL),
  parent(NULL),
  inf(0),
  tag(_tag),
  eNodeDataType(NODE_DATA_INT),
  nStrBufCb(0)
{
  data.val_int = 0;
}

tnode::tnode(const tnode& source)
  :
  left(NULL),
  right(NULL),
  next(NULL),
  parent(NULL),
  inf(source.inf),
  tag(source.tag),
  eNodeDataType(NODE_DATA_INT),
  nStrBufCb(0)
{
	switch(source.eNodeDataType)
	{
	case NODE_DATA_STRING:
		set_string_data(source.data.val_str);
		break;
	case NODE_DATA_INT:
		set_int_data(source.data.val_int);
		break;
	case NODE_DATA_NUMBER:
		set_double_data(source.data.val_number);
		break;
	default:
		assert( 0 );
	}
}

tnode::~tnode()
{
}

tnode& tnode::operator=(const tnode& source)
{
  if (this != &source)
  {
    this->inf = source.inf;
    this->tag = source.tag;
    switch(source.eNodeDataType)
    {
    case NODE_DATA_STRING:
      set_string_data(source.data.val_str);
      break;
    case NODE_DATA_INT:
      set_int_data(source.data.val_int);
      break;
    case NODE_DATA_NUMBER:
      set_double_data(source.data.val_number);
      break;
    default:
      assert( 0 );
    }
  }
  return *this;
}

//------------------------------------------------------------------------------
void tnode::set_string_data(const char* pszStr) 
{
  size_t nLen;

  if ( pszStr == NULL ) 
  {
    /* Call free */
    if ((this->eNodeDataType == NODE_DATA_STRING) && (this->nStrBufCb != 0) && (this->data.val_str != NULL)) 
    {
      free( this->data.val_str );
      this->data.val_str = NULL;
      this->nStrBufCb	= 0;
    }
  }
  else
  {
    nLen = strlen( pszStr ) + 1;		/* Include terminating NULL char */

    if ((this->eNodeDataType != NODE_DATA_STRING) || (this->nStrBufCb < nLen)) 
    {
      /* Need to allocate or reallocate the data buffer */

      if (this->nStrBufCb > 0)
        this->set_string_data(NULL); /* Free Old Buffer - avoid realloc */

      /* Round up the size to 4096 multiple - avoid and on bits */
      nLen = nLen + STR_BUF_SIZE_ROUND_UP - 1;
      nLen = nLen - nLen % STR_BUF_SIZE_ROUND_UP;

      this->data.val_str = (char*)malloc( sizeof( char ) * nLen );
      if (this->data.val_str == NULL) 
      {
        report_error( "Not enough memory", EXIT_ON_MEM_ERROR );
      }
      this->nStrBufCb = nLen;				/* Set Buffer Len */
      this->eNodeDataType	= NODE_DATA_STRING; /* Set Correct Data Type */
    }

    strcpy(this->data.val_str, pszStr);
  }
}

//------------------------------------------------------------------------------
void tnode::append_string_data(char* pszStr) 
{
  size_t nLen, nLen1;

  if ((this->eNodeDataType != NODE_DATA_STRING) || (pszStr == NULL))
  {
    this->set_string_data(pszStr);
    return;
  }

  nLen = strlen(this->data.val_str);	
  if ( nLen == 0 )	/* Empty String */
  {
    this->set_string_data(pszStr);
    return;
  }

  nLen1 = strlen( pszStr );
  if ( nLen1 == 0 )	/* Empty String */
    return;	/* Nothing to do ! */

  nLen += nLen1 + 1;	/* Include terminating NULL char */

  if (this->nStrBufCb < nLen) 
  {
    /* Need to allocate or reallocate the data buffer */
    char *pszBuf;

    /* Round up the size to 4096 multiple - avoid and on bits */
    nLen = nLen + STR_BUF_SIZE_ROUND_UP - 1;
    nLen = nLen - nLen % STR_BUF_SIZE_ROUND_UP;

    pszBuf = (char*)malloc( sizeof( char ) * nLen );
    if ( pszBuf == NULL ) {
      report_error( "Not enough memory", EXIT_ON_MEM_ERROR );
      return;
    }

    strcpy( pszBuf, this->data.val_str );	/* Keep the old string */
    set_string_data( NULL );	/* Free Old Buffer - avoid realloc */
    this->data.val_str = pszBuf;	/* Set the new buffer */

    this->nStrBufCb		= nLen;				/* Set Buffer Len */
    this->eNodeDataType	= NODE_DATA_STRING; /* Set Correct Data Type */
  }

  /* Append the new string */
  strcat( this->data.val_str, pszStr );

}

//------------------------------------------------------------------------------
void tnode::set_int_data( llong nVal ) 
{
  if ( this->eNodeDataType == NODE_DATA_STRING )
			set_string_data( NULL );		/* Free the String Buffer */

  this->data.val_int	 = nVal;
  this->eNodeDataType = NODE_DATA_INT;
}

//------------------------------------------------------------------------------
void tnode::set_double_data( double dVal ) 
{
  if ( this->eNodeDataType == NODE_DATA_STRING )
    set_string_data( NULL );		/* Free the String Buffer */

  this->data.val_number	= dVal;
  this->eNodeDataType	= NODE_DATA_NUMBER;
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
void tnode::set_data( const data_holder_t data, ColumnType type )
{
	switch (type)
	{
	case COL_TYPE_INT:
	case COL_TYPE_BIG_INT:
	case COL_TYPE_DATE1:
	case COL_TYPE_DATE2:
	case COL_TYPE_DATE3:
	case COL_TYPE_DATE4:
		this->tag = K_INTEGER;
		this->set_int_data( (llong) data.val_int );
		break;
	case COL_TYPE_DOUBLE:
		this->tag = K_REAL;
		this->set_double_data( data.val_number );
		break;
	case COL_TYPE_VARCHAR:
		this->tag = K_STRING;
		this->set_string_data( data.val_str );
		break;
	default:
		break;
	}
}

//------------------------------------------------------------------------------
std::string tnode::to_string() const
{
	char szBuffer[STR_BUF_SIZE]; // tma: FIXME
	memset(szBuffer, 0, STR_BUF_SIZE);
	switch( this->eNodeDataType )
	{
	case NODE_DATA_INT: 
		sprintf(szBuffer, "%lld", this->data.val_int);
		return szBuffer;
		break;
	case NODE_DATA_NUMBER: 
		doubleToString( szBuffer, this->data.val_number );
		return szBuffer;
		break;
	case NODE_DATA_STRING:
		return this->data.val_str;
		break;
	default:
		assert( 0 );
	}
	return "";
}

//------------------------------------------------------------------------------
void tnode::to_upper()
{
  if (this->eNodeDataType == NODE_DATA_STRING) boost::to_upper(this->data.val_str);
  if (this->left) this->left->to_upper();
  if (this->right) this->right->to_upper();
  if (this->next) this->next->to_upper();
}

//------------------------------------------------------------------------------
tnode* clone_subtree(tnode* pNode)
{
	stack<tnode*> nodes;
	nodes.push(pNode);
	tnode* pClone = new tnode(*pNode);
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
			cloneIdx->next = new tnode( *nodeIdx->next );
			nodes.push( nodeIdx->next );
			clones.push( cloneIdx->next );
		}
		if( nodeIdx->right )
		{
			cloneIdx->right = new tnode( *nodeIdx->right );
			nodes.push( nodeIdx->right );
			clones.push( cloneIdx->right );
		}
		if( nodeIdx->left )
		{
			cloneIdx->left = new tnode( *nodeIdx->left );
			nodes.push( nodeIdx->left );
			clones.push( cloneIdx->left );
		}
	}
	return pClone;
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
		nodes[idx]->set_string_data( NULL );	/* Delete String Buffer if any */
		delete nodes[idx];
	}
}

//------------------------------------------------------------------------------
void treeListToNodeArray( tnode* pNode, std::vector<tnode*>& nodes, int tag )
{
	if( pNode->getTag() == tag )
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

	tnode* pNode = new tnode( tag );
	tnode* pStart = pNode;
	for( size_t idx = nodes.size() - 1; idx > 1; --idx )
	{
		pNode->left = new tnode( tag );
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
	if ( pNode->getTag() == tag )
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

	if ( pNode->getTag() == tag )
		return pNode;

	return NULL;
}

//------------------------------------------------------------------------------
void dump(const tnode * const pNode, std::ostream& os, std::string indent)
{
  if (indent.size() > 100) 
  {
      os << indent << "..."; 
      return;
  }

	os << indent << "'" << id_to_kstring(pNode->getTag()) << "' [" << pNode->getTag() << ", " << pNode->inf << "] : " << pNode->to_string() << " [address:" << pNode << "] " << std::endl;

  if ((pNode->left != NULL) || (pNode->right != NULL))
  {
    if (pNode->left) dump(pNode->left, os, indent + "  ");
    else os << indent << "  NULL [empty left]" << std::endl;

    if (pNode->right) dump(pNode->right, os, indent + "  ");
    else os << indent << "  NULL [empty right]" << std::endl;
  }
	
	if (pNode->next) 
	{
		os << indent << "next ->" << std::endl;
		dump(pNode->next, os, indent);
	}
}

//------------------------------------------------------------------------------
std::ostream& operator<<(std::ostream& os, const tnode& pNode)
{
	dump(&pNode, os);
	return os;
}

//------------------------------------------------------------------------------
void checkTree( tnode * tree, std::set<tnode*>& nodes)
{

  if (tree == NULL) return;

  std::set<tnode*>::iterator it = nodes.find(tree);
  assert(it == nodes.end());
  if (it != nodes.end())
  {
    throw aq::generic_error(aq::generic_error::INVALID_QUERY, "recurse in tnode structure is not allowed");
  }
  nodes.insert(tree);

  switch (tree->getDataType())
  {
  case NODE_DATA_INT: break;
  case NODE_DATA_NUMBER: break;
  case NODE_DATA_STRING: 
    //assert(tree->data.val_str != NULL); 
    //assert((tree->nStrBufCb % STR_BUF_SIZE_ROUND_UP) == 0);
    //assert(tree->nStrBufCb >= strlen(tree->data.val_str));
    break;
  }

  checkTree(tree->left, nodes);
  checkTree(tree->right, nodes);
  checkTree(tree->next, nodes);

}

}