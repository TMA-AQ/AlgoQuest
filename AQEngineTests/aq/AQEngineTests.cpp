// TestAQEngine.cpp�: d�finit le point d'entr�e pour l'application console.
//

#include "Util.h"
#include <aq/Logger.h>
#include <aq/parser/JeqParser.h>
#include <string>
#include <stdint.h>
#include <fstream>
#include <vector>
#include <iostream>
#include <boost/filesystem.hpp>
#include <boost/program_options.hpp>

namespace po = boost::program_options;

int yyerror(char const *)
{
  return 0;
}

extern int functional_tests(const std::string& dbPath, std::string& queryIdent, const std::string& aqEngine, 
                            const std::string& queriesFilename, const std::string& filter,
                            const std::string& logFilename, uint64_t limit, bool stopOnError);

int main(int argc, char ** argv)
{

  try
  {
    aq::opt o = { "", "", "", "", "", "", "", aq::packet_size, 0, false, false, false, false, false, false, false, false };
    std::string logMode;
    std::string rootPath;
    std::string dbName;
    std::string query;
    unsigned int logLevel;
    bool generate = false;

    po::options_description desc("Allowed options");
    desc.add_options()
      ("help,h", "produce help message")
      ("log-output", po::value<std::string>(&logMode)->default_value("STDOUT"), "[STDOUT|LOCALFILE|SYSLOG]")
      ("log-level", po::value<unsigned int>(&logLevel)->default_value(AQ_LOG_WARNING), "CRITICAL(2), ERROR(3), WARNING(4), NOTICE(5), INFO(6), DEBUG(7)")
      ("root-path,r", po::value<std::string>(&rootPath), "root databases path (mandatory)")
      ("db-name,n", po::value<std::string>(&dbName)->default_value("MSALGOQUEST"), "")
      ("working-path", po::value<std::string>(&o.workingPath)->default_value(""), "")
      ("limit,l", po::value<uint64_t>(&o.limit)->default_value(0), "0 -> no limit")
      ("query-ident,i", po::value<std::string>(&o.queryIdent)->default_value("test_aq_engine"), "")
      ("aq-engine,e", po::value<std::string>(&o.aqEngine), "aq engine path (mandatory)")
      ("queries,q", po::value<std::string>(&o.queriesFilename)->default_value("query.aql"), "")
      ("query", po::value<std::string>(&query)->default_value(""), "")
      ("parse", "")
      ("filter,f", po::value<std::string>(&o.filter)->default_value(""), "")
      ("log,o", po::value<std::string>(&o.logFilename)->default_value("./test_aq_engine.log"), "")

      ("execute", po::bool_switch(&o.execute), "")
      ("generate", po::bool_switch(&generate), "")
      ("check-result", po::bool_switch(&o.checkResult), "")
      ("check-condition", po::bool_switch(&o.checkCondition), "")
      ("aql-2-sql", po::bool_switch(&o.aql2sql), "")
      ("stop-on-error,s", po::bool_switch(&o.stopOnError), "")
      ("display,d", po::bool_switch(&o.display), "display rows")
      ("with-count,c", po::bool_switch(&o.withCount), "display count")
      ("with-index", po::bool_switch(&o.withIndex), "display table index")
      ;
    
    po::positional_options_description p;
    p.add("backward-compatibility", -1);
    
    po::variables_map vm;
    po::store(po::command_line_parser(argc, argv).options(desc).positional(p).run(), vm);
    po::notify(vm);    
    
    if (vm.count("help") || (((rootPath == "")  || (o.aqEngine == "")) && !vm.count("parse") && !vm.count("query")))
    {
      std::cout << desc << "\n";
      return 0;
    }
    
		//
		// Initialize Logger
		aq::Logger::getInstance(argv[0], logMode == "STDOUT" ? STDOUT : logMode == "LOCALFILE" ? LOCALFILE : logMode == "SYSLOG" ? SYSLOG : STDOUT);
		aq::Logger::getInstance().setLevel(logLevel);
    
    o.dbPath = rootPath + "/" + dbName + "/";

    if (vm.count("parse"))
    {
      aq::ParseJeq(query);
      std::cout << query << std::endl;
    }
    else if (generate)
    {
      //aq::QueryGenerator queryGen(query);
      //queryGen.generate(std::cout);
    }
    else
    {
      if (o.workingPath == "") o.workingPath = o.dbPath;
      return static_cast<int>(functional_tests(o));
    }
  }
  catch (const std::exception& e)
  {
    std::cerr << e.what() << std::endl;
    return EXIT_FAILURE;
  }
  
  return EXIT_SUCCESS;
}
