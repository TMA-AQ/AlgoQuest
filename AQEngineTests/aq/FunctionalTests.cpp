#include "Util.h"
#include "QueryReader.h"
#include "WhereValidator.h"
#include <aq/Base.h>
#include <aq/Timer.h>
#include <aq/Exceptions.h>
#include <boost/filesystem.hpp>
#include <boost/program_options.hpp>
#include <boost/algorithm/string.hpp>

int functional_tests(const std::string& dbPath, std::string& queryIdent, const std::string& aqEngine, 
                     const std::string& queriesFilename, const std::string& filter, 
                     const std::string& logFilename, uint64_t limit, bool stopOnError, bool checkCondition)
{
  int rc = 0;
  uint64_t nb_queries_tested = 0;
  uint64_t nb_success = 0;
  uint64_t nb_errors = 0;

  std::fstream log(logFilename.c_str(), std::ifstream::out);

  // generate working directories and aqengine ini
  std::fstream fqueries(queriesFilename.c_str(), std::ifstream::in);
  aq::QueryReader reader(fqueries);
  std::string query;
  while ((query = reader.next()) != "")
  {
    if (filter != "")
    {
      std::string::size_type pos = filter.find("/");
      if (pos != std::string::npos)
      {
        if ((reader.getSuite() + std::string("/") + reader.getIdent()) != filter) 
        {
          continue;
        }
      }
      else if (reader.getSuite() != filter)
      {
        continue;
      }
    }

    std::string iniFilename;
    aq::generate_working_directories(dbPath, queryIdent, iniFilename);

    // for each query read in queries file
    std::ofstream queryFile(dbPath + "calculus/" + queryIdent + "/New_Request.txt");
    queryFile << query ;
    queryFile.close();

    // check for select
    std::vector<std::string> selectedColumns;
    aq::get_columns(selectedColumns, query, "SELECT");

    // check for group
    std::vector<std::string> groupedColumns;
    aq::get_columns(groupedColumns, query, "GROUP");
    
    // check for order
    std::vector<std::string> orderedColumns;
    aq::get_columns(orderedColumns, query, "ORDER");

    // check for Whering
    aq::WhereValidator whereValidator;
    if (checkCondition)
    {
      whereValidator.parseQuery(query);
      // whereValidator.dump(std::cout);
    }

    // execute query
    std::cout << std::endl;
    std::cout << "checking '" << reader.getFullIdent() << "'" << std::endl;
    std::cout << "expecting: " << reader.getExpected() << std::endl;
    aq::Timer timer;
    aq::TProjectSettings settings;
    aq::Base baseDesc;
    aq::AQMatrix matrix(settings, baseDesc);
    rc = aq::run_aq_engine(aqEngine, iniFilename, queryIdent);
    if (rc == 0)
    {
      uint64_t count = reader.extract_value<uint64_t>("count", 0);
      uint64_t nbRows = reader.extract_value<uint64_t>("rows", 0);
      uint64_t nbGroups = reader.extract_value<uint64_t>("groups", 0);
      rc = aq::check_answer_validity(dbPath.c_str(), queryIdent.c_str(), matrix, count, nbRows, nbGroups);
    }
    if (rc == 0)
    {
      std::string answerPath(dbPath);
      answerPath += "/data_orga/tmp/" + std::string(queryIdent) + "/dpy/";
      rc = aq::check_answer_data(answerPath, dbPath, limit, aq::packet_size, selectedColumns, groupedColumns, orderedColumns, whereValidator);
    }
    ++nb_queries_tested;

    // clean tmp directory
    if (rc == 0)
    {
      log << "=== aq engine successfull ===" << std::endl;
      log << "query '" << reader.getFullIdent() << "' :" << std::endl;
      log << query << std::endl;
      log << aq::Timer::getString(timer.getTimeElapsed()) << std::endl;
      std::cout << "aq engine performed on '" << reader.getFullIdent() << "' in " << aq::Timer::getString(timer.getTimeElapsed()) << std::endl;
      boost::filesystem::path tmpPath(dbPath + "data_orga/tmp/" + queryIdent + "/");
      boost::filesystem::remove_all(tmpPath);
      ++nb_success;
    }
    else
    {
      log << "=== aq engine error ===" << std::endl;
      log << "query '" << reader.getFullIdent() << "' :" << std::endl;
      log << query << std::endl;
      log << aq::Timer::getString(timer.getTimeElapsed()) << std::endl;
      std::cerr << "aq engine error on '" << reader.getFullIdent() << "'" << std::endl;
      std::cerr << query << std::endl;
      ++nb_errors;
      if (stopOnError)
        break;
      else
      {
        boost::filesystem::path tmpPath(dbPath + "data_orga/tmp/" + queryIdent + "/");
        boost::filesystem::remove_all(tmpPath);
      }
    }
    
    std::cout << std::endl;
  }

  std::cerr << nb_queries_tested << " queries tested: [ success : " << nb_success << " ; failed : " << nb_errors << " ]" << std::endl;

  log.close();

  return EXIT_SUCCESS;
}
