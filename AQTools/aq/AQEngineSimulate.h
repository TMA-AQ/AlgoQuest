#ifndef __AQENGINESIMULATE_H__
# define __AQENGINESIMULATE_H__

# include <aq/AQEngine_Intf.h>
# include <aq/SQLPrefix.h>
# include <aq/parser/JeqParser.h>
# include <aq/Logger.h>
# include <aq/Settings.h>
# include <aq/Base.h>

namespace aq
{

  class AQEngineSimulate : public aq::engine::AQEngine_Intf
  {
  public:
    AQEngineSimulate(const Base::Ptr _baseDesc, const Settings::Ptr _settings);

    void prepare() const {}
    void clean() const {}

    void call(const std::string& query, aq::engine::AQEngine_Intf::mode_t mode); 
    void call(const aq::core::SelectStatement& query, aq::engine::AQEngine_Intf::mode_t mode); 
    
    void renameResult(unsigned int id, std::vector<std::pair<std::string, std::string> >& resultTables);

    void setAQMatrix(aq::engine::AQMatrix::Ptr _aqMatrix);
    void setTablesIDs(std::vector<llong>& _tableIDs);
    
    aq::engine::AQMatrix::Ptr   getAQMatrix();
    const std::vector<llong>&         getTablesIDs() const;
    
  private:
    void createTableIDs(const std::string& query);
    void createTableIDs(aq::tnode* pNode);

    boost::shared_ptr<aq::engine::AQMatrix> aqMatrix;
    std::vector<llong> tableIDs;
    const Base::Ptr baseDesc;
    const Settings::Ptr settings;
  };

}

#endif // __AQENGINESIMULATE_H__
