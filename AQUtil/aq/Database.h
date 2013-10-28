#ifndef __AQ_DATABASE_H__
#define __AQ_DATABASE_H__

#include <string>
#include <sstream>
#include "BaseDesc.h"

namespace aq
{

class Database
{
public:
  Database(const std::string& _path);
  void dump(std::ostream& os) const;
  bool isValid() const;
  void create();
  std::string getName() const;
  aq::base_t getBaseDesc() const;

private:
  int load();
  std::string getWorkingPath() const;
  std::string getDataPath() const;
  std::string getBaseDescFile() const;

  std::string path;
  aq::base_t baseDesc;
};

}

#endif
