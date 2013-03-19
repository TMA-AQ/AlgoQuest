#include <conio.h>
#include <stdio.h>
#include <tchar.h>
#include <stdlib.h>
#include <iostream>
#include <list>

#include <windows.h>

#include <sqlext.h>                                     // required for ODBC calls

#include <aq/Logger.h>
#include <boost/array.hpp>
#include <boost/program_options.hpp>

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
	goto Cleanup;\
	} \
}

// ---------------------------------- structure -------------------------------
typedef struct BindColInfo {
	SQLSMALLINT			    iColTitleSize;				// size of column title
	char*				        szColTitle;					// column title
	SQLLEN	            iColDisplaySize;            // size to display
	char*				        szColData;                  // display buffer  
	SQLLEN              indPtr;                     // size or null indicator
	BOOL                fChar;                      // character col flag
	struct BindColInfo* next;						// linked list
} BIND_COL_INFO;

// -------------------------- function prototypes -----------------------------
void        ShowDiagMessages        ( SQLSMALLINT hType, SQLHANDLE hValue, SQLRETURN iStatus, char* szMsg );

SQLRETURN	ShowFullResults			( HSTMT hStmt );
void		FreeBindings			( BIND_COL_INFO* pBindColInfo );

// ----------------------------------------------------------------------------
int main ( int argc, char* argv[] )
{
	SQLRETURN       status;

	SQLHANDLE       hEnv = 0;
	SQLHANDLE       hConn = 0;
	SQLHANDLE       hStmt = 0;

	SQLCHAR         szConnStrOut[1024];
	SQLSMALLINT     x;
	
	std::list<std::string> tables_names;

	memset(szConnStrOut, 0, 1024 * sizeof(SQLCHAR));

	//
	// log options
	std::string mode;
	std::string ident;
	unsigned int level;
	unsigned int worker;
	bool lock_mode = false;
	bool date_mode = false;
	bool pid_mode = false;

	//
	// odbc options
	std::string odbcConnection;
	std::string query;

	po::variables_map vm;
	po::options_description desc("Allowed options");
	try
	{
		desc.add_options()
			("help", "produce help message")
			("log-output", po::value<std::string>(&mode)->default_value("STDOUT"), "[STDOUT|LOCALFILE|SYSLOG]")
			("log-level", po::value<unsigned int>(&level)->default_value(LOG_NOTICE), "CRITICAL(2), ERROR(3), WARNING(4), NOTICE(5), INFO(6), DEBUG(7)")
			("log-lock", po::bool_switch(&lock_mode), "for multithread program")
			("log-date", po::bool_switch(&date_mode), "add date to log")
			("log-pid", po::bool_switch(&pid_mode), "add thread id to log")
			("log-ident", po::value<std::string>(&ident)->default_value("sql2prefix.log"), "")
			("odbc", po::value<std::string>(&odbcConnection), "ex: \"Driver={AQ ODBC Driver 0};SERVER=locahost;PORT=9999;DATABASE=test;UID=tma;PWD=tma;\"")
			("query", po::value<std::string>(&query), "ex: \"select * from table1;\"")
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

	// BEFORE U CONNECT

	// allocate ENVIRONMENT
	status = SQLAllocHandle ( SQL_HANDLE_ENV, SQL_NULL_HANDLE, &hEnv );
	ODBC_CHK_ERROR(SQL_HANDLE_ENV, hEnv, status, "");

	// set the ODBC version for behaviour expected
	status = SQLSetEnvAttr ( hEnv,  SQL_ATTR_ODBC_VERSION, (SQLPOINTER)SQL_OV_ODBC3, 0 );
	ODBC_CHK_ERROR(SQL_HANDLE_ENV, hEnv, status, "");

	// allocate CONNECTION
	status = SQLAllocHandle ( SQL_HANDLE_DBC, hEnv, &hConn );
	ODBC_CHK_ERROR(SQL_HANDLE_DBC, hConn, status, "");

	status = SQLDriverConnect ( 
		hConn, 
		SQL_NULL_HANDLE, 
		(SQLCHAR*)odbcConnection.c_str(), 
		SQL_NTS, 
		szConnStrOut, 
		1024, &x, 
		SQL_DRIVER_NOPROMPT );

	ODBC_CHK_ERROR(SQL_HANDLE_DBC, hConn, status, "");
	aq::Logger::getInstance().log(AQ_INFO,  "connection string length %d [%s]\n", x, szConnStrOut);

	// allocate STATEMENT
	status = SQLAllocHandle ( SQL_HANDLE_STMT, hConn, &hStmt );
	ODBC_CHK_ERROR(SQL_HANDLE_DBC, hConn, status, "");

	// retrieve information from the database
	SQLINTEGER l = 65;
	char c1[65];
	char c2[65];
	char c3[65];
	char c4[65];
	char c5[65];
	char c6[65];
	char c7[65];
	char c8[65];

	status = SQLTables(hStmt, (SQLCHAR*)"test", SQL_NTS, NULL, 0, NULL, 0, (SQLCHAR*)"\'TABLE\',\'VIEW\',\'SYNONYM\'", 24);

	SQLSMALLINT iColCount;
	status = SQLNumResultCols (hStmt, &iColCount);
	ODBC_CHK_ERROR(SQL_HANDLE_STMT, hConn, status, "");

	printf("%u\n", iColCount);

	status = SQLBindCol(hStmt, 1, SQL_C_CHAR, (SQLPOINTER) &c1, 65, &l);
	ODBC_CHK_ERROR(SQL_HANDLE_STMT, hConn, status, "");

	status = SQLBindCol(hStmt, 2, SQL_C_CHAR, (SQLPOINTER) &c2, 65, &l);
	ODBC_CHK_ERROR(SQL_HANDLE_STMT, hConn, status, "");

	status = SQLBindCol(hStmt, 3, SQL_C_CHAR, (SQLPOINTER) &c3, 65, &l);
	ODBC_CHK_ERROR(SQL_HANDLE_STMT, hConn, status, "");

	status = SQLBindCol(hStmt, 4, SQL_C_CHAR, (SQLPOINTER) &c4, 65, &l);
	ODBC_CHK_ERROR(SQL_HANDLE_STMT, hConn, status, "");

	status = SQLBindCol(hStmt, 5, SQL_C_CHAR, (SQLPOINTER) &c5, 65, &l);
	ODBC_CHK_ERROR(SQL_HANDLE_STMT, hConn, status, "");

	while ((status = SQLFetch (hStmt)) != SQL_NO_DATA_FOUND)
	{
		// printf("[%s] [%s] [%s] [%s] [%s]\n", c1, c2, c3, c4, c5);
		ODBC_CHK_ERROR(SQL_HANDLE_STMT, hConn, status, "");
		tables_names.push_back(c3);
	}
	
	for (std::list<std::string>::const_iterator it = tables_names.begin(); it != tables_names.end(); ++it)
	{
		status = SQLColumns(hStmt, (SQLCHAR*)"test", 4, NULL, 0, (SQLCHAR*)(*it).c_str(), (*it).size(), NULL, 0);
		ODBC_CHK_ERROR(SQL_HANDLE_STMT, hConn, status, "");

		status = SQLBindCol(hStmt, 4, SQL_C_CHAR, (SQLPOINTER) &c1, 65, &l);
		ODBC_CHK_ERROR(SQL_HANDLE_STMT, hConn, status, "");

		status = SQLBindCol(hStmt, 5, SQL_C_CHAR, (SQLPOINTER) &c2, 65, &l);
		ODBC_CHK_ERROR(SQL_HANDLE_STMT, hConn, status, "");

		printf("%s\n", (*it).c_str());
		while ((status = SQLFetch (hStmt)) != SQL_NO_DATA_FOUND)
		{
			printf("\t[%s] [%s]\n", c1, c2);
			ODBC_CHK_ERROR(SQL_HANDLE_STMT, hConn, status, "");
		}
	}

	exit(0);

	// execute the statement
	status = SQLExecDirect ( hStmt, (SQLCHAR*)query.c_str(), SQL_NTS );
	ODBC_CHK_ERROR(SQL_HANDLE_STMT, hConn, status, "");

	// show the full results row by row
	status = ShowFullResults ( hStmt );

	// check for error
	ODBC_CHK_ERROR(SQL_HANDLE_STMT, hStmt, status, "");

Cleanup:

	// release statement
	if ( hStmt )
	{
		SQLFreeStmt ( hStmt, SQL_CLOSE );
	}

	// release connection
	if ( hConn )
	{
		SQLDisconnect ( hConn );

		SQLFreeConnect ( hConn );
	}

	// release environment
	if ( hEnv ) 
	{
		SQLFreeEnv ( hEnv );
	}

	return EXIT_SUCCESS;
}

// ----------------------------------------------------------------------------
// to display the full results row by row
// ----------------------------------------------------------------------------

SQLRETURN	ShowFullResults ( SQLHANDLE hStmt )
{
	int					i, iCol;
	int					rows;
	int         lineSize = 0;

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
	if (( status = SQLNumResultCols ( hStmt, &iColCount )) != SQL_SUCCESS )
		return status;


	// loop to allocate binding info structure
	for ( iCol = 1; iCol <= iColCount; iCol ++ ) {

		// alloc binding structure
		curr = ( BIND_COL_INFO* )calloc ( 1, sizeof(BIND_COL_INFO));
		if ( curr == NULL ) {
			fprintf ( stderr, "Out of memory!\n" );
			return SQL_SUCCESS;							// its not an ODBC error so no diags r required
		}

		// maintain link list
		if ( iCol == 1 )
			head = curr;								// first col, therefore head of list
		else
			last->next = curr;							// attach

		last = curr;									// tail    

		// get column title size
		if (( status = SQLColAttribute ( hStmt, iCol, SQL_DESC_NAME, NULL, 0, &( curr->iColTitleSize ), NULL )) != SQL_SUCCESS ) {
			FreeBindings ( head );
			return status;
		}
		else ++ curr->iColTitleSize;					// allow space for null char

		// allocate buffer for title
		curr->szColTitle  = ( char* ) calloc ( 1, curr->iColTitleSize * sizeof( char ));
		if ( curr->szColTitle == NULL ) {
			FreeBindings ( head );
			fprintf ( stderr, "Out of memory!\n" );
			return SQL_SUCCESS;							// its not an ODBC error so no diags r required
		}

		// get column title
		if (( status = SQLColAttribute ( hStmt, iCol, SQL_DESC_NAME, curr->szColTitle, curr->iColTitleSize, &( curr->iColTitleSize ), NULL )) != SQL_SUCCESS ) {
			FreeBindings ( head );
			return status;
		}
		// get col length 
		if (( status = SQLColAttribute ( hStmt, iCol, SQL_DESC_DISPLAY_SIZE, NULL, 0, NULL, &( curr->iColDisplaySize ))) != SQL_SUCCESS ) {
			FreeBindings ( head );
			return status;
		}

		// print column title
		printf ( "| %s ", curr->szColTitle );		// check col type basically char or non-char

		// arbitrary limit on display size
		if ( curr->iColDisplaySize > _DISPLAY_MAX )  curr->iColDisplaySize = _DISPLAY_MAX;

		for (unsigned int i = 0; i < (curr->iColDisplaySize - strlen(curr->szColTitle)); ++i)
			printf(" ");

		// allocate buffer for col data + NULL terminator
		curr->szColData = ( char* ) calloc ( 1, ( curr->iColDisplaySize + 1 ) * sizeof(char));
		if ( curr->szColData == NULL ) {
			FreeBindings ( head );
			fprintf ( stderr, "Out of memory!\n" );
			return SQL_SUCCESS;							// its not an ODBC error so no diags r required
		}

		// get col type, not used now but can be checked to print value right aligned etcc
		if (( status = SQLColAttribute ( hStmt, iCol, SQL_DESC_CONCISE_TYPE, NULL, 0, NULL, &iType )) != SQL_SUCCESS ) {
			FreeBindings ( head );
			return status;
		}

		// set col type indicator in struct
		curr->fChar = ( iType == SQL_CHAR || iType == SQL_VARCHAR || iType == SQL_LONGVARCHAR );

		// bind the col buffer so that the driver feeds it with col value on every fetch and use generic char binding for every column
		if (( status = SQLBindCol ( hStmt, iCol, SQL_C_CHAR, ( SQLPOINTER )curr->szColData, ( curr->iColDisplaySize+1 ) * sizeof(char), &( curr->indPtr ))) != SQL_SUCCESS ) {
			FreeBindings ( head );
			return status;
		}

		lineSize += curr->iColDisplaySize + 3;
	}

	lineSize += 2;
	printf(" |\n");

	for (unsigned int i = 0; i < lineSize; ++i)
		printf("-");

	printf("\n");

	// number of rows to show in one shot
	rows = 1;

	// loop to print all the rows one by one
	for ( i = 1, rows = 1; true; i ++ ) {

		// fetch the next row
		if (( status = SQLFetch ( hStmt )) == SQL_NO_DATA_FOUND )
			break;													// no more rows so break

		// check for error
		else if ( status == SQL_ERROR ) {							// fetch failed
			FreeBindings ( head );
			return status;
		}


		for ( curr = head, iCol = 0; iCol < iColCount; iCol ++, curr = curr->next )
		{
			printf ( "| %s ", curr->szColData );		// check col type basically char or non-char

			for (unsigned int i = 0; i < (curr->iColDisplaySize - strlen(curr->szColData)); ++i)
				printf(" ");

		}

		printf(" |");

		// row separator
		printf ( "\n" );

	}

	// free the allocated bindings
	FreeBindings ( head );

	return SQL_SUCCESS;
}


// ----------------------------------------------------------------------------
// to free the col info allocated by ShowFullResults
// ----------------------------------------------------------------------------

void FreeBindings ( BIND_COL_INFO* pBindColInfo )
{
	BIND_COL_INFO* next;

	// precaution 
	if ( pBindColInfo ) {

		do {

			// get the next col binding
			next = pBindColInfo->next;

			// free any buffer for col title
			if ( pBindColInfo->szColTitle ) {
				free ( pBindColInfo->szColTitle );
				pBindColInfo->szColTitle = NULL;
			}

			// free any col data
			if ( pBindColInfo->szColData ) {
				free ( pBindColInfo->szColData );
				pBindColInfo->szColData = NULL;
			}

			// free the current binding
			free ( pBindColInfo );

			// make next the current
			pBindColInfo = next;

		} while ( pBindColInfo );
	}
}

// ----------------------------------------------------------------------------
// to show the ODBC diagnostic messages
// ----------------------------------------------------------------------------

void ShowDiagMessages ( SQLSMALLINT hType, SQLHANDLE hValue, SQLRETURN iStatus, char* szMsg )
{
	SQLSMALLINT	iRec = 0;
	SQLINTEGER	iError;
	SQLCHAR     szMessage[1024];
	SQLCHAR     szState[SQL_SQLSTATE_SIZE + 1];

	// header
	printf ( "\nDiagnostics:\n" );

	// in case of an invalid handle, no message can be extracted
	if ( iStatus == SQL_INVALID_HANDLE ) {
		fprintf ( stderr, "ODBC Error: Invalid handle!\n" );
		return;
	}

	// loop to get all diag messages from driver/driver manager
	while ( SQLGetDiagRec ( hType, hValue, ++ iRec, szState, &iError, szMessage, 1024, ( SQLSMALLINT* )NULL) == SQL_SUCCESS )
		fprintf ( stderr, "[%5.5s] %s (%d)\n", szState, szMessage, iError );
	// fwprintf ( stderr, TEXT("%s (%d)\n"), szMessage, iError );

	// gap
	printf ( "\n" );
}

