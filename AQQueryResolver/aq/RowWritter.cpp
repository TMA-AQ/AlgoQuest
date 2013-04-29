#include "RowWritter.h"
#include <iostream>

using namespace aq;

RowWritter::RowWritter(const std::string& filePath)
	: firstRow(true)
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
	fclose(pFOut);
}

int RowWritter::process(Row& row)
{
	if (this->firstRow)
	{
		//write column names
		for(size_t idx = 0; idx < row.row.size(); ++idx)
    {
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
    for (row_t::const_iterator it = row.row.begin(); it != row.row.end(); ++it)
    {
      assert((*it).item != NULL);
      (*it).item->toString(value, (*it).type);
      fputs(" ; ", pFOut);
      fputs(value, pFOut);
    }
    fputc('\n', pFOut);
  }
	return 0;
}
