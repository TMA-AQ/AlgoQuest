#include "ScalarVerbs.h"
#include "VerbVisitor.h"
#include <cmath>
#include <aq/Exceptions.h>
#include <aq/DateConversion.h>

using namespace std;

namespace aq {
namespace verb {

//------------------------------------------------------------------------------
void ScalarVerb::computeResult( VerbResult::Ptr param )
{
	assert( param );
	Scalar::Ptr scalar = boost::dynamic_pointer_cast<Scalar>( param );

	if( scalar )
	{
		ColumnType type = this->outputType(scalar->Type);
		Scalar::Ptr result = new Scalar( type );
		this->transformItem( scalar->Item, result->Item );
		this->Result = result;
		return;
	}

	//debug13 - there should be an easier way to do this
	VerbResultArray::Ptr resArray = boost::dynamic_pointer_cast<VerbResultArray>(param);
	Column::Ptr column = NULL;
	if( resArray )
	{
		assert(resArray->Results.size() > 0);
		column = boost::dynamic_pointer_cast<Column>(resArray->Results[0]);
	}
	else
	  column = boost::dynamic_pointer_cast<Column>( param );
	assert( column );
	
	ColumnType type = this->outputType(column->Type);
	Column::Ptr newColumn = new Column(type); //debug13 - I used size 0 because column
	//size is only important when loading
	newColumn->Items.resize( column->Items.size() );
	for( size_t idx = 0; idx < column->Items.size(); ++idx )
		if( column->Items[idx] )
		{
			newColumn->Items[idx] = ColumnItem::Ptr(new ColumnItem());
			this->transformItem( *column->Items[idx], *newColumn->Items[idx] );
		}
		else
			newColumn->Items[idx] = NULL;

	newColumn->setCount( column->getCount() );
	this->Result = newColumn;
}

//------------------------------------------------------------------------------
bool ScalarVerb::changeQuery(	aq::tnode* pStart, aq::tnode* pNode,
								VerbResult::Ptr resLeft, 
								VerbResult::Ptr resRight, 
								VerbResult::Ptr resNext )
{
	//debug13 - there should be an easier way to do this
  VerbResultArray::Ptr resArray = boost::dynamic_pointer_cast<VerbResultArray>(resLeft);
	Scalar::Ptr scalar = NULL;
	if( resArray )
	{
		assert(resArray->Results.size() > 0);
		scalar = boost::dynamic_pointer_cast<Scalar>(resArray->Results[0]);
	}
	else
	  scalar = boost::dynamic_pointer_cast<Scalar>( resLeft );
	if( !scalar )
		return false;
	this->computeResult( scalar );
	scalar = boost::dynamic_pointer_cast<Scalar>( this->Result );
	assert( scalar );
	pNode->inf = 1;
	this->Disabled = true;
	delete_subtree( pNode->left );		pNode->left = NULL;
	delete_subtree( pNode->right );		pNode->right = NULL;
	pNode->set_data( scalar->getValue(), scalar->Type );
	return true;
}

//------------------------------------------------------------------------------
void ScalarVerb::changeResult(	Table::Ptr table, 
								VerbResult::Ptr resLeft, 
								VerbResult::Ptr resRight, 
								VerbResult::Ptr resNext )
{
	if( this->Result )
		return;
	assert( resLeft );
	this->computeResult( resLeft );
}

//------------------------------------------------------------------------------
void ScalarVerb::addResult(aq::Row& row)
{
	assert(false);
  //this->computeResult( resLeft );
  //
  //Scalar * scalar = dynamic_cast<Scalar*>(this->Result.get());
  //if (scalar != 0)
  //{
  //  ColumnItem::Ptr item(new ColumnItem(scalar->Item));
  //  row.computedRow.push_back(aq::row_item_t(item, scalar->Type, scalar->Size, "", ""));
  //}
}

//------------------------------------------------------------------------------
void ScalarVerb::accept(VerbVisitor* visitor)
{
  visitor->visit(this);
}

//------------------------------------------------------------------------------
void SqrtVerb::transformItem( const ColumnItem& item, ColumnItem& result )
{
	result.numval = sqrt( item.numval );
}

//------------------------------------------------------------------------------
ColumnType SqrtVerb::outputType( ColumnType inputType )
{
	if( inputType != COL_TYPE_BIG_INT &&
		inputType != COL_TYPE_INT &&
		inputType != COL_TYPE_DOUBLE
		)
		throw verb_error(generic_error::VERB_TYPE_MISMATCH, this->getVerbType() );
	return COL_TYPE_DOUBLE;
}

//------------------------------------------------------------------------------
void AbsVerb::transformItem( const ColumnItem& item, ColumnItem& result )
{
	result.numval = fabs( item.numval );
}

//------------------------------------------------------------------------------
ColumnType AbsVerb::outputType( ColumnType inputType )
{
	if( inputType != COL_TYPE_BIG_INT &&
		inputType != COL_TYPE_INT &&
		inputType != COL_TYPE_DOUBLE
		)
		throw verb_error(generic_error::VERB_TYPE_MISMATCH, this->getVerbType() );
	return inputType;
}

//------------------------------------------------------------------------------
SubstringVerb::SubstringVerb(): StartPos(1), Size(-1)
{}

//------------------------------------------------------------------------------
void SubstringVerb::transformItem( const ColumnItem& item, ColumnItem& result )
{
	if( this->StartPos < 0 )
		this->StartPos = item.strval.length() - (-this->StartPos);
	if( this->Size < 0 )
		this->Size = item.strval.length() - this->StartPos;
	result.strval = item.strval.substr((int)this->StartPos, (int)this->Size);
}

//------------------------------------------------------------------------------
ColumnType SubstringVerb::outputType( ColumnType inputType )
{
	if( inputType != COL_TYPE_VARCHAR )
		throw verb_error(generic_error::VERB_TYPE_MISMATCH, this->getVerbType() );
	return inputType;
}

//------------------------------------------------------------------------------
bool SubstringVerb::preprocessQuery( aq::tnode* pStart, aq::tnode* pNode, aq::tnode* pStartOriginal )
{
	//debug13 - should have a K_VALUE verb that returns a scalar, but for now
	//I inspect the query directly, in order to avoid the potential ripple effects
	//caused by implementing such a verb
	assert( pNode );
	assert( pNode->tag == K_SUBSTRING );
	assert( pNode->left );
	//assert( pNode->left->tag == K_COMMA ); - allow column value to rise up
	//pNode->left->inf = 1;
	assert( pNode->left->right );
	if( pNode->left->right->tag == K_COMMA )
	{
		assert( pNode->left->right->left->tag == K_INTEGER );
		assert( pNode->left->right->right->tag == K_INTEGER );
		this->StartPos = pNode->left->right->left->getData().val_int;
		this->Size = pNode->left->right->right->getData().val_int;
		pNode->left->right->inf = 1;
	}
	else
	{
		assert( pNode->left->right->tag == K_INTEGER );
		this->StartPos = pNode->left->right->getData().val_int;
	}
	if( this->StartPos == 0 )
		this->StartPos = 1;
	if( this->StartPos > 0 )
		--this->StartPos; //SQL SUBSTRING indexing is 1-based, C++ substr is 0-based
	return false;
}

void SubstringVerb::accept(VerbVisitor* visitor)
{
  visitor->visit(this);
}

//------------------------------------------------------------------------------
bool ToDateVerb::preprocessQuery( aq::tnode* pStart, aq::tnode* pNode, aq::tnode* pStartOriginal )
{
	assert( pNode );
	this->OutputType = COL_TYPE_DATE;
	if( pNode->right )
	{
		assert( pNode->right->tag == K_STRING );
	}
	return false;
}

//------------------------------------------------------------------------------
void ToDateVerb::transformItem( const ColumnItem& item, ColumnItem& result )
{
	DateConversion dateConverter;
	llong intval = dateConverter.dateToBigInt(item.strval.c_str());
	result = ColumnItem( (double) intval );
}

//------------------------------------------------------------------------------
ColumnType ToDateVerb::outputType( ColumnType inputType )
{
	if( inputType != COL_TYPE_VARCHAR )
		throw verb_error(generic_error::VERB_TYPE_MISMATCH, this->getVerbType() );
	
	return this->OutputType;
}

//------------------------------------------------------------------------------
void YearVerb::transformItem( const ColumnItem& item, ColumnItem& result )
{
  // TODO
	// int year, month, day, hour, minute, second, millisecond;
  // dateToParts( (llong) item.numval, year, month, day, hour, minute, second, millisecond );
	
  assert(false);
  throw aq::generic_error(aq::generic_error::NOT_IMPLEMENTED, "YearVerb not supported");

  result = ColumnItem(0.0);
}

//------------------------------------------------------------------------------
ColumnType YearVerb::outputType( ColumnType inputType )
{
	if (inputType != COL_TYPE_DATE)
		throw verb_error(generic_error::VERB_TYPE_MISMATCH, this->getVerbType() );

	return COL_TYPE_INT;
}

//------------------------------------------------------------------------------
void MonthVerb::transformItem( const ColumnItem& item, ColumnItem& result )
{
  // TODO
	// int year, month, day, hour, minute, second, millisecond;
	// dateToParts( (llong) item.numval, year, month, day, hour, minute, second, millisecond );

  assert(false);
  throw aq::generic_error(aq::generic_error::NOT_IMPLEMENTED, "MonthVerb not supported");

	result = ColumnItem(0.0);
}

//------------------------------------------------------------------------------
ColumnType MonthVerb::outputType( ColumnType inputType )
{
	if (inputType != COL_TYPE_DATE)
		throw verb_error(generic_error::VERB_TYPE_MISMATCH, this->getVerbType() );

	return COL_TYPE_INT;
}

//------------------------------------------------------------------------------
void DayVerb::transformItem( const ColumnItem& item, ColumnItem& result )
{
  // TODO
	// int year, month, day, hour, minute, second, millisecond;
  // dateToParts( (llong) item.numval, year, month, day, hour, minute, second, millisecond );
  
  assert(false);
  throw aq::generic_error(aq::generic_error::NOT_IMPLEMENTED, "DayVerb not supported");

	result = ColumnItem(0.0);
}

//------------------------------------------------------------------------------
ColumnType DayVerb::outputType( ColumnType inputType )
{
	if (inputType != COL_TYPE_DATE)
		throw verb_error(generic_error::VERB_TYPE_MISMATCH, this->getVerbType() );

	return COL_TYPE_INT;
}

//------------------------------------------------------------------------------
void Convert(	ColumnItem& dest, int destType, 
				const ColumnItem& source, int sourceType );

//------------------------------------------------------------------------------
bool ToCharVerb::changeQuery(	aq::tnode* pStart, aq::tnode* pNode,
								VerbResult::Ptr resLeft, 
								VerbResult::Ptr resRight, 
								VerbResult::Ptr resNext )
{
	if( resRight )
	{
		if( resRight->getType() != VerbResult::SCALAR )
			throw verb_error(generic_error::INVALID_QUERY, this->getVerbType());
		Scalar::Ptr scalar = boost::dynamic_pointer_cast<Scalar>( resRight );
		if( scalar->Type != COL_TYPE_VARCHAR )
			throw verb_error(generic_error::INVALID_QUERY, this->getVerbType());
		this->Format = scalar->Item.strval;
	}
	return ScalarVerb::changeQuery( pStart, pNode, resLeft, resRight, resNext );
}

//------------------------------------------------------------------------------
void ToCharVerb::transformItem( const ColumnItem& item, ColumnItem& result )
{
	Convert( result, COL_TYPE_VARCHAR, item, this->InputType );
}

//------------------------------------------------------------------------------
ColumnType ToCharVerb::outputType( ColumnType inputType )
{
	if (inputType != COL_TYPE_DATE &&
		inputType != COL_TYPE_DOUBLE &&
		inputType != COL_TYPE_INT &&
		inputType != COL_TYPE_BIG_INT )
		throw verb_error(generic_error::VERB_TYPE_MISMATCH, this->getVerbType() );

	this->InputType = inputType;
	return COL_TYPE_VARCHAR;
}

//------------------------------------------------------------------------------
void DateVerb::transformItem( const ColumnItem& item, ColumnItem& result )
{
	int day = 0, month = 0, year = 0;
	int hour = 0, minute = 0, second = 0, millisecond  = 0;
	llong intval;

  // TODO
	//dateToParts( (llong) item.numval, year, month, day, hour, minute, second, millisecond );
	//hour = minute = second = millisecond  = 0;
	//dateFromParts( intval, year, month, day, hour, minute, second, millisecond );
  
  assert(false);
  throw aq::generic_error(aq::generic_error::NOT_IMPLEMENTED, "DateVerb not supported");

	result.numval = (double) intval;
}

//------------------------------------------------------------------------------
ColumnType DateVerb::outputType( ColumnType inputType )
{
	if (inputType != COL_TYPE_DATE)
		throw verb_error(generic_error::VERB_TYPE_MISMATCH, this->getVerbType() );

	return COL_TYPE_DATE;
}

}
}
