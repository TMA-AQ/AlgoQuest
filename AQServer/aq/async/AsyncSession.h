#ifndef __ASYNC_SESSION_H__
#define __ASYNC_SESSION_H__

#define WIN32_LEAN_AND_MEAN

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
namespace async {

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
		void release();

	private:
		Session(boost::asio::io_service& io_service, Configuration::Ptr cfg, unsigned int sessionId);
		void read();
		void readHandler(const boost::system::error_code& error, size_t bytes_transferred);
		void writeHandler(const boost::system::error_code& error, size_t bytes_transferred);
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
		boost::asio::strand m_strand;
		std::list<std::string> m_lMsgBuff; // todo: use buffer and not string
		boost::array<char, 4096> m_recvBuf;
		Configuration::Ptr m_cfg;
		Configuration::base_cfg::Ptr m_current_db_cfg;
		boost::shared_ptr<protocol::Demuxer> m_demuxer;
		boost::thread m_cmdThread;
		boost::mutex m_writeMutex;
		boost::mutex m_queryMutex;
		boost::thread::id m_initialThreadId;
		unsigned int m_sessionId;
		bool m_stop;
		bool m_sqlProcessing;
	};
}
}

#endif
