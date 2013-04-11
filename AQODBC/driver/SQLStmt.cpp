#include "stdafx.h"
#include <aq/Logger.h>
#include <aq/Utilities.h>
#include <boost/array.hpp>
#include <string>
#include <boost/algorithm/string.hpp>

#ifdef __cplusplus
extern "C" {
#endif

SQLRETURN SQL_API SQLExecDirect(SQLHSTMT StatementHandle,
																 _In_reads_opt_(TextLength) SQLCHAR* StatementText,
																 SQLINTEGER TextLength)
{
	aq::Logger::getInstance().log(AQ_DEBUG, "%s called: [%s]\n", __FUNCTION__, StatementText);
  SQLPrepare(StatementHandle, StatementText, TextLength);
  return SQLExecute(StatementHandle);
}

SQLRETURN  SQL_API SQLExecute(SQLHSTMT StatementHandle)
{
	AQ_ODBC_LOG("%s called\n", __FUNCTION__);
	AqHandleStmt * stmt = (AqHandleStmt*)StatementHandle;
	aq::Connection * conn = stmt->conn->connection;
  
  if (conn->server != "")
  {
    //
    // Send Request
    std::string conn_str = "connect " + conn->schema + "\n";
    std::string stmt_str = "execute \"" + stmt->result->getQuery() + "\"\n";
    conn->write(conn_str.c_str());
    conn->write(stmt_str.c_str());

    //
    // Read Answer
    aq::Connection::buffer_t buf;
    while (!stmt->result->eos())
    {
      memset(buf.data(), 0, buf.size());
      conn->read(buf);
      stmt->result->pushResult(buf.data(), buf.size());
    }

    aq::Logger::getInstance().log(AQ_DEBUG, "Read answer successfully\n");
    return SQL_SUCCESS;
  }
  else
  { 
    std::string prg = conn->query_resolver;
    std::string arg;
    arg += " --aq-ini=" + conn->cfg_path + conn->schema + ".ini";
    arg += " --sql-query=\"" + stmt->result->getQuery() + "\" ";
    arg += " --query-ident=odbc --force --log-level 2 ";

    aq::Logger::getInstance().log(AQ_NOTICE, "running: '%s %s'\n", conn->query_resolver.c_str(), arg.c_str());

    DWORD rc = 1;
    STARTUPINFOW si;
    PROCESS_INFORMATION pi;
    ZeroMemory(&si, sizeof(si));
    si.cb = sizeof(si);
    ZeroMemory(&pi, sizeof(pi));

    std::wstring wprg = aq::string2Wstring(prg);
    std::wstring warg = aq::string2Wstring(arg);
    LPCWSTR prg_wstr = wprg.c_str();
    LPCWSTR arg_wstr = warg.c_str();
    if (CreateProcessW(prg_wstr, (LPWSTR)arg_wstr, NULL, NULL, FALSE, CREATE_NO_WINDOW, NULL, NULL, &si, &pi))
    {
      rc = WaitForSingleObject(pi.hProcess, INFINITE);
      CloseHandle(pi.hProcess);
      CloseHandle(pi.hThread);
    }

    if (rc != 0)
    {
      aq::Logger::getInstance().log(AQ_NOTICE, "call to AQTools failed [error:%d]\n", rc);
      return SQL_ERROR;
    }

    std::string resultFile = conn->db_path;
    resultFile += conn->schema;
    resultFile += "/calculus/odbc/Answer.txt";
    stmt->result->openResultCursor(resultFile.c_str());

    return SQL_SUCCESS;
  }
}

SQLRETURN  SQL_API SQLPrepare(SQLHSTMT StatementHandle,
															_In_reads_(TextLength) SQLCHAR* StatementText,
															SQLINTEGER TextLength)
{
	AQ_ODBC_LOG("%s called\n", __FUNCTION__);
	AqHandleStmt * stmt = (AqHandleStmt*)StatementHandle;
  aq::Connection * connection = stmt->conn->connection;
  std::string query(reinterpret_cast<const char *>(StatementText));
  if (connection->schema != "")
    boost::algorithm::replace_all(query, connection->schema + ".", "");
  boost::algorithm::replace_all(query, "_0", ""); // FIXME
  boost::algorithm::replace_all(query, "\n", " "); // FIXME
  boost::algorithm::trim(query);
  if (*query.rbegin() != ';')
    query += ";";
  stmt->result->setQuery(query.c_str());
	return SQL_SUCCESS;
}

#ifdef __cplusplus
}
#endif