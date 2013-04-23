#ifndef __AQEngineBasic_H__
#define __AQEngineBasic_H__

#include <aq\AQEngine_Intf.h>

namespace aq
{

class AQEngineBasic : public AQEngine_Intf
{
public:

	void call(TProjectSettings& settings, 
		tnode *pNode, 
		int mode, 
		int selectLevel);

	boost::shared_ptr<aq::AQMatrix> getAQMatrix();
	const std::vector<llong>& getTablesIDs() const;

private:
	std::vector<llong> tableIDs;
};

}

#endif