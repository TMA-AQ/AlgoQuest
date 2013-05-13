#ifndef __ROW_PROCESS_INTF_H__
#define __ROW_PROCESS_INTF_H__

#include "ColumnItem.h"
#include <aq/DBTypes.h>
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
      aq::aggregate_function_t aggFunc;
      bool computed;
      bool grouped;
      bool displayed;
      row_item_t(ColumnItem::Ptr _item,
        aq::ColumnType _type,
        std::string _tableName,
        std::string _columnName,
        bool _computed = false)
        : item(_item),
        type(_type),
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

    typedef std::vector<row_item_t> row_t;

    struct Row
    {
      Row() : completed(true), flush(false) {}
      row_t row;
      bool completed;
      bool flush;
    };

  public:
    virtual ~RowProcess_Intf() {}
    virtual int process(Row& row) = 0;
    int flush()
    {
      aq::RowProcess_Intf::Row row;
      row.flush = true;
      this->process(row);
      return 0;
    }

  };

}

#endif