#pragma once

#include "VerbNode.h"
#include <aq/DBTypes.h>

//------------------------------------------------------------------------------
class CastVerb: public VerbNode
{
	VERB_DECLARE( CastVerb );
public:
	virtual int getVerbType() const { return K_CAST; };

	virtual bool preprocessQuery( tnode* pStart, tnode* pNode, tnode* pStartOriginal );
	virtual bool changeQuery( tnode* pStart, tnode* pNode,
		VerbResult::Ptr resLeft, VerbResult::Ptr resRight, VerbResult::Ptr resNext );
	virtual void changeResult( Table::Ptr table, 
		VerbResult::Ptr resLeft, VerbResult::Ptr resRight, VerbResult::Ptr resNext );
private:
	void solve( VerbResult::Ptr resLeft );
	aq::ColumnType ConvertType;
};

//------------------------------------------------------------------------------
class NvlVerb: public VerbNode
{
	VERB_DECLARE( NvlVerb );
public:
	virtual int getVerbType() const { return K_NVL; };

	virtual void changeResult( Table::Ptr table, 
		VerbResult::Ptr resLeft, VerbResult::Ptr resRight, VerbResult::Ptr resNext );
};

//------------------------------------------------------------------------------
class DecodeVerb: public VerbNode
{
	VERB_DECLARE( DecodeVerb );
public:
	virtual int getVerbType() const { return K_DECODE; };

	virtual bool preprocessQuery( tnode* pStart, tnode* pNode, tnode* pStartOriginal );
	virtual void changeResult( Table::Ptr table, 
		VerbResult::Ptr resLeft, VerbResult::Ptr resRight, VerbResult::Ptr resNext );
};