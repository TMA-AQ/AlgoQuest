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
    row_item_t();
    row_item_t(ColumnItem::Ptr _item,
      aq::ColumnType _type,
      unsigned int _size,
      std::string _tableName,
      std::string _columnName,
      bool _computed = false);
    row_item_t(const row_item_t& source);
    ~row_item_t();
    row_item_t& operator=(const row_item_t& source);
    bool match(const std::string& _tableName, const std::string& _columnName);
  };
   
  //////////////////////////////////////////////////////////////////////////////////////

  class Row
  {
  public:

    typedef std::vector<row_item_t> row_t;

    Row();
    Row(const Row& source);
    ~Row();
    Row& operator=(const Row& source);

    row_t initialRow;
    row_t computedRow;
    unsigned int count;
    bool completed;
    bool flush;
  };

}

#endif