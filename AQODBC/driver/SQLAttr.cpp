#include "stdafx.h"

#ifdef __cplusplus
extern "C" {
#endif

	SQLRETURN  SQL_API SQLGetEnvAttr(SQLHENV EnvironmentHandle,
		SQLINTEGER Attribute, _Out_writes_(_Inexpressible_(BufferLength)) SQLPOINTER Value,
		SQLINTEGER BufferLength, _Out_opt_ SQLINTEGER *StringLength)
	{
		AQ_ODBC_LOG("%s called\n", __FUNCTION__);
		return SQL_SUCCESS;
	}

	SQLRETURN  SQL_API SQLGetStmtAttr(SQLHSTMT StatementHandle,
		SQLINTEGER Attribute, _Out_writes_opt_(_Inexpressible_(BufferLength)) SQLPOINTER Value,
		SQLINTEGER BufferLength, _Out_opt_ SQLINTEGER *StringLength)
	{
		AQ_ODBC_LOG("%s called\n", __FUNCTION__);

		AqHandleStmt * hstmt = (AqHandleStmt *)StatementHandle;
		AQ_ODBC_LOG("hstmt id: [%d]\n", hstmt->Stmt);


		switch ( Attribute ) {

		case SQL_ATTR_APP_PARAM_DESC: 
			AQ_ODBC_LOG("SQL_ATTR_APP_PARAM_DESC\n");
			*((SQLINTEGER*)Value) = (SQLINTEGER)&(((AqHandleStmt *)StatementHandle)->apd);
			break;

		case SQL_ATTR_APP_ROW_DESC:
			AQ_ODBC_LOG("SQL_ATTR_APP_ROW_DESC\n");
			*((SQLINTEGER*)Value) = (SQLINTEGER)&(((AqHandleStmt *)StatementHandle)->ard);
			break;

		case SQL_ATTR_IMP_PARAM_DESC: 
			AQ_ODBC_LOG("SQL_ATTR_IMP_PARAM_DESC\n"); 
			*((SQLINTEGER*)Value) = (SQLINTEGER)&(((AqHandleStmt *)StatementHandle)->ipd);
			break;

		case SQL_ATTR_IMP_ROW_DESC:
			AQ_ODBC_LOG("SQL_ATTR_IMP_ROW_DESC\n");
			*((SQLINTEGER*)Value) = (SQLINTEGER)&(((AqHandleStmt *)StatementHandle)->ird);
			break;

		case SQL_ATTR_ASYNC_ENABLE: AQ_ODBC_LOG("SQL_ATTR_ASYNC_ENABLE\n"); break;

		case SQL_ATTR_CONCURRENCY: AQ_ODBC_LOG("SQL_ATTR_CONCURRENCY\n"); break;

		case SQL_ATTR_CURSOR_SCROLLABLE: AQ_ODBC_LOG("SQL_ATTR_CURSOR_SCROLLABLE\n"); break;

		case SQL_ATTR_CURSOR_SENSITIVITY: AQ_ODBC_LOG("SQL_ATTR_CURSOR_SENSITIVITY\n"); break;

		case SQL_ATTR_CURSOR_TYPE: AQ_ODBC_LOG("SQL_ATTR_CURSOR_TYPE\n"); break;

		case SQL_ATTR_ENABLE_AUTO_IPD: AQ_ODBC_LOG("SQL_ATTR_ENABLE_AUTO_IPD\n"); break;

		case SQL_ATTR_FETCH_BOOKMARK_PTR: AQ_ODBC_LOG("SQL_ATTR_FETCH_BOOKMARK_PTR\n"); break;

		case SQL_ATTR_KEYSET_SIZE: AQ_ODBC_LOG("SQL_ATTR_KEYSET_SIZE\n"); break;

		case SQL_ATTR_MAX_LENGTH: AQ_ODBC_LOG("SQL_ATTR_MAX_LENGTH\n"); break;

		case SQL_ATTR_MAX_ROWS: AQ_ODBC_LOG("SQL_ATTR_MAX_ROWS\n"); break;

		case SQL_ATTR_METADATA_ID: AQ_ODBC_LOG("SQL_ATTR_METADATA_ID\n"); break;

		case SQL_ATTR_NOSCAN: AQ_ODBC_LOG("SQL_ATTR_NOSCAN\n"); break;

		case SQL_ATTR_PARAM_BIND_TYPE: AQ_ODBC_LOG("SQL_ATTR_PARAM_BIND_TYPE\n"); break;

		case SQL_ATTR_PARAM_BIND_OFFSET_PTR: AQ_ODBC_LOG("SQL_ATTR_PARAM_BIND_OFFSET_PTR\n"); break;

		case SQL_ATTR_PARAM_OPERATION_PTR: AQ_ODBC_LOG("SQL_ATTR_PARAM_OPERATION_PTR\n"); break;

		case SQL_ATTR_PARAM_STATUS_PTR: AQ_ODBC_LOG("SQL_ATTR_PARAM_STATUS_PTR\n"); break;

		case SQL_ATTR_PARAMS_PROCESSED_PTR: AQ_ODBC_LOG("SQL_ATTR_PARAMS_PROCESSED_PTR\n"); break;

		case SQL_ATTR_PARAMSET_SIZE: AQ_ODBC_LOG("SQL_ATTR_PARAMSET_SIZE\n"); break;

		case SQL_ATTR_QUERY_TIMEOUT: AQ_ODBC_LOG("SQL_ATTR_QUERY_TIMEOUT\n"); break;

		case SQL_ATTR_RETRIEVE_DATA: AQ_ODBC_LOG("SQL_ATTR_RETRIEVE_DATA\n"); break;

		case SQL_ROWSET_SIZE: AQ_ODBC_LOG("SQL_ROWSET_SIZE\n"); break;

		case SQL_ATTR_ROW_BIND_OFFSET_PTR: AQ_ODBC_LOG("SQL_ATTR_ROW_BIND_OFFSET_PTR\n"); break; 

		case SQL_ATTR_ROW_BIND_TYPE: AQ_ODBC_LOG("SQL_ATTR_ROW_BIND_TYPE\n"); break;

		case SQL_ATTR_ROW_NUMBER: AQ_ODBC_LOG("SQL_ATTR_ROW_NUMBER\n"); break;

		case SQL_ATTR_ROW_OPERATION_PTR: AQ_ODBC_LOG("SQL_ATTR_ROW_OPERATION_PTR\n"); break; 

		case SQL_ATTR_ROW_STATUS_PTR: AQ_ODBC_LOG("SQL_ATTR_ROW_STATUS_PTR\n"); break;

		case SQL_ATTR_ROWS_FETCHED_PTR: AQ_ODBC_LOG("SQL_ATTR_ROWS_FETCHED_PTR\n"); break;

		case SQL_ATTR_SIMULATE_CURSOR: AQ_ODBC_LOG("SQL_ATTR_SIMULATE_CURSOR\n"); break; 

		default:

			AQ_ODBC_LOG("Stmt attr - unknown %d", Attribute);
			break; 
		}

		return SQL_SUCCESS;
	}

#ifdef __cplusplus
}
#endif
