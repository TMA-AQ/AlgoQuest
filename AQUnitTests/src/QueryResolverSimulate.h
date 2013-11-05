#ifndef __QUERYRESOLVERSIMULATE_H__
# define __QUERYRESOLVERSIMULATE_H__

# include <map>
# include <ostream>
# include <boost/variant.hpp>
# include <boost/shared_ptr.hpp>
# include <aq/TreeUtilities.h>
# include <aq/AQEngine_Intf.h>

namespace aq
{

  class QueryResolverSimulate
  {
  public:
    QueryResolverSimulate(aq::tnode* pNode, aq::Settings* settings, aq::AQEngine_Intf* aqEngine, aq::Base& baseDesc);
    ~QueryResolverSimulate() {}

    void  solve();

    void  setAnswer();
    void  addQueryResolverSimulate(std::string alias, boost::shared_ptr<QueryResolverSimulate> answer);

    void  setTime(int time);

    const std::string& getID()                              const;
    int   getTime()                                         const;
    boost::shared_ptr<QueryResolverSimulate>  getAnswer()   const;

    void dump()                                             const;

  private:
    int                                                               _time;
    std::string                                                       _id;

    aq::tnode*                                                        _node;
    aq::Settings*                                                     _settings;
    aq::AQEngine_Intf*                                                _aqEngine;
    aq::Base&                                                         _baseDesc;

    std::map<std::string, boost::shared_ptr<QueryResolverSimulate> >  _answerTable;
    boost::shared_ptr<QueryResolverSimulate>                          _answer;
  };

}

#endif //__QUERYRESOLVERSIMULATE_H__