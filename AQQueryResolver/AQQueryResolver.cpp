#include <stdio.h>
#include <string.h>
#include <malloc.h>
#include <errno.h>
#include <stdlib.h>						/* _MAX_PATH */
#include "SQLParser/SQLParser.h"
#include "SQLParser/SQLPrefix.h" 
#include "SQLParser/Column2Table.h"
#include "SQLParser/ExprTransform.h"
#include "SQLParser/NestedQueries.h"
#include "SQLParser/Utilities.h"
#include "SQLParser/Exceptions.h"
#include "SQLParser/Log.h"
#include "SQLParser/AQEngine.h"
#include <aq/Logger.h>

#include "Link.h"

//------------------------------------------------------------------------------
/* Next line if commented, application switches to parameter path and file loading */
/* See the two Usage() function content ! */
#define USE_ROOT_DIR_VERSION

//------------------------------------------------------------------------------
/* The thesaurus files will be taken from : 
* - If THESAURUS_BESIDE_DB_DESC defined then the thesaurus path is computed from
* the DB desc. path as follows. From DB path the parent directory is taken (..),
* and "data_orga/vdg/data/" is appended !
* - If THESAURUS_BESIDE_DB_DESC is NOT defined, then the thesaurus path is same as
* the DB desc. path !
*/
#define THESAURUS_BESIDE_DB_DESC

//------------------------------------------------------------------------------
#ifndef _MAX_PATH
#define _MAX_PATH 256
#endif 

//#define CREATE_LOG
//#define OUTPUT_NESTED_QUERIES

#ifdef CREATE_LOG
#include "SQLParser/Log.h"
#endif

//------------------------------------------------------------------------------
extern int yylineno;

//------------------------------------------------------------------------------
int yyerror( const char *pszMsg ) {
#ifdef CREATE_LOG
  Log( "yyerror() : Error: %s encountered at line number: %d !\n", pszMsg, yylineno );
#endif
  {
    char szBuf[ 1000 ];
    sprintf( szBuf, "SQL Parsing Error : %s encountered at line number: %d\n", pszMsg, yylineno );
    ShowError( szBuf );
  }
  return 0;
}

//------------------------------------------------------------------------------
Base BaseDesc;
TProjectSettings	Settings;
AQEngine_Intf * aq_engine;

//------------------------------------------------------------------------------
/* parse and execute the SQL request */
int SQLRequest2PrefixForm( char* pszSQL, TProjectSettings *pSettings ) {
  /* Parse this request and convert it in prefixed polish form */
  /* This is NEEDED by the SQL Parser & Post Processing !!!! */
  tnode			*pNode  = NULL;
  int				nRet;

  if ( pszSQL == NULL )
    return -1;

#ifdef CREATE_LOG
  Log( "\n=====================================================\n"
    "------------------ New Parsing Request ! ------------\n"
    "=====================================================\n" );
#endif

  BaseDesc.loadFromBaseDesc( pSettings->szDBDescFN );

  // Get the SQL Select Statement !
  if ( strlen( pszSQL ) <= 0 ) {
#ifdef CREATE_LOG
    Log( "SQL2Prefix : SQL SELECT Statement TextEdit Control is EMPTY !\n" );
#endif
    ShowError( "Error : No SQL SELECT Statement !" );
    return -1;
  }

#ifdef CREATE_LOG
  Log( "SQL2Prefix : SQL SELECT Statement : \n>>>>>>>>>>\n%s\n<<<<<<<<<<\n", pszSQL );
#endif

	try
	{
		pNode = NULL;
		pSettings->executeNestedQuery = false;
		QueryResolver queryResolver(pNode, pSettings, aq_engine, BaseDesc);
		if ( ( nRet = SQLParse( pszSQL, &pNode ) ) == 0 ) {
			if( (nRet = queryResolver.SolveSQLStatement()) != 0 )
				return -1;
		} else {
#ifdef CREATE_LOG
			Log( "SQL2Prefix : ( SQLParse() != 0 ) Error Parsing the SQL SELECT Statement !\n" );
#endif
		}
		queryResolver.getResult()->saveToAnswer(pSettings->szAnswerFN, pSettings->fieldSeparator);
	}
	catch (const generic_error& ge)
	{
		aq::Logger::getInstance().log(AQ_ERROR, "%s", ge.what());
	}

  if ( pNode != NULL )
    delete_subtree( pNode ); 
  return 0;
}

//------------------------------------------------------------------------------
/* Ret : 0 on success, -1 on error */
int LoadParameters( char szParamFile[ _MAX_PATH ], char szSQLReqFN[ _MAX_PATH ], 
                   char szDBDescFN[ _MAX_PATH ], char szOutputFN[ _MAX_PATH ] ) {
                     FILE* pFIn = NULL;
                     char szTmpBuf[ _MAX_PATH ];
                     char *psz;

                     pFIn = fopenUTF8( szParamFile, "rb" );
                     if ( pFIn == NULL ) {
#ifdef CREATE_LOG
                       Log( "SQL2Prefix : Error Opening Parameters File <%s> !\n", szParamFile );
#endif
                       return -1;
                     }

                     /* First valid line is SQLReq File Path */
                     psz = ReadValidLine( pFIn, szTmpBuf, _MAX_PATH, 0 );
                     if ( psz == NULL ) {
                       fclose( pFIn );
#ifdef CREATE_LOG
                       Log( "SQL2Prefix : Error Reading SQLReq. File Path from Parameter File <%s> !\n", szParamFile );
#endif
                       return -1;
                     }
                     strcpy( szSQLReqFN, psz );

                     /* Second valid line is DB Desc. File Path */
                     psz = ReadValidLine( pFIn, szTmpBuf, _MAX_PATH, 0 );
                     if ( psz == NULL ) {
                       fclose( pFIn );
#ifdef CREATE_LOG
                       Log( "SQL2Prefix : Error Reading DB Desc. File Path from Parameter File <%s> !\n", szParamFile );
#endif
                       return -1;
                     }
                     strcpy( szDBDescFN, psz );

                     /* Third valid line is the Output File Path */
                     psz = ReadValidLine( pFIn, szTmpBuf, _MAX_PATH, 0 );
                     if ( psz == NULL ) {
                       fclose( pFIn );
#ifdef CREATE_LOG
                       Log( "SQL2Prefix : Error Reading Output File Path from Parameter File <%s> !\n", szParamFile );
#endif
                       return -1;
                     }
                     strcpy( szOutputFN, psz );

                     fclose( pFIn );
                     return 0;
}

#ifdef USE_ROOT_DIR_VERSION

//------------------------------------------------------------------------------
/* Ret : 0 on success, -1 on error */
int LoadParametersIni( const char szParamFile[ _MAX_PATH ], 
                      TProjectSettings* pSettings ) {
                        FILE* pFIn = NULL;
                        char *pszLeftSide;
                        char *pszRightSide;
                        char *psz = NULL;
                        char *posChr = NULL;
                        int nFoundRoot = 0;
                        int nFoundEngine = 0;
                        int nFoundSeparator = 0;
                        int nFoundTmpRoot = 0;
                        int nFoundBatch1 = 0;
                        int nFoundBatch2 = 0;

                        pFIn = fopenUTF8( szParamFile, "rb" );
                        if ( pFIn == NULL ) {
#ifdef CREATE_LOG
                          Log( "SQL2Prefix : Error Opening Parameters File <%s> !\n", szParamFile );
#endif
                          return -1;
                        }

												char szBuffer[2048];
                        pSettings->szTempRootPath[0] = '\0';
                        while( psz = ReadValidLine( pFIn, szBuffer, STR_BUF_SIZE, 1 ) )
                        {
                          posChr = strchr(psz, '=');
                          if( posChr == NULL )
                            continue;
                          *posChr = '\0';
                          pszLeftSide = psz;
                          pszRightSide = posChr + 1;
                          --posChr;
                          /* Delete whitespace. */
                          while( *posChr <= ' ' ){ *posChr = '\0'; --posChr; };
                          while( *pszRightSide <= ' ' ) ++pszRightSide;

                          if( strcmp( pszLeftSide, "root.folder" ) == 0 )
                          {
                            pSettings->szRootPath = pszRightSide;
                            nFoundRoot = 1;
                          }
                          else if( strcmp( pszLeftSide, "exeTest.folder" ) == 0 )
                          {
                            pSettings->szEnginePath = pszRightSide;
                            nFoundEngine = 1;
                          } else if( strcmp( pszLeftSide, "step1.field.separator") == 0 )
                          {
                            pSettings->fieldSeparator = pszRightSide[0];
                            nFoundSeparator = 1;
                          } else if( strcmp( pszLeftSide, "k_rep_racine_tmp" ) == 0 )
                          {
                            strcpy( pSettings->szTempRootPath, pszRightSide );
                            nFoundTmpRoot = 1;
                          } else if( strcmp( pszLeftSide, "batch.command.1" ) == 0 )
                          {
                            strcpy( pSettings->szCutInColPath, pszRightSide );
                            char* psz = pSettings->szCutInColPath;
                            while( *psz != '\0' )
                            {
                              if( *psz == ',' )
                                *psz = ' ';
                              ++psz;
                            }
                            nFoundBatch1 = 1;
                          } else if( strcmp( pszLeftSide, "batch.command.2" ) == 0 )
                          {
                            strcpy( pSettings->szLoaderPath, pszRightSide );
                            char* psz = pSettings->szLoaderPath;
                            while( *psz != '\0' )
                            {
                              if( *psz == ',' )
                                *psz = ' ';
                              ++psz;
                            }
                            nFoundBatch2 = 1;
		  } else if( strcmp( pszLeftSide, "step1.format.csv" ) == 0 )
		  {
			  pSettings->csvFormat = atoi(pszRightSide) == 1;
                          }
                        }
                        if( !nFoundTmpRoot )
                          strcpy( pSettings->szTempRootPath, pSettings->szRootPath.c_str() );

                        fclose( pFIn );
                        if( nFoundRoot && nFoundEngine && nFoundSeparator && nFoundBatch1 &&
                          nFoundBatch2 )
                          return 0;
                        return -1;
}

//------------------------------------------------------------------------------
void Usage( char* pszFN ) {
  fprintf( stdout, "%s <ini_file> <randompath>\n", pszFN );
  fprintf( stdout, "\t<ini_file> : ex: /AlgoQuestSuite/DB/ini.properties \n" );
  fprintf( stdout, "\t<randompath> : ex: rnd12345 used to be concated to "
    "/calculus/rnd12345/*Request.txt ... \n" );

  fprintf( stdout, "\tOther Files Accessed :\n" );
  fprintf( stdout, "\t\tAQ Engine    : exeTest.folder \n" );
  fprintf( stdout, "\t\tDB Desc.     : root.folder/base_struct/base. \n" );
  fprintf( stdout, "\t\tDB thesaurus : root.folder/data_orga/vdg/data/*.the or *.the.txt \n" );
  fprintf( stdout, "\t\tSQL Query    : root.folder/calculus/<randompath>/Request.txt \n" );
  fprintf( stdout, "\t\tResult       : root.folder/calculus/<randompath>/New_Request.txt \n" );
  fprintf( stdout, "\t\tTempFolder1  : root.folder/data_orga/tmp/<randompath>/ \n" );
  fprintf( stdout, "\t\tTempFolder2  : root.folder/data_orga/tmp/<randompath>/dpy/ \n" );
}

//------------------------------------------------------------------------------
int main( int argc, char* argv[] ) {
  char *pszSQL		= NULL;
  char *pszResult		= NULL;
  char c;

  /* Parse Arguments */
  if ( argc != 3 ) {
    Usage( argv[ 0 ] );
    return -1;
  }

  /* Load project settings */
  if( LoadParametersIni( argv[1], &Settings ) != 0 )
  {
    ShowError( "Error loading ini file !" );
    return -1;
  }
  c = Settings.szRootPath[ Settings.szRootPath.size() - 1 ];
  if ( c != '\\' && c != '/' )
    Settings.szRootPath += "/";

	//
	// Initialize Logger
	aq::Logger::getInstance(argv[0], STDOUT);
	aq::Logger::getInstance().setLevel(LOG_NOTICE);

  /* Prepare engine arguments */
  sprintf( Settings.szEngineParamsDisplay, "%s %s Dpy", argv[1], argv[2] );
  sprintf( Settings.szEngineParamsNoDisplay, "%s %s NoDpy", argv[1], argv[2] );

  /* Create the file paths */
  strcpy( Settings.szSQLReqFN, Settings.szRootPath.c_str() );
  strcat( Settings.szSQLReqFN, "calculus/" );
  strcat( Settings.szSQLReqFN, argv[2] );
  strcat( Settings.szSQLReqFN, "/Request.txt" );
  strcpy( Settings.szDBDescFN, Settings.szRootPath.c_str() );
  strcat( Settings.szDBDescFN, "base_struct/base." );
  strcpy( Settings.szOutputFN, Settings.szRootPath.c_str() );
  strcat( Settings.szOutputFN, "calculus/" );
  strcat( Settings.szOutputFN, argv[2] );
  strcpy( Settings.szAnswerFN, Settings.szOutputFN );
  strcat( Settings.szOutputFN, "/New_Request.txt" );
  strcat( Settings.szAnswerFN, "/Answer.txt" );
  strcpy( Settings.szThesaurusPath, Settings.szRootPath.c_str() );
  strcat( Settings.szThesaurusPath, "data_orga/vdg/data/" );
  strcpy( Settings.szTempPath1, Settings.szTempRootPath );
  strcat( Settings.szTempPath1, "data_orga/tmp/" );
  strcat( Settings.szTempPath1, argv[2] );
  strcpy( Settings.szTempPath2, Settings.szTempPath1 );
  strcat( Settings.szTempPath2, "/dpy" );

  /* Load SQL Request */
  pszSQL = LoadFile( Settings.szSQLReqFN );
  if ( pszSQL == NULL ) {
    ShowError( "Error Loading SQL Request File !" );
    return -1;
  }

  try
  {
    /* Prepare AQ engine */
    aq_engine = new AQEngine();

    /* SQL to Prefix Form */
    if ( SQLRequest2PrefixForm( pszSQL, &Settings ) != 0 ) {
      ShowError( "Error Parsing and Converting to Prefix Format !" );
      free( pszSQL );
      return -1;
    }
  }
  catch (generic_error& error)
  {
    Log( error.what() );
  }

  return 0;
}

#else /* USE_ROOT_DIR_VERSION */

//------------------------------------------------------------------------------
void Usage( char* pszFN ) {
  fprintf( stdout, "%s [parameter_file_directory] parameter_file_name\n", pszFN );
  fprintf( stdout, "\tparameter_file_directory : ex: user/toto/parameters \n" );
  fprintf( stdout, "\tparameter_file_name      : ex: fname.txt\n" );

  fprintf( stdout, "\tContent of the parameter_file_name :\n" );
  fprintf( stdout, "\t\tsql_query_file_path<NewLine>\n" );
  fprintf( stdout, "\t\tdb_structure_file_path<NewLine>\n" );
  fprintf( stdout, "\t\toutput_file_path\n" );
}

//------------------------------------------------------------------------------
int main( int argc, char* argv[] ) {
  char *pszSQL		= NULL;
  char *pszDB			= NULL;
  char *pszResult		= NULL;
  char szParamFile[ _MAX_PATH ];
  char szSQLReqFN[ _MAX_PATH ];
  char szDBDescFN[ _MAX_PATH ];
  char szOutputFN[ _MAX_PATH ];
  char szThesaurusPath[ _MAX_PATH ];
  char *pszTmp;

  /* Parse Arguments */
  if ( argc < 2 || argc > 3 ) {
    Usage( argv[ 0 ] );
    return -1;
  }

  szParamFile[ 0 ] = '\0';
  strcpy( szParamFile, argv[ 1 ] );
  if ( argc == 3 ) {
    /* We have a directory specified */
    char c = szParamFile[ strlen( szParamFile ) - 1 ];
    if ( c != '\\' && c != '/' )
      strcat( szParamFile, "/" );
    strcat( szParamFile, argv[ 2 ] );
  }

  if ( LoadParameters( szParamFile, szSQLReqFN, szDBDescFN, szOutputFN ) != 0 ) {
    ShowError( "Error Loading Parameter File !" );
    return -1;
  }

  /* Load SQL Request */
  pszSQL = LoadFile( szSQLReqFN );
  if ( pszSQL == NULL ) {
    ShowError( "Error Loading SQL Request File !" );
    return -1;
  }

  /* Load DataBase Structure */
  pszDB = LoadFile( szDBDescFN );
  if ( pszDB == NULL ) {
    ShowError( "Error Loading DataBase Description File !" );
    free( pszSQL );
    return -1;
  }

  /* Create Path to DB directory */
  strcpy( szThesaurusPath, szDBDescFN );
  pszTmp = strrchr( szThesaurusPath, '\\' );
  if ( pszTmp == NULL )
    pszTmp = strrchr( szThesaurusPath, '/' );
  if ( pszTmp != NULL ) {
    /* Cat off the file name, but keep the \ or / ! */
    pszTmp[ 1 ] = '\0';
  }

#ifndef THESAURUS_BESIDE_DB_DESC
  /* Skip last directory ! Like "cd .." */
  pszTmp = strrchr( szThesaurusPath, '\\' );
  if ( pszTmp == NULL )
    pszTmp = strrchr( szThesaurusPath, '/' );
  if ( pszTmp != NULL ) {
    /* Cat off the file name, but keep the \ or / ! */
    pszTmp[ 1 ] = '\0';
  }
  /* Add subdirectories : "data_orga/vdg/data/" */
  strcat( szThesaurusPath, "data_orga/vdg/data/" );
#endif /* THESAURUS_BESIDE_DB_DESC */


  /* SQL to Prefix Form */
  pszResult = SQLRequest2PrefixForm( pszSQL, pszDB, szThesaurusPath );
  if ( pszResult == NULL ) {
    ShowError( "Error Parsing and Converting to Prefix Format !" );
    free( pszSQL );
    free( pszDB );
    return -1;
  }

  /* Save Prefix Form to the Output File */
  if ( SaveFile( szOutputFN, pszResult ) != 0 ) {
    ShowError( "Error Saving Result File (Prefix Format) !" );
    free( pszSQL );
    free( pszDB );
    free( pszResult );
    return -1;
  }

  return 0;
}

#endif /* USE_ROOT_DIR_VERSION */