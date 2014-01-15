#ifndef __ROW_TABLE_WRITTER_H__
#define __ROW_TABLE_WRITTER_H__

#include "RowWritter_Intf.h"
#include <aq/Table.h>

namespace aq
{
  
  /// \ingroup row_writter
  /// \brief write into a Table
  /// used by nested queries
  class RowTableWritter : public aq::RowWritter_Intf
  {
  public:
    RowTableWritter(Table::Ptr table);
    RowTableWritter(const RowTableWritter& o);
    ~RowTableWritter();
    const std::vector<Column::Ptr>& getColumns() const;
    void setColumn(std::vector<Column::Ptr> _columns);
    unsigned int getTotalCount() const;
    int process(std::vector<Row>& rows);

    RowProcess_Intf * clone()
    {
      return new RowTableWritter(*this);
    }

  protected:
    int process(Row& rows);
  
  private:
    Table::Ptr table;
  };

}

#endif