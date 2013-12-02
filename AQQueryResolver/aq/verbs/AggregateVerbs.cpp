#include "AggregateVerbs.h"
#include "VerbVisitor.h"
#include <aq/Exceptions.h>

using namespace std;

namespace aq {
namespace verb {

//------------------------------------------------------------------------------
AggregateVerb::AggregateVerb()
  : index(-1), count(0)
{}

//------------------------------------------------------------------------------
bool AggregateVerb::changeQuery(aq::tnode* pStart, 
                                aq::tnode* pNode,
                                VerbResult::Ptr resLeft,
                                VerbResult::Ptr resRight, 
                                VerbResult::Ptr resNext )
{
	if ((this->Context != K_SELECT) && (this->Context != K_HAVING))
  {
		throw generic_error( generic_error::AGGREGATE_NOT_IN_SELECT_OR_HAVING, "" );
  }
  pNode->tag = K_DELETED;
	return resLeft != nullptr;
}

//------------------------------------------------------------------------------
void AggregateVerb::changeResult(	Table::Ptr table, 
									VerbResult::Ptr resLeft,
									VerbResult::Ptr resRight,
									VerbResult::Ptr resNext )
{
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
    this->item = row_item.item;
  }
  else
  {
    switch (row_item.type)
    {
    case aq::ColumnType::COL_TYPE_INT:
      boost::get<aq::ColumnItem<int32_t> >(this->item).applyAggregate(row_item.aggFunc, boost::get<aq::ColumnItem<int32_t> >(row_item.item));
      break;
    case aq::ColumnType::COL_TYPE_BIG_INT:
      boost::get<aq::ColumnItem<int64_t> >(this->item).applyAggregate(row_item.aggFunc, boost::get<aq::ColumnItem<int64_t> >(row_item.item));
      break;
    case aq::ColumnType::COL_TYPE_DOUBLE:
      boost::get<aq::ColumnItem<double> >(this->item).applyAggregate(row_item.aggFunc, boost::get<aq::ColumnItem<double> >(row_item.item));
      break;
    case aq::ColumnType::COL_TYPE_VARCHAR:
      boost::get<aq::ColumnItem<char> >(this->item).applyAggregate(row_item.aggFunc, boost::get<aq::ColumnItem<char> >(row_item.item));
      break;
    }
    // aq::apply_aggregate(row_item.aggFunc, row_item.type, this->item, this->count, *row_item.item, row.count);
  }

  if (!row_item.null)
  {
    this->count += row.count;
  }

  if (this->getRightChild() != nullptr)
  {
    // FIXME : manage partition and Frame
    row_item.item = this->item;
    row.completed = true;
    // this->count = 0;
    // this->item.numval = 0;
    // this->item.strval = "";
  }
  else if (row.completed)
  {
    row_item.item = this->item;
    if (row_item.aggFunc == aq::aggregate_function_t::COUNT)
    {
      row_item.type = aq::ColumnType::COL_TYPE_BIG_INT;
      // row_item.item->numval = static_cast<double>(this->count);
      row_item.null = false;
    }
    this->count = 0;
    // FIXME : reset item
    //this->item.numval = 0;
    //this->item.strval = "";
  }

}

//------------------------------------------------------------------------------
void AggregateVerb::accept(VerbVisitor* visitor)
{
  visitor->visit(this);
}

}
}
