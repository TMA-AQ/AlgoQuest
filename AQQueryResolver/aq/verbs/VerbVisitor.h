#ifndef __VERB_VISITOR_H__
#define __VERB_VISITOR_H__

#include "AggregateVerbs.h"
#include "ArithmeticVerbs.h"
#include "AuxiliaryVerbs.h"
#include "CaseVerb.h"
#include "ComparisonVerbs.h"
#include "ConversionVerbs.h"
#include "DateVerbs.h"
#include "JoinVerbs.h"
#include "MainVerbs.h"
#include "OverVerbs.h"
#include "ScalarVerbs.h"

class VerbVisitor
{
public:

	// Default Verbs
	virtual void visit(Verb*) = 0;

	// Aggregate Verbs
	virtual void visit(AggregateVerb*) = 0;
	virtual void visit(AvgVerb*) = 0;
	virtual void visit(CountVerb*) = 0;
	virtual void visit(FirstValueVerb*) = 0;
	virtual void visit(LagVerb*) = 0;
	virtual void visit(MaxVerb*) = 0;
	virtual void visit(MinVerb*) = 0;
	virtual void visit(RowNumberVerb*) = 0;
	virtual void visit(SumVerb*) = 0;
	
	// Arithmetics Verbs
	virtual void visit(BinaryVerb*) = 0;
	virtual void visit(DivideVerb*) = 0;
	virtual void visit(MinusVerb*) = 0;
	virtual void visit(MultiplyVerb*) = 0;
	virtual void visit(PlusVerb*) = 0;

	// Auxiliary Verbs
	virtual void visit(AndVerb*) = 0;
	virtual void visit(AscVerb*) = 0;
	virtual void visit(AsteriskVerb*) = 0;
	virtual void visit(AsVerb*) = 0;
	virtual void visit(ColumnVerb*) = 0;
	virtual void visit(CommaVerb*) = 0;
	virtual void visit(DoubleValueVerb*) = 0;
	virtual void visit(IntValueVerb*) = 0;
	virtual void visit(InVerb*) = 0;
	virtual void visit(StringValueVerb*) = 0;

	// Case Verbs
	virtual void visit(CaseVerb*) = 0;
	virtual void visit(ElseVerb*) = 0;
	virtual void visit(WhenVerb*) = 0;

	// Comparison Verbs
	virtual void visit(BetweenVerb*) = 0;
	virtual void visit(ComparisonVerb*) = 0;
	virtual void visit(EqVerb*) = 0;
	virtual void visit(GeqVerb*) = 0;
	virtual void visit(GtVerb*) = 0;
	virtual void visit(JeqVerb*) = 0;
	virtual void visit(JieqVerb*) = 0;
	virtual void visit(JinfVerb*) = 0;
	virtual void visit(JneqVerb*) = 0;
	virtual void visit(JseqVerb*) = 0;
	virtual void visit(JsupVerb*) = 0;
	virtual void visit(LeqVerb*) = 0;
	virtual void visit(LikeVerb*) = 0;
	virtual void visit(LtVerb*) = 0;
	virtual void visit(NeqVerb*) = 0;
	virtual void visit(NotBetweenVerb*) = 0;
	virtual void visit(NotLikeVerb*) = 0;

	// Conversion Verbs
	virtual void visit(CastVerb*) = 0;
	virtual void visit(DecodeVerb*) = 0;
	virtual void visit(NvlVerb*) = 0;

	// Date Verbs
	virtual void visit(CurrentDateVerb*) = 0;

	// Join Verbs
	virtual void visit(FullJoinVerb*) = 0;
	virtual void visit(JoinVerb*) = 0;
	virtual void visit(LeftJoinVerb*) = 0;
	virtual void visit(RightJoinVerb*) = 0;

	// Main Verbs
	virtual void visit(ByVerb*) = 0;
	virtual void visit(FromVerb*) = 0;
	virtual void visit(GroupVerb*) = 0;
	virtual void visit(OrderVerb*) = 0;
	virtual void visit(SelectVerb*) = 0;
	virtual void visit(WhereVerb*) = 0;

	// Over Verbs
	virtual void visit(FrameVerb*) = 0;
	virtual void visit(PartitionVerb*) = 0;
};

#endif