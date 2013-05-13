#ifndef __ROW_TEMPORAY_WRITTER_H__
#define __ROW_TEMPORAY_WRITTER_H__

#include "RowWritter.h"
#include "ColumnTemporaryWritter.h"
#include <cstdio>
#include <string>
#include <vector>
#include <boost/shared_ptr.hpp>

namespace aq
{

class RowTemporaryWritter : public aq::RowWritter
{
public:
  RowTemporaryWritter(unsigned int _tableId, const char * _path);
  ~RowTemporaryWritter();
  virtual int process(Row& row);
  const std::vector<Column::Ptr>& getColumns() const { return this->columns; }
private:
  std::string path;
  unsigned int tableId;
  std::vector<boost::shared_ptr<ColumnTemporaryWritter> > columnsWritter;
  std::vector<Column::Ptr> columns;
};

}

#endif