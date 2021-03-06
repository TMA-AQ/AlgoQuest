#include "ArithmeticVerbs.h"
#include "VerbVisitor.h"
#include <aq/Exceptions.h>

using namespace std;

namespace aq {
namespace verb {

//------------------------------------------------------------------------------
void BinaryVerb::computeResult( VerbResult::Ptr param1, VerbResult::Ptr param2 )
{
	assert( param1 && param2 );
	

	if( param1->getType() == VerbResult::SCALAR &&
		param2->getType() == VerbResult::SCALAR )
	{
	  Scalar::Ptr scalar1 = boost::static_pointer_cast<Scalar>( param1 );
	  Scalar::Ptr scalar2 = boost::static_pointer_cast<Scalar>( param2 );
		
		ColumnType type = this->outputType(scalar1->Type, scalar2->Type);
		Scalar::Ptr result = new Scalar( type, scalar1->Size );
		this->transformItem( scalar1->Item, scalar2->Item, type, result->Item );
		this->Result = result;
		return;
	}

	bool switched = false;
	if( param1->getType() == VerbResult::SCALAR &&
		param2->getType() == VerbResult::COLUMN )
	{
		swap( param1, param2 );
		switched = true;
	}

	if( param1->getType() == VerbResult::COLUMN &&
		param2->getType() == VerbResult::SCALAR )
	{
	  Column::Ptr column = boost::static_pointer_cast<Column>( param1 );
	  Scalar::Ptr scalar = boost::static_pointer_cast<Scalar>( param2 );

		ColumnType type = this->outputType(column->Type, scalar->Type);

		Column::Ptr newColumn = new Column(type);
		newColumn->setCount( column->getCount() ); //debug13
		newColumn->Items.resize( column->Items.size() );
		for( size_t idx = 0; idx < column->Items.size(); ++idx )
			if( column->Items[idx] )
			{
				newColumn->Items[idx] = ColumnItem::Ptr(new ColumnItem());
				if( switched )
					this->transformItem( scalar->Item, *column->Items[idx],
					type, *newColumn->Items[idx] );
				else
					this->transformItem( *column->Items[idx], scalar->Item, 
						type, *newColumn->Items[idx] );
			}
			else
				newColumn->Items[idx] = NULL;
		this->Result = newColumn;
		return;
	}

	if( param1->getType() == VerbResult::COLUMN &&
		param2->getType() == VerbResult::COLUMN )
	{
	  Column::Ptr column1 = boost::static_pointer_cast<Column>( param1 );
	  Column::Ptr column2 = boost::static_pointer_cast<Column>( param2 );

		ColumnType type = this->outputType(column1->Type, column2->Type);

		Column::Ptr newColumn = new Column(type);
		newColumn->setCount( column1->getCount() ); //debug13
		newColumn->Items.resize( column1->Items.size() );
		assert( column1->Items.size() == column2->Items.size() );
		for( size_t idx = 0; idx < column1->Items.size(); ++idx )
			if( column1->Items[idx] && column2->Items[idx] )
			{
				newColumn->Items[idx] = ColumnItem::Ptr(new ColumnItem());
				this->transformItem(	*column1->Items[idx], 
										*column2->Items[idx], 
										type,
										*newColumn->Items[idx] );
			}
			else
				newColumn->Items[idx] = NULL;
		this->Result = newColumn;
		return;
	}

	assert( 0 );
}

//------------------------------------------------------------------------------
bool BinaryVerb::changeQuery(	aq::tnode* pStart, aq::tnode* pNode,
								VerbResult::Ptr resLeft, 
								VerbResult::Ptr resRight, 
								VerbResult::Ptr resNext )
{
	if( !resLeft || !resRight )
		return false;
	if( !(resLeft->getType() == VerbResult::SCALAR &&
		resRight->getType() == VerbResult::SCALAR) )
		return false;
	this->computeResult( resLeft, resRight );
	Scalar::Ptr scalar = boost::dynamic_pointer_cast<Scalar>( this->Result );
	assert( scalar );
	pNode->inf = 1;
	this->Disabled = true;
	delete_subtree( pNode->left );		pNode->left = NULL;
	delete_subtree( pNode->right );		pNode->right = NULL;
	pNode->set_data( scalar->getValue(), scalar->Type );
	return true;
}

//------------------------------------------------------------------------------
void BinaryVerb::changeResult(	Table::Ptr table, 
								VerbResult::Ptr resLeft, 
								VerbResult::Ptr resRight, 
								VerbResult::Ptr resNext )
{
	if( this->Result )
		return;
	assert( resLeft && resRight );
	this->computeResult( resLeft, resRight );
}

//------------------------------------------------------------------------------
void BinaryVerb::addResult(aq::Row& row)
{
  throw aq::generic_error(aq::generic_error::NOT_IMPLEMENTED, "binary operation are not implemented");
	// this->computeResult( resLeft, resRight );
  // this->computeResult(this->getLeftChild()->Result, this->getRightChild()->Result);
}

//------------------------------------------------------------------------------
void BinaryVerb::accept(VerbVisitor* visitor)
{
  visitor->visit(this);
}

//------------------------------------------------------------------------------
void MinusVerb::transformItem(	const ColumnItem& item1, const ColumnItem& item2, 
								ColumnType resultType, ColumnItem& result )
{
	result.numval = item1.numval - item2.numval;
}

//------------------------------------------------------------------------------
ColumnType MinusVerb::outputType( ColumnType inputType1, ColumnType inputType2 )
{
	if ( inputType1 == COL_TYPE_DATE && inputType2 == COL_TYPE_DATE)
		return inputType1;
	if ( (inputType1 == COL_TYPE_DOUBLE || inputType1 == COL_TYPE_INT) &&
		(inputType2 == COL_TYPE_DOUBLE || inputType2 == COL_TYPE_INT))
		return inputType1;
	throw verb_error(generic_error::VERB_TYPE_MISMATCH, this->getVerbType() );
}

//------------------------------------------------------------------------------
void PlusVerb::transformItem(	const ColumnItem& item1, const ColumnItem& item2, 
								ColumnType resultType, ColumnItem& result )
{
	if( resultType == COL_TYPE_VARCHAR )
		result.strval = item1.strval + item2.strval;
	else
		result.numval = item1.numval + item2.numval;
}

//------------------------------------------------------------------------------
ColumnType PlusVerb::outputType( ColumnType inputType1, ColumnType inputType2 )
{
	if (inputType1 == COL_TYPE_DATE && inputType2 == COL_TYPE_DATE)
		return min(inputType1, inputType2);
	if ((inputType1 == COL_TYPE_DOUBLE || inputType1 == COL_TYPE_INT) && (inputType2 == COL_TYPE_DOUBLE || inputType2 == COL_TYPE_INT))
		return max(inputType1, inputType2);
	if( inputType1 == COL_TYPE_VARCHAR && inputType2 == COL_TYPE_VARCHAR )
		return inputType1;
	throw verb_error(generic_error::VERB_TYPE_MISMATCH, this->getVerbType() );
}

//------------------------------------------------------------------------------
void MultiplyVerb::transformItem(	const ColumnItem& item1, const ColumnItem& item2, 
									ColumnType resultType, ColumnItem& result )
{
	result.numval = item1.numval * item2.numval;
}

//------------------------------------------------------------------------------
ColumnType MultiplyVerb::outputType( ColumnType inputType1, ColumnType inputType2 )
{
	if( (inputType1 == COL_TYPE_DOUBLE || inputType1 == COL_TYPE_INT) &&
		(inputType2 == COL_TYPE_DOUBLE || inputType2 == COL_TYPE_INT)
		)
		return inputType1;
	throw verb_error(generic_error::VERB_TYPE_MISMATCH, this->getVerbType() );
}

//------------------------------------------------------------------------------
void DivideVerb::transformItem(	const ColumnItem& item1, const ColumnItem& item2, 
								 ColumnType resultType, ColumnItem& result )
{
	result.numval = item1.numval * item2.numval;
}

//------------------------------------------------------------------------------
ColumnType DivideVerb::outputType( ColumnType inputType1, ColumnType inputType2 )
{
	if( (inputType1 == COL_TYPE_DOUBLE || inputType1 == COL_TYPE_INT) &&
		(inputType2 == COL_TYPE_DOUBLE || inputType2 == COL_TYPE_INT)
		)
		return inputType1;
	throw verb_error(generic_error::VERB_TYPE_MISMATCH, this->getVerbType() );
}

}
}
