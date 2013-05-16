#ifndef __LINK_H__
#define __LINK_H__

// i need this to force static link: all verb are registered statically at init
// todo: find a best way to do this
#include "aq/verbs/ArithmeticVerbs.h"
#include "aq/verbs/AggregateVerbs.h"
#include "aq/verbs/ArithmeticVerbs.h"
#include "aq/verbs/AuxiliaryVerbs.h"
#include "aq/verbs/CaseVerb.h"
#include "aq/verbs/ComparisonVerbs.h"
#include "aq/verbs/ConversionVerbs.h"
#include "aq/verbs/DateVerbs.h"
#include "aq/verbs/JoinVerbs.h"
#include "aq/verbs/MainVerbs.h"
#include "aq/verbs/OverVerbs.h"
#include "aq/verbs/ScalarVerbs.h"
#include "aq/verbs/Verb.h"

void verb_register()
{
	AggregateVerb AggregateVerb_Var;
	SumVerb SumVerb_Var;
	CountVerb CountVerb_Var;
	AvgVerb AvgVerb_Var;
	MinVerb MinVerb_Var;
	MaxVerb MaxVerb_Var;
	FirstValueVerb FirstValueVerb_Var;
	LagVerb LagVerb_Var;
	RowNumberVerb RowNumberVerb_Var;
	BinaryVerb BinaryVerb_Var;
	MinusVerb MinusVerb_Var;
	PlusVerb PlusVerb_Var;
	MultiplyVerb MultiplyVerb_Var;
	DivideVerb DivideVerb_Var;
	ColumnVerb ColumnVerb_Var;
	CommaVerb CommaVerb_Var;
	AndVerb AndVerb_Var;
	InVerb InVerb_Var;
	IntValueVerb IntValueVerb_Var;
	DoubleValueVerb DoubleValueVerb_Var;
	StringValueVerb StringValueVerb_Var;
	AsVerb AsVerb_Var;
	AsteriskVerb AsteriskVerb_Var;
	AscVerb AscVerb_Var;
	CaseVerb CaseVerb_Var;
	WhenVerb WhenVerb_Var;
	ElseVerb ElseVerb_Var;
	ComparisonVerb ComparisonVerb_Var;
	EqVerb EqVerb_Var;
	JeqVerb JeqVerb_Var;
	LtVerb LtVerb_Var;
	LeqVerb LeqVerb_Var;
	GtVerb GtVerb_Var;
	GeqVerb GeqVerb_Var;
	BetweenVerb BetweenVerb_Var;
	NotBetweenVerb NotBetweenVerb_Var;
	LikeVerb LikeVerb_Var;
	NotLikeVerb NotLikeVerb_Var;
	JinfVerb JinfVerb_Var;
	JieqVerb JieqVerb_Var;
	JsupVerb JsupVerb_Var;
	JseqVerb JseqVerb_Var;
	NeqVerb NeqVerb_Var;
	JneqVerb JneqVerb_Var;
	CastVerb CastVerb_Var;
	NvlVerb NvlVerb_Var;
	DecodeVerb DecodeVerb_Var;
	CurrentDateVerb CurrentDateVerb_Var;
	JoinVerb JoinVerb_Var;
	LeftJoinVerb LeftJoinVerb_Var;
	RightJoinVerb RightJoinVerb_Var;
	FullJoinVerb FullJoinVerb_Var;
	SelectVerb SelectVerb_Var;
	WhereVerb WhereVerb_Var;
	OrderVerb OrderVerb_Var;
	ByVerb ByVerb_Var;
	FromVerb FromVerb_Var;
	GroupVerb GroupVerb_Var;
	PartitionVerb PartitionVerb_Var;
	FrameVerb FrameVerb_Var;
	ScalarVerb ScalarVerb_Var;
	SqrtVerb SqrtVerb_Var;
	AbsVerb AbsVerb_Var;
	SubstringVerb SubstringVerb_Var;
	ToDateVerb ToDateVerb_Var;
	YearVerb YearVerb_Var;
	MonthVerb MonthVerb_Var;
	DayVerb DayVerb_Var;
	ToCharVerb ToCharVerb_Var;
	DateVerb DateVerb_Var;
	SelectVerb Select_Var;
	///////////////////////////////
}

#endif