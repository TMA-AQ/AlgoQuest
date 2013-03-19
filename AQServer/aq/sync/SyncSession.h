#ifndef __SYNC_SESSION_H__
#define __SYNC_SESSION_H__

#include "../Configuration.h"
#include "../protocol/Demuxer.h"
#include "../protocol/CommandHandler.h"
#include <boost/bind.hpp>
#include <boost/array.hpp>
#include <boost/asio.hpp>
#include <boost/thread.hpp>
#include <boost/thread/mutex.hpp>
#include <boost/enable_shared_from_this.hpp>

namespace aq {
namespace sync {

	class Session : public boost::enable_shared_from_this<Session>,
		public protocol::CommandHandler
	{
	public:
		typedef boost::shared_ptr<Session> ptr;

		~Session();

		static ptr create(boost::asio::io_service& io_service, Configuration::Ptr cfg)
		{
			static unsigned int id = 1;
			return ptr(new Session(io_service, cfg, id++));
		}
		
		boost::asio::ip::tcp::socket& socket()
		{
			return m_socket;
		}

		void setDemuxer(boost::shared_ptr<protocol::Demuxer> demuxer) { this->m_demuxer = demuxer; }

		void start();

	private:
		Session(boost::asio::io_service& io_service, Configuration::Ptr cfg, unsigned int sessionId);
		void read();
		bool check(const boost::system::error_code& error);
		void processSQL(std::string& sqlQuery);

		//
		// AQ Protocol handler implementation
		void deliver(const std::string& buffer);
		void onAuth(std::string& args);
		void onConnect(std::string& dbName);
		void onDesc(std::string& args);
		void onExecuteSQL(std::string& sqlQuery);
		void onQuit(std::string& args);
		void onShow(std::string& args);
		void onVersion(std::string& args);

		//
		//

		boost::asio::ip::tcp::socket m_socket;
		std::list<std::string> m_lMsgBuff; // todo: use buffer and not string
		boost::array<char, 4096> m_recvBuf;
		Configuration::Ptr m_cfg;
		Configuration::base_cfg::Ptr m_current_db_cfg;
		boost::shared_ptr<protocol::Demuxer> m_demuxer;
		unsigned int m_sessionId;
		bool m_stop;
	};

}
}

#endif
