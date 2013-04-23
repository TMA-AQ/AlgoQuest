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

int RowWritter::process(row_t& row)
{
	if (this->firstRow)
	{
		//write column names
		for(size_t idx = 0; idx < row.size(); ++idx)
    {
      fputs(" ; ", pFOut);
      if (row[idx].tableName != "")
      {
        fputs(row[idx].tableName.c_str(), pFOut);
        fputs(".", pFOut);
      }
      fputs(row[idx].columnName.c_str(), pFOut);

			//if (idx < this->columns.size())
			//{
			//	if(this->columns[idx]->Invisible)
			//		continue;
			//	fputs(" ; ", pFOut);
			//	assert(this->columns[idx]);
			//	fputs(this->columns[idx]->getDisplayName().c_str(), pFOut);
			//}
			//else
			//{
			//	fputs(" ; Count", pFOut);
			//}
		}
		fputc('\n', pFOut);
		this->firstRow = false;
	}

	for (row_t::const_iterator it = row.begin(); it != row.end(); ++it)
	{
		(*it).item->toString(value, (*it).type);
		fputs(" ; ", pFOut);
		fputs(value, pFOut);
	}
	fputc('\n', pFOut);
	return 0;
}
