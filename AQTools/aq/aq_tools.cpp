#include <aq/AQEngine.h>
#include <aq/SQLPrefix.h>
#include <aq/Column2Table.h>
#include <aq/QueryResolver.h>
#include <aq/parser/ID2Str.h>
#include <aq/parser/SQLParser.h>
#include <aq/parser/JeqParser.h>
#include <aq/Exceptions.h>
#include <aq/BaseDesc.h>
#include <aq/TreeUtilities.h>
#include <aq/db_loader/DatabaseLoader.h>
#include <aq/Logger.h>
#include <aq/QueryReader.h>
#include "AQEngineSimulate.h"
#include <io.h>
#include <cstdio>
#include <iostream>
#include <list>
#include <fstream>
#include <string>
#include <boost/thread.hpp>
#include <boost/program_options.hpp>
#include <boost/filesystem.hpp>
#include <boost/property_tree/ptree.hpp>
#include <boost/property_tree/ini_parser.hpp>
#include <boost/algorithm/string/trim.hpp>
#include <boost/algorithm/string/replace.hpp>
#include <boost/date_time/posix_time/posix_time.hpp>
#include <boost/uuid/uuid.hpp>
#include <boost/uuid/uuid_generators.hpp>
#include <boost/uuid/uuid_io.hpp>
#include <boost/logic/tribool.hpp>

#include <aq/AQThreadRequest.h>

// #include <aq/AQFunctor.h>

// fixme
#include "Link.h"

#define AQ_TOOLS_VERSION "0.1.0"

namespace po = boost::program_options;
namespace fs = boost::filesystem;

bool interact_cmd_line;
size_t failedQueries = 0;
boost::mutex parserMutex;

extern int yylineno;
int yyerror( const char *pszMsg ) 
{
	aq::Logger::getInstance().log(AQ_ERROR, "SQL Parsing Error : %s encountered at line number: %d\n", pszMsg, yylineno);
	return 0;
}

// -------------------------------------------------------------------------------------------------
int processAQMatrix(const std::string& query, const std::string& aqMatrixFileName, const std::string& answerFileName, aq::TProjectSettings& settings, aq::Base& baseDesc)
{
	aq::tnode	*pNode;
	int	nRet;

	//
	// Parse SQL request
	{

		boost::mutex::scoped_lock lock(parserMutex);
		aq::Logger::getInstance().log(AQ_INFO, "parse sql query: '%s'\n", query.c_str());
		if ((nRet = SQLParse(query.c_str(), &pNode)) != 0 ) 
		{
			aq::Logger::getInstance().log(AQ_ERROR, "error parsing sql request '%s'\n", query.c_str());
			return EXIT_FAILURE;
		}

	}
	
  boost::array<uint32_t, 6> categories_order = { K_FROM, K_WHERE, K_SELECT, K_GROUP, K_HAVING, K_ORDER };
	aq::verb::VerbNode::Ptr spTree = aq::verb::VerbNode::BuildVerbsTree(pNode, categories_order, baseDesc, &settings );
	spTree->changeQuery();
	aq::cleanQuery( pNode );

	settings.answerFile = aqMatrixFileName;

	boost::shared_ptr<aq::AQMatrix> aqMatrix(new aq::AQMatrix(settings, baseDesc));
	std::vector<llong> tableIDs;
	
	aq::Timer timer;
	std::vector<std::string> answerFile;
	answerFile.push_back(aqMatrixFileName);
	for (std::vector<std::string>::const_iterator it = answerFile.begin(); it != answerFile.end(); ++it)
	{
		aqMatrix->load((*it).c_str(), tableIDs);
	}
	// aqMatrix->simulate(113867938, 2);
	// aqMatrix->simulate(10, 2);
	// tableIDs.push_back(8);
	aq::Logger::getInstance().log(AQ_INFO, "Load AQ Matrix: Elapsed Time = %s\n", aq::Timer::getString(timer.getTimeElapsed()).c_str());

	aq::AQEngineSimulate aqEngine(baseDesc, settings);
	aqEngine.setAQMatrix(aqMatrix);
	aqEngine.setTablesIDs(tableIDs);
  
  unsigned int id_generator = 1;
	aq::QueryResolver queryResolver(pNode, &settings, &aqEngine, baseDesc, id_generator);
  queryResolver.solveAQMatrix(spTree);

	aq::Table::Ptr result = queryResolver.getResult();
	if (result)
	{
		result->saveToAnswer(answerFileName.c_str(), settings.fieldSeparator);
	}

	return 0;
}

// -------------------------------------------------------------------------------------------------
int transformQuery(const std::string& query, aq::TProjectSettings& settings, aq::Base& baseDesc)
{
	aq::tnode	*pNode;
	int	nRet;

	//
	// Parse SQL request
	{

		boost::mutex::scoped_lock lock(parserMutex);
		aq::Logger::getInstance().log(AQ_INFO, "parse sql query %s\n", query.c_str());
		if ((nRet = SQLParse(query.c_str(), &pNode)) != 0 ) 
		{
			aq::Logger::getInstance().log(AQ_ERROR, "error parsing sql request '%s'\n", query.c_str());
			return EXIT_FAILURE;
		}

	}
  
  boost::array<uint32_t, 6> categories_order = { K_FROM, K_WHERE, K_SELECT, K_GROUP, K_HAVING, K_ORDER };
	aq::verb::VerbNode::Ptr spTree = aq::verb::VerbNode::BuildVerbsTree(pNode, categories_order, baseDesc, &settings );
	spTree->changeQuery();
	aq::cleanQuery( pNode );
	
	std::string str;
	aq::syntax_tree_to_aql_form(pNode, str);
  aq::ParseJeq( str );

	return 0;
}

// -------------------------------------------------------------------------------------------------
int init_base_desc(const std::string& file, aq::Base& bd)
{
  if (file == "")
  {
    throw aq::generic_error(aq::generic_error::INVALID_BASE_FILE, "no database specify");
  }
  aq::Logger::getInstance().log(AQ_INFO, "load base %s\n", file.c_str());
  std::fstream bdFile(file.c_str());
  aq::base_t baseDescHolder;
  if (file.substr(file.size() - 4) == ".xml")
  {
    aq::build_base_from_xml(bdFile, baseDescHolder);
    bd.loadFromBaseDesc(baseDescHolder);
  }
  else
  {
    // aq::build_base_from_raw(baseDescr.c_str(), baseDescHolder);
    bd.loadFromRawFile(file.c_str());
  }
  // baseDesc.loadFromBaseDesc(baseDescHolder);
  return 0;
}

// -------------------------------------------------------------------------------------------------
int load_database(const aq::TProjectSettings& settings, aq::base_t& baseDesc, const std::string& tableNameToLoad)
{
  for (size_t t = 0; t < baseDesc.table.size(); ++t)
  {
    if ((tableNameToLoad != "") && (tableNameToLoad != baseDesc.table[t].nom))
    {
      continue;
    }
    aq::DatabaseLoader loader(baseDesc, settings.aqLoader, settings.rootPath, settings.packSize, ','/*settings.fieldSeparator*/, settings.csvFormat); // FIXME
    loader.generate_ini();
    for (size_t c = 0; c < baseDesc.table[t].colonne.size(); ++c)
    {
      aq::Logger::getInstance().log(AQ_INFO, "loading column %d of table %d\n", c + 1, t + 1);
      loader.run(t + 1, c + 1);
    }
  }
  return EXIT_SUCCESS;
}

// -------------------------------------------------------------------------------------------------
int prepareQuery(const std::string& query, const aq::TProjectSettings& settingsBase, aq::Base& baseDesc, aq::TProjectSettings& settings, std::string& displayFile, const std::string queryIdentStr, bool force)
{		
	//
	// generate ident and ini file
  std::string queryIdentTmp = queryIdentStr;
	if (queryIdentTmp == "")
  {
    boost::uuids::uuid queryIdent = boost::uuids::random_generator()();
    std::ostringstream queryIdentOSS;
    queryIdentOSS << queryIdent;
    queryIdentTmp = queryIdentOSS.str();
    settings.changeIdent(queryIdentOSS.str());
  }
  else
  {
    settings.changeIdent(queryIdentTmp);
  }

	//
	// create directories
	std::list<fs::path> lpaths;
	lpaths.push_back(fs::path(settings.rootPath + "calculus/" + queryIdentTmp));
	lpaths.push_back(fs::path(settings.tmpPath));
	lpaths.push_back(fs::path(settings.dpyPath));
	for (std::list<fs::path>::const_iterator dir = lpaths.begin(); dir != lpaths.end(); ++dir)
	{
		if (fs::exists(*dir))
		{
      aq::Logger::getInstance().log(AQ_WARNING, "directory already exist '%s'\n", (*dir).string().c_str());
      if (!force)
      {
        return EXIT_FAILURE;
      }
		}
		else if (!fs::create_directory(*dir))
		{
			aq::Logger::getInstance().log(AQ_WARNING, "cannot create directory '%s'\n", (*dir).string().c_str());
      if (!force)
      {
        return EXIT_FAILURE;
      }
		}
	}

  //
  // write request file
  std::string queryFilename(settings.rootPath + "calculus/" + queryIdentTmp + "/Request.sql");
  std::ofstream queryFile(queryFilename.c_str());
  queryFile << query;
  queryFile.close();

	//
	// write ini file (it is needed for now by AQEngine)
	std::ofstream iniFile(settings.iniFile.c_str());
	settings.writeAQEngineIni(iniFile);
	iniFile.close();

	// generate answer file
	// displayFile = settings.szRootPath + "/calculus/" + queryIdentStr + "/display.txt"; // TODO
	displayFile = settings.rootPath + "calculus/" + queryIdentStr + "/answer.txt"; // TODO
	aq::Logger::getInstance().log(AQ_INFO, "save answer to %s\n", displayFile.c_str());

	return EXIT_SUCCESS;
}

// -------------------------------------------------------------------------------------------------
int processQuery(const std::string& query, aq::TProjectSettings& settings, aq::Base& baseDesc, aq::AQEngine_Intf * aq_engine,
                 const std::string& answer, bool keepFiles)
{
  int rc = EXIT_SUCCESS;

	try
	{
	
		aq::Logger::getInstance().log(AQ_INFO, "processing sql query\n");

		aq::tnode	*pNode  = NULL;
		int	nRet;

		//
		// Parse SQL request
		{

			boost::mutex::scoped_lock lock(parserMutex);
			aq::Logger::getInstance().log(AQ_INFO, "parse sql query %s\n", query.c_str());
			if ((nRet = SQLParse(query.c_str(), &pNode)) != 0 ) 
			{
				aq::Logger::getInstance().log(AQ_ERROR, "error parsing sql request '%s'\n", query.c_str());
				return EXIT_FAILURE;
			}

#if defined(_DEBUG) && defined(_TRACE)
			std::cout << *pNode << std::endl;
#endif

    }

    //---------------------------------------------------------------------------------------
    // test du functor
    //---------------------------------------------------------------------------------------
#if defined(_FUNCTOR)
    try
    {
      aq::AQFunctor test(pNode, "C:/Users/AlgoQuest/Documents/AlgoQuest/AQSuite/x64/Debug/AQFunctionTest.dll");
      test.dump(std::cout);
      test.callFunctor();
    }
    catch (...)
    {
      throw;
    }
#endif

    //---------------------------------------------------------------------------------------


    //
		// Transform SQL request in prefix form, 
    unsigned int id_generator = 1;
    aq::Timer timer;
		aq::QueryResolver queryResolver(pNode, &settings, aq_engine, baseDesc, id_generator);
		aq::Table::Ptr result = queryResolver.solve();
    timer.stop();

    if (interact_cmd_line)
    {
      std::cout << queryResolver.getNbRows() << " rows processed in " << aq::Timer::getString(timer.getTimeElapsed()) << std::endl;
      std::cout << std::endl;
    }

		delete pNode;
	}
	catch (const aq::generic_error& ge)
	{
		aq::Logger::getInstance().log(AQ_ERROR, "%s\n", ge.what());
		rc = EXIT_FAILURE;
	}
	catch (const std::exception& ex)
	{
		aq::Logger::getInstance().log(AQ_ERROR, "%s\n", ex.what());
		rc = EXIT_FAILURE;
	}
	catch (...)
	{
		aq::Logger::getInstance().log(AQ_ERROR, "unknown exception\n");
		rc = EXIT_FAILURE;
	}

  if (!keepFiles)
  {
    aq::Logger::getInstance().log(AQ_NOTICE, "remove temporary directory '%s'\n", settings.tmpPath.c_str());
    aq::DeleteFolder(settings.tmpPath.c_str());
    aq::Logger::getInstance().log(AQ_NOTICE, "remove working directory '%s'\n", settings.workingPath.c_str());
    aq::DeleteFolder(settings.workingPath.c_str());
  }

	return rc;
}

// -------------------------------------------------------------------------------------------------
int processSQLQueries(const std::string& query, 
                      const aq::TProjectSettings& settingsBase, aq::Base& baseDesc, bool simulateAQEngine,
                      bool keepFiles, const std::string queryIdent, bool force)
{

  aq::Logger::getInstance().log(AQ_INFO, "%s\n", query.c_str());
  boost::posix_time::ptime begin(boost::posix_time::microsec_clock::local_time());

  //
  // Settings
  aq::TProjectSettings settings(settingsBase);

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
    boost::posix_time::ptime end(boost::posix_time::microsec_clock::local_time());
    std::ostringstream oss;
    oss << "Query Time elapsed: " << (end - begin);
    aq::Logger::getInstance().log(AQ_NOTICE, "%s\n", oss.str().c_str());
  }

	return EXIT_SUCCESS;
}

// -------------------------------------------------------------------------------------------------
int parse_queries(aq::TProjectSettings& settings, aq::Base& baseDesc, 
                  const std::string& queryIdent, const std::string& sqlQueriesFile, const std::string aqMatrixFileName, 
                  bool transform, bool simulateAQEngine, bool keepFiles, bool force)
{
  //
  // print info if use as command line tool
  if (interact_cmd_line)
  {
    std::cout << "Welcome to AlgoQuest Monitor version " << AQ_TOOLS_VERSION << std::endl;
    std::cout << "Copyright (c) 2013, AlgoQuest System. All rights reserved." << std::endl;
    std::cout << std::endl;
    std::cout << "Connected to database " << settings.rootPath << std::endl;
    std::cout << std::endl;
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
    if (_isatty(_fileno(stdin)))
      reader = new aq::QueryReader(std::cin, "aq");
    else
      reader = new aq::QueryReader(std::cin);
  }

  std::string query;
  while ((query = reader->next()) != "")
  {
    if (transform)
    {
      transformQuery(query, settings, baseDesc);
    }
    else if (aqMatrixFileName != "")
    {
      processAQMatrix(query, aqMatrixFileName, settings.outputFile, settings, baseDesc);
    }
    else 
    {
      processSQLQueries(query, settings, baseDesc, simulateAQEngine, keepFiles, queryIdent, force);
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
		aq::TProjectSettings settings;      
    settings.outputFile = "stdout";

		// log options
		std::string mode;
		std::string ident;
		unsigned int level;
		bool lock_mode = false;
		bool date_mode = false;
		bool pid_mode = false;

		// aq options
		std::string propertiesFile;
		std::string queryIdent;
		std::string sqlQuery;
		std::string sqlQueriesFile;
		std::string baseDescr;
		std::string answerPathStr;
		std::string aqMatrixFileName;
		std::string answerFileName;
    std::string DLLFunction;
    std::string tableNameToLoad;
		unsigned int worker;
		bool simulateAQEngine = false;
		bool multipleAnswerFiles = false;
		bool keepFiles = false;
		bool display = false;
		bool transform = false;
		bool skipNestedQuery = false;
		bool loadDatabase = false;
    bool force = false;
    bool useTextAQMatrix = false;

		// old args for backward compatibility
		std::vector<std::string> oldArgs;
    
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
			("query-ident,i", po::value<std::string>(&queryIdent), "")
      ("queries-file,f", po::value<std::string>(&sqlQueriesFile), "")
			("answer-path", po::value<std::string>(&answerPathStr), "DEPRECATED") // deprecated
			("answer-file", po::value<std::string>(&answerFileName)->default_value("answer.txt"), "DEPRECATED") // deprecated
			("output,o", po::value<std::string>(&settings.outputFile), "")
			("base-descr", po::value<std::string>(&settings.dbDesc), "")
			("worker", po::value<unsigned int>(&worker), "number of thread assigned to resolve the bunch of sql queries")
			("query-worker,w", po::value<size_t>(&settings.process_thread)->default_value(settings.process_thread), "number of thread assigned resolve one sql queries")
			("display-count", po::value<bool>(&settings.displayCount), "")
      ("force", po::bool_switch(&force), "force use of directory if it already exists")
			("keep-file,k", po::bool_switch(&keepFiles), "")
      ;

    po::options_description testing("Testing");
    testing.add_options()
      ("simulate-aq-engine,z", po::bool_switch(&simulateAQEngine), "")
			("transform", po::bool_switch(&transform), "")
			("skip-nested-query", po::value<bool>(&settings.skipNestedQuery), "")
			("aq-matrix", po::value<std::string>(&aqMatrixFileName), "")
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

    all.add(log_options).add(engine).add(testing).add(external).add(loader);

		po::variables_map vm;
		po::store(po::command_line_parser(argc, argv).options(all).run(), vm);
		po::notify(vm);    

		if (vm.count("help"))
		{
			std::cout << all << "\n";
			return 1;
		}
		
		//
		// Initialize Logger
		aq::Logger::getInstance(ident.c_str(), mode == "STDOUT" ? STDOUT : mode == "LOCALFILE" ? LOCALFILE : mode == "SYSLOG" ? SYSLOG : STDOUT);
		aq::Logger::getInstance().setLevel(level);
		aq::Logger::getInstance().setLockMode(lock_mode);
		aq::Logger::getInstance().setDateMode(date_mode);
		aq::Logger::getInstance().setPidMode(pid_mode);
    
    //
    // check if aq-tools is used as an interactive cmd line
    interact_cmd_line = _isatty(_fileno(stdin)) && (sqlQueriesFile == "");

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
		}
    else
    {
      aq::Base bd;
      init_base_desc(settings.dbDesc, bd);
      return parse_queries(
        settings, bd, 
        queryIdent, sqlQueriesFile, aqMatrixFileName, 
        transform, simulateAQEngine, keepFiles, force);
    }

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
