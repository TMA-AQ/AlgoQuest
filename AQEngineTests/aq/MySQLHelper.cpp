#include "MySQLHelper.h"

#include <mysql_connection.h>
#include <cppconn/driver.h>
#include <cppconn/exception.h>
#include <cppconn/resultset.h>
#include <cppconn/statement.h>
#include <cppconn/prepared_statement.h>
#include <cppconn/metadata.h>

using namespace aq;

MySQLDatabase::MySQLDatabase(const std::string& _host, const std::string& _user, const std::string& _pass, const std::string& _name)
  : host(_host), user(_user), pass(_pass), name(_name)
{
  driver = get_driver_instance();
  if (driver == nullptr)
    throw std::exception("[mysql-connector] : cannot get mysql driver");
  con = driver->connect(host, user, pass);
  if (con == nullptr)
    throw std::exception("[mysql-connector] : cannot connect to mysql driver");
  con->setSchema(name);
  stmt = con->createStatement();
  if (stmt == nullptr)
    throw std::exception("[mysql-connector] : cannot create statement");
}

void MySQLDatabase::createTable(const DatabaseGenerator::handle_t::tables_t::key_type& table)
{
  query.str("");
  query << "drop table if exists `" << table << "`;";
  stmt->execute(query.str());

  query.str("");
  query << "create table `" << table << "`(id int primary key auto_increment, v1 int) engine=innodb";
  stmt->execute(query.str());

}

void MySQLDatabase::insertValues(const DatabaseGenerator::handle_t::tables_t::value_type& values)
{
  if (values.second.empty())
  {
    return;
  }
  query.str("");
  query << "insert into `" << values.first << "` (v1) values ";
  for (auto it = values.second.begin(); it != values.second.end();)
  {
    query << "(" << *it << ")";
    ++it;
    if (it != values.second.end())
      query << ",";
  }
  query << ";";
  stmt->execute(query.str());
}

bool MySQLDatabase::execute(const aq::core::SelectStatement& ss, DatabaseIntf::result_t& result)
{
  std::string query;
  ss.setOutput(aq::core::SelectStatement::output_t::SQL);
  ss.to_string(query);
  
  if (query.find("full outer join") != std::string::npos)
  {
    return true;
  }

  try
  {
    sql::ResultSet * res = stmt->executeQuery(query);
    sql::ResultSetMetaData * meta = res->getMetaData();
    this->columns.clear();
    for (size_t c = 1; c <= meta->getColumnCount(); c++)
    {
      auto & s = meta->getColumnName(c);
      this->columns.push_back(s.c_str());
    }
    result.clear();
    while (res->next())
    {
      result.push_back(result_t::value_type());
      auto& r = *result.rbegin();
      // for (const auto& c : columns)
      for (size_t c = 1; c <= columns.size(); c++)
      {
        auto & s = res->getString(c);
        r.push_back(s);
      }
    }
  }
  catch (const sql::SQLException& ex)
  {
    std::string msg = ex.what();
  }
  return true;
}
