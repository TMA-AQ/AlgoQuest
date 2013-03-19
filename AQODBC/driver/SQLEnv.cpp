#include "stdafx.h"
#include "SQLTypes.h"
#include <cstdio>

#ifdef __cplusplus
extern "C" {
#endif

// -------------------------------------------------------------------------------------
SQLRETURN  SQL_API SQLAllocStmt(SQLHDBC ConnectionHandle,
																_Out_ SQLHSTMT *StatementHandle)
{
	AQ_ODBC_LOG("%s called\n", __FUNCTION__);
	AQ_ODBC_LOG("connection handler: %d\n", ((AqHandleConn*)ConnectionHandle)->Conn);
	
	if (StatementHandle == 0)
		return SQL_ERROR;
		
  // HGLOBAL hstmt= GlobalAlloc(GMEM_MOVEABLE | GMEM_ZEROINIT, sizeof (AqHandleStmt));
	AqHandleStmt * hstmt = new AqHandleStmt;
	
	if (!hstmt)
	{
		AQ_ODBC_LOG("memory hstmt alloc error\n");
		return SQL_ERROR;
	}
	
	AqHandleStmt * c = (AqHandleStmt*)hstmt;
	c->Stmt = 888;
	c->result = new aq::ResultSet;
	c->conn = (AqHandleConn*)ConnectionHandle;

	*StatementHandle = (HSTMT)hstmt;
	
	AQ_ODBC_LOG("output hstmt address: [%x]\n", StatementHandle);

	return SQL_SUCCESS;
}

// -------------------------------------------------------------------------------------
SQLRETURN  SQL_API SQLAllocConnect(SQLHENV EnvironmentHandle,
																	 _Out_ SQLHDBC *ConnectionHandle)
{
	AQ_ODBC_LOG("%s called\n", __FUNCTION__);
	AQ_ODBC_LOG("envionment handler: %d\n", ((AqHandleEnv*)EnvironmentHandle)->Env);

	if (ConnectionHandle == 0)
		return SQL_ERROR;

  // HGLOBAL hdbc= GlobalAlloc(GMEM_MOVEABLE | GMEM_ZEROINIT, sizeof (AqHandleConn));
	AqHandleConn * hdbc = new AqHandleConn;

	if (!hdbc)
	{
		AQ_ODBC_LOG("memory hdbc alloc error\n");
		return SQL_ERROR;
	}

	AqHandleConn * c = (AqHandleConn*)hdbc;
	c->Conn = 777;
	c->ioService = new boost::asio::io_service;
	c->connection = new aq::Connection(*(c->ioService));
	c->ioThread = NULL;
	c->env = (AqHandleEnv*)EnvironmentHandle;

	*ConnectionHandle = (HDBC)hdbc;
	
	AQ_ODBC_LOG("output hdbc address: [%x]\n", ConnectionHandle);

	return SQL_SUCCESS;
}

// -------------------------------------------------------------------------------------
SQLRETURN  SQL_API SQLAllocEnv(_Out_ SQLHENV *EnvironmentHandle)
{
	AQ_ODBC_LOG("%s called\n", __FUNCTION__);

	if (EnvironmentHandle == 0)
		return SQL_ERROR;

	// HGLOBAL henv= GlobalAlloc(GMEM_MOVEABLE | GMEM_ZEROINIT, sizeof (AqHandleEnv));
	
	AqHandleEnv * henv = new AqHandleEnv;

	if (!henv)
	{
		AQ_ODBC_LOG("memory henv alloc error\n");
		return SQL_ERROR;
	}
	
	henv->Env = 666;

	*EnvironmentHandle = (HENV)henv;
	
	AQ_ODBC_LOG("output henv address: [%x]\n", EnvironmentHandle);

	return SQL_SUCCESS;
}

// -------------------------------------------------------------------------------------
SQLRETURN  SQL_API SQLAllocHandle(SQLSMALLINT HandleType,
																	SQLHANDLE InputHandle, 
																	_Out_ SQLHANDLE *OutputHandle)
{
	AQ_ODBC_LOG("%s called\n", __FUNCTION__);

	switch ( HandleType ) 
	{
	case SQL_HANDLE_ENV:
		return SQLAllocEnv ( OutputHandle );

	case SQL_HANDLE_DBC:
		return SQLAllocConnect ( InputHandle, OutputHandle );

	case SQL_HANDLE_STMT:
		return SQLAllocStmt ( InputHandle, OutputHandle );

	case SQL_HANDLE_DESC:
		AQ_ODBC_LOG("SQLAllocHandle - Explicit descriptor requested - not supported\n");
		return SQL_SUCCESS;
	}
	
	return SQL_ERROR;
}

#ifdef __cplusplus
}
#endif