#ifndef __AQ_SQL_TYPES_H__
#define __AQ_SQL_TYPES_H__

#include "ResultSet.h"
#include "Connection.h"

#include <windows.h>
#include <sqlext.h>
#include <odbcinst.h>

#include <string>
#include <list>
#include <cstdint>

#include <boost/shared_ptr.hpp>
#include <boost/thread.hpp>
#include <boost/asio.hpp>

#define AQ_ODBC_LOG(args, ...) \
	do \
	{ \
		/* printf("[%s:%s:%u]: ", __FILE__, __FUNCTION__, __LINE__); */ \
		/* printf(args, ## __VA_ARGS__); */ \
	} \
	while (0);

struct AqHandleEnv;
struct AqHandleConn;
struct AqHandleStmt;

//#ifdef __cplusplus
//extern "C" {
//#endif

struct app_row_desc_t
{
	SQLSMALLINT id;
};

struct imp_row_desc_t
{
	SQLSMALLINT id;
};

struct app_parm_desc_t
{
	SQLSMALLINT id;
};

struct imp_parm_desc_t
{
	SQLSMALLINT id;
};

struct AqHandleStmt
{
	SQLSMALLINT Stmt;

	app_row_desc_t       ard; ///< application row descriptor
	imp_row_desc_t       ird; ///< implementation row descriptor
	app_parm_desc_t      apd; ///< application parm descriptor
	imp_parm_desc_t      ipd; ///< implementation parm descriptor

	aq::ResultSet * result;

	AqHandleConn * conn;
};

struct AqHandleConn
{
	SQLSMALLINT Conn;

	aq::Connection * connection;
	boost::asio::io_service * ioService;
	boost::thread * ioThread;

	//boost::shared_ptr<aq::Connection> connection;
	//boost::shared_ptr<boost::asio::io_service> ioService;
	//boost::shared_ptr<boost::thread> ioThread;

	AqHandleEnv * env;
};

struct AqHandleEnv
{
	SQLSMALLINT Env;
};

//#ifdef __cplusplus
//}
//#endif

#endif