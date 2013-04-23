#pragma once

#include "VerbNode.h"

//------------------------------------------------------------------------------
class CurrentDateVerb: public VerbNode
{
	VERB_DECLARE( CurrentDateVerb );
public:
	virtual int getVerbType() const { return K_CURRENT_DATE; };
	virtual bool changeQuery( tnode* pStart, tnode* pNode,
		VerbResult::Ptr resLeft, VerbResult::Ptr resRight, VerbResult::Ptr resNext );
};