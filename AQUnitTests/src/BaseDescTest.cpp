#include <boost/test/unit_test.hpp>
#include <boost/program_options.hpp>
#include <fstream>

#include <aq/Base.h>

namespace po = boost::program_options;

BOOST_AUTO_TEST_SUITE(BaseDesc)

void generate_raw_base_file(const char * baseFilename)
{
  std::ofstream fin;
  fin.open(baseFilename, std::ios::trunc);
  if (!fin.is_open())
  {
    std::cerr << "cannot open file '" << baseFilename << "'" << std::endl;
    exit(-1);
  }
  fin << "TEST" << std::endl;
  fin << "2" << std::endl;
  fin << std::endl;

  fin << "\"table_1\" 1 10000 7" << std::endl;
  fin << "\"id\"      1 1 INT" << std::endl;
  fin << "\"champ_1\" 2 11 VARCHAR2" << std::endl;
  fin << "\"val_1\"   3 1 INT" << std::endl;
  fin << "\"val_2\"   4 1 FLOAT" << std::endl;
  fin << "\"val_3\"   5 1 DOUBLE" << std::endl;
  fin << "\"val_4\"   6 1 BIG_INT" << std::endl;
  fin << "\"date_1\"  7 1 DATE" << std::endl;
  fin << std::endl;

  fin << "\"table_2\" 2 100000 7" << std::endl;
  fin << "\"id\"      1 1 INT" << std::endl;
  fin << "\"champ_1\" 2 11 VARCHAR2" << std::endl;
  fin << "\"val_1\"   3 1 INT" << std::endl;
  fin << "\"val_2\"   4 1 FLOAT" << std::endl;
  fin << "\"val_3\"   5 1 DOUBLE" << std::endl;
  fin << "\"val_4\"   6 1 BIG_INT" << std::endl;
  fin << "\"date_1\"  7 1 DATE" << std::endl;

  fin.close();
}

BOOST_AUTO_TEST_CASE(test_base_desc)
{
  std::string baseFilename = "./base_desc_unit_test.aq";
  generate_raw_base_file(baseFilename.c_str());

  aq::Base b1(baseFilename.c_str());
  
  aq::Base b2;
  aq::base_t baseDescHolder;
  std::stringstream ss;
  b1.dumpXml(ss);
  aq::build_base_from_xml(ss, baseDescHolder);
  b2.loadFromBaseDesc(baseDescHolder);

}

BOOST_AUTO_TEST_SUITE_END()