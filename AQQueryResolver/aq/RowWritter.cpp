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

int RowWritter::process(Row& row)
{
  assert(row.row.size() > 0);

	if (this->firstRow)
	{
		//write column names
		for(size_t idx = row.row.size() - 1; idx > 0; --idx)
    {
      if (!row.row[idx].displayed)
        continue;
      fputs(" ; ", pFOut);
      if (row.row[idx].tableName != "")
      {
        fputs(row.row[idx].tableName.c_str(), pFOut);
        fputs(".", pFOut);
      }
      fputs(row.row[idx].columnName.c_str(), pFOut);
		}
		fputc('\n', pFOut);
		this->firstRow = false;
	}

  if (row.completed)
  {
    this->totalCount += 1;
    for (row_t::const_reverse_iterator it = row.row.rbegin(); it != row.row.rend(); ++it)
    {
      if (!(*it).displayed)
        continue;
      assert((*it).item != NULL);
      (*it).item->toString(value, (*it).type);
      fputs(" ; ", pFOut);
      fputs(value, pFOut);
    }
    fputc('\n', pFOut);
  }
	return 0;
}
