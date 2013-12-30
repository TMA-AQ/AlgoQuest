#pragma once

#include "AQEngine_Intf.h"
#include <aq/Utilities.h>

namespace aq
{

class AQEngine : public AQEngine_Intf
{
public:
	AQEngine(Base& _baseDesc, Settings& _settings);
	~AQEngine();
  
  void call(const std::string& query, mode_t mode = aq::AQEngine_Intf::mode_t::REGULAR);
  void call(const aq::core::SelectStatement& query, mode_t mode = mode_t::REGULAR);
  
  void renameResult(unsigned int id, std::vector<std::pair<std::string, std::string> >& resultTables);
	boost::shared_ptr<aq::AQMatrix> getAQMatrix() { return aqMatrix; }
	const std::vector<llong>& getTablesIDs() const { return tableIDs; }

  void prepare() const;
  void clean() const;

protected:
  virtual int run(const char * prg, const char * args) const = 0;

private:
	const Base& baseDesc;
	const Settings& settings;
  boost::shared_ptr<aq::AQMatrix> aqMatrix;
	std::vector<llong> tableIDs;
};

#ifdef WIN32

class AQEngineWindows : public AQEngine
{
public:
  AQEngineWindows(Base& _baseDesc, Settings& settings);
protected:
  int run(const char * prg, const char * args) const;
};

#endif

class AQEngineSystem : public AQEngine
{
public:
  AQEngineSystem(Base& _baseDesc, Settings& settings);
protected:
  int run(const char * prg, const char * args) const;
};

}
