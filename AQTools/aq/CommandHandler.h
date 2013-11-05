#ifndef __AQ_COMMAND_HANDLER_H__
#define __AQ_COMMAND_HANDLER_H__

#include <aq/Settings.h>
#include <aq/Base.h>

namespace aq
{

class CommandHandler
{
public:
  CommandHandler(const std::string& _dbsPath, const std::string& _dbName, aq::Settings& _settings, aq::Base& _baseDesc) 
    : databasesPath(_dbsPath), databaseName(_dbName), settings(_settings), baseDesc(_baseDesc)
  {
  }

  int process(const std::string& cmd);

private:
  std::string    databasesPath;
  std::string    databaseName;
  aq::Settings & settings;
  aq::Base     & baseDesc;
};

}

#endif