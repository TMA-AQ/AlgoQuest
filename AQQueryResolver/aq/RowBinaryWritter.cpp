#include "RowBinaryWritter.h"

namespace aq
{
  
RowBinaryWritter::RowBinaryWritter(const std::string& filePath)
  : RowWritter(filePath), uint32_value(0), uint64_value(0)
{
  pFOut = fopen(filePath.c_str(), "wb");
}

RowBinaryWritter::~RowBinaryWritter()
{
  if (pFOut)
    fclose(pFOut);
}

template <typename T>
int write(const aq::row_item_t& row_item, FILE * fd)
{
  int rc = 0;
  if (row_item.displayed)
  {
    T value = boost::get<T>(row_item.item).getValue();
    rc = (int)fwrite(&value, sizeof(T), row_item.size, fd);
  }
  return rc;
}

int RowBinaryWritter::process(Row& row)
{
  if (row.completed)
  {
    this->totalCount += 1;
    for (Row::row_t::const_iterator it = row.computedRow.begin(); it != row.computedRow.end(); ++it)
    {
      auto& row_item = *it;
      switch (row_item.type)
      {
      case aq::ColumnType::COL_TYPE_VARCHAR:
        {
          write<aq::ColumnItem<int32_t> >(row_item, pFOut);
        }
        break;
      case aq::ColumnType::COL_TYPE_INT:
        {
          write<aq::ColumnItem<int64_t> >(row_item, pFOut);
        }
        break;
      case aq::ColumnType::COL_TYPE_DOUBLE:
        {
          write<aq::ColumnItem<double> >(row_item, pFOut);
        }
        break;
      case aq::ColumnType::COL_TYPE_BIG_INT:
      case aq::ColumnType::COL_TYPE_DATE:
        {
          write<aq::ColumnItem<int64_t> >(row_item, pFOut);
        }
        break;
      }
    }
  }
  return 0;
}

}