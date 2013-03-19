#ifndef __SYNC_SERVER_H__
#define __SYNC_SERVER_H__

#include "SyncSession.h"
#include "../Configuration.h"
#include <boost/asio.hpp>

namespace aq {
namespace sync {

	class Server
	{
	public:
		Server(boost::asio::io_service& io_service, boost::uint16_t port, Configuration::Ptr);

	private:
		void accept();
		boost::asio::ip::tcp::acceptor m_acceptor;
		Configuration::Ptr m_cfg;
	};

}
}

#endif