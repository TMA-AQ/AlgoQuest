#include "AQEngineSimulate.h"

#include <aq/Exceptions.h>
#include <aq/Logger.h>
#include <aq/Timer.h>
#include <aq/QueryReader.h>
#include <aq/AQEngine.h>
#include "VerbBuilder.h"
#include "AQManager.h"
#include "CommandHandler.h"
#include "AQEngineSimulate.h"

// #include <aq/AQThreadRequest.h>
// #include <aq/AQFunctor.h>

#if defined (WIN32)
#  include <io.h>
#else
#  include <unistd.h>
#  define _isatty isatty
#  define _fileno fileno
#endif
#include <iostream>
#include <fstream>
#include <boost/thread.hpp>
#include <boost/program_options.hpp>
#include <boost/algorithm/string/replace.hpp>

#define AQ_TOOLS_VERSION "0.1.0"

namespace po = boost::program_options;

size_t failedQueries = 0;
boost::mutex parserMutex;

extern int yylineno;
int yyerror( const char *pszMsg ) 
{
	aq::Logger::getInstance().log(AQ_ERROR, "SQL Parsing Error : %s encountered at line number: %d\n", pszMsg, yylineno);
	return 0;
}

// -------------------------------------------------------------------------------------------------
int processSQLQueries(const std::string  & query, 
                      const aq::Settings & settingsBase, 
                      const std::string    queryIdent, 
                      aq::Base           & baseDesc, 
                      bool                 simulateAQEngine,
                      bool                 keepFiles, 
                      bool                 force)
{

  aq::Logger::getInstance().log(AQ_INFO, "%s\n", query.c_str());
  aq::Timer timer;

  //
  // Settings
  aq::Settings settings(settingsBase);

  //
  // Load AQ engine
  aq::AQEngine_Intf * aq_engine;
  if (simulateAQEngine)
  {
    aq::Logger::getInstance().log(AQ_INFO, "Do not use aq engine\n");
    aq_engine = new aq::AQEngineSimulate(baseDesc, settings);
  }
  else
  {
    aq::Logger::getInstance().log(AQ_INFO, "Use aq engine: '%s'\n", settings.aqEngine.c_str());
    aq_engine = new aq::AQEngineSystem(baseDesc, settings);
  }

  //
  // prepare and process query
  std::string answer;

  if (!((prepareQuery(query, settingsBase, baseDesc, settings, answer, queryIdent, force) == EXIT_SUCCESS) &&
    (processQuery(query, settings, baseDesc, aq_engine, answer, keepFiles) == EXIT_SUCCESS)))
  {
    aq::Logger::getInstance().log(AQ_DEBUG, "QUERY FAILED:\n%s\n", query.c_str());
  }
  else
  {
    aq::Logger::getInstance().log(AQ_NOTICE, "Query Time elapsed: %s\n", aq::Timer::getString(timer.getTimeElapsed()).c_str());
  }

	return EXIT_SUCCESS;
}

// -------------------------------------------------------------------------------------------------
int parse_queries(const std::string & aqHome, 
                  const std::string & aqName, 
                  const std::string & queryIdent, 
                  const std::string & sqlQueriesFile, 
                  const std::string & aqMatrixFileName,
                  aq::Settings      & settings, 
                  aq::Base          & baseDesc, 
                  bool                transform, 
                  bool                simulateAQEngine, 
                  bool                keepFiles, 
                  bool                force)
{
  //
  // print info if use as command line tool
  if (settings.cmdLine)
  {
    std::cout << "Welcome to AlgoQuest Monitor version " << AQ_TOOLS_VERSION << std::endl;
    std::cout << "Copyright (c) 2013, AlgoQuest System. All rights reserved." << std::endl;
    std::cout << std::endl;
    if (settings.rootPath != "")
    {
      std::cout << "Connected to database " << settings.rootPath << std::endl;
      std::cout << std::endl;
    }
  }

  //
  // parse inpout
  aq::QueryReader * reader = 0;
  std::fstream * fqueries = 0;

  if (sqlQueriesFile != "")
  {
    boost::filesystem::path p(sqlQueriesFile);
    if (!boost::filesystem::exists(p)) 
    {
      std::cerr << "cannot find file " << p << std::endl;
      return -1;
    }

    fqueries = new std::fstream(sqlQueriesFile.c_str(), std::ifstream::in);
    reader = new aq::QueryReader(*fqueries);
  }
  else
  {
    reader = new aq::QueryReader(std::cin, settings.cmdLine ? "aq" : "");
  }

  // 
  // Get current database name from settings if set
  if (aqName == "")
  {
    std::string databaseName = settings.rootPath;
    boost::replace_all(databaseName, "\\", "/");
    while ((databaseName.size() > 0) && (*databaseName.rbegin() == '/'))
    {
      databaseName.erase(databaseName.size() - 1);
    }
    std::string::size_type pos = databaseName.find_last_of('/');
    if (pos == std::string::npos)
    {
      databaseName = "";
    }
    else
    {
      databaseName = databaseName.substr(pos + 1);
    }
  }
  else
  {
    settings.initPath(aqHome + aqName);
  }

  std::string query;
  aq::CommandHandler cmdHandler(aqHome, aqName, settings, baseDesc);
  while ((query = reader->next()) != "")
  {
    if (transform)
    {
      transform_query(query, settings, baseDesc);
    }
    else if (aqMatrixFileName != "")
    {
      process_aq_matrix(query, aqMatrixFileName, settings.outputFile, settings, baseDesc);
    }
    else 
    {
      if (cmdHandler.process(query) == -1)
      {
        processSQLQueries(query, settings, queryIdent, baseDesc, simulateAQEngine, keepFiles, force);
      }
    }
  }
  return EXIT_SUCCESS;
}

// -------------------------------------------------------------------------------------------------
int main(int argc, char**argv)
{
	try
	{

		// Settings
		aq::Settings settings;      
    settings.outputFile = "stdout";

		// log options
		std::string mode;
		std::string ident;
		unsigned int level;
		bool lock_mode = false;
		bool date_mode = false;
		bool pid_mode = false;

		// aq options
    std::string aqHome;
    std::string aqName;
		std::string propertiesFile;
		std::string queryIdent;
		std::string sqlQuery;
		std::string sqlQueriesFile;
		std::string baseDescr;
    std::string DLLFunction;
		unsigned int worker;
		bool multipleAnswerFiles = false;
		bool keepFiles = false;
		bool display = false;
    bool displayCount = false;
    bool trace = false;
		bool loadDatabase = false;
    bool force = false;
    bool useTextAQMatrix = false;

    // testing purpose options
		std::string aqMatrixFileName;
		bool transform = false;
    bool checkDatabase = false;
		bool simulateAQEngine = false;
		bool skipNestedQuery = false;

    // load option
    std::string tableNameToLoad;

    // generate tmp table option
    unsigned int nbValues = 100;
    unsigned int minValue = 0;
    unsigned int maxValue = 100;
    unsigned int nbTables = 1;
    bool generateTmpTable = false;

    char * s = ::getenv("AQ_HOME");
    if (s != NULL)
      aqHome = s;

    //
    // initialize verb builder
    aq::VerbBuilder vb;
    aq::verb::VerbFactory::GetInstance().setBuilder(&vb);

    //
    // if aq.ini exists in current directory, use it as default settings
    settings.iniFile = "aq.ini";
    boost::filesystem::path iniFile(settings.iniFile);
    if (boost::filesystem::exists(iniFile))
    {
      settings.load(settings.iniFile);
    }

    //
    // look for properties file in args
    for (size_t i = 1; i < argc; i++)
    {
      //
      // read ini file
      if ((strcmp(argv[i], "-s") == 0) || (strcmp(argv[i], "--settings") == 0))
      {
        if ((i + 1) < argc)
        {
          propertiesFile = argv[i+1];
          settings.load(propertiesFile);
        }
      }

    }

    //
    // command line arguments are prior to settings file
    po::options_description all("Allowed options");
		all.add_options()
			("help,h", "produce help message")
      ;

    po::options_description log_options("Logging");
    log_options.add_options()
			("log-output", po::value<std::string>(&mode)->default_value("STDOUT"), "[STDOUT|LOCALFILE|SYSLOG]")
			("log-level,v", po::value<unsigned int>(&level)->default_value(AQ_LOG_WARNING), "CRITICAL(2), ERROR(3), WARNING(4), NOTICE(5), INFO(6), DEBUG(7)")
			("log-lock", po::bool_switch(&lock_mode), "for multithread program")
			("log-date", po::bool_switch(&date_mode), "add date to log")
			("log-pid", po::bool_switch(&pid_mode), "add thread id to log")
			("log-ident", po::value<std::string>(&ident)->default_value("aq_query_resolver"), "")
			;

    po::options_description engine("Engine");
    engine.add_options()
      ("settings,s", po::value<std::string>(&propertiesFile), "")
      ("aq-engine,e", po::value<std::string>(&settings.aqEngine))
      ("aq-home,r", po::value<std::string>(&aqHome)->default_value(aqHome), "set AQ Home (AQ_HOME environment variable)")
      ("aq-name,n", po::value<std::string>(&aqName), "")
			("query-ident,i", po::value<std::string>(&queryIdent), "")
      ("queries-file,f", po::value<std::string>(&sqlQueriesFile), "")
			("output,o", po::value<std::string>(&settings.outputFile), "")
			("worker,w", po::value<unsigned int>(&worker), "number of thread assigned to resolve the bunch of sql queries")
			("parralellize,p", po::value<size_t>(&settings.process_thread)->default_value(settings.process_thread), "number of thread assigned resolve one sql queries")
			("display-count", po::bool_switch(&displayCount), "")
      ("force", po::bool_switch(&force), "force use of directory if it already exists")
			("keep-file,k", po::bool_switch(&keepFiles), "")
      ("trace,t", po::bool_switch(&trace), "")
      ;

    po::positional_options_description positionalOptions; 
    positionalOptions.add("aq-name", -1); 

    po::options_description testing("Testing");
    testing.add_options()
      ("simulate-aq-engine,z", po::bool_switch(&simulateAQEngine), "")
			("transform", po::bool_switch(&transform), "")
			("skip-nested-query", po::value<bool>(&settings.skipNestedQuery), "")
			("aq-matrix", po::value<std::string>(&aqMatrixFileName), "")
      ("check-database", po::bool_switch(&checkDatabase), "")
      ;

    po::options_description external("External");
    external.add_options()
      ("use-dll-function", po::value<std::string>(&DLLFunction), "Choise your own .dll to use your function")
      ("use-bin-aq-matrix", po::bool_switch(&useTextAQMatrix), "")
      ;

    po::options_description loader("Loader");
    loader.add_options()
      ("aq-loader,l", po::value<std::string>(&settings.aqLoader))
			("load-db", po::bool_switch(&loadDatabase), "")
      ("load-table", po::value<std::string>(&tableNameToLoad), "")
			;
    
    po::options_description genTmpTable("GenerateTmpTable [TESTING PURPOSE]");
    genTmpTable.add_options()
      ("gen-tmp-table", po::bool_switch(&generateTmpTable), "")
			("nb-values", po::value<unsigned int>(&nbValues), "")
      ("min-value", po::value<unsigned int>(&minValue), "")
      ("max-value", po::value<unsigned int>(&maxValue), "")
      ("nb-tables", po::value<unsigned int>(&nbTables), "")
			;

    all.add(log_options).add(engine).add(testing).add(external).add(loader).add(genTmpTable);

		po::variables_map vm;
		po::store(po::command_line_parser(argc, argv).options(all).positional(positionalOptions).run(), vm);
		po::notify(vm);    

		if (vm.count("help"))
		{
			std::cout << all << "\n";
			return 1;
		}

    // parse positional options
    if (vm.count("aq-name"))
    {
      aqName = vm["aq-name"].as<std::string>();
    }
		
    //
    // settings flags bool
    settings.trace = trace || settings.trace;
    settings.displayCount = displayCount || settings.displayCount;
    settings.cmdLine = _isatty(_fileno(stdin)) != 0;
    
    //
    //
    boost::replace_all(aqHome, "\\", "/");
    boost::replace_all(aqHome, "//", "/");
    if ((!aqHome.empty()) && (*aqHome.rbegin() != '/'))
    {
      aqHome = aqHome + "/";
    }

    //
    //
    if (aqName == "")
    {
      aqName = settings.aqName;
    }
    if ((aqHome != "") && (aqName != ""))
    {
      settings.initPath(aqHome + aqName);
    }

		//
		// Initialize Logger
		aq::Logger::getInstance(ident.c_str(), mode == "STDOUT" ? STDOUT : mode == "LOCALFILE" ? LOCALFILE : mode == "SYSLOG" ? SYSLOG : STDOUT);
		aq::Logger::getInstance().setLevel(level);
		aq::Logger::getInstance().setLockMode(lock_mode);
		aq::Logger::getInstance().setDateMode(date_mode);
		aq::Logger::getInstance().setPidMode(pid_mode);
    
		//
		// print Project Settings
    aq::Logger::getInstance().log(AQ_DEBUG, "Settings:\n%s\n", settings.to_string().c_str());

		//
		// If Load database is invoked
		if (loadDatabase)
		{
      aq::base_t bd;
      if (aq::build_base_from_raw(settings.dbDesc.c_str(), bd) != -1)
      {
        return load_database(settings, bd, tableNameToLoad);
      }
      else
      {
        aq::Logger::getInstance().log(AQ_CRITICAL, "cannot find database desc file '%s'\n", settings.dbDesc.c_str());
        return EXIT_FAILURE;
      }
      assert(false);
		}

    //
    // Check Database
    if (checkDatabase)
    {
      return check_database(settings);
    }

    //
    // If generated temporary table is invoked
    if (generateTmpTable)
    {
      aq::base_t bd;
      if (aq::build_base_from_raw(settings.dbDesc.c_str(), bd) != -1)
      {
        int rc = 0;
        while ((nbTables-- > 0) && ((rc = generate_tmp_table(settings, bd, nbValues, minValue, maxValue)) == 0));
        return rc;
      }
      else
      {
        aq::Logger::getInstance().log(AQ_CRITICAL, "cannot find database desc file '%s'\n", settings.dbDesc.c_str());
        return EXIT_FAILURE;
      }
    }

    //
    // Solve Queries
    aq::Base bd(settings.dbDesc);
    return parse_queries(
      aqHome, aqName, queryIdent, sqlQueriesFile, aqMatrixFileName, 
      settings, bd,
      transform, simulateAQEngine, keepFiles, force);

  }
	catch (const aq::generic_error& error)
	{
    aq::Logger::getInstance().log(AQ_CRITICAL, error.what());
		std::cerr << "generic error: " << error.what() << std::endl;
		return error.getType();
	}
	catch (const std::exception& ex)
	{
		std::cerr << "standard exception" << ex.what() << std::endl;
		return EXIT_FAILURE;
	}
	catch (...)
	{
		std::cerr << "UNHANDLE EXCEPTION" << std::endl;
		return EXIT_FAILURE;
	}

  if (failedQueries) return EXIT_FAILURE;
	return EXIT_SUCCESS;
}
