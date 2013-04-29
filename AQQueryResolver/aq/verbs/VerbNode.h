#ifndef __VERB_NODE_H__
#define __VERB_NODE_H__

#include "Verb.h"

#include <aq/Object.h>
#include <aq/Settings.h>
#include <aq/BaseDesc.h>
#include <aq/Table.h>
#include <aq/RowProcess_Intf.h>
#include <aq/parser/SQLParser.h>

// forward declaration
class VerbVisitor;

//--------------------------------------------------------------------------
class VerbNode: public Verb
{
	OBJECT_DECLARE( VerbNode );
public:
	VerbNode();
	
  /// Verb interface default implementation
  virtual bool preprocessQuery( tnode* pStart, tnode* pNode, tnode* pStartOriginal ) { return false; }
  virtual bool changeQuery( tnode* pStart, tnode* pNode, VerbResult::Ptr resLeft, VerbResult::Ptr resRight, VerbResult::Ptr resNext ) { return false; }
  virtual void changeResult( Table::Ptr table, VerbResult::Ptr resLeft, VerbResult::Ptr resRight, VerbResult::Ptr resNext ) {}
  virtual void addResult ( aq::RowProcess_Intf::Row& row, VerbResult::Ptr resLeft, VerbResult::Ptr resRight, VerbResult::Ptr resNext ) {}

  virtual VerbNode* clone() const = 0;

	void setLeftChild( VerbNode::Ptr child );
	void setRightChild( VerbNode::Ptr child );
	void setBrother( VerbNode::Ptr brother );

	//true - executed and done (the node should be deleted)
	void changeQuery();
	void changeResult( Table::Ptr table );
  void addResult(aq::RowProcess_Intf::Row& row);
	
  virtual void accept(VerbVisitor*);
	//void acceptTopLeftRight(VerbVisitor*);
 // void acceptLeftTopRight(VerbVisitor* visitor);

	// Verb::Ptr getVerbObject();
	VerbNode::Ptr getLeftChild();
	VerbNode::Ptr getRightChild();
	VerbNode::Ptr getBrother();
  
  bool isToSolved() const { return this->toSolve; }

	static VerbNode::Ptr build( tnode* pStart, tnode* pNode, tnode* pStartOriginal, int context, Base& BaseDesc, TProjectSettings& settings );

	/// build a subtree for each major category
	/// order is given by \a categories_order (the last one will be executed first)
	/// engine actually executes GROUP BY before select and after where, but
	/// I need to delete it after select gets the grouping columns
  static VerbNode::Ptr BuildVerbsTree( tnode* pStart, const std::vector<unsigned int>& categories_order, Base& baseDesc, TProjectSettings * settings );
   
  /// build a VerbNode subtree corresponding to the ppStart subtree
  /// a branch will end when a VerbNode for that tnode cannot be found
  /// top level node in the subtree will not have a brother
  static VerbNode::Ptr BuildVerbsSubtree( tnode* pSelect, tnode* pStart, tnode* pStartOriginal, int context, Base& BaseDesc, TProjectSettings *pSettings );

  /// Debug purpose
  static void dump(std::ostream& os, VerbNode::Ptr tree, std::string ident = "");

private:
	// Verb::Ptr VerbObject;
	tnode* pStart; //top node in the query tree
	tnode* pNode; //tnode to which this VerbNode corresponds
	std::vector<VerbNode*> Parents;
	VerbNode::Ptr Left, Right, Brother;
  bool toSolve;
};

#include "VerbFactory.h"

#endif