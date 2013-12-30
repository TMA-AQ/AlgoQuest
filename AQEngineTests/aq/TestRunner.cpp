#include "TestRunner.h"

#include <boost/algorithm/string/case_conv.hpp>
#include <boost/property_tree/ptree.hpp>
#include <boost/property_tree/ini_parser.hpp>

using namespace aq;

namespace helper
{
  template <class T>
  T get_opt_value(boost::property_tree::ptree& pt, const char * key, T default_value)
  {
    boost::optional<T> opt = pt.get_optional<T>(boost::property_tree::ptree::path_type(key));
    if (opt.is_initialized()) return opt.get();
    else return default_value;
  }

  bool get_opt_value(boost::property_tree::ptree& pt, const char * key, bool default_value)
  {
    boost::optional<std::string> opt = pt.get_optional<std::string>(boost::property_tree::ptree::path_type(key));
    if (opt.is_initialized()) 
    {
      std::string s = opt.get();
      boost::to_upper(s);
      return s == "TRUE" || s == "YES" || s == "1";
    }
    else return default_value;
  }
}

void TestCase::opt_t::parse(std::istream& is)
{
  try
  {
    boost::property_tree::ptree pt;
    boost::property_tree::ini_parser::read_ini(is, pt);

    // algoquest options
    this->aq_name = helper::get_opt_value(pt, "algoquest.db-name", this->aq_name);
    this->aq_path = helper::get_opt_value(pt, "algoquest.db-path", this->aq_path);

    // mysql options
    this->mysql_host = helper::get_opt_value(pt, "mysql.db-host", this->mysql_host);
    this->mysql_name = helper::get_opt_value(pt, "mysql.db-name", this->mysql_name);
    this->mysql_user = helper::get_opt_value(pt, "mysql.user", this->mysql_pass);
    this->mysql_pass = helper::get_opt_value(pt, "mysql.pass", this->mysql_user);

    // generation options
    this->generator_filename = helper::get_opt_value(pt, "generator.filename", this->generator_filename);
    this->max_value = helper::get_opt_value(pt, "generator.max-value", this->max_value);
    this->min_value = helper::get_opt_value(pt, "generator.min-value", this->min_value);
    this->nb_rows = helper::get_opt_value(pt, "generator.nb-rows", this->nb_rows);
    this->nb_tables = helper::get_opt_value(pt, "generator.nb-tables", this->nb_tables);
    this->point_mode = DatabaseGenerator::point_mode_t::MIN_MAX; // TODO
    this->gen_mode = DatabaseGenerator::gen_mode_t::INTERSECT; // TODO
    this->value_mode = DatabaseGenerator::value_mode_t::RANDOM; // TODO
  }
  catch (const boost::property_tree::ptree_error& e)
  {
    // TODO
  }
}

TestCase::TestCase()
  : nb_result(0),
  nb_tests(0),
  nb_success(0),
  nb_failure(0)
{
}

TestCase::TestCase(const TestCase& o)
{
}

TestCase::~TestCase()
{
}

TestCase& TestCase::operator=(const TestCase& o)
{
  return *this;
}

void TestCase::clean()
{
  nb_result = 0;
  nb_tests = 0;
  nb_success = 0;
  nb_failure = 0;
  for (auto& db : databases)
  {
    db->clean();
  }
}

void TestCase::createTable(const DatabaseGenerator::handle_t::tables_t::key_type& table)
{
  for (auto& db : databases)
  {
    db->createTable(table);
  }
}

void TestCase::insertValues(const DatabaseGenerator::handle_t::tables_t::value_type& values)
{
  for (auto& db : databases)
  {
    db->insertValues(values);
  }
}

bool TestCase::execute(const aq::core::SelectStatement& ss, aq::DatabaseIntf::result_t&)
{
  if (this->databases.empty())
    return true;
  nb_tests += 1;
  auto it = this->databases.begin();
  aq::DatabaseIntf::result_t r1, r2;
  (*it)->execute(ss, r1);
  ++it;
  for (;it != this->databases.end(); ++it)
  {
    (*it)->execute(ss, r2);
    nb_result += std::max(r1.size(), r2.size());
    // std::cout << nb_result << std::endl;
    if (!this->compare(r1, r2))
    {
      nb_failure += 1;
      return false;
    }
  }
  nb_success += 1;
  return true;
}

bool TestCase::compare(const DatabaseIntf::result_t& result1, const DatabaseIntf::result_t& result2)
{
  if (result1.size() != result2.size())
    return false;
  if (result1.empty())
    return true;

  for (const auto& r1 : result1)
  {
    bool find = false;
    for (const auto& r2 : result2)
    {
      if (r1 == r2)
      {
        find = true;
        break;
      }
    }
    if (!find)
    {
      std::cout << "tuple r1(";
      std::copy(r1.begin(), r1.end(), std::ostream_iterator<std::string>(std::cout, " "));
      std::cout << ") not find in r2";
      return false;
    }
  }
  return true;
}
