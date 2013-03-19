#ifndef __DEMUXER_H__
#define __DEMUXER_H__

#include "CommandHandler.h"
#include <map>
#include <boost/weak_ptr.hpp>
#include <boost/function.hpp>

namespace aq {
	namespace protocol {

		class Demuxer
		{
		public:
			Demuxer(boost::shared_ptr<CommandHandler> handler);
			void push(std::string& data);
			void pause() { m_pause = true; }
			void unpause() { m_pause = false; }
		private:
			typedef boost::function<void (boost::shared_ptr<CommandHandler>, std::string&)> f_ptr_t;
			typedef std::map<std::string, f_ptr_t> dispatcher_t;

			void run(std::map<std::string, f_ptr_t>::const_iterator, std::string& args);

			dispatcher_t dispatcher;
			boost::weak_ptr<CommandHandler> m_handler;
			std::string m_data;
			bool m_pause;
		};

	}
}

#endif
