#include "stdafx.h"
#include <boost/algorithm/string.hpp>
#include <boost/property_tree/ptree.hpp>
#include <boost/property_tree/xml_parser.hpp>

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

  if (catalog_name == "%")
  {
    if (conn->server == "")
    {
      result->fillBases(db_path.c_str());
    }
    else
    {
      //
      // Send Request
      std::string stmt_str = "show\n";
      conn->write(stmt_str.c_str());

      //
      // Read Show
      size_t len = 0;
      std::stringstream showData;
      aq::Connection::buffer_t buf;
      while (true)
      {
        memset(buf.data(), 0, buf.size());
        len = conn->read(buf);
        std::string s(buf.data(), len);
        showData << s;
        if (showData.str().find("<Databases>") != std::string::npos)
        {
          break;
        }
      }
      
      // Parse Show
      std::string s = showData.str();
      std::string::size_type pos = std::string::npos;
      if ((pos = s.find("<Databases")) != std::string::npos)
      {
        s = s.substr(pos);
        showData.str("");
        showData << s;
        s = showData.str();
        result->clearBases();

        try
        {
          boost::property_tree::ptree parser;
          boost::property_tree::xml_parser::read_xml(showData, parser);
          boost::property_tree::ptree dbs = parser.get_child("Databases");
          std::for_each(dbs.begin(), dbs.end(), [&] (const boost::property_tree::ptree::value_type& db) {
            std::string dbName = db.second.get<std::string>("<xmlattr>.Name");
            result->addBases(dbName.c_str());
          });
        }
        catch (const std::exception& ex)
        {
        }
      }


    }
  }
  else
  { 
    if ((db_name == "") || (db_name == "%"))
        db_name = ((AqHandleStmt*)StatementHandle)->conn->connection->schema;

    if (conn->server == "")
    {
      result->loadCatalg((db_path + db_name).c_str());
    }
    else
    {
      //
      // Send Request
      std::string conn_str = "connect " + db_name + "\n";
      std::string stmt_str = "desc\n";
      conn->write(conn_str.c_str());
      conn->write(stmt_str.c_str());

      //
      // Read Desc
      size_t len = 0;
      size_t size = 0;
      std::string descData;
      aq::Connection::buffer_t buf;
      std::string::size_type pos = std::string::npos;
      while (true)
      {
        memset(buf.data(), 0, buf.size());
        len = conn->read(buf);
        descData += std::string(buf.data(), len);
        if ((pos = descData.find("</Database>")) != std::string::npos)
        {
          break;
        }
      }
      if (((pos = descData.find("<Database")) != std::string::npos) && (descData.find("</Database>") != std::string::npos))
      {
        std::stringstream ssData;
        descData = descData.substr(pos);
        ssData << descData;
        result->loadCatalg(ssData);
      }
    }
    result->fillTables();
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
  std::string tn = "%";
  if (TableName != NULL)
    tn = (const char *)TableName;
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

