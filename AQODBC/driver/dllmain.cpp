// dllmain.cpp : Définit le point d'entrée pour l'application DLL.
#include "stdafx.h"

#include <sqlext.h>								// core ODBC functions
#include <odbcinst.h>
#include <sqlucode.h>

#include <cstdio>
#include <sstream>

#include <aq/Logger.h>

#ifdef __cplusplus
extern "C" {
#endif

BOOL APIENTRY DllMain( HMODULE hModule,
											DWORD  ul_reason_for_call,
											LPVOID lpReserved
											)
{
	aq::Logger::getInstance().setLevel(LOG_INFO);

	switch (ul_reason_for_call)
	{
	case DLL_PROCESS_ATTACH:
		AQ_ODBC_LOG("DLL_PROCESS_ATTACH\n");
		break;

	case DLL_THREAD_ATTACH: 
		AQ_ODBC_LOG("DLL_THREAD_ATTACH\n");
		break;

	case DLL_THREAD_DETACH:
		AQ_ODBC_LOG("DLL_THREAD_DETACH\n");
		break;
	case DLL_PROCESS_DETACH:
		AQ_ODBC_LOG("DLL_PROCESS_DETACH\n");
		break;
	}
	return TRUE;
}

/////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////

SQLRETURN  SQL_API SQLBindParam(SQLHSTMT StatementHandle,
																SQLUSMALLINT ParameterNumber, SQLSMALLINT ValueType,
																SQLSMALLINT ParameterType, SQLULEN LengthPrecision,
																SQLSMALLINT ParameterScale, SQLPOINTER ParameterValue,
																SQLLEN *StrLen_or_Ind)
{
	AQ_ODBC_LOG("%s called\n", __FUNCTION__);
	return SQL_SUCCESS;
}

SQLRETURN  SQL_API SQLCancel(SQLHSTMT StatementHandle)
{
	AQ_ODBC_LOG("%s called\n", __FUNCTION__);
	return SQL_SUCCESS;
}

SQLRETURN  SQL_API SQLCancelHandle(SQLSMALLINT HandleType, SQLHANDLE InputHandle)
{
	AQ_ODBC_LOG("%s called\n", __FUNCTION__);
	return SQL_SUCCESS;
}

SQLRETURN  SQL_API SQLCloseCursor(SQLHSTMT StatementHandle)
{
	AQ_ODBC_LOG("%s called\n", __FUNCTION__);
	return SQL_SUCCESS;
}

SQLRETURN SQL_API SQLCompleteAsync(SQLSMALLINT    HandleType,
																	 SQLHANDLE      Handle, 
																	 _Out_ RETCODE* AsyncRetCodePtr)
{
	AQ_ODBC_LOG("%s called\n", __FUNCTION__);
	return SQL_SUCCESS;
}

SQLRETURN  SQL_API SQLCopyDesc(SQLHDESC SourceDescHandle,
															 SQLHDESC TargetDescHandle)
{
	AQ_ODBC_LOG("%s called\n", __FUNCTION__);
	return SQL_SUCCESS;
}

SQLRETURN  SQL_API SQLDataSources(SQLHENV EnvironmentHandle,
																	SQLUSMALLINT Direction, _Out_writes_opt_(BufferLength1) SQLCHAR *ServerName,
																	SQLSMALLINT BufferLength1, _Out_opt_ SQLSMALLINT *NameLength1Ptr,
																	_Out_writes_opt_(BufferLength2) SQLCHAR *Description, SQLSMALLINT BufferLength2,
																	_Out_opt_ SQLSMALLINT *NameLength2Ptr)
{
	AQ_ODBC_LOG("%s called\n", __FUNCTION__);
	return SQL_SUCCESS;
}

SQLRETURN  SQL_API SQLDescribeCol(SQLHSTMT StatementHandle,
																	SQLUSMALLINT ColumnNumber, _Out_writes_opt_(BufferLength) SQLCHAR *ColumnName,
																	SQLSMALLINT BufferLength, _Out_opt_ SQLSMALLINT *NameLength,
																	_Out_opt_ SQLSMALLINT *DataType, _Out_opt_ SQLULEN *ColumnSize,
																	_Out_opt_ SQLSMALLINT *DecimalDigits, _Out_opt_ SQLSMALLINT *Nullable)
{
	AQ_ODBC_LOG("%s called\n", __FUNCTION__);
	return SQL_SUCCESS;
}

SQLRETURN  SQL_API SQLDisconnect(SQLHDBC ConnectionHandle)
{
	AQ_ODBC_LOG("%s called\n", __FUNCTION__);
	return SQL_SUCCESS;
}

SQLRETURN  SQL_API SQLEndTran(SQLSMALLINT HandleType, SQLHANDLE Handle,
															SQLSMALLINT CompletionType)
{
	AQ_ODBC_LOG("%s called\n", __FUNCTION__);
	return SQL_SUCCESS;
}

SQLRETURN  SQL_API SQLError(SQLHENV EnvironmentHandle,
														SQLHDBC ConnectionHandle, SQLHSTMT StatementHandle,
														_Out_writes_(6) SQLCHAR *Sqlstate, _Out_opt_ SQLINTEGER *NativeError,
														_Out_writes_opt_(BufferLength) SQLCHAR *MessageText, SQLSMALLINT BufferLength,
														_Out_opt_ SQLSMALLINT *TextLength)
{
	AQ_ODBC_LOG("%s called\n", __FUNCTION__);
	return SQL_SUCCESS;
}

SQLRETURN  SQL_API SQLExecute(SQLHSTMT StatementHandle)
{
	AQ_ODBC_LOG("%s called\n", __FUNCTION__);
	return SQL_SUCCESS;
}

SQLRETURN  SQL_API SQLFetchScroll(SQLHSTMT StatementHandle,
																	SQLSMALLINT FetchOrientation, SQLLEN FetchOffset)
{
	AQ_ODBC_LOG("%s called\n", __FUNCTION__);
	return SQL_SUCCESS;
}

SQLRETURN  SQL_API SQLFreeConnect(SQLHDBC ConnectionHandle)
{
	AQ_ODBC_LOG("%s called\n", __FUNCTION__);
	return SQL_SUCCESS;
}

SQLRETURN  SQL_API SQLFreeEnv(SQLHENV EnvironmentHandle)
{
	AQ_ODBC_LOG("%s called\n", __FUNCTION__);
	return SQL_SUCCESS;
}

SQLRETURN  SQL_API SQLFreeHandle(SQLSMALLINT HandleType, SQLHANDLE Handle)
{
	AQ_ODBC_LOG("%s called\n", __FUNCTION__);
	return SQL_SUCCESS;
}

SQLRETURN  SQL_API SQLFreeStmt(SQLHSTMT StatementHandle,
															 SQLUSMALLINT Option)
{
	AQ_ODBC_LOG("%s called\n", __FUNCTION__);
	return SQL_SUCCESS;
}

SQLRETURN  SQL_API SQLGetConnectAttr(SQLHDBC ConnectionHandle,
																		 SQLINTEGER Attribute, _Out_writes_opt_(_Inexpressible_(BufferLength)) SQLPOINTER Value,
																		 SQLINTEGER BufferLength, _Out_opt_ SQLINTEGER *StringLengthPtr)
{
	AQ_ODBC_LOG("%s called\n", __FUNCTION__);
	return SQL_SUCCESS;
}

SQLRETURN  SQL_API SQLGetConnectOption(SQLHDBC ConnectionHandle,
																			 SQLUSMALLINT Option, SQLPOINTER Value)
{
	AQ_ODBC_LOG("%s called\n", __FUNCTION__);
	return SQL_SUCCESS;
}

SQLRETURN  SQL_API SQLGetCursorName(SQLHSTMT StatementHandle,
																		_Out_writes_opt_(BufferLength) SQLCHAR *CursorName,
																		SQLSMALLINT BufferLength,
																		_Out_opt_
																		SQLSMALLINT *NameLengthPtr)
{
	AQ_ODBC_LOG("%s called\n", __FUNCTION__);
	return SQL_SUCCESS;
}

SQLRETURN  SQL_API SQLGetData(SQLHSTMT StatementHandle,
															SQLUSMALLINT ColumnNumber, SQLSMALLINT TargetType,
															_Out_writes_opt_(_Inexpressible_(BufferLength)) SQLPOINTER TargetValue, SQLLEN BufferLength,
															_Out_opt_ SQLLEN *StrLen_or_IndPtr)
{
	AQ_ODBC_LOG("%s called\n", __FUNCTION__);
	return SQL_SUCCESS;
}

SQLRETURN  SQL_API SQLGetDescField(SQLHDESC DescriptorHandle,
																	 SQLSMALLINT RecNumber, SQLSMALLINT FieldIdentifier,
																	 _Out_writes_opt_(_Inexpressible_(BufferLength)) SQLPOINTER Value, SQLINTEGER BufferLength,
																	 _Out_opt_ SQLINTEGER *StringLength)
{
	AQ_ODBC_LOG("%s called\n", __FUNCTION__);
	return SQL_SUCCESS;
}

SQLRETURN  SQL_API SQLGetDescRec(SQLHDESC DescriptorHandle,
																 SQLSMALLINT RecNumber, _Out_writes_opt_(BufferLength) SQLCHAR *Name,
																 SQLSMALLINT BufferLength, _Out_opt_ SQLSMALLINT *StringLengthPtr,
																 _Out_opt_ SQLSMALLINT *TypePtr, _Out_opt_ SQLSMALLINT *SubTypePtr,
																 _Out_opt_ SQLLEN     *LengthPtr, _Out_opt_ SQLSMALLINT *PrecisionPtr,
																 _Out_opt_ SQLSMALLINT *ScalePtr, _Out_opt_ SQLSMALLINT *NullablePtr)
{
	AQ_ODBC_LOG("%s called\n", __FUNCTION__);
	return SQL_SUCCESS;
}

SQLRETURN  SQL_API SQLGetStmtOption(SQLHSTMT StatementHandle,
																		SQLUSMALLINT Option, SQLPOINTER Value)
{
	AQ_ODBC_LOG("%s called\n", __FUNCTION__);
	return SQL_SUCCESS;
}

SQLRETURN  SQL_API SQLParamData(SQLHSTMT StatementHandle,
																_Out_opt_ SQLPOINTER *Value)
{
	AQ_ODBC_LOG("%s called\n", __FUNCTION__);
	return SQL_SUCCESS;
}

SQLRETURN  SQL_API SQLPrepare(SQLHSTMT StatementHandle,
															_In_reads_(TextLength) SQLCHAR* StatementText,
															SQLINTEGER TextLength)
{
	AQ_ODBC_LOG("%s called\n", __FUNCTION__);
	return SQL_SUCCESS;
}

SQLRETURN  SQL_API SQLPutData(SQLHSTMT StatementHandle,
															_In_reads_(_Inexpressible_(StrLen_or_Ind)) SQLPOINTER Data, SQLLEN StrLen_or_Ind)
{
	AQ_ODBC_LOG("%s called\n", __FUNCTION__);
	return SQL_SUCCESS;
}

SQLRETURN  SQL_API SQLRowCount(_In_ SQLHSTMT StatementHandle,
															 _Out_ SQLLEN* RowCount)
{
	AQ_ODBC_LOG("%s called\n", __FUNCTION__);
	return SQL_SUCCESS;
}

SQLRETURN  SQL_API SQLSetConnectAttr(SQLHDBC ConnectionHandle,
																		 SQLINTEGER Attribute, _In_reads_bytes_opt_(StringLength) SQLPOINTER Value,
																		 SQLINTEGER StringLength)
{
	AQ_ODBC_LOG("%s called\n", __FUNCTION__);
	return SQL_SUCCESS;
}

SQLRETURN  SQL_API SQLSetConnectOption(SQLHDBC ConnectionHandle,
																			 SQLUSMALLINT Option, SQLULEN Value)
{
	AQ_ODBC_LOG("%s called\n", __FUNCTION__);
	return SQL_SUCCESS;
}

SQLRETURN  SQL_API SQLSetCursorName(SQLHSTMT StatementHandle,
																		_In_reads_(NameLength) SQLCHAR* CursorName,
																		SQLSMALLINT NameLength)
{
	AQ_ODBC_LOG("%s called\n", __FUNCTION__);
	return SQL_SUCCESS;
}

SQLRETURN  SQL_API SQLSetDescField(SQLHDESC DescriptorHandle,
																	 SQLSMALLINT RecNumber, SQLSMALLINT FieldIdentifier,
																	 _In_reads_(_Inexpressible_(BufferLength)) SQLPOINTER Value, SQLINTEGER BufferLength)
{
	AQ_ODBC_LOG("%s called\n", __FUNCTION__);
	return SQL_SUCCESS;
}

SQLRETURN  SQL_API SQLSetDescRecW(SQLHDESC DescriptorHandle,
																	SQLSMALLINT RecNumber, SQLSMALLINT Type,
																	SQLSMALLINT SubType, SQLLEN Length,
																	SQLSMALLINT Precision, SQLSMALLINT Scale,
																	_Inout_updates_bytes_opt_(Length) SQLPOINTER Data, _Inout_opt_ SQLLEN *StringLength,
																	_Inout_opt_ SQLLEN *Indicator)
{
	AQ_ODBC_LOG("%s called\n", __FUNCTION__);
	return SQL_SUCCESS;
}

SQLRETURN  SQL_API SQLSetEnvAttr(SQLHENV EnvironmentHandle,
																 SQLINTEGER Attribute, _In_reads_bytes_opt_(StringLength) SQLPOINTER Value,
																 SQLINTEGER StringLength)
{
	AQ_ODBC_LOG("%s called\n", __FUNCTION__);
	return SQL_SUCCESS;
}

SQLRETURN  SQL_API SQLSetParam(SQLHSTMT StatementHandle,
															 SQLUSMALLINT ParameterNumber, SQLSMALLINT ValueType,
															 SQLSMALLINT ParameterType, SQLULEN LengthPrecision,
															 SQLSMALLINT ParameterScale, _In_reads_(_Inexpressible_(*StrLen_or_Ind)) SQLPOINTER ParameterValue,
															 _Inout_ SQLLEN *StrLen_or_Ind)
{
	AQ_ODBC_LOG("%s called\n", __FUNCTION__);
	return SQL_SUCCESS;
}

SQLRETURN  SQL_API SQLSetStmtAttr(SQLHSTMT StatementHandle,
																	SQLINTEGER Attribute, _In_reads_(_Inexpressible_(StringLength)) SQLPOINTER Value,
																	SQLINTEGER StringLength)
{
	AQ_ODBC_LOG("%s called\n", __FUNCTION__);
	return SQL_SUCCESS;
}

SQLRETURN  SQL_API SQLSetStmtOption(SQLHSTMT StatementHandle,
																		SQLUSMALLINT Option, SQLULEN Value)
{
	AQ_ODBC_LOG("%s called\n", __FUNCTION__);
	return SQL_SUCCESS;
}

SQLRETURN  SQL_API SQLSpecialColumns(SQLHSTMT StatementHandle,
																		 SQLUSMALLINT IdentifierType, 
																		 _In_reads_opt_(NameLength1) SQLCHAR *CatalogName, SQLSMALLINT NameLength1,
																		 _In_reads_opt_(NameLength2) SQLCHAR *SchemaName, SQLSMALLINT NameLength2, 
																		 _In_reads_opt_(NameLength3) SQLCHAR *TableName, SQLSMALLINT NameLength3, 
																		 SQLUSMALLINT Scope, SQLUSMALLINT Nullable)
{
	AQ_ODBC_LOG("%s called\n", __FUNCTION__);
	return SQL_SUCCESS;
}

SQLRETURN  SQL_API SQLStatistics(SQLHSTMT StatementHandle,
																 _In_reads_opt_(NameLength1) SQLCHAR *CatalogName, SQLSMALLINT NameLength1,
																 _In_reads_opt_(NameLength2) SQLCHAR *SchemaName, SQLSMALLINT NameLength2,
																 _In_reads_opt_(NameLength3) SQLCHAR *TableName, SQLSMALLINT NameLength3,
																 SQLUSMALLINT Unique, SQLUSMALLINT Reserved)
{
	AQ_ODBC_LOG("%s called\n", __FUNCTION__);
	return SQL_SUCCESS;
}

SQLRETURN  SQL_API SQLTransact(SQLHENV EnvironmentHandle,
															 SQLHDBC ConnectionHandle, SQLUSMALLINT CompletionType)
{
	AQ_ODBC_LOG("%s called\n", __FUNCTION__);
	return SQL_SUCCESS;
}

#ifdef __cplusplus
}
#endif