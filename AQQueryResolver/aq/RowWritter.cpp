#include "RowWritter.h"
#include <iostream>

using namespace aq;

RowWritter::RowWritter(const std::string& _filePath)
	: totalCount(0), filePath(_filePath), firstRow(true)
{
	value = static_cast<char*>(malloc(128 * sizeof(char)));
	if (filePath == "stdout")
		pFOut = stdout;
	else
		pFOut = fopen( filePath.c_str(), "wt" );
}

RowWritter::RowWritter(const RowWritter& o)
	: 
  totalCount(o.totalCount), 
  pFOut(o.pFOut),
  filePath(o.filePath),
  firstRow(o.firstRow)
{
	value = static_cast<char*>(malloc(128 * sizeof(char)));
}

RowWritter::~RowWritter()
{
	free(value);
  if (pFOut && (pFOut != stdout))
    fclose(pFOut);
}

int RowWritter::process(std::vector<Row>& rows)
{
  for (auto& row : rows) 
  { 
    this->process(row); 
  }
  return 0;
}

void RowWritter::write_value(const aq::row_item_t& row_item) const
{
  if (row_item.displayed)
  {
    switch(row_item.type)
    {
    case aq::ColumnType::COL_TYPE_INT:
      boost::get<aq::ColumnItem<int32_t> >(row_item.item).toString(value);
      break;
    case aq::ColumnType::COL_TYPE_DOUBLE:
      boost::get<aq::ColumnItem<double> >(row_item.item).toString(value);
      break;
    case aq::ColumnType::COL_TYPE_DATE:
    case aq::ColumnType::COL_TYPE_BIG_INT:
      boost::get<aq::ColumnItem<int64_t> >(row_item.item).toString(value);
      break;
    case aq::ColumnType::COL_TYPE_VARCHAR:
      boost::get<aq::ColumnItem<char*> >(row_item.item).toString(value);
      break;
    }
  }
}

int RowWritter::process(Row& row)
{
	if (this->firstRow)
	{
		//write column names
    bool first = true;
    for (Row::row_t::const_reverse_iterator it = row.computedRow.rbegin(); it != row.computedRow.rend(); ++it)
    {
      auto& row_item = *it;
      if (row_item.displayed)
      {
        size_t size = row_item.tableName != "" ? row_item.tableName.size() + 1 : 0;
        size += row_item.columnName.size();
        if (!first) 
          fputs(" | ", pFOut);
        for (size_t i = size; i < 8; i++)
          fputs(" ", pFOut);
        if (row_item.tableName != "")
        {
          fputs(row_item.tableName.c_str(), pFOut);
          fputs(".", pFOut);
        }
        fputs(row_item.columnName.c_str(), pFOut);
        first = false;
        widths.push_back(std::max(size, (size_t)8));
      }
		}
		fputc('\n', pFOut);
		for (size_t w = 0; w < this->widths.size(); w++)
    {
      for (size_t i = 0; i < (this->widths[w] + 1); i++)
        fputs("-", pFOut);
      if ((w + 1) < this->widths.size())
      {
        fputs("+", pFOut);
        fputs("-", pFOut);
      }
    }
		fputc('\n', pFOut);
    this->firstRow = false;
	}

  // write values
  if (row.completed)
  {
    this->totalCount += 1;
    bool first = true;
    size_t k = 0;
    for (Row::row_t::const_reverse_iterator it = row.computedRow.rbegin(); it != row.computedRow.rend(); ++it)
    {
      if (!first) 
        fputs(" | ", pFOut);
      if ((*it).null)
      {
        for (size_t i = 4; i < this->widths[k]; i++)
          fputs(" ", pFOut);
        fputs("NULL", pFOut);
      }
      else
      {
        write_value(*it);
        for (size_t i = strlen(value); i < this->widths[k]; i++)
          fputs(" ", pFOut);
        fputs(value, pFOut);
      }
      k += 1;
      first = false;
    }
    //fputs(" ; ", pFOut);
    //sprintf(value, "%u", row.count);
    //fputs(value, pFOut);
    fputc('\n', pFOut);
    fflush(pFOut);
  }
	return 0;
}
