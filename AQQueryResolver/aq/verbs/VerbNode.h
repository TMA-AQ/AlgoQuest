#ifndef __VERB_NODE_H__
#define __VERB_NODE_H__

#include "Verb.h"

#include <aq/Object.h>
#include <aq/Settings.h>
#include <aq/BaseDesc.h>
#include <aq/Table.h>
#include <aq/Row.h>
#include <aq/RowProcess_Intf.h>
#include <aq/parser/SQLParser.h>
#include <boost/array.hpp>

namespace aq {
namespace verb {

// forward declaration
class VerbVisitor;

/// \brief define a verb tree
/// Each Verb node can have three childs (left, right, next), making a verb tree
class VerbNode: public Verb
{
public:
  typedef boost::intrusive_ptr<VerbNode> Ptr;

	VerbNode();
	
  virtual bool preprocessQuery(aq::tnode* pStart, aq::tnode* pNode, aq::tnode* pStartOriginal) { return false; }
  virtual bool changeQuery(aq::tnode* pStart, aq::tnode* pNode, VerbResult::Ptr resLeft, VerbResult::Ptr resRight, VerbResult::Ptr resNext) { return false; }
  virtual void changeResult(Table::Ptr table, VerbResult::Ptr resLeft, VerbResult::Ptr resRight, VerbResult::Ptr resNext) {}
  virtual void addResult (aq::Row& row) {}

  /// \brief clone a verb tree
  /// \param verbTree the verbTree to clone
  void cloneSubtree(const VerbNode::Ptr verbTree);

  /// \name getter/setter
  /// set and get childs
  /// \{
  void setLeftChild(VerbNode::Ptr _left) { this->left = _left; }
  void setRightChild(VerbNode::Ptr _right) { this->right = _right; }
  void setBrother(VerbNode::Ptr _brother) { this->brother = _brother; }
  
  VerbNode::Ptr getLeftChild() { return this->left; }
  VerbNode::Ptr getRightChild() { return this->right; }
  VerbNode::Ptr getBrother() { return this->brother; }
  /// \}

  /// \brief call change query 
  /// 
  //// childs are called in this order: brother, left then right
	void changeQuery();

  /// \brief add a result row on verb tree, and process it
  /// \param row
  /// \fixme the call order is not sure (brother, right then left
  void addResultOnChild(aq::Row& row);
	
  virtual void accept(VerbVisitor*);
  
  /// \brief apply visitor in this order : brother, left then right
  void apply(VerbVisitor*);

  /// \brief build a verb
  /// \param pStart
  /// \param pNode
  /// \param pStartOriginal
  /// \param context
  /// \param BaseDesc
  /// \param settings
	static VerbNode::Ptr build(aq::tnode* pStart, aq::tnode* pNode, aq::tnode* pStartOriginal, tnode::tag_t context, Base& BaseDesc, Settings& settings);

	/// \brief build a subtree for each major category
  /// \param pStart top node of query tree
	/// \param categories_order order in which main verb are build (the last one will be executed first !)
  /// \param baseDesc base description
  /// \param settings settings
  /// \return the subtree builded
  static VerbNode::Ptr BuildVerbsTree( aq::tnode* pStart, const boost::array<aq::tnode::tag_t, 6>& categories_order, Base& baseDesc, Settings * settings );
   
  /// \brief build a VerbNode subtree corresponding to the pStart subtree
  /// \param pSelect
  /// \param pStart
  /// \param pStartOriginal
  /// \param context
  /// \param baseDesc
  /// \param settings
  /// a branch will end when a VerbNode for that aq::tnode cannot be found
  /// top level node in the subtree will not have a brother
  static VerbNode::Ptr BuildVerbsSubtree( aq::tnode* pSelect, aq::tnode* pStart, aq::tnode* pStartOriginal, tnode::tag_t context, Base& BaseDesc, Settings * settings );

  /// \brief Debug purpose
  static void dump(std::ostream& os, VerbNode::Ptr tree, std::string ident = "");

private:
	aq::tnode* pStart; ///< top node in the query tree
	aq::tnode* pNode; ///< node to which this VerbNode corresponds
	VerbNode::Ptr left, right, brother; ///< childs
  bool toSolve; ///< indicate is the verb need to be solved
};

}
}

#endif
