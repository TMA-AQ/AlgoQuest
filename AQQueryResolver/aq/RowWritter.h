#ifndef __ROW_WRITTER_H__
#define __ROW_WRITTE_H__

#include "Table.h"
#include "verbs/VerbNode.h"
#include "RowProcess_Intf.h"
#include <vector>

namespace aq
{

class RowWritter : public aq::RowProcess_Intf
{
public:
	RowWritter(const std::string& filePath);
	~RowWritter();

	void setColumn(std::vector<Column::Ptr> _columns) { this->columns = _columns; }
	virtual int process(Row& row);

private:
  VerbNode::Ptr spTree;
	std::vector<Column::Ptr> columns;
	char * value;
	FILE * pFOut;
	bool firstRow;

};

}

#endif
