#include "MainVerbs.h"
#include "VerbVisitor.h"
#include <algorithm>
#include <aq/parser/ID2Str.h>
#include <aq/Exceptions.h>
#include <aq/TreeUtilities.h>
#include <aq/Column2Table.h>
#include <aq/Logger.h>

using namespace std;

namespace aq {
namespace verb {

//------------------------------------------------------------------------------
bool SelectVerb::preprocessQuery( aq::tnode* pStart, aq::tnode* pNode, aq::tnode* pStartOriginal )
{
	assert( pStartOriginal->tag == pNode->tag );
	vector<aq::tnode*> columns;
	columns.clear();
	aq::getColumnsList( pStartOriginal->left, columns );
	for( size_t idx = 0; idx < columns.size(); ++idx )
	{
		std::string name;
		aq::extractName( columns[idx], name );
		this->ColumnsDisplay.push_back( name );
	}
	return false;
}

//------------------------------------------------------------------------------
bool SelectVerb::changeQuery(	aq::tnode* pStart, aq::tnode* pNode,
								VerbResult::Ptr resLeft, 
								VerbResult::Ptr resRight, 
								VerbResult::Ptr resNext )
{

	if( resLeft && resLeft->getType() == VerbResult::ASTERISK )
	{
		this->Columns.clear();
		solveSelectStar( pNode, *this->m_baseDesc, this->Columns, this->ColumnsDisplay );
		return false;
	}
	vector<aq::tnode*> columns;
	getColumnsList( pNode->left, columns );
	for( size_t idx = 0; idx < columns.size(); ++idx )
	{
		std::string name;
		if( columns[idx]->tag == K_PERIOD )
		{
			name += columns[idx]->left->getData().val_str;
			name += ".";
			name += columns[idx]->right->getData().val_str;
		}
		else
		{
			if( columns[idx]->tag == K_AS && columns[idx]->left->tag == K_PERIOD )
			{
				name += columns[idx]->left->left->getData().val_str;
				name += ".";
				name += columns[idx]->left->right->getData().val_str;
			}
			columns[idx]->tag = K_DELETED;
			columns[idx] = NULL;
		}
		this->Columns.push_back( name );
	}

	assert( this->Columns.size() == this->ColumnsDisplay.size() );
	getAllColumns( pNode, columns );

	//add extra columns
	if( this->Columns.size() == columns.size() )
		return false; //no extra columns
		
	if( pNode->left->tag == K_COMMA )
	{
		pNode = pNode->left;
		while( pNode->left && pNode->left->tag == K_COMMA )
			pNode = pNode->left;
	}
	
	aq::tnode* pAuxNode = pNode->left;
	pNode->left = new aq::tnode( K_COMMA );
	pNode = pNode->left;
	pNode->right = pAuxNode;
	
	for( size_t idx = this->Columns.size(); idx < columns.size() - 1; ++idx )
	{
		pNode->left = new aq::tnode( K_COMMA );
		pNode = pNode->left;
		pNode->right = columns[idx];
	}
	pNode->left = columns[columns.size() - 1];

	return false;
}

//------------------------------------------------------------------------------
void SelectVerb::changeResult(	Table::Ptr table, 
								VerbResult::Ptr resLeft,
								VerbResult::Ptr resRight, VerbResult::Ptr resNext )
{
	assert( resLeft && !resRight );
	if( resLeft->getType() == VerbResult::ASTERISK )
	{
		assert( this->Columns.size() <= table->Columns.size() );
		for( size_t idx = 0; idx < this->Columns.size(); ++idx )
			table->Columns[idx]->setName( this->Columns[idx] );
		return;
	}
	//either one column or one K_COMMA operator
	VerbResultArray::Ptr resArray = boost::dynamic_pointer_cast<VerbResultArray>( resLeft );
	if( !resArray )
	{
		resArray = new VerbResultArray();
		resArray->Results.push_back( resLeft );
	}

	std::vector<Column::Ptr> newColumns;
	std::deque<VerbResult::Ptr>& params = resArray->Results;
	assert( params.size() == this->Columns.size() );
	Column::Ptr foundColumn = NULL;
	size_t nrColumns = table->Columns.size();
	if( table->HasCount )
		--nrColumns;
	for( size_t idx = 0; idx < nrColumns; ++idx )
		table->Columns[idx]->Invisible = true;
	for( size_t idx = 0; idx < this->Columns.size(); ++idx )
	{
		if( !params[idx] )
			throw generic_error(generic_error::GENERIC, "");
		Column::Ptr column;
		if( params[idx]->getType() == VerbResult::COLUMN )
		{
		  column = boost::static_pointer_cast<Column>( params[idx] );
			newColumns.push_back( column );
			if( !foundColumn )
				foundColumn = column;
		}
		else
		{
			assert( params[idx]->getType() == VerbResult::SCALAR );
			Scalar::Ptr scalar = boost::static_pointer_cast<Scalar>( params[idx] );
			column = new Column(scalar->Type);
			column->Items.push_back(new ColumnItem(scalar->Item));
			newColumns.push_back( column );
		}
		if( idx < this->Columns.size() )
		{
			column->setName( this->Columns[idx] );
			column->setDisplayName( this->ColumnsDisplay[idx] );
		}
		column->Invisible = false;
	}

	//add the rest of the columns
	for( size_t idx = 0; idx < table->Columns.size(); ++idx )
		if( table->Columns[idx]->Invisible )
			newColumns.push_back( table->Columns[idx] );
	/*{
		bool found = false;
		for( size_t idx2 = 0; idx2 < newColumns.size(); ++idx2 )
			if( newColumns[idx2]->getName() == table->Columns[idx]->getName() )
			{
				found = true;
				break;
			}
			if( !found )
			{
				table->Columns[idx]->Invisible = true;
				newColumns.push_back( table->Columns[idx] );
			}
	}*/

	if( table->HasCount && foundColumn )
		newColumns.push_back( table->Columns[table->Columns.size()-1] );
	if( table->HasCount && !foundColumn )
		table->HasCount = false;

	//turn scalars into columns
	if( foundColumn )
		for( size_t idx = 0; idx < newColumns.size(); ++idx )
			if( newColumns[idx]->Items.size() < foundColumn->Items.size() )
				newColumns[idx]->increase( foundColumn->Items.size() );

	if( !foundColumn )
		table->TotalCount = 1;

	table->Columns = newColumns;
}

void SelectVerb::accept(VerbVisitor* visitor)
{
	visitor->visit(this);
}

//------------------------------------------------------------------------------
bool WhereVerb::preprocessQuery( aq::tnode* pStart, aq::tnode* pNode, aq::tnode* pStartOriginal )
{
	// eliminate K_NOT
	aq::processNot(pNode->left, false);
	return false;
}

//------------------------------------------------------------------------------
bool WhereVerb::changeQuery( aq::tnode* pStart, aq::tnode* pNode,
	VerbResult::Ptr resLeft, VerbResult::Ptr resRight, VerbResult::Ptr resNext )
{
	aq::addInnerOuterNodes( pNode->left, K_INNER, K_INNER );
	return false;
}

//------------------------------------------------------------------------------
void WhereVerb::changeResult( Table::Ptr table, 
	VerbResult::Ptr resLeft, VerbResult::Ptr resRight, VerbResult::Ptr resNext )
{
	assert(false);
}

void WhereVerb::accept(VerbVisitor* visitor)
{
	visitor->visit(this);
}

//------------------------------------------------------------------------------
bool OrderVerb::preprocessQuery( aq::tnode* pStart, aq::tnode* pNode, aq::tnode* pStartOriginal )
{
	vector<aq::tnode*> columns;
	getColumnsList( pNode->left->left, columns );
	vector<aq::tnode*> selectColumns;
	getColumnsList( pStart->left, selectColumns );
	for( size_t idx = 0; idx < columns.size(); ++idx )
		if( columns[idx]->tag == K_IDENT || 
			columns[idx]->tag == K_COLUMN )
		{
			int colIdx = -1;
			for( size_t idx2 = 0; idx2 < selectColumns.size(); ++idx2 )
				if( selectColumns[idx2] &&
					selectColumns[idx2]->tag == K_AS &&
					strcmp( selectColumns[idx2]->right->getData().val_str,
						columns[idx]->getData().val_str ) == 0
					)
				{
					colIdx = (int) idx2;
					break;
				}
			if( colIdx < 0 )
				throw verb_error(generic_error::INVALID_QUERY, this->getVerbType());
			columns[idx]->tag = K_INTEGER;
			columns[idx]->set_int_data( colIdx + 1 );
		}
	return false;
}

//------------------------------------------------------------------------------
bool OrderVerb::changeQuery(	aq::tnode* pStart, aq::tnode* pNode,
								VerbResult::Ptr resLeft, VerbResult::Ptr resRight, VerbResult::Ptr resNext )
{
	// pNode->tag = K_DELETED;
	return false;
}

//------------------------------------------------------------------------------
void OrderVerb::changeResult(	Table::Ptr table, 
								VerbResult::Ptr resLeft, VerbResult::Ptr resRight, VerbResult::Ptr resNext )
{
	assert(false);
}

//------------------------------------------------------------------------------
void OrderVerb::accept(VerbVisitor* visitor)
{
  visitor->visit(this);
}

//------------------------------------------------------------------------------
bool ByVerb::changeQuery( aq::tnode* pStart, aq::tnode* pNode,
							VerbResult::Ptr resLeft, VerbResult::Ptr resRight, VerbResult::Ptr resNext )
{
	return false;
}

//------------------------------------------------------------------------------
void ByVerb::changeResult(	Table::Ptr table, 
							VerbResult::Ptr resLeft, VerbResult::Ptr resRight, VerbResult::Ptr resNext )
{
	//simply move the parameters up to the parent
	assert( resLeft && !resRight );
	this->Result = resLeft;
}

//------------------------------------------------------------------------------
void ByVerb::accept(VerbVisitor* visitor)
{
	visitor->visit(this);
}

//------------------------------------------------------------------------------
bool FromVerb::preprocessQuery( aq::tnode* pStart, aq::tnode* pNode, aq::tnode* pStartOriginal )
{
  if (enforce_qualified_column_reference(pStart, *this->m_baseDesc) != 0)
  {
		throw generic_error(generic_error::INVALID_QUERY, "Error : No or bad tables specified in SQL SELECT ... FROM Statement");
  }
  aq::getTablesList(pNode, this->tables);
	return false;
}

//------------------------------------------------------------------------------
bool FromVerb::changeQuery( aq::tnode* pStart, aq::tnode* pNode,
	VerbResult::Ptr resLeft, VerbResult::Ptr resRight, VerbResult::Ptr resNext )
{
	aq::solveOneTableInFrom( pStart, *this->m_baseDesc );
	aq::moveFromJoinToWhere( pStart, *this->m_baseDesc );
	return false;
}

//------------------------------------------------------------------------------
void FromVerb::accept(VerbVisitor* visitor)
{
	visitor->visit(this);
}

//------------------------------------------------------------------------------
bool GroupVerb::preprocessQuery( aq::tnode* pStart, aq::tnode* pNode, aq::tnode* pStartOriginal )
{
  return this->useRowResolver;
}

//------------------------------------------------------------------------------
bool GroupVerb::changeQuery(	aq::tnode* pStart, aq::tnode* pNode,
								VerbResult::Ptr resLeft, 
								VerbResult::Ptr resRight, 
								VerbResult::Ptr resNext )
{
	pNode->tag = K_DELETED;
	return false;
}

//------------------------------------------------------------------------------
void GroupVerb::changeResult(	Table::Ptr table, 
								VerbResult::Ptr resLeft, 
								VerbResult::Ptr resRight, 
								VerbResult::Ptr resNext )
{
  assert(false);
}

//------------------------------------------------------------------------------
void GroupVerb::addResult(aq::Row& row)
{
  assert((this->row_acc.computedRow.size() == 0) || (row.flush) || (row.computedRow.size() == this->row_acc.computedRow.size()));
  assert((this->row_prv.initialRow.size() == 0) || (row.flush) || (row.initialRow.size() == this->row_prv.initialRow.size()));
  assert(this->row_prv.computedRow.size() == this->row_acc.computedRow.size());
  
  row.completed = false;

  // flush
  if (row.flush)
  {
    row.completed = true;
    row.computedRow.clear();
    std::copy(this->row_acc.computedRow.begin(), this->row_acc.computedRow.end(), std::back_inserter<aq::Row::row_t>(row.computedRow));
    return;
  }

  // check if new group
  bool new_group = false;
  if (row_prv.computedRow.size() > 0)
  {
    for (size_t i = 0; i < row.initialRow.size(); ++i)
    {
      if (row.initialRow[i].grouped && !ColumnItem::equal(row_prv.initialRow[i].item.get(), row.initialRow[i].item.get(), row_prv.initialRow[i].type))
      {
        new_group = true;
      }
    }
  }
  row.completed = new_group;

  // store previous
  row_prv = row;

  //
  if (new_group)
  {
    row.computedRow.clear();
    std::copy(this->row_acc.computedRow.begin(), this->row_acc.computedRow.end(), std::back_inserter<aq::Row::row_t>(row.computedRow));
    this->row_acc.computedRow.clear();
  }
   
  // compute and store in row_acc
  if (row_acc.computedRow.empty())
  {
    std::copy(row_prv.computedRow.begin(), row_prv.computedRow.end(), std::back_inserter<aq::Row::row_t>(row_acc.computedRow));
    row_acc.count= row_prv.count;
    for (size_t i = 0; i < row.computedRow.size(); ++i)
    {
      switch (row.computedRow[i].aggFunc)
      {
      case SUM:
        row_acc.computedRow[i].item->numval *= row_acc.count;
        break;
      default:
        break;
      }
    }
  }
  else
  {
    for (size_t i = 0; i < row.computedRow.size(); ++i)
    {
      aq::apply_aggregate(row.computedRow[i].aggFunc, row.computedRow[i].type, *row_acc.computedRow[i].item, row_acc.count, *row_prv.computedRow[i].item, row_prv.count);
    }
    this->row_acc.count += this->row_prv.count;
  }

}

//------------------------------------------------------------------------------
void GroupVerb::accept(VerbVisitor* visitor)
{
	visitor->visit(this);
}

//------------------------------------------------------------------------------
bool HavingVerb::preprocessQuery(	aq::tnode* pStart, aq::tnode* pNode, 
									aq::tnode* pStartOriginal )
{
	//eliminate K_NOT
	aq::processNot( pNode->left, false );
	return false;
}

//------------------------------------------------------------------------------
void HavingVerb::changeResult(	Table::Ptr table, 
								VerbResult::Ptr resLeft, 
								VerbResult::Ptr resRight, 
								VerbResult::Ptr resNext )
{
  assert(false);
}

//------------------------------------------------------------------------------
void HavingVerb::accept(VerbVisitor* visitor)
{
  visitor->visit(this);
}

}
}
