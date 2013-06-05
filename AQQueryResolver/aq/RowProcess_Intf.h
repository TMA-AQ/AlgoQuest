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
    virtual int process(std::vector<Row>& rows) = 0;
    int flush(std::vector<Row>& rows)
    {
      rows[0].flush = true;
      this->process(rows);
      return 0;
    }

  };

}

#endif