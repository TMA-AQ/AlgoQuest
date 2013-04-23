#include "DumpVisitor.h"
#include "parser/ID2Str.h"
#include <aq/Logger.h>

// Default Verbs
void DumpVisitor::visit(Verb* v)
{
	aq::Logger::getInstance().log(AQ_WARNING, "verb %s not implemented for DumpVisitor\n", id_to_string(v->getVerbType()));
}

// Aggregate Verbs
void DumpVisitor::visit(AggregateVerb*)
{
	assert(false);
}

void DumpVisitor::visit(AvgVerb*)
{
	this->query += " AVG ";
}

void DumpVisitor::visit(CountVerb*)
{
	this->query += " COUNT ";
}

void DumpVisitor::visit(FirstValueVerb*)
{
	this->query += " FIRST_VALUE ";
}

void DumpVisitor::visit(LagVerb*)
{
	this->query += " LAG ";
}

void DumpVisitor::visit(MaxVerb*)
{
	this->query += " MAX ";
}

void DumpVisitor::visit(MinVerb*)
{
	this->query += " MIN ";
}

void DumpVisitor::visit(RowNumberVerb*)
{
	this->query += " ROW_NUMBER ";
}

void DumpVisitor::visit(SumVerb*)
{
	this->query += " SUM ";
}


// Arithmetics Verbs
void DumpVisitor::visit(BinaryVerb*)
{
	assert(false);
}

void DumpVisitor::visit(DivideVerb*)
{
	this->query += " / ";
}

void DumpVisitor::visit(MinusVerb*)
{
	this->query += " - ";
}

void DumpVisitor::visit(MultiplyVerb*)
{
	this->query += " x ";
}

void DumpVisitor::visit(PlusVerb*)
{
	this->query += " + ";
}


// Auxiliary Verbs
void DumpVisitor::visit(AndVerb*)
{
	this->query += " and ";
}

void DumpVisitor::visit(AscVerb*)
{
	this->query += " asc ";
}

void DumpVisitor::visit(AsteriskVerb*)
{
	this->query += " * ";
}

void DumpVisitor::visit(AsVerb* as)
{
	this->query += " as " + as->getIdent() + " ";
}

void DumpVisitor::visit(ColumnVerb* c)
{
	this->query += " " + c->getTableName() + "." + c->getColumnName() + " ";
}

void DumpVisitor::visit(CommaVerb*)
{
	this->query += " , ";
}

void DumpVisitor::visit(DoubleValueVerb*)
{
	this->query += " DoubleValue ";
}

void DumpVisitor::visit(IntValueVerb*)
{
	this->query += " IntValue ";
}

void DumpVisitor::visit(InVerb*)
{
	this->query += " in ";
}

void DumpVisitor::visit(StringValueVerb*)
{
	this->query += " StringValue ";
}


// Case Verbs
void DumpVisitor::visit(CaseVerb*)
{
}

void DumpVisitor::visit(ElseVerb*)
{
}

void DumpVisitor::visit(WhenVerb*)
{
}


// Comparison Verbs
void DumpVisitor::visit(BetweenVerb*)
{
	this->query += " between ";
}

void DumpVisitor::visit(ComparisonVerb*)
{
	assert(false);
}

void DumpVisitor::visit(EqVerb*)
{
	this->query += " = ";
}

void DumpVisitor::visit(GeqVerb*)
{
	this->query += " >= ";
}

void DumpVisitor::visit(GtVerb*)
{
	this->query += " > ";
}

void DumpVisitor::visit(JeqVerb*)
{
	this->query += " = ";
}

void DumpVisitor::visit(JieqVerb*)
{
	this->query += " JIeq ";
}

void DumpVisitor::visit(JinfVerb*)
{
	this->query += " < ";
}

void DumpVisitor::visit(JneqVerb*)
{
	this->query += " != ";
}

void DumpVisitor::visit(JseqVerb*)
{
	this->query += " JSeq ";
}

void DumpVisitor::visit(JsupVerb*)
{
	this->query += " > ";
}

void DumpVisitor::visit(LeqVerb*)
{
	this->query += " <= ";
}

void DumpVisitor::visit(LikeVerb*)
{
	this->query += " like ";
}

void DumpVisitor::visit(LtVerb*)
{
	this->query += " < ";
}

void DumpVisitor::visit(NeqVerb*)
{
	this->query += " != ";
}

void DumpVisitor::visit(NotBetweenVerb*)
{
	this->query += " between ";
}

void DumpVisitor::visit(NotLikeVerb*)
{
	this->query += " not like ";
}


// Conversion Verbs
void DumpVisitor::visit(CastVerb*)
{
}

void DumpVisitor::visit(DecodeVerb*)
{
}

void DumpVisitor::visit(NvlVerb*)
{
}


// Date Verbs
void DumpVisitor::visit(CurrentDateVerb*)
{
}


// Join Verbs
void DumpVisitor::visit(FullJoinVerb*)
{
}

void DumpVisitor::visit(JoinVerb*)
{
}

void DumpVisitor::visit(LeftJoinVerb*)
{
}

void DumpVisitor::visit(RightJoinVerb*)
{
}


// Main Verbs
void DumpVisitor::visit(ByVerb*)
{
}

void DumpVisitor::visit(FromVerb*)
{
  this->fromStr = " FROM " + this->query + "\n";
  this->query = "";
}

void DumpVisitor::visit(GroupVerb*)
{
}

void DumpVisitor::visit(OrderVerb*)
{
}

void DumpVisitor::visit(SelectVerb*)
{
  this->selectStr = " SELECT " + this->query + "\n";
  this->query = "";
}

void DumpVisitor::visit(WhereVerb*)
{
  this->whereStr = " WHERE " + this->query + "\n";
  this->query = "";
}


// Over Verbs
void DumpVisitor::visit(FrameVerb*)
{
}

void DumpVisitor::visit(PartitionVerb*)
{
}
