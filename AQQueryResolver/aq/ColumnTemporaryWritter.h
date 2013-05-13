#ifndef __COLUMN_TEMPORARY_WRITTER_H__
#define __COLUMN_TEMPORARY_WRITTER_H__

#include "Column.h"
#include <cstdio>
#include <string>

namespace aq
{

class ColumnTemporaryWritter
{

public:

  Column::Ptr column;
  std::string filename;
  FILE * file;

};

}

#endif