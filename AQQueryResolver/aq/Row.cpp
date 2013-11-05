#include "Row.h"

namespace aq
{

row_item_t::row_item_t()
  :
  item(new aq::ColumnItem),
  type(COL_TYPE_BIG_INT),
  size(0),
  tableName(""),
  columnName(""),
  computed(false),
  grouped(false),
  displayed(false),
  null(false)
{
}

row_item_t::row_item_t(ColumnItem::Ptr _item,
                       aq::ColumnType _type,
                       unsigned int _size,
                       std::string _tableName,
                       std::string _columnName,
                       bool _computed)
                       : item(_item),
                       type(_type),
                       size(_size),
                       tableName(_tableName),
                       columnName(_columnName),
                       computed(_computed),
                       grouped(false),
                       displayed(false),
                       null(false)
{
}

row_item_t::row_item_t(const row_item_t& source)
  : 
  type(source.type),
  size(source.size),
  tableName(source.tableName),
  columnName(source.columnName),
  aggFunc(source.aggFunc),
  computed(source.computed),
  grouped(source.grouped),
  displayed(source.displayed),
  null(source.null)
{
  if (source.item)
  {
    this->item.reset(new ColumnItem(*source.item.get()));
  }
}

row_item_t::~row_item_t()
{
}

row_item_t& row_item_t::operator=(const row_item_t& source)
{
  if (this != &source)
  {
    if (source.item)
    {
      item.reset(new ColumnItem(*source.item.get()));
    }
    type = source.type;
    size = source.size;
    tableName = source.tableName;
    columnName = source.columnName;
    aggFunc = source.aggFunc;
    computed = source.computed;
    grouped = source.grouped;
    displayed = source.displayed;
    null = source.null;
  }
  return *this;
}

//////////////////////////////////////////////////////////////////////////////////////

bool row_item_t::match(const std::string& _tableName, const std::string& _columnName)
{
  return (this->tableName == _tableName) && (this->columnName == _columnName);
}

Row::Row() 
  : 
  count(0), 
  completed(true), 
  reinit(false),
  flush(false) 
{
}

Row::Row(const Row& source)
  :
  count(source.count),
  completed(source.completed),
  reinit(source.reinit),
  flush(source.flush)
{
  std::copy(source.indexes.begin(), source.indexes.end(), std::back_inserter<aq::Row::index_t>(this->indexes));
  std::copy(source.initialRow.begin(), source.initialRow.end(), std::back_inserter<aq::Row::row_t>(this->initialRow));
  std::copy(source.computedRow.begin(), source.computedRow.end(), std::back_inserter<aq::Row::row_t>(this->computedRow));
}

Row::~Row()
{
}

Row& Row::operator=(const Row& source)
{
  if (this != &source)
  {
    this->count = source.count;
    this->completed = source.completed;
    this->reinit = source.reinit;
    this->flush = source.flush;
    this->computedRow.clear();
    this->initialRow.clear();
    std::copy(source.indexes.begin(), source.indexes.end(), std::back_inserter<aq::Row::index_t>(this->indexes));
    std::copy(source.initialRow.begin(), source.initialRow.end(), std::back_inserter<aq::Row::row_t>(this->initialRow));
    std::copy(source.computedRow.begin(), source.computedRow.end(), std::back_inserter<aq::Row::row_t>(this->computedRow));
  }
  return *this;
}

}