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
      if (!first) 
        fputs(" ; ", pFOut);
      if ((*it).tableName != "")
      {
        fputs((*it).tableName.c_str(), pFOut);
        fputs(".", pFOut);
      }
      fputs((*it).columnName.c_str(), pFOut);
      first = false;
		}
		fputc('\n', pFOut);
		this->firstRow = false;
	}

  if (row.completed)
  {
    this->totalCount += 1;
    bool first = true;
    for (Row::row_t::const_reverse_iterator it = row.computedRow.rbegin(); it != row.computedRow.rend(); ++it)
    {
      if (!(*it).displayed)
        continue;
      assert((*it).item != NULL);
      if (!first) 
        fputs(" ; ", pFOut);
      if ((*it).null)
      {
        fputs("NULL", pFOut);
      }
      else
      {
        (*it).item->toString(value, (*it).type);
        fputs(value, pFOut);
      }
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
