#include "stdafx.h"
#include <cstdio>

#ifdef __cplusplus
extern "C" {
#endif

SQLRETURN  SQL_API SQLGetDiagField(SQLSMALLINT HandleType, 
																	 SQLHANDLE Handle,
																	 SQLSMALLINT RecNumber, 
																	 SQLSMALLINT DiagIdentifier,
																	 _Out_writes_opt_(_Inexpressible_(BufferLength)) SQLPOINTER DiagInfo, 
																	 SQLSMALLINT BufferLength,
																	 _Out_opt_ SQLSMALLINT *StringLength)
{
	AQ_ODBC_LOG("%s called\n", __FUNCTION__);
	switch ( DiagIdentifier ) {

	case SQL_DIAG_CLASS_ORIGIN:
		AQ_ODBC_LOG("SQL_DIAG_CLASS_ORIGIN\n");
		break;

	case SQL_DIAG_COLUMN_NUMBER:
		AQ_ODBC_LOG("SQL_DIAG_COLUMN_NUMBER\n");
		break;

	case SQL_DIAG_CONNECTION_NAME:
		AQ_ODBC_LOG("SQL_DIAG_CONNECTION_NAME\n");
		break;

	case SQL_DIAG_MESSAGE_TEXT:
		AQ_ODBC_LOG("SQL_DIAG_MESSAGE_TEXT\n");
		break;

	case SQL_DIAG_NATIVE:
		AQ_ODBC_LOG("SQL_DIAG_NATIVE\n");
		break;

	case SQL_DIAG_ROW_NUMBER:
		AQ_ODBC_LOG("SQL_DIAG_ROW_NUMBER\n");
		break;

	case SQL_DIAG_SERVER_NAME:
		{
			AQ_ODBC_LOG("SQL_DIAG_SERVER_NAME\n");	
			if ( HandleType  == SQL_HANDLE_DBC ) {
				AQ_ODBC_LOG("SQL_HANDLE_DBC\n");
			}
			else if ( HandleType == SQL_HANDLE_STMT) {
				AQ_ODBC_LOG("SQL_HANDLE_STMT\n");
			}
			else {
				AQ_ODBC_LOG("SQL_HANDLE_UNKNOWN\n");
			}
		}

	case SQL_DIAG_SQLSTATE:
		AQ_ODBC_LOG("SQL_DIAG_SQLSTATE\n");
		break;

	case SQL_DIAG_SUBCLASS_ORIGIN:
		AQ_ODBC_LOG("SQL_DIAG_SUBCLASS_ORIGIN\n");
		break;

	default:
		AQ_ODBC_LOG("SQLGetDiagField called, HandleType: %d, RecNum: %d, InfoType: %d, BufLen: %d\n", HandleType, RecNumber, DiagIdentifier, BufferLength);
		return SQL_ERROR;
	}

	return SQL_SUCCESS;
}

SQLRETURN  SQL_API SQLGetDiagRec(SQLSMALLINT HandleType,
																 SQLHANDLE Handle,
																 SQLSMALLINT RecNumber,
																 _Out_writes_opt_(6) SQLCHAR *Sqlstate,
																 SQLINTEGER *NativeError,
																 _Out_writes_opt_(BufferLength) SQLCHAR* MessageText,
																 SQLSMALLINT BufferLength,
																 _Out_opt_
																 SQLSMALLINT *TextLength)
{
	AQ_ODBC_LOG("%s called\n", __FUNCTION__);
	return SQL_SUCCESS;
}

SQLRETURN  SQL_API SQLGetFunctions(SQLHDBC ConnectionHandle,
																	 SQLUSMALLINT FunctionId, 
																	 _Out_writes_opt_(_Inexpressible_("Buffer length pfExists points to depends on fFunction value.")) 
																	 SQLUSMALLINT *Supported)
{
	AQ_ODBC_LOG("%s called [functionId: %d]\n", __FUNCTION__, FunctionId);
	*Supported = SQL_TRUE;
	switch(FunctionId)
	{

	case SQL_API_SQLALLOCHANDLE:       AQ_ODBC_LOG("SQL_API_SQLALLOCHANDLE\n");       break;
	case SQL_API_SQLGETDESCFIELD:      AQ_ODBC_LOG("SQL_API_SQLGETDESCFIELD\n");      break;
	case SQL_API_SQLBINDCOL:           AQ_ODBC_LOG("SQL_API_SQLBINDCOL\n");           break;
	case SQL_API_SQLGETDESCREC:        AQ_ODBC_LOG("SQL_API_SQLGETDESCREC\n");        break;
	case SQL_API_SQLCANCEL:            AQ_ODBC_LOG("SQL_API_SQLCANCEL\n");            break;
	case SQL_API_SQLGETDIAGFIELD:      AQ_ODBC_LOG("SQL_API_SQLGETDIAGFIELD\n");      break;
	case SQL_API_SQLCLOSECURSOR:       AQ_ODBC_LOG("SQL_API_SQLCLOSECURSOR\n");       break;
	case SQL_API_SQLGETDIAGREC:        AQ_ODBC_LOG("SQL_API_SQLGETDIAGREC\n");        break;
	case SQL_API_SQLCOLATTRIBUTE:      AQ_ODBC_LOG("SQL_API_SQLCOLATTRIBUTE\n");      break;
	case SQL_API_SQLGETENVATTR:        AQ_ODBC_LOG("SQL_API_SQLGETENVATTR\n");        break;
	case SQL_API_SQLCONNECT:           AQ_ODBC_LOG("SQL_API_SQLCONNECT\n");           break;
	case SQL_API_SQLGETFUNCTIONS:      AQ_ODBC_LOG("SQL_API_SQLGETFUNCTIONS\n");      break;
	case SQL_API_SQLCOPYDESC:          AQ_ODBC_LOG("SQL_API_SQLCOPYDESC\n");          break;
	case SQL_API_SQLGETINFO:           AQ_ODBC_LOG("SQL_API_SQLGETINFO\n");           break;
	case SQL_API_SQLDATASOURCES:       AQ_ODBC_LOG("SQL_API_SQLDATASOURCES\n");       break;
	case SQL_API_SQLGETSTMTATTR:       AQ_ODBC_LOG("SQL_API_SQLGETSTMTATTR\n");       break;
	case SQL_API_SQLDESCRIBECOL:       AQ_ODBC_LOG("SQL_API_SQLDESCRIBECOL\n");       break;
	case SQL_API_SQLGETTYPEINFO:       AQ_ODBC_LOG("SQL_API_SQLGETTYPEINFO\n");       break;
	case SQL_API_SQLDISCONNECT:        AQ_ODBC_LOG("SQL_API_SQLDISCONNECT\n");        break;
	case SQL_API_SQLNUMRESULTCOLS:     AQ_ODBC_LOG("SQL_API_SQLNUMRESULTCOLS\n");     break;
	case SQL_API_SQLDRIVERS:           AQ_ODBC_LOG("SQL_API_SQLDRIVERS\n");           break;
	case SQL_API_SQLPARAMDATA:         AQ_ODBC_LOG("SQL_API_SQLPARAMDATA\n");         break;
	case SQL_API_SQLENDTRAN:           AQ_ODBC_LOG("SQL_API_SQLENDTRAN\n");           break;
	case SQL_API_SQLPREPARE:           AQ_ODBC_LOG("SQL_API_SQLPREPARE\n");           break;
	case SQL_API_SQLEXECDIRECT:        AQ_ODBC_LOG("SQL_API_SQLEXECDIRECT\n");        break;
	case SQL_API_SQLPUTDATA:           AQ_ODBC_LOG("SQL_API_SQLPUTDATA\n");           break;
	case SQL_API_SQLEXECUTE:           AQ_ODBC_LOG("SQL_API_SQLEXECUTE\n");           break;
	case SQL_API_SQLROWCOUNT:          AQ_ODBC_LOG("SQL_API_SQLROWCOUNT\n");          break;
	case SQL_API_SQLFETCH:             AQ_ODBC_LOG("SQL_API_SQLFETCH\n");             break;
	case SQL_API_SQLSETCONNECTATTR:    AQ_ODBC_LOG("SQL_API_SQLSETCONNECTATTR\n");    break;
	case SQL_API_SQLFETCHSCROLL:       AQ_ODBC_LOG("SQL_API_SQLFETCHSCROLL\n");       break;
	case SQL_API_SQLSETCURSORNAME:     AQ_ODBC_LOG("SQL_API_SQLSETCURSORNAME\n");     break;
	case SQL_API_SQLFREEHANDLE:        AQ_ODBC_LOG("SQL_API_SQLFREEHANDLE\n");        break;
	case SQL_API_SQLSETDESCFIELD:      AQ_ODBC_LOG("SQL_API_SQLSETDESCFIELD\n");      break;
	case SQL_API_SQLFREESTMT:          AQ_ODBC_LOG("SQL_API_SQLFREESTMT\n");          break;
	case SQL_API_SQLSETDESCREC:        AQ_ODBC_LOG("SQL_API_SQLSETDESCREC\n");        break;
	case SQL_API_SQLGETCONNECTATTR:    AQ_ODBC_LOG("SQL_API_SQLGETCONNECTATTR\n");    break;
	case SQL_API_SQLSETENVATTR:        AQ_ODBC_LOG("SQL_API_SQLSETENVATTR\n");        break;
	case SQL_API_SQLGETCURSORNAME:     AQ_ODBC_LOG("SQL_API_SQLGETCURSORNAME\n");     break;
	case SQL_API_SQLSETSTMTATTR:       AQ_ODBC_LOG("SQL_API_SQLSETSTMTATTR\n");       break;
	case SQL_API_SQLGETDATA:           AQ_ODBC_LOG("SQL_API_SQLGETDATA\n");           break;
	case SQL_API_SQLCOLUMNS:           AQ_ODBC_LOG("SQL_API_SQLCOLUMNS\n");           break;
	case SQL_API_SQLSTATISTICS:        AQ_ODBC_LOG("SQL_API_SQLSTATISTICS\n");        break;
	case SQL_API_SQLSPECIALCOLUMNS:    AQ_ODBC_LOG("SQL_API_SQLSPECIALCOLUMNS\n");    break;
	case SQL_API_SQLTABLES:            AQ_ODBC_LOG("SQL_API_SQLTABLES\n");            break;
	case SQL_API_SQLBINDPARAMETER:     AQ_ODBC_LOG("SQL_API_SQLBINDPARAMETER\n");     break;
	case SQL_API_SQLNATIVESQL:         AQ_ODBC_LOG("SQL_API_SQLNATIVESQL\n");         break;
	case SQL_API_SQLBROWSECONNECT:     AQ_ODBC_LOG("SQL_API_SQLBROWSECONNECT\n");     break;
	case SQL_API_SQLNUMPARAMS:         AQ_ODBC_LOG("SQL_API_SQLNUMPARAMS\n");         break;
	case SQL_API_SQLBULKOPERATIONS:    AQ_ODBC_LOG("SQL_API_SQLBULKOPERATIONS\n");    break;
	case SQL_API_SQLPRIMARYKEYS:       AQ_ODBC_LOG("SQL_API_SQLPRIMARYKEYS\n");       break;
	case SQL_API_SQLCOLUMNPRIVILEGES:  AQ_ODBC_LOG("SQL_API_SQLCOLUMNPRIVILEGES\n");  break;
	case SQL_API_SQLPROCEDURECOLUMNS:  AQ_ODBC_LOG("SQL_API_SQLPROCEDURECOLUMNS\n");  break;
	case SQL_API_SQLDESCRIBEPARAM:     AQ_ODBC_LOG("SQL_API_SQLDESCRIBEPARAM\n");     break;
	case SQL_API_SQLPROCEDURES:        AQ_ODBC_LOG("SQL_API_SQLPROCEDURES\n");        break;
	case SQL_API_SQLDRIVERCONNECT:     AQ_ODBC_LOG("SQL_API_SQLDRIVERCONNECT\n");     break;
	case SQL_API_SQLSETPOS:            AQ_ODBC_LOG("SQL_API_SQLSETPOS\n");            break;
	case SQL_API_SQLFOREIGNKEYS:       AQ_ODBC_LOG("SQL_API_SQLFOREIGNKEYS\n");       break;
	case SQL_API_SQLTABLEPRIVILEGES:   AQ_ODBC_LOG("SQL_API_SQLTABLEPRIVILEGES\n");   break;
	case SQL_API_SQLMORERESULTS:       AQ_ODBC_LOG("SQL_API_SQLMORERESULTS\n");       break;
	default:                           AQ_ODBC_LOG("UNKNOWN FUNCTION\n");             break;
	}

	return SQL_SUCCESS;
}

#ifdef __cplusplus
}
#endif