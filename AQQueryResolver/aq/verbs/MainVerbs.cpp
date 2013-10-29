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
	if( !resLeft )
		return;

	if( resLeft->getType() != VerbResult::ROW_VALIDATION )
		throw verb_error(verb_error::VERB_BAD_SYNTAX, this->getVerbType());
	RowValidation::Ptr rv = boost::dynamic_pointer_cast<RowValidation>( resLeft );
	
	//recreate table
	vector<Column::Ptr> newColumns = table->getColumnsTemplate();

	for( size_t idx = 0; idx < rv->ValidRows.size(); ++idx )
	{
		if( !rv->ValidRows[idx] )
			continue;
		for( size_t idx2 = 0; idx2 < table->Columns.size(); ++idx2 )
			newColumns[idx2]->Items.push_back( table->Columns[idx2]->Items[idx] );
	}

	table->updateColumnsContent( newColumns );
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
	assert( resLeft );
	VerbResultArray::Ptr resArray = boost::dynamic_pointer_cast<VerbResultArray>( resLeft );
	if( !resArray )
	{
		resArray = new VerbResultArray();
		resArray->Results.push_back( resLeft );
	}
	assert( resArray->Results.size() > 0 );
	vector<Column::Ptr> selectColumns;
	for( size_t idx = 0; idx < table->Columns.size(); ++idx )
		if( !table->Columns[idx]->Invisible )
			selectColumns.push_back( table->Columns[idx] );

	vector<Column::Ptr> columns;
	for( size_t idx = 0; idx < resArray->Results.size(); ++idx )
	{
		
		VerbResult::Ptr result = resArray->Results[idx];
		assert( result );
		switch( result->getType() )
		{
		case VerbResult::COLUMN:
		  columns.push_back( boost::static_pointer_cast<Column>( result ) );
			break;
		case VerbResult::SCALAR:
			{
			  Scalar::Ptr scalar = boost::static_pointer_cast<Scalar>( result );
				if( scalar->Type != COL_TYPE_INT &&
					scalar->Type != COL_TYPE_BIG_INT )
					throw verb_error(generic_error::INVALID_QUERY, this->getVerbType());
				int colNum = (int) scalar->Item.numval;
				--colNum;
				if( colNum < 0 || colNum >= (int) selectColumns.size() )
					throw verb_error(generic_error::INVALID_QUERY, this->getVerbType());
				columns.push_back( selectColumns[colNum] );
			}
			break;
		default:
			throw verb_error(generic_error::INVALID_QUERY, this->getVerbType());
		}
	}

	TablePartition::Ptr partition = NULL;
	if( this->Context == K_SELECT )
	{
		if( resRight )
		{
			assert( resRight->getType() == VerbResult::TABLE_PARTITION );
			partition = boost::static_pointer_cast<TablePartition>( resRight );
		}
	}
	
	if( this->Context != K_SELECT )
	{
		//we do a final group by at the end of the query, but it should not interfere
		//with the results of a final order by, so do the group by before
		table->cleanRedundantColumns();
		table->groupBy();
		table->OrderByApplied = true;
	}
	//make copy so that the partition returned by PARTITION BY is not changed
	if( !partition )
	{
		partition = new TablePartition();
		partition->FrameEndType = TablePartition::AQ_RELATIVE;
		partition->Rows.push_back( 0 );
		if( table->Columns.size() > 0 )
			partition->Rows.push_back( (int) table->Columns[0]->Items.size() );
		else
			partition->Rows.push_back( 0 );
	}
	else
		if( !partition->FrameUnitsInitialized )
			partition->FrameEndType = TablePartition::AQ_RELATIVE;

	TablePartition::Ptr partitionCopy = new TablePartition( *partition );
	table->orderBy( columns, partitionCopy );
	
	if( this->Context == K_SELECT )
		this->Result = partition;
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
	aq::PreProcessSelect( pStart, *this->m_baseDesc );
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
	assert( resLeft );
	assert( resLeft->getType() == VerbResult::ARRAY ||
			resLeft->getType() == VerbResult::COLUMN );
	
	vector<Column::Ptr> columns;
	if( resLeft->getType() == VerbResult::ARRAY )
	{
	  VerbResultArray::Ptr resArray = boost::static_pointer_cast<VerbResultArray>( resLeft );
		for( size_t idx = 0; idx < resArray->Results.size(); ++idx )
		  columns.push_back( boost::static_pointer_cast<Column>( resArray->Results[idx] ) );
	}
	else
	  columns.push_back( boost::static_pointer_cast<Column>( resLeft ) );

	table->groupBy();
	table->Partition = new TablePartition();
	table->orderBy( columns, table->Partition );
	table->GroupByApplied = true;
	/*

	Column::Ptr foundColumn = NULL;
	//get extra columns needed for group by
	Column::Ptr count = NULL;
	if( table->HasCount )
	{
		count = table->Columns[table->Columns.size() - 1];
		table->Columns.pop_back();
	}
	for( size_t idx = 0; idx < columns.size(); ++idx )
	{
		bool found = false;
		for( size_t idx2 = 0; idx2 < table->Columns.size(); ++idx2 )
			if( table->Columns[idx2]->getName() == columns[idx]->getName() )
			{
				table
				break;
			}
			if( !found )
			{
				if( !foundColumn )
					foundColumn = columns[idx];
				columns[idx]->Invisible = true;
				table->Columns.push_back( columns[idx] );
			}
	}
	if( table->HasCount )
		table->Columns.push_back( count );

	//make sure to grow ex-scalars if needed
	if( foundColumn )
		for( size_t idx = 0; idx < table->Columns.size(); ++idx )
			if( table->Columns[idx]->Items.size() < foundColumn->Items.size() )
				table->Columns[idx]->increase( foundColumn->Items.size() );
	*/
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
	if( !resLeft )
		return;

	if( resLeft->getType() != VerbResult::ROW_VALIDATION )
		throw verb_error(verb_error::VERB_BAD_SYNTAX, this->getVerbType());
	RowValidation::Ptr rv = boost::dynamic_pointer_cast<RowValidation>( resLeft );

	//recreate table
	vector<Column::Ptr> newColumns = table->getColumnsTemplate();

	for( size_t idx = 0; idx < rv->ValidRows.size(); ++idx )
	{
		if( !rv->ValidRows[idx] )
			continue;
		for( size_t idx2 = 0; idx2 < table->Columns.size(); ++idx2 )
			newColumns[idx2]->Items.push_back( table->Columns[idx2]->Items[idx] );
	}

	table->updateColumnsContent( newColumns );

	//update group by partitions
	std::vector<size_t>& oldRows = table->Partition->Rows;
	assert( oldRows.size() > 0 );
	std::vector<size_t> rows;
	size_t skipped = 0;
	for( size_t idx = 0; idx < oldRows.size() - 1; ++idx )
		if( !rv->ValidRows[oldRows[idx]] )
		{
			skipped += oldRows[idx+1] - oldRows[idx];
		}
		else
		{
			size_t last = oldRows[idx] - skipped;
			if( rows.size() == 0 || rows[rows.size() - 1] != last )
				rows.push_back( last );
			rows.push_back( oldRows[idx + 1] - skipped );
		}
	if( rows.size() == 0 )
	{
		rows.push_back( 0 );
		rows.push_back( 0 );
	}
	table->Partition->Rows = rows;
}

//------------------------------------------------------------------------------
void HavingVerb::accept(VerbVisitor* visitor)
{
  visitor->visit(this);
}

}
}
