#pragma once

#include "Verb.h"

//------------------------------------------------------------------------------
class CurrentDateVerb: public Verb
{
	VERB_DECLARE( CurrentDateVerb );
public:
	virtual int getVerbType() const { return K_CURRENT_DATE; };
	virtual bool changeQuery( tnode* pStart, tnode* pNode,
		VerbResult::Ptr resLeft, VerbResult::Ptr resRight, VerbResult::Ptr resNext );
};