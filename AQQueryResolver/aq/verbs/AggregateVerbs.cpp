#include "AggregateVerbs.h"
#include "VerbVisitor.h"
#include <aq/Exceptions.h>

using namespace std;

namespace aq {
namespace verb {

//------------------------------------------------------------------------------
VERB_IMPLEMENT(AggregateVerb);

//------------------------------------------------------------------------------
AggregateVerb::AggregateVerb()
  : index(-1), count(0)
{}

//------------------------------------------------------------------------------
bool AggregateVerb::changeQuery(	aq::tnode* pStart, aq::tnode* pNode,
									VerbResult::Ptr resLeft,
									VerbResult::Ptr resRight, VerbResult::Ptr resNext )
{
	if( this->Context != K_SELECT && this->Context != K_HAVING )
		throw generic_error( generic_error::AGGREGATE_NOT_IN_SELECT_OR_HAVING, "" );
	pNode->tag = K_DELETED;
	if( !resLeft )
		return false;
	return true;
}

//------------------------------------------------------------------------------
void AggregateVerb::changeResult(	Table::Ptr table, 
									VerbResult::Ptr resLeft,
									VerbResult::Ptr resRight,
									VerbResult::Ptr resNext )
{
	if( this->Result )
		return;
	Column::Ptr column = dynamic_pointer_cast<Column>(resLeft);
	TablePartition::Ptr partition = NULL;
	if( resRight )
	{
		assert( resRight->getType() == VerbResult::TABLE_PARTITION );
		partition = static_pointer_cast<TablePartition>(resRight);
	}

	bool oldGroupByApplied = table->GroupByApplied;
	if( table->GroupByApplied && !partition )
	{
		//apply the same algorithm used for 
		//PARTITION BY UNBOUNDED PRECEDING UNBOUNDED FOLLOWING
		//but as if there is no GROUP BY (count column counts)
		table->GroupByApplied = false;
		partition = table->Partition;
	}
	if( partition )
	{
		if( partition->FrameStartType == TablePartition::AQ_UNBOUNDED &&
			partition->FrameEndType == TablePartition::AQ_UNBOUNDED &&
			partition->FrameUnits == TablePartition::ROWS )
		{
			Column::Ptr result = new Column();
			for( size_t idx = 0; idx < partition->Rows.size() - 1; ++idx )
			{
				size_t partitionStart = partition->Rows[idx];
				size_t partitionEnd = partition->Rows[idx + 1];

				Scalar::Ptr scalar = this->computeResultRegular( column, table,
					partitionStart, partitionEnd );
				result->Type = scalar->Type;
				for( size_t idx2 = partitionStart; idx2 < partitionEnd; ++idx2 )
					result->Items.push_back( new ColumnItem(scalar->Item) );
			}
			if( result->Items.size() == 0 )
				throw generic_error(generic_error::GENERIC, "");
			this->Result = result;
		}
		else
			this->Result = this->computeResultPartition( column, table, partition );
	}
	else
	{
		llong end = 0;
		if( column )
			end = column->Items.size();
		this->Result = this->computeResultRegular( column, table, 0, end );
	}
	
	table->GroupByApplied = oldGroupByApplied;
}

//------------------------------------------------------------------------------
void AggregateVerb::addResult(aq::Row& row)
{
  if (this->index == -1)
  {
    assert(row.computedRow.size() <= std::numeric_limits<size_t>::max());
    this->index = static_cast<int>(row.computedRow.size()) - 1;
    aq::row_item_t& row_item = row.computedRow[this->index];
    switch (this->getVerbType())
    {
    case K_MIN:
      row_item.aggFunc = aq::aggregate_function_t::MIN;
      break;
    case K_MAX:
      row_item.aggFunc = aq::aggregate_function_t::MAX;
      break;
    case K_SUM:
      row_item.aggFunc = aq::aggregate_function_t::SUM;
      break;
    case K_AVG:
      row_item.aggFunc = aq::aggregate_function_t::AVG;
      break;
    case K_COUNT:
      row_item.aggFunc = aq::aggregate_function_t::COUNT;
      break;
    }
  }

  aq::row_item_t& row_item = row.computedRow[this->index];
  if ((this->count == 0) || (row.reinit))
  {
    this->count = 0;
    this->item = *row_item.item;
  }
  else
  {
    aq::apply_aggregate(row_item.aggFunc, row_item.type, this->item, this->count, *row_item.item, row.count);
  }
  this->count += row.count;
 
  if (this->getRightChild() != NULL)
  {
    // FIXME : manage partition and Frame
    *row_item.item = this->item;
    row.completed = true;
    // this->count = 0;
    // this->item.numval = 0;
    // this->item.strval = "";
  }
  else if (row.completed)
  {
    *row_item.item = this->item;
    this->count = 0;
    this->item.numval = 0;
    this->item.strval = "";
  }

}

//------------------------------------------------------------------------------
void AggregateVerb::accept(VerbVisitor* visitor)
{
  visitor->visit(this);
}

//------------------------------------------------------------------------------
VERB_IMPLEMENT(SumVerb);

//------------------------------------------------------------------------------
SumVerb::SumVerb()
{
}

//------------------------------------------------------------------------------
//auxiliary function for AVG and SUM
double computeSum( Column::Ptr column, Column::Ptr count, size_t start, size_t end )
{
	assert( column );
	assert( column->Type == COL_TYPE_INT || column->Type == COL_TYPE_DOUBLE ||
			column->Type == COL_TYPE_BIG_INT );
	double sum = 0;
	if( count )
		for( size_t idx = start; idx < end; ++idx )
			sum += column->Items[idx]->numval * count->Items[idx]->numval;
	else
		for( size_t idx = start; idx < end; ++idx )
			sum += column->Items[idx]->numval;
	return sum;
}

//------------------------------------------------------------------------------
Column::Ptr computeSumPartition(	Column::Ptr column, 
									Table::Ptr table,
									TablePartition::Ptr partition )
{
	table->unravel( partition );
	Column::Ptr sum = new Column();
	sum->Type = column->Type;
	for( size_t idx = 0; idx < partition->Rows.size() - 1; ++idx )
	{
		llong partitionStart = partition->Rows[idx];
		llong partitionEnd = partition->Rows[idx + 1];
		double sumVal = 0;
		//compute value of the first sum
		llong start;
		if( partition->FrameStartType == TablePartition::AQ_UNBOUNDED )
			start = partitionStart;
		else
			start = partitionStart + partition->FrameStart;
		llong end;
		if( partition->FrameEndType == TablePartition::AQ_UNBOUNDED )
			end = partitionEnd;
		else
			end = partitionStart + partition->FrameEnd + 1;
		llong startAux = max(start, partitionStart);
		llong endAux = min(end, partitionEnd);
		for( llong idx2 = startAux; idx2 < endAux; ++idx2 )
			sumVal += column->Items[idx2]->numval;

		for( llong idx2 = partitionStart; idx2 < partitionEnd; ++idx2 )
		{
			llong oldStart = start;
			llong oldEnd = end;
			if( partition->FrameStartType == TablePartition::AQ_UNBOUNDED )
				start = partitionStart;
			else
				start = idx2 + partition->FrameStart;
			if( partition->FrameEndType == TablePartition::AQ_UNBOUNDED )
				end = partitionEnd;
			else
				end = idx2 + partition->FrameEnd + 1;
			if( start > oldStart && oldStart >= partitionStart && oldStart < partitionEnd )
				sumVal -= column->Items[oldStart]->numval;
			if( end > oldEnd && end <= partitionEnd && end > partitionStart )
				sumVal += column->Items[end-1]->numval;
			if( end > partitionStart && start < partitionEnd )
				sum->Items.push_back( new ColumnItem(sumVal) );
			else
				sum->Items.push_back( NULL );
		}
	}
	return sum;
}

//------------------------------------------------------------------------------
VerbResult::Ptr SumVerb::computeResultPartition(	Column::Ptr column, 
													Table::Ptr table,
													TablePartition::Ptr partition )
{
	assert( column );
	if( column->Type != COL_TYPE_BIG_INT && 
		column->Type != COL_TYPE_DOUBLE && 
		column->Type != COL_TYPE_INT
		)
		throw verb_error( generic_error::VERB_TYPE_MISMATCH, this->getVerbType() );

	return computeSumPartition( column, table, partition );
}

//------------------------------------------------------------------------------
Scalar::Ptr SumVerb::computeResultRegular(	Column::Ptr column, 
											Table::Ptr table,
											llong start,
											llong end )
{
	assert( column );
	if( column->Type != COL_TYPE_BIG_INT && 
		column->Type != COL_TYPE_DOUBLE && 
		column->Type != COL_TYPE_INT
		)
		throw verb_error( generic_error::VERB_TYPE_MISMATCH, this->getVerbType() );
	Column::Ptr count = NULL;
	if( table->HasCount )
		count = table->Columns[table->Columns.size() - 1];
	double sum = computeSum(column, count, start, end);
	return new Scalar(column->Type, static_cast<unsigned>(column->Size), ColumnItem(sum));
}

//------------------------------------------------------------------------------
//void SumVerb::accept(VerbVisitor* visitor)
//{
//	visitor->visit(this);
//}

//------------------------------------------------------------------------------
VERB_IMPLEMENT( CountVerb );

//------------------------------------------------------------------------------
CountVerb::CountVerb()
{}

//------------------------------------------------------------------------------
bool CountVerb::preprocessQuery(	aq::tnode* pStart, aq::tnode* pNode, aq::tnode* pStartOriginal )
{
	//count parameter is irrelevant
	delete_subtree( pNode->left );
	pNode->left = NULL;
	return false;
}

//------------------------------------------------------------------------------
llong computeCount( Table::Ptr table, llong start, llong end )
{
	if( table->Columns.size() == 0 ||
		( start == 0 ) && ( end == table->Columns[0]->Items.size() ) ||
		( end <= 0 ) )
		return table->TotalCount;
	if( !table->HasCount )
		return end - start;
	Column::Ptr count = table->Columns[table->Columns.size() - 1];
	return (llong) computeSum(count, NULL, start, end);
}

//------------------------------------------------------------------------------
Scalar::Ptr CountVerb::computeResultRegular(	Column::Ptr column, 
												Table::Ptr table,
												llong start,
												llong end )
{
	llong count = computeCount(table, start, end);
	return new Scalar(COL_TYPE_INT, 4, ColumnItem( (double) count ));
}

//------------------------------------------------------------------------------
Column::Ptr computeCountPartition(	Column::Ptr column, 
									Table::Ptr table,
									TablePartition::Ptr partition )
{
	table->unravel( partition );
	Column::Ptr count = new Column();
	count->Type = COL_TYPE_INT;
	for( size_t idx = 0; idx < partition->Rows.size() - 1; ++idx )
	{
		llong partitionStart = partition->Rows[idx];
		llong partitionEnd = partition->Rows[idx + 1];

		for( llong idx2 = partitionStart; idx2 < partitionEnd; ++idx2 )
		{
			llong start, end;
			if( partition->FrameStartType == TablePartition::AQ_UNBOUNDED )
				start = partitionStart;
			else
				start = max(partitionStart, idx2 + partition->FrameStart);
			if( partition->FrameEndType == TablePartition::AQ_UNBOUNDED )
				end = partitionEnd;
			else
				end = min(partitionEnd, idx2 + partition->FrameEnd + 1);
			if( start >= end )
				count->Items.push_back( NULL );
			else
				count->Items.push_back( new ColumnItem((double) (end - start)) );
		}
	}
	return count;
}

//------------------------------------------------------------------------------
VerbResult::Ptr CountVerb::computeResultPartition(	Column::Ptr column, 
													Table::Ptr table,
													TablePartition::Ptr partition )
{
	return computeCountPartition( column, table, partition );
}

//------------------------------------------------------------------------------
//void CountVerb::accept(VerbVisitor* visitor)
//{
//	visitor->visit(this);
//}

//------------------------------------------------------------------------------
VERB_IMPLEMENT(AvgVerb);

//------------------------------------------------------------------------------
AvgVerb::AvgVerb()
{
}

//------------------------------------------------------------------------------
Scalar::Ptr AvgVerb::computeResultRegular(	Column::Ptr column, 
											Table::Ptr table,
											llong start,
											llong end )
{
	assert( column );
	if( column->Type != COL_TYPE_BIG_INT && 
		column->Type != COL_TYPE_DOUBLE && 
		column->Type != COL_TYPE_INT
		)
		throw verb_error( generic_error::VERB_TYPE_MISMATCH, this->getVerbType() );

	Column::Ptr countCol = NULL;
	if( table->HasCount )
		countCol = table->Columns[table->Columns.size() - 1];
	double sum = computeSum(column, countCol, start, end);
	llong count = computeCount(table, start, end);
	double avg;
	if( column->Type == COL_TYPE_DOUBLE )
		avg = sum / count;
	else
		avg = (double)((llong)((sum / count) + 0.5));
	return new Scalar(column->Type, static_cast<unsigned>(column->Size), ColumnItem(avg));
}

//------------------------------------------------------------------------------
VerbResult::Ptr AvgVerb::computeResultPartition(	Column::Ptr column, 
													Table::Ptr table,
													TablePartition::Ptr partition )
{
	assert( column );
	if( column->Type != COL_TYPE_BIG_INT && 
		column->Type != COL_TYPE_DOUBLE && 
		column->Type != COL_TYPE_INT
		)
		throw verb_error( generic_error::VERB_TYPE_MISMATCH, this->getVerbType() );

	Column::Ptr sum = computeSumPartition( column, table, partition );
	Column::Ptr count = computeCountPartition( column, table, partition );
	Column::Ptr avg = new Column(*column);
	for( size_t idx = 0; idx < sum->Items.size(); ++idx )
	{
		if( !sum->Items[idx] || !count->Items[idx] )
		{
			avg->Items.push_back( NULL );
			continue;
		}
		double avgVal;
		double sumVal = sum->Items[idx]->numval;
		double countVal = count->Items[idx]->numval;
		if( column->Type == COL_TYPE_DOUBLE )
			avgVal = sumVal / countVal;
		else
			avgVal = (double)((llong)((sumVal / countVal) + 0.5));
		avg->Items.push_back( new ColumnItem(avgVal) );
	}

	return avg;
}

//------------------------------------------------------------------------------
//void AvgVerb::accept(VerbVisitor* visitor)
//{
//	visitor->visit(this);
//}

//------------------------------------------------------------------------------
VERB_IMPLEMENT(MinVerb);

//------------------------------------------------------------------------------
MinVerb::MinVerb()
{}

//------------------------------------------------------------------------------
Scalar::Ptr MinVerb::computeResultRegular(	Column::Ptr column, 
											Table::Ptr table,
											llong start,
											llong end )
{
	assert( column );
	Scalar::Ptr scalar = new Scalar( column->Type );
	size_t minMaxIdx = start;
	for( llong idx = start + 1; idx < end; ++idx )
		if( ColumnItem::lessThan(	column->Items[idx].get(), 
						column->Items[minMaxIdx].get(), 
						column->Type)  )
			minMaxIdx = idx;
	scalar->Item = *column->Items[minMaxIdx];
	return scalar;
}

//------------------------------------------------------------------------------
Column::Ptr computeMinMaxPartition(	Column::Ptr column, 
									TablePartition::Ptr partition,
									bool isMin )
{
	Column::Ptr minMaxCol = new Column(*column);

	if( partition->FrameStartType == TablePartition::AQ_UNBOUNDED &&
		partition->FrameEndType == TablePartition::AQ_RELATIVE )
	{
		for( size_t idx = 0; idx < partition->Rows.size() - 1; ++idx )
		{
			llong partitionStart = partition->Rows[idx];
			llong partitionEnd = partition->Rows[idx + 1];

			ColumnItem::Ptr minMaxVal = column->Items[partitionStart];
			llong end = partitionStart + partition->FrameEnd + 1;
			for( llong idx2 = partitionStart + 1; idx2 < end; ++idx2 )
				if( ColumnItem::lessThan( column->Items[idx2].get(), 
					minMaxVal.get(), column->Type ) == isMin )
					minMaxVal = column->Items[idx2];

			for( llong idx2 = partitionStart; idx2 < partitionEnd; ++idx2 )
			{
				end = idx2 + partition->FrameEnd + 1;
				if( end <= partitionStart )
					minMaxCol->Items.push_back( NULL );
				else
				{
					if( end <= partitionEnd &&
						ColumnItem::lessThan( column->Items[end-1].get(), 
						minMaxVal.get(), column->Type ) == isMin )
						minMaxVal = column->Items[end-1];
					minMaxCol->Items.push_back( new ColumnItem(*minMaxVal) );
				}
			}
		}
	}
	else if( partition->FrameStartType == TablePartition::AQ_RELATIVE &&
		partition->FrameEndType == TablePartition::AQ_UNBOUNDED )
	{
		for( size_t idx = 0; idx < partition->Rows.size() - 1; ++idx )
		{
			llong partitionStart = partition->Rows[idx];
			llong partitionEnd = partition->Rows[idx + 1];

			ColumnItem::Ptr minMaxVal = column->Items[partitionEnd-1];
			llong start = partitionEnd - 1 + partition->FrameStart;
			for( llong idx2 = partitionEnd - 2; idx2 >= start; --idx2 )
				if( ColumnItem::lessThan( column->Items[idx2].get(), 
					minMaxVal.get(), column->Type ) == isMin )
					minMaxVal = column->Items[idx2];

			vector<ColumnItem::Ptr> tempCol;
			for( llong idx2 = partitionEnd-1; idx2 >= partitionStart; --idx2 )
			{
				start = idx2 + partition->FrameStart;
				if( start >= partitionEnd )
					minMaxCol->Items.push_back( NULL );
				else
				{
					if( start >= partitionStart &&
						ColumnItem::lessThan( column->Items[start].get(), 
						minMaxVal.get(), column->Type ) == isMin )
						minMaxVal = column->Items[start];
					tempCol.push_back( new ColumnItem(*minMaxVal) );
				}
			}
			for( size_t idx = tempCol.size(); idx > 0; --idx )
				minMaxCol->Items.push_back( new ColumnItem(*tempCol[idx-1]) );
		}
	}
	else if( partition->FrameStartType == TablePartition::AQ_RELATIVE &&
		partition->FrameEndType == TablePartition::AQ_RELATIVE )
	{
		for( size_t idx = 0; idx < partition->Rows.size() - 1; ++idx )
		{
			llong partitionStart = partition->Rows[idx];
			llong partitionEnd = partition->Rows[idx + 1];

			for( llong idx2 = partitionStart; idx2 < partitionEnd; ++idx2 )
			{
				llong start, end;
				if( partition->FrameStartType == TablePartition::AQ_UNBOUNDED )
					start = partitionStart;
				else
					start = max(partitionStart, idx2 + partition->FrameStart);
				if( partition->FrameEndType == TablePartition::AQ_UNBOUNDED )
					end = partitionEnd;
				else
					end = min(partitionEnd, idx2 + partition->FrameEnd + 1);
				if( start >= end )
					minMaxCol->Items.push_back( NULL );
				else
				{
					ColumnItem::Ptr minMaxVal = column->Items[start];
					for( llong idx3 = start + 1; idx3 < end; ++idx3 )
						if( ColumnItem::lessThan(column->Items[idx3].get(), 
							minMaxVal.get(), column->Type) == isMin )
							minMaxVal = column->Items[idx3];
					minMaxCol->Items.push_back( new ColumnItem(*minMaxVal) );
				}
			}
		}
	}
	else
		throw generic_error(generic_error::GENERIC, "");

	return minMaxCol;
}

//------------------------------------------------------------------------------
VerbResult::Ptr MinVerb::computeResultPartition(	Column::Ptr column, 
													Table::Ptr table,
													TablePartition::Ptr partition )
{
	assert( column );
	return computeMinMaxPartition( column, partition, true );
}

//------------------------------------------------------------------------------
//void MinVerb::accept(VerbVisitor* visitor)
//{
//	visitor->visit(this);
//}

//------------------------------------------------------------------------------
VERB_IMPLEMENT(MaxVerb);

//------------------------------------------------------------------------------
MaxVerb::MaxVerb()
{}

//------------------------------------------------------------------------------
Scalar::Ptr MaxVerb::computeResultRegular(	Column::Ptr column, 
											Table::Ptr table,
											llong start,
											llong end )
{
	assert( column );
	Scalar::Ptr scalar = new Scalar( column->Type );
	size_t minMaxIdx = start;
	for( llong idx = start + 1; idx < end; ++idx )
		if( ColumnItem::lessThan(	column->Items[minMaxIdx].get(),
						column->Items[idx].get(),
						column->Type)  )
						minMaxIdx = idx;
	scalar->Item = *column->Items[minMaxIdx];
	return scalar;
}

//------------------------------------------------------------------------------
VerbResult::Ptr MaxVerb::computeResultPartition(	Column::Ptr column, 
													Table::Ptr table,
													TablePartition::Ptr partition )
{
	assert( column );
	return computeMinMaxPartition( column, partition, false );
}

//------------------------------------------------------------------------------
//void MaxVerb::accept(VerbVisitor* visitor)
//{
//	visitor->visit(this);
//}

//------------------------------------------------------------------------------
VERB_IMPLEMENT( FirstValueVerb );

//------------------------------------------------------------------------------
FirstValueVerb::FirstValueVerb()
{}

//------------------------------------------------------------------------------
Column::Ptr OffsetColumn(	Column::Ptr column, TablePartition::Ptr partition, 
							bool firstValue, llong offsetVal, Table::Ptr table,
							ColumnItem::Ptr defaultValue )
{
	Column::Ptr newColumn = new Column( "", 0, 0, column->Type );
	vector<size_t>& partitions = partition->Rows;
	size_t partIdx = 0;
	assert( partitions.size() > 1 );
	assert( partitions[partitions.size() - 1] >= column->Items.size() );
	Column::Ptr columnCount = column->getCount();
	Column::Ptr newColumnCount = NULL;
	if( columnCount )
	{
		newColumnCount = new Column( *columnCount );
		assert( table->HasCount );
	}
	vector<llong> newColumnToColumnMap;
	for( size_t idx = 0; idx < column->Items.size(); ++idx )
	{
		if( idx >= partitions[partIdx + 1] )
		{
			++partIdx;
			assert( partitions[partIdx] - partitions[partIdx-1] > 0 );
		}

		//get itemIndex such that column[itemIndex] contains the row that is 
		//Offset rows behind the first row in column[idx]
		//(remember that each column[idx] can represent more than 1 row)
		llong offset = offsetVal;
		size_t itemIndex = (idx > 0) ? idx - 1 : std::string::npos;
		while((itemIndex != std::string::npos) && (itemIndex >= partitions[partIdx]) && (offset > columnCount->Items[itemIndex]->numval) )
		{
			offset -= static_cast<llong>(columnCount->Items[itemIndex]->numval);
			--itemIndex;
		}

		//lag and create new rows as needed
		llong count = 1;
		if( columnCount )
			count = static_cast<llong>(columnCount->Items[idx]->numval);
		llong unsolvedItems = count;
		bool solvedItemsInIdx = false;
		while( unsolvedItems > 0 )
		{	
			ColumnItem::Ptr lagItem = defaultValue;
			//get the item as long as it doesn't pass the upper partition boundary
			if ((itemIndex != std::string::npos) && (itemIndex >= partitions[partIdx]))
				lagItem = column->Items[itemIndex];
			else
				if( firstValue )
					lagItem = column->Items[partitions[partIdx]]; //get the first available item

			llong itemIndexCount = 1;
			if( columnCount )
			{
				//test if we are at the first column[idx] we use to solve rows
				//if yes then it solves offset rows for us
				if( !solvedItemsInIdx ) 
					itemIndexCount = (llong) offset;
				else
					itemIndexCount = (llong) columnCount->Items[itemIndex]->numval;
			}
			//get number of rows we can solve at this step, using column[itemIndex] value
			llong solveItems = min(unsolvedItems, itemIndexCount);
			llong newColumnSize = (llong) newColumn->Items.size();
			if( columnCount && solvedItemsInIdx &&
				ColumnItem::equal(	lagItem.get(), 
						newColumn->Items[newColumnSize - 1].get(), 
						column->Type) )
			{
				if( columnCount )
					newColumnCount->Items[newColumnSize - 1]->numval += solveItems;
			}
			else
			{
				newColumn->Items.push_back( lagItem );
				if( columnCount )
					newColumnCount->Items.push_back( new ColumnItem((double) solveItems) );
				newColumnToColumnMap.push_back((llong) idx);
				solvedItemsInIdx = true;
			}
			unsolvedItems -= solveItems;
			++itemIndex;
		}
		assert( itemIndex <= (idx + 1) );
	}

	assert( newColumn->Items.size() >= column->Items.size() );

	if( newColumn->Items.size() == column->Items.size() )
		return newColumn; //no need to change table

	//recreate table (split row groups)
	vector<Column::Ptr> newColumns = table->getColumnsTemplate();
	if( table->HasCount )
		newColumns.pop_back();

	for( size_t idx = 0; idx < newColumn->Items.size(); ++idx )
		for( size_t idx2 = 0; idx2 < newColumns.size(); ++idx2 )
			newColumns[idx2]->Items.push_back( 
			table->Columns[idx2]->Items[newColumnToColumnMap[idx]] );
	if( table->HasCount )
		newColumns.push_back( newColumnCount );
	table->updateColumnsContent( newColumns );

	return newColumn;
}

//------------------------------------------------------------------------------
void FirstValueVerb::changeResult(	Table::Ptr table, 
									VerbResult::Ptr resLeft, 
									VerbResult::Ptr resRight, VerbResult::Ptr resNext )
{
	Column::Ptr column = dynamic_pointer_cast<Column>(resLeft);
	assert( column );
	TablePartition::Ptr partition = dynamic_pointer_cast<TablePartition>(resRight);
	assert( partition );
	if( partition->FrameStartType == TablePartition::AQ_RELATIVE )
	{
		llong offset = partition->FrameStart >= 0 ? partition->FrameStart : -partition->FrameStart;
		this->Result = OffsetColumn( column, partition, true, offset, table, NULL );
	}
	else
	{
		//UNBOUNDED PRECEDING
		Column::Ptr newColumn = new Column( "", static_cast<unsigned int>(0), static_cast<unsigned int>(column->Size), column->Type );
		vector<size_t>& partitions = partition->Rows;
		int partIdx = 0;
		assert( partitions.size() > 1 );
		assert( partitions[partitions.size() - 1] >= column->Items.size() );
		for( size_t idx = 0; idx < column->Items.size(); ++idx )
		{
			if( idx >= partitions[partIdx + 1] )
			{
				++partIdx;
				assert( partitions[partIdx] - partitions[partIdx-1] > 0 );
			}
			newColumn->Items.push_back( column->Items[partitions[partIdx]] );
		}

		this->Result = newColumn;
	}
}

//------------------------------------------------------------------------------
void FirstValueVerb::accept(VerbVisitor* visitor)
{
  visitor->visit(this);
}

//------------------------------------------------------------------------------
VERB_IMPLEMENT( LagVerb );

//------------------------------------------------------------------------------
LagVerb::LagVerb(): Offset(0), Default(NULL)
{}

//------------------------------------------------------------------------------
LagVerb::~LagVerb()
{
	delete this->Default ;
}

//------------------------------------------------------------------------------
bool LagVerb::preprocessQuery( aq::tnode* pStart, aq::tnode* pNode, aq::tnode* pStartOriginal )
{
	//debug13 - should have a K_VALUE verb that returns a scalar, but for now
	//I inspect the query directly, in order to avoid the potential ripple effects
	//caused by implementing such a verb
	assert( pNode );
	assert( pNode->tag == K_LAG );
	assert( pNode->left );
	if( pNode->left->tag == K_COMMA )
	{
		assert( pNode->left->right );
		if( pNode->left->right->tag == K_COMMA )
		{
			aq::tnode* pAux = pNode->left->right;
			assert( pAux->left && pAux->left->tag == K_INTEGER );
			this->Offset = pAux->left->getData().val_int;
			assert( pAux->right );
			this->Default = new aq::tnode( *pAux->right );
		}
		else
		{
			assert( pNode->left->right->tag == K_INTEGER );
			this->Offset = pNode->left->right->getData().val_int;
		}
	}
	return false;
}

//------------------------------------------------------------------------------
void LagVerb::changeResult(	Table::Ptr table, 
							VerbResult::Ptr resLeft, 
							VerbResult::Ptr resRight, VerbResult::Ptr resNext )
{
	VerbResultArray::Ptr params = dynamic_pointer_cast<VerbResultArray>(resLeft);
	assert(params && params->Results.size() > 0);
	Column::Ptr column = dynamic_pointer_cast<Column>(params->Results[0]);
	assert( column );
	TablePartition::Ptr partition = dynamic_pointer_cast<TablePartition>(resRight);
	assert( partition );

	ColumnItem::Ptr defaultValue = NULL;
	if( this->Default )
	{
		switch( column->Type )
		{
		case COL_TYPE_INT:
		case COL_TYPE_BIG_INT:
		case COL_TYPE_DATE:
		case COL_TYPE_DOUBLE:
			defaultValue = new ColumnItem(static_cast<double>(this->Default->getData().val_int));
			break;
		case COL_TYPE_VARCHAR:
			defaultValue = new ColumnItem(this->Default->getData().val_str);
			break;
		}
	}
	this->Result = OffsetColumn( column, partition, false, this->Offset, table,
		defaultValue );
}

//------------------------------------------------------------------------------
void LagVerb::accept(VerbVisitor* visitor)
{
}
//------------------------------------------------------------------------------
VERB_IMPLEMENT( RowNumberVerb );

//------------------------------------------------------------------------------
RowNumberVerb::RowNumberVerb()
{}

//------------------------------------------------------------------------------
void RowNumberVerb::changeResult(	Table::Ptr table, 
									VerbResult::Ptr resLeft, 
									VerbResult::Ptr resRight,
									VerbResult::Ptr resNext )
{
	assert( resRight->getType() == VerbResult::TABLE_PARTITION );
	TablePartition::Ptr partition = static_pointer_cast<TablePartition>( resRight );
	assert( partition );

	Column::Ptr count = table->Columns[table->Columns.size() - 1];

	//create new column
	Column::Ptr newColumn = new Column( COL_TYPE_BIG_INT );
	for( size_t idx = 0; idx + 1 < partition->Rows.size(); ++idx )
	{
		size_t rowNumber = 0;
		for( size_t idx2 = partition->Rows[idx]; idx2 < partition->Rows[idx+1]; ++idx2 )
			for( size_t idx3 = 0; idx3 < (size_t)count->Items[idx2]->numval; ++idx3 )
				newColumn->Items.push_back( new ColumnItem( (double) ++rowNumber ) );
	}

	this->Result = newColumn;
	table->unravel( partition );
}

//------------------------------------------------------------------------------
void RowNumberVerb::accept(VerbVisitor* visitor)
{
  visitor->visit(this);
}

}
}
