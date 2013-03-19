#pragma once

#include "Verb.h"

//------------------------------------------------------------------------------
class ScalarVerb: public Verb
{
	VERB_DECLARE( ScalarVerb );
public:
	virtual int getVerbType() const { return -1; };
	virtual bool changeQuery( tnode* pStart, tnode* pNode,
		VerbResult::Ptr resLeft, VerbResult::Ptr resRight, VerbResult::Ptr resNext );
	virtual void changeResult( Table::Ptr table, 
		VerbResult::Ptr resLeft, VerbResult::Ptr resRight, VerbResult::Ptr resNext );
protected:
	virtual void computeResult( VerbResult::Ptr param );
	virtual void transformItem( const ColumnItem& item, ColumnItem& result ){};
	virtual ColumnType outputType( ColumnType inputType ){ return COL_TYPE_INT; };
};

//------------------------------------------------------------------------------
class SqrtVerb: public ScalarVerb
{
	VERB_DECLARE( SqrtVerb );
public:
	virtual int getVerbType() const { return K_SQRT; }
protected:
	virtual void transformItem( const ColumnItem& item, ColumnItem& result );
	virtual ColumnType outputType( ColumnType inputType );
};

//------------------------------------------------------------------------------
class AbsVerb: public ScalarVerb
{
	VERB_DECLARE( AbsVerb );
public:
	virtual int getVerbType() const { return K_ABS; }
protected:
	virtual void transformItem( const ColumnItem& item, ColumnItem& result );
	virtual ColumnType outputType( ColumnType inputType );
};

//------------------------------------------------------------------------------
class SubstringVerb: public ScalarVerb
{
	VERB_DECLARE( SubstringVerb );
public:
	virtual int getVerbType() const { return K_SUBSTRING; }
	bool preprocessQuery( tnode* pStart, tnode* pNode, tnode* pStartOriginal );
protected:
	virtual void transformItem( const ColumnItem& item, ColumnItem& result );
	virtual ColumnType outputType( ColumnType inputType );
	llong StartPos, Size;
};

//------------------------------------------------------------------------------
class ToDateVerb: public ScalarVerb
{
	VERB_DECLARE( ToDateVerb );
public:
	virtual int getVerbType() const { return K_TO_DATE; };
	bool preprocessQuery( tnode* pStart, tnode* pNode, tnode* pStartOriginal );
protected:
	virtual void transformItem( const ColumnItem& item, ColumnItem& result );
	virtual ColumnType outputType( ColumnType inputType );
	ColumnType OutputType;
};

//------------------------------------------------------------------------------
class YearVerb: public ScalarVerb
{
	VERB_DECLARE( YearVerb );
public:
	virtual int getVerbType() const { return K_YEAR; };
protected:
	virtual void transformItem( const ColumnItem& item, ColumnItem& result );
	virtual ColumnType outputType( ColumnType inputType );
};

//------------------------------------------------------------------------------
class MonthVerb: public ScalarVerb
{
	VERB_DECLARE( MonthVerb );
public:
	virtual int getVerbType() const { return K_MONTH; };
protected:
	virtual void transformItem( const ColumnItem& item, ColumnItem& result );
	virtual ColumnType outputType( ColumnType inputType );
};

//------------------------------------------------------------------------------
class DayVerb: public ScalarVerb
{
	VERB_DECLARE( DayVerb );
public:
	virtual int getVerbType() const { return K_DAY; };
protected:
	virtual void transformItem( const ColumnItem& item, ColumnItem& result );
	virtual ColumnType outputType( ColumnType inputType );
};

//------------------------------------------------------------------------------
class ToCharVerb: public ScalarVerb
{
	VERB_DECLARE( ToCharVerb );
public:
	virtual int getVerbType() const { return K_TO_CHAR; };
	virtual bool changeQuery(	tnode* pStart, tnode* pNode,
								VerbResult::Ptr resLeft, 
								VerbResult::Ptr resRight, 
								VerbResult::Ptr resNext );
protected:
	virtual void transformItem( const ColumnItem& item, ColumnItem& result );
	virtual ColumnType outputType( ColumnType inputType );
	ColumnType InputType;
	std::string Format;
};

//------------------------------------------------------------------------------
class DateVerb: public ScalarVerb
{
	VERB_DECLARE( DateVerb );
public:
	virtual int getVerbType() const { return K_DATE; };
protected:
	virtual void transformItem( const ColumnItem& item, ColumnItem& result );
	virtual ColumnType outputType( ColumnType inputType );
};