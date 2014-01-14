#include "VerbNode.h"
#include "VerbFactory.h"
#include "VerbVisitor.h"
#include <aq/parser/ID2Str.h>

namespace aq {
namespace verb {

//------------------------------------------------------------------------------
VerbNode::VerbNode(): pStart(nullptr), pNode(nullptr)
{	
}

//------------------------------------------------------------------------------
void VerbNode::cloneSubtree(VerbNode::Ptr v)
{
  if (v->getLeftChild())
  {
    this->left = aq::verb::VerbFactory::GetInstance().getVerb(v->getLeftChild()->getVerbType());
    this->left->cloneSubtree(v->getLeftChild());
  }
  if (v->getRightChild())
  {
    this->right = aq::verb::VerbFactory::GetInstance().getVerb(v->getRightChild()->getVerbType());
    this->right->cloneSubtree(v->getRightChild());
  }
  if (v->getBrother())
  {
    this->brother = aq::verb::VerbFactory::GetInstance().getVerb(v->getBrother()->getVerbType());
    this->brother->cloneSubtree(v->getBrother());
  }
}

//------------------------------------------------------------------------------
void VerbNode::changeQuery()
{
	if( this->brother )
		this->brother->changeQuery();
	if( this->left )
		this->left->changeQuery();
	if( this->right)
		this->right->changeQuery();
	
	if( pNode->inf == 1 )
		return;
	
	VerbResult::Ptr param1 = this->left ? this->left->getResult() : nullptr;
	VerbResult::Ptr param2 = this->right ? this->right->getResult() : nullptr;
	VerbResult::Ptr param3 = this->brother ? this->brother->getResult() : nullptr;
	if( this->changeQuery( this->pStart, this->pNode, param1, param2, param3 ) )
	{
		assert( this->pNode );
		this->pNode->inf = 1;
		this->disable();
	}
	
}

//------------------------------------------------------------------------------
void VerbNode::addResultOnChild(aq::Row& row)
{
	if( this->brother )
		this->brother->addResultOnChild( row );

	if( this->isDisabled() )
		return;

	// right first because partition by modifies the column pointers in table
	// and it should be executed before order by collects his list of column pointers
	if( this->right )
		this->right->addResultOnChild( row );
	if( this->left )
		this->left->addResultOnChild( row );

	this->addResult( row );
  
}

void VerbNode::accept(VerbVisitor* visitor)
{
  visitor->visit(this);
}

void VerbNode::apply(VerbVisitor* visitor)
{
  if (this->brother) this->brother->apply(visitor);
  if (this->left) this->left->apply(visitor);
  if (this->right) this->right->apply(visitor);
  this->accept(visitor);
}

//------------------------------------------------------------------------------
// helper struct for BuildVerbsTree
struct tnodeVerbNode
{
	tnodeVerbNode( aq::tnode* pNode, VerbNode::Ptr spNode ): 
		pNode(pNode), spNode(spNode){}
	aq::tnode* pNode;
	VerbNode::Ptr spNode;
};

//------------------------------------------------------------------------------
VerbNode::Ptr VerbNode::build(aq::tnode* pStart, aq::tnode* pNode, aq::tnode* pStartOriginal, tnode::tag_t context, Base& BaseDesc, Settings& settings)
{
  aq::Logger::getInstance().log(AQ_DEBUG, "build verb '%s'\n", id_to_kstring(pNode->tag));
	VerbNode::Ptr verb = VerbFactory::GetInstance().getVerb( pNode->tag );
  
	if( !verb )
  {
		// throw aq::generic_error(aq::generic_error::NOT_IMPLEMENTED, "Verb Not implemented");
    aq::Logger::getInstance().log(AQ_DEBUG, "Verb '%s' Not implemented\n", id_to_kstring(pNode->tag));
    return verb;
  }

  verb->pStart = pStart;
  verb->pNode = pNode;

	if( !pStart || !pNode )
  {
    verb->toSolve = false;
  }
  else if( pNode->inf == 1 ) 
  {
    //when a subtree is copied from a nested select into
		//the outer select, it may have nodes who have inf set to 1
		//do not bother creating verbs for these nodes at all, they are considered
		//solved by the engine
    verb->toSolve = false;
  }
  else 
  {
    verb->setContext( context );
    verb->setBaseDesc(&BaseDesc);
    verb->setSettings(&settings);

    if( verb->preprocessQuery( pStart, pNode, pStartOriginal ) )
    {
      pNode->inf = 1;
      verb->toSolve = false;
    }
    else
    {
      verb->toSolve = true;
    }

  }

	return verb;
}

//------------------------------------------------------------------------------
VerbNode::Ptr VerbNode::BuildVerbsTree(aq::tnode* pStart, const boost::array<aq::tnode::tag_t, 6>& categories_order, Base& baseDesc, Settings * settings)
{
  aq::Logger::getInstance().log(AQ_DEBUG, "build verb tree\n");
	if( pStart->tag != K_SELECT )
		throw 0; // TODO

	aq::tnode* pStartOriginal = pStart->clone_subtree();
	
	VerbNode::Ptr spLast = nullptr;
	for( size_t idx = 0; idx < categories_order.size(); ++idx )
	{
		aq::tnode* pNode = pStart;
		while( pNode && (pNode->tag != categories_order[idx]) )
			pNode = pNode->next;
		if( !pNode )
			continue;
		
		VerbNode::Ptr spNode = VerbNode::BuildVerbsSubtree( pStart, pNode, pStartOriginal, categories_order[idx], baseDesc, settings );
		spNode->setBrother( spLast );
		spLast = spNode;
	}
	aq::tnode::delete_subtree(pStartOriginal);
	return spLast;
}

//------------------------------------------------------------------------------
VerbNode::Ptr VerbNode::BuildVerbsSubtree(aq::tnode* pSelect, aq::tnode* pStart, aq::tnode* pStartOriginal, tnode::tag_t context, Base& BaseDesc, Settings *pSettings)
{
	VerbNode::Ptr spStart = VerbNode::build(pSelect, pStart, pStartOriginal, context, BaseDesc, *pSettings);
	if( !spStart || !spStart->toSolve )
		return spStart;
	std::deque<tnodeVerbNode> deq;
	deq.push_back( tnodeVerbNode(pStart, spStart) );

	while( deq.size() > 0 )
	{
		tnodeVerbNode& currentnode = deq[0];
    aq::Logger::getInstance().log(AQ_DEBUG, "%u\n", currentnode.pNode->getTag());

		//next
		if( currentnode.pNode != pStart )
		{
			aq::tnode* pNext = currentnode.pNode->next;
			if( pNext )
			{
				VerbNode::Ptr spNewVerbNode = VerbNode::build(pSelect, pNext, pStartOriginal, context, BaseDesc, *pSettings);
				if( spNewVerbNode && spNewVerbNode->toSolve )
				{
					currentnode.spNode->setBrother( spNewVerbNode );
					deq.push_back( tnodeVerbNode( pNext, spNewVerbNode ) );
				}
			}
		}
		//left
		aq::tnode* pLeft = currentnode.pNode->left;
		if( pLeft )
		{
			VerbNode::Ptr spNewVerbNode = VerbNode::build(pSelect, pLeft, pStartOriginal, context, BaseDesc, *pSettings);
			if( spNewVerbNode && spNewVerbNode->toSolve )
			{
				currentnode.spNode->setLeftChild( spNewVerbNode );
				deq.push_back( tnodeVerbNode( pLeft, spNewVerbNode ) );
			}
		}
		//right
		aq::tnode* pRight = currentnode.pNode->right;
		if( pRight )
		{
			VerbNode::Ptr spNewVerbNode = VerbNode::build(pSelect, pRight, pStartOriginal, context, BaseDesc, *pSettings);
			if( spNewVerbNode && spNewVerbNode->toSolve )
			{
				currentnode.spNode->setRightChild( spNewVerbNode );
				deq.push_back( tnodeVerbNode( pRight, spNewVerbNode ) );
			}
		}

		deq.pop_front();
	}

	return spStart;
}

//------------------------------------------------------------------------------
void VerbNode::dump(std::ostream& os, VerbNode::Ptr tree, std::string ident)
{
  if (tree != nullptr)
  {
    os << id_to_string(tree->getVerbType()) << " [" << tree.get() << "]" << std::endl;
    os << ident << "  left -> ";
    dump(os, tree->getLeftChild(), ident + "  ");
    os << ident << "  right -> ";
    dump(os, tree->getRightChild(), ident + "  ");
    os << ident ;
    dump(os, tree->getBrother(), ident);
  }
  else 
  {
    os << "nullptr" << std::endl;
  }
}

}
}
