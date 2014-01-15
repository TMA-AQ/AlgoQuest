#ifndef __COLUMN_TEMPORARY_WRITTER_H__
#define __COLUMN_TEMPORARY_WRITTER_H__

#include <aq/Column.h>

#include <cstdio>
#include <string>

namespace aq
{
  
/// \ingroup row_writter
/// \deprecated
class ColumnTemporaryWritter
{

public:

  ColumnTemporaryWritter()
    : nbEl(0)
  {
  }

  Column::Ptr column;
  std::string filename;
  FILE * file;
  size_t nbEl;

};

}

#endif