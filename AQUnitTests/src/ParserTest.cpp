#include <boost/test/unit_test.hpp>
#include <boost/program_options.hpp>

namespace po = boost::program_options;

BOOST_AUTO_TEST_SUITE(Parser)

BOOST_AUTO_TEST_CASE(test_parser_1)
{
  boost::unit_test::framework::master_test_suite().argv;
  boost::unit_test::framework::master_test_suite().argc;
  std::string rootDirectory;
  po::options_description desc("Allowed options");
  desc.add_options()
    ("root-dir,r", po::value<std::string>(&rootDirectory)->default_value("./tests"), "")
    ;
  
  po::variables_map vm;
  po::store(po::command_line_parser(
    boost::unit_test::framework::master_test_suite().argc, 
    boost::unit_test::framework::master_test_suite().argv).options(desc).positional(p).run(), vm);
  po::notify(vm); 

  BOOST_CHECK(true);
  BOOST_REQUIRE(true);
  BOOST_TEST_MESSAGE("a message");
}

BOOST_AUTO_TEST_SUITE_END()