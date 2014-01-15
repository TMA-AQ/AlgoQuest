#ifndef __AQEngineBasic_H__
#define __AQEngineBasic_H__

#include <aq/AQEngine_Intf.h>

namespace aq
{

class AQEngineBasic : public aq::engine::AQEngine_Intf
{
public:
  void prepare() const {}
  void clean() const {}
  void call(const std::string& query, aq::engine::AQEngine_Intf::mode_t mode);
	void call(const aq::core::SelectStatement& stmt, aq::engine::AQEngine_Intf::mode_t mode);
  
  void renameResult(unsigned int id, std::vector<std::pair<std::string, std::string> >& resultTables) 
    {
      // TODO
    }
	boost::shared_ptr<aq::engine::AQMatrix> getAQMatrix();
	const std::vector<llong>& getTablesIDs() const;

private:
	std::vector<llong> tableIDs;
};

}

#endif
