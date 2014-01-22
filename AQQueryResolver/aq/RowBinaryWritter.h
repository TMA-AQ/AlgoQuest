#ifndef __ROW_BINARAY_WRITTER_H__
#define __ROW_BINARAY_WRITTER_H__

#include "RowWritter.h"
#include <cstdio>
#include <string>
#include <vector>
#include <boost/shared_ptr.hpp>

namespace aq
{
  
/// \ingroup row_writter
/// \deprecated
class RowBinaryWritter : public aq::RowWritter
{
public:
  RowBinaryWritter(const std::string& filepath);
  ~RowBinaryWritter();
protected:
  int process(Row& row);
};

}

#endif