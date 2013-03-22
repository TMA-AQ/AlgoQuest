#include "Verb.h"
#include "VerbVisitor.h"
#include "ID2Str.h"
#include <aq/Logger.h>

using namespace boost;

//------------------------------------------------------------------------------
verb_error::verb_error( EType type, int verbTag ):
	generic_error( type, "" )
{
	this->Message +=  id_to_string( verbTag );
}

//------------------------------------------------------------------------------
Verb::Verb(): Result(NULL), Context(0), Disabled(false)
{
}

//------------------------------------------------------------------------------
void Verb::setContext( int context )
{
	this->Context = context;
}

//------------------------------------------------------------------------------
VerbResult::Ptr Verb::getResult() const
{
	return this->Result;
}

//------------------------------------------------------------------------------
void Verb::accept(VerbVisitor* visitor)
{
	visitor->visit(this);
}

/*
//------------------------------------------------------------------------------
void addColumn( tnode* pNode, tnode* pColumnNode )
{
	if( !pNode || !pColumnNode )
		return;
	if( pNode->tag != K_SELECT )
		return;
	assert( pNode->right == NULL );
	
	if( !pNode->left )
	{
		pNode->left = pColumnNode;
		return;
	}
	while( pNode->right && pNode->right->tag == K_COMMA )
		pNode = pNode->right;
	
	tnode* pAuxNode = pNode->right;
	pNode->right = new_node( K_COMMA );
	pNode->right->left = pAuxNode;
	pNode->right->right = pColumnNode;
}


*/



//------------------------------------------------------------------------------
VerbNode::VerbNode(): pStart(NULL), pNode(NULL), VerbObject(NULL)
{	
}

//------------------------------------------------------------------------------
bool VerbNode::build( tnode* pStart, tnode* pNode, tnode* pStartOriginal, int context, Base& BaseDesc, TProjectSettings& settings )
{
	this->pStart = pStart;
	this->pNode = pNode;
	if( !pStart || !pNode )
		return false;
		
	if( pNode->inf == 1 ) //when a subtree is copied from a nested select into
		//the outer select, it may have nodes who have inf set to 1
		//do not bother creating verbs for these nodes at all, they are considered
		//solved by the engine
		return false;

	this->VerbObject = VerbFactory::GetInstance().getVerb( pNode->tag );
	if( !this->VerbObject )
		return false;
	this->VerbObject->setContext( context );
  this->VerbObject->setBaseDesc(&BaseDesc);
  this->VerbObject->setSettings(&settings);

	if( this->VerbObject->preprocessQuery( pStart, pNode, pStartOriginal ) )
	{
		pNode->inf = 1;
		return false;
	}

	return true;
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
	if( !this->VerbObject )
		return;
	if( this->Left )
		this->Left->changeQuery();
	if( this->Right)
		this->Right->changeQuery();
	
	if( pNode->inf == 1 )
		return;
	
	//std::cout << "===" << std::endl << std::endl;
	//std::cout << *this->pNode << std::endl << std::endl;
	//std::cout << "---" << std::endl << std::endl;

	VerbResult::Ptr param1 = this->Left && this->Left->VerbObject ? this->Left->VerbObject->getResult() : NULL;
	VerbResult::Ptr param2 = this->Right && this->Right->VerbObject ? this->Right->VerbObject->getResult() : NULL;
	VerbResult::Ptr param3 = this->Brother && this->Brother->VerbObject ? this->Brother->VerbObject->getResult() : NULL;
	if( this->VerbObject->changeQuery( this->pStart, this->pNode, param1, param2, param3 ) )
	{
		assert( this->pNode );
		this->pNode->inf = 1;
		this->VerbObject->Disabled = true;
	}
	
	//std::cout << *this->pNode << std::endl << std::endl;
	//std::cout << "===" << std::endl << std::endl;

}

//------------------------------------------------------------------------------
void VerbNode::changeResult( Table::Ptr table )
{
	if( this->Brother )
		this->Brother->changeResult( table );
	if( !this->VerbObject )
		return;
//	if( pNode->inf == 1 ) //debug13 ! I cannot check this information because nodes are deleted by the time I check it
//		return; //but i also need to make sure that I do not build Columns that appear in jeq
	if( this->VerbObject->Disabled )
		return;

	//right first because partition by modifies the column pointers in table
	//and it should be executed before order by collects his list of column pointers
	if( this->Right )
		this->Right->changeResult( table );
	if( this->Left )
		this->Left->changeResult( table );

	VerbResult::Ptr param1 = this->Left && this->Left->VerbObject ? this->Left->VerbObject->getResult() : NULL;
	VerbResult::Ptr param2 = this->Right && this->Right->VerbObject ? this->Right->VerbObject->getResult() : NULL;
	VerbResult::Ptr param3 = this->Brother && this->Brother->VerbObject ? this->Brother->VerbObject->getResult() : NULL;

	this->VerbObject->changeResult( table, param1, param2, param3 );

	//add column results to the table, to be sorted/modified along with the other
	//columns
	if( this->VerbObject->Result &&
		this->VerbObject->Result->getType() == VerbResult::COLUMN )
	{
		bool found = false;
		for( size_t idx = 0; idx < table->Columns.size(); ++idx )
			if( table->Columns[idx].get() == this->VerbObject->Result.get() )
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
			table->Columns.push_back( static_pointer_cast<Column>(this->VerbObject->Result) );
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
void VerbNode::accept(VerbVisitor* visitor)
{
	this->VerbObject->accept(visitor);
	if (this->Left) this->Left->accept(visitor);
	if (this->Right) this->Right->accept(visitor);
	if (this->Brother) this->Brother->accept(visitor);
}

//------------------------------------------------------------------------------
Verb::Ptr VerbNode::getVerbObject(){ return this->VerbObject; }
VerbNode::Ptr VerbNode::getLeftChild(){ return this->Left; }
VerbNode::Ptr VerbNode::getRightChild(){ return this->Right; }
VerbNode::Ptr VerbNode::getBrother(){ return this->Brother; }

//------------------------------------------------------------------------------
VerbFactory& VerbFactory::GetInstance()
{
	static VerbFactory Instance;
	return Instance;
}

//------------------------------------------------------------------------------
void VerbFactory::addVerb( Verb::Ptr verb )
{
	if( verb )
		this->Verbs.push_back( verb );
}

//------------------------------------------------------------------------------
Verb::Ptr VerbFactory::getVerb( int verbType ) const
{
	for( size_t idx = 0; idx < this->Verbs.size(); ++idx )
		if( this->Verbs[idx]->getVerbType() == verbType )
			return this->Verbs[idx]->clone();
	return NULL;
}
