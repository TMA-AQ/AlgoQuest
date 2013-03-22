#include "DateVerbs.h"
#include <aq/Exceptions.h>

using namespace aq;
using namespace std;
using namespace boost;

//------------------------------------------------------------------------------
VERB_IMPLEMENT(CurrentDateVerb);

//------------------------------------------------------------------------------
CurrentDateVerb::CurrentDateVerb()
{}

//------------------------------------------------------------------------------
bool CurrentDateVerb::changeQuery(	tnode* pStart, tnode* pNode,
									VerbResult::Ptr resLeft,
									VerbResult::Ptr resRight, 
									VerbResult::Ptr resNext )
{
	Scalar::Ptr scalar = new Scalar( COL_TYPE_DATE1, 
		ColumnItem( (double) currentDate() ) );
	pNode->tag = K_DATE_VALUE;
	set_data( pNode, *scalar );
	this->Result = scalar;
	return true;
}