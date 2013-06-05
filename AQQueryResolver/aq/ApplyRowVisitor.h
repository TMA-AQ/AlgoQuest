#ifndef __APPLY_ROW_VISITOR_H__
#define __APPLY_ROW_VISITOR_H__

#include "verbs/VerbVisitor.h"
#include "Row.h"

namespace aq
{

class ApplyRowVisitor : public verb::VerbVisitor
{
public:

  std::vector<aq::Row> rows;

  ApplyRowVisitor();

  void clear() { this->n = 0; }
  void inc() { this->n++; }
  bool full() const { return (this->n + 1) == rows.size(); }
  
	// Aggregate Verbs
	void visit(verb::AggregateVerb*);
	void visit(verb::AvgVerb*);
	void visit(verb::CountVerb*);
	void visit(verb::FirstValueVerb*);
	void visit(verb::LagVerb*);
	void visit(verb::MaxVerb*);
	void visit(verb::MinVerb*);
	void visit(verb::RowNumberVerb*);
	void visit(verb::SumVerb*);
	
	// Arithmetics Verbs
	void visit(verb::BinaryVerb*);
	void visit(verb::DivideVerb*);
	void visit(verb::MinusVerb*);
	void visit(verb::MultiplyVerb*);
	void visit(verb::PlusVerb*);

	// Auxiliary Verbs
	void visit(verb::AndVerb*);
	void visit(verb::AscVerb*);
	void visit(verb::AsteriskVerb*);
	void visit(verb::AsVerb*);
	void visit(verb::ColumnVerb*);
	void visit(verb::CommaVerb*);
	void visit(verb::DoubleValueVerb*);
	void visit(verb::IntValueVerb*);
	void visit(verb::InVerb*);
	void visit(verb::StringValueVerb*);

	// Case Verbs
	void visit(verb::CaseVerb*);
	void visit(verb::ElseVerb*);
	void visit(verb::WhenVerb*);

	// Comparison Verbs
	void visit(verb::BetweenVerb*);
	void visit(verb::ComparisonVerb*);
	void visit(verb::EqVerb*);
	void visit(verb::GeqVerb*);
	void visit(verb::GtVerb*);
	void visit(verb::JeqVerb*);
	void visit(verb::JieqVerb*);
	void visit(verb::JinfVerb*);
	void visit(verb::JneqVerb*);
	void visit(verb::JseqVerb*);
	void visit(verb::JsupVerb*);
	void visit(verb::LeqVerb*);
	void visit(verb::LikeVerb*);
	void visit(verb::LtVerb*);
	void visit(verb::NeqVerb*);
	void visit(verb::NotBetweenVerb*);
	void visit(verb::NotLikeVerb*);

	// Conversion Verbs
	void visit(verb::CastVerb*);
	void visit(verb::DecodeVerb*);
	void visit(verb::NvlVerb*);

	// Date Verbs
	void visit(verb::CurrentDateVerb*);

	// Join Verbs
	void visit(verb::FullJoinVerb*);
	void visit(verb::JoinVerb*);
	void visit(verb::LeftJoinVerb*);
	void visit(verb::RightJoinVerb*);

	// Main Verbs
	void visit(verb::ByVerb*);
	void visit(verb::FromVerb*);
	void visit(verb::GroupVerb*);
	void visit(verb::OrderVerb*);
	void visit(verb::SelectVerb*);
	void visit(verb::WhereVerb*);

	// Over Verbs
	void visit(verb::FrameVerb*);
	void visit(verb::PartitionVerb*);

  // Scalar Vervs
  void visit(verb::ScalarVerb*);
  void visit(verb::SubstringVerb*);

private:
  size_t n;
  unsigned int context;
};

}

#endif