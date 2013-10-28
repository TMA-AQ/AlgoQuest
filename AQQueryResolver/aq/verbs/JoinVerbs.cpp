#include "JoinVerbs.h"
#include "VerbVisitor.h"
#include <aq/TreeUtilities.h>

namespace aq {
namespace verb {

//------------------------------------------------------------------------------
VERB_IMPLEMENT(JoinVerb);

//------------------------------------------------------------------------------
JoinVerb::JoinVerb()
{
}

//------------------------------------------------------------------------------
static void addInnerOuter(aq::tnode * j, aq::tnode * n, int tag)
{
  std::vector<std::string> tables;
  std::vector<aq::tnode*> tablesNodes;
  if (n->tag == K_IDENT)
  {
    tables.push_back(n->getData().val_str);
  }
  else
  {
    joinlistToNodeArray(n, tablesNodes);
    for (auto& n : tablesNodes)
    {
      tables.push_back(n->getData().val_str);
    }
  }
  aq::addInnerOuterNodes(j->next->left, tag, tables );
}

//------------------------------------------------------------------------------
bool JoinVerb::preprocessQuery( aq::tnode* pStart, aq::tnode* pNode, aq::tnode* pStartOriginal )
{
	assert( pNode && pStart );
	aq::tnode* pJoin = pNode;
	while( pJoin->left && pJoin->tag != K_JOIN )
		pJoin = pJoin->left;
	assert( pJoin->tag == K_JOIN );
	if( pJoin->next && pJoin->next->tag == K_ON )
	{
    addInnerOuter(pJoin, pJoin->left, this->leftTag());
    addInnerOuter(pJoin, pJoin->right, this->rightTag());

  //  std::vector<std::string> tables;
  //  std::vector<aq::tnode*> tablesNodes;
  //  
  //  if (pJoin->left->tag == K_IDENT)
  //  {
  //    tables.push_back(pJoin->left->getData().val_str);
  //  }
  //  else
  //  {
  //    joinlistToNodeArray(pJoin->left, tablesNodes);
  //    for (auto& n : tablesNodes)
  //    {
  //      tables.push_back(n->getData().val_str);
  //    }
  //  }
  //  aq::addInnerOuterNodes( pJoin->next->left, this->leftTag(), tables );

  //  tables.clear();
  //  tablesNodes.clear();
  //  if (pJoin->right->tag == K_IDENT)
  //  {
  //    tables.push_back(pJoin->left->getData().val_str);
  //  }
  //  else
  //  {
  //    joinlistToNodeArray(pJoin->right, tablesNodes);
  //    for (auto& n : tablesNodes)
  //    {
  //      tables.push_back(n->getData().val_str);
  //    }
  //  }
		//aq::addInnerOuterNodes( pJoin->next->left, this->rightTag(), tables );
		
    aq::addConditionsToWhere( pJoin->next->left, pStart );
	}
  // FIXME
	//assert( pJoin->left && (pJoin->left->tag == K_IDENT || pJoin->left->tag == K_AS && pJoin->left->left && pJoin->left->left->tag == K_IDENT ) );
	//assert( pJoin->right && (pJoin->right->tag == K_IDENT || pJoin->right->tag == K_AS && pJoin->right->left && pJoin->right->left->tag == K_IDENT ) );
	aq::tnode* table1 = pJoin->left;
	pJoin->left = NULL;
	aq::tnode* table2 = pJoin->right;
	pJoin->right = NULL;
	delete_subtree( pNode->left );
	delete_subtree( pNode->right );
	pNode->tag = K_COMMA;
	pNode->left = table1;
	pNode->right = table2;

	return false;
}

//------------------------------------------------------------------------------
int JoinVerb::leftTag(){ return K_INNER; };
int JoinVerb::rightTag(){ return K_INNER; };

//------------------------------------------------------------------------------
void JoinVerb::accept(VerbVisitor* visitor)
{
  visitor->visit(this);
}

//------------------------------------------------------------------------------
VERB_IMPLEMENT(LeftJoinVerb);

//------------------------------------------------------------------------------
LeftJoinVerb::LeftJoinVerb()
{
}

//------------------------------------------------------------------------------
int LeftJoinVerb::leftTag(){ return K_OUTER; };

//------------------------------------------------------------------------------
VERB_IMPLEMENT(RightJoinVerb);

//------------------------------------------------------------------------------
RightJoinVerb::RightJoinVerb()
{
}

//------------------------------------------------------------------------------
int RightJoinVerb::rightTag(){ return K_OUTER; };

//------------------------------------------------------------------------------
VERB_IMPLEMENT(FullJoinVerb);

//------------------------------------------------------------------------------
FullJoinVerb::FullJoinVerb()
{
}

//------------------------------------------------------------------------------
int FullJoinVerb::leftTag(){ return K_OUTER; };
int FullJoinVerb::rightTag(){ return K_OUTER; };

}
}