#include "DateVerbs.h"
#include "VerbVisitor.h"
#include <aq/Exceptions.h>
#include <aq/DateConversion.h>

using namespace std;

namespace aq {
namespace verb {

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
	Scalar::Ptr scalar = new Scalar( COL_TYPE_DATE, 8, ColumnItem( (double) DateConversion::currentDate() ) );
	pNode->tag = K_DATE_VALUE;
	pNode->set_data( scalar->getValue(), scalar->Type );
	this->Result = scalar;
	return true;
}

//------------------------------------------------------------------------------
void CurrentDateVerb::accept(VerbVisitor* visitor)
{
  visitor->visit(this);
}

}
}