#pragma once

#include "AQEngine_Intf.h"
#include <aq/Utilities.h>
#include "Table.h"

class AQEngine : public AQEngine_Intf
{
public:
	AQEngine(Base& _baseDesc);
	~AQEngine();

	void call(TProjectSettings& settings,
				tnode *pNode, 
				int mode, 
				int selectLevel);

	boost::shared_ptr<aq::AQMatrix> getAQMatrix() { return aqMatrix; }
	const std::vector<llong>& getTablesIDs() const { return tableIDs; }

private:
	void generateAQMatrixFromPRM(const std::string prmFile, tnode * whereNode);

	const Base& baseDesc;
	boost::shared_ptr<aq::AQMatrix> aqMatrix;
	std::vector<llong> tableIDs;
};

