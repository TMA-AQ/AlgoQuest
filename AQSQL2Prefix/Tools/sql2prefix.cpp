#include <SQLParser/AQEngine.h>
#include <SQLParser/SQLParser.h>
#include <SQLParser/SQLPrefix.h>
#include <SQLParser/Column2Table.h>
#include <SQLParser/NestedQueries.h>
#include <SQLParser/JeqParser.h>
#include <SQLParser/Exceptions.h>
#include <DBLoader/DatabaseLoader.h>
#include <iostream>
#include <list>
#include <fstream>
#include <string>
#include <codecvt>
#include <aq/Logger.h>
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

boost::mutex parserMutex;

// -------------------------------------------------------------------------------------------------
class AQEngineSimulate : public AQEngine_Intf
{
public:
	void call(TProjectSettings& settings, tnode * pNode, int mode, int selectLevel) {

		//
		// Get prefix form of query.
		std::string str;
		syntax_tree_to_prefix_form(pNode, str);

		aq::Logger::getInstance().log(AQ_INFO, "---");
		aq::Logger::getInstance().log(AQ_INFO, "Get prefix form of query");
		aq::Logger::getInstance().log(AQ_INFO, str.c_str());

		//
		// Process with parse jeq
		ParseJeq(str);

		aq::Logger::getInstance().log(AQ_INFO, "---");
		aq::Logger::getInstance().log(AQ_INFO, "Get prefix form of query after jeq parser");
		aq::Logger::getInstance().log(AQ_INFO, str.c_str());

	}
	
	void setAQMatrix(boost::shared_ptr<aq::AQMatrix> _aqMatrix)
	{
		aqMatrix = _aqMatrix;
	}
	
	void setTablesIDs(std::vector<llong>& _tableIDs)
	{
		tableIDs.resize(_tableIDs.size());
		std::copy(_tableIDs.begin(), _tableIDs.end(), tableIDs.begin());
	}

	boost::shared_ptr<aq::AQMatrix> getAQMatrix()
	{
		return aqMatrix;
	}

	const std::vector<llong>& getTablesIDs() const
	{
		return tableIDs;
	}

private:
	boost::shared_ptr<aq::AQMatrix> aqMatrix;
	std::vector<llong> tableIDs;
};

// -------------------------------------------------------------------------------------------------
int processAQMatrix(const std::string& query, const std::string& aqMatrixFileName, const std::string& answerFileName, TProjectSettings& settings, Base& baseDesc)
{
	tnode	*pNode;
	int	nRet;

	std::cout << query << std::endl << std::endl;
	
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
	
	VerbNode::Ptr spTree = QueryResolver::BuildVerbsTree(pNode, baseDesc, &settings );
	spTree->changeQuery();
	QueryResolver::cleanQuery( pNode );

	strcpy(settings.szAnswerFN, aqMatrixFileName.c_str());

	boost::shared_ptr<aq::AQMatrix> aqMatrix(new aq::AQMatrix);
	std::vector<llong> tableIDs;
	
	aq::Timer timer;
	std::vector<std::string> answerFile;
	answerFile.push_back(aqMatrixFileName);
	for (std::vector<std::string>::const_iterator it = answerFile.begin(); it != answerFile.end(); ++it)
	{
		// aqMatrix->load((*it).c_str(), settings.fieldSeparator, tableIDs);
	}
	aqMatrix->simulate(113867938, 2);
	// aqMatrix->simulate(10, 2);
	tableIDs.push_back(8);
	aq::Logger::getInstance().log(AQ_INFO, "Load AQ Matrix: Elapsed Time = %s\n", aq::Timer::getString(timer.getTimeElapsed()).c_str());

	AQEngineSimulate aqEngine;
	aqEngine.setAQMatrix(aqMatrix);
	aqEngine.setTablesIDs(tableIDs);

	QueryResolver queryResolver(pNode, &settings, &aqEngine, baseDesc);
	if (settings.useRowResolver)
		queryResolver.solveAQMatriceByRows(spTree);
	else
		queryResolver.solveAQMatriceByColumns(spTree);
		

	Table::Ptr result = queryResolver.getResult();
	if (result)
	{
		result->saveToAnswer(answerFileName.c_str(), settings.fieldSeparator);
	}

	return 0;
}

// -------------------------------------------------------------------------------------------------
int transformQuery(
	const std::string& query,
	TProjectSettings& settings, 
	Base& baseDesc)
{
	tnode	*pNode;
	int	nRet;

	std::cout << query << std::endl << std::endl;

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

	std::cout << *pNode << std::endl;

	VerbNode::Ptr spTree = QueryResolver::BuildVerbsTree(pNode, baseDesc, &settings );
	spTree->changeQuery();
	QueryResolver::cleanQuery( pNode );
	std::cout << std::endl;
	std::cout << *pNode << std::endl << std::endl;
	
	std::string str;
	syntax_tree_to_prefix_form(pNode, str);
	ParseJeq( str );

	std::cout << str << std::endl << std::endl;

	return 0;
}

// -------------------------------------------------------------------------------------------------
int prepareQuery(
	const TProjectSettings& settingsBase, 
	Base& baseDesc,
	TProjectSettings& settings, 
	std::string& displayFile)
{		
	//
	// generate ident and ini file
	std::string queryIdentStr = "";
	boost::uuids::uuid queryIdent = boost::uuids::random_generator()();
	std::ostringstream queryIdentOSS;
	queryIdentOSS << queryIdent;
	queryIdentStr = queryIdentOSS.str();
	settings.changeIdent(queryIdentOSS.str());

	//
	// create directories
	std::list<fs::path> lpaths;
	lpaths.push_back(fs::path(settings.szRootPath + "/calculus/" + queryIdentOSS.str()));
	lpaths.push_back(fs::path(settings.szTempPath1));
	lpaths.push_back(fs::path(settings.szTempPath2));
	for (std::list<fs::path>::const_iterator dir = lpaths.begin(); dir != lpaths.end(); ++dir)
	{
		if (fs::exists(*dir))
		{
			aq::Logger::getInstance().log(AQ_ERROR, "directory already exist '%s'", (*dir).c_str());
			return EXIT_FAILURE;
		}

		if (!fs::create_directory(*dir))
		{
			aq::Logger::getInstance().log(AQ_ERROR, "cannot create directory '%s'", (*dir).c_str());
			return EXIT_FAILURE;
		}
	}

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
int processQuery( 
	const std::string& query,
	TProjectSettings& settings, 
	Base& baseDesc,
	AQEngine_Intf * aq_engine,
	const std::string& answer,
	bool display,
	bool clean)
{
	try
	{
	
		aq::Logger::getInstance().log(AQ_INFO, "processing sql query\n");

		tnode	*pNode  = NULL;
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

		//
		// Transform SQL request in prefix form, 
		QueryResolver queryResolver(pNode, &settings, aq_engine, baseDesc);
		if( (nRet = queryResolver.SolveSQLStatement()) != 0 )
		{
			aq::Logger::getInstance().log(AQ_ERROR, "error resolving sql query\n");
			return EXIT_FAILURE;
		}

		Table::Ptr result = queryResolver.getResult();
		if (result)
		{
			aq::Timer timer;
			result->saveToAnswer(settings.szAnswerFN, settings.fieldSeparator);
			aq::Logger::getInstance().log(AQ_INFO, "Save Answer: Time Elapsed = %s\n", aq::Timer::getString(timer.getTimeElapsed()).c_str());
		}

		if (display)
		{
			std::ifstream fin(answer.c_str());
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
			DeleteFolder(workingDirectory.c_str());
		}

		delete pNode;
	}
	catch (const generic_error& ge)
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
int processSQLQueries(
	std::list<std::string>::const_iterator itBegin, 
	std::list<std::string>::const_iterator itEnd, 
	const TProjectSettings& settingsBase, 
	Base& baseDesc,
	AQEngine_Intf * aq_engine,
	bool display,
	bool clean)
{

	unsigned int i = 1;
	std::list<std::string> queriesKO;
	for (std::list<std::string>::const_iterator it = itBegin; it != itEnd; ++it)
	{

		aq::Logger::getInstance().log(AQ_NOTICE, "'%s'\n", (*it).c_str());
		boost::posix_time::ptime begin(boost::posix_time::microsec_clock::local_time());

		//
		// prepare and process query
		std::string answer;
		TProjectSettings settings(settingsBase);
		if (!((prepareQuery(settingsBase, baseDesc, settings, answer) == EXIT_SUCCESS) &&
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
		aq::Logger::getInstance().log(AQ_ERROR, "BAD QUERIES:\n");
		std::for_each(queriesKO.begin(), queriesKO.end(), [] (const std::string& q) {
			aq::Logger::getInstance().log(AQ_ERROR, "%s\n", q.c_str());
		});
	}

	return EXIT_SUCCESS;
}

// -------------------------------------------------------------------------------------------------
int main(int argc, char**argv)
{
	
	try
	{

		// Settings
		TProjectSettings settings;

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
		unsigned int worker;
		unsigned int queryWorker;
		bool simulateAQEngine = false;
		bool multipleAnswerFiles = false;
		bool clean = false;
		bool display = false;
		bool transform = false;
		bool skipNestedQuery = false;
		bool loadDatabase = false;

		// old args for backward compatibility
		std::vector<std::string> oldArgs;

		po::options_description desc("Allowed options");
		desc.add_options()
			("help", "produce help message")
			("log-output", po::value<std::string>(&mode)->default_value("STDOUT"), "[STDOUT|LOCALFILE|SYSLOG]")
			("log-level", po::value<unsigned int>(&level)->default_value(LOG_NOTICE), "CRITICAL(2), ERROR(3), WARNING(4), NOTICE(5), INFO(6), DEBUG(7)")
			("log-lock", po::bool_switch(&lock_mode), "for multithread program")
			("log-date", po::bool_switch(&date_mode), "add date to log")
			("log-pid", po::bool_switch(&pid_mode), "add thread id to log")
			("log-ident", po::value<std::string>(&ident)->default_value("sql2prefix.log"), "")
			("aq-ini", po::value<std::string>(&propertiesFile), "")
			("query-ident", po::value<std::string>(&queryIdent), "")
			("simulate-aq-engine", po::bool_switch(&simulateAQEngine), "")
			("sql-query", po::value<std::string>(&sqlQuery), "")
			("sql-queries-file", po::value<std::string>(&sqlQueriesFile), "")
			("answer-path", po::value<std::string>(&answerPathStr), "")
			("base-descr", po::value<std::string>(&baseDescr), "")
			("worker", po::value<unsigned int>(&worker)->default_value(1), "number of thread assigned to resolve the bunch of sql queries")
			("query-worker", po::value<unsigned int>(&queryWorker)->default_value(1), "number of thread assigned resolve one sql queries")
			("clean", po::bool_switch(&clean), "")
			("display", po::bool_switch(&display), "")
			("transform", po::bool_switch(&transform), "")
			("skip-nested-query", po::bool_switch(&skipNestedQuery), "")
			("aq-matrix", po::value<std::string>(&aqMatrixFileName), "")
			("answer-file", po::value<std::string>(&answerFileName)->default_value("answer.txt"), "")
			("use-row-resolver", po::bool_switch(&settings.useRowResolver), "")
			("load-db", po::bool_switch(&loadDatabase), "")
			/*
			("root-path", po::value<std::string>(&Settings.szRootPath), "old root.folder in properties.ini")
			("engine-path", po::value<char *>(Settings.szEnginePath), "old exeTest.folder in properties.ini")
			("tmp-path", po::value<char *>(Settings.szTempRootPath), "old k_rep_racine_tmp in properties.ini")
			("field-separator", po::value<char>(&Settings.fieldSeparator), "old step1.field.separator in properties.ini")
			("cut-int-col", po::value<char *>(Settings.szCutInColPath), "old batch.command.1 in properties.ini")
			("loader", po::value<char *>(Settings.szLoaderPath), "old batch.command.2 in properties.ini")
			*/
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
		if (baseDescr == "")
			baseDescr = settings.szDBDescFN;
		aq::Logger::getInstance().log(AQ_INFO, "load base %s\n", baseDescr.c_str());
		Base baseDesc;
		baseDesc.loadFromBaseDesc(baseDescr.c_str());
		
		//
		// If Load database is invoked
		if (loadDatabase)
		{
			for (size_t t = 0; t < baseDesc.Tables.size(); ++t)
			{
				for (size_t c = 0; c < baseDesc.Tables[t].Columns.size(); ++c)
				{
					aq::Logger::getInstance().log(AQ_INFO, "loading column %d of table %d", c + 1, t + 1);
					cut_in_col(propertiesFile.c_str(), t + 1, c + 1);
				}
			}
			return EXIT_SUCCESS;
		}

		//
		// Load AQ engine
		AQEngine_Intf * aq_engine;
		if (simulateAQEngine)
		{
			aq::Logger::getInstance().log(AQ_INFO, "Do not use aq engine\n");
			aq_engine = new AQEngineSimulate();
		}
		else
		{
			aq::Logger::getInstance().log(AQ_INFO, "Use aq engine: '%s'\n", settings.szEnginePath.c_str());
			aq_engine = new AQEngine(baseDesc);
		}

		//
		// keep backward compatibility
		if (oldArgs.size() == 2)
		{
			//
			// Log file
			 std::string logFilename = settings.szRootPath + "/calculus/" + queryIdent + "/sql2prefix.log";
			 aq::Logger::getInstance().setLocalFile(logFilename.c_str());
			 aq::Logger::getInstance().setLevel(LOG_DEBUG);

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
				std::ifstream fin(sqlQueriesFile, std::ifstream::in);
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
				grp.create_thread(boost::bind(processSQLQueries, itBegin, itEnd, settings, baseDesc, aq_engine, display, clean));
				itBegin = itEnd;
			}
			grp.join_all();
			boost::posix_time::ptime fullEnd(boost::posix_time::microsec_clock::local_time());
			std::ostringstream oss;
			oss << "Full Time elapsed: " << (fullEnd - fullBegin);
			aq::Logger::getInstance().log(AQ_NOTICE, "%s\n", oss.str().c_str());

		}
	}
	catch (const generic_error& error)
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

	return EXIT_SUCCESS;
}
