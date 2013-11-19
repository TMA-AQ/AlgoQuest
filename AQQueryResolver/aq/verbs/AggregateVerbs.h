#pragma once

#include "VerbNode.h"

namespace aq {
namespace verb {

//------------------------------------------------------------------------------
class AggregateVerb: public VerbNode
{
public:
  AggregateVerb();
	virtual int getVerbType() const { return -1; };
	virtual bool changeQuery( aq::tnode* pStart, aq::tnode* pNode, VerbResult::Ptr resLeft, VerbResult::Ptr resRight, VerbResult::Ptr resNext );
	virtual void changeResult( Table::Ptr table, VerbResult::Ptr resLeft, VerbResult::Ptr resRight, VerbResult::Ptr resNext );
  virtual void addResult(aq::Row& row);
  virtual void accept(VerbVisitor* visitor);
private:
  int index;
  uint64_t count;
  aq::row_item_t::item_t item;
};

//------------------------------------------------------------------------------
template <class V>
struct Visitable
{
  virtual void accept(V * v)
  {
    v->visit(this);
  }
};

//------------------------------------------------------------------------------
class SumVerb: public AggregateVerb // , protected Visitable<VerbVisitor>
{
public:
	virtual int getVerbType() const { return K_SUM; };
};

//------------------------------------------------------------------------------
class CountVerb: public AggregateVerb
{
public:
	virtual int getVerbType() const { return K_COUNT; };
};

//------------------------------------------------------------------------------
class AvgVerb: public AggregateVerb
{
public:
	virtual int getVerbType() const { return K_AVG; };
};

//------------------------------------------------------------------------------
class MinVerb: public AggregateVerb
{
public:
	virtual int getVerbType() const { return K_MIN; };
};

//------------------------------------------------------------------------------
class MaxVerb: public AggregateVerb
{
public:
	virtual int getVerbType() const { return K_MAX; };
};

}
}