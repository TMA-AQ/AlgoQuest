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
 
class tnode;

/// Returns 0 on success, 1 on error
/// Returns in *ppNode the created sintax tree
extern int SQLParse(const char *pszStr, aq::tnode*&ppNode); // define in sql92_Grm.y

//------------------------------------------------------------------------------
class tnode 
{
public:
  static std::string indentStep;
  typedef int tag_t; // FIXME

  enum tnodeDataType
  {
    NODE_DATA_STRING,
    NODE_DATA_INT,
    NODE_DATA_NUMBER
  };

public:
  tnode(tag_t _tag);
  tnode(const tnode& source);
  ~tnode();
  tnode& operator=(const tnode& source);

	tnode * left;
	tnode * right;
	tnode * next;
  tnode * parent;
	int inf; /// used by Verbs to exchange information, TODO : use a callback interface
	tag_t tag;
  
  void setTag(tag_t _tag) { this->tag = _tag; }
  tag_t getTag() const { return tag; }
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
  
  tnode* clone_subtree();
  
  tnode* find_main_node(tag_t tag); ///< find FROM, WHERE, GROUP BY, HAVING, main ORDER BY
  tnode* find_first_node(tag_t tag);
  tnode* findOut_IDENT(const std::string& name);
  tnode* find_deeper_node(tag_t tag);
  tnode* find_first_node_diffTag(tag_t tag, int diffTag); ///< find out the pNode TAG (COLUMN) with no PERIOD's parent
  void find_nodes(tag_t tag, std::vector<tnode*>& nodes);

  void treeListToNodeArray(std::vector<tnode*>& nodes, tag_t tag);
  void joinlistToNodeArray(std::vector<tnode*>& nodes);
  
  void dump(std::ostream& os, std::string indent = "") const;

  // static methods
  static tnode* get_leftmost_child(tnode * pNode);
  static tnode* nodeArrayToTreeList(const std::vector<tnode*>& nodes, tag_t tag);
  static void checkTree(tnode * tree, std::set<tnode*>& nodes);
  static void delete_subtree(tnode *& pNode);

  static bool isJoinTag(tag_t tag);

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

inline std::ostream& operator<<(std::ostream& os, const tnode& pNode)
{
	pNode.dump(os);
	return os;
}

}

#endif /* __AQ_SQLPARSER_H__ */
