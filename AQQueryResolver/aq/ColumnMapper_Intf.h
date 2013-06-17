#ifndef __COLUMN_MAPPER_INTF_H__
#define __COLUMN_MAPPER_INTF_H__

#include "ColumnItem.h"
#include <boost/shared_ptr.hpp>

namespace aq
{

class ColumnMapper_Intf
{
public:
	typedef boost::shared_ptr<ColumnMapper_Intf> Ptr;
	virtual int loadValue(size_t index, ColumnItem& value) = 0;
  virtual const aq::ColumnType getType() const = 0;
};

}

#endif