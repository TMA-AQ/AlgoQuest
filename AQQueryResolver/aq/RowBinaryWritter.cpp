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

int RowBinaryWritter::process(Row& row)
{
  if (row.completed)
  {
    this->totalCount += 1;
    for (Row::row_t::const_iterator it = row.computedRow.begin(); it != row.computedRow.end(); ++it)
    {
      if (!(*it).displayed)
        continue;
      assert((*it).item != NULL);
      switch((*it).type)
      {
      case aq::ColumnType::COL_TYPE_VARCHAR:
        fwrite((*it).item->strval.c_str(), sizeof(char), (*it).item->strval.size(), pFOut);
        break;
      case aq::ColumnType::COL_TYPE_INT:
        uint32_value = static_cast<uint32_t>((*it).item->numval);
        fwrite(&uint32_value, sizeof(uint32_t), 1, pFOut);
        break;
      case aq::ColumnType::COL_TYPE_DOUBLE:
        fwrite(&(*it).item->numval, sizeof(double), 1, pFOut);
        break;
      case aq::ColumnType::COL_TYPE_BIG_INT:
      case aq::ColumnType::COL_TYPE_DATE:
        uint64_value = static_cast<uint64_t>((*it).item->numval);
        fwrite(&uint64_value, sizeof(uint64_t), 1, pFOut);
        break;
      }
    }
  }
  return 0;
}

}