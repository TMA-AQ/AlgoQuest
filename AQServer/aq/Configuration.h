#ifndef __CONFIGURATION_H__
#define __CONFIGURATION_H__

#include <aq/Utilities.h>
#include <SQLParser/Table.h>
#include <SQLParser/AQEngine_Intf.h>

#include <map>
#include <boost/shared_ptr.hpp>

namespace aq
{

	class Configuration
	{
	public:

		struct base_cfg
		{
			typedef boost::shared_ptr<base_cfg> Ptr;
			base_cfg() : settings(new TProjectSettings), baseDesc(new Base), m_aq_engine(0) {}
			const TProjectSettings::Ptr settings;
			const boost::shared_ptr<Base> baseDesc;
			AQEngine_Intf * m_aq_engine; 
		};

		typedef boost::shared_ptr<Configuration> Ptr;
		typedef std::map<const std::string, base_cfg::Ptr> map_cfg_t;

		Configuration();
		Configuration(const char * cfgFile);

		void load(const char * cfgFile);
		void dump(std::ostream& os) const;

		base_cfg::Ptr getCfg(const std::string& dbName) const;
		map_cfg_t::const_iterator begin() const { return this->m_cfgs.begin(); }
		map_cfg_t::const_iterator end() const { return this->m_cfgs.end(); }

	private:
		std::string m_cfgFile;
		map_cfg_t m_cfgs;

		void load();
	};

}

#endif