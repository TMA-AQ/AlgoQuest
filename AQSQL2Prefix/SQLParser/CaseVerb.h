#pragma once

#include "Verb.h"

//------------------------------------------------------------------------------
class CaseVerb: public Verb
{
	VERB_DECLARE( CaseVerb );
public:
	virtual int getVerbType() const { return K_CASE; };
	virtual void changeResult( Table::Ptr table, 
		VerbResult::Ptr resLeft, VerbResult::Ptr resRight, VerbResult::Ptr resNext );
};

//------------------------------------------------------------------------------
class WhenVerb: public Verb
{
	VERB_DECLARE( WhenVerb );
public:
	virtual int getVerbType() const { return K_WHEN; };
	virtual void changeResult( Table::Ptr table, 
		VerbResult::Ptr resLeft, VerbResult::Ptr resRight, VerbResult::Ptr resNext );
};

//------------------------------------------------------------------------------
class ElseVerb: public Verb
{
	VERB_DECLARE( ElseVerb );
public:
	virtual int getVerbType() const { return K_ELSE; };
	virtual void changeResult( Table::Ptr table, 
		VerbResult::Ptr resLeft, VerbResult::Ptr resRight, VerbResult::Ptr resNext );
};