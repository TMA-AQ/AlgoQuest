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
	aq::Connection * conn = ((AqHandleStmt*)StatementHandle)->conn->connection;

  std::string catalog_name;
  if (CatalogName != NULL)
    catalog_name = reinterpret_cast<const char *>(CatalogName);

  std::string db_name;
  if (SchemaName != NULL) 
    db_name = reinterpret_cast<const char *>(SchemaName);

  std::string db_path = ((AqHandleConn*)((AqHandleStmt*)StatementHandle)->conn)->connection->db_path;
  std::string cfg_path = ((AqHandleConn*)((AqHandleStmt*)StatementHandle)->conn)->connection->cfg_path;
  
  if (conn->server != "")
  {
    //
    // Send Request
    std::string conn_str = "connect " + conn->schema + "\n";
    std::string stmt_str = "desc\n";
    conn->write(conn_str.c_str());
    conn->write(stmt_str.c_str());

    //
    // Read Answer
    aq::Connection::buffer_t buf;
    while (!result->eos())
    {
      memset(buf.data(), 0, buf.size());
      conn->read(buf);
      result->pushResult(buf.data(), buf.size());
    }

  }
  else
  {
    if (catalog_name == "%")
    {
      result->fillBases(db_path.c_str());
    }
    else
    {
      if (db_name == "")
        db_name = ((AqHandleStmt*)StatementHandle)->conn->connection->schema;
      result->loadCatalg((db_path + db_name).c_str());
      result->fillTables();
    }
  }

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
  std::string tn((const char *)TableName);
	result->fillColumns(tn.c_str());
	
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
	aq::ResultSet * result = ((AqHandleStmt*)StatementHandle)->result;
  std::string tn((const char *)TableName);
	result->fillColumns(tn.c_str());
	return SQL_SUCCESS;
}

SQLRETURN  SQL_API SQLGetTypeInfo(SQLHSTMT StatementHandle,
																	SQLSMALLINT DataType)
{
	AQ_ODBC_LOG("%s called\n", __FUNCTION__);
	aq::ResultSet * result = ((AqHandleStmt*)StatementHandle)->result;
	result->fillTypeInfos();
	return SQL_SUCCESS;
}

#ifdef __cplusplus
}
#endif

