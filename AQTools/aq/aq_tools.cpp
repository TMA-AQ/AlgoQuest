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
#include "AQEngineSimulate.h"
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

namespace po = boost::program_options;
namespace fs = boost::filesystem;

extern int yylineno;
int yyerror( const char *pszMsg ) 
{
	std::cerr << "SQL Parsing Error : " << pszMsg << " encountered at line number: " << yylineno << std::endl;
	return 0;
}

size_t failedQueries = 0;
boost::mutex parserMutex;

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
  if (!settings.useRowResolver)
  {
    boost::array<uint32_t, 6> new_order = { K_FROM, K_WHERE, K_GROUP, K_HAVING, K_SELECT, K_ORDER };
    categories_order = new_order;
  }
	aq::verb::VerbNode::Ptr spTree = aq::verb::VerbNode::BuildVerbsTree(pNode, categories_order, baseDesc, &settings );
	spTree->changeQuery();
	aq::cleanQuery( pNode );

	strcpy(settings.szAnswerFN, aqMatrixFileName.c_str());

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
	if (settings.useRowResolver)
		queryResolver.solveAQMatriceByRows(spTree);
	else
		queryResolver.solveAQMatriceByColumns(spTree);
		

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
  if (!settings.useRowResolver)
  {
    boost::array<uint32_t, 6> new_order = { K_FROM, K_WHERE, K_GROUP, K_HAVING, K_SELECT, K_ORDER };
    categories_order = new_order;
  }
	aq::verb::VerbNode::Ptr spTree = aq::verb::VerbNode::BuildVerbsTree(pNode, categories_order, baseDesc, &settings );
	spTree->changeQuery();
	aq::cleanQuery( pNode );
	
	std::string str;
	aq::syntax_tree_to_prefix_form(pNode, str);
  aq::ParseJeq( str );

	return 0;
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
	lpaths.push_back(fs::path(settings.szRootPath + "/calculus/" + queryIdentTmp));
	lpaths.push_back(fs::path(settings.szTempPath1));
	lpaths.push_back(fs::path(settings.szTempPath2));
	for (std::list<fs::path>::const_iterator dir = lpaths.begin(); dir != lpaths.end(); ++dir)
	{
		if (fs::exists(*dir))
		{
			aq::Logger::getInstance().log(AQ_ERROR, "directory already exist '%s'\n", (*dir).c_str());
      if (!force)
      {
        return EXIT_FAILURE;
      }
		}

		if (!fs::create_directory(*dir))
		{
			aq::Logger::getInstance().log(AQ_ERROR, "cannot create directory '%s'\n", (*dir).c_str());
      if (!force)
      {
        return EXIT_FAILURE;
      }
		}
	}

  //
  // write request file
  std::string queryFilename(settings.szRootPath + "/calculus/" + queryIdentTmp + "/Request.sql");
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
	displayFile = settings.szRootPath + "/calculus/" + queryIdentStr + "/answer.txt"; // TODO
	aq::Logger::getInstance().log(AQ_INFO, "save answer to %s\n", displayFile.c_str());

	return EXIT_SUCCESS;
}

// -------------------------------------------------------------------------------------------------
int processQuery(const std::string& query, aq::TProjectSettings& settings, aq::Base& baseDesc, aq::AQEngine_Intf * aq_engine,
                 const std::string& answer, bool display, bool clean)
{
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

#if defined(_DEBUG)
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
		aq::QueryResolver queryResolver(pNode, &settings, aq_engine, baseDesc, id_generator);
		aq::Table::Ptr result = queryResolver.solve();
		if (!settings.useRowResolver && result)
		{
			aq::Timer timer;
			result->saveToAnswer(settings.szAnswerFN, settings.fieldSeparator);
			aq::Logger::getInstance().log(AQ_INFO, "Save Answer: Time Elapsed = %s\n", aq::Timer::getString(timer.getTimeElapsed()).c_str());
		}

		if (display)
		{
			std::ifstream fin(settings.szAnswerFN);
			std::string line;
			while (std::getline(fin, line))
			{
				std::cout << line << std::endl;
			}
			fin.close();
		}

		if (clean)
		{
			std::string workingDirectory = settings.szRootPath + "/calculus/" + settings.queryIdent;
			aq::Logger::getInstance().log(AQ_NOTICE, "remove working directory '%s'\n", workingDirectory.c_str());
			aq::DeleteFolder(workingDirectory.c_str());
		}

		delete pNode;
	}
	catch (const aq::generic_error& ge)
	{
		aq::Logger::getInstance().log(AQ_ERROR, "%s\n", ge.what());
		return EXIT_FAILURE;
	}
	catch (const std::exception& ex)
	{
		aq::Logger::getInstance().log(AQ_ERROR, "%s\n", ex.what());
		return EXIT_FAILURE;
	}
	catch (...)
	{
		aq::Logger::getInstance().log(AQ_ERROR, "unknown exception\n");
		return EXIT_FAILURE;
	}

	return EXIT_SUCCESS;
}

// -------------------------------------------------------------------------------------------------
int processSQLQueries(std::list<std::string>::const_iterator itBegin, std::list<std::string>::const_iterator itEnd, 
                      const aq::TProjectSettings& settingsBase, aq::Base& baseDesc, bool simulateAQEngine,
                      bool display, bool clean, const std::string queryIdent, bool force)
{

	unsigned int i = 1;
	std::list<std::string> queriesKO;
	for (std::list<std::string>::const_iterator it = itBegin; it != itEnd; ++it)
	{

		aq::Logger::getInstance().log(AQ_INFO, "'%s'\n", (*it).c_str());
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
			aq::Logger::getInstance().log(AQ_INFO, "Use aq engine: '%s'\n", settings.szEnginePath.c_str());
			aq_engine = new aq::AQEngineSystem(baseDesc, settings);
		}
    
		//
		// prepare and process query
		std::string answer;

		if (!((prepareQuery(*it, settingsBase, baseDesc, settings, answer, queryIdent, force) == EXIT_SUCCESS) &&
				  (processQuery(*it, settings, baseDesc, aq_engine, answer, display, clean) == EXIT_SUCCESS)))
		{
			queriesKO.push_back(*it);
		}

		boost::posix_time::ptime end(boost::posix_time::microsec_clock::local_time());
		std::ostringstream oss;
		oss << "Query Time elapsed: " << (end - begin);
		aq::Logger::getInstance().log(AQ_NOTICE, "%s\n", oss.str().c_str());
	}

	if (queriesKO.size())
	{
		aq::Logger::getInstance().log(AQ_DEBUG, "BAD QUERIES:\n");
		std::for_each(queriesKO.begin(), queriesKO.end(), [] (const std::string& q) {
			aq::Logger::getInstance().log(AQ_DEBUG, "%s\n", q.c_str());
		});
    failedQueries += queriesKO.size();
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
		unsigned int worker;
		unsigned int queryWorker;
    unsigned int tableIdToLoad;
		bool simulateAQEngine = false;
		bool multipleAnswerFiles = false;
		bool clean = false;
		bool display = false;
		bool transform = false;
		bool skipNestedQuery = false;
		bool loadDatabase = false;
    bool force = false;
    bool useColumnResolver = false;
    bool useTextAQMatrix = false;

		// old args for backward compatibility
		std::vector<std::string> oldArgs;

		po::options_description desc("Allowed options");
		desc.add_options()
			("help,h", "produce help message")
			("log-output", po::value<std::string>(&mode)->default_value("STDOUT"), "[STDOUT|LOCALFILE|SYSLOG]")
			("log-level,v", po::value<unsigned int>(&level)->default_value(AQ_LOG_NOTICE), "CRITICAL(2), ERROR(3), WARNING(4), NOTICE(5), INFO(6), DEBUG(7)")
			("log-lock", po::bool_switch(&lock_mode), "for multithread program")
			("log-date", po::bool_switch(&date_mode), "add date to log")
			("log-pid", po::bool_switch(&pid_mode), "add thread id to log")
			("log-ident", po::value<std::string>(&ident)->default_value("aq_query_resolver"), "")
			("aq-ini,s", po::value<std::string>(&propertiesFile), "")
			("query-ident", po::value<std::string>(&queryIdent), "")
      ("force", po::bool_switch(&force), "force use of directory if it already exists")
			("simulate-aq-engine,z", po::bool_switch(&simulateAQEngine), "")
			("sql-query,q", po::value<std::string>(&sqlQuery), "")
			("sql-queries-file,f", po::value<std::string>(&sqlQueriesFile), "")
			("answer-path", po::value<std::string>(&answerPathStr), "")
			("base-descr", po::value<std::string>(&baseDescr), "")
			("worker", po::value<unsigned int>(&worker)->default_value(1), "number of thread assigned to resolve the bunch of sql queries")
			("query-worker,w", po::value<unsigned int>(&queryWorker)->default_value(1), "number of thread assigned resolve one sql queries")
			("clean,c", po::bool_switch(&clean), "")
			("display,d", po::bool_switch(&display), "")
			("transform", po::bool_switch(&transform), "")
			("skip-nested-query", po::bool_switch(&skipNestedQuery), "")
			("aq-matrix", po::value<std::string>(&aqMatrixFileName), "")
			("answer-file", po::value<std::string>(&answerFileName)->default_value("answer.txt"), "")
      ("use-dll-function", po::value<std::string>(&DLLFunction), "Choise your own .dll to use your function")
			("use-column-resolver", po::bool_switch(&useColumnResolver), "")
      ("use-bin-aq-matrix", po::bool_switch(&useTextAQMatrix), "")
			("load-db", po::bool_switch(&loadDatabase), "")
      ("load-table", po::value<unsigned int>(&tableIdToLoad)->default_value(0), "")
			("backward-compatibility", po::value< std::vector<std::string> >(&oldArgs), "old arguments")
			;

		po::positional_options_description p;
		p.add("backward-compatibility", -1);

		po::variables_map vm;
		po::store(po::command_line_parser(argc, argv).options(desc).positional(p).run(), vm);
		po::notify(vm);    

		if (vm.count("help") || (argc <= 1) || ((oldArgs.size() != 0 ) && (oldArgs.size() != 2)))
		{
			std::cout << desc << "\n";
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
    // Column Resolver (old manner)
    if (useColumnResolver)
    {
      aq::Logger::getInstance().log(AQ_INFO, "use column resolver mode\n");
      settings.useRowResolver = false;
    }
    else
    {
      aq::Logger::getInstance().log(AQ_INFO, "use row resolver mode\n");
      settings.useRowResolver = true;
    }
    
    //
    // Column Binary AQ Matrix (next feature to come)
    if (!useTextAQMatrix)
    {
      aq::Logger::getInstance().log(AQ_INFO, "use binary aq matrix\n");
      settings.useBinAQMatrix = true;
    }
    else
    {
      aq::Logger::getInstance().log(AQ_INFO, "use text aq matrix\n");
      settings.useBinAQMatrix = false;
    }

		//
		// Check old args
		// std::copy(oldArgs.begin(), oldArgs.end(), std::ostream_iterator<std::string>(std::cout, " "));
		if (oldArgs.size() == 2)
		{
			propertiesFile = oldArgs[0];
			queryIdent = oldArgs[1];
		}
		
		//
		// read ini file
		if (propertiesFile != "")
		{
			aq::Logger::getInstance().log(AQ_INFO, "read %s\n", propertiesFile.c_str());
			settings.load(propertiesFile);
			settings.executeNestedQuery = !skipNestedQuery;
			if (display)
				settings.output = "stdout";
		}

		//
		// print Project Settings
		{
			std::stringstream oss;
			settings.dump(oss);
			aq::Logger::getInstance().log(AQ_DEBUG, "ProjectSettings:\n===========\n%s\n===========\n", oss.str().c_str());
		}

		//
		// Load DB Schema
		aq::Base baseDesc;
		if (baseDescr == "")
			baseDescr = settings.szDBDescFN;
		aq::Logger::getInstance().log(AQ_INFO, "load base %s\n", baseDescr.c_str());
    std::fstream bdFile(baseDescr.c_str());
    aq::base_t baseDescHolder;
    if (baseDescr.substr(baseDescr.size() - 4) == ".xml")
    {
      aq::build_base_from_xml(bdFile, baseDescHolder);
      baseDesc.loadFromBaseDesc(baseDescHolder);
    }
    else
    {
      // aq::build_base_from_raw(baseDescr.c_str(), baseDescHolder);
      baseDesc.loadFromRawFile(baseDescr.c_str());
    }
    // baseDesc.loadFromBaseDesc(baseDescHolder);
		
		//
		// If Load database is invoked
		if (loadDatabase)
		{
			for (size_t t = 0; t < baseDesc.getTables().size(); ++t)
			{
        if ((tableIdToLoad != 0) && (tableIdToLoad != baseDesc.getTables()[t]->ID))
        {
          continue;
        }
				for (size_t c = 0; c < baseDesc.getTables()[t]->Columns.size(); ++c)
				{
					aq::Logger::getInstance().log(AQ_INFO, "loading column %d of table %d\n", c + 1, t + 1);
					cut_in_col(propertiesFile.c_str(), t + 1, c + 1);
				}
			}
			return EXIT_SUCCESS;
		}

		//
		// keep backward compatibility
		if (oldArgs.size() == 2)
		{
			//
			// Log file
			 std::string logFilename = settings.szRootPath + "/calculus/" + queryIdent + "/aq_query_resolver.log";
			 aq::Logger::getInstance().setLocalFile(logFilename.c_str());
			 aq::Logger::getInstance().setLevel(AQ_LOG_DEBUG);

			 std::cout << "log in " << logFilename << std::endl;

			//
			// read sql query
			settings.changeIdent(queryIdent);
			settings.computeAnswer = true;
			std::ifstream queryFile(settings.szSQLReqFN);
			std::string query;
			std::string line;
			while (std::getline(queryFile, line))
			{
				query += " ";
				std::size_t pos = line.find("--");
				if (pos != std::string::npos)
					line = line.erase(pos);
				boost::algorithm::trim(line);
				if (((char)line[0] == (char)0xEF) && ((char)line[1] == (char)0xBB) && ((char)line[2] == (char)0xBF))
					query += line.substr(3);
				else
					query += line;
			}

			//
			// write ini file (it is needed for now by AQEngine)
			std::ofstream iniFile(settings.iniFile.c_str());
			settings.writeAQEngineIni(iniFile);
			iniFile.close();

			//
			// process
			std::string answer;
      aq::AQEngine_Intf * aq_engine = new aq::AQEngineSystem(baseDesc, settings);
			processQuery(query, settings, baseDesc, aq_engine, answer, false, false);

		}
		else // new version
		{

			//
			// load sql queries
			std::list<std::string> queries;
			if (sqlQueriesFile != "")
			{
				std::string line;
				std::ifstream fin(sqlQueriesFile.c_str(), std::ifstream::in);
				std::string currentQuery = "";
				while (std::getline(fin, line))
				{
					boost::algorithm::trim(line);
					std::string::size_type pos = line.find("--");
					if (pos != std::string::npos)
						line.erase(pos);
					if ((line == ""))
						continue;
					currentQuery += " " + line;
					pos = line.find(";");
					if (pos != std::string::npos)
					{
            boost::trim(currentQuery);
            if (currentQuery == "exit;") 
              break;
						queries.push_back(currentQuery);
						currentQuery = "";
					}
				}
			}
			else if (sqlQuery != "")
			{
				queries.push_back(sqlQuery);
			}
			else
			{
				aq::Logger::getInstance().log(AQ_ERROR, "you need to specify a sql query\n");
			}

			if (transform)
			{
				std::for_each(queries.begin(), queries.end(), [&] (const std::string& query) {
					transformQuery(query, settings, baseDesc);
				});
				return 0;
			}

			if (aqMatrixFileName != "")
			{
				std::for_each(queries.begin(), queries.end(), [&] (const std::string& query) {
					processAQMatrix(query, aqMatrixFileName, answerFileName, settings, baseDesc);
				});
				return 0;
			}

			//
			// distribute bunch of sql queries to each thread fo pool
			unsigned int i = 1;
			boost::posix_time::ptime fullBegin(boost::posix_time::microsec_clock::local_time());
			boost::thread_group grp;
			std::list<std::string>::const_iterator itBegin = queries.begin();
			std::list<std::string>::const_iterator itEnd = queries.begin();
			while (itBegin != queries.end())
			{
				// std::advance(itEnd, queries.size() / worker); // fixme: doesn't work under visual !!!
				for (unsigned int i = 0; (i < queries.size() / worker) && (itEnd != queries.end()); i++) ++itEnd;
				grp.create_thread(boost::bind(processSQLQueries, itBegin, itEnd, settings, baseDesc, simulateAQEngine, display, clean, queryIdent, force));
				itBegin = itEnd;
			}
			grp.join_all();
			boost::posix_time::ptime fullEnd(boost::posix_time::microsec_clock::local_time());
			std::ostringstream oss;
			oss << "Full Time elapsed: " << (fullEnd - fullBegin);
			aq::Logger::getInstance().log(AQ_NOTICE, "%s\n", oss.str().c_str());

		}
	}
	catch (const aq::generic_error& error)
	{
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
