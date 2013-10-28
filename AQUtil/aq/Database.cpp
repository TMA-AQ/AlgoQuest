#include "Database.h"
#include "Exceptions.h"
#include <fstream>
#include <boost/filesystem.hpp>
#include <boost/algorithm/string/replace.hpp>

namespace aq
{

Database::Database(const std::string& _path) 
  : path(_path)
{
  boost::replace_all(path, "\\", "/");
  if (!path.empty() && (*path.rbegin() != '/'))
  {
    path += "/";
  }
  this->load();
}

bool Database::isValid() const
{
  if (baseDesc.nom == "")
    return false;

  std::string bdFname = this->getBaseDescFile();
  boost::filesystem::path bdFile(bdFname);
  if (!boost::filesystem::exists(bdFile))
    return false;

  boost::filesystem::path workingPath(this->getWorkingPath());
  if (!boost::filesystem::exists(workingPath))
    return false;

  boost::filesystem::path dataPath(this->getDataPath());
  if (!boost::filesystem::exists(dataPath))
    return false;

  return true;
}

void Database::create()
{
  boost::filesystem::path p(this->path);
  if (!boost::filesystem::exists(p))
  {
    boost::filesystem::create_directories(p);
  }

  boost::filesystem::path bdFile(this->getBaseDescFile());
  if (!boost::filesystem::exists(bdFile))
  {
    boost::filesystem::create_directories(bdFile);
  }

  boost::filesystem::path workingPath(this->getWorkingPath());
  if (!boost::filesystem::exists(workingPath))
  {
    boost::filesystem::create_directories(workingPath);
  }

  boost::filesystem::path dataPath(this->getDataPath());
  if (!boost::filesystem::exists(dataPath))
  {
    boost::filesystem::create_directories(dataPath);
  }
}

std::string Database::getName() const
{
  std::string name;
  std::ifstream f(this->getBaseDescFile().c_str(), std::ios::in);
  if (f.is_open())
  {
    std::getline(f, name);
  }
  return name;
}

aq::base_t Database::getBaseDesc() const
{
  return baseDesc;
}

int Database::load()
{
  int rc = 0;
  std::string bdFname = this->getBaseDescFile();
  std::ifstream fin(bdFname.c_str(), std::ios::in);
  if (bdFname.find(".aqb") != std::string::npos)
    aq::build_base_from_raw(fin, baseDesc);
  else if (bdFname.find(".xml") != std::string::npos)
    aq::build_base_from_xml(fin, baseDesc);
  else
    rc = -1;
  return rc;
}

std::string Database::getWorkingPath() const
{
  return this->path + "calculus/";
}

std::string Database::getDataPath() const
{
  return this->path + "data_orga/vdg/data/";
}

std::string Database::getBaseDescFile() const
{
  return this->path + "base_struct/base.aqb";
}

void Database::dump(std::ostream& os) const
{
  os << this->getName() << std::endl;
  os << this->path << std::endl;
  os << this->getBaseDescFile() << std::endl;
  os << this->getWorkingPath() << std::endl;
  os << this->getDataPath() << std::endl;
  aq::dump_raw_base(os, this->baseDesc);
}

}
