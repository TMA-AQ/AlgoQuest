#pragma once

#include "VerbNode.h"
/*
//------------------------------------------------------------------------------
class OverVerb: public Verb
{
	VERB_DECLARE( OverVerb );
public:
	virtual int getVerbType() const { return K_OVER; };
	virtual void changeResult( Table::Ptr table, 
		VerbResult::Ptr resLeft, VerbResult::Ptr resRight, VerbResult::Ptr resNext );
};
*/

//------------------------------------------------------------------------------
class PartitionVerb: public VerbNode
{
	VERB_DECLARE( PartitionVerb );
public:
	virtual int getVerbType() const { return K_PARTITION; };
  virtual bool changeQuery( tnode* pStart, tnode* pNode, VerbResult::Ptr resLeft, VerbResult::Ptr resRight, VerbResult::Ptr resNext );
	virtual void changeResult( Table::Ptr table, VerbResult::Ptr resLeft, VerbResult::Ptr resRight, VerbResult::Ptr resNext );
	virtual void addResult( aq::RowProcess_Intf::Row& row, VerbResult::Ptr resLeft, VerbResult::Ptr resRight, VerbResult::Ptr resNext );
  virtual void accept(VerbVisitor* visitor);
};

//------------------------------------------------------------------------------
class FrameVerb: public VerbNode
{
	VERB_DECLARE( FrameVerb );
public:
	virtual int getVerbType() const { return K_FRAME; };
	virtual bool preprocessQuery( tnode* pStart, tnode* pNode, tnode* pStartOriginal );
  virtual void accept(VerbVisitor* visitor);
};
