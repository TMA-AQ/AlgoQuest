#ifndef __COMMAND_HANDLER_H__
#define __COMMAND_HANDLER_H__

#include <string>
#include "Demuxer.h"

namespace aq {
	namespace protocol {

		class CommandHandler
		{
		public:
			virtual ~CommandHandler() {}

			virtual void deliver(const std::string& data) = 0;
			virtual void onAuth(std::string& args) = 0;
			virtual void onConnect(std::string& dbName) = 0;
			virtual void onDesc(std::string& args) = 0;
			virtual void onExecuteSQL(std::string& sqlQuery) = 0;
			virtual void onQuit(std::string& args) = 0;
			virtual void onShow(std::string& args) = 0;
			virtual void onVersion(std::string& args) = 0;
		};

	}
}

#endif
