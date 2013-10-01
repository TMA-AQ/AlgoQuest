#include "Util.h"
#include "QueryReader.h"
#include "WhereValidator.h"
#include <aq/AQLParser.h>
#include <aq/Base.h>
#include <aq/Timer.h>
#include <aq/Exceptions.h>
#include <iostream>
#include <fstream>
#include <boost/filesystem.hpp>
#include <boost/program_options.hpp>
#include <boost/algorithm/string.hpp>

namespace aq
{

uint64_t functional_tests(const struct opt& o)
{
  int rc = 0;
  uint64_t nb_queries_tested = 0;
  uint64_t nb_success = 0;
  uint64_t nb_errors = 0;
  aq::Timer timer;

  std::fstream log(o.logFilename.c_str(), std::ifstream::out);

  boost::filesystem::path p(o.queriesFilename);
  if (!boost::filesystem::exists(p)) {
    std::cerr << "cannot find file " << p << std::endl;
    return -1;
  }

  // generate working directories and aqengine ini
  std::fstream fqueries(o.queriesFilename.c_str(), std::ifstream::in);
  aq::QueryReader reader(fqueries);
  std::string query;
  while ((query = reader.next()) != "")
  {
    if (o.filter != "")
    {
      std::string::size_type pos = o.filter.find("/");
      if (pos != std::string::npos)
      {
        if ((reader.getSuite() + std::string("/") + reader.getIdent()) != o.filter) 
        {
          continue;
        }
      }
      else if (reader.getSuite() != o.filter)
      {
        continue;
      }
    }

    // parse query
    aq::core::SelectStatement ss;
    if (o.aql2sql)
    {
      aq::parser::parse(query, ss);
      ss.output = aq::core::SelectStatement::SQL;
      std::cout << ss << std::endl;
    }

    // check for select
    std::vector<std::string> selectedColumns;
    aq::get_columns(selectedColumns, query, "SELECT");

    // check for group
    std::vector<std::string> groupedColumns;
    aq::get_columns(groupedColumns, query, "GROUP");
    
    // check for order
    std::vector<std::string> orderedColumns;
    aq::get_columns(orderedColumns, query, "ORDER");

    // execute query
    if (o.execute || o.checkCondition || o.checkResult || o.display)
    {
      std::string iniFilename;
      aq::generate_working_directories(o, iniFilename);

      // for each query read in queries file
      std::ofstream queryFile(std::string(o.dbPath + "calculus/" + o.queryIdent + "/New_Request.txt").c_str());
      queryFile << query ;
      queryFile.close();

      // check for Whering
      aq::WhereValidator whereValidator;
      if (o.checkCondition)
      {
        whereValidator.addJoinConditions(ss.joinConditions);
        whereValidator.addInConditions(ss.inConditions);
        // whereValidator.parseQuery(query);
        // whereValidator.dump(std::cout);
      }

      aq::Logger::getInstance().log(AQ_LOG_INFO, "checking '%s'\n", reader.getFullIdent().c_str());
      aq::Logger::getInstance().log(AQ_LOG_INFO, "expecting: %s\n", reader.getExpected());
      aq::TProjectSettings settings;
      aq::Base baseDesc;
      aq::AQMatrix matrix(settings, baseDesc);

      // run aq engine
      timer.start();
      rc = aq::run_aq_engine(o.aqEngine, iniFilename, o.queryIdent);
      std::string aq_engine_time_elapsed = aq::Timer::getString(timer.getTimeElapsed());
      aq::Logger::getInstance().log(AQ_LOG_INFO, "aq engine performed on '%s' in %s\n", reader.getFullIdent().c_str(), aq_engine_time_elapsed.c_str());

      if (rc == 0 && (o.checkResult || o.display))
      {
        // check answer validity
        if (o.checkResult)
        {
          uint64_t count = reader.extract_value<uint64_t>("count", 0);
          uint64_t nbRows = reader.extract_value<uint64_t>("rows", 0);
          uint64_t nbGroups = reader.extract_value<uint64_t>("groups", 0);
          rc = aq::check_answer_validity(o, matrix, count, nbRows, nbGroups);
        }

        // check data validity
        if (o.checkResult)
        {
          std::string answerPath(o.dbPath);
          answerPath += "/data_orga/tmp/" + std::string(o.queryIdent) + "/dpy/";
          rc = aq::check_answer_data(std::cout, answerPath, o, selectedColumns, groupedColumns, orderedColumns, whereValidator);
        }
        else
        {
          std::string answerPath(o.dbPath);
          answerPath += "/data_orga/tmp/" + std::string(o.queryIdent) + "/dpy/";
          rc = aq::display(std::cout, answerPath, o, selectedColumns);
        }
      }
      
      ++nb_queries_tested;

      // clean tmp directory
      if (rc == 0)
      {
        log << "=== aq engine successfull ===" << std::endl;
        log << "query '" << reader.getFullIdent() << "' :" << std::endl;
        log << query << std::endl;
        log << aq_engine_time_elapsed << std::endl;
        boost::filesystem::path tmpPath(o.dbPath + "data_orga/tmp/" + o.queryIdent + "/");
        boost::filesystem::remove_all(tmpPath);
        ++nb_success;
      }
      else
      {
        log << "=== aq engine error ===" << std::endl;
        log << "query '" << reader.getFullIdent() << "' :" << std::endl;
        log << query << std::endl;
        log << aq::Timer::getString(timer.getTimeElapsed()) << std::endl;
        aq::Logger::getInstance().log(AQ_ERROR, "aq engine error on '%s'\n", reader.getFullIdent().c_str());
        aq::Logger::getInstance().log(AQ_ERROR, "%s\n", query.c_str());
        ++nb_errors;
        if (o.stopOnError)
          break;
        else
        {
          boost::filesystem::path tmpPath(o.dbPath + "data_orga/tmp/" + o.queryIdent + "/");
          boost::filesystem::remove_all(tmpPath);
        }

      }
      
    }
    
  }

  aq::Logger::getInstance().log(AQ_LOG_INFO, "%u queries tested: [ success : %u ; failed : %u ; \n", nb_queries_tested, nb_success, nb_errors);

  log.close();

  return nb_errors;
}

}
