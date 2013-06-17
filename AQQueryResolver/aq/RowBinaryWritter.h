#ifndef __ROW_BINARAY_WRITTER_H__
#define __ROW_BINARAY_WRITTER_H__

#include "RowWritter.h"
#include <cstdio>
#include <string>
#include <vector>
#include <boost/shared_ptr.hpp>

namespace aq
{

class RowBinaryWritter : public aq::RowWritter
{
public:
  RowBinaryWritter(const std::string& filepath);
  ~RowBinaryWritter();
protected:
  int process(Row& row);
private:
  uint32_t uint32_value;
  uint64_t uint64_value;
};

}

#endif