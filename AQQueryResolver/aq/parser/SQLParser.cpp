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

std::string tnode::indentStep("    ");

tnode::tnode(tag_t _tag)
  : 
  left(nullptr),
  right(nullptr),
  next(nullptr),
  parent(nullptr),
  inf(0),
  tag(_tag),
  eNodeDataType(NODE_DATA_INT),
  nStrBufCb(0)
{
  data.val_int = 0;
}

tnode::tnode(const tnode& source)
  :
  left(nullptr),
  right(nullptr),
  next(nullptr),
  parent(nullptr),
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

  if ( pszStr == nullptr ) 
  {
    /* Call free */
    if ((this->eNodeDataType == NODE_DATA_STRING) && (this->nStrBufCb != 0) && (this->data.val_str != nullptr)) 
    {
      free( this->data.val_str );
      this->data.val_str = nullptr;
      this->nStrBufCb	= 0;
    }
  }
  else
  {
    nLen = strlen( pszStr ) + 1;		/* Include terminating nullptr char */

    if ((this->eNodeDataType != NODE_DATA_STRING) || (this->nStrBufCb < nLen)) 
    {
      /* Need to allocate or reallocate the data buffer */

      if (this->nStrBufCb > 0)
        this->set_string_data(nullptr); /* Free Old Buffer - avoid realloc */

      /* Round up the size to 4096 multiple - avoid and on bits */
      nLen = nLen + STR_BUF_SIZE_ROUND_UP - 1;
      nLen = nLen - nLen % STR_BUF_SIZE_ROUND_UP;

      this->data.val_str = (char*)malloc( sizeof( char ) * nLen );
      if (this->data.val_str == nullptr) 
      {
        throw aq::generic_error(aq::generic_error::GENERIC, "Not enough memory [%u]", EXIT_ON_MEM_ERROR);
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

  if ((this->eNodeDataType != NODE_DATA_STRING) || (pszStr == nullptr))
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

  nLen += nLen1 + 1;	/* Include terminating nullptr char */

  if (this->nStrBufCb < nLen) 
  {
    /* Need to allocate or reallocate the data buffer */
    char *pszBuf;

    /* Round up the size to 4096 multiple - avoid and on bits */
    nLen = nLen + STR_BUF_SIZE_ROUND_UP - 1;
    nLen = nLen - nLen % STR_BUF_SIZE_ROUND_UP;

    pszBuf = (char*)malloc( sizeof( char ) * nLen );
    if ( pszBuf == nullptr ) {
      throw aq::generic_error(aq::generic_error::GENERIC, "Not enough memory [%u]", EXIT_ON_MEM_ERROR);
    }

    strcpy( pszBuf, this->data.val_str );	/* Keep the old string */
    set_string_data( nullptr );	/* Free Old Buffer - avoid realloc */
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
			set_string_data( nullptr );		/* Free the String Buffer */

  this->data.val_int	 = nVal;
  this->eNodeDataType = NODE_DATA_INT;
  this->nStrBufCb = 0;
}

//------------------------------------------------------------------------------
void tnode::set_double_data( double dVal ) 
{
  if ( this->eNodeDataType == NODE_DATA_STRING )
    set_string_data( nullptr );		/* Free the String Buffer */

  this->data.val_number	= dVal;
  this->eNodeDataType	= NODE_DATA_NUMBER;
  this->nStrBufCb = 0;
}

//------------------------------------------------------------------------------
void tnode::set_data( const data_holder_t data, ColumnType type )
{
	switch (type)
	{
	case COL_TYPE_INT:
	case COL_TYPE_BIG_INT:
	case COL_TYPE_DATE:
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
    assert(this->data.val_str);
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
  if ((this->eNodeDataType == NODE_DATA_STRING) && (this->tag != K_STRING)) boost::to_upper(this->data.val_str);
  if (this->left) this->left->to_upper();
  if (this->right) this->right->to_upper();
  if (this->next) this->next->to_upper();
}

//------------------------------------------------------------------------------
bool tnode::cmp(const tnode * n2) const
{
  if (this == n2) return true;
  if ((this->tag == n2->tag) && (this->eNodeDataType == n2->eNodeDataType))
  {
    switch (this->eNodeDataType)
    {
    case NODE_DATA_INT:
      return this->data.val_int == n2->data.val_int;
    case NODE_DATA_NUMBER:
      return this->data.val_number == n2->data.val_number;
    case NODE_DATA_STRING:
      return strcmp(this->data.val_str, n2->data.val_str) == 0; 
    }
  }
  return false;
}

//------------------------------------------------------------------------------
tnode* tnode::clone_subtree()
{
	stack<tnode*> nodes;
	nodes.push(this);
	tnode* pClone = new tnode(*this);
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
void tnode::treeListToNodeArray(std::vector<tnode*>& nodes, tag_t tag)
{
	if (this->getTag() == tag)
	{
		this->right->treeListToNodeArray(nodes, tag);
		this->left->treeListToNodeArray(nodes, tag);
	}
	else
  {
		nodes.push_back(this->clone_subtree());
  }
}

//------------------------------------------------------------------------------
void tnode::joinlistToNodeArray(std::vector<tnode*>& nodes)
{
  aq::tnode * join = this->find_first_node(K_JOIN);
  aq::tnode * join2 = nullptr;
  while (join != nullptr)
  {
    nodes.push_back(join->right->clone_subtree());
    if ((join2 = join->left->find_deeper_node(K_JOIN)) == nullptr)
    {
      nodes.push_back(join->left->clone_subtree());
    }
    join = join2;
  }
}

//------------------------------------------------------------------------------
void tnode::find_nodes(tag_t tag, std::vector<tnode*>& l)
{
  if (this->getTag() == tag)
    l.push_back(this);
  if (this->left != nullptr) 
    this->left->find_nodes(tag, l);
  if (this->right != nullptr)
    this->right->find_nodes(tag, l);
  if (this->next != nullptr)
    this->next->find_nodes(tag, l);
}

//------------------------------------------------------------------------------
tnode* tnode::find_main_node(tag_t tag)
{
	if (this->getTag() == tag)
		return this;
	if (this->next != nullptr) 
    return this->next->find_main_node(tag);
	return nullptr;
}

//------------------------------------------------------------------------------
tnode* tnode::find_first_node(tag_t tag)
{	
  tnode * pNodeFound = nullptr;
	if (this->getTag() == tag)
		return this;

 	if ((this->left != nullptr) && ((pNodeFound = this->left->find_first_node(tag)) != nullptr))
 		return pNodeFound;

 	if ((this->right != nullptr) && ((pNodeFound = this->right->find_first_node(tag)) != nullptr))
		return pNodeFound;

	return nullptr;
}
//------------------------------------------------------------------------------
tnode* tnode::findOut_IDENT(const std::string& name)
{	
  tnode * pNodeFound = nullptr;

	if ((this->getTag() == K_COLUMN || this->getTag() == K_IDENT) && this->getData().val_str == name)
		return this;

 	if ((this->left != nullptr) && ((pNodeFound = this->left->findOut_IDENT(name)) != nullptr))
 		return pNodeFound;

 	if ((this->right != nullptr) && ((pNodeFound = this->right->findOut_IDENT(name)) != nullptr))
		return pNodeFound;

	return nullptr;
}
//------------------------------------------------------------------------------
tnode* tnode::find_first_node_diffTag(tag_t tag, tnode::tag_t diffTag)
{	
  tnode * pNodeFound = nullptr;
  
  if (this->getTag() == tag && this->parent && this->parent->getTag() != diffTag)
		return this;

 	if ((this->left != nullptr) && ((pNodeFound = this->left->find_first_node_diffTag(tag, diffTag)) != nullptr))
 		return pNodeFound;

 	if ((this->right != nullptr) && ((pNodeFound = this->right->find_first_node_diffTag(tag, diffTag)) != nullptr))
		return pNodeFound;

	return nullptr;
}
//------------------------------------------------------------------------------
tnode* tnode::find_deeper_node(tag_t tag) 
{
	tnode * pNodeFound = nullptr;

 	if ((this->left != nullptr) && ((pNodeFound = this->left->find_deeper_node(tag)) != nullptr))
 		return pNodeFound;

 	if ((this->right != nullptr) && ((pNodeFound = this->right->find_deeper_node(tag)) != nullptr))
		return pNodeFound;

	if (this->getTag() == tag)
		return this;

	return nullptr;
}

//------------------------------------------------------------------------------
void tnode::dump(std::ostream& os, std::string indent) const
{
  if (indent.size() > 100) 
  {
      os << indent << "..."; 
      return;
  }

	os << "[address:" << this << "] " << indent << id_to_kstring(this->getTag()) << " [" << this->getTag() << "] : " << this->to_string() << std::endl;

  if ((this->left != nullptr) || (this->right != nullptr))
  {
    if (this->left != nullptr) 
      this->left->dump(os, indent + tnode::indentStep);
    else 
      os << "[address:" << (void*)0 << "] " << indent << tnode::indentStep << "[EMPTY LEFT]" << std::endl;

    if (this->right != nullptr) 
      this->right->dump(os, indent + "    ");
    else
      os << "[address:" << (void*)0 << "] " << indent << tnode::indentStep << "[EMPTY RIGHT]" << std::endl;
  }
	
	if (this->next != nullptr) 
	{
		os << "[address:" << this->next << "] " << indent << "NEXT ->" << std::endl;
		this->next->dump(os, indent);
	}
}

//------------------------------------------------------------------------------
//------------------------------------------------------------------------------

// STATIC METHODS

//------------------------------------------------------------------------------
tnode* tnode::get_leftmost_child(tnode * pNode) {
	if ( pNode == nullptr )
		return nullptr;
	if ( pNode->left != nullptr )
		return get_leftmost_child( pNode->left );
	else if ( pNode->right != nullptr )
		return get_leftmost_child( pNode->right );
	return pNode;
}

//------------------------------------------------------------------------------
void tnode::checkTree(tnode * tree, std::set<tnode*>& nodes)
{

  if (tree == nullptr) return;

  std::set<tnode*>::iterator it = nodes.find(tree);
  assert(it == nodes.end());
  if (it != nodes.end())
  {
    throw aq::generic_error(aq::generic_error::INVALID_QUERY, "recurse in tnode structure is not allowed");
  }
  nodes.insert(tree);

  switch (tree->getDataType())
  {
  case aq::tnode::tnodeDataType::NODE_DATA_INT: break;
  case aq::tnode::tnodeDataType::NODE_DATA_NUMBER: break;
  case aq::tnode::tnodeDataType::NODE_DATA_STRING: 
    //assert(tree->data.val_str != nullptr); 
    //assert((tree->nStrBufCb % STR_BUF_SIZE_ROUND_UP) == 0);
    //assert(tree->nStrBufCb >= strlen(tree->data.val_str));
    break;
  }

  checkTree(tree->left, nodes);
  checkTree(tree->right, nodes);
  checkTree(tree->next, nodes);

}

//------------------------------------------------------------------------------
tnode* tnode::nodeArrayToTreeList( const std::vector<tnode*>& nodes, tag_t tag )
{
	if( nodes.size() < 1 )
		return nullptr;
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
void tnode::delete_subtree(tnode *& pNode) 
{
  if (pNode == nullptr)
    return;
	deque<tnode*> nodes;
	nodes.push_back(pNode);
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
		nodes[idx]->set_string_data(nullptr);	/* Delete String Buffer if any */
		delete nodes[idx];
	}
  pNode = nullptr;
}

//------------------------------------------------------------------------------
bool tnode::isJoinTag(tag_t tag)
{
  return tag == K_JAUTO || tag == K_JEQ || tag == K_JIEQ || tag == K_JINF || tag == K_JNEQ || tag == K_JNEQ || tag == K_JNO || tag == K_JSEQ || tag == K_JSUP;
}

}
