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
  void create(aq::base_t& base);
  void erase();
  std::string getName() const;
  aq::base_t getBaseDesc() const;
  
  std::string getRootPath() const;

  std::string getWorkingPath() const;
  std::string getDataPath() const;
  std::string getBaseDescFile() const;

  std::string getTemporaryWorkingPath() const;
  std::string getTemporaryTableLoadPath() const;
  std::string getTemporaryColumnLoadPath() const;

private:
  int load();
  std::string path;
  aq::base_t baseDesc;
};

}

#endif
