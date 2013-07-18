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
  virtual ~ColumnMapper_Intf() {}
	virtual int loadValue(size_t index, ColumnItem& value) = 0;
  virtual const aq::ColumnType getType() const = 0;
};

// helper function
template <typename T> struct type_conversion { static const aq::ColumnType type = aq::ColumnType::COL_TYPE_INT; };
template <> struct type_conversion<int32_t> { static const aq::ColumnType type = aq::ColumnType::COL_TYPE_INT; };
template <> struct type_conversion<int64_t> { static const aq::ColumnType type = aq::ColumnType::COL_TYPE_BIG_INT; };
template <> struct type_conversion<double> { static const aq::ColumnType type = aq::ColumnType::COL_TYPE_DOUBLE; };
template <> struct type_conversion<char> { static const aq::ColumnType type = aq::ColumnType::COL_TYPE_VARCHAR; };

template <typename T>
void fill_item(ColumnItem& item, T * value, size_t size)
{
  item.numval = static_cast<double>(*value);
}

template <>
inline void fill_item<char>(ColumnItem& item, char * value, size_t size)
{
  item.strval = std::string(value, size);
}

}

#endif