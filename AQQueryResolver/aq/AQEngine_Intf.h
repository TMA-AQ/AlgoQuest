#pragma once

#include "SQLParser.h"
#include "AQMatrix.h"
#include "Settings.h"

class AQEngineCallback_Intf
{
public:
	virtual void getValue(uint64_t key, size_t packet, uint8_t*& values, size_t& size) const = 0;
};

class AQEngine_Intf
{
public:
	virtual ~AQEngine_Intf() {}

	virtual void call(tnode *pNode, int mode, int selectLevel) = 0;

	virtual boost::shared_ptr<aq::AQMatrix> getAQMatrix() = 0;
	virtual const std::vector<llong>& getTablesIDs() const = 0;

	// virtual void run(Base& BaseDesc, const char * query, AQEngineCallback_Intf * callback);
};
