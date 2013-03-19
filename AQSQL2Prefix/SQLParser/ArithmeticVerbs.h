#pragma once

#include "Verb.h"

//------------------------------------------------------------------------------
class BinaryVerb: public Verb
{
	VERB_DECLARE( BinaryVerb );
public:
	virtual int getVerbType() const { return -1; };
	virtual bool changeQuery( tnode* pStart, tnode* pNode,
		VerbResult::Ptr resLeft, VerbResult::Ptr resRight, VerbResult::Ptr resNext );
	virtual void changeResult( Table::Ptr table, 
		VerbResult::Ptr resLeft, VerbResult::Ptr resRight, VerbResult::Ptr resNext );
protected:
	virtual void computeResult( VerbResult::Ptr param1, VerbResult::Ptr param2 );
	virtual void transformItem( const ColumnItem& item1, const ColumnItem& item2, 
								ColumnType resultType, ColumnItem& result ){};
	virtual ColumnType outputType( ColumnType inputType1, ColumnType inputType2 )
	{ return COL_TYPE_INT; };
};

//------------------------------------------------------------------------------
class MinusVerb: public BinaryVerb
{
	VERB_DECLARE( MinusVerb );
public:
	virtual int getVerbType() const { return K_MINUS; };
protected:
	virtual void transformItem( const ColumnItem& item1, const ColumnItem& item2, 
		ColumnType resultType, ColumnItem& result );
	virtual ColumnType outputType( ColumnType inputType1, ColumnType inputType2 );
};

//------------------------------------------------------------------------------
class PlusVerb: public BinaryVerb
{
	VERB_DECLARE( PlusVerb );
public:
	virtual int getVerbType() const { return K_PLUS; };
protected:
	virtual void transformItem( const ColumnItem& item1, const ColumnItem& item2, 
		ColumnType resultType, ColumnItem& result );
	virtual ColumnType outputType( ColumnType inputType1, ColumnType inputType2 );
};

//------------------------------------------------------------------------------
class MultiplyVerb: public BinaryVerb
{
	VERB_DECLARE( MultiplyVerb );
public:
	virtual int getVerbType() const { return K_MUL; };
protected:
	virtual void transformItem( const ColumnItem& item1, const ColumnItem& item2, 
		ColumnType resultType, ColumnItem& result );
	virtual ColumnType outputType( ColumnType inputType1, ColumnType inputType2 );
};

//------------------------------------------------------------------------------
class DivideVerb: public BinaryVerb
{
	VERB_DECLARE( DivideVerb );
public:
	virtual int getVerbType() const { return K_DIV; };
protected:
	virtual void transformItem( const ColumnItem& item1, const ColumnItem& item2, 
		ColumnType resultType, ColumnItem& result );
	virtual ColumnType outputType( ColumnType inputType1, ColumnType inputType2 );
};