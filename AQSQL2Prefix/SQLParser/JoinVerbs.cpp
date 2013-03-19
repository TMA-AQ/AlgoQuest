#include "JoinVerbs.h"
#include "TreeUtilities.h"

//------------------------------------------------------------------------------
VERB_IMPLEMENT(JoinVerb);

//------------------------------------------------------------------------------
JoinVerb::JoinVerb()
{
}

//------------------------------------------------------------------------------
bool JoinVerb::preprocessQuery( tnode* pStart, tnode* pNode, tnode* pStartOriginal )
{
	assert( pNode && pStart );
	tnode* pJoin = pNode;
	while( pJoin->left && pJoin->tag != K_JOIN )
		pJoin = pJoin->left;
	assert( pJoin->tag == K_JOIN );
	if( pJoin->next && pJoin->next->tag == K_ON )
	{
		addInnerOuterNodes( pJoin->next->left,
			this->leftTag(), this->rightTag() );
		addConditionsToWhere( pJoin->next->left, pStart );
	}
	assert( pJoin->left && (pJoin->left->tag == K_IDENT 
		|| pJoin->left->tag == K_AS && pJoin->left->left
		&& pJoin->left->left->tag == K_IDENT ) );
	assert( pJoin->right && (pJoin->right->tag == K_IDENT
		|| pJoin->right->tag == K_AS && pJoin->right->left
		&& pJoin->right->left->tag == K_IDENT ) );
	tnode* table1 = pJoin->left;
	pJoin->left = NULL;
	tnode* table2 = pJoin->right;
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
