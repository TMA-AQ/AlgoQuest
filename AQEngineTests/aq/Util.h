#ifndef __UTIL_H__
#define __UTIL_H__

// #include "WhereValidator.h"

# include <aq/Base.h>
#include <aq/AQMatrix.h>
#include <string>
#include <vector>
#include <sstream>

namespace aq
{
struct opt
{
  opt() : 
    packetSize(aq::packet_size), limit(0), execute(false), stopOnError(false), checkResult(false), checkCondition(false),
    jeqParserActivated(false), aql2sql(false), display(true), withCount(false), withIndex(false), force(false)
  {
  }
  std::string dbPath;
  std::string workingPath;
  std::string queryIdent;
  std::string aqEngine;
  std::string queriesFilename;
  std::string filter;
  std::string logFilename;
  uint64_t packetSize;
  uint64_t limit;
  bool execute;
  bool stopOnError;
  bool checkResult;
  bool checkCondition;
  bool jeqParserActivated;
  bool aql2sql;
  bool display;
  bool withCount;
  bool withIndex;
  bool force;
};

struct display_cb
{
  virtual void push(const std::string& value) = 0;
  virtual void next() = 0;
};

uint64_t functional_tests(const struct opt& o);

int generate_database(const char * path, const char * name);
int generate_working_directories(const struct opt& o, std::string& iniFilename);
int run_aq_engine(const std::string& aq_engine, const std::string& iniFilename, const std::string& ident);

void get_columns(std::vector<std::string>& columns, const std::string& query, const std::string& key);
int check_answer_validity(const struct opt& o, aq::AQMatrix& matrix, 
                          const uint64_t count, const uint64_t nbRows, const uint64_t nbGroups);
//int check_answer_data(std::ostream& os,
//                      const std::string& answerPath,
//                      const struct opt& o,
//                      const std::vector<std::string>& selectedColumns,
//                      const std::vector<std::string>& groupedColumns,
//                      const std::vector<std::string>& orderedColumns
//                      // WhereValidator& whereValidator
//                      );
int display(display_cb *,
            const std::string& answerPath,
            const struct opt& o,
            const std::vector<std::string>& selectedColumns);
int print_temporary_table(const std::string& tablePath, const std::string& dbPath, const size_t limit, const size_t packetSize);

}

#endif