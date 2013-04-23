#include "VerbNode.h"
#include "VerbFactory.h"
#include <aq/parser/ID2Str.h>

//------------------------------------------------------------------------------
VerbNode::VerbNode(): pStart(NULL), pNode(NULL)
{	
}

//------------------------------------------------------------------------------
void VerbNode::setLeftChild( VerbNode::Ptr child )
{
	this->Left = child;
}

//------------------------------------------------------------------------------
void VerbNode::setRightChild( VerbNode::Ptr child )
{
	this->Right = child;
}

//------------------------------------------------------------------------------
void VerbNode::setBrother( VerbNode::Ptr brother )
{
	this->Brother = brother;
}

//------------------------------------------------------------------------------
void VerbNode::changeQuery()
{
	if( this->Brother )
		this->Brother->changeQuery();
	if( this->Left )
		this->Left->changeQuery();
	if( this->Right)
		this->Right->changeQuery();
	
	if( pNode->inf == 1 )
		return;
	
	//std::cout << "===" << std::endl << std::endl;
	//std::cout << *this->pNode << std::endl << std::endl;
	//std::cout << "---" << std::endl << std::endl;

	VerbResult::Ptr param1 = this->Left ? this->Left->getResult() : NULL;
	VerbResult::Ptr param2 = this->Right ? this->Right->getResult() : NULL;
	VerbResult::Ptr param3 = this->Brother ? this->Brother->getResult() : NULL;
	if( this->changeQuery( this->pStart, this->pNode, param1, param2, param3 ) )
	{
		assert( this->pNode );
		this->pNode->inf = 1;
		this->Disabled = true;
	}
	
	//std::cout << *this->pNode << std::endl << std::endl;
	//std::cout << "===" << std::endl << std::endl;

}

//------------------------------------------------------------------------------
void VerbNode::changeResult( Table::Ptr table )
{
	if( this->Brother )
		this->Brother->changeResult( table );
//	if( pNode->inf == 1 ) //debug13 ! I cannot check this information because nodes are deleted by the time I check it
//		return; //but i also need to make sure that I do not build Columns that appear in jeq
	if( this->Disabled )
		return;

	//right first because partition by modifies the column pointers in table
	//and it should be executed before order by collects his list of column pointers
	if( this->Right )
		this->Right->changeResult( table );
	if( this->Left )
		this->Left->changeResult( table );

	VerbResult::Ptr param1 = this->Left ? this->Left->getResult() : NULL;
	VerbResult::Ptr param2 = this->Right ? this->Right->getResult() : NULL;
	VerbResult::Ptr param3 = this->Brother ? this->Brother->getResult() : NULL;
	this->changeResult( table, param1, param2, param3 );

	//add column results to the table, to be sorted/modified along with the other
	//columns
	if( this->Result && this->Result->getType() == VerbResult::COLUMN )
	{
		bool found = false;
		for( size_t idx = 0; idx < table->Columns.size(); ++idx )
			if( table->Columns[idx].get() == this->Result.get() )
			{
				found = true;
				break;
			}

		if( !found )
		{
			Column::Ptr count;
			if( table->HasCount )
			{
				count = table->Columns[table->Columns.size() - 1];
				table->Columns.pop_back();
			}
			table->Columns.push_back( boost::static_pointer_cast<Column>(this->Result) );
			if( table->HasCount )
				table->Columns.push_back( count );
		}
	}
	/*
	//delete now so that if we have a big tree
	//we don't delete it all by each node's destructor calling the
	//child's destructor, there is stack overflow danger
	this->Left = NULL;
	this->Right = NULL;
	this->Brother = NULL;
	*/
}

//------------------------------------------------------------------------------
void VerbNode::addResult(aq::RowProcess_Intf::row_t& row)
{
	if( this->Brother )
		this->Brother->addResult( row );

	if( this->Disabled )
		return;

	//right first because partition by modifies the column pointers in table
	//and it should be executed before order by collects his list of column pointers
	if( this->Right )
		this->Right->addResult( row );
	if( this->Left )
		this->Left->addResult( row );

	VerbResult::Ptr param1 = this->Left ? this->Left->getResult() : NULL;
	VerbResult::Ptr param2 = this->Right ? this->Right->getResult() : NULL;
	VerbResult::Ptr param3 = this->Brother ? this->Brother->getResult() : NULL;
	this->addResult( row, param1, param2, param3 );
  
}

//------------------------------------------------------------------------------
void VerbNode::acceptTopLeftRight(VerbVisitor* visitor)
{
	this->accept(visitor);
	if (this->Left) this->Left->acceptTopLeftRight(visitor);
	if (this->Right) this->Right->acceptTopLeftRight(visitor);
	if (this->Brother) this->Brother->acceptTopLeftRight(visitor);
}

//------------------------------------------------------------------------------
void VerbNode::acceptLeftTopRight(VerbVisitor* visitor)
{
	if (this->Left) this->Left->acceptLeftTopRight(visitor);
	this->accept(visitor);
	if (this->Right) this->Right->acceptLeftTopRight(visitor);
	if (this->Brother) this->Brother->acceptLeftTopRight(visitor);
}

//------------------------------------------------------------------------------
VerbNode::Ptr VerbNode::getLeftChild(){ return this->Left; }
VerbNode::Ptr VerbNode::getRightChild(){ return this->Right; }
VerbNode::Ptr VerbNode::getBrother(){ return this->Brother; }

//------------------------------------------------------------------------------
//helper struct for BuildVerbsTree
struct tnodeVerbNode
{
	tnodeVerbNode( tnode* pNode, VerbNode::Ptr spNode ): 
		pNode(pNode), spNode(spNode){}
	tnode* pNode;
	VerbNode::Ptr spNode;
};

//------------------------------------------------------------------------------
VerbNode::Ptr VerbNode::build( tnode* pStart, tnode* pNode, tnode* pStartOriginal, int context, Base& BaseDesc, TProjectSettings& settings )
{
	VerbNode::Ptr verb = VerbFactory::GetInstance().getVerb( pNode->tag );
  
	if( !verb )
  {
		// throw aq::generic_error(aq::generic_error::NOT_IMPLEMENED, "Verb Not implemented");
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
VerbNode::Ptr VerbNode::BuildVerbsTree( tnode* pStart, const std::vector<unsigned int>& categories_order, Base& baseDesc, TProjectSettings * settings )
{
	if( pStart->tag != K_SELECT )
		throw 0; // TODO

	tnode* pStartOriginal = clone_subtree( pStart );
	
	VerbNode::Ptr spLast = NULL;
	for( size_t idx = 0; idx < categories_order.size(); ++idx )
	{
		tnode* pNode = pStart;
		while( pNode && pNode->tag != categories_order[idx] ) 
			pNode = pNode->next;
		if( !pNode )
			continue;
		
		VerbNode::Ptr spNode = VerbNode::BuildVerbsSubtree( pStart, pNode, pStartOriginal, categories_order[idx], baseDesc, settings );
		spNode->setBrother( spLast );
		spLast = spNode;
	}
	delete_subtree( pStartOriginal );
	return spLast;
}

//------------------------------------------------------------------------------
VerbNode::Ptr VerbNode::BuildVerbsSubtree(	tnode* pSelect, tnode* pStart, tnode* pStartOriginal, int context, Base& BaseDesc, TProjectSettings *pSettings  )
{
	VerbNode::Ptr spStart = VerbNode::build(pSelect, pStart, pStartOriginal, context, BaseDesc, *pSettings);
	if( !spStart || !spStart->isToSolved() )
		return spStart;
	std::deque<tnodeVerbNode> deq;
	deq.push_back( tnodeVerbNode(pStart, spStart) );

	while( deq.size() > 0 )
	{
		tnodeVerbNode& currentNode = deq[0];
		//next
		if( currentNode.pNode != pStart )
		{
			tnode* pNext = currentNode.pNode->next;
			if( pNext )
			{
				VerbNode::Ptr spNewVerbNode = VerbNode::build(pSelect, pNext, pStartOriginal, context, BaseDesc, *pSettings);
				if( spNewVerbNode && spNewVerbNode->isToSolved() )
				{
					currentNode.spNode->setBrother( spNewVerbNode );
					deq.push_back( tnodeVerbNode( pNext, spNewVerbNode ) );
				}
			}
		}
		//left
		tnode* pLeft = currentNode.pNode->left;
		if( pLeft )
		{
			VerbNode::Ptr spNewVerbNode = VerbNode::build(pSelect, pLeft, pStartOriginal, context, BaseDesc, *pSettings);
			if( spNewVerbNode && spNewVerbNode->isToSolved() )
			{
				currentNode.spNode->setLeftChild( spNewVerbNode );
				deq.push_back( tnodeVerbNode( pLeft, spNewVerbNode ) );
			}
		}
		//right
		tnode* pRight = currentNode.pNode->right;
		if( pRight )
		{
			VerbNode::Ptr spNewVerbNode = VerbNode::build(pSelect, pRight, pStartOriginal, context, BaseDesc, *pSettings);
			if( spNewVerbNode && spNewVerbNode->isToSolved() )
			{
				currentNode.spNode->setRightChild( spNewVerbNode );
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
  if (tree != NULL)
  {
    os << id_to_string(tree->getVerbType()) << std::endl;
    os << ident << "  left -> ";
    dump(os, tree->getLeftChild(), ident + "  ");
    os << ident << "  right -> ";
    dump(os, tree->getRightChild(), ident + "  ");
    os << ident ;
    dump(os, tree->getBrother(), ident);
  }
  else 
  {
    os << "NULL" << std::endl;
  }
}
