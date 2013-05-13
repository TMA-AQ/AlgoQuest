#include "stdafx.h"

#include <aq/Exceptions.h>
#include <aq/BaseDesc.h>
#include <aq/Logger.h>
#include <aq/AQEngine.h>
#include <aq/SQLPrefix.h>
#include <aq/Column2Table.h>
#include <aq/QueryResolver.h>
#include <aq/parser/SQLParser.h>
#include <aq/parser/JeqParser.h>
#include <aq/db_loader/DatabaseLoader.h>
#include <iostream>
#include <list>
#include <fstream>
#include <string>
#include <codecvt>
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

size_t failedQueries = 0;
boost::mutex parserMutex;

// -------------------------------------------------------------------------------------------------
int prepareQuery(const std::string& query, const TProjectSettings& settingsBase, Base& baseDesc, TProjectSettings& settings, 
                 std::string& displayFile, const std::string queryIdentStr, bool force)
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
		else if (!fs::create_directory(*dir))
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
int processQuery(const std::string& query, TProjectSettings& settings, Base& baseDesc, AQEngine_Intf * aq_engine,
                 const std::string& answer, bool clean)
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
int processSQLQueries(const std::string query, 
                      const TProjectSettings& settingsBase, Base& baseDesc,
                      const std::string queryIdent, bool clean, bool force)
{

  aq::Logger::getInstance().log(AQ_NOTICE, "'%s'\n", query.c_str());
  boost::posix_time::ptime begin(boost::posix_time::microsec_clock::local_time());

  //
  // prepare and process query
  std::string answer;
  TProjectSettings settings(settingsBase);

  //
  // Load AQ engine
  AQEngine_Intf * aq_engine = new AQEngineSystem(baseDesc, settings);
    
  if (!((prepareQuery(query, settingsBase, baseDesc, settings, answer, queryIdent, force) == EXIT_SUCCESS) &&
    (processQuery(query, settings, baseDesc, aq_engine, answer, clean) == EXIT_SUCCESS)))
  {
    return EXIT_FAILURE;
  }

  boost::posix_time::ptime end(boost::posix_time::microsec_clock::local_time());
  std::ostringstream oss;
  oss << "Query Time elapsed: " << (end - begin);
  aq::Logger::getInstance().log(AQ_NOTICE, "%s\n", oss.str().c_str());

	return EXIT_SUCCESS;
}

// ------------------------------------------------------------------------------------------------
extern "C"
{

// -------------------------------------------------------------------------------------------------
int solve_query(const char * _query, const char * _iniFilename, const char * _workingDirectory, 
                const char * _logIdent, const char * _logMode, unsigned int logLevel,
                bool clean, bool force)
{

  // verb_register(); // FIXME

	try
  {
    std::string query(_query); 
    std::string iniFilename(_iniFilename); 
    std::string workingDirectory(_workingDirectory); 
    std::string logIdent(_logIdent); 
    std::string logMode(_logMode); 

		// Settings
		TProjectSettings settings;

		// log options
		bool lock_mode = false;
		bool date_mode = false;
		bool pid_mode = false;

		// aq options
		bool skipNestedQuery = false;

		//
		// Initialize Logger
		aq::Logger::getInstance(logIdent.c_str(), logMode == "STDOUT" ? STDOUT : logMode == "LOCALFILE" ? LOCALFILE : logMode == "SYSLOG" ? SYSLOG : STDOUT);
		aq::Logger::getInstance().setLevel(logLevel);
		aq::Logger::getInstance().setLockMode(lock_mode);
		aq::Logger::getInstance().setDateMode(date_mode);
		aq::Logger::getInstance().setPidMode(pid_mode);

		//
		// read ini file
		if (iniFilename != "")
		{
			aq::Logger::getInstance().log(AQ_INFO, "read %s\n", iniFilename.c_str());
			settings.load(iniFilename);
			settings.executeNestedQuery = !skipNestedQuery;
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
		Base baseDesc;
		aq::Logger::getInstance().log(AQ_INFO, "load base %s\n", settings.szDBDescFN);
    aq::base_t baseDescHolder;
    if (strncmp(settings.szDBDescFN + strlen(settings.szDBDescFN) - 4, ".xml", 4) == 0)
    {
      std::fstream bdFile(settings.szDBDescFN);
      aq::build_base_from_xml(bdFile, baseDescHolder);
    }
    else
    {
      aq::build_base_from_raw(settings.szDBDescFN, baseDescHolder);
      // baseDesc.loadFromRawFile(settings.szDBDescFN);
    }
    baseDesc.loadFromBaseDesc(baseDescHolder);
		
		//
		// Load AQ engine
		aq::Logger::getInstance().log(AQ_INFO, "Use aq engine: '%s'\n", settings.szEnginePath.c_str());
		processSQLQueries(query, settings, baseDesc,  workingDirectory, clean, force);
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

int load_db(const char * propertiesFile, unsigned int tableId)
{					
  
  //
  // read ini file
  TProjectSettings settings;
  if (propertiesFile != "")
  {
    aq::Logger::getInstance().log(AQ_INFO, "read %s\n", propertiesFile);
    settings.load(propertiesFile);
  }

  //
  // Load DB Schema
  Base baseDesc;
  std::string baseDescr_filename = settings.szDBDescFN;
  aq::Logger::getInstance().log(AQ_INFO, "load base %s\n", baseDescr_filename.c_str());
  std::fstream bdFile(baseDescr_filename.c_str());
  aq::base_t baseDescHolder;
  if (baseDescr_filename.substr(baseDescr_filename.size() - 4) == ".xml")
  {
    aq::build_base_from_xml(bdFile, baseDescHolder);
    baseDesc.loadFromBaseDesc(baseDescHolder);
  }
  else
  {
    baseDesc.loadFromRawFile(baseDescr_filename.c_str());
  }

  //
  // load base
  for (size_t t = 0; t < baseDesc.Tables.size(); ++t)
  {
    if ((tableId != 0) && (tableId != baseDesc.Tables[t].ID))
    {
      continue;
    }
    for (size_t c = 0; c < baseDesc.Tables[t].Columns.size(); ++c)
    {
      aq::Logger::getInstance().log(AQ_INFO, "loading column %d of table %d\n", c + 1, t + 1);
      cut_in_col(propertiesFile, t + 1, c + 1);
    }
  }
  return EXIT_SUCCESS;
}

int test_aq_lib(const char * query)
{
  return boost::lexical_cast<int>(query);
}

}
