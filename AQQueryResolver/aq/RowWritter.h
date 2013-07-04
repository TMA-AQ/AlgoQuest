#ifndef __ROW_WRITTER_H__
#define __ROW_WRITTER_H__

#include "Table.h"
#include "verbs/VerbNode.h"
#include "RowWritter_Intf.h"
#include <vector>

namespace aq
{

class RowWritter : public RowWritter_Intf
{
public:
	RowWritter(const std::string& filePath);
  RowWritter(const RowWritter& o);
	~RowWritter();

  const std::vector<Column::Ptr>& getColumns() const { return this->columns; }
	void setColumn(std::vector<Column::Ptr> _columns) { this->columns = _columns; }
	int process(std::vector<Row>& rows);
  unsigned int getTotalCount() const { return this->totalCount; }
  
  RowProcess_Intf * clone()
  {
    return new RowWritter(*this);
  }

protected:
  virtual int process(Row& row);

  aq::verb::VerbNode::Ptr spTree;
	std::vector<Column::Ptr> columns;
  unsigned int totalCount;
	FILE * pFOut;
  const std::string& filePath;
	bool firstRow;

private:
	char * value;
};

}

#endif
