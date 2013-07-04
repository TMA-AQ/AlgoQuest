#ifndef __ROW_TEMPORAY_WRITTER_H__
#define __ROW_TEMPORAY_WRITTER_H__

#include "RowWritter_Intf.h"
#include "ColumnTemporaryWritter.h"
#include <cstdio>
#include <string>
#include <vector>
#include <boost/shared_ptr.hpp>

namespace aq
{

class RowTemporaryWritter : public RowWritter_Intf
{
public:
  RowTemporaryWritter(unsigned int _tableId, const char * _path, unsigned int _packetSize);
  RowTemporaryWritter(const RowTemporaryWritter& o);
  ~RowTemporaryWritter();
  
  const std::vector<Column::Ptr>& getColumns() const { return this->columns; }
	void setColumn(std::vector<Column::Ptr> _columns) { this->columns = _columns; }
  unsigned int getTotalCount() const { return this->totalCount; }
	int process(std::vector<Row>& rows);
  
  RowProcess_Intf * clone()
  {
    return new RowTemporaryWritter(*this);
  }

protected:
  virtual int process(Row& row);

private:
  std::string path;
  unsigned int tableId;
  unsigned int packetSize;
  unsigned int totalCount;
	std::vector<Column::Ptr> columns;
  std::vector<boost::shared_ptr<ColumnTemporaryWritter> > columnsWritter;
};

}

#endif