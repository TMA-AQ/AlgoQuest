#ifndef __ROW_PROCESS_INTF_H__
#define __ROW_PROCESS_INTF_H__

#include "ColumnItem.h"
#include <list>
#include <vector>
#include <boost/shared_ptr.hpp>

namespace aq
{

  class RowProcess_Intf
  {
  public:
    struct row_item_t
    {
      ColumnItem::Ptr item;
      aq::ColumnType type;
      std::string tableName;
      std::string columnName;
      row_item_t(ColumnItem::Ptr _item,
        aq::ColumnType _type,
        std::string _tableName,
        std::string _columnName)
        : item(_item),
        type(_type),
        tableName(_tableName),
        columnName(_columnName)
      {
      }
      bool match(const std::string& _tableName, const std::string& _columnName)
      {
        return (this->tableName == _tableName) && (this->columnName == _columnName);
      }
    };

    typedef std::vector<row_item_t> row_t;

  public:
    virtual ~RowProcess_Intf() {}
    virtual int process(row_t& row) = 0;
  };

}

#endif