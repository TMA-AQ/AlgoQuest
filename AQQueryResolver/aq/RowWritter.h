#ifndef __ROW_WRITTER_H__
#define __ROW_WRITTER_H__

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

  virtual const std::vector<Column::Ptr>& getColumns() const { return this->columns; }
	void setColumn(std::vector<Column::Ptr> _columns) { this->columns = _columns; }
	virtual int process(Row& row);
  unsigned int getTotalCount() const { return this->totalCount; }

protected:
  VerbNode::Ptr spTree;
	std::vector<Column::Ptr> columns;
  unsigned int totalCount;
	char * value;
	FILE * pFOut;
	bool firstRow;

};

}

#endif
