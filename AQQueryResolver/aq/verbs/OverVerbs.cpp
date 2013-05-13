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

//------------------------------------------------------------------------------
bool PartitionVerb::changeQuery( tnode* pStart, tnode* pNode, VerbResult::Ptr resLeft, VerbResult::Ptr resRight, VerbResult::Ptr resNext )
{
  return false;
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

	TablePartition::Ptr partition = NULL;
	if( resRight )
	{
		assert( resRight->getType() == VerbResult::TABLE_PARTITION );
		partition = static_pointer_cast<TablePartition>( resRight );
	}
	else
		partition = new TablePartition();
	table->orderBy( columns, partition );

	this->Result = partition;
}

//------------------------------------------------------------------------------
void PartitionVerb::addResult( aq::RowProcess_Intf::Row& row, VerbResult::Ptr resLeft, VerbResult::Ptr resRight, VerbResult::Ptr resNext )
{
}

//------------------------------------------------------------------------------
void PartitionVerb::accept(VerbVisitor* visitor)
{
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
			type = TablePartition::AQ_UNBOUNDED; offset = 0; break;
		case K_INTEGER: 
			type = TablePartition::AQ_RELATIVE;
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
			type = TablePartition::AQ_UNBOUNDED; offset = 0; break;
		case K_INTEGER: 
			type = TablePartition::AQ_RELATIVE;
			offset = (int) bound->left->data.val_int; 
			break;
		default: assert(0);
		}
		break;
	case K_CURRENT: type = TablePartition::AQ_RELATIVE; offset = 0; break;
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
			partition->FrameEndType = TablePartition::AQ_RELATIVE;
		}
		break;
	default:
		assert( 0 );
	}
	this->Result = partition;
	return false;
}

//------------------------------------------------------------------------------
void FrameVerb::accept(VerbVisitor* visitor)
{
}
