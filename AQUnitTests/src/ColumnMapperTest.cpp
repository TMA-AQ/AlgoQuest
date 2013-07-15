#include <boost/test/unit_test.hpp>

#include <iostream>
#include <set>
#include <aq/FileMapper.h>
#include <aq/WIN32FileMapper.h>
#include <aq/ColumnMapper.h>
#include <aq/BaseDesc.h>
#include <aq/Utilities.h>

void load(aq::ColumnMapper_Intf * cm, size_t limit)
{
  aq::ColumnItem item;
  char buffer[128];
  for (size_t i = 0; i < limit; i++)
  {
    cm->loadValue(i, item);
    item.toString(buffer, cm->getType());
  }
}

BOOST_AUTO_TEST_SUITE(ColumnMapper)

BOOST_AUTO_TEST_CASE(basic_int_column_mapper)
{
  size_t tableId = 1; 
  size_t columnId = 8; 
  size_t size = 1;
  std::string path = "E:/AQ_DATABASES/DB/MSALGOQUEST/data_orga/vdg/data/"; 
  aq::ColumnMapper_Intf * cm = new aq::ColumnMapper<int32_t, aq::FileMapper>(path.c_str(), tableId, columnId, size, aq::packet_size);
  load(cm, 10);
}

BOOST_AUTO_TEST_CASE(basic_big_int_column_mapper)
{
  size_t tableId = 1; 
  size_t columnId = 11; 
  size_t size = 1;
  std::string path = "E:/AQ_DATABASES/DB/MSALGOQUEST/data_orga/vdg/data/"; 
  aq::ColumnMapper_Intf * cm = new aq::ColumnMapper<int64_t, aq::FileMapper>(path.c_str(), tableId, columnId, size, aq::packet_size);
  load(cm, 10);
}

BOOST_AUTO_TEST_CASE(basic_double_column_mapper)
{
  size_t tableId = 1; 
  size_t columnId = 10; 
  size_t size = 1;
  std::string path = "E:/AQ_DATABASES/DB/MSALGOQUEST/data_orga/vdg/data/"; 
  aq::ColumnMapper_Intf * cm = new aq::ColumnMapper<double, aq::FileMapper>(path.c_str(), tableId, columnId, size, aq::packet_size);
  load(cm, 10);
}

BOOST_AUTO_TEST_CASE(basic_varchar_column_mapper)
{
  size_t tableId = 1; 
  size_t columnId = 2; 
  size_t size = 11;
  std::string path = "E:/AQ_DATABASES/DB/MSALGOQUEST/data_orga/vdg/data/"; 
  aq::ColumnMapper_Intf * cm = new aq::ColumnMapper<char, aq::FileMapper>(path.c_str(), tableId, columnId, size, aq::packet_size);
  load(cm, 10);
}

BOOST_AUTO_TEST_CASE(windows_int_column_mapper)
{
  size_t tableId = 1; 
  size_t columnId = 8; 
  size_t size = 1;
  std::string path = "E:/AQ_DATABASES/DB/MSALGOQUEST/data_orga/vdg/data/"; 
  aq::ColumnMapper_Intf * cm = new aq::ColumnMapper<int32_t, aq::WIN32FileMapper>(path.c_str(), tableId, columnId, size, aq::packet_size);
  load(cm, 10);
}

BOOST_AUTO_TEST_CASE(windows_big_int_column_mapper)
{
  size_t tableId = 1; 
  size_t columnId = 11; 
  size_t size = 1;
  std::string path = "E:/AQ_DATABASES/DB/MSALGOQUEST/data_orga/vdg/data/"; 
  aq::ColumnMapper_Intf * cm = new aq::ColumnMapper<int64_t, aq::WIN32FileMapper>(path.c_str(), tableId, columnId, size, aq::packet_size);
  load(cm, 10);
}

BOOST_AUTO_TEST_CASE(windows_double_column_mapper)
{
  size_t tableId = 1; 
  size_t columnId = 10; 
  size_t size = 1;
  std::string path = "E:/AQ_DATABASES/DB/MSALGOQUEST/data_orga/vdg/data/"; 
  aq::ColumnMapper_Intf * cm = new aq::ColumnMapper<double, aq::WIN32FileMapper>(path.c_str(), tableId, columnId, size, aq::packet_size);
  load(cm, 10);
}

BOOST_AUTO_TEST_CASE(windows_varchar_column_mapper)
{
  size_t tableId = 1; 
  size_t columnId = 2; 
  size_t size = 11;
  std::string path = "E:/AQ_DATABASES/DB/MSALGOQUEST/data_orga/vdg/data/"; 
  aq::ColumnMapper_Intf * cm = new aq::ColumnMapper<char, aq::WIN32FileMapper>(path.c_str(), tableId, columnId, size, aq::packet_size);
  load(cm, 10);
}

BOOST_AUTO_TEST_SUITE_END()
