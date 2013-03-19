#include "stdafx.h"
#include "Connection.h"
#include <aq/Logger.h>

using namespace aq;

Connection::Connection(boost::asio::io_service& _io_service)
	: io_service(_io_service), 
	socket(_io_service),
	_host("localhost"),
	_port(9999)
{
	aq::Logger::getInstance().log(AQ_DEBUG, "");
}

Connection::~Connection()
{
	aq::Logger::getInstance().log(AQ_NOTICE, "deallocate Connection\n");
}

void Connection::connect(const char * host, boost::uint16_t port)
{
	aq::Logger::getInstance().log(AQ_INFO, "connect to %s:%u\n", host, port);
	_host = host;
	_port = port;
	std::ostringstream os;
	os << port;
	boost::system::error_code ec;
	boost::asio::ip::tcp::resolver resolver(io_service);
	boost::asio::ip::tcp::resolver::query query(boost::asio::ip::tcp::v4(), host, "9999"/*os.str()*/);
	boost::asio::ip::tcp::resolver::iterator iterator = resolver.resolve(query, ec);
	if (ec)
		aq::Logger::getInstance().log(AQ_ERROR, "cannot resolver %s:%u\n", host, port);
	boost::asio::connect(socket, iterator, ec);
	if (ec)
		aq::Logger::getInstance().log(AQ_ERROR, "cannot connect to %s:%u\n", host, port);
}

void Connection::disconnect()
{
	aq::Logger::getInstance().log(AQ_INFO, "disconnect from %s:%u\n", _host.c_str(), _port);
	socket.shutdown(boost::asio::ip::tcp::socket::shutdown_both);
}

void Connection::write(const char * stmt)
{	
	aq::Logger::getInstance().log(AQ_INFO, "send %s to %s:%u\n", stmt, _host.c_str(), _port);
	boost::asio::write(socket, boost::asio::buffer(stmt, strlen(stmt)));
}

void Connection::read(buffer_t& buf)
{
	aq::Logger::getInstance().log(AQ_INFO, "read from %s:%u\n", _host.c_str(), _port);

	boost::system::error_code error;
	size_t len = socket.read_some(boost::asio::buffer(buf), error);

	//_eos = error == boost::asio::error::eof;
	//_eos = error != 0;
}
