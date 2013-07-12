// TestAQEngine.cpp : définit le point d'entrée pour l'application console.
//

#include "Util.h"
#include <aq/Logger.h>
#include <string>
#include <cstdint>
#include <fstream>
#include <vector>
#include <iostream>
#include <boost/filesystem.hpp>
#include <boost/program_options.hpp>

namespace po = boost::program_options;

bool aq::verbose = false;

extern int functional_tests(const std::string& dbPath, std::string& queryIdent, const std::string& aqEngine, 
                            const std::string& queriesFilename, const std::string& logFilename, uint64_t limit, bool stopOnError);

int main(int argc, char ** argv)
{

  try
  {
    std::string logMode;
    std::string rootPath;
    std::string dbName;
    std::string queryIdent;
    std::string aqEngine;
    std::string queriesFilename;
    std::string logFilename;
    size_t limit;
    unsigned int logLevel;
    bool stopOnError = false;

    po::options_description desc("Allowed options");
		desc.add_options()
			("help,h", "produce help message")
			("log-output", po::value<std::string>(&logMode)->default_value("STDOUT"), "[STDOUT|LOCALFILE|SYSLOG]")
			("log-level", po::value<unsigned int>(&logLevel)->default_value(AQ_LOG_NOTICE), "CRITICAL(2), ERROR(3), WARNING(4), NOTICE(5), INFO(6), DEBUG(7)")
      ("root-path,r", po::value<std::string>(&rootPath)->default_value("E:/AQ_DATABASES/DB/"), "")
      ("db-name,n", po::value<std::string>(&dbName)->default_value("MSALGOQUEST"), "")
      ("limit,l", po::value<size_t>(&limit)->default_value(0), "0 -> no limit")
      ("query-ident,i", po::value<std::string>(&queryIdent)->default_value("test_aq_engine"), "")
      ("aq-engine,e", po::value<std::string>(&aqEngine)->default_value("E:/AQ_Bin/AQ_Engine_21_11.exe"), "")
      ("queries,q", po::value<std::string>(&queriesFilename)->default_value("queries.aql"), "")
      ("log,o", po::value<std::string>(&logFilename)->default_value("./test_aq_engine.log"), "")
      ("stop-on-error,s", po::bool_switch(&stopOnError), "")
      
      ("verbose,v", po::bool_switch(&aq::verbose), "set verbosity")
			;

		po::positional_options_description p;
		p.add("backward-compatibility", -1);

		po::variables_map vm;
		po::store(po::command_line_parser(argc, argv).options(desc).positional(p).run(), vm);
		po::notify(vm);    

    if (vm.count("help"))
    {
      std::cout << desc << "\n";
      return 0;
    }
    
		//
		// Initialize Logger
		aq::Logger::getInstance(argv[0], logMode == "STDOUT" ? STDOUT : logMode == "LOCALFILE" ? LOCALFILE : logMode == "SYSLOG" ? SYSLOG : STDOUT);
		aq::Logger::getInstance().setLevel(logLevel);
    
    std::string dbPath = rootPath + "/" + dbName + "/";

    return functional_tests(dbPath, queryIdent, aqEngine, queriesFilename, logFilename, limit, stopOnError);
  }
  catch (const std::exception& e)
  {
    std::cerr << e.what() << std::endl;
  }

  return EXIT_SUCCESS;
}