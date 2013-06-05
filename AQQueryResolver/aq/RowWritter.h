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
	virtual int process(std::vector<Row>& rows);
  unsigned int getTotalCount() const { return this->totalCount; }

protected:
  virtual int process(Row& row);

  aq::verb::VerbNode::Ptr spTree;
	std::vector<Column::Ptr> columns;
  unsigned int totalCount;
	FILE * pFOut;
	bool firstRow;

private:
	char * value;
};

}

#endif
