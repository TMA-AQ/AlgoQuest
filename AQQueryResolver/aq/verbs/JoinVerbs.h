#pragma once

#include "Verb.h"

//------------------------------------------------------------------------------
class JoinVerb: public Verb
{
	VERB_DECLARE( JoinVerb );
public:
	virtual int getVerbType() const { return -1; };
	virtual bool preprocessQuery( tnode* pStart, tnode* pNode, tnode* pStartOriginal );
protected:
	virtual int leftTag();
	virtual int rightTag();
};

//------------------------------------------------------------------------------
class LeftJoinVerb: public JoinVerb
{
	VERB_DECLARE( LeftJoinVerb );
public:
	virtual int getVerbType() const { return K_LEFT; };
protected:
	virtual int leftTag();
};

//------------------------------------------------------------------------------
class RightJoinVerb: public JoinVerb
{
	VERB_DECLARE( RightJoinVerb );
public:
	virtual int getVerbType() const { return K_RIGHT; };
protected:
	virtual int rightTag();
};

//------------------------------------------------------------------------------
class FullJoinVerb: public JoinVerb
{
	VERB_DECLARE( FullJoinVerb );
public:
	virtual int getVerbType() const { return K_FULL; };
protected:
	virtual int leftTag();
	virtual int rightTag();
};