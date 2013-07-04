#ifndef __ROW_WRITTER_INTF_H__
#define __ROW_WRITTER_INTF_H__

#include "RowProcess_Intf.h"
#include "Column.h"

namespace aq
{
  class RowWritter_Intf : public RowProcess_Intf
  {
  public:
    virtual ~RowWritter_Intf() {}
    virtual const std::vector<Column::Ptr>& getColumns() const = 0;
    virtual void setColumn(std::vector<Column::Ptr> _columns) = 0;
    virtual unsigned int getTotalCount() const = 0;
  };
}

#endif