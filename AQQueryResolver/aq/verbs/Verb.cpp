#include "Verb.h"
#include "VerbFactory.h"
#include "VerbVisitor.h"
#include <aq/parser/ID2Str.h>
#include <aq/Logger.h>

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
void addColumn( aq::tnode* pNode, aq::tnode* pColumnNode )
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
	
	aq::tnode* pAuxNode = pNode->right;
	pNode->right = new_node( K_COMMA );
	pNode->right->left = pAuxNode;
	pNode->right->right = pColumnNode;
}


*/

