#include "Row.h"

namespace aq
{

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
                       displayed(false)
{
}

row_item_t::row_item_t(const row_item_t& source)
  : item(source.item),
  type(source.type),
  size(source.size),
  tableName(source.tableName),
  columnName(source.columnName),
  computed(source.computed),
  grouped(source.grouped),
  displayed(source.displayed)
{
}

row_item_t::~row_item_t()
{
}

row_item_t& row_item_t::operator=(const row_item_t& source)
{
  if (this != &source)
  {
    item = source.item;
    type = source.type;
    size = source.size;
    tableName = source.tableName;
    columnName = source.columnName;
    computed = source.computed;
    grouped = source.grouped;
    displayed = source.displayed;
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
  flush(false) 
{
}

Row::Row(const Row& source)
  :
  count(source.count),
  completed(source.completed),
  flush(source.flush)
{
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
    this->flush = source.flush;
    this->computedRow.clear();
    this->initialRow.clear();
    std::copy(source.initialRow.begin(), source.initialRow.end(), std::back_inserter<aq::Row::row_t>(this->initialRow));
    std::copy(source.computedRow.begin(), source.computedRow.end(), std::back_inserter<aq::Row::row_t>(this->computedRow));
  }
  return *this;
}

}