#include "stdafx.h"
#include "SQLTypes.h"
#include "Connection.h"
#include <cstdio>
#include <boost/bind.hpp>
#include<boost/tokenizer.hpp>

#ifdef __cplusplus
extern "C" {
#endif

SQLRETURN SQL_API SQLConnect(SQLHDBC ConnectionHandle,
															_In_reads_(NameLength1) SQLCHAR *ServerName, 
															SQLSMALLINT NameLength1,
															_In_reads_(NameLength2) SQLCHAR *UserName, 
															SQLSMALLINT NameLength2,
															_In_reads_(NameLength3) SQLCHAR *Authentication, 
															SQLSMALLINT NameLength3)
{
	AQ_ODBC_LOG("%s called\n", __FUNCTION__);
	return SQL_SUCCESS;
}

RETCODE SQL_API SQLDriverConnect(SQLHDBC         pConn,
																	SQLHWND         pWndHandle,
																	SQLCHAR*       pInConnStr,
																	SQLSMALLINT     pInConnStrLen,
																	SQLCHAR*       pOutConnStr,
																	SQLSMALLINT     pOutConnStrLen,
																	SQLSMALLINT*    pOutConnStrLenPtr,
																	SQLUSMALLINT    pDriverCompletion )
{
	AQ_ODBC_LOG("%s called with: %s\n", __FUNCTION__, pInConnStr);

	if (pInConnStrLen == SQL_NTS)
		pInConnStrLen = (SQLSMALLINT)strlen((const char *)pInConnStr);
		
	if (pInConnStr && (pInConnStrLen > 0))
	{
    if (pOutConnStr == NULL)
      pOutConnStr = static_cast<SQLCHAR*>(::malloc(strlen((const char *)pInConnStr) + 1));
		strcpy((char *)pOutConnStr, (const char *)pInConnStr);
		*pOutConnStrLenPtr = (SQLSMALLINT)strlen((const char *)pInConnStr);
	}
	
	AqHandleConn * c = (AqHandleConn*)pConn;

  std::string connectStr((const char *)pInConnStr);
  boost::char_separator<char> sep(";");
  boost::tokenizer<boost::char_separator<char> > tok(connectStr, sep);
  
  for (boost::tokenizer<boost::char_separator<char> >::iterator it = tok.begin(); it != tok.end(); ++it)
  {
    std::string::size_type pos = (*it).find("=");
    if (pos != std::string::npos) 
    {
      if ((*it).substr(0, pos) == "DATABASE") 
        c->connection->schema = (*it).substr(pos + 1);
      else if ((*it).substr(0, pos) == "AQ_DB_PATH")
        c->connection->db_path = (*it).substr(pos + 1);
      else if ((*it).substr(0, pos) == "AQ_CFG_PATH")
        c->connection->cfg_path = (*it).substr(pos + 1);
      else if ((*it).substr(0, pos) == "AQ_RSLV")
        c->connection->query_resolver = (*it).substr(pos + 1);
      else if ((*it).substr(0, pos) == "SERVER")
        c->connection->server = (*it).substr(pos + 1);
      else if ((*it).substr(0, pos) == "PORT")
        c->connection->port = (*it).substr(pos + 1);
      else if ((*it).substr(0, pos) == "UID")
        c->connection->uid = (*it).substr(pos + 1);
      else if ((*it).substr(0, pos) == "PWD")
        c->connection->pwd = (*it).substr(pos + 1);
    }
  }
	
  AQ_ODBC_LOG("hdbc id: %d [%x]\n", c->Conn, c);
	
	//c->connection->connect("localhost", 9999);

	//c->connection->write("show\n");
	//aq::Connection::buffer_t buf;
	//c->connection->read(buf);

	// c->ioThread.reset(new boost::thread(boost::bind(&boost::asio::io_service::run, c->ioService)));

	return SQL_SUCCESS;
}

SQLRETURN SQL_API SQLGetInfo (SQLHDBC         pConn,
															 SQLUSMALLINT    pInfoType,
															 SQLPOINTER      pInfoValuePtr,
															 SQLSMALLINT     pBufferLength,
															 SQLSMALLINT*    pStringLengthPtr )
{
	AQ_ODBC_LOG("SQLGetInfo called: Field: %d, Length: %d\n", pInfoType, pBufferLength );

	switch (pInfoType)
	{
		// Driver Information
	case SQL_ACTIVE_ENVIRONMENTS: AQ_ODBC_LOG("SQL_ACTIVE_ENVIRONMENTS\n"); break;
	case SQL_FORWARD_ONLY_CURSOR_ATTRIBUTES2: AQ_ODBC_LOG("SQL_FORWARD_ONLY_CURSOR_ATTRIBUTES2\n"); break;
	case SQL_ASYNC_DBC_FUNCTIONS: AQ_ODBC_LOG("SQL_ASYNC_DBC_FUNCTIONS\n"); break;
	case SQL_FILE_USAGE: AQ_ODBC_LOG("SQL_FILE_USAGE\n"); break;
	case SQL_ASYNC_MODE: AQ_ODBC_LOG("SQL_ASYNC_MODE\n"); break;
	case SQL_GETDATA_EXTENSIONS: 
		AQ_ODBC_LOG("SQL_GETDATA_EXTENSIONS\n"); 
		*(( uint32_t* )pInfoValuePtr) = SQL_GD_ANY_COLUMN;
		break;
	case SQL_ASYNC_NOTIFICATION: AQ_ODBC_LOG("SQL_ASYNC_NOTIFICATION\n"); break;
	case SQL_INFO_SCHEMA_VIEWS: AQ_ODBC_LOG("SQL_INFO_SCHEMA_VIEWS\n"); break;
	case SQL_BATCH_ROW_COUNT: AQ_ODBC_LOG("SQL_BATCH_ROW_COUNT\n"); break;
	case SQL_KEYSET_CURSOR_ATTRIBUTES1: AQ_ODBC_LOG("SQL_KEYSET_CURSOR_ATTRIBUTES1\n"); break;
	case SQL_BATCH_SUPPORT: AQ_ODBC_LOG("SQL_BATCH_SUPPORT\n"); break;
	case SQL_KEYSET_CURSOR_ATTRIBUTES2: AQ_ODBC_LOG("SQL_KEYSET_CURSOR_ATTRIBUTES2\n"); break;
	case SQL_DATA_SOURCE_NAME: 
		{
			((SQLCHAR*)pInfoValuePtr)[0] = L'A';
			((SQLCHAR*)pInfoValuePtr)[1] = L'Q';
			((SQLCHAR*)pInfoValuePtr)[2] = L'O';
			((SQLCHAR*)pInfoValuePtr)[3] = L'D';
			((SQLCHAR*)pInfoValuePtr)[4] = L'B';
			((SQLCHAR*)pInfoValuePtr)[5] = L'C';
			*pStringLengthPtr = 6 * sizeof(SQLCHAR);
			AQ_ODBC_LOG("SQL_DATA_SOURCE_NAME: [%s]\n", (SQLCHAR*)pInfoValuePtr);
		}
		break;
	case SQL_MAX_ASYNC_CONCURRENT_STATEMENTS: AQ_ODBC_LOG("SQL_MAX_ASYNC_CONCURRENT_STATEMENTS\n"); break;
	case SQL_DRIVER_AWARE_POOLING_SUPPORTED: AQ_ODBC_LOG("SQL_DRIVER_AWARE_POOLING_SUPPORTED\n"); break;
	case SQL_MAX_CONCURRENT_ACTIVITIES: AQ_ODBC_LOG("SQL_MAX_CONCURRENT_ACTIVITIES\n"); break;
	case SQL_DRIVER_HDBC: AQ_ODBC_LOG("SQL_DRIVER_HDBC\n"); break;
	case SQL_MAX_DRIVER_CONNECTIONS: AQ_ODBC_LOG("SQL_MAX_DRIVER_CONNECTIONS\n"); break;
	case SQL_DRIVER_HDESC: AQ_ODBC_LOG("SQL_DRIVER_HDESC\n"); break;
	case SQL_ODBC_INTERFACE_CONFORMANCE: AQ_ODBC_LOG("SQL_ODBC_INTERFACE_CONFORMANCE\n"); break;
	case SQL_DRIVER_HENV: AQ_ODBC_LOG("SQL_DRIVER_HENV\n"); break;
		// case SQL_ODBC_STANDARD_CLI_CONFORMANCE: AQ_ODBC_LOG("SQL_ODBC_STANDARD_CLI_CONFORMANCE\n"); break;
	case SQL_DRIVER_HLIB: AQ_ODBC_LOG("SQL_DRIVER_HLIB\n"); break;
	case SQL_ODBC_VER: AQ_ODBC_LOG("SQL_ODBC_VER\n"); break;
	case SQL_DRIVER_HSTMT: AQ_ODBC_LOG("SQL_DRIVER_HSTMT\n"); break;
	case SQL_PARAM_ARRAY_ROW_COUNTS: AQ_ODBC_LOG("SQL_PARAM_ARRAY_ROW_COUNTS\n"); break;
	case SQL_DRIVER_NAME: 
		{
			((SQLCHAR*)pInfoValuePtr)[0] = L'a';
			((SQLCHAR*)pInfoValuePtr)[1] = L'q';
			((SQLCHAR*)pInfoValuePtr)[2] = L'o';
			((SQLCHAR*)pInfoValuePtr)[3] = L'd';
			((SQLCHAR*)pInfoValuePtr)[4] = L'b';
			((SQLCHAR*)pInfoValuePtr)[5] = L'c';
			((SQLCHAR*)pInfoValuePtr)[6] = L'.';
			((SQLCHAR*)pInfoValuePtr)[7] = L'd';
			((SQLCHAR*)pInfoValuePtr)[8] = L'l';
			((SQLCHAR*)pInfoValuePtr)[9] = L'l';
			*pStringLengthPtr = 10 * sizeof(SQLCHAR);
			AQ_ODBC_LOG("SQL_DRIVER_NAME: [%s]\n", (SQLCHAR*)pInfoValuePtr);
		}
		break;
	case SQL_PARAM_ARRAY_SELECTS: AQ_ODBC_LOG("SQL_PARAM_ARRAY_SELECTS\n"); break;
	case SQL_DRIVER_ODBC_VER: 
		{
			((SQLCHAR*)pInfoValuePtr)[0] = L'0';
			((SQLCHAR*)pInfoValuePtr)[1] = L'3';
			((SQLCHAR*)pInfoValuePtr)[2] = L'.';
			((SQLCHAR*)pInfoValuePtr)[3] = L'5';
			((SQLCHAR*)pInfoValuePtr)[4] = L'1';
			*pStringLengthPtr = 5 * sizeof(SQLCHAR);
			AQ_ODBC_LOG("SQL_DRIVER_ODBC_VER: [%s]\n", (SQLCHAR*)pInfoValuePtr);
		}
		break;
	case SQL_ROW_UPDATES: AQ_ODBC_LOG("SQL_ROW_UPDATES\n"); break;
	case SQL_DRIVER_VER: AQ_ODBC_LOG("SQL_DRIVER_VER\n"); break;
	case SQL_SEARCH_PATTERN_ESCAPE: 
		{
			((SQLCHAR*)pInfoValuePtr)[0] = L'\\';
			*pStringLengthPtr = 1 * sizeof(SQLCHAR);
			AQ_ODBC_LOG("SQL_SEARCH_PATTERN_ESCAPE\n"); 
		}
		break;
	case SQL_DYNAMIC_CURSOR_ATTRIBUTES1: AQ_ODBC_LOG("SQL_DYNAMIC_CURSOR_ATTRIBUTES1\n"); break;
	case SQL_SERVER_NAME: AQ_ODBC_LOG("SQL_SERVER_NAME\n"); break;
	case SQL_DYNAMIC_CURSOR_ATTRIBUTES2: AQ_ODBC_LOG("SQL_DYNAMIC_CURSOR_ATTRIBUTES2\n"); break;
	case SQL_STATIC_CURSOR_ATTRIBUTES1: AQ_ODBC_LOG("SQL_STATIC_CURSOR_ATTRIBUTES1\n"); break;
	case SQL_FORWARD_ONLY_CURSOR_ATTRIBUTES1: AQ_ODBC_LOG("SQL_FORWARD_ONLY_CURSOR_ATTRIBUTES1\n"); break;
	case SQL_STATIC_CURSOR_ATTRIBUTES2: AQ_ODBC_LOG("SQL_STATIC_CURSOR_ATTRIBUTES2\n"); break;

		// Data Source Information:
	case SQL_ACCESSIBLE_PROCEDURES: AQ_ODBC_LOG("SQL_ACCESSIBLE_PROCEDURES\n"); break;
	case SQL_DATABASE_NAME: 
		{
			((SQLCHAR*)pInfoValuePtr)[0] = L't';
			((SQLCHAR*)pInfoValuePtr)[1] = L'e';
			((SQLCHAR*)pInfoValuePtr)[2] = L's';
			((SQLCHAR*)pInfoValuePtr)[3] = L't';
			*pStringLengthPtr = 4 * sizeof(SQLCHAR);
			AQ_ODBC_LOG("SQL_DATABASE_NAME: [%s]\n", (SQLCHAR*)pInfoValuePtr);
		}
		break;
	case SQL_MULT_RESULT_SETS: AQ_ODBC_LOG("SQL_MULT_RESULT_SETS\n"); break;
	case SQL_ACCESSIBLE_TABLES: AQ_ODBC_LOG("SQL_ACCESSIBLE_TABLES\n"); break;
	case SQL_MULTIPLE_ACTIVE_TXN: AQ_ODBC_LOG("SQL_MULTIPLE_ACTIVE_TXN\n"); break;
	case SQL_BOOKMARK_PERSISTENCE: AQ_ODBC_LOG("SQL_BOOKMARK_PERSISTENCE\n"); break;
	case SQL_NEED_LONG_DATA_LEN: AQ_ODBC_LOG("SQL_NEED_LONG_DATA_LEN\n"); break;
	case SQL_CATALOG_TERM: 
		{
			((SQLCHAR*)pInfoValuePtr)[0] = L'd';
			((SQLCHAR*)pInfoValuePtr)[1] = L'a';
			((SQLCHAR*)pInfoValuePtr)[2] = L't';
			((SQLCHAR*)pInfoValuePtr)[3] = L'a';
			((SQLCHAR*)pInfoValuePtr)[4] = L'b';
			((SQLCHAR*)pInfoValuePtr)[5] = L'a';
			((SQLCHAR*)pInfoValuePtr)[6] = L's';
			((SQLCHAR*)pInfoValuePtr)[7] = L'e';
			*pStringLengthPtr = 8 * sizeof(SQLCHAR);
			AQ_ODBC_LOG("SQL_CATALOG_TERM: [%s]\n", (SQLCHAR*)pInfoValuePtr);
		}
		break;
	case SQL_NULL_COLLATION: AQ_ODBC_LOG("SQL_NULL_COLLATION\n"); break;
	case SQL_COLLATION_SEQ: AQ_ODBC_LOG("SQL_COLLATION_SEQ\n"); break;
	case SQL_PROCEDURE_TERM: AQ_ODBC_LOG("SQL_PROCEDURE_TERM\n"); break;
	case SQL_CONCAT_NULL_BEHAVIOR: AQ_ODBC_LOG("SQL_CONCAT_NULL_BEHAVIOR\n"); break;
	case SQL_SCHEMA_TERM: AQ_ODBC_LOG("SQL_SCHEMA_TERM\n"); break;
	case SQL_CURSOR_COMMIT_BEHAVIOR: 
		AQ_ODBC_LOG("SQL_CURSOR_COMMIT_BEHAVIOR\n"); 
		*(( short* )pInfoValuePtr) = SQL_CB_CLOSE;
		break;
	case SQL_SCROLL_OPTIONS: AQ_ODBC_LOG("SQL_SCROLL_OPTIONS\n"); break;
	case SQL_CURSOR_ROLLBACK_BEHAVIOR: 
		AQ_ODBC_LOG("SQL_CURSOR_ROLLBACK_BEHAVIOR\n"); 
		*(( short* )pInfoValuePtr) = SQL_CB_CLOSE;
		break;
	case SQL_TABLE_TERM: AQ_ODBC_LOG("SQL_TABLE_TERM\n"); break;
	case SQL_CURSOR_SENSITIVITY: AQ_ODBC_LOG("SQL_CURSOR_SENSITIVITY\n"); break;
	case SQL_TXN_CAPABLE: AQ_ODBC_LOG("SQL_TXN_CAPABLE\n"); break;
	case SQL_DATA_SOURCE_READ_ONLY: 
		{
			((SQLCHAR*)pInfoValuePtr)[0] = L'Y';
			*pStringLengthPtr = 1 * sizeof(SQLCHAR);
			AQ_ODBC_LOG("SQL_DATA_SOURCE_READ_ONLY\n"); 
		} 
		break;
	case SQL_TXN_ISOLATION_OPTION: AQ_ODBC_LOG("SQL_TXN_ISOLATION_OPTION\n"); break;
	case SQL_DEFAULT_TXN_ISOLATION: AQ_ODBC_LOG("SQL_DEFAULT_TXN_ISOLATION\n"); break;
	case SQL_USER_NAME: AQ_ODBC_LOG("SQL_USER_NAME\n"); break;
	case SQL_DESCRIBE_PARAMETER: AQ_ODBC_LOG("SQL_DESCRIBE_PARAMETER\n"); break;

		// Supported SQL
	case SQL_AGGREGATE_FUNCTIONS: AQ_ODBC_LOG("SQL_AGGREGATE_FUNCTIONS\n"); break;
	case SQL_DROP_TABLE: AQ_ODBC_LOG("SQL_DROP_TABLE\n"); break;
	case SQL_ALTER_DOMAIN: AQ_ODBC_LOG("SQL_ALTER_DOMAIN\n"); break;
	case SQL_DROP_TRANSLATION: AQ_ODBC_LOG("SQL_DROP_TRANSLATION\n"); break;
		// case SQL_ALTER_SCHEMA: AQ_ODBC_LOG("SQL_ALTER_SCHEMA\n"); break;
	case SQL_DROP_VIEW: AQ_ODBC_LOG("SQL_DROP_VIEW\n"); break;
	case SQL_ALTER_TABLE: AQ_ODBC_LOG("SQL_ALTER_TABLE\n"); break;
	case SQL_EXPRESSIONS_IN_ORDERBY: AQ_ODBC_LOG("SQL_EXPRESSIONS_IN_ORDERBY\n"); break;
		// case SQL_ANSI_SQL_DATETIME_LITERALS: AQ_ODBC_LOG("SQL_ANSI_SQL_DATETIME_LITERALS\n"); break;
	case SQL_GROUP_BY: AQ_ODBC_LOG("SQL_GROUP_BY\n"); break;
	case SQL_CATALOG_LOCATION: AQ_ODBC_LOG("SQL_CATALOG_LOCATION\n"); break;
		// case SQL_IDENTIFIER_case: AQ_ODBC_LOG("SQL_IDENTIFIER_case\n"); break;
	case SQL_CATALOG_NAME: AQ_ODBC_LOG("SQL_CATALOG_NAME\n"); break;
	case SQL_IDENTIFIER_QUOTE_CHAR: 
		{
			((SQLCHAR*)pInfoValuePtr)[0] = L'\'';
			*pStringLengthPtr = 1 * sizeof(SQLCHAR);
			AQ_ODBC_LOG("SQL_IDENTIFIER_QUOTE_CHAR\n"); 
		}
		break;
	case SQL_CATALOG_NAME_SEPARATOR: 
		{
			((SQLCHAR*)pInfoValuePtr)[0] = L'.';
			*pStringLengthPtr = 1 * sizeof(SQLCHAR);
			AQ_ODBC_LOG("SQL_CATALOG_NAME_SEPARATOR\n"); 
		}
		break;
	case SQL_INDEX_KEYWORDS: AQ_ODBC_LOG("SQL_INDEX_KEYWORDS\n"); break;
	case SQL_CATALOG_USAGE: AQ_ODBC_LOG("SQL_CATALOG_USAGE\n"); break;
	case SQL_INSERT_STATEMENT: AQ_ODBC_LOG("SQL_INSERT_STATEMENT\n"); break;
	case SQL_COLUMN_ALIAS: AQ_ODBC_LOG("SQL_COLUMN_ALIAS\n"); break;
	case SQL_INTEGRITY: AQ_ODBC_LOG("SQL_INTEGRITY\n"); break;
	case SQL_CORRELATION_NAME: 
		{
			*(( short* )pInfoValuePtr) = SQL_CN_DIFFERENT;
			AQ_ODBC_LOG("SQL_CORRELATION_NAME\n"); 
		}
		break;
	case SQL_KEYWORDS: AQ_ODBC_LOG("SQL_KEYWORDS\n"); break;
	case SQL_CREATE_ASSERTION: AQ_ODBC_LOG("SQL_CREATE_ASSERTION\n"); break;
	case SQL_LIKE_ESCAPE_CLAUSE: AQ_ODBC_LOG("SQL_LIKE_ESCAPE_CLAUSE\n"); break;
	case SQL_CREATE_CHARACTER_SET: AQ_ODBC_LOG("SQL_CREATE_CHARACTER_SET\n"); break;
	case SQL_NON_NULLABLE_COLUMNS: AQ_ODBC_LOG("SQL_NON_NULLABLE_COLUMNS\n"); break;
	case SQL_CREATE_COLLATION: AQ_ODBC_LOG("SQL_CREATE_COLLATION\n"); break;
	case SQL_SQL_CONFORMANCE: AQ_ODBC_LOG("SQL_SQL_CONFORMANCE\n"); break;
	case SQL_CREATE_DOMAIN: AQ_ODBC_LOG("SQL_CREATE_DOMAIN\n"); break;
	case SQL_OJ_CAPABILITIES: AQ_ODBC_LOG("SQL_OJ_CAPABILITIES\n"); break;
	case SQL_CREATE_SCHEMA: AQ_ODBC_LOG("SQL_CREATE_SCHEMA\n"); break;
	case SQL_ORDER_BY_COLUMNS_IN_SELECT: AQ_ODBC_LOG("SQL_ORDER_BY_COLUMNS_IN_SELECT\n"); break;
	case SQL_CREATE_TABLE: AQ_ODBC_LOG("SQL_CREATE_TABLE\n"); break;
	case SQL_OUTER_JOINS: AQ_ODBC_LOG("SQL_OUTER_JOINS\n"); break;
	case SQL_CREATE_TRANSLATION: AQ_ODBC_LOG("SQL_CREATE_TRANSLATION\n"); break;
	case SQL_PROCEDURES: AQ_ODBC_LOG("SQL_PROCEDURES\n"); break;
	case SQL_DDL_INDEX: AQ_ODBC_LOG("SQL_DDL_INDEX\n"); break;
		// case SQL_QUOTED_IDENTIFIER_case: AQ_ODBC_LOG("SQL_QUOTED_IDENTIFIER_case\n"); break;
	case SQL_DROP_ASSERTION: AQ_ODBC_LOG("SQL_DROP_ASSERTION\n"); break;
	case SQL_SCHEMA_USAGE: AQ_ODBC_LOG("SQL_SCHEMA_USAGE\n"); break;
	case SQL_DROP_CHARACTER_SET: AQ_ODBC_LOG("SQL_DROP_CHARACTER_SET\n"); break;
	case SQL_SPECIAL_CHARACTERS: AQ_ODBC_LOG("SQL_SPECIAL_CHARACTERS\n"); break;
	case SQL_DROP_COLLATION: AQ_ODBC_LOG("SQL_DROP_COLLATION\n"); break;
	case SQL_SUBQUERIES: AQ_ODBC_LOG("SQL_SUBQUERIES\n"); break;
	case SQL_DROP_DOMAIN: AQ_ODBC_LOG("SQL_DROP_DOMAIN\n"); break;
	case SQL_UNION: AQ_ODBC_LOG("SQL_UNION\n"); break;
	case SQL_DROP_SCHEMA: AQ_ODBC_LOG("SQL_DROP_SCHEMA\n"); break;

		// SQL LIMITS
	case  SQL_MAX_BINARY_LITERAL_LEN: AQ_ODBC_LOG(" SQL_MAX_BINARY_LITERAL_LEN\n"); break;
	case SQL_MAX_IDENTIFIER_LEN: AQ_ODBC_LOG("SQL_MAX_IDENTIFIER_LEN\n"); break;
	case SQL_MAX_CATALOG_NAME_LEN: AQ_ODBC_LOG("SQL_MAX_CATALOG_NAME_LEN\n"); break;
	case SQL_MAX_INDEX_SIZE: AQ_ODBC_LOG("SQL_MAX_INDEX_SIZE\n"); break;
	case SQL_MAX_CHAR_LITERAL_LEN: AQ_ODBC_LOG("SQL_MAX_CHAR_LITERAL_LEN\n"); break;
	case SQL_MAX_PROCEDURE_NAME_LEN: AQ_ODBC_LOG("SQL_MAX_PROCEDURE_NAME_LEN\n"); break;
	case SQL_MAX_COLUMN_NAME_LEN: AQ_ODBC_LOG("SQL_MAX_COLUMN_NAME_LEN\n"); break;
	case SQL_MAX_ROW_SIZE: AQ_ODBC_LOG("SQL_MAX_ROW_SIZE\n"); break;
	case SQL_MAX_COLUMNS_IN_GROUP_BY: AQ_ODBC_LOG("SQL_MAX_COLUMNS_IN_GROUP_BY\n"); break;
	case SQL_MAX_ROW_SIZE_INCLUDES_LONG: AQ_ODBC_LOG("SQL_MAX_ROW_SIZE_INCLUDES_LONG\n"); break;
	case SQL_MAX_COLUMNS_IN_INDEX: AQ_ODBC_LOG("SQL_MAX_COLUMNS_IN_INDEX\n"); break;
	case SQL_MAX_SCHEMA_NAME_LEN: AQ_ODBC_LOG("SQL_MAX_SCHEMA_NAME_LEN\n"); break;
	case SQL_MAX_COLUMNS_IN_ORDER_BY: AQ_ODBC_LOG("SQL_MAX_COLUMNS_IN_ORDER_BY\n"); break;
	case SQL_MAX_STATEMENT_LEN: AQ_ODBC_LOG("SQL_MAX_STATEMENT_LEN\n"); break;
	case SQL_MAX_COLUMNS_IN_SELECT: AQ_ODBC_LOG("SQL_MAX_COLUMNS_IN_SELECT\n"); break;
	case SQL_MAX_TABLE_NAME_LEN: AQ_ODBC_LOG("SQL_MAX_TABLE_NAME_LEN\n"); break;
	case SQL_MAX_COLUMNS_IN_TABLE: AQ_ODBC_LOG("SQL_MAX_COLUMNS_IN_TABLE\n"); break;
	case SQL_MAX_TABLES_IN_SELECT: AQ_ODBC_LOG("SQL_MAX_TABLES_IN_SELECT\n"); break;
	case SQL_MAX_CURSOR_NAME_LEN: AQ_ODBC_LOG("SQL_MAX_CURSOR_NAME_LEN\n"); break;
	case SQL_MAX_USER_NAME_LEN: AQ_ODBC_LOG("SQL_MAX_USER_NAME_LEN\n"); break;

		// SCALAR FUNTCION INFORMATION
		// case QL_CONVERT_FUNCTIONS: AQ_ODBC_LOG("QL_CONVERT_FUNCTIONS\n"); break;
	case SQL_TIMEDATE_ADD_INTERVALS: AQ_ODBC_LOG("SQL_TIMEDATE_ADD_INTERVALS\n"); break;
	case SQL_NUMERIC_FUNCTIONS: AQ_ODBC_LOG("SQL_NUMERIC_FUNCTIONS\n"); break;
	case SQL_TIMEDATE_DIFF_INTERVALS: AQ_ODBC_LOG("SQL_TIMEDATE_DIFF_INTERVALS\n"); break;
	case SQL_STRING_FUNCTIONS: AQ_ODBC_LOG("SQL_STRING_FUNCTIONS\n"); break;
	case SQL_TIMEDATE_FUNCTIONS: AQ_ODBC_LOG("SQL_TIMEDATE_FUNCTIONS\n"); break;
	case SQL_SYSTEM_FUNCTIONS: AQ_ODBC_LOG("SQL_SYSTEM_FUNCTIONS\n"); break;

		// CONVERSION INFORMATION
	case SQL_CONVERT_BIGINT: AQ_ODBC_LOG("SQL_CONVERT_BIGINT\n"); break;
	case SQL_CONVERT_LONGVARBINARY: AQ_ODBC_LOG("SQL_CONVERT_LONGVARBINARY\n"); break;
	case SQL_CONVERT_BINARY: AQ_ODBC_LOG("SQL_CONVERT_BINARY\n"); break;
	case SQL_CONVERT_LONGVARCHAR: AQ_ODBC_LOG("SQL_CONVERT_LONGVARCHAR\n"); break;
	case SQL_CONVERT_BIT: AQ_ODBC_LOG("SQL_CONVERT_BIT\n"); break;
	case SQL_CONVERT_NUMERIC: AQ_ODBC_LOG("SQL_CONVERT_NUMERIC\n"); break;
	case SQL_CONVERT_CHAR: AQ_ODBC_LOG("SQL_CONVERT_CHAR\n"); break;
	case SQL_CONVERT_REAL: AQ_ODBC_LOG("SQL_CONVERT_REAL\n"); break;
	case SQL_CONVERT_DATE: AQ_ODBC_LOG("SQL_CONVERT_DATE\n"); break;
	case SQL_CONVERT_SMALLINT: AQ_ODBC_LOG("SQL_CONVERT_SMALLINT\n"); break;
	case SQL_CONVERT_DECIMAL: AQ_ODBC_LOG("SQL_CONVERT_DECIMAL\n"); break;
	case SQL_CONVERT_TIME: AQ_ODBC_LOG("SQL_CONVERT_TIME\n"); break;
	case SQL_CONVERT_DOUBLE: AQ_ODBC_LOG("SQL_CONVERT_DOUBLE\n"); break;
	case SQL_CONVERT_TIMESTAMP: AQ_ODBC_LOG("SQL_CONVERT_TIMESTAMP\n"); break;
	case SQL_CONVERT_FLOAT: AQ_ODBC_LOG("SQL_CONVERT_FLOAT\n"); break;
	case SQL_CONVERT_TINYINT: AQ_ODBC_LOG("SQL_CONVERT_TINYINT\n"); break;
	case SQL_CONVERT_INTEGER: AQ_ODBC_LOG("SQL_CONVERT_INTEGER\n"); break;
	case SQL_CONVERT_VARBINARY: AQ_ODBC_LOG("SQL_CONVERT_VARBINARY\n"); break;
	case SQL_CONVERT_INTERVAL_YEAR_MONTH: AQ_ODBC_LOG("SQL_CONVERT_INTERVAL_YEAR_MONTH\n"); break;
	case SQL_CONVERT_VARCHAR: AQ_ODBC_LOG("SQL_CONVERT_VARCHAR\n"); break;
	case SQL_CONVERT_INTERVAL_DAY_TIME: AQ_ODBC_LOG("SQL_CONVERT_INTERVAL_DAY_TIME\n"); break;

		// ADDED TO ODBC 3.x
	case SQL_DM_VER: AQ_ODBC_LOG("SQL_DM_VER\n"); break;
	case SQL_XOPEN_CLI_YEAR: AQ_ODBC_LOG("SQL_XOPEN_CLI_YEAR\n"); break;

		//
	default: AQ_ODBC_LOG("TYPE NOT SUPPORTED\n"); break;
	}

	return SQL_SUCCESS;
}

#ifdef __cplusplus
}
#endif