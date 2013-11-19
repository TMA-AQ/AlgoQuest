#include "CaseVerb.h"
#include "VerbVisitor.h"
#include <aq/Exceptions.h>
#include <algorithm>

using namespace std;

namespace aq {
namespace verb {

//------------------------------------------------------------------------------
void CaseVerb::changeResult(	Table::Ptr table, 
								VerbResult::Ptr resLeft, 
								VerbResult::Ptr resRight, 
								VerbResult::Ptr resNext )
{
}

//------------------------------------------------------------------------------
void CaseVerb::accept(VerbVisitor* visitor)
{
  visitor->visit(this);
}

//------------------------------------------------------------------------------
void WhenVerb::changeResult(	Table::Ptr table, 
								VerbResult::Ptr resLeft, 
								VerbResult::Ptr resRight, 
								VerbResult::Ptr resNext )
{
}

//------------------------------------------------------------------------------
void WhenVerb::accept(VerbVisitor* visitor)
{
  visitor->visit(this);
}

//------------------------------------------------------------------------------
void ElseVerb::changeResult(	Table::Ptr table, 
								VerbResult::Ptr resLeft, 
								VerbResult::Ptr resRight, 
								VerbResult::Ptr resNext )
{
}

//------------------------------------------------------------------------------
void ElseVerb::accept(VerbVisitor* visitor)
{
  visitor->visit(this);
}

}
}
