#ifndef __ASYNC_SERVER_H__
#define __ASYNC_SERVER_H__

#include "AsyncSession.h"
#include "../Configuration.h"
#include <boost/asio.hpp>

namespace aq {
namespace async {

	class Server
	{
	public:
		Server(boost::asio::io_service& io_service, boost::uint16_t port, Configuration::Ptr);

	private:
		void accept();
		void acceptHandler(Session::ptr new_connection, const boost::system::error_code& error);
		boost::asio::ip::tcp::acceptor m_acceptor;
		Configuration::Ptr m_cfg;
	};

}
}

#endif