#include <cstdio>
#include <cstdlib>
#include <iostream>
#include <sstream>
#include <list>

#include <windows.h>
#include <sqlext.h>                                     // required for ODBC calls

#include <aq/Exceptions.h>
#include <aq/Logger.h>
#include <boost/array.hpp>
#include <boost/program_options.hpp>
#include <boost/property_tree/ptree.hpp>
#include <boost/property_tree/ini_parser.hpp>
#include <boost/algorithm/string/trim.hpp>
#include <boost/algorithm/string/replace.hpp>

namespace po = boost::program_options;

// ------------- maximum length to be displayed for a column ------------------
#define _DISPLAY_MAX				64

// -------------------- macro to handle error situations ----------------------
#define ODBC_CHK_ERROR(hType,hValue,iStatus,szMsg)          \
{\
	if ( status != SQL_SUCCESS ) {\
	ShowDiagMessages ( hType, hValue, iStatus, szMsg );\
	/* goto Cleanup; */ \
	}\
	if ( status == SQL_ERROR ) { \
	throw aq::generic_error(aq::generic_error::GENERIC, szMsg);\
	} \
}

// -----------------------------------------------------------------------------
typedef struct BindColInfo 
{
	SQLSMALLINT			    iColTitleSize;				// size of column title
	char*				        szColTitle;					// column title
	SQLLEN	            iColDisplaySize;            // size to display
	char*				        szColData;                  // display buffer  
	SQLLEN              indPtr;                     // size or null indicator
	BOOL                fChar;                      // character col flag
	struct BindColInfo* next;						// linked list
} BIND_COL_INFO;

// -------------------------- function prototypes -----------------------------
void ShowDiagMessages(SQLSMALLINT hType, SQLHANDLE hValue, SQLRETURN iStatus, char* szMsg);
SQLRETURN	ShowDatabases(SQLHANDLE hStmt, SQLHANDLE hConn, std::ostream& os);
SQLRETURN	ShowDatabase(SQLHANDLE hStmt, SQLHANDLE hConn, const char * dbName, std::ostream& os);
SQLRETURN	ShowFullResults(HSTMT hStmt);
void FreeBindings(BIND_COL_INFO* pBindColInfo);
void LoadODBCConnections(const char * odbcConnectionsFile, std::string& odbcConnection);

// ----------------------------------------------------------------------------
int main ( int argc, char* argv[] )
{
	SQLRETURN       status;

	SQLHANDLE       hEnv = 0;
	SQLHANDLE       hConn = 0;
	SQLHANDLE       hStmt = 0;

	SQLCHAR         szConnStrOut[1024];
	SQLSMALLINT     x = 0;
	
	memset(szConnStrOut, 0, 1024 * sizeof(SQLCHAR));

	//
	// log options
	std::string mode;
	std::string ident;
	unsigned int level;
	bool lock_mode = false;
	bool date_mode = false;
	bool pid_mode = false;
  
	//
	// odbc options
	std::string query;
	std::string odbcConnection;
  std::string odbcConnectionsFile;
  bool showDB;

	po::variables_map vm;
	po::options_description desc("Allowed options");
	try
	{
		desc.add_options()
			("help", "produce help message")
			("log-output", po::value<std::string>(&mode)->default_value("STDOUT"), "[STDOUT|LOCALFILE|SYSLOG]")
			("log-level", po::value<unsigned int>(&level)->default_value(AQ_LOG_INFO), "CRITICAL(2), ERROR(3), WARNING(4), NOTICE(5), INFO(6), DEBUG(7)")
			("log-lock", po::bool_switch(&lock_mode), "for multithread program")
			("log-date", po::bool_switch(&date_mode), "add date to log")
			("log-pid", po::bool_switch(&pid_mode), "add thread id to log")
			("log-ident", po::value<std::string>(&ident)->default_value("sql2prefix.log"), "")
			("odbc", po::value<std::string>(&odbcConnection), "ex: \"Driver={AQ ODBC Driver 0};SERVER=locahost;PORT=9999;DATABASE=BNP;UID=tma;PWD=tma;\"")
			("odbc-connections-file", po::value<std::string>(&odbcConnectionsFile)->default_value("odbc-connections.ini"), "Store ODBC Connection string")
			("query", po::value<std::string>(&query), "ex: \"select * from table1;\"")
      ("show-db", po::bool_switch(&showDB), "show description of database")
			;

		po::positional_options_description p;
		p.add("backward-compatibility", -1);

		po::store(po::command_line_parser(argc, argv).options(desc).positional(p).run(), vm);
		po::notify(vm);    

	}
	catch (const std::exception& ex)
	{
		std::cerr << ex.what() << std::endl;
		return EXIT_FAILURE;
	}

	if (vm.count("help"))
	{
		std::cout << desc << "\n";
		return 1;
	}

	//
	// Initialize Logger
	aq::Logger::getInstance(ident.c_str(), mode == "STDOUT" ? STDOUT : mode == "LOCALFILE" ? LOCALFILE : mode == "SYSLOG" ? SYSLOG : STDOUT);
	aq::Logger::getInstance().setLevel(level);
	aq::Logger::getInstance().setLockMode(lock_mode);
	aq::Logger::getInstance().setDateMode(date_mode);
	aq::Logger::getInstance().setPidMode(pid_mode);
  
  try
  {

    // Load Connections file, and store choosen connection in odbcConnection
    if (odbcConnection == "")
      LoadODBCConnections(odbcConnectionsFile.c_str(), odbcConnection);
    
    aq::Logger::getInstance().log(AQ_INFO,  "Starting ODBC client [%s]\n", odbcConnection.c_str());

    // connect to odbc driver and prepare handlers
    status = SQLAllocHandle (SQL_HANDLE_ENV, SQL_NULL_HANDLE, &hEnv);
    ODBC_CHK_ERROR(SQL_HANDLE_ENV, hEnv, status, "");

    status = SQLSetEnvAttr (hEnv,  SQL_ATTR_ODBC_VERSION, (SQLPOINTER)SQL_OV_ODBC3, 0);
    ODBC_CHK_ERROR(SQL_HANDLE_ENV, hEnv, status, "");

    status = SQLAllocHandle (SQL_HANDLE_DBC, hEnv, &hConn);
    ODBC_CHK_ERROR(SQL_HANDLE_DBC, hConn, status, "");

    status = SQLDriverConnect(hConn, SQL_NULL_HANDLE, (SQLCHAR*)odbcConnection.c_str(), SQL_NTS, szConnStrOut, 1024, &x, SQL_DRIVER_NOPROMPT );
    ODBC_CHK_ERROR(SQL_HANDLE_DBC, hConn, status, "");
    aq::Logger::getInstance().log(AQ_INFO,  "connection string length %d [%s]\n", x, szConnStrOut);

    status = SQLAllocHandle ( SQL_HANDLE_STMT, hConn, &hStmt );
    ODBC_CHK_ERROR(SQL_HANDLE_DBC, hConn, status, "");

    if (showDB)
    {
      // show database description
      status = ShowDatabases(hStmt, hConn, std::cout);
    }
    
    if (query != "")
    {
      // execute the statement
      status = SQLExecDirect(hStmt, (SQLCHAR*)query.c_str(), SQL_NTS);
      ODBC_CHK_ERROR(SQL_HANDLE_STMT, hConn, status, "");
    
      // show the full results row by row
      status = ShowFullResults(hStmt);
    }

    // check for error
    ODBC_CHK_ERROR(SQL_HANDLE_STMT, hStmt, status, "");

  }
  catch (const aq::generic_error& e)
  {
    aq::Logger::getInstance().log(AQ_ERROR, e.what());
  }

	// release statement
	if (hStmt)
	{
		SQLFreeStmt(hStmt, SQL_CLOSE);
	}

	// release connection
	if (hConn)
	{
		SQLDisconnect(hConn);
		SQLFreeConnect(hConn);
	}

	// release environment
	if (hEnv) 
	{
		SQLFreeEnv(hEnv);
	}

	return EXIT_SUCCESS;
}

// ----------------------------------------------------------------------------
// to display the database description
// ----------------------------------------------------------------------------

SQLRETURN	ShowDatabases(SQLHANDLE hStmt, SQLHANDLE hConn, std::ostream& os)
{
	SQLRETURN status;
	std::list<std::string> db_names;

  // get databases
  std::string dbName = "%";
  aq::Logger::getInstance().log(AQ_DEBUG, "get tables of base '%s'\n", dbName.c_str());
  status = SQLTables(hStmt, (SQLCHAR*)dbName.c_str(), SQL_NTS, NULL, 0, NULL, 0, (SQLCHAR*)"\'TABLE\',\'VIEW\',\'SYNONYM\'", 24);

  SQLSMALLINT iColCount;
  status = SQLNumResultCols (hStmt, &iColCount);
  ODBC_CHK_ERROR(SQL_HANDLE_STMT, hConn, status, "");

  printf("%u\n", iColCount);
  std::vector<char*> cols;
  cols.resize(iColCount);
  SQLLEN len = 65;
  for (SQLSMALLINT i = 0; i < iColCount; ++i)
  {
    cols[i] = static_cast<char*>(::malloc(len * sizeof(char)));
    ::memset(cols[i], 0, len);
    status = SQLBindCol(hStmt, i + 1, SQL_C_CHAR, (SQLPOINTER) cols[i], len, &len);
    ODBC_CHK_ERROR(SQL_HANDLE_STMT, hConn, status, "");
  }

  while ((status = SQLFetch (hStmt)) != SQL_NO_DATA_FOUND)
  {
    ODBC_CHK_ERROR(SQL_HANDLE_STMT, hConn, status, "");
    for (std::vector<char*>::const_iterator it = cols.begin(); it != cols.end(); ++it)
    {
      aq::Logger::getInstance().log(AQ_DEBUG, "[%s] ", *it);
    }
    aq::Logger::getInstance().log(AQ_DEBUG, "\n");
    db_names.push_back(cols[0]);
  }
 
  for (std::list<std::string>::const_iterator it = db_names.begin(); it != db_names.end(); ++it)
  {
    os << "==========================" << std::endl;
    os << "BASE " << (*it) << std::endl;
    ShowDatabase(hStmt, hConn, (*it).c_str(), os);
  }

  return SQL_SUCCESS;
}

SQLRETURN	ShowDatabase(SQLHANDLE hStmt, SQLHANDLE hConn, const char * dbName, std::ostream& os)
{ 
	SQLRETURN status;
	std::list<std::string> tables_names;

  // get tables
  aq::Logger::getInstance().log(AQ_DEBUG, "get tables of base '%s'\n", dbName);
  status = SQLTables(hStmt, NULL, 0, (SQLCHAR*)dbName, SQL_NTS, NULL, 0, (SQLCHAR*)"\'TABLE\',\'VIEW\',\'SYNONYM\'", 24);

  SQLSMALLINT iColCount;
  status = SQLNumResultCols (hStmt, &iColCount);
  ODBC_CHK_ERROR(SQL_HANDLE_STMT, hConn, status, "");

  std::vector<char*> cols;
  cols.resize(iColCount);
  SQLLEN len = 65;
  for (SQLSMALLINT i = 0; i < iColCount; ++i)
  {
    cols[i] = static_cast<char*>(::malloc(len * sizeof(char)));
    ::memset(cols[i], 0, len);
    status = SQLBindCol(hStmt, i + 1, SQL_C_CHAR, (SQLPOINTER) cols[i], len, &len);
    ODBC_CHK_ERROR(SQL_HANDLE_STMT, hConn, status, "");
  }

  while ((status = SQLFetch (hStmt)) != SQL_NO_DATA_FOUND)
  {
    ODBC_CHK_ERROR(SQL_HANDLE_STMT, hConn, status, "");
    tables_names.push_back(cols[2]);
  }

  // get columns
  SQLCHAR* columnName = static_cast<SQLCHAR*>(::malloc(len * sizeof(SQLCHAR)));
  int      columnType;
  SQLCHAR* columnTypeName = static_cast<SQLCHAR*>(::malloc(len * sizeof(SQLCHAR)));;
  for (std::list<std::string>::const_iterator it = tables_names.begin(); it != tables_names.end(); ++it)
  {
    aq::Logger::getInstance().log(AQ_DEBUG, "get columns of table '%s'\n", (*it).c_str());

    status = SQLColumns(hStmt, (SQLCHAR*)dbName, SQL_NTS, NULL, 0, (SQLCHAR*)(*it).c_str(), SQL_NTS, NULL, 0);
    ODBC_CHK_ERROR(SQL_HANDLE_STMT, hConn, status, "");

    status = SQLBindCol(hStmt, 4, SQL_C_CHAR, (SQLPOINTER) columnName, 65, &len);
    ODBC_CHK_ERROR(SQL_HANDLE_STMT, hConn, status, "");

    status = SQLBindCol(hStmt, 5, SQL_C_SSHORT, (SQLPOINTER) &columnType, sizeof(int), &len);
    ODBC_CHK_ERROR(SQL_HANDLE_STMT, hConn, status, "");
    
    status = SQLBindCol(hStmt, 6, SQL_C_CHAR, (SQLPOINTER) columnTypeName, 65, &len);
    ODBC_CHK_ERROR(SQL_HANDLE_STMT, hConn, status, "");

    os << "\t" << "TABLE " << (*it) << std::endl;
    while ((status = SQLFetch (hStmt)) != SQL_NO_DATA_FOUND)
    {
      os << "\t" << "\t" << columnName << " " << columnType << " " << columnTypeName << std::endl;
      ODBC_CHK_ERROR(SQL_HANDLE_STMT, hConn, status, "");
    }
  }
  
  return SQL_SUCCESS;
}

// ----------------------------------------------------------------------------
// to display the full results row by row
// ----------------------------------------------------------------------------

SQLRETURN	ShowFullResults(SQLHANDLE hStmt)
{
	int					i, iCol;
	size_t      lineSize = 0;

	BIND_COL_INFO*		head = NULL;
	BIND_COL_INFO*		last = NULL;
	BIND_COL_INFO*		curr = NULL;

	SQLRETURN			status;
	SQLLEN			iType;
	SQLSMALLINT			iColCount;

	// initializations
	head = NULL;

	// ALLOCATE SPACE TO FETCH A COMPLETE ROW

	// get number of cols
	if ((status = SQLNumResultCols(hStmt, &iColCount)) != SQL_SUCCESS)
		return status;


	// loop to allocate binding info structure
	for (iCol = 1; iCol <= iColCount; iCol ++) 
  {
		// alloc binding structure
		curr = (BIND_COL_INFO*)calloc(1, sizeof(BIND_COL_INFO));
		if (curr == NULL) 
    {
			fprintf(stderr, "Out of memory!\n");
			return SQL_SUCCESS; // its not an ODBC error so no diags r required
		}

		// maintain link list
		if (iCol == 1)
			head = curr; // first col, therefore head of list
		else
			last->next = curr; // attach

		last = curr; // tail    

    status = SQLColAttribute(hStmt, iCol, SQL_DESC_NAME, NULL, 0, &(curr->iColTitleSize), NULL);
		if ((status != SQL_SUCCESS) && (status != SQL_SUCCESS_WITH_INFO))
    {
			FreeBindings(head);
			return status;
		}
		else 
    {
      ++ curr->iColTitleSize; // allow space for null char
    }

		curr->szColTitle  = (char*)calloc(1, curr->iColTitleSize * sizeof(char));
		if (curr->szColTitle == NULL) 
    {
			FreeBindings(head);
			fprintf(stderr, "Out of memory!\n");
			return SQL_SUCCESS;							// its not an ODBC error so no diags r required
		}

		if ((status = SQLColAttribute(hStmt, iCol, SQL_DESC_NAME, curr->szColTitle, curr->iColTitleSize, &(curr->iColTitleSize), NULL)) != SQL_SUCCESS) 
    {
			FreeBindings(head);
			return status;
		}
		if ((status = SQLColAttribute(hStmt, iCol, SQL_DESC_DISPLAY_SIZE, NULL, 0, NULL, &( curr->iColDisplaySize ))) != SQL_SUCCESS) 
    {
			FreeBindings(head);
			return status;
		}
		printf("| %s ", curr->szColTitle);		// check col type basically char or non-char
		if (curr->iColDisplaySize > _DISPLAY_MAX) 
      curr->iColDisplaySize = _DISPLAY_MAX;
		for (unsigned int i = 0; i < (curr->iColDisplaySize - strlen(curr->szColTitle)); ++i)
			printf(" ");
		curr->szColData = (char*)calloc(1, (curr->iColDisplaySize + 1) * sizeof(char));
		if (curr->szColData == NULL) 
    {
			FreeBindings(head);
			fprintf(stderr, "Out of memory!\n");
			return SQL_SUCCESS;							// its not an ODBC error so no diags r required
		}
		if ((status = SQLColAttribute(hStmt, iCol, SQL_DESC_CONCISE_TYPE, NULL, 0, NULL, &iType)) != SQL_SUCCESS) 
    {
			FreeBindings(head);
			return status;
		}
		curr->fChar = (iType == SQL_CHAR || iType == SQL_VARCHAR || iType == SQL_LONGVARCHAR);
		if ((status = SQLBindCol(hStmt, iCol, SQL_C_CHAR, ( SQLPOINTER )curr->szColData, ( curr->iColDisplaySize+1 ) * sizeof(char), &( curr->indPtr ))) != SQL_SUCCESS) 
    {
			FreeBindings(head);
			return status;
		}
		lineSize += curr->iColDisplaySize + 3;
	}

	lineSize += 2;
	printf(" |\n");

	for (unsigned int i = 0; i < lineSize; ++i)
		printf("-");

	printf("\n");

	// loop to print all the rows one by one
	for (i = 1; true; ++i) 
  {
		if ((status = SQLFetch(hStmt)) == SQL_NO_DATA_FOUND)
			break;													// no more rows so break
		else if (status == SQL_ERROR) // fetch failed 
    {
			FreeBindings(head);
			return status;
		}

		//for ( curr = head, iCol = 0; iCol < iColCount; iCol ++, curr = curr->next )
		//{
		//	printf ( "| %s ", curr->szColData );		// check col type basically char or non-char
		//	for (unsigned int i = 0; i < (curr->iColDisplaySize - strlen(curr->szColData)); ++i)
		//		printf(" ");
		//}
		//printf(" |");
    
		for (curr = head, iCol = 0; iCol < iColCount; iCol ++, curr = curr->next)
		{
			printf("%s ", curr->szColData);		// check col type basically char or non-char
    }
		printf("\n");

	}
	FreeBindings(head);

	return SQL_SUCCESS;
}


// ----------------------------------------------------------------------------
// to free the col info allocated by ShowFullResults
// ----------------------------------------------------------------------------

void FreeBindings(BIND_COL_INFO* pBindColInfo)
{
	BIND_COL_INFO* next;
	if (pBindColInfo) 
  {
		do 
    {
			next = pBindColInfo->next;
			if (pBindColInfo->szColTitle) 
      {
				free(pBindColInfo->szColTitle);
				pBindColInfo->szColTitle = NULL;
			}
			if (pBindColInfo->szColData)
      {
				free(pBindColInfo->szColData);
				pBindColInfo->szColData = NULL;
			}
			free(pBindColInfo);
			pBindColInfo = next;
		} while (pBindColInfo);
	}
}

// ----------------------------------------------------------------------------
// to show the ODBC diagnostic messages
// ----------------------------------------------------------------------------

void ShowDiagMessages(SQLSMALLINT hType, SQLHANDLE hValue, SQLRETURN iStatus, char* szMsg)
{
	SQLSMALLINT	iRec = 0;
	SQLINTEGER	iError;
	SQLCHAR     szMessage[1024];
	SQLCHAR     szState[SQL_SQLSTATE_SIZE + 1];

	printf("\nDiagnostics:\n");
	if (iStatus == SQL_INVALID_HANDLE) 
  {
		fprintf(stderr, "ODBC Error: Invalid handle!\n");
		return;
	}
	while (SQLGetDiagRec(hType, hValue, ++ iRec, szState, &iError, szMessage, 1024, ( SQLSMALLINT* )NULL) == SQL_SUCCESS)
  {
		fprintf ( stderr, "[%5.5s] %s (%d)\n", szState, szMessage, iError );
  }
	printf("\n");
}

void LoadODBCConnections(const char * odbcConnectionsFile, std::string& odbcConnection)
{
  try
  {
    std::ifstream fin(odbcConnectionsFile, std::ifstream::in);
    boost::property_tree::ptree pt;
    boost::property_tree::ini_parser::read_ini(fin, pt);
    odbcConnection = pt.get<std::string>(boost::property_tree::ptree::path_type("active.odbc-connection"));
    odbcConnection = pt.get<std::string>(boost::property_tree::ptree::path_type(odbcConnection));
  }
  catch (const boost::property_tree::ptree_error& e)
  {
    aq::Logger::getInstance().log(AQ_CRITICAL, "cannot load odbc connections ini file: '%s' [error:%s]", odbcConnectionsFile, e.what());
    throw aq::generic_error(aq::generic_error::INVALID_FILE, "cannot load odbc connections ini file");
  }
}