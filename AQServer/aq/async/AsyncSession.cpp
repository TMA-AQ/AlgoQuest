#include <SQLParser/AQEngine.h>
#include <SQLParser/SQLParser.h>
#include <SQLParser/SQLPrefix.h>
#include <SQLParser/Column2Table.h>
#include <SQLParser/NestedQueries.h>
#include <SQLParser/JeqParser.h>
#include <SQLParser/Exceptions.h>
#include <aq/Logger.h>

#include "AsyncSession.h"

#include <fstream>
#include <boost/lexical_cast.hpp>
#include <boost/date_time/posix_time/posix_time.hpp>

#ifndef __AQSERVER_VERSION__ 
# define __AQSERVER_VERSION__ "0.0.1"
#endif

using namespace aq;
using namespace aq::async;

typedef boost::tokenizer<boost::char_separator<char> > tokenizer;

Session::Session(boost::asio::io_service& io_service, Configuration::Ptr cfg, unsigned int sessionId) 
	: 
	m_socket(io_service),
	m_strand(io_service),
	m_cfg(cfg),
	m_initialThreadId(boost::this_thread::get_id()),
	m_sessionId(sessionId),
	m_stop(false),
	m_sqlProcessing(false)
{
}

Session::~Session()
{
	aq::Logger::getInstance().log(AQ_DEBUG, "destroy session %u\n", this->m_sessionId);
}

void Session::start()
{
	try
	{
		std::string args = "Welcome to AQ Server";
		this->onVersion(args);
	}
	catch (const boost::system::system_error &error)
	{
		aq::Logger::getInstance().log(AQ_ERROR, "error %s\n", error.what());
	}
	catch (...)
	{
		aq::Logger::getInstance().log(AQ_ERROR, "unkwnow exception\n");
	}
}

void Session::release()
{
	this->m_stop = true;
}

void Session::read()
{
	aq::Logger::getInstance().log(AQ_DEBUG, "read on socket [sessionId:%u]\n", m_sessionId);
	this->m_socket.async_read_some(boost::asio::buffer(this->m_recvBuf), 
		m_strand.wrap(boost::bind(&Session::readHandler, shared_from_this(), boost::asio::placeholders::error, boost::asio::placeholders::bytes_transferred)));
}

void Session::deliver(const std::string& buffer)
{
	aq::Logger::getInstance().log(AQ_DEBUG, "write on socket [sessionId:%u]\n", m_sessionId);
	boost::mutex::scoped_lock lock(this->m_writeMutex);
	bool write_in_progress = !this->m_lMsgBuff.empty();
	this->m_lMsgBuff.push_back(buffer);

	if (!write_in_progress)
	{
		boost::asio::async_write(this->m_socket,
			boost::asio::buffer(this->m_lMsgBuff.front().c_str(), this->m_lMsgBuff.front().size()),
			m_strand.wrap(boost::bind(&Session::writeHandler, shared_from_this(), boost::asio::placeholders::error, boost::asio::placeholders::bytes_transferred)));
	}
}

void Session::readHandler(const boost::system::error_code& error, size_t bytes_transferred)
{  
	aq::Logger::getInstance().log(AQ_DEBUG, "read completed [sessionId:%u]\n", m_sessionId);
	if (this->check(error)) return;

	char * buf = new char[bytes_transferred + 1]; 
	::memcpy(buf, this->m_recvBuf.data(), bytes_transferred);
	buf[bytes_transferred] = 0;
	std::string cmd(buf);
	aq::Logger::getInstance().log(AQ_DEBUG, "push in aq demuxer a buffer of size %u [sessionId:%u]\n", cmd.size(), m_sessionId);
	m_demuxer->push(cmd);
	if (m_sqlProcessing)
	{
	}
}

void Session::writeHandler(const boost::system::error_code& error,
													 size_t bytes_transferred)
{
	aq::Logger::getInstance().log(AQ_DEBUG, "write completed [sessionId:%u]\n", m_sessionId);
	if (this->check(error)) return;

	this->m_lMsgBuff.pop_front();
	if (!this->m_lMsgBuff.empty())
	{
		boost::asio::async_write(this->m_socket,
			boost::asio::buffer(this->m_lMsgBuff.front().c_str(), this->m_lMsgBuff.front().size()),
			m_strand.wrap(boost::bind(&Session::writeHandler, shared_from_this(), boost::asio::placeholders::error, boost::asio::placeholders::bytes_transferred)));
	}
	else if (m_stop)
	{
		aq::Logger::getInstance().log(AQ_DEBUG, "close socket [sessionId:%u]\n", m_sessionId);
		this->m_socket.close();
	}
	//else if (!m_sqlProcessing)
	//{
	//	this->read();
	//}
}

bool Session::check(const boost::system::error_code& error)
{
	if (error)
	{
		this->release();
		return true;
	}
	return false;
}

// ---------------------------------------------------------------------------
// Commands Handlers

void Session::onAuth(std::string& args)
{
	if (m_stop) return;
	aq::Logger::getInstance().log(AQ_DEBUG, "[session:%u]\n", this->m_sessionId);
	// todo
}

void Session::onConnect(std::string& dbName)
{
	if (m_stop) return;
	aq::Logger::getInstance().log(AQ_DEBUG, "[session:%u]\n", this->m_sessionId);
	std::ostringstream oss;
	try
	{
		this->m_current_db_cfg = this->m_cfg->getCfg(dbName);
		oss << "successfully connected to database " << dbName << std::endl;
	}
	catch (const generic_error& ex)
	{
		aq::Logger::getInstance().log(AQ_ERROR, ex.what());
		oss << ex.what() << std::endl;
	}
	this->deliver(oss.str());
}

void Session::onDesc(std::string& args)
{
	if (m_stop) return;
	aq::Logger::getInstance().log(AQ_DEBUG, "[session:%u]\n", this->m_sessionId);
	std::ostringstream oss;
	if (this->m_current_db_cfg)
	{
		this->m_current_db_cfg->baseDesc->dump(oss);
	}
	else
	{
		oss << "not connected to any database" << std::endl;
		aq::Logger::getInstance().log(AQ_INFO, oss.str().c_str());
	}
	this->deliver(oss.str());
}

void Session::onExecuteSQL(std::string& sqlQuery)
{
	if (m_stop) return;
	aq::Logger::getInstance().log(AQ_DEBUG, "[session:%u]\n", this->m_sessionId);
	boost::mutex::scoped_lock lock(m_queryMutex);
	m_sqlProcessing = true;
	m_demuxer->pause();
	m_cmdThread = boost::thread(boost::bind(&Session::processSQL, shared_from_this(), sqlQuery));
}

void Session::onQuit(std::string& args)
{
	if (m_stop) return;
	aq::Logger::getInstance().log(AQ_DEBUG, "[session:%u]\n", this->m_sessionId);
	this->release();
}

void Session::onShow(std::string& args)
{
	if (m_stop) return;
	aq::Logger::getInstance().log(AQ_DEBUG, "[session:%u]\n", this->m_sessionId);
	std::ostringstream oss;
	for (Configuration::map_cfg_t::const_iterator it = this->m_cfg->begin(); it != this->m_cfg->end(); ++it)
	{
		oss << it->first << std::endl;
	}
	this->deliver(oss.str());
}

void Session::onVersion(std::string& args)
{
	if (m_stop) return;
	aq::Logger::getInstance().log(AQ_DEBUG, "[session:%u]\n", this->m_sessionId);
	std::ostringstream oss;
	oss << "Welcome to AQServer version " << __AQSERVER_VERSION__ << std::endl;
	this->deliver(oss.str());
}

// -----------------------------------------------------------------------------------------------
// SQL Query Processing

void Session::processSQL(std::string& sqlQuery)
{
	std::ostringstream oss;
	if (this->m_current_db_cfg)
	{

		boost::posix_time::ptime begin(boost::posix_time::microsec_clock::local_time());

		try
		{

			tnode	*pNode  = NULL;
			int	nRet;

			//
			// Prepare

			//
			// Parse SQL request
			aq::Logger::getInstance().log(AQ_INFO, "parse sql request %s\n", sqlQuery.c_str());
			if ((nRet = SQLParse(sqlQuery.c_str(), &pNode)) != 0 ) 
			{
				oss << "error parsing sql query '" << sqlQuery << "'" << std::endl;
				aq::Logger::getInstance().log(AQ_ERROR, oss.str().c_str());
				throw generic_error(generic_error::INVALID_QUERY, oss.str());
			}

			//
			// Transform SQL request in prefix form
			QueryResolver queryResolver(this->m_current_db_cfg->settings.get(), this->m_current_db_cfg->m_aq_engine, *this->m_current_db_cfg->baseDesc.get());
			aq::Logger::getInstance().log(AQ_INFO, "execute query %s\n", sqlQuery.c_str());
			if( (nRet = queryResolver.SolveSQLStatement(pNode)) != 0 )
			{
				oss << "error converting into prefix form sql query '" << sqlQuery << "'" << std::endl;
				aq::Logger::getInstance().log(AQ_ERROR, oss.str().c_str());
				throw generic_error(generic_error::GENERIC, oss.str());
			}

			//
			// read result file and deliver on the socket
			std::ifstream answerFile(this->m_current_db_cfg->settings->szAnswerFN);
			std::string bloc;
			std::string line;
			while (std::getline(answerFile, line))
			{
				line += "\n";
				bloc += line;
				if (bloc.size() > 2048)
				{
					this->deliver(bloc);
					bloc = "";
				}
			}
			if (bloc.size())
				this->deliver(bloc);

			oss << "sql request successfully executed" << std::endl;
		}
		catch (const generic_error& ex)
		{
			aq::Logger::getInstance().log(AQ_ERROR, "%s\n", ex.what());
			oss << "error during sql processing: " << ex.what() << std::endl;
		}

		boost::posix_time::ptime end(boost::posix_time::microsec_clock::local_time());
		oss << "Time elapsed: " << (end - begin) << std::endl;
	}
	else
	{
		oss << "not connected to any database" << std::endl;
		aq::Logger::getInstance().log(AQ_NOTICE, oss.str().c_str());
	}
	this->deliver(oss.str());
	this->m_demuxer->unpause();
}
