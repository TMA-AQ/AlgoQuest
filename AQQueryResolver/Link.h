#ifndef __LINK_H__
#define __LINK_H__

// i need this to force static link: all verb are registered statically at init
// todo: find a best way to do this
#include <aq/verbs/ArithmeticVerbs.h>
#include <aq/verbs/AggregateVerbs.h>
#include <aq/verbs/ArithmeticVerbs.h>
#include <aq/verbs/AuxiliaryVerbs.h>
#include <aq/verbs/CaseVerb.h>
#include <aq/verbs/ComparisonVerbs.h>
#include <aq/verbs/ConversionVerbs.h>
#include <aq/verbs/DateVerbs.h>
#include <aq/verbs/JoinVerbs.h>
#include <aq/verbs/MainVerbs.h>
#include <aq/verbs/OverVerbs.h>
#include <aq/verbs/ScalarVerbs.h>
#include <aq/verbs/Verb.h>

void verb_register()
{
  aq::verb::AggregateVerb AggregateVerb_Var;
  aq::verb::SumVerb SumVerb_Var;
  aq::verb::CountVerb CountVerb_Var;
  aq::verb::AvgVerb AvgVerb_Var;
  aq::verb::MinVerb MinVerb_Var;
  aq::verb::MaxVerb MaxVerb_Var;
  aq::verb::FirstValueVerb FirstValueVerb_Var;
  aq::verb::LagVerb LagVerb_Var;
  aq::verb::RowNumberVerb RowNumberVerb_Var;
  aq::verb::BinaryVerb BinaryVerb_Var;
  aq::verb::MinusVerb MinusVerb_Var;
  aq::verb::PlusVerb PlusVerb_Var;
  aq::verb::MultiplyVerb MultiplyVerb_Var;
  aq::verb::DivideVerb DivideVerb_Var;
  aq::verb::ColumnVerb ColumnVerb_Var;
  aq::verb::CommaVerb CommaVerb_Var;
  aq::verb::AndVerb AndVerb_Var;
  aq::verb::InVerb InVerb_Var;
  aq::verb::IntValueVerb IntValueVerb_Var;
  aq::verb::DoubleValueVerb DoubleValueVerb_Var;
  aq::verb::StringValueVerb StringValueVerb_Var;
  aq::verb::AsVerb AsVerb_Var;
  aq::verb::AsteriskVerb AsteriskVerb_Var;
  aq::verb::AscVerb AscVerb_Var;
  aq::verb::CaseVerb CaseVerb_Var;
  aq::verb::WhenVerb WhenVerb_Var;
  aq::verb::ElseVerb ElseVerb_Var;
  aq::verb::ComparisonVerb ComparisonVerb_Var;
  aq::verb::EqVerb EqVerb_Var;
  aq::verb::JeqVerb JeqVerb_Var;
  aq::verb::LtVerb LtVerb_Var;
  aq::verb::LeqVerb LeqVerb_Var;
  aq::verb::GtVerb GtVerb_Var;
  aq::verb::GeqVerb GeqVerb_Var;
  aq::verb::BetweenVerb BetweenVerb_Var;
  aq::verb::NotBetweenVerb NotBetweenVerb_Var;
  aq::verb::LikeVerb LikeVerb_Var;
  aq::verb::NotLikeVerb NotLikeVerb_Var;
  aq::verb::JinfVerb JinfVerb_Var;
  aq::verb::JieqVerb JieqVerb_Var;
  aq::verb::JsupVerb JsupVerb_Var;
  aq::verb::JseqVerb JseqVerb_Var;
  aq::verb::NeqVerb NeqVerb_Var;
  aq::verb::JneqVerb JneqVerb_Var;
  aq::verb::CastVerb CastVerb_Var;
  aq::verb::NvlVerb NvlVerb_Var;
  aq::verb::DecodeVerb DecodeVerb_Var;
  aq::verb::CurrentDateVerb CurrentDateVerb_Var;
  aq::verb::JoinVerb JoinVerb_Var;
  aq::verb::LeftJoinVerb LeftJoinVerb_Var;
  aq::verb::RightJoinVerb RightJoinVerb_Var;
  aq::verb::FullJoinVerb FullJoinVerb_Var;
  aq::verb::SelectVerb SelectVerb_Var;
  aq::verb::WhereVerb WhereVerb_Var;
  aq::verb::OrderVerb OrderVerb_Var;
  aq::verb::ByVerb ByVerb_Var;
  aq::verb::FromVerb FromVerb_Var;
  aq::verb::GroupVerb GroupVerb_Var;
  aq::verb::PartitionVerb PartitionVerb_Var;
  aq::verb::FrameVerb FrameVerb_Var;
  aq::verb::ScalarVerb ScalarVerb_Var;
  aq::verb::SqrtVerb SqrtVerb_Var;
  aq::verb::AbsVerb AbsVerb_Var;
  aq::verb::SubstringVerb SubstringVerb_Var;
  aq::verb::ToDateVerb ToDateVerb_Var;
  aq::verb::YearVerb YearVerb_Var;
  aq::verb::MonthVerb MonthVerb_Var;
  aq::verb::DayVerb DayVerb_Var;
  aq::verb::ToCharVerb ToCharVerb_Var;
  aq::verb::DateVerb DateVerb_Var;
  aq::verb::SelectVerb Select_Var;
  ///////////////////////////////
}

#endif