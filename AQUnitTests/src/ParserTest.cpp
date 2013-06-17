#include <boost/test/unit_test.hpp>
#include <boost/program_options.hpp>

#include <aq/TreeUtilities.h>
#include <aq/parser/SQLParser.h>
#include <aq/SQLPrefix.h>

namespace po = boost::program_options;

extern int yylineno;
int yyerror( const char *pszMsg ) 
{
	std::cerr << "SQL Parsing Error : " << pszMsg << " encountered at line number: " << yylineno << std::endl;
	return 0;
}

void runParserTest( std::string query, std::string expected, std::string baseFilename )
{
  std::string rootDirectory;
//  "SELECT T1.A, B, TMP2.A, TMP2.D, E, SUM(T1.A + B - TMP2.E) as v_sum , CASE T1.A	WHEN 1 THEN B	WHEN 2 THEN E END AS t1a_case FROM T1, T2 as TMP2, ( SELECT A	FROM T1	WHERE A = B AND A < 10 AND B IN (1, 2, 3)	ORDER BY A ) AS TMP3,	(	SELECT A, SUM(D) AS AA FROM T2 WHERE A = T2.E GROUP BY A, E ) AS TMP4 WHERE T1.A = TMP4.A	AND B = D	AND TMP2.D = TMP2.E	AND SUBSTRING(D, 4, 3) = T1.B	AND TMP3.A = TMP4.AA GROUP BY T1.A, B, TMP2.A, D, E HAVING v_sum < 10 ORDER BY E;"
  po::options_description desc("Allowed options");
  desc.add_options()
    ("root-dir,r", po::value<std::string>(&rootDirectory)->default_value("./tests"), "")
    ("base,b", po::value<std::string>(&baseFilename)->default_value("base_test.aq"), "")
    ;
  
  po::variables_map vm;
  po::store(po::command_line_parser(
    boost::unit_test::framework::master_test_suite().argc, 
    boost::unit_test::framework::master_test_suite().argv).options(desc).run(), vm);
  po::notify(vm); 

  BOOST_CHECK(true);
  BOOST_REQUIRE(true);
  BOOST_TEST_MESSAGE("a message");
  
  aq::Base baseDesc;
  baseDesc.loadFromRawFile(baseFilename.c_str());

  aq::tnode * pNode;
  if ((SQLParse(query.c_str(), &pNode)) != 0 ) 
  {
    BOOST_REQUIRE(false);
  }

  std::string new_query;
  aq::tnode * n = aq::clone_subtree(pNode);
  try
  {
    aq::solveIdentRequest(n, baseDesc);
    //std::cout << *n << std::endl;
    aq::generate_parent( n, NULL );
    aq::syntax_tree_to_sql_form(n, new_query);
    //std::cout << new_query << std::endl;
  }
  catch ( const std::exception & e )
  {
    aq::generate_parent( n, NULL );
    aq::syntax_tree_to_sql_form(n, new_query);
    delete_subtree(n);
    delete_subtree(pNode);
    if ( expected == e.what())
      BOOST_REQUIRE(true);
    else
    {
      std::cerr << "Answer : [" << e.what() << "]" << std::endl;
      std::cout << "Expected : ["<< expected << "]" << std::endl;
      std::cout << "new query : ["<< new_query << "]" << std::endl;
      BOOST_REQUIRE(false);
    }
    return;
  }
  
  delete_subtree(n);
  delete_subtree(pNode);

  if ( new_query == expected )
    BOOST_CHECK(true);
  else
  {
    std::cout << "Answer : ["<< new_query << "]" << std::endl;
    std::cout << "Expected : ["<< expected << "]" << std::endl;
    BOOST_CHECK(false);
  }
}

#define BOOST_TEST_DETECT_MEMORY_LEAK 0

BOOST_AUTO_TEST_SUITE(Parser)

BOOST_AUTO_TEST_CASE(existance_simple)
{
  std::string query = "SELECT Z FROM T1 , T2 ;";
  std::string expected = "Problem detected: Parsing: Existance: this COLUMN: Z doesn't exist.";
  std::string baseFilename = "base_test.aq";
  runParserTest( query, expected, baseFilename );
}

BOOST_AUTO_TEST_CASE(ambigue_simple)
{
  std::string query = "SELECT A FROM T1 , T2 ;";
  std::string expected = "Problem detected: Parsing: Ambigue: A exist in defferent Tab -> {T1} - {T2}.";
  std::string baseFilename = "base_test.aq";
  runParserTest( query, expected, baseFilename );
}

BOOST_AUTO_TEST_CASE(KSTAR_asNotFound_simple)
{
  std::string query = "SELECT * FROM ( SELECT * FROM T1 , T2 ) ;";
  std::string expected = "Problem detected: Parsing: STAR failed, no AS found -> [ SELECT  K_STAR ] <-.";
  std::string baseFilename = "base_test.aq";
  runParserTest( query, expected, baseFilename );
}

BOOST_AUTO_TEST_CASE(solveOne_simple)
{
  std::string query = "SELECT A FROM T1 ;";
  std::string expected = "SELECT T1 . A FROM T1";
  std::string baseFilename = "base_test.aq";
  runParserTest( query, expected, baseFilename );
}

BOOST_AUTO_TEST_CASE(solveOne_AS_simple)
{
  std::string query = "SELECT A FROM T1 AS TMP ;";
  std::string expected = "SELECT TMP . A FROM T1 AS TMP";
  std::string baseFilename = "base_test.aq";
  runParserTest( query, expected, baseFilename );
}

BOOST_AUTO_TEST_CASE(solveTwo_simple)
{
  std::string query = "SELECT A , B FROM T1 ;";
  std::string expected = "SELECT T1 . A , T1 . B FROM T1";
  std::string baseFilename = "base_test.aq";
  runParserTest( query, expected, baseFilename );
}

BOOST_AUTO_TEST_CASE(solveTwo_solveStar_simple)
{
  std::string query = "SELECT A , B FROM ( SELECT * FROM T1 ) AS TMP ;";
  std::string expected = "SELECT TMP . A , TMP . B FROM ( SELECT T1 . A , T1 . B FROM T1 ) AS TMP";
  std::string baseFilename = "base_test.aq";
  runParserTest( query, expected, baseFilename );
}

BOOST_AUTO_TEST_CASE(solveOne_One_Two_medium)
{
  std::string query = "SELECT A FROM ( SELECT B FROM ( SELECT A , B FROM T1 AS TMP ) AS TMP1 ) AS TMP2 ;";
  std::string expected = "Problem detected: Parsing: Existance: this COLUMN: A doesn't exist.";
  std::string baseFilename = "base_test.aq";
  runParserTest( query, expected, baseFilename );
}

BOOST_AUTO_TEST_CASE(big_boss)
{
  std::string query = "SELECT T1.A, B, TMP2.A, TMP2.D, E, SUM(T1.A + B - TMP2.E) as v_sum , CASE T1.A	WHEN 1 THEN B	WHEN 2 THEN E END AS t1a_case FROM T1, T2 as TMP2, ( SELECT A	FROM T1	WHERE A = B AND A < 10 AND B IN (1, 2, 3)	ORDER BY A ) AS TMP3,	(	SELECT A, SUM(D) AS AA FROM T2 WHERE A = T2.E GROUP BY A, E ) AS TMP4 WHERE T1.A = TMP4.A	AND B = D	AND TMP2.D = TMP2.E	AND SUBSTRING(D, 4, 3) = T1.B	AND TMP3.A = TMP4.AA GROUP BY T1.A, B, TMP2.A, D, E HAVING v_sum < 10 ORDER BY E;";
  std::string expected = "SELECT T1 . A , T1 . B , TMP2 . A , TMP2 . D , TMP2 . E , SUM ( T1 . A + T1 . B - TMP2 . E ) AS v_sum , T1 . A CASE 1 WHEN T1 . B 2 WHEN TMP2 . E AS t1a_case FROM T1 , T2 AS TMP2 , ( SELECT T1 . A FROM T1 WHERE T1 . A = T1 . B AND T1 . A < 10 AND T1 . B IN 1 , 2 , 3 ORDER  ( T1 . A ) ) AS TMP3 , ( SELECT T2 . A , SUM ( T2 . D ) AS AA FROM T2 WHERE T2 . A = T2 . E GROUP  ( T2 . A , T2 . E ) ) AS TMP4 WHERE T1 . A = TMP4 . A AND T1 . B = TMP4 . D AND TMP2 . D = TMP2 . E AND SUBSTRING ( TMP4 . D , 4 , 3 ) = T1 . B AND TMP3 . A = TMP4 . AA GROUP  ( T1 . A , T1 . B , TMP2 . A , TMP4 . D , TMP2 . E ) HAVING v_sum < 10 ORDER  ( TMP2 . E )";
  std::string baseFilename = "base_test.aq";
  runParserTest( query, expected, baseFilename );
}

BOOST_AUTO_TEST_CASE(max_simple)
{
  std::string query = "SELECT MAX( A ) AS TEST FROM T1 ;";
  std::string expected = "SELECT MAX ( T1 . A ) AS TEST FROM T1";
  std::string baseFilename = "base_test.aq";
  runParserTest( query, expected, baseFilename );
}

BOOST_AUTO_TEST_CASE(max_as_simple)
{
  std::string query = "SELECT MAX( A ) AS TEST FROM T1 AS TMP ;";
  std::string expected = "SELECT MAX ( TMP . A ) AS TEST FROM T1 AS TMP";
  std::string baseFilename = "base_test.aq";
  runParserTest( query, expected, baseFilename );
}

BOOST_AUTO_TEST_CASE(STAR_WHEN_simple)
{
  std::string query = "SELECT * FROM T1 WHERE A = B ;";
  std::string expected = "SELECT T1 . A , T1 . B FROM T1 WHERE T1 . A = T1 . B";
  std::string baseFilename = "base_test.aq";
  runParserTest( query, expected, baseFilename );
}

BOOST_AUTO_TEST_CASE(solveTwoAs_max_as_medium)
{
  std::string query = "SELECT A , B FROM ( SELECT MAX( A ) AS TEST FROM T1 AS TMP1 ) AS TMP2 ;";
  std::string expected = "Problem detected: Parsing: Existance: this COLUMN: B doesn't exist.";
  std::string baseFilename = "base_test.aq";
  runParserTest( query, expected, baseFilename );
}

BOOST_AUTO_TEST_CASE(STAR_2FROM_simple)
{
  std::string query = "SELECT * FROM T1 , T2 ;";
  std::string expected = "SELECT T2 . A , T2 . D , T2 . E , T1 . A , T1 . B FROM T1 , T2";
  std::string baseFilename = "base_test.aq";
  runParserTest( query, expected, baseFilename );
}

BOOST_AUTO_TEST_CASE(solveThree_medium)
{
  std::string query = "SELECT T1.A , B , D , E FROM T1 AS TMP , T2 ;";
  std::string expected = "SELECT T1 . A , TMP . B , T2 . D , T2 . E FROM T1 AS TMP , T2";
  std::string baseFilename = "base_test.aq";
  runParserTest( query, expected, baseFilename );
}

BOOST_AUTO_TEST_CASE(STARAS_STAR_simple)
{
  std::string query = "SELECT * FROM ( SELECT * FROM T1 ) AS TMP ;";
  std::string expected = "SELECT TMP . A , TMP . B FROM ( SELECT T1 . A , T1 . B FROM T1 ) AS TMP";
  std::string baseFilename = "base_test.aq";
  runParserTest( query, expected, baseFilename );
}

BOOST_AUTO_TEST_CASE(solveStar_AS_simple)
{
  std::string query = "SELECT * FROM T1 AS TMP ;";
  std::string expected = "SELECT TMP . A , TMP . B FROM T1 AS TMP";
  std::string baseFilename = "base_test.aq";
  runParserTest( query, expected, baseFilename );
}

BOOST_AUTO_TEST_CASE(solveTwoStar_AS_medium)
{
  std::string query = "SELECT * FROM ( SELECT * FROM T1 AS TMP1 ) AS TMP2 ;";
  std::string expected = "SELECT TMP2 . A , TMP2 . B FROM ( SELECT TMP1 . A , TMP1 . B FROM T1 AS TMP1 ) AS TMP2";
  std::string baseFilename = "base_test.aq";
  runParserTest( query, expected, baseFilename );
}

BOOST_AUTO_TEST_CASE(solveOne_STAR_AS_medium)
{
  std::string query = "SELECT A FROM ( SELECT * FROM T1 AS TMP1 ) AS TMP2 ;";
  std::string expected = "SELECT TMP2 . A FROM ( SELECT TMP1 . A , TMP1 . B FROM T1 AS TMP1 ) AS TMP2";
  std::string baseFilename = "base_test.aq";
  runParserTest( query, expected, baseFilename );
}

BOOST_AUTO_TEST_CASE(table_doesnt_exist)
{
  std::string query = "SELECT A FROM TEST ;";
  std::string expected = "[INVALID_TABLE] cannot find table TEST";
  std::string baseFilename = "base_test.aq";
  runParserTest( query, expected, baseFilename );
}

BOOST_AUTO_TEST_CASE(column_as_sum_simple)
{
  std::string query = "SELECT AA FROM ( SELECT SUM( B ) AS AA FROM T1 AS TMP1 ) AS TMP2 ;";
  std::string expected = "SELECT TMP2 . AA FROM ( SELECT SUM ( TMP1 . B ) AS AA FROM T1 AS TMP1 ) AS TMP2";
  std::string baseFilename = "base_test.aq";
  runParserTest( query, expected, baseFilename );
}

BOOST_AUTO_TEST_CASE(column_as_sum_medium)
{
  std::string query = "SELECT AA FROM ( SELECT SUM( B ) AS AA FROM T1 AS TMP1 ) ;";
  std::string expected = "Problem detected: Parsing: SELECT WITHOUT ALIAS:  -> [ SELECT  TMP1  .  B  SUM  AS  AA ] <-.";
  std::string baseFilename = "base_test.aq";
  runParserTest( query, expected, baseFilename );
}

BOOST_AUTO_TEST_CASE(partition_by)
{
  std::string query = "SELECT A, SUM(A) OVER (PARTITION BY A ORDER BY B) FROM T1 ;";
  std::string expected = "SELECT T1 . A , T1 . A SUM ORDER  ( T1 . B ) PARTITION (  ( T1 . A ) ) FROM T1";
  std::string baseFilename = "base_test.aq";
  runParserTest( query, expected, baseFilename );
}

BOOST_AUTO_TEST_CASE(affichage)
{
  std::string query = "SELECT A, SUM(A + B - A) AS TMP1 FROM T1 AS TMP2 ;";
  std::string expected = "SELECT TMP2 . A , SUM ( TMP2 . A + TMP2 . B - TMP2 . A ) AS TMP1 FROM T1 AS TMP2";
  std::string baseFilename = "base_test.aq";
  runParserTest( query, expected, baseFilename );
}

BOOST_AUTO_TEST_SUITE_END()