#include "ApplyRowVisitor.h"

namespace aq
{

ApplyRowVisitor::ApplyRowVisitor()
  :
  rows(1), // fixme
  n(0)
{
}

// Aggregate Verbs
void ApplyRowVisitor::visit(verb::AggregateVerb* agg)
{
  std::for_each(this->rows.begin(), this->rows.end(), [&] (aq::Row& row) {
    agg->addResult(row);
  });
}

void ApplyRowVisitor::visit(verb::AvgVerb*){}
void ApplyRowVisitor::visit(verb::CountVerb*){}
void ApplyRowVisitor::visit(verb::FirstValueVerb*){}
void ApplyRowVisitor::visit(verb::LagVerb*){}
void ApplyRowVisitor::visit(verb::MaxVerb*){}
void ApplyRowVisitor::visit(verb::MinVerb*){}
void ApplyRowVisitor::visit(verb::RowNumberVerb*){}
void ApplyRowVisitor::visit(verb::SumVerb*){}

// Arithmetics Verbs
void ApplyRowVisitor::visit(verb::BinaryVerb*){}
void ApplyRowVisitor::visit(verb::DivideVerb*){}
void ApplyRowVisitor::visit(verb::MinusVerb*){}
void ApplyRowVisitor::visit(verb::MultiplyVerb*){}
void ApplyRowVisitor::visit(verb::PlusVerb*){}

// Auxiliary Verbs
void ApplyRowVisitor::visit(verb::AndVerb*){}
void ApplyRowVisitor::visit(verb::AscVerb*){}
void ApplyRowVisitor::visit(verb::AsteriskVerb*){}

void ApplyRowVisitor::visit(verb::AsVerb* as)
{
  std::for_each(this->rows.begin(), this->rows.end(), [&] (aq::Row& row) {
    as->addResult(row);
  });
}

void ApplyRowVisitor::visit(verb::ColumnVerb* column)
{
  std::for_each(this->rows.begin(), this->rows.end(), [&] (aq::Row& row) {
    column->addResult(row);
  });
}

void ApplyRowVisitor::visit(verb::CommaVerb*){}
void ApplyRowVisitor::visit(verb::DoubleValueVerb*){}
void ApplyRowVisitor::visit(verb::IntValueVerb*){}
void ApplyRowVisitor::visit(verb::InVerb*){}
void ApplyRowVisitor::visit(verb::StringValueVerb*){}

// Case Verbs
void ApplyRowVisitor::visit(verb::CaseVerb*){}
void ApplyRowVisitor::visit(verb::ElseVerb*){}
void ApplyRowVisitor::visit(verb::WhenVerb*){}

// Comparison Verbs
void ApplyRowVisitor::visit(verb::BetweenVerb*){}
void ApplyRowVisitor::visit(verb::ComparisonVerb*){}
void ApplyRowVisitor::visit(verb::EqVerb*){}
void ApplyRowVisitor::visit(verb::GeqVerb*){}
void ApplyRowVisitor::visit(verb::GtVerb*){}
void ApplyRowVisitor::visit(verb::JeqVerb*){}
void ApplyRowVisitor::visit(verb::JieqVerb*){}
void ApplyRowVisitor::visit(verb::JinfVerb*){}
void ApplyRowVisitor::visit(verb::JneqVerb*){}
void ApplyRowVisitor::visit(verb::JseqVerb*){}
void ApplyRowVisitor::visit(verb::JsupVerb*){}
void ApplyRowVisitor::visit(verb::LeqVerb*){}
void ApplyRowVisitor::visit(verb::LikeVerb*){}
void ApplyRowVisitor::visit(verb::LtVerb*){}
void ApplyRowVisitor::visit(verb::NeqVerb*){}
void ApplyRowVisitor::visit(verb::NotBetweenVerb*){}
void ApplyRowVisitor::visit(verb::NotLikeVerb*){}

// Conversion Verbs
void ApplyRowVisitor::visit(verb::CastVerb*){}
void ApplyRowVisitor::visit(verb::DecodeVerb*){}
void ApplyRowVisitor::visit(verb::NvlVerb*){}

// Date Verbs
void ApplyRowVisitor::visit(verb::CurrentDateVerb*){}

// Join Verbs
void ApplyRowVisitor::visit(verb::FullJoinVerb*){}
void ApplyRowVisitor::visit(verb::JoinVerb*){}
void ApplyRowVisitor::visit(verb::LeftJoinVerb*){}
void ApplyRowVisitor::visit(verb::RightJoinVerb*){}

// Main Verbs
void ApplyRowVisitor::visit(verb::ByVerb*)
{
}

void ApplyRowVisitor::visit(verb::FromVerb*)
{
  this->context = K_FROM;
}

void ApplyRowVisitor::visit(verb::GroupVerb* group)
{
  this->context = K_GROUP;
  std::for_each(this->rows.begin(), this->rows.end(), [&] (aq::Row& row) {
    group->addResult(row);
  });
}

void ApplyRowVisitor::visit(verb::OrderVerb*)
{
  this->context = K_ORDER;
}

void ApplyRowVisitor::visit(verb::SelectVerb*)
{
  this->context = K_SELECT;
}

void ApplyRowVisitor::visit(verb::WhereVerb*)
{
  this->context = K_WHERE;
}

// Over Verbs
void ApplyRowVisitor::visit(verb::FrameVerb*){}
void ApplyRowVisitor::visit(verb::PartitionVerb*){}

// Scalar Vervs
void ApplyRowVisitor::visit(verb::ScalarVerb*){}
void ApplyRowVisitor::visit(verb::SubstringVerb*){}

}