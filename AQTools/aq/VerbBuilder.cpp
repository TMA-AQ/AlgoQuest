#include "VerbBuilder.h"

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

namespace aq
{

  aq::verb::VerbNode::Ptr VerbBuilder::build(unsigned int type) const
  {
    aq::verb::VerbNode::Ptr v;
    switch (type)
    {

      // aggregate verbs
    case K_SUM: v = new aq::verb::SumVerb; break;
    case K_COUNT: v = new aq::verb::CountVerb; break;
    case K_AVG: v = new aq::verb::AvgVerb; break;
    case K_MIN: v = new aq::verb::MinVerb; break;
    case K_MAX: v = new aq::verb::MaxVerb; break;
    case K_FIRST_VALUE: v = new aq::verb::FirstValueVerb; break;
    case K_LAG: v = new aq::verb::LagVerb; break;
    case K_ROW_NUMBER: v = new aq::verb::RowNumberVerb; break;
    
      // arithmetics verbs
    case K_MINUS: v = new aq::verb::MinusVerb; break;
    case K_PLUS: v = new aq::verb::PlusVerb; break;
    case K_MUL: v = new aq::verb::MultiplyVerb; break;
    case K_DIV: v = new aq::verb::DivideVerb; break;

      // auxiliary verbs
    case K_PERIOD: v = new aq::verb::ColumnVerb; break;
    case K_COMMA: v = new aq::verb::CommaVerb; break;
    case K_AND: v = new aq::verb::AndVerb; break;
    case K_IN: v = new aq::verb::InVerb; break;
    case K_INTEGER: v = new aq::verb::IntValueVerb; break;
    case K_REAL: v = new aq::verb::DoubleValueVerb; break;
    case K_STRING: v = new aq::verb::StringValueVerb; break;
    case K_AS: v = new aq::verb::AsVerb; break;
    case K_STAR: v = new aq::verb::AsteriskVerb; break;
    case K_ASC: v = new aq::verb::AscVerb; break;

      // case verbs
    case K_CASE: v = new aq::verb::CaseVerb; break;
    case K_WHEN: v = new aq::verb::WhenVerb; break;
    case K_ELSE: v = new aq::verb::ElseVerb; break;

      // comparison verbs
    case K_EQ: v = new aq::verb::EqVerb; break;
    case K_JEQ: v = new aq::verb::JeqVerb; break;
    case K_JAUTO: v = new aq::verb::JautoVerb; break;
    case K_LT: v = new aq::verb::LtVerb; break;
    case K_LEQ: v = new aq::verb::LeqVerb; break;
    case K_GT: v = new aq::verb::GtVerb; break;
    case K_GEQ: v = new aq::verb::GeqVerb; break;
    case K_BETWEEN: v = new aq::verb::BetweenVerb; break;
    case K_NOT_BETWEEN: v = new aq::verb::NotBetweenVerb; break;
    case K_LIKE: v = new aq::verb::LikeVerb; break;
    case K_NOT_LIKE: v = new aq::verb::NotLikeVerb; break;
    case K_JINF: v = new aq::verb::JinfVerb; break;
    case K_JIEQ: v = new aq::verb::JieqVerb; break;
    case K_JSUP: v = new aq::verb::JsupVerb; break;
    case K_JSEQ: v = new aq::verb::JseqVerb; break;
    case K_NEQ: v = new aq::verb::NeqVerb; break;
    case K_JNEQ: v = new aq::verb::JneqVerb; break;

      // conversion verbs
    case K_CAST: v = new aq::verb::CastVerb; break;
    case K_NVL: v = new aq::verb::NvlVerb; break;
    case K_DECODE: v = new aq::verb::DecodeVerb; break;

      // date verbs
    case K_CURRENT_DATE: v = new aq::verb::CurrentDateVerb; break;
    
      // join verbs
    case K_JOIN: v = new aq::verb::JoinVerb; break;
    case K_LEFT: v = new aq::verb::LeftJoinVerb; break;
    case K_RIGHT: v = new aq::verb::RightJoinVerb; break;
    case K_FULL: v = new aq::verb::FullJoinVerb; break;

      // main verbs
    case K_SELECT: v = new aq::verb::SelectVerb; break;
    case K_FROM: v = new aq::verb::FromVerb; break;
    case K_WHERE: v = new aq::verb::WhereVerb; break;
    case K_GROUP: v = new aq::verb::GroupVerb; break;
    case K_ORDER: v = new aq::verb::OrderVerb; break;
    case K_BY: v = new aq::verb::ByVerb; break;
    case K_PARTITION: v = new aq::verb::PartitionVerb; break;
    case K_FRAME: v = new aq::verb::FrameVerb; break;

      // scalar verbs
    case K_SQRT: v = new aq::verb::SqrtVerb; break;
    case K_ABS: v = new aq::verb::AbsVerb; break;
    case K_SUBSTRING: v = new aq::verb::SubstringVerb; break;
    case K_TO_DATE: v = new aq::verb::ToDateVerb; break;
    case K_YEAR: v = new aq::verb::YearVerb; break;
    case K_MONTH: v = new aq::verb::MonthVerb; break;
    case K_DAY: v = new aq::verb::DayVerb; break;
    case K_TO_CHAR: v = new aq::verb::ToCharVerb; break;
    case K_DATE: v = new aq::verb::DateVerb; break;
    }

    assert(!v.get() || (v->getVerbType() == type));

    return v;
  }
};