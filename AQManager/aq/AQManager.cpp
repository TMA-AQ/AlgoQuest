#if defined(WIN32)
# include "stdafx.h"
# include <codecvt>
#endif

#include <aq/Exceptions.h>
#include <aq/Logger.h>
#include <aq/Base.h>
#include <aq/Database.h>
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
#include <boost/thread.hpp>
#include <boost/filesystem.hpp>
#include <boost/property_tree/ptree.hpp>
#include <boost/property_tree/ini_parser.hpp>
#include <boost/algorithm/string/trim.hpp>
#include <boost/algorithm/string/replace.hpp>
#include <boost/date_time/posix_time/posix_time.hpp>
#include <boost/uuid/uuid.hpp>
#include <boost/uuid/uuid_generators.hpp>
#include <boost/uuid/uuid_io.hpp>

namespace fs = boost::filesystem;

// -------------------------------------------------------------------------------------------------
extern int yylineno;
int yyerror( const char *pszMsg ) 
{
	std::cerr << "SQL Parsing Error : " << pszMsg << " encountered at line number: " << yylineno << std::endl;
	return 0;
}

size_t failedQueries = 0;
boost::mutex parserMutex;

// -------------------------------------------------------------------------------------------------
int prepareQuery(const std::string& query, const aq::Settings& settingsBase, aq::Base& baseDesc, aq::Settings& settings, 
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
	lpaths.push_back(fs::path(settings.workingPath));
	lpaths.push_back(fs::path(settings.tmpPath));
	lpaths.push_back(fs::path(settings.dpyPath));
	for (std::list<fs::path>::const_iterator dir = lpaths.begin(); dir != lpaths.end(); ++dir)
	{
		if (fs::exists(*dir))
		{
			aq::Logger::getInstance().log(AQ_WARNING, "directory already exist '%s'\n", (*dir).c_str());
      if (!force)
      {
        return EXIT_FAILURE;
      }
		}
		else if (!fs::create_directory(*dir))
		{
			aq::Logger::getInstance().log(AQ_WARNING, "cannot create directory '%s'\n", (*dir).c_str());
      if (!force)
      {
        return EXIT_FAILURE;
      }
		}
	}

  //
  // write request file
  std::string queryFilename(settings.workingPath + "/Request.sql");
  std::ofstream queryFile(queryFilename.c_str());
  queryFile << query;
  queryFile.close();

	//
	// write ini file (it is needed for now by AQEngine)
	std::ofstream iniFile(settings.iniFile.c_str());
	settings.writeAQEngineIni(iniFile);
	iniFile.close();

  //
	// generate answer file
	aq::Logger::getInstance().log(AQ_INFO, "save answer to %s\n", settings.answerFile.c_str());

	return EXIT_SUCCESS;
}

// -------------------------------------------------------------------------------------------------
int processQuery(const std::string& query, aq::Settings& settings, aq::Base& baseDesc, aq::AQEngine_Intf * aq_engine,
                 const std::string& answer, bool clean)
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

		//
		// Transform SQL request in prefix form, 
    unsigned int id = 1;
		aq::QueryResolver queryResolver(pNode, &settings, aq_engine, baseDesc, id);
    queryResolver.solve();

    aq::Table::Ptr result = queryResolver.getResult();

		if (clean)
		{
			aq::Logger::getInstance().log(AQ_NOTICE, "remove working directory '%s'\n", settings.workingPath.c_str());
			aq::DeleteFolder(settings.workingPath.c_str());
			aq::Logger::getInstance().log(AQ_NOTICE, "remove tmp working directory '%s'\n", settings.tmpPath.c_str());
			aq::DeleteFolder(settings.tmpPath.c_str());
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
                      const aq::Settings& settingsBase, aq::Base& baseDesc,
                      const std::string queryIdent, bool clean, bool force)
{

  aq::Logger::getInstance().log(AQ_NOTICE, "'%s'\n", query.c_str());
  boost::posix_time::ptime begin(boost::posix_time::microsec_clock::local_time());

  //
  // prepare and process query
  std::string answer;
  aq::Settings settings(settingsBase);

  //
  // Load AQ engine
  aq::AQEngine_Intf * aq_engine = new aq::AQEngineSystem(baseDesc, settings);
    
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
  
int get_db_list_on_path(const char * path, std::vector<const char *>& db_list)
{
  boost::filesystem::path p(path);
  if (boost::filesystem::exists(p) && boost::filesystem::is_directory(p))
  {
    boost::filesystem::directory_iterator end_it;
    for (boost::filesystem::directory_iterator it(p); it != end_it; ++it)
    {
      if (boost::filesystem::is_directory(it->status()))
      {
        aq::Database db(it->path().string());
        if (db.isValid())
        {
          db_list.push_back(db.getName().c_str());
        }
      }
    }
  }
  return EXIT_SUCCESS;
}
  
int get_db_list(std::vector<const char *>& db_list)
{
  char * s = ::getenv("AQ_HOME");
  if (s == NULL)
  {
    aq::Logger::getInstance().log(AQ_ERROR, "AQ_HOME environment variable is not set");
  }
  return get_db_list_on_path(s, db_list);
}

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
		aq::Settings settings;

		// log options
		bool lock_mode = false;
		bool date_mode = false;
		bool pid_mode = false;
		
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
		aq::Logger::getInstance().log(AQ_INFO, "load base %s\n", settings.aqName);
    aq::base_t baseDescHolder;
    std::fstream bdFile(settings.dbDesc);
    if (settings.dbDesc.substr(settings.dbDesc.size() - 4) == ".xml")
    {
      aq::build_base_from_xml(bdFile, baseDescHolder);
    }
    else
    {
      aq::build_base_from_xml(bdFile, baseDescHolder);
    }
    baseDesc.loadFromBaseDesc(baseDescHolder);
		
		//
		// Load AQ engine
		aq::Logger::getInstance().log(AQ_INFO, "Use aq engine: '%s'\n", settings.aqEngine.c_str());
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
  aq::Settings settings;
  if (strcmp(propertiesFile, "") == 0)
  {
    aq::Logger::getInstance().log(AQ_INFO, "no properties file");
    return -1;
  }

  aq::Logger::getInstance().log(AQ_INFO, "read %s\n", propertiesFile);
  settings.load(propertiesFile);
  
  //
  // Load DB Schema
  std::string baseDescr_filename = settings.dbDesc;
  aq::Logger::getInstance().log(AQ_INFO, "load base %s\n", baseDescr_filename.c_str());
  std::fstream bdFile(baseDescr_filename.c_str());
  aq::base_t bd;
  if (baseDescr_filename.substr(baseDescr_filename.size() - 4) == ".xml")
  {
    aq::build_base_from_xml(bdFile, bd);
  }
  else
  {
    aq::build_base_from_raw(bdFile, bd);
  }

  //
  // load base
  for (size_t t = 0; t < bd.table.size(); ++t)
  {
    if ((tableId != 0) && (tableId != bd.table[t].num))
    {
      continue;
    }
    aq::DatabaseLoader loader(bd, settings.aqLoader, settings.rootPath, settings.packSize, ','/*settings.fieldSeparator*/, settings.csvFormat); // FIXME
    for (size_t c = 0; c < bd.table[t].colonne.size(); ++c)
    {
      aq::Logger::getInstance().log(AQ_INFO, "loading column %d of table %d\n", c + 1, t + 1);
      loader.run(t + 1, c + 1);
    }
  }
  return EXIT_SUCCESS;
}

const char * aql2sql(const char * aql_query)
{
  return "TODO";
}

}
