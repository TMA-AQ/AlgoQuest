#ifndef __AQEngineBasic_H__
#define __AQEngineBasic_H__

#include <aq/AQEngine_Intf.h>

namespace aq
{

class AQEngineBasic : public AQEngine_Intf
{
public:
  void call(const std::string& query, aq::AQEngine_Intf::mode_t mode);
	void call(TProjectSettings& settings, 
		tnode *pNode, 
		aq::AQEngine_Intf::mode_t mode, 
		int selectLevel);
  
  void renameResult(unsigned int id, std::vector<std::pair<std::string, std::string> >& resultTables) 
    {
      // TODO
    }
	boost::shared_ptr<aq::AQMatrix> getAQMatrix();
	const std::vector<llong>& getTablesIDs() const;

private:
	std::vector<llong> tableIDs;
};

}

#endif
