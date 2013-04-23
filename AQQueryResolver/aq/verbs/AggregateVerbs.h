#pragma once

#include "VerbNode.h"

//------------------------------------------------------------------------------
class AggregateVerb: public VerbNode
{
	VERB_DECLARE( AggregateVerb );
public:
	virtual int getVerbType() const { return -1; };
	virtual bool changeQuery( tnode* pStart, tnode* pNode,
		VerbResult::Ptr resLeft, VerbResult::Ptr resRight, VerbResult::Ptr resNext );
	virtual void changeResult( Table::Ptr table, 
		VerbResult::Ptr resLeft, VerbResult::Ptr resRight, VerbResult::Ptr resNext );
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
};

//------------------------------------------------------------------------------
class SumVerb: public AggregateVerb
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
  void addResult(aq::RowProcess_Intf::row_t& row, 
    VerbResult::Ptr resLeft, VerbResult::Ptr resRight, VerbResult::Ptr resNext );
  void accept(VerbVisitor* visitor);
};

//------------------------------------------------------------------------------
class CountVerb: public AggregateVerb
{
	VERB_DECLARE( CountVerb );
public:
	virtual int getVerbType() const { return K_COUNT; };
	virtual bool preprocessQuery( tnode* pStart, tnode* pNode, tnode* pStartOriginal );
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

	virtual bool preprocessQuery( tnode* pStart, tnode* pNode, tnode* pStartOriginal );
	virtual void changeResult( Table::Ptr table, 
		VerbResult::Ptr resLeft, VerbResult::Ptr resRight, VerbResult::Ptr resNext );
//debug13 ugly hack to let FirstValueVerb use LagVerb private:
	llong Offset;
	tnode* Default;
};

//------------------------------------------------------------------------------
class RowNumberVerb: public VerbNode
{
	VERB_DECLARE( RowNumberVerb );
public:
	virtual int getVerbType() const { return K_ROW_NUMBER; };
	virtual void changeResult( Table::Ptr table, 
		VerbResult::Ptr resLeft, VerbResult::Ptr resRight, VerbResult::Ptr resNext );
};