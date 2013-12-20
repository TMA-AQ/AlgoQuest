#include "TestRunner.h"

using namespace aq;

TestCase::TestCase()
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
  auto it = this->databases.begin();
  aq::DatabaseIntf::result_t r1, r2;
  (*it)->execute(ss, r1);
  ++it;
  for (;it != this->databases.end(); ++it)
  {
    (*it)->execute(ss, r2);
    if (!this->compare(r1, r2))
      return false;
  }
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
      return false;
  }
  return true;
}
