#include "ConversionVerbs.h"
#include "VerbVisitor.h"
#include <algorithm>
#include <aq/Exceptions.h>
#include <aq/DateConversion.h>

using namespace std;

namespace aq {
namespace verb {

//------------------------------------------------------------------------------
bool CastVerb::preprocessQuery( aq::tnode* pStart, aq::tnode* pNode, aq::tnode* pStartOriginal )
{
	assert( pNode && pNode->right );
	switch( pNode->right->tag )
	{
	case K_INTEGER_TYPE: this->ConvertType = COL_TYPE_INT; break;
	case K_REAL_TYPE: this->ConvertType = COL_TYPE_DOUBLE; break;
	case K_STRING_TYPE: this->ConvertType = COL_TYPE_VARCHAR; break;
	default:
		assert( 0 );
	}
	return false;
}

//------------------------------------------------------------------------------
bool CastVerb::changeQuery(aq::tnode* pStart, 
                           aq::tnode* pNode,
                           VerbResult::Ptr resLeft, 
                           VerbResult::Ptr resRight, 
                           VerbResult::Ptr resNext )
{
  // TODO
	if (!resLeft)
  {
		return false;
  }
  assert(!resRight && !resNext);
	return false;
}

//------------------------------------------------------------------------------
void CastVerb::changeResult(Table::Ptr table, 
                            VerbResult::Ptr resLeft,
                            VerbResult::Ptr resRight, 
                            VerbResult::Ptr resNext )
{
  assert(false);
}

//------------------------------------------------------------------------------
void CastVerb::accept(VerbVisitor* visitor)
{
  visitor->visit(this);
}

//------------------------------------------------------------------------------
void NvlVerb::changeResult( Table::Ptr table, 
							VerbResult::Ptr resLeft, 
							VerbResult::Ptr resRight, 
							VerbResult::Ptr resNext )
{
  assert(false);
}

//------------------------------------------------------------------------------
void NvlVerb::accept(VerbVisitor* visitor)
{
  visitor->visit(this);
}

//------------------------------------------------------------------------------
bool DecodeVerb::preprocessQuery( aq::tnode* pStart, aq::tnode* pNode, aq::tnode* pStartOriginal )
{
	return false;
}

//------------------------------------------------------------------------------
void DecodeVerb::changeResult( Table::Ptr table, 
							   VerbResult::Ptr resLeft, 
							   VerbResult::Ptr resRight, 
							   VerbResult::Ptr resNext )
{
  assert(false);
}

//------------------------------------------------------------------------------
void DecodeVerb::accept(VerbVisitor* visitor)
{
  visitor->visit(this);
}

}
}
