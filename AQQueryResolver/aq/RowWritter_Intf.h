#ifndef __ROW_WRITTER_INTF_H__
#define __ROW_WRITTER_INTF_H__

#include "RowProcess_Intf.h"
#include <aq/Column.h>

namespace aq
{

  /// \defgroup row_writter Row Writter Processing
  
  /// \ingroup row_writter
  /// \brief Specialisation or row process to write columns
  /// This row process type should be used at the end of the chain of row process
  class RowWritter_Intf : public RowProcess_Intf
  {
  public:
    virtual ~RowWritter_Intf() {}
  
    /// \brief Retrieve columns to be write
    /// \return a std::vector of columns
    virtual const std::vector<Column::Ptr>& getColumns() const = 0;

    /// \brief set the columns to write
    /// \param _columns the columns to write
    virtual void setColumn(std::vector<Column::Ptr> _columns) = 0;

    /// \brief get the number of row write
    /// \return the number of row write
    virtual unsigned int getTotalCount() const = 0;
  };
}

#endif