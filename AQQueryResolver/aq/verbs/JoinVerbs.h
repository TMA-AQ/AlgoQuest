#pragma once

#include "VerbNode.h"

namespace aq {
namespace verb {

//------------------------------------------------------------------------------
class JoinVerb: public VerbNode
{
	VERB_DECLARE( JoinVerb );
public:
	virtual int getVerbType() const { return -1; };
	virtual bool preprocessQuery( aq::tnode* pStart, aq::tnode* pNode, aq::tnode* pStartOriginal );
  virtual void accept(VerbVisitor* visitor);
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

}
}
