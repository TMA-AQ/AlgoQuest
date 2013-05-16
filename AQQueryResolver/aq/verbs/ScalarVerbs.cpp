#include "ScalarVerbs.h"
#include "VerbVisitor.h"
#include <cmath>
#include <aq/Exceptions.h>
#include <aq/DateConversion.h>

using namespace std;

namespace aq {
namespace verb {

//------------------------------------------------------------------------------
VERB_IMPLEMENT(ScalarVerb);

//------------------------------------------------------------------------------
ScalarVerb::ScalarVerb()
{}

//------------------------------------------------------------------------------
void ScalarVerb::computeResult( VerbResult::Ptr param )
{
	assert( param );
	Scalar::Ptr scalar = dynamic_pointer_cast<Scalar>( param );

	if( scalar )
	{
		ColumnType type = this->outputType(scalar->Type);
		Scalar::Ptr result = new Scalar( type );
		this->transformItem( scalar->Item, result->Item );
		this->Result = result;
		return;
	}

	//debug13 - there should be an easier way to do this
	VerbResultArray::Ptr resArray = dynamic_pointer_cast<VerbResultArray>(param);
	Column::Ptr column = NULL;
	if( resArray )
	{
		assert(resArray->Results.size() > 0);
		column = dynamic_pointer_cast<Column>(resArray->Results[0]);
	}
	else
		column = dynamic_pointer_cast<Column>( param );
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
	VerbResultArray::Ptr resArray = dynamic_pointer_cast<VerbResultArray>(resLeft);
	Scalar::Ptr scalar = NULL;
	if( resArray )
	{
		assert(resArray->Results.size() > 0);
		scalar = dynamic_pointer_cast<Scalar>(resArray->Results[0]);
	}
	else
		scalar = dynamic_pointer_cast<Scalar>( resLeft );
	if( !scalar )
		return false;
	this->computeResult( scalar );
	scalar = dynamic_pointer_cast<Scalar>( this->Result );
	assert( scalar );
	pNode->inf = 1;
	this->Disabled = true;
	delete_subtree( pNode->left );		pNode->left = NULL;
	delete_subtree( pNode->right );		pNode->right = NULL;
	set_data( *pNode, scalar->getValue(), scalar->Type );
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
void ScalarVerb::addResult(	aq::RowProcess_Intf::Row& row, 
								VerbResult::Ptr resLeft, 
								VerbResult::Ptr resRight, 
								VerbResult::Ptr resNext )
{
	assert( resLeft );
  this->computeResult( resLeft );
  
  Scalar * scalar = dynamic_cast<Scalar*>(this->Result.get());
  if (scalar != 0)
  {
    ColumnItem::Ptr item(new ColumnItem(scalar->Item));
    row.row.push_back(aq::RowProcess_Intf::row_item_t(item, scalar->Type, scalar->Size, "", ""));
  }
}

//------------------------------------------------------------------------------
void ScalarVerb::accept(VerbVisitor* visitor)
{
  visitor->visit(this);
}

//------------------------------------------------------------------------------
VERB_IMPLEMENT(SqrtVerb);

//------------------------------------------------------------------------------
SqrtVerb::SqrtVerb()
{}

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
VERB_IMPLEMENT(AbsVerb);

//------------------------------------------------------------------------------
AbsVerb::AbsVerb()
{}

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
VERB_IMPLEMENT(SubstringVerb);

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
		this->StartPos = pNode->left->right->left->data.val_int;
		this->Size = pNode->left->right->right->data.val_int;
		pNode->left->right->inf = 1;
	}
	else
	{
		assert( pNode->left->right->tag == K_INTEGER );
		this->StartPos = pNode->left->right->data.val_int;
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
VERB_IMPLEMENT(ToDateVerb);

//------------------------------------------------------------------------------
ToDateVerb::ToDateVerb()
{}

//------------------------------------------------------------------------------
bool ToDateVerb::preprocessQuery( aq::tnode* pStart, aq::tnode* pNode, aq::tnode* pStartOriginal )
{
	assert( pNode );
	this->OutputType = COL_TYPE_DATE1;
	if( pNode->right )
	{
		assert( pNode->right->tag == K_STRING );
		char *strval = pNode->right->data.val_str;
		if( strcmp( strval, "DD/MM/YYYY" ) == 0 )
			this->OutputType = COL_TYPE_DATE2;
		else if( strcmp( strval, "DD/MM/YY" ) == 0 )
			this->OutputType = COL_TYPE_DATE3;
	}
	return false;
}

//------------------------------------------------------------------------------
void ToDateVerb::transformItem( const ColumnItem& item, ColumnItem& result )
{
	llong intval;
	DateType resultType;
	if( !dateToBigInt( item.strval.c_str(), &resultType, &intval ) )
		throw verb_error( generic_error::TYPE_MISMATCH, this->getVerbType() );

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
VERB_IMPLEMENT(YearVerb);

//------------------------------------------------------------------------------
YearVerb::YearVerb()
{}

//------------------------------------------------------------------------------
void YearVerb::transformItem( const ColumnItem& item, ColumnItem& result )
{
	int year, month, day, hour, minute, second, millisecond;
	dateToParts( (llong) item.numval, year, month, day, hour, minute, second, millisecond );
	result = ColumnItem( (double) year );
}

//------------------------------------------------------------------------------
ColumnType YearVerb::outputType( ColumnType inputType )
{
	if( !(inputType >= COL_TYPE_DATE1 && inputType <= COL_TYPE_DATE4) )
		throw verb_error(generic_error::VERB_TYPE_MISMATCH, this->getVerbType() );

	return COL_TYPE_INT;
}

//------------------------------------------------------------------------------
VERB_IMPLEMENT(MonthVerb);

//------------------------------------------------------------------------------
MonthVerb::MonthVerb()
{}

//------------------------------------------------------------------------------
void MonthVerb::transformItem( const ColumnItem& item, ColumnItem& result )
{
	int year, month, day, hour, minute, second, millisecond;
	dateToParts( (llong) item.numval, year, month, day, hour, minute, second, millisecond );
	result = ColumnItem( (double) month );
}

//------------------------------------------------------------------------------
ColumnType MonthVerb::outputType( ColumnType inputType )
{
	if( !(inputType >= COL_TYPE_DATE1 && inputType <= COL_TYPE_DATE4) )
		throw verb_error(generic_error::VERB_TYPE_MISMATCH, this->getVerbType() );

	return COL_TYPE_INT;
}

//------------------------------------------------------------------------------
VERB_IMPLEMENT(DayVerb);

//------------------------------------------------------------------------------
DayVerb::DayVerb()
{}

//------------------------------------------------------------------------------
void DayVerb::transformItem( const ColumnItem& item, ColumnItem& result )
{
	int year, month, day, hour, minute, second, millisecond;
	dateToParts( (llong) item.numval, year, month, day, hour, minute, second, millisecond );
	result = ColumnItem( (double) day );
}

//------------------------------------------------------------------------------
ColumnType DayVerb::outputType( ColumnType inputType )
{
	if( !(inputType >= COL_TYPE_DATE1 && inputType <= COL_TYPE_DATE4) )
		throw verb_error(generic_error::VERB_TYPE_MISMATCH, this->getVerbType() );

	return COL_TYPE_INT;
}

//------------------------------------------------------------------------------
VERB_IMPLEMENT(ToCharVerb);

//------------------------------------------------------------------------------
ToCharVerb::ToCharVerb()
{}

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
		Scalar::Ptr scalar = dynamic_pointer_cast<Scalar>( resRight );
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
	if( !(inputType >= COL_TYPE_DATE1 && inputType <= COL_TYPE_DATE4) &&
		inputType != COL_TYPE_DOUBLE &&
		inputType != COL_TYPE_INT &&
		inputType != COL_TYPE_BIG_INT )
		throw verb_error(generic_error::VERB_TYPE_MISMATCH, this->getVerbType() );

	this->InputType = inputType;
	if( this->Format == "YYYYMM" )
		this->InputType = COL_TYPE_DATE4;
	return COL_TYPE_VARCHAR;
}

//------------------------------------------------------------------------------
VERB_IMPLEMENT(DateVerb);

//------------------------------------------------------------------------------
DateVerb::DateVerb()
{}

//------------------------------------------------------------------------------
void DateVerb::transformItem( const ColumnItem& item, ColumnItem& result )
{
	int day = 0, month = 0, year = 0;
	int hour = 0, minute = 0, second = 0, millisecond  = 0;
	dateToParts( (llong) item.numval, year, month, day, hour, minute, second, 
		millisecond );
	hour = minute = second = millisecond  = 0;
	llong intval;
	dateFromParts( intval, year, month, day, hour, minute, second, millisecond );
	result.numval = (double) intval;
}

//------------------------------------------------------------------------------
ColumnType DateVerb::outputType( ColumnType inputType )
{
	if( !(inputType >= COL_TYPE_DATE1 && inputType <= COL_TYPE_DATE4) )
		throw verb_error(generic_error::VERB_TYPE_MISMATCH, this->getVerbType() );

	return COL_TYPE_DATE2;
}

}
}