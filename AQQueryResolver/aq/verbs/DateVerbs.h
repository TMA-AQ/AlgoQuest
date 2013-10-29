#pragma once

#include "VerbNode.h"

namespace aq {
namespace verb {

//------------------------------------------------------------------------------
class CurrentDateVerb: public VerbNode
{
public:
	virtual int getVerbType() const { return K_CURRENT_DATE; };
	virtual bool changeQuery( aq::tnode* pStart, aq::tnode* pNode,
		VerbResult::Ptr resLeft, VerbResult::Ptr resRight, VerbResult::Ptr resNext );
  virtual void accept(VerbVisitor* visitor);
};

}
}