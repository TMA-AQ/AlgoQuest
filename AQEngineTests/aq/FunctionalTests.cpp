#include "Util.h"
#include "QueryReader.h"
#include "WhereValidator.h"
#include <aq/AQLParser.h>
#include <aq/Base.h>
#include <aq/Timer.h>
#include <aq/Exceptions.h>
#include <aq/parser/JeqParser.h>
#include <io.h>
#include <stdio.h>
#include <iostream>
#include <fstream>
#include <boost/filesystem.hpp>
#include <boost/program_options.hpp>
#include <boost/algorithm/string.hpp>


namespace aq
{

class stream_cb : public display_cb
{
public:
  enum align_t
  {
    LEFT,
    CENTER,
    RIGHT,
  };

public:
  stream_cb(std::ostream& _os) : os(_os), index(0), align(RIGHT), headers(false)
  {
  }
  void push(const std::string& value)
  {
    if (index > 0)
    {
      os << " | ";
    }
    if (index == widths.size())
    {
      if (value.size() < 8)
      {
        for (size_t i = 0; i < 8 - value.size(); i++)
        {
          os << " ";
        }
      }
      os << value;
      widths.push_back(value.size() < 8 ? 8 : value.size());
    }
    else
    {
      if (value.size() < this->widths[this->index])
      {
        if (this->align == RIGHT)
        {
          for (size_t i = 0; i < this->widths[this->index] - value.size(); i++)
          {
            os << " ";
          }
          os << value;
        }
      }
    }
    index += 1;
  }
  void next()
  {
    os << std::endl;
    if (!this->headers)
    {
      for (size_t i = 0; i < this->widths.size(); i++)
      {
        for (size_t j = 0; j < this->widths[i]; j++)
        {
          os << "-";
        }
        if ((i + 1) != this->widths.size())
        {
          os << "---";
        }
      }
      os << std::endl;
      this->headers = true;
    }
    index = 0;
  }
private:
  std::ostream& os;
  std::vector<size_t> widths;
  size_t index;
  align_t align;
  bool headers;
};

uint64_t functional_tests(const struct opt& o)
{
  int rc = 0;
  uint64_t nb_queries_tested = 0;
  uint64_t nb_success = 0;
  uint64_t nb_errors = 0;
  aq::Timer timer;

  std::fstream log(o.logFilename.c_str(), std::ifstream::out);

  aq::QueryReader * reader = 0;
  std::fstream * fqueries = 0;
  if (o.queriesFilename != "")
  {
    boost::filesystem::path p(o.queriesFilename);
    if (!boost::filesystem::exists(p)) 
    {
      std::cerr << "cannot find file " << p << std::endl;
      return -1;
    }

    fqueries = new std::fstream(o.queriesFilename.c_str(), std::ifstream::in);
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
    if (o.filter != "")
    {
      std::string::size_type pos = o.filter.find("/");
      if (pos != std::string::npos)
      {
        if ((reader->getSuite() + std::string("/") + reader->getIdent()) != o.filter) 
        {
          continue;
        }
      }
      else if (reader->getSuite() != o.filter)
      {
        continue;
      }
    }
    
    // add eof on each important keyword (FROM, WHERE, GROUP, ORDER, K_JXXX, IN)
    boost::replace_all(query, "\n", " ");
    std::string keywords[] = { 
      "FROM", "WHERE", "GROUP", "ORDER",
      "IN ", "K_JNO ", "K_JEQ ", "K_JNEQ ", "K_JINF ", "K_JIEQ ", "K_JSEQ ", "K_JSUP " };
    for (auto& kw : keywords)
    {
      boost::replace_all(query, kw, "\n" + kw);
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
    assert(!selectedColumns.empty());

    // check for group
    std::vector<std::string> groupedColumns;
    aq::get_columns(groupedColumns, query, "GROUP");
    
    // check for order
    std::vector<std::string> orderedColumns;
    aq::get_columns(orderedColumns, query, "ORDER");
    
    // FIXME : should be in k_jeq_parser
    std::string::size_type pos = query.find("WHERE");
    if (pos == std::string::npos)
    {
      pos = query.find(";");
      assert(pos != std::string::npos);
      query.erase(pos);
      std::string group, order;
      pos = query.find("ORDER");
      if (pos != std::string::npos)
      {
        order = query.substr(pos);
        query.erase(pos);
      }
      pos = query.find("GROUP");
      if (pos != std::string::npos)
      {
        group = query.substr(pos);
        query.erase(pos);
      }
      std::string col = selectedColumns[0];
      pos = col.find(".");
      assert(pos != std::string::npos);
      query += "\nWHERE K_JNO K_INNER . " + col.substr(0, pos) + " " + col.substr(pos+1) ;
      if (group != "")
        query += "\n" + group;
      if (order != "")
        query += "\n" + order;
    }

    // kjeq_parse
    if (o.jeqParserActivated)
    {
      aq::ParseJeq(query);
    }
    
    aq::Logger::getInstance().log(AQ_INFO, "processing query: \n%s\n", query.c_str());

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

      aq::Logger::getInstance().log(AQ_INFO, "checking '%s'\n", reader->getFullIdent().c_str());
      aq::Logger::getInstance().log(AQ_INFO, "expecting: %s\n", reader->getExpected());
      aq::TProjectSettings settings;
      aq::Base baseDesc;
      aq::AQMatrix matrix(settings, baseDesc);

      // run aq engine
      timer.start();
      rc = aq::run_aq_engine(o.aqEngine, iniFilename, o.queryIdent);
      std::string aq_engine_time_elapsed = aq::Timer::getString(timer.getTimeElapsed());
      aq::Logger::getInstance().log(AQ_INFO, "aq engine performed on '%s' in %s\n", reader->getFullIdent().c_str(), aq_engine_time_elapsed.c_str());

      if (rc == 0 && (o.checkResult || o.display))
      {
        // check answer validity
        if (o.checkResult)
        {
          uint64_t count = reader->extract_value<uint64_t>("count", 0);
          uint64_t nbRows = reader->extract_value<uint64_t>("rows", 0);
          uint64_t nbGroups = reader->extract_value<uint64_t>("groups", 0);
          rc = aq::check_answer_validity(o, matrix, count, nbRows, nbGroups);
        }

        // check data validity
        if (o.checkResult)
        {
          std::string answerPath(o.dbPath);
          answerPath += "/data_orga/tmp/" + std::string(o.queryIdent) + "/dpy/";
          rc = aq::check_answer_data(std::cout, answerPath, o, selectedColumns, groupedColumns, orderedColumns /*, whereValidator*/);
        }
        else
        {
          display_cb * cb = new stream_cb(std::cout); 
          std::string answerPath(o.dbPath);
          answerPath += "/data_orga/tmp/" + std::string(o.queryIdent) + "/dpy/";
          rc = aq::display(cb, answerPath, o, selectedColumns);
        }
      }
      
      ++nb_queries_tested;

      // clean tmp directory
      if (rc == 0)
      {
        log << "=== aq engine successfull ===" << std::endl;
        log << "query '" << reader->getFullIdent() << "' :" << std::endl;
        log << query << std::endl;
        log << aq_engine_time_elapsed << std::endl;
        boost::filesystem::path tmpPath(o.dbPath + "data_orga/tmp/" + o.queryIdent + "/");
        boost::filesystem::remove_all(tmpPath);
        ++nb_success;
      }
      else
      {
        log << "=== aq engine error ===" << std::endl;
        log << "query '" << reader->getFullIdent() << "' :" << std::endl;
        log << query << std::endl;
        log << aq::Timer::getString(timer.getTimeElapsed()) << std::endl;
        aq::Logger::getInstance().log(AQ_ERROR, "aq engine error on '%s'\n", reader->getFullIdent().c_str());
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

  aq::Logger::getInstance().log(AQ_INFO, "%u queries tested: [ success : %u ; failed : %u ; \n", nb_queries_tested, nb_success, nb_errors);

  log.close();

  if (fqueries != 0)
  {
    fqueries->close();
    delete fqueries;
  }

  return nb_errors;
}

}
