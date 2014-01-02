#include "DatabaseHelper.h"

// forward declaration
namespace sql
{
  class Driver;
  class Connection;
  class Statement;
}

namespace aq
{

  class MySQLDatabase : public DatabaseIntf
  {
  public:
    MySQLDatabase(const std::string& _host, const std::string& _user, const std::string& _pass, const std::string& _name);
    void clean() {};
    void createTable(const DatabaseGenerator::handle_t::tables_t::key_type& table);
    void insertValues(const DatabaseGenerator::handle_t::tables_t::value_type& values);
    bool execute(const aq::core::SelectStatement& ss, DatabaseIntf::result_t& r1);
  private:
    std::string              host;
    std::string              user;
    std::string              pass;
    std::string              name;
    std::stringstream        query;
    sql::Driver            * driver;
    sql::Connection        * con;
    sql::Statement         * stmt;
    std::list<std::string>   columns;
  };
  
}