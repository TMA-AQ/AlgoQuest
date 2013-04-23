#ifndef __DUMP_VISITOR_H__
#define __DUMP_VISITOR_H__

#include "verbs/VerbVisitor.h"

class DumpVisitor : public VerbVisitor
{
public:
	// Default Verbs
	virtual void visit(Verb*);

	// Aggregate Verbs
	virtual void visit(AggregateVerb*);
	virtual void visit(AvgVerb*);
	virtual void visit(CountVerb*);
	virtual void visit(FirstValueVerb*);
	virtual void visit(LagVerb*);
	virtual void visit(MaxVerb*);
	virtual void visit(MinVerb*);
	virtual void visit(RowNumberVerb*);
	virtual void visit(SumVerb*);
	
	// Arithmetics Verbs
	virtual void visit(BinaryVerb*);
	virtual void visit(DivideVerb*);
	virtual void visit(MinusVerb*);
	virtual void visit(MultiplyVerb*);
	virtual void visit(PlusVerb*);

	// Auxiliary Verbs
	virtual void visit(AndVerb*);
	virtual void visit(AscVerb*);
	virtual void visit(AsteriskVerb*);
	virtual void visit(AsVerb*);
	virtual void visit(ColumnVerb*);
	virtual void visit(CommaVerb*);
	virtual void visit(DoubleValueVerb*);
	virtual void visit(IntValueVerb*);
	virtual void visit(InVerb*);
	virtual void visit(StringValueVerb*);

	// Case Verbs
	virtual void visit(CaseVerb*);
	virtual void visit(ElseVerb*);
	virtual void visit(WhenVerb*);

	// Comparison Verbs
	virtual void visit(BetweenVerb*);
	virtual void visit(ComparisonVerb*);
	virtual void visit(EqVerb*);
	virtual void visit(GeqVerb*);
	virtual void visit(GtVerb*);
	virtual void visit(JeqVerb*);
	virtual void visit(JieqVerb*);
	virtual void visit(JinfVerb*);
	virtual void visit(JneqVerb*);
	virtual void visit(JseqVerb*);
	virtual void visit(JsupVerb*);
	virtual void visit(LeqVerb*);
	virtual void visit(LikeVerb*);
	virtual void visit(LtVerb*);
	virtual void visit(NeqVerb*);
	virtual void visit(NotBetweenVerb*);
	virtual void visit(NotLikeVerb*);

	// Conversion Verbs
	virtual void visit(CastVerb*);
	virtual void visit(DecodeVerb*);
	virtual void visit(NvlVerb*);

	// Date Verbs
	virtual void visit(CurrentDateVerb*);

	// Join Verbs
	virtual void visit(FullJoinVerb*);
	virtual void visit(JoinVerb*);
	virtual void visit(LeftJoinVerb*);
	virtual void visit(RightJoinVerb*);

	// Main Verbs
	virtual void visit(ByVerb*);
	virtual void visit(FromVerb*);
	virtual void visit(GroupVerb*);
	virtual void visit(OrderVerb*);
	virtual void visit(SelectVerb*);
	virtual void visit(WhereVerb*);

	// Over Verbs
	virtual void visit(FrameVerb*);
	virtual void visit(PartitionVerb*);

	const std::string& getQuery() { 
    this->query = this->selectStr + this->fromStr + this->whereStr ; 
    return this->query;
  }

private:
  std::string selectStr;
  std::string fromStr;
  std::string whereStr;
  std::string groupStr;
  std::string orderStr;
	std::string query;

};

#endif