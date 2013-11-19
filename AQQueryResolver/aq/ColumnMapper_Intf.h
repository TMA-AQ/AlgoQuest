#ifndef __COLUMN_MAPPER_INTF_H__
#define __COLUMN_MAPPER_INTF_H__

#include "ColumnItem.h"
#include <cstring>
#include <boost/shared_ptr.hpp>

namespace aq
{

template <typename T>
class ColumnMapper_Intf
{
public:
	typedef boost::shared_ptr<ColumnMapper_Intf> Ptr;
  virtual ~ColumnMapper_Intf() {}
	virtual int loadValue(size_t index, T * value) = 0;
  virtual int setValue(size_t index, T * value) = 0;
  virtual int append(T * value) = 0;
  const aq::ColumnType getType() const { return aq::type_conversion<T>::type; } ;
};

// helper function
template <typename T> struct type_conversion { static const aq::ColumnType type = aq::ColumnType::COL_TYPE_INT; };
template <> struct type_conversion<int32_t> { static const aq::ColumnType type = aq::ColumnType::COL_TYPE_INT; };
template <> struct type_conversion<int64_t> { static const aq::ColumnType type = aq::ColumnType::COL_TYPE_BIG_INT; };
template <> struct type_conversion<double> { static const aq::ColumnType type = aq::ColumnType::COL_TYPE_DOUBLE; };
template <> struct type_conversion<char> { static const aq::ColumnType type = aq::ColumnType::COL_TYPE_VARCHAR; };

//template <typename T> struct enum_to_type { typedef int32_t type; };
//template <> struct enum_to_type<aq::ColumnType = aq::ColumnType::COL_TYPE_INT> { static const aq::ColumnType type = aq::ColumnType::COL_TYPE_INT; };
//template <> struct enum_to_type<aq::ColumnType::COL_TYPE_BIG_INT> { static const aq::ColumnType type = aq::ColumnType::COL_TYPE_BIG_INT; };
//template <> struct enum_to_type<aq::ColumnType::COL_TYPE_DOUBLE> { static const aq::ColumnType type = aq::ColumnType::COL_TYPE_DOUBLE; };
//template <> struct enum_to_type<aq::ColumnType::COL_TYPE_VARCHAR> { static const aq::ColumnType type = aq::ColumnType::COL_TYPE_VARCHAR; };

//template <typename T>
//void fill_item(ColumnItem<T>& item, T * value, size_t size)
//{
//  assert(size == 1);
//  item.value = *value;
//}
//
//template <> inline
//void fill_item<char*>(ColumnItem<char*>& item, char * value, size_t size)
//{
//  item.setValue(value); // FIXME
//}
//
//template <typename T>
//void dump_item(T * value, size_t size, ColumnItem<T>& item)
//{
//  assert(size == 1);
//  *value = item.value;
//}
//
//template <> inline 
//void dump_item<char*>(char * value, size_t size, ColumnItem<char*>& item)
//{
//  ::memcpy(value, item.strval.c_str(), size);
//}

}

#endif
