#ifndef __ROW_H__
#define __ROW_H__

#include "ColumnItem.h"
#include <vector>

namespace aq
{

  struct row_item_t
  {
    ColumnItem::Ptr item;
    aq::ColumnType type;
    unsigned int size;
    std::string tableName;
    std::string columnName;
    aq::aggregate_function_t aggFunc;
    bool computed;
    bool grouped;
    bool displayed;
    row_item_t(ColumnItem::Ptr _item,
      aq::ColumnType _type,
      unsigned int _size,
      std::string _tableName,
      std::string _columnName,
      bool _computed = false)
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
    bool match(const std::string& _tableName, const std::string& _columnName)
    {
      return (this->tableName == _tableName) && (this->columnName == _columnName);
    }
  };

  class Row
  {
  public:

    typedef std::vector<row_item_t> row_t;

    Row() : count(0), completed(true), flush(false) {}

    row_t initialRow;
    row_t computedRow;
    unsigned int count;
    bool completed;
    bool flush;
  };

}

#endif