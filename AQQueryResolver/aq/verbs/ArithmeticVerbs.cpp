#include "ArithmeticVerbs.h"
#include "VerbVisitor.h"
#include <aq/Exceptions.h>

using namespace std;

namespace aq {
namespace verb {

//------------------------------------------------------------------------------
void BinaryVerb::computeResult( VerbResult::Ptr param1, VerbResult::Ptr param2 )
{
  //assert( param1 && param2 );
  //if ((param1->getType() == VerbResult::SCALAR) && (param2->getType() == VerbResult::SCALAR))
  //{
  //  Scalar::Ptr scalar1 = boost::static_pointer_cast<Scalar>( param1 );
  //  Scalar::Ptr scalar2 = boost::static_pointer_cast<Scalar>( param2 );
  //	
  //	ColumnType type = this->outputType(scalar1->Type, scalar2->Type);
  //	Scalar::Ptr result = new Scalar( type, scalar1->Size );
  //   
  //   // TODO
  //   // this->transformItem( scalar1->Item, scalar2->Item, type, result->Item );
  //	
  //   this->Result = result;
  //	return;
  //}
	assert(false);
}

//------------------------------------------------------------------------------
bool BinaryVerb::changeQuery(aq::tnode* pStart, 
                             aq::tnode* pNode,
                             VerbResult::Ptr resLeft, 
                             VerbResult::Ptr resRight, 
                             VerbResult::Ptr resNext)
{
  throw aq::generic_error(aq::generic_error::NOT_IMPLEMENTED, "binary verb are not implemented");

	if (!resLeft || !resRight)
  {
		return false;
  }
	if(!((resLeft->getType() == VerbResult::SCALAR) && (resRight->getType() == VerbResult::SCALAR)))
  {
    return false;
  }

  // TODO
	// this->computeResult(resLeft, resRight);
 // Scalar::Ptr scalar = boost::dynamic_pointer_cast<Scalar>( this->Result );
	//assert(scalar);
	//pNode->inf = 1;
	//this->Disabled = true;
	//aq::delete_subtree(pNode->left);		
	//aq::delete_subtree(pNode->right);		
	//pNode->set_data(scalar->getValue(), scalar->Type);
	
  return true;
}

//------------------------------------------------------------------------------
void BinaryVerb::changeResult(	Table::Ptr table, 
								VerbResult::Ptr resLeft, 
								VerbResult::Ptr resRight, 
								VerbResult::Ptr resNext )
{
  assert(false);
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
ColumnType MultiplyVerb::outputType( ColumnType inputType1, ColumnType inputType2 )
{
	if( (inputType1 == COL_TYPE_DOUBLE || inputType1 == COL_TYPE_INT) &&
		(inputType2 == COL_TYPE_DOUBLE || inputType2 == COL_TYPE_INT)
		)
		return inputType1;
	throw verb_error(generic_error::VERB_TYPE_MISMATCH, this->getVerbType() );
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
