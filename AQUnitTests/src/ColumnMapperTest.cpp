#include <boost/test/unit_test.hpp>

#include <iostream>
#include <set>
#include <aq/GenericFileMapper.h>

#ifdef WIN32
#include <aq/WIN32FileMapper.h>
#endif

#include <aq/ColumnMapper.h>
#include <aq/BaseDesc.h>
#include <aq/Utilities.h>

template <typename T>
struct loader_t
{
  T value;
  aq::ColumnItem<T> item;
  char buffer[128];
  bool load(aq::ColumnMapper_Intf<T> * cm, size_t index)
  {
    int rc;
    if ((rc = cm->loadValue(index, &value)) == 0)
    {
      item.setValue(value);
      item.toString(buffer);
    }
    return rc == 0;
  }
  void dump(std::ostream& os) const
  {
    os << buffer;
  }
};

BOOST_AUTO_TEST_SUITE(ColumnMapper)

BOOST_AUTO_TEST_CASE(basic_int_column_mapper)
{
  size_t tableId = 1; 
  size_t columnId = 8; 
  size_t size = 1;
  std::string path = "E:/AQ_DATABASES/DB/MSALGOQUEST/data_orga/vdg/data/"; 
  aq::ColumnMapper_Intf<int32_t> * cm = new aq::ColumnMapper<int32_t, aq::GenericFileMapper>(path.c_str(), tableId, columnId, size, aq::packet_size);
  loader_t<int32_t> loader;
  loader.load(cm, 10);
}

BOOST_AUTO_TEST_CASE(basic_big_int_column_mapper)
{
  size_t tableId = 1; 
  size_t columnId = 11; 
  size_t size = 1;
  std::string path = "E:/AQ_DATABASES/DB/MSALGOQUEST/data_orga/vdg/data/"; 
  aq::ColumnMapper_Intf<int64_t> * cm = new aq::ColumnMapper<int64_t, aq::GenericFileMapper>(path.c_str(), tableId, columnId, size, aq::packet_size);
  loader_t<int64_t> loader;
  loader.load(cm, 10);
}

BOOST_AUTO_TEST_CASE(basic_double_column_mapper)
{
  size_t tableId = 1; 
  size_t columnId = 10; 
  size_t size = 1;
  std::string path = "E:/AQ_DATABASES/DB/MSALGOQUEST/data_orga/vdg/data/"; 
  aq::ColumnMapper_Intf<double> * cm = new aq::ColumnMapper<double, aq::GenericFileMapper>(path.c_str(), tableId, columnId, size, aq::packet_size);
  loader_t<double> loader;
  loader.load(cm, 10);
}

BOOST_AUTO_TEST_CASE(basic_varchar_column_mapper)
{
  size_t tableId = 1; 
  size_t columnId = 2; 
  size_t size = 11;
  std::string path = "E:/AQ_DATABASES/DB/MSALGOQUEST/data_orga/vdg/data/"; 
  aq::ColumnMapper_Intf<char> * cm = new aq::ColumnMapper<char, aq::GenericFileMapper>(path.c_str(), tableId, columnId, size, aq::packet_size);
  loader_t<char> loader;
  loader.load(cm, 10);
}

#ifdef WIN32

BOOST_AUTO_TEST_CASE(windows_int_column_mapper)
{
  size_t tableId = 1; 
  size_t columnId = 8; 
  size_t size = 1;
  std::string path = "E:/AQ_DATABASES/DB/MSALGOQUEST/data_orga/vdg/data/"; 
  aq::ColumnMapper_Intf<int32_t> * cm = new aq::ColumnMapper<int32_t, aq::WIN32FileMapper>(path.c_str(), tableId, columnId, size, aq::packet_size);
  loader_t<int32_t> loader;
  loader.load(cm, 10);
}

BOOST_AUTO_TEST_CASE(windows_big_int_column_mapper)
{
  size_t tableId = 1; 
  size_t columnId = 11; 
  size_t size = 1;
  std::string path = "E:/AQ_DATABASES/DB/MSALGOQUEST/data_orga/vdg/data/"; 
  aq::ColumnMapper_Intf<int64_t> * cm = new aq::ColumnMapper<int64_t, aq::WIN32FileMapper>(path.c_str(), tableId, columnId, size, aq::packet_size);
  loader_t<int64_t> loader;
  loader.load(cm, 10);
}

BOOST_AUTO_TEST_CASE(windows_double_column_mapper)
{
  size_t tableId = 1; 
  size_t columnId = 10; 
  size_t size = 1;
  std::string path = "E:/AQ_DATABASES/DB/MSALGOQUEST/data_orga/vdg/data/"; 
  aq::ColumnMapper_Intf<double> * cm = new aq::ColumnMapper<double, aq::WIN32FileMapper>(path.c_str(), tableId, columnId, size, aq::packet_size);
  loader_t<double> loader;
  loader.load(cm, 10);
}

BOOST_AUTO_TEST_CASE(windows_varchar_column_mapper)
{
  size_t tableId = 1; 
  size_t columnId = 2; 
  size_t size = 11;
  std::string path = "E:/AQ_DATABASES/DB/MSALGOQUEST/data_orga/vdg/data/"; 
  aq::ColumnMapper_Intf<char> * cm = new aq::ColumnMapper<char, aq::WIN32FileMapper>(path.c_str(), tableId, columnId, size, aq::packet_size);
  loader_t<char> loader;
  loader.load(cm, 10);
}

BOOST_AUTO_TEST_CASE(column_mapper)
{
  size_t tableId = 1; 
  size_t columnId = 1; 
  size_t size = 1;
  std::string path = "E:/AQ_DB/"; 
  aq::ColumnMapper_Intf<int32_t> * cm = new aq::ColumnMapper<int32_t, aq::WIN32FileMapper>(path.c_str(), tableId, columnId, size, aq::packet_size);
  
  size_t index = 0;
  int32_t value = 0;
  loader_t<int32_t> loader;

  while (loader.load(cm, index++))
  {
    loader.dump(std::cout);
    std::cout << std::endl;
  }

  index = 1;
  value = 3;
  cm->setValue(index, &value);
  
  index = 0;
  while (loader.load(cm, index++))
  {
    loader.dump(std::cout);
    std::cout << std::endl;
  }
  
  index = 1;
  value = 2;
  cm->setValue(index, &value);
  
  index = 0;
  while (loader.load(cm, index++))
  {
    loader.dump(std::cout);
    std::cout << std::endl;
  }
  
  index = 1;
  value = 4;
  cm->setValue(index, &value);
  
  index = 0;
  while (loader.load(cm, index++))
  {
    loader.dump(std::cout);
    std::cout << std::endl;
  }

}

#endif

BOOST_AUTO_TEST_SUITE_END()
