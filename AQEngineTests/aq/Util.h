#ifndef __UTIL_H__
#define __UTIL_H__

#include "WhereValidator.h"

#include <aq/AQMatrix.h>
#include <string>
#include <vector>

namespace aq
{
extern bool verbose;

int generate_database(const char * path, const char * name);
int generate_working_directories(const std::string& dbPath, std::string& queryIdent, std::string& iniFilename);
int run_aq_engine(const std::string& aq_engine, const std::string& iniFilename, const std::string& ident);

void get_columns(std::vector<std::string>& columns, const std::string& query, const std::string& key);
int check_answer_validity(const char * dbPath, const char * queryIdent, aq::AQMatrix& matrix, 
                          const uint64_t count, const uint64_t nbRows, const uint64_t nbGroups);
int check_answer_data(const std::string& answerPath, const std::string& dbPath, const size_t limit, const size_t packetSize, 
                      const std::vector<std::string>& selectedColumns,
                      const std::vector<std::string>& groupedColumns,
                      const std::vector<std::string>& orderedColumns,
                      WhereValidator& whereValidator);
int print_temporary_table(const std::string& tablePath, const std::string& dbPath, const size_t limit, const size_t packetSize);

}

#endif