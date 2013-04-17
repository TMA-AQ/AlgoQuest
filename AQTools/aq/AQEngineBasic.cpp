#include "AQEngineBasic.h"

using namespace aq;

void AQEngineBasic::call(TProjectSettings& settings, 
												 tnode *pNode, 
												 int mode, 
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
