#ifndef __AQ_SQLPARSER_H__
#define __AQ_SQLPARSER_H__

//------------------------------------------------------------------------------
#include <aq/Utilities.h>
#include <aq/DBTypes.h>
#include <set>
#include <vector>
#include <string>
#include <sstream>

namespace aq
{

//------------------------------------------------------------------------------
typedef enum {
	NODE_DATA_STRING,
	NODE_DATA_INT,
	NODE_DATA_NUMBER
} tnodeDataType;

//------------------------------------------------------------------------------
struct tnode 
{
	short tag;
	tnodeDataType	eNodeDataType;
	data_holder_t data;		 /// data of the node
	size_t nStrBufCb;		/// String Buffer (data.val_str) allocated bytes
	int inf; /// used by Verbs to exchange information, TODO : use a callback interface

	struct tnode * left;
	struct tnode * right;
	struct tnode * next;
  struct tnode * parent;

} ;

//------------------------------------------------------------------------------
void report_error( char* pszMsg, int bExit );
tnode* new_node( short tag );
tnode* new_node( tnode* pNode );
tnode* set_string_data( tnode* pNode, const char* pszStr );
tnode* append_string_data( tnode* pNode, char* pszStr );
tnode* set_int_data( tnode* pNode, llong nVal );
tnode* set_double_data( tnode* pNode, double dVal );
tnode* delete_node( tnode* pNode );
void delete_subtree( tnode* pNode );
tnode* get_leftmost_child( tnode *pNode );
tnode& set_data( tnode& pNode, const data_holder_t data, ColumnType type );
std::string to_string(const tnode* const pNode );
tnode* clone_subtree( tnode* pNode );
void to_upper(tnode* pNode);

//------------------------------------------------------------------------------
void treeListToNodeArray( tnode* pNode, std::vector<tnode*>& nodes, int tag );
tnode* nodeArrayToTreeList( const std::vector<tnode*>& nodes, int tag );
void commaListToNodeArray( tnode* pNode, std::vector<tnode*>& nodes );
tnode* nodeArrayToCommaList( const std::vector<tnode*>& nodes );
void andListToNodeArray( tnode* pNode, std::vector<tnode*>& nodes );
tnode* nodeArrayToAndList( const std::vector<tnode*>& nodes );

//find FROM, WHERE, GROUP BY, HAVING, main ORDER BY
tnode* find_main_node( tnode *pNode, int tag );
tnode* find_deeper_node(tnode * pNode, int tag, bool with_next = false );

// dump tnode from left to rigth
// debug purpose
void dump(const tnode * const pNode, std::ostream& os, std::string indent = "");
std::ostream& operator<<(std::ostream& os, const tnode& pNode);

void checkTree( tnode * tree, std::set<tnode*>& nodes);

}

//------------------------------------------------------------------------------
/// Returns 0 on success, 1 on error
/// Returns in *ppNode the created sintax tree
int SQLParse( const char *pszStr, aq::tnode** ppNode );

#endif /* __AQ_SQLPARSER_H__ */