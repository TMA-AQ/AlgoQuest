#ifndef __ROW_PROCESSING_H__
#define __ROW_PROCESSING_H__

#include "Table.h"
#include <vector>

namespace aq
{

class RowProcessing
{
public:
	typedef std::vector<std::pair<ColumnItem::Ptr, ColumnType> > row_t;

	RowProcessing(const std::string& filePath);
	~RowProcessing();

	void setColumn(std::vector<Column::Ptr> _columns) { this->columns = _columns; }
	virtual int process(row_t row);

private:
	std::vector<Column::Ptr> columns;
	char * value;
	FILE * pFOut;
	bool firstRow;

};

}

#endif
