#include <iostream>
#include <boost/test/unit_test.hpp>
#include <boost/program_options.hpp>
#include <boost/algorithm/string/trim.hpp>
#include <boost/algorithm/string/replace.hpp>

#include <aq/parser/JeqParser.h>

bool cmp_string(std::string& query, std::string& expected)
{
  boost::algorithm::replace_all(query, "\n", " ");
  boost::algorithm::replace_all(expected, "\n", " ");
  while (query.find("  ") != std::string::npos)
    boost::algorithm::replace_all(query, "  ", " ");
  while (expected.find("  ") != std::string::npos)
    boost::algorithm::replace_all(expected, "  ", " ");
  boost::algorithm::trim(query);
  boost::algorithm::trim(expected);
  return query == expected;
}

BOOST_AUTO_TEST_SUITE(JeqParser)

  BOOST_AUTO_TEST_CASE(test_2_tables_one_column)
{
  std::string query, expected;

  query  = " FROM , T1 T2 WHERE AND ";
  query += " K_JEQ K_INNER . T1 C1 K_INNER . T2 C1 ";

  expected  = " FROM , T1 T2 WHERE AND ";
  expected += " K_JEQ K_INNER . T2 C1 K_INNER . T1 C1 ";

  aq::ParseJeq(query);

  BOOST_CHECK(cmp_string(query, expected));
}

BOOST_AUTO_TEST_CASE(test_2_tables_multiple_column)
{
  std::string query, expected;

  query  = " FROM , T1 T2 WHERE AND ";
  query += " K_JEQ K_INNER . T1 C1 K_INNER . T2 C1 ";
  query += " K_JEQ K_INNER . T2 C2 K_INNER . T1 C2 ";
  query += " K_JEQ K_INNER . T2 C3 K_INNER . T1 C3 ";
  query += " K_JEQ K_INNER . T1 C4 K_INNER . T2 C4 ";

  expected  = " FROM , T1 T2 WHERE AND ";
  expected += " K_JEQ K_INNER . T2 C1 K_INNER . T1 C1 ";
  expected += " K_JEQ K_INNER . T2 C2 K_INNER . T1 C2 ";
  expected += " K_JEQ K_INNER . T2 C3 K_INNER . T1 C3 ";
  expected += " K_JEQ K_INNER . T2 C4 K_INNER . T1 C4 ";

  aq::ParseJeq(query);
  
  BOOST_CHECK(cmp_string(query, expected));
}

BOOST_AUTO_TEST_CASE(test_3_tables_linear_1)
{
  std::string query, expected;

  query  = " FROM , , T1 T2 T3 WHERE AND ";
  query += " K_JEQ K_INNER . T1 C1 K_INNER . T2 C1";
  query += " K_JEQ K_INNER . T2 C2 K_INNER . T3 C2";

  expected  = " FROM , , T1 T2 T3 WHERE AND ";
  expected += " K_JEQ K_INNER . T2 C1 K_INNER . T1 C1 ";
  expected += " K_JEQ K_INNER . T3 C2 K_INNER . T2 C2 ";

  aq::ParseJeq(query);

  BOOST_CHECK(cmp_string(query, expected));
}

BOOST_AUTO_TEST_CASE(test_3_tables_linear_2)
{
  std::string query, expected;

  query  = " FROM , , T1 T2 T3 WHERE AND ";
  query += " K_JEQ K_INNER . T2 C1 K_INNER . T1 C1 ";
  query += " K_JEQ K_INNER . T2 C2 K_INNER . T3 C2 ";

  expected  = " FROM , , T1 T2 T3 WHERE AND ";
  expected += " K_JEQ K_INNER . T2 C1 K_INNER . T1 C1 ";
  expected += " K_JEQ K_INNER . T3 C2 K_INNER . T2 C2 ";

  aq::ParseJeq(query);
  
  BOOST_CHECK(cmp_string(query, expected));
}

BOOST_AUTO_TEST_CASE(test_3_tables_circular)
{
  std::string query, expected;

  query  = " FROM , , T1 T2 T3 WHERE AND AND ";
  query += " K_JEQ K_INNER . T1 C1 K_INNER . T2 C1 ";
  query += " K_JEQ K_INNER . T2 C2 K_INNER . T3 C2 ";
  query += " K_JEQ K_INNER . T3 C2 K_INNER . T1 C2 ";

  expected  = " FROM , , T1 T2 T3 WHERE AND AND ";
  expected += " K_JEQ K_INNER . T2 C1 K_INNER . T1 C1 ";
  expected += " K_JEQ K_INNER . T3 C2 K_INNER . T2 C2 ";
  expected += " K_JEQ K_INNER . T1 C2 K_INNER . T3 C2 ";

  aq::ParseJeq(query);
  
  BOOST_CHECK(cmp_string(query, expected));
}

BOOST_AUTO_TEST_CASE(test_4_tables_linear)
{
  std::string query, expected;

  query  = " FROM , , T1 T2 T3 T4 WHERE AND ";
  query += " K_JEQ K_INNER . T1 C1 K_INNER . T2 C1 ";
  query += " K_JEQ K_INNER . T3 C1 K_INNER . T4 C1 ";
  query += " K_JEQ K_INNER . T2 C2 K_INNER . T3 C2 ";

  expected  = " FROM , , T1 T2 T3 T4 WHERE AND ";
  expected += " K_JEQ K_INNER . T2 C1 K_INNER . T1 C1 ";
  expected += " K_JEQ K_INNER . T3 C2 K_INNER . T2 C2 ";
  expected += " K_JEQ K_INNER . T4 C1 K_INNER . T3 C1 ";

  aq::ParseJeq(query);

  BOOST_CHECK(cmp_string(query, expected));
}

BOOST_AUTO_TEST_CASE(test_4_tables_star)
{
  std::string query, expected;

  query  = " FROM , , T1 T2 T3 T4 WHERE AND AND ";
  query += " K_JEQ K_INNER . T1 C1 K_INNER . T2 C1 ";
  query += " K_JEQ K_INNER . T1 C1 K_INNER . T4 C1 ";
  query += " K_JEQ K_INNER . T1 C2 K_INNER . T3 C2 ";

  expected  = " FROM , , T1 T2 T3 T4 WHERE AND AND AND ";
  expected += " K_JEQ K_INNER . T1 C1 K_INNER . T2 C1 ";
  expected += " K_JEQ K_INNER . T4 C1 K_INNER . T1 C1 ";
  expected += " K_JEQ K_INNER . T1 C1 K_INNER . T4 C1 ";
  expected += " K_JEQ K_INNER . T3 C2 K_INNER . T1 C2 ";

  aq::ParseJeq(query);
  
  BOOST_CHECK(cmp_string(query, expected));
}

BOOST_AUTO_TEST_CASE(test_4_tables_circular)
{
  std::string query, expected;

  query  = " FROM , , T1 T2 T3 T4 WHERE AND AND AND ";
  query += " K_JEQ K_INNER . T1 C1 K_INNER . T2 C1";
  query += " K_JEQ K_INNER . T2 C1 K_INNER . T3 C1";
  query += " K_JEQ K_INNER . T3 C2 K_INNER . T4 C2";
  query += " K_JEQ K_INNER . T4 C2 K_INNER . T1 C2";

  expected  = " FROM , , T1 T2 T3 T4 WHERE AND AND AND ";
  expected += " K_JEQ K_INNER . T2 C1 K_INNER . T1 C1 ";
  expected += " K_JEQ K_INNER . T3 C1 K_INNER . T2 C1 ";
  expected += " K_JEQ K_INNER . T4 C2 K_INNER . T3 C2 ";
  expected += " K_JEQ K_INNER . T1 C2 K_INNER . T4 C2 ";

  aq::ParseJeq(query);
  
  BOOST_CHECK(cmp_string(query, expected));
}

BOOST_AUTO_TEST_SUITE_END()