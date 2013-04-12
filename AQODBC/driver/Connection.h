#ifndef __CONNECTION_H__
#define __CONNECTION_H__

#include <string>
#include <vector>
#include <list>
#include <boost/asio.hpp>
#include <boost/array.hpp>
#include <boost/tuple/tuple.hpp>

namespace aq
{

class Connection
{
public:
  
	// type definitions
	typedef boost::array<char, 2048> buffer_t;
public:
	Connection(boost::asio::io_service& io_service);
	~Connection();
	
	void connect(const char * host, boost::uint16_t port);
	void disconnect();

	void write(const char * stmt);
	size_t read(buffer_t& buf);
  
  std::string query_resolver;
  std::string schema;
  std::string db_path;
  std::string cfg_path;
  std::string server;
  std::string port;
  std::string uid;
  std::string pwd;

private:
	boost::asio::io_service& io_service;
	boost::asio::ip::tcp::socket socket;
	
	// 
	std::string _host;
	boost::uint16_t _port;

};

}

#endif