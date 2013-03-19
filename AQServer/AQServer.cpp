#include "aq/Server.h"
#include <cstdlib>
#include <iostream>
#include <boost/program_options.hpp>
#include <aq/Logger.h>

namespace po = boost::program_options;

int main(int argc, char ** argv)
{
  // log options
  std::string mode;
  unsigned int level = LOG_DEBUG;
  bool lock_mode = false;
  bool date_mode = false;
  bool pid_mode = false;

  // aq options
  boost::uint16_t port;
  std::string properties;

  po::options_description desc("Allowed options");
  desc.add_options()
    ("help", "produce help message")
    ("log-output", po::value<std::string>(&mode)->default_value("STDOUT"), "")
    ("log-level", po::value<unsigned int>(&level), "")
    ("log-lock-mode", po::bool_switch(&lock_mode), "")
    ("log-date-mode", po::bool_switch(&date_mode), "")
    ("log-pid-mode", po::bool_switch(&pid_mode), "")
    ("aq-port", po::value<boost::uint16_t>(&port)->default_value(9999), "")
    ("aq-ini", po::value<std::string>(&properties)->default_value("properties.ini"), "")
    ;

  po::variables_map vm;
  po::store(po::parse_command_line(argc, argv, desc), vm);
  po::notify(vm);    

  if (vm.count("help")) 
  {
    std::cout << desc << "\n";
    return 1;
  }

  aq::Logger::getInstance("check-log.log", mode == "STDOUT" ? STDOUT : mode == "LOCALFILE" ? LOCALFILE : mode == "SYSLOG" ? SYSLOG : STDOUT);
  aq::Logger::getInstance().setLevel(level);
  aq::Logger::getInstance().setLockMode(lock_mode);
  aq::Logger::getInstance().setDateMode(date_mode);
  aq::Logger::getInstance().setPidMode(pid_mode);

  try
  {
    aq::Logger::getInstance().log(AQ_NOTICE, "starting algoquest-server on %u\n", port);
    boost::asio::io_service io_service;
    aq::Server server(io_service, port);
    io_service.run();
  }
  catch (const std::exception& ex)
  {
    std::cerr << ex.what() << std::endl;
    return EXIT_FAILURE;
  }

  return EXIT_SUCCESS;
}

