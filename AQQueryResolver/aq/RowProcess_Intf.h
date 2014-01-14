#ifndef __ROW_PROCESS_INTF_H__
#define __ROW_PROCESS_INTF_H__

#include "Row.h"
#include <aq/DBTypes.h>
#include <list>
#include <vector>
#include <boost/shared_ptr.hpp>

namespace aq
{

  /// \brief Interface defining a row processing
  class RowProcess_Intf
  {
  public:
  public:
    virtual ~RowProcess_Intf() {}

    /// \brief process a row
    /// \param rows a vector of rows to proceed
    /// \return \todo
    virtual int process(std::vector<Row>& rows) = 0;

    /// \brief clone this process
    /// \return this process clone
    virtual RowProcess_Intf * clone() = 0;

    int flush(std::vector<Row>& rows)
    {
      for (auto& r : rows)
        r.flush = true;
      this->process(rows);
      return 0; // \fixme shouldn't return value of process ???
    }
  };

}

#endif