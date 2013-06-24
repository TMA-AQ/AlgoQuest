#include <boost/test/unit_test.hpp>
#include <boost/program_options.hpp>
#include <fstream>

#include <aq/Base.h>

namespace po = boost::program_options;

BOOST_AUTO_TEST_SUITE(BaseDesc)

void getFilename(std::string& fname)
{  
  po::options_description desc("Allowed options");
  desc.add_options()
    ("base,b", po::value<std::string>(&fname)->default_value("./test/base.aq"), "")
    ;
  
  po::variables_map vm;
  po::store(po::command_line_parser(
    boost::unit_test::framework::master_test_suite().argc, 
    boost::unit_test::framework::master_test_suite().argv).options(desc).run(), vm);
  po::notify(vm); 
}

BOOST_AUTO_TEST_CASE(test_base_desc)
{
  std::string baseFilename;
  getFilename(baseFilename);

  aq::Base b1;
  b1.loadFromRawFile(baseFilename.c_str());
  
  aq::Base b2;
  aq::base_t baseDescHolder;
  std::stringstream ss;
  b1.dumpXml(ss);
  aq::build_base_from_xml(ss, baseDescHolder);
  b2.loadFromBaseDesc(baseDescHolder);

}

BOOST_AUTO_TEST_SUITE_END()