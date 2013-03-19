#include "SyncServer.h"
#include "SyncSession.h"

using namespace aq;
using namespace aq::sync;

Server::Server(boost::asio::io_service& io_service, boost::uint16_t port, Configuration::Ptr cfg)
	: 
	m_acceptor(io_service, boost::asio::ip::tcp::endpoint(boost::asio::ip::tcp::v4(), port)),
	m_cfg(cfg)
{
	accept();
}

void Server::accept()
{
	for (;;)
	{
		Session::ptr newSession = Session::create(m_acceptor.get_io_service(), this->m_cfg);
		boost::shared_ptr<protocol::Demuxer> demuxer(new protocol::Demuxer(newSession));
		newSession->setDemuxer(demuxer);

    m_acceptor.accept(newSession->socket());
		boost::thread t(boost::bind(&Session::start, newSession));
	}
}
