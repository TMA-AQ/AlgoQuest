#include "DateVerbs.h"
#include "VerbVisitor.h"
#include <aq/Exceptions.h>
#include <aq/DateConversion.h>

using namespace aq;
using namespace std;
using namespace boost;

//------------------------------------------------------------------------------
VERB_IMPLEMENT(CurrentDateVerb);

//------------------------------------------------------------------------------
CurrentDateVerb::CurrentDateVerb()
{}

//------------------------------------------------------------------------------
bool CurrentDateVerb::changeQuery(	aq::tnode* pStart, aq::tnode* pNode,
									VerbResult::Ptr resLeft,
									VerbResult::Ptr resRight, 
									VerbResult::Ptr resNext )
{
	Scalar::Ptr scalar = new Scalar( COL_TYPE_DATE1, 8, ColumnItem( (double) currentDate() ) );
	pNode->tag = K_DATE_VALUE;
	set_data( *pNode, scalar->getValue(), scalar->Type );
	this->Result = scalar;
	return true;
}

//------------------------------------------------------------------------------
void CurrentDateVerb::accept(VerbVisitor* visitor)
{
  visitor->visit(this);
}