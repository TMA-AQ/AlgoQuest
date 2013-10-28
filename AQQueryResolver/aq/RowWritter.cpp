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

int RowWritter::process(Row& row)
{
	if (this->firstRow)
	{
		//write column names
    bool first = true;
    for (Row::row_t::const_reverse_iterator it = row.computedRow.rbegin(); it != row.computedRow.rend(); ++it)
    {
      if (!(*it).displayed)
        continue;
      size_t size = (*it).tableName != "" ? (*it).tableName.size() + 1 : 0;
      size += (*it).columnName.size();
      if (!first) 
        fputs(" | ", pFOut);
      for (size_t i = size; i < 8; i++)
        fputs(" ", pFOut);
      if ((*it).tableName != "")
      {
        fputs((*it).tableName.c_str(), pFOut);
        fputs(".", pFOut);
      }
      fputs((*it).columnName.c_str(), pFOut);
      first = false;
      widths.push_back(std::max(size, (size_t)8));
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

  if (row.completed)
  {
    this->totalCount += 1;
    bool first = true;
    size_t k = 0;
    for (Row::row_t::const_reverse_iterator it = row.computedRow.rbegin(); it != row.computedRow.rend(); ++it)
    {
      if (!(*it).displayed)
        continue;
      assert((*it).item != NULL);
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
        (*it).item->toString(value, (*it).type);
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
