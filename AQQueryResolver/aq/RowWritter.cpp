#include "RowWritter.h"
#include <iostream>

using namespace aq;

RowWritter::RowWritter(const std::string& filePath)
	: totalCount(0), firstRow(true)
{
	value = static_cast<char*>(malloc(128 * sizeof(char)));
	if (filePath == "stdout")
		pFOut = stdout;
	else
		pFOut = fopen( filePath.c_str(), "wt" );
}

RowWritter::~RowWritter()
{
	free(value);
  if (pFOut)
    fclose(pFOut);
}

int RowWritter::process(std::vector<Row>& rows)
{
  std::for_each(rows.begin(), rows.end(), [&] (Row& row) { this->process(row); });
  return 0;
}

int RowWritter::process(Row& row)
{
	if (this->firstRow)
	{
		//write column names
    for (Row::row_t::const_reverse_iterator it = row.computedRow.rbegin(); it != row.computedRow.rend(); ++it)
    {
      if (!(*it).displayed)
        continue;
      fputs(" ; ", pFOut);
      if ((*it).tableName != "")
      {
        fputs((*it).tableName.c_str(), pFOut);
        fputs(".", pFOut);
      }
      fputs((*it).columnName.c_str(), pFOut);
		}
		fputc('\n', pFOut);
		this->firstRow = false;
	}

  if (row.completed)
  {
    this->totalCount += 1;
    for (Row::row_t::const_reverse_iterator it = row.computedRow.rbegin(); it != row.computedRow.rend(); ++it)
    {
      if (!(*it).displayed)
        continue;
      assert((*it).item != NULL);
      (*it).item->toString(value, (*it).type);
      fputs(" ; ", pFOut);
      fputs(value, pFOut);
    }
    fputs(" => ", pFOut);
    sprintf(value, "%u", row.count);
    fputs(value, pFOut);
    fputc('\n', pFOut);
  }
	return 0;
}
