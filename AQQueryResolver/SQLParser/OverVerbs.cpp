#include "OverVerbs.h"
#include <algorithm>
#include <aq/Exceptions.h>

using namespace aq;
using namespace std;
using namespace boost;
/*
//------------------------------------------------------------------------------
VERB_IMPLEMENT(OverVerb);

//------------------------------------------------------------------------------
OverVerb::OverVerb()
{}

//------------------------------------------------------------------------------
void OverVerb::changeResult(	Table::Ptr table, 
								VerbResult::Ptr resLeft, 
								VerbResult::Ptr resRight, 
								VerbResult::Ptr resNext )
{
	
}
*/
//------------------------------------------------------------------------------
VERB_IMPLEMENT(PartitionVerb);

//------------------------------------------------------------------------------
PartitionVerb::PartitionVerb()
{}

namespace{

//------------------------------------------------------------------------------
//Column* equalColumn; // tma FIXME
//bool equalColumnFct(int idx1, int idx2)
//{
//	return equal(	equalColumn->Items[idx1].get(), 
//					equalColumn->Items[idx2].get(), 
//					equalColumn->Type );
//}
//
////------------------------------------------------------------------------------
//Column* lessThanColumn; // tma FIXME
//bool lessThanColumnFct(int idx1, int idx2)
//{
//	return lessThan(	lessThanColumn->Items[idx1].get(), 
//						lessThanColumn->Items[idx2].get(), 
//						lessThanColumn->Type );
//}

}

//------------------------------------------------------------------------------
void PartitionVerb::changeResult(	Table::Ptr table, 
									VerbResult::Ptr resLeft, VerbResult::Ptr resRight, VerbResult::Ptr resNext )
{
	assert( resLeft );
	VerbResultArray::Ptr resArray = dynamic_pointer_cast<VerbResultArray>( resLeft );
	if( !resArray )
	{
		resArray = new VerbResultArray();
		resArray->Results.push_back( resLeft );
	}
	assert( resArray->Results.size() > 0 );
	vector<Column::Ptr> columns;
	for( size_t idx = 0; idx < resArray->Results.size(); ++idx )
	{
		Column::Ptr column = dynamic_pointer_cast<Column>( resArray->Results[idx] );
		assert( column );
		columns.push_back( column );
	}
	if( table->Columns[0]->Items.size() < 2 )
		return; //nothing to partition
/*
	//index = 0, 1, 2, 3 ..
	vector<size_t> index;
	index.resize( table->Columns[0]->Items.size() );
	for( size_t idx = 0; idx < index.size(); ++idx )
		index[idx] = idx;
*/
	TablePartition::Ptr partition = NULL;
	if( resRight )
	{
		assert( resRight->getType() == VerbResult::TABLE_PARTITION );
		partition = static_pointer_cast<TablePartition>( resRight );
	}
	else
		partition = new TablePartition();
	table->orderBy( columns, partition );
/*	std::vector<int>& partitions = partition->Rows;
	partitions.clear();
	partitions.push_back( 0 );

	//need final pass to gather partitions of last sorted column
	columns.push_back( NULL );
	
	for( size_t idx = 0; idx < columns.size(); ++idx )
	{
		Column::Ptr column;
		if( idx < columns.size() - 1 )
		{
			column = columns[idx];
			lessThanColumn = column.get();
		}

		Column::Ptr lastColumn;
		if( idx > 0 )
		{
			lastColumn = columns[idx - 1];
			equalColumn = lastColumn.get();
		}

		vector<size_t>::iterator itBegin = index.begin();
		vector<size_t>::iterator itEnd = index.begin();
		while( itEnd != index.end() )
		{
			++itEnd;
			bool foundPartition = false;
			if( idx == 0 || itEnd == index.end() )
			{
				itEnd = index.end();
				foundPartition = true;
			}
			else
			{
				foundPartition = !equalColumnFct( *itBegin, *itEnd );
			}
			if( foundPartition )
			{
				if( (idx == (columns.size() - 1)) )
				{
					if( itEnd != index.end() )
						partitions.push_back( itEnd - index.begin() );
				}	
				else
				{
					sort( itBegin, itEnd, lessThanColumnFct);
				}
				itBegin = itEnd;
			}
		}
	}

	partitions.push_back( index.size() );

	//create sorted table
	vector<Column::Ptr> newColumns;
	for( size_t idx = 0; idx < table->Columns.size(); ++idx )
	{
		Column& col = *table->Columns[idx].get();
		Column::Ptr newColumn = new Column( col.getOriginalName(), col.ID, 
			col.Size, col.Type );
		newColumns.push_back( newColumn );
		if( columns[columns.size()-2].get() == table->Columns[idx].get() )
			partition->LastColumn = newColumn;
	}

	for( size_t idx = 0; idx < index.size(); ++idx )
		for( size_t idx2 = 0; idx2 < table->Columns.size(); ++idx2 )
			newColumns[idx2]->Items.push_back( table->Columns[idx2]->Items[index[idx]] );
	
	table->Columns = newColumns;*/
	this->Result = partition;
}

//------------------------------------------------------------------------------
VERB_IMPLEMENT( FrameVerb );

//------------------------------------------------------------------------------
FrameVerb::FrameVerb()
{}

//------------------------------------------------------------------------------
void extractFrameTypeOffset(	tnode* bound, TablePartition::FrameBoundType& type, 
								int& offset, bool start, int verbTag )
{
	assert( bound );
	assert( !bound->right );
	switch( bound->tag )
	{
	case K_PRECEDING:
		assert( bound->left );
		switch( bound->left->tag )
		{
		case K_UNBOUNDED: 
			if( !start )
				throw verb_error( generic_error::VERB_BAD_SYNTAX, verbTag );
			type = TablePartition::UNBOUNDED; offset = 0; break;
		case K_INTEGER: 
			type = TablePartition::RELATIVE;
			offset = (int) -bound->left->data.val_int; 
			break;
		default: assert(0);
		}
		break;
	case K_FOLLOWING:
		assert( bound->left );
		switch( bound->left->tag )
		{
		case K_UNBOUNDED: 
			if( start )
				throw verb_error( generic_error::VERB_BAD_SYNTAX, verbTag );
			type = TablePartition::UNBOUNDED; offset = 0; break;
		case K_INTEGER: 
			type = TablePartition::RELATIVE;
			offset = (int) bound->left->data.val_int; 
			break;
		default: assert(0);
		}
		break;
	case K_CURRENT: type = TablePartition::RELATIVE; offset = 0; break;
	default:
		assert( 0 );
	}
}

//------------------------------------------------------------------------------
bool FrameVerb::preprocessQuery( tnode* pStart, tnode* pNode, tnode* pStartOriginal )
{
	assert( pNode );
	assert( pNode->left );
	TablePartition::Ptr partition = new TablePartition();
	partition->FrameUnitsInitialized = true;
	switch( pNode->left->tag )
	{
	case K_ROWS: partition->FrameUnits = TablePartition::ROWS; break;
	case K_RANGE: partition->FrameUnits = TablePartition::RANGE; break;
	default:
		assert( 0 );
	}
	assert( pNode->right );
	TablePartition::FrameBoundType type;
	int offset;
	switch( pNode->right->tag )
	{
	case K_AND:
		{
			tnode* startBound = pNode->right->left;
			extractFrameTypeOffset( startBound, type, offset, true, this->getVerbType() );
			partition->FrameStartType = type;
			partition->FrameStart = offset;
			tnode* endBound = pNode->right->right;
			extractFrameTypeOffset( endBound, type, offset, false, this->getVerbType() );
			partition->FrameEndType = type;
			partition->FrameEnd = offset;
		}
		break;
	case K_PRECEDING:
		{
			tnode* startBound = pNode->right;
			extractFrameTypeOffset( startBound, type, offset, true, this->getVerbType() );
			partition->FrameStartType = type;
			partition->FrameStart = offset;
			partition->FrameEndType = TablePartition::RELATIVE;
		}
		break;
	default:
		assert( 0 );
	}
	this->Result = partition;
	return false;
}