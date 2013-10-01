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

BOOST_AUTO_TEST_CASE(test_2_tables_one_col)
{
  std::string query, expected;

  query  = " FROM , T1 T2 WHERE AND ";
  query += " K_JEQ K_INNER . T1 C1 K_INNER . T2 C1 ";

  expected  = " FROM , T1 T2 WHERE AND ";
  expected += " K_JEQ K_INNER . T1 C1 K_INNER . T2 C1 ";

  aq::ParseJeq(query);

  BOOST_CHECK(cmp_string(query, expected));
}

BOOST_AUTO_TEST_CASE(test_2_tables_multi_col)
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

BOOST_AUTO_TEST_CASE(test_2_tables_sup_1)
{
  std::string query, expected;

  query  = " FROM , T1 T2 WHERE AND ";
  query += " K_JSUP K_INNER . T2 C1 K_INNER . T1 C1 ";

  expected  = " FROM , T1 T2 WHERE AND ";
  expected += " K_JSUP K_INNER . T2 C1 K_INNER . T1 C1 ";

  aq::ParseJeq(query);
  
  BOOST_CHECK(cmp_string(query, expected));
}

BOOST_AUTO_TEST_CASE(test_2_tables_sup_2)
{
  std::string query, expected;

  query  = " FROM , T1 T2 WHERE AND ";
  query += " K_JSUP K_INNER . T2 C1 K_INNER . T1 C1 ";

  expected  = " FROM , T1 T2 WHERE AND ";
  expected += " K_JSUP K_INNER . T2 C1 K_INNER . T1 C1 ";

  aq::ParseJeq(query);
  
  BOOST_CHECK(cmp_string(query, expected));
}

BOOST_AUTO_TEST_CASE(test_2_tables_sup_3)
{
  std::string from, query, expected;

  from = " FROM , , T1 T2 T3 WHERE AND AND ";

  query  = from;
  query += " K_JSUP K_INNER . T1 C1 K_INNER . T2 C1 ";
  query += " K_JSUP K_INNER . T3 C1 K_INNER . T2 C1 ";

  expected  = from;
  expected += " K_JINF K_INNER . T2 C1 K_INNER . T1 C1 ";
  expected += " K_JSUP K_INNER . T3 C1 K_INNER . T2 C1 ";

  aq::ParseJeq(query);
  
  BOOST_CHECK(cmp_string(query, expected));
}

BOOST_AUTO_TEST_CASE(test_2_tables_sup_4)
{
  std::string query, expected;

  query  = " FROM , SOURCE DESTINATION WHERE AND ";
  query += " K_JSUP K_INNER . DESTINATION C1 K_INNER . SOURCE C1 ";

  expected  = " FROM , SOURCE DESTINATION WHERE AND ";
  expected += " K_JSUP K_INNER . DESTINATION C1 K_INNER . SOURCE C1 ";

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

BOOST_AUTO_TEST_CASE(test_3_tables_sup_1)
{
  std::string from, query, expected;

  from = " FROM , , SRC INT DST WHERE AND AND ";

  query  = from;
  query += " K_JSUP K_INNER . INT C1 K_INNER . SRC C1 ";
  query += " K_JSUP K_INNER . DST C1 K_INNER . INT C1 ";

  expected  = from;
  expected += " K_JINF K_INNER . INT C1 K_INNER . DST C1 ";
  expected += " K_JINF K_INNER . SRC C1 K_INNER . INT C1 ";

  aq::ParseJeq(query);
  
  BOOST_CHECK(cmp_string(query, expected));
}

BOOST_AUTO_TEST_CASE(test_3_tables_sup_2)
{
  std::string from, query, expected;

  from = " FROM , , SRC INT DST WHERE AND AND ";

  query  = from;
  query += " K_JSUP K_INNER . DST C1 K_INNER . INT C1 ";
  query += " K_JSUP K_INNER . INT C1 K_INNER . SRC C1 ";

  expected  = from;
  expected += " K_JINF K_INNER . INT C1 K_INNER . DST C1 ";
  expected += " K_JINF K_INNER . SRC C1 K_INNER . INT C1 ";

  aq::ParseJeq(query);
  
  BOOST_CHECK(cmp_string(query, expected));
}

BOOST_AUTO_TEST_CASE(test_3_tables_sup_3)
{
  std::string from, query, expected;

  from = " FROM , , A B C WHERE AND AND ";

  query  = from;
  query += " K_JSUP K_INNER . A C1 K_INNER . B C1 ";
  query += " K_JSUP K_INNER . B C1 K_INNER . C C1 ";

  expected  = from;
  expected += " K_JINF K_INNER . B C1 K_INNER . A C1 ";
  expected += " K_JINF K_INNER . C C1 K_INNER . B C1 ";

  aq::ParseJeq(query);
  
  BOOST_CHECK(cmp_string(query, expected));
}

BOOST_AUTO_TEST_CASE(test_4_tables_linear)
{
  std::string from, query, expected;

  from = " FROM , , T1 T2 T3 T4 WHERE AND ";

  query  = from;
  query += " K_JEQ K_INNER . T1 C1 K_INNER . T2 C1 ";
  query += " K_JEQ K_INNER . T3 C1 K_INNER . T4 C1 ";
  query += " K_JEQ K_INNER . T2 C2 K_INNER . T3 C2 ";

  expected  = from;
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