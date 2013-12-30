#include "AQEngineBasic.h"

using namespace aq;

void AQEngineBasic::call(const std::string&, aq::AQEngine_Intf::mode_t)
{
}

void AQEngineBasic::call(const aq::core::SelectStatement& stmt, aq::AQEngine_Intf::mode_t mode)
{
}

void renameResult(unsigned int, std::vector<std::pair<std::string, std::string> >&)
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
