#pragma once

#include "VerbNode.h"

namespace aq {
namespace verb {

//------------------------------------------------------------------------------
class AggregateVerb: public VerbNode
{
	VERB_DECLARE( AggregateVerb );
public:
	virtual int getVerbType() const { return -1; };
	virtual bool changeQuery( aq::tnode* pStart, aq::tnode* pNode,
		VerbResult::Ptr resLeft, VerbResult::Ptr resRight, VerbResult::Ptr resNext );
	virtual void changeResult( Table::Ptr table, 
		VerbResult::Ptr resLeft, VerbResult::Ptr resRight, VerbResult::Ptr resNext );
  virtual void addResult(aq::Row& row);
  virtual void accept(VerbVisitor* visitor);
protected:
	virtual Scalar::Ptr computeResultRegular(	Column::Ptr column, 
												Table::Ptr table,
												llong start,
												llong end )
	{ assert(0); return NULL; };
	virtual VerbResult::Ptr computeResultPartition(	Column::Ptr column, 
													Table::Ptr table,
													TablePartition::Ptr partition )
	{ assert(0); return NULL; };
private:
  int index;
  uint64_t count;
  aq::ColumnItem item;
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
	VERB_DECLARE( SumVerb );
public:
	virtual int getVerbType() const { return K_SUM; };
protected:
	virtual Scalar::Ptr computeResultRegular(	Column::Ptr column, 
												Table::Ptr table,
												llong start,
												llong end );
	virtual VerbResult::Ptr computeResultPartition(	Column::Ptr column, 
													Table::Ptr table,
													TablePartition::Ptr partition );
  // void accept(VerbVisitor* visitor);
};

//------------------------------------------------------------------------------
class CountVerb: public AggregateVerb
{
	VERB_DECLARE( CountVerb );
public:
	virtual int getVerbType() const { return K_COUNT; };
	virtual bool preprocessQuery( aq::tnode* pStart, aq::tnode* pNode, aq::tnode* pStartOriginal );
  // void accept(VerbVisitor* visitor);
protected:
	virtual Scalar::Ptr computeResultRegular(	Column::Ptr column, 
												Table::Ptr table,
												llong start,
												llong end );
	virtual VerbResult::Ptr computeResultPartition(	Column::Ptr column, 
													Table::Ptr table,
													TablePartition::Ptr partition );
};

//------------------------------------------------------------------------------
class AvgVerb: public AggregateVerb
{
	VERB_DECLARE( AvgVerb );
public:
	virtual int getVerbType() const { return K_AVG; };
  // void accept(VerbVisitor* visitor);
protected:
	virtual Scalar::Ptr computeResultRegular(	Column::Ptr column, 
												Table::Ptr table,
												llong start,
												llong end );
	virtual VerbResult::Ptr computeResultPartition(	Column::Ptr column, 
													Table::Ptr table,
													TablePartition::Ptr partition );
};

//------------------------------------------------------------------------------
class MinVerb: public AggregateVerb
{
	VERB_DECLARE( MinVerb );
public:
	virtual int getVerbType() const { return K_MIN; };
  // void accept(VerbVisitor* visitor);
protected:
	virtual Scalar::Ptr computeResultRegular(	Column::Ptr column, 
												Table::Ptr table,
												llong start,
												llong end );
	virtual VerbResult::Ptr computeResultPartition(	Column::Ptr column, 
													Table::Ptr table,
													TablePartition::Ptr partition );
};

//------------------------------------------------------------------------------
class MaxVerb: public AggregateVerb
{
	VERB_DECLARE( MaxVerb );
public:
	virtual int getVerbType() const { return K_MAX; };
  // void accept(VerbVisitor* visitor);
protected:
	virtual Scalar::Ptr computeResultRegular(	Column::Ptr column, 
												Table::Ptr table,
												llong start,
												llong end );
	virtual VerbResult::Ptr computeResultPartition(	Column::Ptr column, 
													Table::Ptr table,
													TablePartition::Ptr partition );
};

//------------------------------------------------------------------------------
class FirstValueVerb: public VerbNode
{
	VERB_DECLARE( FirstValueVerb );
public:
	virtual int getVerbType() const { return K_FIRST_VALUE; };
	virtual void changeResult( Table::Ptr table, 
		VerbResult::Ptr resLeft, VerbResult::Ptr resRight, VerbResult::Ptr resNext );
  virtual void accept(VerbVisitor* visitor);
};

//------------------------------------------------------------------------------
//debug13
//WARNING! Lag modifies the result in such a way that it assumes items in columns 
//will only ever be read or deleted, but never modified
//this is an assumption that holds at the time of writing this comment but it may
//change
//Obs: enforcing this assumption in the future would allow for some optimizations
//and useful code refactoring
class LagVerb: public VerbNode
{
	VERB_DECLARE( LagVerb );
public:
	virtual int getVerbType() const { return K_LAG; };
	~LagVerb();

	virtual bool preprocessQuery( aq::tnode* pStart, aq::tnode* pNode, aq::tnode* pStartOriginal );
	virtual void changeResult( Table::Ptr table, 
		VerbResult::Ptr resLeft, VerbResult::Ptr resRight, VerbResult::Ptr resNext );
  virtual void accept(VerbVisitor* visitor);
//debug13 ugly hack to let FirstValueVerb use LagVerb private:
	llong Offset;
	aq::tnode* Default;
};

//------------------------------------------------------------------------------
class RowNumberVerb: public VerbNode
{
	VERB_DECLARE( RowNumberVerb );
public:
	virtual int getVerbType() const { return K_ROW_NUMBER; };
	virtual void changeResult( Table::Ptr table, 
		VerbResult::Ptr resLeft, VerbResult::Ptr resRight, VerbResult::Ptr resNext );
  virtual void accept(VerbVisitor* visitor);
};

}
}