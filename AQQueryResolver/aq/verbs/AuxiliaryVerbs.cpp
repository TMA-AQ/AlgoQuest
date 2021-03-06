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
	//auxcol.setName( string(pNode->left->data.val_str) + "." + string(pNode->right->data.val_str) );
	auxcol.setName( pNode->right->getData().val_str );
	this->ColumnOnlyName = auxcol.getName();
	//debug13 - select will have changed the column names in the result table
	//to fully qualified names by the time Order::changeResult is called and
	//the changeResults of its columns are called. So until, identify columns by their
	//unqualified name because that's how AQ_Engine returns them.
	//but afterwards, identify them by their full name.
	//Also, check if it isn't possible to always use fully qualified names.
	//if( this->Context == K_ORDER || this->Context == K_GROUP )
	//{
		auxcol.setName( this->TableName + "." + this->ColumnOnlyName );
		this->ColumnName = auxcol.getName();
	//}
	//update: it IS possible

  boost::to_upper(this->TableName);
  boost::to_upper(this->ColumnOnlyName);
  boost::to_upper(this->ColumnName);

	if( this->Context != K_WHERE )
		return false; //must perform the query before we know the values

	return false; //debug13 temporary tryout because of a hunch: there is no

	//case in which obtaining a column in the WHERE clause is necessary
	//the only exceptions (I can think of now) would be:
	// - aggregate functions (but they are not allowed in WHERE)
	// - columns with only 1 value (they should be read and turned into scalars)
	//it is also damaging to obtain a column in the WHERE clause because
	//it will need to be read from the table if the above operation cannot be
	//solved now and it will not because Result will already be set.
	//All this results in a simple rule: don't load columns in changeQuery

	//TODO: only get a part at a time for a column

	/* Loop on Thesaurus Parts (biggest nLoopCnt is 999, three digit !) */

	//int pErr;
	//Column::Ptr column = new Column();
	//column->setName( auxcol.getName() );
	//for ( int nLoopCnt = 0; nLoopCnt < 1000; nLoopCnt++ )
	//	get_thesaurus_for_column_reference( *column, pNode, nLoopCnt, this->m_baseDesc, 
	//		this->m_settings->szThesaurusPath, &pErr );
	//this->Result = column;
	//return false;

}

//------------------------------------------------------------------------------
void ColumnVerb::changeResult(	Table::Ptr table, 
								VerbResult::Ptr resLeft,
								VerbResult::Ptr resRight, VerbResult::Ptr resNext )
{
	if( this->Result )
		return;
	for( size_t idx = 0; idx < table->Columns.size(); ++idx )
  {
    std::string name = table->Columns[idx]->getTableName() + "." + table->Columns[idx]->getName();
		if( name == this->ColumnName )
		{
			Column::Ptr column = table->Columns[idx];
			if( table->HasCount )
				column->setCount( table->Columns[table->Columns.size()-1] );
			this->Result = column;
			switch( this->Context )
			{
			case K_GROUP: column->GroupBy = true; break;
			case K_ORDER: column->OrderBy = true; break;
			}
			return;
		}
  }
	// assert( (table->Columns.size() == 0) || (table->Columns.size() == 1) && table->HasCount ); // FIXME ??????
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
    (*row.computedRow.rbegin()).item.reset(new ColumnItem);
    assert(row.computedRow.size() <= std::numeric_limits<size_t>::max());
    this->computed_index = static_cast<int>(row.computedRow.size()) - 1;
  }
  
  if (static_cast<size_t>(this->computed_index) >= row.computedRow.size())
  {
    row.computedRow.push_back(row_item);
    (*row.computedRow.rbegin()).item.reset(new ColumnItem);
    assert(static_cast<size_t>(this->computed_index) == (row.computedRow.size() - 1));
  }

  // this->Result.reset(new Scalar(row_item.type, row_item.size, *row_item.item.get()));
  aq::row_item_t& row_computed_item = row.computedRow[this->computed_index];
  row_computed_item.null = row_item.null; // row.initialRow[this->index].null;
  *row_computed_item.item = *row_item.item; // perform copy
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
	VerbResultArray::Ptr resArray = NULL;
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
  // this->changeResult(NULL, resLeft, resRight, resNext);
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
	this->Result = new Scalar(COL_TYPE_INT, 4, ColumnItem((double)pNode->getData().val_int));
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
	this->Result = new Scalar(COL_TYPE_DOUBLE, 8, ColumnItem(pNode->getData().val_number));
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
	this->Result = new Scalar(COL_TYPE_VARCHAR, 128, ColumnItem(pNode->getData().val_str)); // FIXME
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
    pNode->right = NULL;
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
