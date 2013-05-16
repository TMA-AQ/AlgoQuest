#include "SyncSession.h"

#include <aq/AQEngine.h>
#include <aq/SQLPrefix.h>
#include <aq/Column2Table.h>
#include <aq/QueryResolver.h>
#include <aq/parser/SQLParser.h>
#include <aq/parser/JeqParser.h>
#include <aq/Exceptions.h>
#include <aq/Logger.h>

#include <fstream>
#include <boost/lexical_cast.hpp>
#include <boost/date_time/posix_time/posix_time.hpp>
#include <boost/uuid/uuid.hpp>
#include <boost/uuid/uuid_generators.hpp>
#include <boost/uuid/uuid_io.hpp>
#include <boost/filesystem.hpp>

namespace fs = boost::filesystem;

#ifndef __AQSERVER_VERSION__ 
# define __AQSERVER_VERSION__ "0.0.1"
#endif

using namespace aq;
using namespace aq::sync;

typedef boost::tokenizer<boost::char_separator<char> > tokenizer;

Session::Session(boost::asio::io_service& io_service, Configuration::Ptr cfg, unsigned int sessionId) 
	: 
	m_socket(io_service),
	m_cfg(cfg),
	m_sessionId(sessionId),
	m_stop(false)
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
		while (!this->m_stop)
			this->read();
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

void Session::read()
{
	aq::Logger::getInstance().log(AQ_DEBUG, "read on socket [sessionId:%u]\n", m_sessionId);

	boost::system::error_code error;
	size_t length = m_socket.read_some(boost::asio::buffer(m_recvBuf), error);
	
	if (error == boost::asio::error::eof)
	{
		m_stop = true;
		return; // Connection closed cleanly by peer.
	}
	else if (error)
	{
		m_stop = true;
		throw boost::system::system_error(error); // Some other error.
	}

	// push in demuxer
	char * buf = new char[length + 1]; 
	::memcpy(buf, this->m_recvBuf.data(), length);
	buf[length] = 0;
	std::string cmd(buf);
	aq::Logger::getInstance().log(AQ_DEBUG, "push in aq demuxer a buffer of size %u [sessionId:%u]\n", cmd.size(), m_sessionId);
	m_demuxer->push(cmd);
}

void Session::deliver(const std::string& buffer)
{
	aq::Logger::getInstance().log(AQ_DEBUG, "write on socket a buffer of size %u [sessionId:%u]\n", buffer.size(), m_sessionId);
  boost::asio::write(m_socket, boost::asio::buffer(buffer.c_str(), buffer.size()));
}

// ---------------------------------------------------------------------------
// Commands Handlers

void Session::onAuth(std::string& args)
{
	aq::Logger::getInstance().log(AQ_DEBUG, "[session:%u]\n", this->m_sessionId);
	// todo
}

void Session::onConnect(std::string& dbName)
{
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
	aq::Logger::getInstance().log(AQ_DEBUG, "[session:%u]\n", this->m_sessionId);
	std::ostringstream oss;
	if (this->m_current_db_cfg)
	{
		this->m_current_db_cfg->baseDesc->dumpXml(oss);
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
	aq::Logger::getInstance().log(AQ_DEBUG, "[session:%u]\n", this->m_sessionId);
	this->processSQL(sqlQuery);
}

void Session::onQuit(std::string& args)
{
	aq::Logger::getInstance().log(AQ_DEBUG, "[session:%u]\n", this->m_sessionId);
	m_stop = true;
}

void Session::onShow(std::string& args)
{
	aq::Logger::getInstance().log(AQ_DEBUG, "[session:%u]\n", this->m_sessionId);
	std::ostringstream oss;
  oss << "<Databases>" << std::endl;
	for (Configuration::map_cfg_t::const_iterator it = this->m_cfg->begin(); it != this->m_cfg->end(); ++it)
	{
		oss << "<Database Name=\"" << it->first << "\"/>" << std::endl;
	}
  oss << "</Databases>" << std::endl;
	this->deliver(oss.str());
}

void Session::onVersion(std::string& args)
{
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
			TProjectSettings settings(*m_current_db_cfg->settings);
			std::string displayFile;

			//
			// generate ident and ini file
			std::string queryIdentStr = "";
			boost::uuids::uuid queryIdent = boost::uuids::random_generator()();
			std::ostringstream queryIdentOSS;
			queryIdentOSS << queryIdent;
			queryIdentStr = queryIdentOSS.str();
			settings.changeIdent(queryIdentOSS.str());

			//
			// create directories
			std::list<fs::path> lpaths;
			lpaths.push_back(fs::path(settings.szRootPath + "/calculus/" + queryIdentOSS.str()));
			lpaths.push_back(fs::path(settings.szTempPath1));
			lpaths.push_back(fs::path(settings.szTempPath2));
			for (std::list<fs::path>::const_iterator dir = lpaths.begin(); dir != lpaths.end(); ++dir)
			{
				std::ostringstream ossDirName;
				ossDirName << *dir;
				aq::Logger::getInstance().log(AQ_DEBUG, "create directory '%s'\n", ossDirName.str().c_str());

				if (fs::exists(*dir))
				{
					aq::Logger::getInstance().log(AQ_ERROR, "directory already exist '%s'\n", ossDirName.str().c_str());
					return;
				}

				if (!fs::create_directory(*dir))
				{
					aq::Logger::getInstance().log(AQ_ERROR, "cannot create directory '%s'\n", ossDirName.str().c_str());
					return;
				}

				aq::Logger::getInstance().log(AQ_DEBUG, "directory '%s' created\n", ossDirName.str().c_str());
			}

			//
			// write ini file (it is needed for now by AQEngine)
			std::ofstream iniFile(settings.iniFile.c_str());
			settings.writeAQEngineIni(iniFile);
			iniFile.close();

			// generate answer file
			displayFile = settings.szRootPath + "/calculus/" + queryIdentStr + "/display.txt"; // TODO
			aq::Logger::getInstance().log(AQ_INFO, "save answer to %s\n", displayFile.c_str());


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
			unsigned int id = 1;
      QueryResolver queryResolver(pNode, &settings, this->m_current_db_cfg->m_aq_engine, *this->m_current_db_cfg->baseDesc.get(), id);
			aq::Logger::getInstance().log(AQ_INFO, "execute query %s\n", sqlQuery.c_str());
			queryResolver.solve();

      //
      // Generate result file
      Table::Ptr result = queryResolver.getResult();
      if (result)
      {
        aq::Timer timer;
        result->saveToAnswer(settings.szAnswerFN, settings.fieldSeparator);
        aq::Logger::getInstance().log(AQ_INFO, "Save Answer: Time Elapsed = %s\n", aq::Timer::getString(timer.getTimeElapsed()).c_str());
        std::ofstream fresult(settings.szAnswerFN, std::ios::app);
        fresult << "EOS";
        fresult.close();
      }

			//
			// read result file and deliver on the socket
			aq::Logger::getInstance().log(AQ_DEBUG, "read answer file %s\n", settings.szAnswerFN);
			std::ifstream answerFile(settings.szAnswerFN);
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
}
