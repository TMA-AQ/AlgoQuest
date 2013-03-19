#include "stdafx.h"

#ifdef __cplusplus
extern "C" {
#endif

SQLRETURN  SQL_API SQLTables(SQLHSTMT StatementHandle,
														 _In_reads_opt_(NameLength1) SQLCHAR *CatalogName, SQLSMALLINT NameLength1,
														 _In_reads_opt_(NameLength2) SQLCHAR *SchemaName, SQLSMALLINT NameLength2,
														 _In_reads_opt_(NameLength3) SQLCHAR *TableName, SQLSMALLINT NameLength3,
														 _In_reads_opt_(NameLength4) SQLCHAR *TableType, SQLSMALLINT NameLength4)
{
	AQ_ODBC_LOG("%s called\n", __FUNCTION__);

	aq::ResultSet * result = ((AqHandleStmt*)StatementHandle)->result;
	// c->write("connect test\ndesc");

	//while (!c->eos())
	//{
	//	TODO
	//}

	if (strncmp((const char *)CatalogName, "test", NameLength1) == 0)
		result->fillSimulateResultTables2();
	else
		result->fillSimulateResultTables1();
	 
	return SQL_SUCCESS;
}

SQLRETURN  SQL_API SQLColumns(SQLHSTMT StatementHandle,
															_In_reads_opt_(NameLength1) SQLCHAR *CatalogName, SQLSMALLINT NameLength1,
															_In_reads_opt_(NameLength2) SQLCHAR *SchemaName, SQLSMALLINT NameLength2,
															_In_reads_opt_(NameLength3) SQLCHAR *TableName, SQLSMALLINT NameLength3,
															_In_reads_opt_(NameLength4) SQLCHAR *ColumnName, SQLSMALLINT NameLength4)
{
	AQ_ODBC_LOG("%s called\n", __FUNCTION__);
	aq::ResultSet * result = ((AqHandleStmt*)StatementHandle)->result;
	// c->write("connect test\ndesc");

	//while (!c->eos())
	//{
	//	TODO
	//}

	result->fillSimulateResultColumns((const char *)TableName, NameLength3);
	return SQL_SUCCESS;
}

SQLRETURN  SQL_API SQLGetTypeInfo(SQLHSTMT StatementHandle,
																	SQLSMALLINT DataType)
{
	AQ_ODBC_LOG("%s called\n", __FUNCTION__);
	aq::ResultSet * result = ((AqHandleStmt*)StatementHandle)->result;
	// c->write("connect test\ndesc");

	//while (!c->eos())
	//{
	//	TODO
	//}

	result->fillSimulateResultTypeInfos();
	return SQL_SUCCESS;
}

#ifdef __cplusplus
}
#endif

