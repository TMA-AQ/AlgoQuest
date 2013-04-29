#pragma once

#include "VerbNode.h"

//------------------------------------------------------------------------------
class CaseVerb: public VerbNode
{
	VERB_DECLARE( CaseVerb );
public:
	virtual int getVerbType() const { return K_CASE; };
	virtual void changeResult( Table::Ptr table, 
		VerbResult::Ptr resLeft, VerbResult::Ptr resRight, VerbResult::Ptr resNext );
  virtual void accept(VerbVisitor* visitor);
};

//------------------------------------------------------------------------------
class WhenVerb: public VerbNode
{
	VERB_DECLARE( WhenVerb );
public:
	virtual int getVerbType() const { return K_WHEN; };
	virtual void changeResult( Table::Ptr table, 
		VerbResult::Ptr resLeft, VerbResult::Ptr resRight, VerbResult::Ptr resNext );
  virtual void accept(VerbVisitor* visitor);
};

//------------------------------------------------------------------------------
class ElseVerb: public VerbNode
{
	VERB_DECLARE( ElseVerb );
public:
	virtual int getVerbType() const { return K_ELSE; };
	virtual void changeResult( Table::Ptr table, 
		VerbResult::Ptr resLeft, VerbResult::Ptr resRight, VerbResult::Ptr resNext );
  virtual void accept(VerbVisitor* visitor);
};