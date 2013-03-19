#include "stdafx.h"
#include <aq/Logger.h>
#include <boost/array.hpp>
#include <string>

#ifdef __cplusplus
extern "C" {
#endif

SQLRETURN SQL_API SQLExecDirect(SQLHSTMT StatementHandle,
																 _In_reads_opt_(TextLength) SQLCHAR* StatementText,
																 SQLINTEGER TextLength)
{
	aq::Logger::getInstance().log(AQ_DEBUG, "%s called: [%s]\n", __FUNCTION__, StatementText);
	
	//
	// Get pointer on connection
	AqHandleStmt * stmt = (AqHandleStmt*)StatementHandle;
	aq::Connection * conn = stmt->conn->connection;

	//
	// Send Request
	const std::string stmt_str = "execute " + std::string((const char *)StatementText) + "\n";
	conn->write("connect test\n");
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

	//
	//
	aq::Logger::getInstance().log(AQ_NOTICE, "Read answer successfully\n");

	//
	// FIXME: simulate result, fill it
	// stmt.conn->connection->fillSimulateResult();

	return SQL_SUCCESS;
}

#ifdef __cplusplus
}
#endif