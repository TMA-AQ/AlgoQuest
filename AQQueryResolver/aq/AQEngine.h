#pragma once

#include "AQEngine_Intf.h"
#include <aq/Utilities.h>
#include "Table.h"

namespace aq
{

class AQEngine : public AQEngine_Intf
{
public:
	AQEngine(Base& _baseDesc, TProjectSettings& _settings);
	~AQEngine();

	void call(aq::tnode *pNode, mode_t mode, int selectLevel);
  virtual int run(const char * prg, const char * args) const = 0;

	boost::shared_ptr<aq::AQMatrix> getAQMatrix() { return aqMatrix; }
	const std::vector<llong>& getTablesIDs() const { return tableIDs; }

private:
	void generateAQMatrixFromPRM(const std::string prmFile, aq::tnode * whereNode);

	const Base& baseDesc;
	const TProjectSettings& settings;
  boost::shared_ptr<aq::AQMatrix> aqMatrix;
	std::vector<llong> tableIDs;
};

class AQEngineWindows : public AQEngine
{
public:
  AQEngineWindows(Base& _baseDesc, TProjectSettings& settings);
  int run(const char * prg, const char * args) const;
};

class AQEngineSystem : public AQEngine
{
public:
  AQEngineSystem(Base& _baseDesc, TProjectSettings& settings);
  int run(const char * prg, const char * args) const;
};

}