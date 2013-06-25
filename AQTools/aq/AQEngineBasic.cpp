#include "AQEngineBasic.h"

using namespace aq;

void AQEngineBasic::call(const std::string&, aq::AQEngine_Intf::mode_t)
{
}

void AQEngineBasic::call(TProjectSettings& settings, 
												 tnode *pNode, 
												 aq::AQEngine_Intf::mode_t mode, 
												 int selectLevel)
{
}

boost::shared_ptr<aq::AQMatrix> AQEngineBasic::getAQMatrix()
{
	return boost::shared_ptr<aq::AQMatrix>();
}

const std::vector<llong>& AQEngineBasic::getTablesIDs() const
{
	return tableIDs;
}
