#ifndef __ROW_PROCESS_INTF_H__
#define __ROW_PROCESS_INTF_H__

#include "Row.h"
#include <aq/DBTypes.h>
#include <list>
#include <vector>
#include <boost/shared_ptr.hpp>

namespace aq
{

  class RowProcess_Intf
  {
  public:
  public:
    virtual ~RowProcess_Intf() {}
    virtual int process(Row& row) = 0;
    int flush()
    {
      aq::Row row;
      row.flush = true;
      this->process(row);
      return 0;
    }

  };

}

#endif