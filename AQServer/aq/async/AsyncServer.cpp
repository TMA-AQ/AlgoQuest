#include "AsyncServer.h"
#include "AsyncSession.h"

using namespace aq;
using namespace aq::async;

Server::Server(boost::asio::io_service& io_service, boost::uint16_t port, Configuration::Ptr cfg)
	: 
	m_acceptor(io_service, boost::asio::ip::tcp::endpoint(boost::asio::ip::tcp::v4(), port)),
	m_cfg(cfg)
{
	accept();
}

void Server::accept()
{
	Session::ptr newSession = Session::create(m_acceptor.get_io_service(), this->m_cfg);
	boost::shared_ptr<protocol::Demuxer> demuxer(new protocol::Demuxer(newSession));
	newSession->setDemuxer(demuxer);

	m_acceptor.async_accept(newSession->socket(),
		boost::bind(&Server::acceptHandler, this, newSession,
		boost::asio::placeholders::error));
}

void Server::acceptHandler(Session::ptr new_Session, const boost::system::error_code& error)
{
	if (!error)
	{
		new_Session->start();
	}

	accept();
}
