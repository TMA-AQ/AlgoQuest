#include <boost/test/unit_test.hpp>

#include <iostream>
#include <set>
#include <aq/GenericFileMapper.h>
#include <aq/WIN32FileMapper.h>
#include <aq/ColumnMapper.h>
#include <aq/BaseDesc.h>
#include <aq/Utilities.h>

struct loader_t
{
  aq::ColumnItem item;
  char buffer[128];
  bool load(aq::ColumnMapper_Intf * cm, size_t index)
  {
    int rc;
    if ((rc = cm->loadValue(index, item)) == 0)
      item.toString(buffer, cm->getType());
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
  aq::ColumnMapper_Intf * cm = new aq::ColumnMapper<int32_t, aq::GenericFileMapper>(path.c_str(), tableId, columnId, size, aq::packet_size);
  loader_t loader;
  loader.load(cm, 10);
}

BOOST_AUTO_TEST_CASE(basic_big_int_column_mapper)
{
  size_t tableId = 1; 
  size_t columnId = 11; 
  size_t size = 1;
  std::string path = "E:/AQ_DATABASES/DB/MSALGOQUEST/data_orga/vdg/data/"; 
  aq::ColumnMapper_Intf * cm = new aq::ColumnMapper<int64_t, aq::GenericFileMapper>(path.c_str(), tableId, columnId, size, aq::packet_size);
  loader_t loader;
  loader.load(cm, 10);
}

BOOST_AUTO_TEST_CASE(basic_double_column_mapper)
{
  size_t tableId = 1; 
  size_t columnId = 10; 
  size_t size = 1;
  std::string path = "E:/AQ_DATABASES/DB/MSALGOQUEST/data_orga/vdg/data/"; 
  aq::ColumnMapper_Intf * cm = new aq::ColumnMapper<double, aq::GenericFileMapper>(path.c_str(), tableId, columnId, size, aq::packet_size);
  loader_t loader;
  loader.load(cm, 10);
}

BOOST_AUTO_TEST_CASE(basic_varchar_column_mapper)
{
  size_t tableId = 1; 
  size_t columnId = 2; 
  size_t size = 11;
  std::string path = "E:/AQ_DATABASES/DB/MSALGOQUEST/data_orga/vdg/data/"; 
  aq::ColumnMapper_Intf * cm = new aq::ColumnMapper<char, aq::GenericFileMapper>(path.c_str(), tableId, columnId, size, aq::packet_size);
  loader_t loader;
  loader.load(cm, 10);
}

BOOST_AUTO_TEST_CASE(windows_int_column_mapper)
{
  size_t tableId = 1; 
  size_t columnId = 8; 
  size_t size = 1;
  std::string path = "E:/AQ_DATABASES/DB/MSALGOQUEST/data_orga/vdg/data/"; 
  aq::ColumnMapper_Intf * cm = new aq::ColumnMapper<int32_t, aq::WIN32FileMapper>(path.c_str(), tableId, columnId, size, aq::packet_size);
  loader_t loader;
  loader.load(cm, 10);
}

BOOST_AUTO_TEST_CASE(windows_big_int_column_mapper)
{
  size_t tableId = 1; 
  size_t columnId = 11; 
  size_t size = 1;
  std::string path = "E:/AQ_DATABASES/DB/MSALGOQUEST/data_orga/vdg/data/"; 
  aq::ColumnMapper_Intf * cm = new aq::ColumnMapper<int64_t, aq::WIN32FileMapper>(path.c_str(), tableId, columnId, size, aq::packet_size);
  loader_t loader;
  loader.load(cm, 10);
}

BOOST_AUTO_TEST_CASE(windows_double_column_mapper)
{
  size_t tableId = 1; 
  size_t columnId = 10; 
  size_t size = 1;
  std::string path = "E:/AQ_DATABASES/DB/MSALGOQUEST/data_orga/vdg/data/"; 
  aq::ColumnMapper_Intf * cm = new aq::ColumnMapper<double, aq::WIN32FileMapper>(path.c_str(), tableId, columnId, size, aq::packet_size);
  loader_t loader;
  loader.load(cm, 10);
}

BOOST_AUTO_TEST_CASE(windows_varchar_column_mapper)
{
  size_t tableId = 1; 
  size_t columnId = 2; 
  size_t size = 11;
  std::string path = "E:/AQ_DATABASES/DB/MSALGOQUEST/data_orga/vdg/data/"; 
  aq::ColumnMapper_Intf * cm = new aq::ColumnMapper<char, aq::WIN32FileMapper>(path.c_str(), tableId, columnId, size, aq::packet_size);
  loader_t loader;
  loader.load(cm, 10);
}

BOOST_AUTO_TEST_CASE(column_mapper)
{
  size_t tableId = 1; 
  size_t columnId = 1; 
  size_t size = 1;
  std::string path = "E:/AQ_DB/"; 
  aq::ColumnMapper_Intf * cm = new aq::ColumnMapper<int32_t, aq::WIN32FileMapper>(path.c_str(), tableId, columnId, size, aq::packet_size);
  
  size_t index = 0;
  loader_t loader;

  while (loader.load(cm, index++))
  {
    loader.dump(std::cout);
    std::cout << std::endl;
  }

  index = 1;
  loader.item.numval = 3;
  cm->setValue(index, loader.item);
  
  index = 0;
  while (loader.load(cm, index++))
  {
    loader.dump(std::cout);
    std::cout << std::endl;
  }
  
  index = 1;
  loader.item.numval = 2;
  cm->setValue(index, loader.item);
  
  index = 0;
  while (loader.load(cm, index++))
  {
    loader.dump(std::cout);
    std::cout << std::endl;
  }
  
  index = 1;
  loader.item.numval = 4;
  cm->setValue(index, loader.item);
  
  index = 0;
  while (loader.load(cm, index++))
  {
    loader.dump(std::cout);
    std::cout << std::endl;
  }

}

BOOST_AUTO_TEST_SUITE_END()
