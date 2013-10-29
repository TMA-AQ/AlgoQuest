#pragma once

#include "VerbNode.h"
#include <aq/DBTypes.h>

namespace aq {
namespace verb {

//------------------------------------------------------------------------------
class ScalarVerb: public VerbNode
{
public:
	virtual int getVerbType() const { return -1; };
	virtual bool changeQuery( aq::tnode* pStart, aq::tnode* pNode,
		VerbResult::Ptr resLeft, VerbResult::Ptr resRight, VerbResult::Ptr resNext );
	virtual void changeResult( Table::Ptr table, 
		VerbResult::Ptr resLeft, VerbResult::Ptr resRight, VerbResult::Ptr resNext );
	virtual void addResult(aq::Row& row);
  virtual void accept(VerbVisitor* visitor);
protected:
	virtual void computeResult( VerbResult::Ptr param );
	virtual void transformItem( const ColumnItem& item, ColumnItem& result ){};
	virtual aq::ColumnType outputType( aq::ColumnType inputType ){ return aq::ColumnType::COL_TYPE_INT; };
};

//------------------------------------------------------------------------------
class SqrtVerb: public ScalarVerb
{
public:
	virtual int getVerbType() const { return K_SQRT; }
protected:
	virtual void transformItem( const ColumnItem& item, ColumnItem& result );
	virtual aq::ColumnType outputType( aq::ColumnType inputType );
};

//------------------------------------------------------------------------------
class AbsVerb: public ScalarVerb
{
public:
	virtual int getVerbType() const { return K_ABS; }
protected:
	virtual void transformItem( const ColumnItem& item, ColumnItem& result );
	virtual aq::ColumnType outputType( aq::ColumnType inputType );
};

//------------------------------------------------------------------------------
class SubstringVerb: public ScalarVerb
{
public:
  SubstringVerb();
	virtual int getVerbType() const { return K_SUBSTRING; }
	bool preprocessQuery( aq::tnode* pStart, aq::tnode* pNode, aq::tnode* pStartOriginal );
  void accept(VerbVisitor*);
  llong getStartPos() const { return StartPos; }
  llong getSize() const { return Size; }
protected:
	virtual void transformItem( const ColumnItem& item, ColumnItem& result );
	virtual aq::ColumnType outputType( aq::ColumnType inputType );
	llong StartPos, Size;
};

//------------------------------------------------------------------------------
class ToDateVerb: public ScalarVerb
{
public:
	virtual int getVerbType() const { return K_TO_DATE; };
	bool preprocessQuery( aq::tnode* pStart, aq::tnode* pNode, aq::tnode* pStartOriginal );
protected:
	virtual void transformItem( const ColumnItem& item, ColumnItem& result );
	virtual aq::ColumnType outputType( aq::ColumnType inputType );
	aq::ColumnType OutputType;
};

//------------------------------------------------------------------------------
class YearVerb: public ScalarVerb
{
public:
	virtual int getVerbType() const { return K_YEAR; };
protected:
	virtual void transformItem( const ColumnItem& item, ColumnItem& result );
	virtual aq::ColumnType outputType( aq::ColumnType inputType );
};

//------------------------------------------------------------------------------
class MonthVerb: public ScalarVerb
{
public:
	virtual int getVerbType() const { return K_MONTH; };
protected:
	virtual void transformItem( const ColumnItem& item, ColumnItem& result );
	virtual aq::ColumnType outputType( aq::ColumnType inputType );
};

//------------------------------------------------------------------------------
class DayVerb: public ScalarVerb
{
public:
	virtual int getVerbType() const { return K_DAY; };
protected:
	virtual void transformItem( const ColumnItem& item, ColumnItem& result );
	virtual aq::ColumnType outputType( aq::ColumnType inputType );
};

//------------------------------------------------------------------------------
class ToCharVerb: public ScalarVerb
{
public:
	virtual int getVerbType() const { return K_TO_CHAR; };
	virtual bool changeQuery(	aq::tnode* pStart, aq::tnode* pNode,
								VerbResult::Ptr resLeft, 
								VerbResult::Ptr resRight, 
								VerbResult::Ptr resNext );
protected:
	virtual void transformItem( const ColumnItem& item, ColumnItem& result );
	virtual aq::ColumnType outputType( aq::ColumnType inputType );
	aq::ColumnType InputType;
	std::string Format;
};

//------------------------------------------------------------------------------
class DateVerb: public ScalarVerb
{
public:
	virtual int getVerbType() const { return K_DATE; };
protected:
	virtual void transformItem( const ColumnItem& item, ColumnItem& result );
	virtual aq::ColumnType outputType( aq::ColumnType inputType );
};

}
}