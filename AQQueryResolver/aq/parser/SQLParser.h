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


// forward declaration
class tnode;

/// \brief Parse SQL query
/// \param query the sql query string representation
/// \pnode pNode the 
/// \return 0 on success, 1 on error
int SQLParse(const char * query, aq::tnode *& pNode); // define in sql92_Grm.y

/// \brief structure to hold sql query
///
/// structure holding the sql query after bison/flex parsing
class tnode 
{
public:
  static std::string indentStep;
  typedef unsigned int tag_t; // FIXME

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

	tnode * left; ///< left child
	tnode * right; ///< right child
	tnode * next; ///< next child
  tnode * parent; ///< parent (must be generated)
	int inf; ///< used by Verbs to exchange information, \todo use a callback interface \deprecated
	tag_t tag; ///< tag's node (see bison parser tag)
  
  /// \name getter/setter
  /// use to set/get/update value of current node
  /// \{
  
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
  
  /// \}

  /// \brief Compare the value of n2 with this. Children are not compared.
  bool cmp(const tnode * n2) const;
  
  /// \brief clone to a new tnode tree
  tnode * clone_subtree() const;
  
  /// \brief Get a list of tnode separate by tag
  /// \param nodes the output list of tnodes
  /// \param tag the matching tag
  void treeListToNodeArray(std::vector<tnode*>& nodes, tag_t tag) const;
  
  /// \brief Get a list of join tnode
  /// \param nodes the output list of tnodes
  void joinlistToNodeArray(std::vector<tnode*>& nodes) const;
   
  /// \brief Deep First Search
  /// \param cmp
  /// \param withNext
  /// \return first node matching the functor
  template <class CMP> tnode * find_DFS(CMP& cmp, bool withNext = false);

  /// \brief Breadth First Search
  /// \param cmp
  /// \param withNext
  /// \return first node matchin the functor
  template <class CMP> tnode * find_BFS(CMP& cmp, bool withNext = false);

  /// \brief find SELECT, FROM, WHERE, GROUP BY, HAVING, ORDER BY
  /// \param tag the tag to match
  /// \return the matching node
  /// the search is only made on next child
  tnode * find_main(tag_t tag);

  /// \brief find the first node by BFS (Breadth First Search). next child is omitted
  /// \param tag the tag to match
  /// \return the matching node
  tnode * find_first(tag_t tag);

  /// \brief find the first node by BFS (Breadth First Search). next child is omitted
  /// \param value the value of the node to return
  /// \return the matching node
  /// the searching node is of type NODE_DATA_STRING
  tnode * find_first(const std::string& value);

  /// \brief find the deepest node matchin tag (Deep First Search)
  /// \param tag the tag to match
  /// \return the matching node
  tnode * find_deeper(tag_t tag);

  /// \brief find the first node matching tag wiich has a parent unmatching diff tag
  /// \param tag the tag to match
  /// \param diff the parent tag to unmatch
  /// \return the matching node
  tnode * find_first(tag_t tag, tag_t diff);

  /// \brief find all nodes matching tag, and put them in a vector
  /// \param tag
  /// \param nodes the list of all matching nodes
  /// if nodes is not empty before the call, nodes are added to the end)
  void find_nodes(tag_t tag, std::vector<tnode*>& nodes);
  
  const tnode * find_main(tag_t tag) const { return const_cast<tnode*>(this)->find_main(tag); }
  const tnode * find_first(tag_t tag) const { return const_cast<tnode*>(this)->find_first(tag); }
  const tnode * find_first(const std::string& name) const { return const_cast<tnode*>(this)->find_first(name); }
  const tnode * find_deeper(tag_t tag) const { return const_cast<tnode*>(this)->find_deeper(tag); }
  const tnode * find_first(tag_t tag, tag_t diffTag) const { return const_cast<tnode*>(this)->find_first(tag, diffTag); }
  void find_nodes(tag_t tag, std::vector<const tnode*>& nodes) const;

  /// \brief check is node represent a column
  /// \return return true is node refer to a column 
  bool isColumnReference() const;

  /// \brief dump tree. debug purpose
  /// \param os output stream
  /// \param indent current indentation
  void dump(std::ostream& os, std::string indent = "") const;

  /// \brief todo
  /// \param pNode
  /// \return todo
  static tnode* get_leftmost_child(tnode * pNode);

  /// \brief transform a stl container to a tree list tnode join by tag
  /// \param nodes the stl container to proceed
  /// \param tag the join tag
  /// \return the tree node list
  template <class C> 
  static tnode* nodeArrayToTreeList(const C& nodes, tag_t tag);

  /// \brief check if tree is valid
  /// \param tree the tree to check
  static void checkTree(const tnode * tree);

  /// \brief delete a tnode tree
  /// \param pNode the tree to delete
  /// if succeed pNode has value nullptr
  static void delete_subtree(tnode *& pNode);

  /// \defgroup tag_classification tag classification
  /// \{
  static bool isMainTag(tag_t tag); ///< \return true is tag is a main tag
  static bool isJoinTag(tag_t tag); ///< \return true is tag is a join tag
  /// \}

private:
	tnodeDataType	eNodeDataType; ///< data type of the node
	data_holder_t data; ///< data of the node
	size_t nStrBufCb; ///< string Buffer (data.val_str) allocated bytes
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

template <class CMP>
tnode * tnode::find_DFS(CMP& cmp, bool withNext)
{
  if ((cmp)(this))
		return this;
  
 	if ((this->left != nullptr) && ((pNodeFound = this->left->find_DFS(cmp)) != nullptr))
 		return pNodeFound;

 	if ((this->right != nullptr) && ((pNodeFound = this->right->find_DFS(cmp)) != nullptr))
		return pNodeFound;
  
  tnode * pNodeFound = nullptr;

 	if (withNext && (this->next != nullptr) && ((pNodeFound = this->next->find_DFS(cmp)) != nullptr))
		return pNodeFound;

	return nullptr;
}

template <class CMP>
tnode * tnode::find_BFS(CMP& cmp, bool withNext)
{
  if ((cmp)(this))
		return this;
  
  tnode * pNodeFound = nullptr;

 	if ((this->left != nullptr) && ((pNodeFound = this->left->find_BFS(cmp)) != nullptr))
 		return pNodeFound;

 	if ((this->right != nullptr) && ((pNodeFound = this->right->find_BFS(cmp)) != nullptr))
		return pNodeFound;
  
 	if (withNext && (this->next != nullptr) && ((pNodeFound = this->next->find_BFS(cmp)) != nullptr))
		return pNodeFound;

	return nullptr;
}

struct node_cmp_tag_t
{
  node_cmp_tag_t(tnode::tag_t _tag) : tag(_tag) {}
  bool operator()(const tnode * n)
  {
    return n->getTag() == tag;
  }
  tnode::tag_t tag;
};

struct node_cmp_name_t
{
  node_cmp_name_t(std::string _name) : name(_name) {}
  bool operator()(const tnode * n)
  {
    // return (((n->getTag() == K_COLUMN) || (n->getTag() == K_IDENT)) && (n->getData().val_str == name));
    return true; // TODO
  }
  std::string name;
};

struct node_cmp_diff_tag_t
{
  node_cmp_diff_tag_t(tnode::tag_t _tag, tnode::tag_t _diff) : tag(_tag), diff(_diff) {}
  bool operator()(const tnode * n)
  {
    return (n->getTag() == tag) && n->parent && (n->parent->getTag() != diff);
  }
  tnode::tag_t tag;
  tnode::tag_t diff;
};

//------------------------------------------------------------------------------
template <class C>
tnode* tnode::nodeArrayToTreeList(const C& nodes, tag_t tag)
{
	if (nodes.empty())
		return nullptr;
	if( nodes.size() == 1 )
		return *nodes.begin();

	tnode * pNode = new tnode(tag);
	tnode * pStart = pNode;
	for (size_t idx = nodes.size() - 1; idx > 1; --idx)
	{
		pNode->left = new tnode(tag);
		pNode->right = nodes[idx];
		pNode = pNode->left;
	}
	pNode->left = nodes[0];
	pNode->right = nodes[1];
	return pStart;
}

}

#endif /* __AQ_SQLPARSER_H__ */
