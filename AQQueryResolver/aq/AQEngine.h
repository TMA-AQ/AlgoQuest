#pragma once

#include "AQEngine_Intf.h"
#include <aq/Utilities.h>
#include "Table.h"

namespace aq
{

class AQEngine : public AQEngine_Intf
{
public:
	AQEngine(Base& _baseDesc, Settings& _settings);
	~AQEngine();
  
  void call(const std::string& query, mode_t mode);
	void call(aq::tnode *pNode, mode_t mode, int selectLevel);
  virtual int run(const char * prg, const char * args) const = 0;
  
  void renameResult(unsigned int id, std::vector<std::pair<std::string, std::string> >& resultTables);
	boost::shared_ptr<aq::AQMatrix> getAQMatrix() { return aqMatrix; }
	const std::vector<llong>& getTablesIDs() const { return tableIDs; }

private:
	void generateAQMatrixFromPRM(const std::string prmFile, aq::tnode * whereNode);

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
  int run(const char * prg, const char * args) const;
};

#endif

class AQEngineSystem : public AQEngine
{
public:
  AQEngineSystem(Base& _baseDesc, Settings& settings);
  int run(const char * prg, const char * args) const;
};

}
