#include <boost/test/unit_test.hpp>

#include <iostream>
#include <set>
#include <aq/ThesaurusReader.h>
#include <aq/ColumnMapper.h>
#include <aq/FileMapper.h>
#include <aq/WIN32FileMapper.h>
#include <aq/DateConversion.h>
#include <aq/ExprTransform.h>
#include <aq/SQLPrefix.h>
#include <aq/parser/sql92_grm_tab.hpp>

#include <boost/algorithm/string/trim.hpp>
#include <boost/algorithm/string/replace.hpp>

BOOST_AUTO_TEST_SUITE(ExprTransform)

std::string transform(const char * ini, const char * query, int tag)
{ 
  int pErr;
  aq::tnode * tree = NULL;
  aq::TProjectSettings settings;
  aq::Base base;
  
  aq::Logger::getInstance().setLevel(2);

  settings.load(ini);
  base.loadFromRawFile(settings.szDBDescFN);
  pErr = SQLParse(query, &tree);
  BOOST_REQUIRE(pErr == 0);
  BOOST_REQUIRE(tree != NULL);
  
  // preprocessing
  aq::dateNodeToBigInt(tree);
  aq::solveIdentRequest(tree, base);
  aq::generate_parent(tree, NULL);

  aq::tnode * cmpNode = aq::find_main_node(tree, K_WHERE);
  BOOST_REQUIRE(cmpNode != NULL);
  cmpNode = aq::find_first_node(cmpNode, tag);
  BOOST_REQUIRE(cmpNode != NULL);
  
  aq::expression_transform::transform<aq::WIN32FileMapper>(base, settings, cmpNode);

  std::string queryTransformed;
  aq::syntax_tree_to_prefix_form(tree, queryTransformed);
  boost::trim(queryTransformed);
  boost::replace_all(queryTransformed, "\n", " ");
  std::string::size_type pos = queryTransformed.find("  ");
  while (pos != std::string::npos)
  {
    queryTransformed.replace(pos, 2, " ");
  }
  return queryTransformed;
}

BOOST_AUTO_TEST_CASE(inf)
{
  std::string queryTransformed = transform("msaq.ini", "select table_1.id from table_1 where table_1.id < 4 ;", K_LT);
  BOOST_CHECK(queryTransformed == "SELECT . table_1 id FROM table_1 WHERE IN . table_1 id , K_VALUE 1 , K_VALUE 2 K_VALUE 3");
}

BOOST_AUTO_TEST_CASE(inf_eqal)
{
  std::string queryTransformed = transform("msaq.ini", "select table_1.id from table_1 where table_1.id <= 3 ;", K_LEQ);
  BOOST_CHECK(queryTransformed == "SELECT . table_1 id FROM table_1 WHERE IN . table_1 id , K_VALUE 1 , K_VALUE 2 K_VALUE 3");
}

BOOST_AUTO_TEST_CASE(sup)
{
  std::string queryTransformed = transform("msaq.ini", "select table_1.id from table_1 where table_1.id > 9997 ;", K_GT);
  BOOST_CHECK(queryTransformed == "SELECT . table_1 id FROM table_1 WHERE IN . table_1 id , K_VALUE 9998 , K_VALUE 9999 K_VALUE 10000");
}

BOOST_AUTO_TEST_CASE(sup_egal)
{
  std::string queryTransformed = transform("msaq.ini", "select table_1.id from table_1 where table_1.id >= 9998 ;", K_GEQ);
  BOOST_CHECK(queryTransformed == "SELECT . table_1 id FROM table_1 WHERE IN . table_1 id , K_VALUE 9998 , K_VALUE 9999 K_VALUE 10000");
}

BOOST_AUTO_TEST_CASE(between)
{
  std::string queryTransformed = transform("msaq.ini", "select table_1.id from table_1 where table_1.id between 1 and 3 ;", K_BETWEEN);
  BOOST_CHECK(queryTransformed == "SELECT . table_1 id FROM table_1 WHERE IN . table_1 id , K_VALUE 1 , K_VALUE 2 K_VALUE 3");
}

BOOST_AUTO_TEST_CASE(not_between)
{
  std::string queryTransformed = transform("msaq.ini", "select table_1.id from table_1 where table_1.id not between 3 and 9999 ;", K_NOT_BETWEEN);
  BOOST_CHECK(queryTransformed == "SELECT . table_1 id FROM table_1 WHERE IN . table_1 id , K_VALUE 1 , K_VALUE 2 K_VALUE 10000");
}

BOOST_AUTO_TEST_CASE(like)
{
  std::string queryTransformed = transform("msaq.ini", "select table_1.id from table_1 where table_1.champ_1 like '%bcghi%' ; ", K_LIKE);
  std::string expected = "SELECT . table_1 id FROM table_1 WHERE IN . table_1 champ_1 , K_VALUE 'abcabcghi' , K_VALUE 'abcghiabc' , K_VALUE 'abcghidef'";
  expected += " , K_VALUE 'abcghighi' , K_VALUE 'abcghijkl' , K_VALUE 'abcghimno' , K_VALUE 'abcghipqr' , K_VALUE 'abcghistu' , K_VALUE 'abcghivwx'";
  expected += " , K_VALUE 'abcghiyz' , K_VALUE 'defabcghi' , K_VALUE 'ghiabcghi' , K_VALUE 'jklabcghi' , K_VALUE 'mnoabcghi' , K_VALUE 'pqrabcghi'";
  expected += " , K_VALUE 'stuabcghi' , K_VALUE 'vwxabcghi' K_VALUE 'yzabcghi'";
  BOOST_CHECK(queryTransformed == expected);
}

BOOST_AUTO_TEST_CASE(not_like)
{
  std::string queryTransformed = transform("msaq.ini", "select table_1.id from table_1 where table_1.champ_1 not like '%abc%' ; ", K_NOT_LIKE);
}

BOOST_AUTO_TEST_CASE(in_date)
{
  std::string queryTransformed = transform("bnp.ini", "select td_temps_calendrier.sem_court from td_temps_calendrier where td_temps_calendrier.dat_ref in (TO_DATE('08/03/2012', '%d/%m/%Y'), TO_DATE('09/03/2012', '%d/%m/%Y'));", K_IN);
  BOOST_CHECK(queryTransformed == "SELECT . td_temps_calendrier sem_court FROM td_temps_calendrier WHERE IN . td_temps_calendrier dat_ref , K_VALUE 93285063354334176 K_VALUE 93285163698509952");
}

BOOST_AUTO_TEST_CASE(inf_date)
{
  std::string queryTransformed = transform("bnp.ini", "select td_temps_calendrier.sem_court from td_temps_calendrier where td_temps_calendrier.dat_ref < TO_DATE('08/03/2012', '%d/%m/%Y');", K_LT);
  BOOST_CHECK(queryTransformed != "");
}

BOOST_AUTO_TEST_CASE(inf_on_multi_packet)
{
  std::string queryTransformed = transform("msaq.ini", "select table_4.val_1 from table_4 where table_4.val_1 < 50 ;", K_LT);
  BOOST_CHECK(queryTransformed != "");
}

BOOST_AUTO_TEST_SUITE_END()
