#ifndef __AQ_TEST_RUNNER_H__
#define __AQ_TEST_RUNNER_H__

#include <list>
#include <string>
#include <memory>

#include <boost/shared_ptr.hpp>

#include "DatabaseHelper.h"

namespace aq 
{
  class TestCase : public DatabaseIntf
  {
  public:
    typedef boost::shared_ptr<TestCase> Ptr;
    struct opt_t
    {
      opt_t()
        : 
        generator_filename("queries.gen"),
        nb_tables(2),
        nb_rows(10),
        min_value(1000),
        max_value(1100),
        point_mode(DatabaseGenerator::point_mode_t::FULL),
        gen_mode(DatabaseGenerator::gen_mode_t::ALL),
        value_mode(DatabaseGenerator::value_mode_t::ALL_UNIQUE),
        aq_path(""),
        aq_name("algoquest"),
        mysql_host("localhost"),
        mysql_user("algoquest"),
        mysql_pass("algoquest"),
        mysql_name("algoquest")
      {
      }
      std::string generator_filename;
      size_t nb_tables;
      size_t nb_rows;
      int min_value;
      int max_value;
      DatabaseGenerator::point_mode_t point_mode;
      DatabaseGenerator::gen_mode_t gen_mode;
      DatabaseGenerator::value_mode_t value_mode;
      std::string aq_path;
      std::string aq_name;
      std::string mysql_host;
      std::string mysql_user;
      std::string mysql_pass;
      std::string mysql_name;
    };
  public:
    TestCase();
    TestCase(const TestCase& o);
    ~TestCase();
    TestCase& operator=(const TestCase& o);
    void add(boost::shared_ptr<DatabaseIntf> db) { this->databases.push_back(db); }
    void clean();
    void createTable(const DatabaseGenerator::handle_t::tables_t::key_type& table);
    void insertValues(const DatabaseGenerator::handle_t::tables_t::value_type& values);
    bool execute(const aq::core::SelectStatement& ss, DatabaseIntf::result_t& r1);
  protected:
    bool compare(const DatabaseIntf::result_t& r1, const DatabaseIntf::result_t& r2);
  private:
    std::list<boost::shared_ptr<DatabaseIntf> > databases;
  };
}

#endif