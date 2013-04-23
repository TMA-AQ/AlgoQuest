#pragma once

#include "Verb.h"
#include <aq/DBTypes.h>

//------------------------------------------------------------------------------
class CastVerb: public Verb
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
class NvlVerb: public Verb
{
	VERB_DECLARE( NvlVerb );
public:
	virtual int getVerbType() const { return K_NVL; };

	virtual void changeResult( Table::Ptr table, 
		VerbResult::Ptr resLeft, VerbResult::Ptr resRight, VerbResult::Ptr resNext );
};

//------------------------------------------------------------------------------
class DecodeVerb: public Verb
{
	VERB_DECLARE( DecodeVerb );
public:
	virtual int getVerbType() const { return K_DECODE; };

	virtual bool preprocessQuery( tnode* pStart, tnode* pNode, tnode* pStartOriginal );
	virtual void changeResult( Table::Ptr table, 
		VerbResult::Ptr resLeft, VerbResult::Ptr resRight, VerbResult::Ptr resNext );
};