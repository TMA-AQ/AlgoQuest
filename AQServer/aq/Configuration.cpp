#include "Configuration.h"
#include <aq/Logger.h>
#include <SQLParser/Exceptions.h>
#include <fstream>
#include <boost/property_tree/ptree.hpp>
#include <boost/property_tree/xml_parser.hpp>
#include <boost/algorithm/string/trim.hpp>
#include <boost/algorithm/string/replace.hpp>
#include <boost/algorithm/string/case_conv.hpp>
#include <SQLParser/AQEngine.h>

using namespace aq;

Configuration::Configuration()
{
}

Configuration::Configuration(const char * cfgFile)
	: m_cfgFile(cfgFile)
{
	this->load();
}

Configuration::base_cfg::Ptr Configuration::getCfg(const std::string& dbName) const
{
	map_cfg_t::const_iterator it = this->m_cfgs.find(dbName);
	if (it == this->m_cfgs.end())
	{
		aq::Logger::getInstance().log(AQ_NOTICE, "no db '%s'\n", dbName.c_str());
		throw generic_error(generic_error::GENERIC, "cannot find db");  
	}
	return it->second;
}

void Configuration::load(const char * cfgFile)
{
	this->m_cfgFile = cfgFile;
}

void Configuration::load()
{
	this->m_cfgs.clear();
	try
	{
		std::ifstream fin(this->m_cfgFile.c_str(), std::ifstream::in);
		boost::property_tree::ptree pt;
		boost::property_tree::xml_parser::read_xml(fin, pt);

		for (boost::property_tree::ptree::iterator itDB = pt.get_child("aq_config.databases").begin(); itDB != pt.get_child("aq_config.databases").end(); ++itDB)
		{
			std::string dbName = itDB->second.get<std::string>("dbname");
			std::string iniFile = itDB->second.get<std::string>("iniFile");
			boost::algorithm::to_lower(dbName);
			base_cfg::Ptr cfg(new base_cfg);
			// settings->load(iniFile, "[QUERY_IDENT]");
			cfg->settings->load(iniFile, "test"); // tma FIXME
			cfg->baseDesc->loadFromBaseDesc(cfg->settings->szDBDescFN);
			cfg->m_aq_engine = new AQEngine(); 
			this->m_cfgs.insert(std::make_pair(dbName, cfg));
		}
	}
	catch (const boost::property_tree::ptree_error& e)
	{
		aq::Logger::getInstance().log(AQ_ERROR, "%s\n", e.what());
		std::ostringstream oss;
		oss << "invalid ini file: " << this->m_cfgFile << std::endl;
		throw generic_error(generic_error::INVALID_FILE, oss.str());
	}
}

void Configuration::dump(std::ostream& os) const
{
	for (map_cfg_t::const_iterator it = this->m_cfgs.begin(); it != this->m_cfgs.end(); ++it)
	{
		os << "database '" << it->first << "' configuration:" << std::endl;
		it->second->settings->dump(os);
	}
}