#pragma once

#include "VerbNode.h"

namespace aq {
namespace verb {

//------------------------------------------------------------------------------
class JoinVerb: public VerbNode
{
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
public:
	virtual int getVerbType() const { return K_LEFT; };
protected:
	virtual int leftTag();
};

//------------------------------------------------------------------------------
class RightJoinVerb: public JoinVerb
{
public:
	virtual int getVerbType() const { return K_RIGHT; };
protected:
	virtual int rightTag();
};

//------------------------------------------------------------------------------
class FullJoinVerb: public JoinVerb
{
public:
	virtual int getVerbType() const { return K_FULL; };
protected:
	virtual int leftTag();
	virtual int rightTag();
};

}
}
