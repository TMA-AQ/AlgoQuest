#include "RowTableWritter.h"

namespace aq
{

  RowTableWritter::RowTableWritter(Table::Ptr _table)
    : table(_table)
  {
  }
  
  RowTableWritter::RowTableWritter(const RowTableWritter& o)
    : table(o.table)
  {
  }

  RowTableWritter::~RowTableWritter()
  {
  }

  const std::vector<Column::Ptr>& RowTableWritter::getColumns() const
  {
    return this->table->Columns;
  }

  void RowTableWritter::setColumn(std::vector<Column::Ptr>)
  {
    assert(false);
  }
  
  int RowTableWritter::process(std::vector<Row>& rows)
  {
    std::for_each(rows.begin(), rows.end(), [&] (Row& row) { this->process(row); });
    return 0;
  }
  
  int RowTableWritter::process(Row& rows)
  {
    if (this->table->Columns.empty())
    {
      for (auto it = rows.computedRow.begin(); it != rows.computedRow.end(); ++it)
      {
        Column::Ptr column(new Column);
        column->setType((*it).type);
        // TODO
        this->table->Columns.push_back(column);
      }
    }
    if (rows.completed)
    {
      auto itColumn  = this->table->Columns.begin();
      for (auto it = rows.computedRow.begin(); it != rows.computedRow.end(); ++it)
      {
        assert(itColumn != this->table->Columns.end());

        // TODO
        //for (size_t i = 0; i < rows.count; ++i)
        //  (*itColumn)->Items.push_back(new ColumnItem(*(*it).item));

        ++itColumn;
      }
    }
    return 0;
  }

  unsigned int RowTableWritter::getTotalCount() const
  {
    return static_cast<unsigned int>(this->table->getTotalCount());
  }


}
