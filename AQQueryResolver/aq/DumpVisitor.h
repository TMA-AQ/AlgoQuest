#ifndef __DUMP_VISITOR_H__
#define __DUMP_VISITOR_H__

#include "verbs/VerbVisitor.h"
#include <aq/Logger.h>

namespace aq
{

class DumpVisitor : public aq::verb::VerbVisitor
{
public:

	// Aggregate Verbs
	virtual void visit(aq::verb::AggregateVerb*);
	virtual void visit(aq::verb::AvgVerb*);
	virtual void visit(aq::verb::CountVerb*);
	virtual void visit(aq::verb::FirstValueVerb*);
	virtual void visit(aq::verb::LagVerb*);
	virtual void visit(aq::verb::MaxVerb*);
	virtual void visit(aq::verb::MinVerb*);
	virtual void visit(aq::verb::RowNumberVerb*);
	virtual void visit(aq::verb::SumVerb*);
	
	// Arithmetics Verbs
	virtual void visit(aq::verb::BinaryVerb*);
	virtual void visit(aq::verb::DivideVerb*);
	virtual void visit(aq::verb::MinusVerb*);
	virtual void visit(aq::verb::MultiplyVerb*);
	virtual void visit(aq::verb::PlusVerb*);

	// Auxiliary Verbs
	virtual void visit(aq::verb::AndVerb*);
	virtual void visit(aq::verb::AscVerb*);
	virtual void visit(aq::verb::AsteriskVerb*);
	virtual void visit(aq::verb::AsVerb*);
	virtual void visit(aq::verb::ColumnVerb*);
	virtual void visit(aq::verb::CommaVerb*);
	virtual void visit(aq::verb::DoubleValueVerb*);
	virtual void visit(aq::verb::IntValueVerb*);
	virtual void visit(aq::verb::InVerb*);
	virtual void visit(aq::verb::StringValueVerb*);

	// Case Verbs
	virtual void visit(aq::verb::CaseVerb*);
	virtual void visit(aq::verb::ElseVerb*);
	virtual void visit(aq::verb::WhenVerb*);

	// Comparison Verbs
	virtual void visit(aq::verb::BetweenVerb*);
	virtual void visit(aq::verb::ComparisonVerb*);
	virtual void visit(aq::verb::EqVerb*);
	virtual void visit(aq::verb::GeqVerb*);
	virtual void visit(aq::verb::GtVerb*);
	virtual void visit(aq::verb::JeqVerb*);
	virtual void visit(aq::verb::JieqVerb*);
	virtual void visit(aq::verb::JinfVerb*);
	virtual void visit(aq::verb::JneqVerb*);
	virtual void visit(aq::verb::JseqVerb*);
	virtual void visit(aq::verb::JsupVerb*);
	virtual void visit(aq::verb::LeqVerb*);
	virtual void visit(aq::verb::LikeVerb*);
	virtual void visit(aq::verb::LtVerb*);
	virtual void visit(aq::verb::NeqVerb*);
	virtual void visit(aq::verb::NotBetweenVerb*);
	virtual void visit(aq::verb::NotLikeVerb*);

	// Conversion Verbs
	virtual void visit(aq::verb::CastVerb*);
	virtual void visit(aq::verb::DecodeVerb*);
	virtual void visit(aq::verb::NvlVerb*);

	// Date Verbs
	virtual void visit(aq::verb::CurrentDateVerb*);

	// Join Verbs
	virtual void visit(aq::verb::FullJoinVerb*);
	virtual void visit(aq::verb::JoinVerb*);
	virtual void visit(aq::verb::LeftJoinVerb*);
	virtual void visit(aq::verb::RightJoinVerb*);

	// Main Verbs
	virtual void visit(aq::verb::ByVerb*);
	virtual void visit(aq::verb::FromVerb*);
	virtual void visit(aq::verb::GroupVerb*);
	virtual void visit(aq::verb::OrderVerb*);
	virtual void visit(aq::verb::SelectVerb*);
	virtual void visit(aq::verb::WhereVerb*);

	// Over Verbs
	virtual void visit(aq::verb::FrameVerb*);
	virtual void visit(aq::verb::PartitionVerb*);

  // Scalar Vervs
  virtual void visit(aq::verb::ScalarVerb*);
  virtual void visit(aq::verb::SubstringVerb*);

	const std::string& getQuery() { 
    this->query = this->selectStr + this->fromStr + this->whereStr + this->groupStr + this->havingStr + this->orderStr; 
    return this->query;
  }

private:
  std::string selectStr;
  std::string fromStr;
  std::string whereStr;
  std::string groupStr;
  std::string havingStr;
  std::string orderStr;
	std::string leftQuery;
	std::string rightQuery;
	std::string query;
};

}

#endif
