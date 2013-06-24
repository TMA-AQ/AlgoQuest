#ifndef __AQENGINESIMULATE_H__
# define __AQENGINESIMULATE_H__

# include <aq/AQEngine_Intf.h>
# include <aq/SQLPrefix.h>
# include <aq/parser/JeqParser.h>
# include <aq/Logger.h>
# include <aq/Settings.h>
# include <aq/base.h>

namespace aq
{

  class AQEngineSimulate : public aq::AQEngine_Intf
  {
  public:
    AQEngineSimulate(Base& _baseDesc, TProjectSettings& settings);
    void call(aq::tnode * pNode, aq::AQEngine_Intf::mode_t mode, int selectLevel);

    void setAQMatrix(boost::shared_ptr<aq::AQMatrix> _aqMatrix);
    void setTablesIDs(std::vector<llong>& _tableIDs);

    boost::shared_ptr<aq::AQMatrix>   getAQMatrix();
    const std::vector<llong>&         getTablesIDs() const;

    void  AQEngineSimulate::createTableIDs(aq::tnode* pNode);

  private:
    boost::shared_ptr<aq::AQMatrix> aqMatrix;
    std::vector<llong> tableIDs;
    const Base& baseDesc;
    const TProjectSettings& settings;
  };

}

#endif // __AQENGINESIMULATE_H__