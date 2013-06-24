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
class tnode 
{
public:
  tnode(short _tag);
  tnode(const tnode& source);
  ~tnode();
  tnode& operator=(const tnode& source);

	tnode * left;
	tnode * right;
	tnode * next;
  tnode * parent;
	int inf; /// used by Verbs to exchange information, TODO : use a callback interface
	short tag;
  
  void setTag(short _tag) { this->tag = _tag; }
  short getTag() const { return tag; }
  tnodeDataType getDataType() const { return eNodeDataType; }
  const data_holder_t& getData() const { return data; }
  std::string to_string() const;
 
  void set_string_data(const char* pszStr);
  void append_string_data(char* pszStr);
  void to_upper();
  void set_int_data(llong nVal);
  void set_double_data(double dVal);
  void set_data(const data_holder_t data, ColumnType type);

  bool cmp(const tnode * n2) const;

private:
	tnodeDataType	eNodeDataType;
	data_holder_t data;		 /// data of the node
	size_t nStrBufCb;		/// String Buffer (data.val_str) allocated bytes
} ;

struct node_cmp_t
{
  bool operator()(const tnode * n1, const tnode * n2)
  {
    return n1->cmp(n2);
  }
};

//------------------------------------------------------------------------------
void report_error( char* pszMsg, int bExit );
tnode* clone_subtree( tnode* pNode );
void delete_subtree( tnode*& pNode );
tnode* get_leftmost_child( tnode *pNode );

//------------------------------------------------------------------------------
void treeListToNodeArray( tnode* pNode, std::vector<tnode*>& nodes, int tag );
tnode* nodeArrayToTreeList( const std::vector<tnode*>& nodes, int tag );
void commaListToNodeArray( tnode* pNode, std::vector<tnode*>& nodes );
tnode* nodeArrayToCommaList( const std::vector<tnode*>& nodes );
void andListToNodeArray( tnode* pNode, std::vector<tnode*>& nodes );
tnode* nodeArrayToAndList( const std::vector<tnode*>& nodes );

//------------------------------------------------------------------------------
void find_nodes(tnode * pNode, int tag, std::vector<tnode*>& nodes);
/// find FROM, WHERE, GROUP BY, HAVING, main ORDER BY
tnode* find_main_node( tnode *pNode, int tag );
tnode* find_first_node( tnode * pNode, int tag );
tnode* findOut_IDENT( tnode * pNode, std::string name );
tnode* find_deeper_node(tnode * pNode, int tag, bool with_next = false );
tnode* find_first_node_diffTag(tnode * pNode, int tag, int diffTag ); // find out the pNode TAG (COLUMN) with no PERIOD's parent

void treeListToNodeArraySecond( tnode* pNode, std::vector<tnode*>& nodes, int tag );
void commaListToNodeArraySecond( tnode* pNode, std::vector<tnode*>& nodes );

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