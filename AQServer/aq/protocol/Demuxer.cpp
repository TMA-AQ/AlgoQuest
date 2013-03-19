#include "Demuxer.h"
#include <aq/Logger.h>
#include <boost/shared_ptr.hpp>
#include <boost/algorithm/string/trim.hpp>
#include <boost/algorithm/string/case_conv.hpp>
#include <boost/algorithm/string/replace.hpp>

using namespace aq::protocol;

Demuxer::Demuxer(boost::shared_ptr<CommandHandler> handler)
	: m_handler(handler),
	  m_pause(false)
{
	this->dispatcher.insert(std::make_pair("auth", &CommandHandler::onAuth));
	this->dispatcher.insert(std::make_pair("connect", &CommandHandler::onConnect));
	this->dispatcher.insert(std::make_pair("desc", &CommandHandler::onDesc));
	this->dispatcher.insert(std::make_pair("execute", &CommandHandler::onExecuteSQL));
	this->dispatcher.insert(std::make_pair("quit", &CommandHandler::onQuit));
	this->dispatcher.insert(std::make_pair("show", &CommandHandler::onShow));
	this->dispatcher.insert(std::make_pair("version", &CommandHandler::onVersion));
}

void Demuxer::push(std::string& data)
{
	this->m_data += data;
	std::string::size_type end = this->m_data.find('\n');
	while ((end != std::string::npos) && !m_pause)
	{
		std::string cmd = this->m_data.substr(0, end);
		aq::Logger::getInstance().log(AQ_INFO, "cmd: %s\n", cmd.c_str());

		boost::algorithm::to_lower(cmd);

		std::string el1, el2;
		std::string::size_type pos = cmd.find_first_of(" ");
		if (pos != std::string::npos)
		{
			el1 = cmd.substr(0, pos);
			el2 = cmd.substr(pos);
		}
		else
		{
			el1 = cmd;
		}

		// remove white spaces
		boost::algorithm::trim(el1);
		boost::algorithm::trim(el2);  

		std::map<std::string, f_ptr_t>::const_iterator it_dispatch = this->dispatcher.find(el1);
		boost::shared_ptr<CommandHandler>	ptr = this->m_handler.lock();
		if (it_dispatch != this->dispatcher.end())
		{
			it_dispatch->second(ptr, el2);
		}
		else
		{
			std::ostringstream oss;
			oss << "cannot find command: '" << cmd << "'" << std::endl;
			aq::Logger::getInstance().log(AQ_ERROR, oss.str().c_str());
			ptr->deliver(oss.str());
		}

		this->m_data = this->m_data.substr(end + 1);
		end = this->m_data.find('\n');

	}
}
