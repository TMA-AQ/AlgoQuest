#include "RowProcessing.h"
#include <iostream>

using namespace aq;

RowProcessing::RowProcessing(const std::string& filePath)
	: firstRow(true)
{
	value = static_cast<char*>(malloc(128 * sizeof(char)));
	if (filePath == "stdout")
		pFOut = stdout;
	else
		pFOut = fopen( filePath.c_str(), "wt" );
}

RowProcessing::~RowProcessing()
{
	free(value);
	fclose(pFOut);
}

int RowProcessing::process(row_t row)
{
	if (this->firstRow)
	{
		//write column names
		for(size_t idx = 0; idx < row.size(); ++idx)
		{
			if (idx < this->columns.size())
			{
				if(this->columns[idx]->Invisible)
					continue;
				fputs(" ; ", pFOut);
				assert(this->columns[idx]);
				fputs(this->columns[idx]->getDisplayName().c_str(), pFOut);
			}
			else
			{
				fputs(" ; Count", pFOut);
			}
		}
		fputc('\n', pFOut);
		this->firstRow = false;
	}

	for (row_t::const_iterator it = row.begin(); it != row.end(); ++it)
	{
		it->first->toString(value, it->second);
		fputs(" ; ", pFOut);
		fputs(value, pFOut);
	}
	fputc('\n', pFOut);
	return 0;
}
