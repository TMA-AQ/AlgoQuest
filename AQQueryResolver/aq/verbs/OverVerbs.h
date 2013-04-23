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
	virtual void changeResult( Table::Ptr table, 
		VerbResult::Ptr resLeft, VerbResult::Ptr resRight, VerbResult::Ptr resNext );
};

//------------------------------------------------------------------------------
class FrameVerb: public VerbNode
{
	VERB_DECLARE( FrameVerb );
public:
	virtual int getVerbType() const { return K_FRAME; };
	virtual bool preprocessQuery( tnode* pStart, tnode* pNode, tnode* pStartOriginal );
};
