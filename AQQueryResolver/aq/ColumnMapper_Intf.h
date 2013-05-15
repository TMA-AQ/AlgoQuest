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
	virtual ColumnItem::Ptr loadValue(size_t index) = 0;
};

}

#endif