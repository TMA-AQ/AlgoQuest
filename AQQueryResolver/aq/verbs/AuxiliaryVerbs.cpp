#include "AuxiliaryVerbs.h"
#include "VerbVisitor.h"
#include <aq/Utilities.h>
#include <aq/ExprTransform.h>
#include <aq/Exceptions.h>
#include <memory>
#include <algorithm>
#include <boost/algorithm/string.hpp>

using namespace std;

namespace aq {
namespace verb {

//------------------------------------------------------------------------------
ColumnVerb::ColumnVerb()
  : index(-1), computed_index(-1)
{}

//------------------------------------------------------------------------------
bool ColumnVerb::changeQuery(	aq::tnode* pStart, aq::tnode* pNode,
								VerbResult::Ptr resLeft, 
								VerbResult::Ptr resRight, VerbResult::Ptr resNext )
{
	assert( pNode->left->tag == K_IDENT );
	assert( pNode->right->tag == K_COLUMN );
	this->TableName = pNode->left->getData().val_str;

  Table::Ptr table = this->m_baseDesc->getTable(this->TableName);
  this->TableName = table->getName();

	Column auxcol;
	auxcol.setName( pNode->right->getData().val_str );
	this->ColumnOnlyName = auxcol.getName();
  auxcol.setName( this->TableName + "." + this->ColumnOnlyName );
  this->ColumnName = auxcol.getName();

  boost::to_upper(this->TableName);
  boost::to_upper(this->ColumnOnlyName);
  boost::to_upper(this->ColumnName);

	if( this->Context != K_WHERE )
		return false;

	return false;
}

//------------------------------------------------------------------------------
void ColumnVerb::changeResult(	Table::Ptr table, 
								VerbResult::Ptr resLeft,
								VerbResult::Ptr resRight, VerbResult::Ptr resNext )
{
  assert(false);
}

//------------------------------------------------------------------------------
void ColumnVerb::addResult(aq::Row& row)
{
  if (this->Context != K_SELECT)
  {
    // TODO
    return;
  }

  if (this->index == -1)
  {
    int i = 0;
    for (aq::Row::row_t::iterator it = row.initialRow.begin(); it != row.initialRow.end(); ++it)
    {
      if ((*it).match(this->TableName, this->ColumnOnlyName))
      {
        index = i;
      }
      i++;
    }
  }
  assert(this->index != -1);

  if (row.flush)
  {
    // this is a flush
    return;
  }

  aq::row_item_t& row_item = row.initialRow[this->index];

  if (this->computed_index == -1)
  {
    row.computedRow.push_back(row_item);
    assert(row.computedRow.size() <= std::numeric_limits<size_t>::max());
    this->computed_index = static_cast<int>(row.computedRow.size()) - 1;
  }
  
  if (static_cast<size_t>(this->computed_index) >= row.computedRow.size())
  {
    row.computedRow.push_back(row_item);
    assert(static_cast<size_t>(this->computed_index) == (row.computedRow.size() - 1));
  }

  // this->Result.reset(new Scalar(row_item.type, row_item.size, *row_item.item.get()));
  aq::row_item_t& row_computed_item = row.computedRow[this->computed_index];
  row_computed_item.null = row_item.null; // row.initialRow[this->index].null;
  row_computed_item.item = row_item.item; // perform copy
}

//------------------------------------------------------------------------------
std::string ColumnVerb::getTableName() const{ return this->TableName; }
std::string ColumnVerb::getColumnName() const{ return this->ColumnName; };
std::string ColumnVerb::getColumnOnlyName() const{ return this->ColumnOnlyName; };

//------------------------------------------------------------------------------
void ColumnVerb::accept(VerbVisitor* visitor)
{
	visitor->visit(this);
}

//------------------------------------------------------------------------------
bool CommaVerb::changeQuery(	aq::tnode* pStart, aq::tnode* pNode,
								VerbResult::Ptr resLeft,
								VerbResult::Ptr resRight, VerbResult::Ptr resNext )
{
	VerbResultArray::Ptr resArray = nullptr;
	if( resLeft && resLeft->getType() == VerbResult::ARRAY )
	{
	  resArray = boost::static_pointer_cast<VerbResultArray>( resLeft );
		resArray->Results.push_back( resRight );
	}
	else
  {
		if( resRight && resRight->getType() == VerbResult::ARRAY )
		{
		  resArray = boost::static_pointer_cast<VerbResultArray>( resRight );
			resArray->Results.push_front( resLeft );
		}
		else
		{
			resArray = new VerbResultArray();
			resArray->Results.push_back( resLeft );
			resArray->Results.push_back( resRight );
		}
  }
	this->Result = resArray;
	return false;
}

//------------------------------------------------------------------------------
void CommaVerb::changeResult(	Table::Ptr table, 
								VerbResult::Ptr resLeft, VerbResult::Ptr resRight, VerbResult::Ptr resNext )
{
	if( resLeft && resLeft->getType() == VerbResult::ARRAY )
	{
	  VerbResultArray::Ptr resArray = boost::static_pointer_cast<VerbResultArray>( resLeft );
		resArray->Results.push_back( resRight );
    this->Result = resArray;
	}
	else
  {
		if( resRight && resRight->getType() == VerbResult::ARRAY )
		{
		  VerbResultArray::Ptr resArray = boost::static_pointer_cast<VerbResultArray>( resRight );
			resArray->Results.push_front( resLeft );
      this->Result = resArray;
		}
		else if ( resRight )
		{
			VerbResultArray::Ptr resArray = new VerbResultArray();
			resArray->Results.push_back( resLeft );
			resArray->Results.push_back( resRight );
      this->Result = resArray;
		}
    else
    {
      this->Result = resLeft;
    }
  }
}

//------------------------------------------------------------------------------
void CommaVerb::addResult( aq::Row& row )
{
  // this->changeResult(nullptr, resLeft, resRight, resNext);
}

//------------------------------------------------------------------------------
void CommaVerb::accept(VerbVisitor* visitor)
{
	visitor->visit(this);
}

//------------------------------------------------------------------------------
void AndVerb::changeResult(	Table::Ptr table, 
							VerbResult::Ptr resLeft, 
							VerbResult::Ptr resRight, 
							VerbResult::Ptr resNext )
{
	if( this->Context != K_WHERE && this->Context != K_HAVING )
		return;
	if( !resLeft && !resRight )
		return;
	if( !resLeft && resRight )
	{
		this->Result = resRight;
		return;
	}
	if( resLeft && !resRight )
	{
		this->Result = resLeft;
		return;
	}

	//combine row validations
	if( resLeft->getType() != VerbResult::ROW_VALIDATION )
		throw verb_error(verb_error::VERB_BAD_SYNTAX, this->getVerbType());
	RowValidation::Ptr rv1 = boost::dynamic_pointer_cast<RowValidation>( resLeft );

	if( resRight->getType() != VerbResult::ROW_VALIDATION )
		throw verb_error(verb_error::VERB_BAD_SYNTAX, this->getVerbType());
	RowValidation::Ptr rv2 = boost::dynamic_pointer_cast<RowValidation>( resRight );

	if( rv1->ValidRows.size() != rv2->ValidRows.size() )
		throw verb_error(verb_error::VERB_BAD_SYNTAX, this->getVerbType());

	for( size_t idx2 = 0; idx2 < rv1->ValidRows.size(); ++idx2 )
		rv1->ValidRows[idx2] = rv1->ValidRows[idx2] && rv2->ValidRows[idx2];
	this->Result = rv1;
}

void AndVerb::accept(VerbVisitor* visitor)
{
	visitor->visit(this);
}

//------------------------------------------------------------------------------
bool InVerb::preprocessQuery(	aq::tnode* pStart, aq::tnode* pNode, 
								aq::tnode* pStartOriginal )
{
	if( !pNode || !pNode->left )
		return false;
	if( !(pNode->right->tag == K_IN_VALUES) )
		return false;
	return true;
}

//------------------------------------------------------------------------------
bool InVerb::changeQuery(	aq::tnode* pStart, aq::tnode* pNode,
							VerbResult::Ptr resLeft,
							VerbResult::Ptr resRight, VerbResult::Ptr resNext )
{
	return true;
}

//------------------------------------------------------------------------------
void InVerb::accept(VerbVisitor* visitor)
{
	visitor->visit(this);
}

//------------------------------------------------------------------------------
bool IntValueVerb::preprocessQuery( aq::tnode* pStart, aq::tnode* pNode, aq::tnode* pStartOriginal )
{
	assert( pNode->getDataType() == aq::tnode::tnodeDataType::NODE_DATA_INT );
	this->Result = new Scalar<int32_t>(COL_TYPE_INT, 4, aq::ColumnItem<int32_t>((int32_t)pNode->getData().val_int));
	return false;
}

//------------------------------------------------------------------------------
void IntValueVerb::accept(VerbVisitor* visitor)
{
	visitor->visit(this);
}

//------------------------------------------------------------------------------
bool DoubleValueVerb::preprocessQuery( aq::tnode* pStart, aq::tnode* pNode, aq::tnode* pStartOriginal )
{
	assert( pNode->getDataType() == aq::tnode::tnodeDataType::NODE_DATA_NUMBER );
	this->Result = new Scalar<double>(COL_TYPE_DOUBLE, 8, aq::ColumnItem<double>(pNode->getData().val_number));
	return false;
}

//------------------------------------------------------------------------------
void DoubleValueVerb::accept(VerbVisitor* visitor)
{
	visitor->visit(this);
}

//------------------------------------------------------------------------------
bool StringValueVerb::preprocessQuery( aq::tnode* pStart, aq::tnode* pNode, aq::tnode* pStartOriginal )
{
	assert( pNode->getDataType() == aq::tnode::tnodeDataType::NODE_DATA_STRING );
	this->Result = new Scalar<char*>(COL_TYPE_VARCHAR, 128, aq::ColumnItem<char*>(pNode->getData().val_str));
	return false;
}

//------------------------------------------------------------------------------
void StringValueVerb::accept(VerbVisitor* visitor)
{
	visitor->visit(this);
}

//------------------------------------------------------------------------------
void replaceTableIdent( aq::tnode* pNode, const char* oldIdent, const char* newIdent )
{
	if( !pNode )
		return;
	if( pNode->tag == K_PERIOD && strcmp(oldIdent, pNode->left->getData().val_str) == 0 )
		pNode->left->set_string_data( newIdent );

	replaceTableIdent( pNode->left, oldIdent, newIdent );
	replaceTableIdent( pNode->right, oldIdent, newIdent );
	replaceTableIdent( pNode->next, oldIdent, newIdent );
}

AsVerb::AsVerb()
  : ident(""), index(-1)
{
}

//------------------------------------------------------------------------------
bool AsVerb::preprocessQuery( aq::tnode* pStart, aq::tnode* pNode, aq::tnode* pStartOriginal )
{
	switch( this->Context )
	{
	case K_FROM:
		assert( pNode && pNode->left );
		replaceTableIdent( pStart, pNode->right->getData().val_str, pNode->left->getData().val_str );
		delete pNode->right ;
    pNode->right = nullptr;
		*pNode = *pNode->left; //no memory leaks
		return true;
	case K_SELECT:
    assert(pNode->right);
    this->ident = pNode->right->getData().val_str;
		return false;
	default:
		throw verb_error(generic_error::INVALID_QUERY, this->getVerbType());
	}
	
	return false;
}

//------------------------------------------------------------------------------
void AsVerb::changeResult(	Table::Ptr table, 
							VerbResult::Ptr resLeft,
							VerbResult::Ptr resRight, VerbResult::Ptr resNext )
{
	this->Result = resLeft;
}

//------------------------------------------------------------------------------
void AsVerb::addResult(aq::Row& row)
{
  if (this->Context == K_SELECT)
  {
    if (this->index == -1)
    {
      assert(!row.computedRow.empty());
      assert(row.computedRow.size() <= std::numeric_limits<int>::max());
      this->index = static_cast<int>(row.computedRow.size()) - 1;
      row.computedRow[this->index].tableName = "";
      row.computedRow[this->index].columnName = this->ident;
    }

    if (row.flush)
    {
      return;
    }

    assert(this->index < row.computedRow.size());
    aq::row_item_t& row_item = row.computedRow[this->index];
    row_item.displayed = true;
  }
}

//------------------------------------------------------------------------------
void AsVerb::accept(VerbVisitor* visitor)
{
	visitor->visit(this);
}

//------------------------------------------------------------------------------
bool AsteriskVerb::preprocessQuery( aq::tnode* pStart, aq::tnode* pNode, aq::tnode* pStartOriginal )
{
	this->Result = new Asterisk();
	return false;
}

//------------------------------------------------------------------------------
void AsteriskVerb::accept(VerbVisitor* visitor)
{
	visitor->visit(this);
}

//------------------------------------------------------------------------------
void AscVerb::changeResult(	Table::Ptr table, 
							VerbResult::Ptr resLeft, 
							VerbResult::Ptr resRight, 
							VerbResult::Ptr resNext )
{
	//simply move the parameters up to the parent
	assert( resLeft && !resRight );
	this->Result = resLeft;
}

//------------------------------------------------------------------------------
void AscVerb::accept(VerbVisitor* visitor)
{
	visitor->visit(this);
}

}
}
