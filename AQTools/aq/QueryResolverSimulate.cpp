#include "QueryResolverSimulate.h"

#include <boost/timer.hpp>
#include <boost/version.hpp>
# include <boost/thread/thread.hpp>
#if BOOST_VERSION < 105000
#define TIME_UTC_ TIME_UTC
#endif

namespace aq
{

  QueryResolverSimulate::QueryResolverSimulate(aq::tnode* pNode, aq::TProjectSettings* settings, aq::AQEngine_Intf* aqEngine, aq::Base& baseDesc)
    : _node(pNode), _settings(settings), _aqEngine(aqEngine), _baseDesc(baseDesc)
  {
    this->_id = "";
    for (int i = 0; i < 10; ++i)
      this->_id += static_cast<char>((rand() % 26) + 65);
    this->dump();
  }

  void  QueryResolverSimulate::solve()
  {
    boost::xtime xt; // a virer, c'est pour les tests
    boost::xtime_get(&xt, boost::TIME_UTC_); // a virer, c'est pour les tests
    xt.sec += (this->getTime() * 2); // a virer, c'est pour les tests
    boost::thread::sleep(xt); // a virer, c'est pour les tests
    this->setAnswer();

    if (this->_answerTable.size() > 0)
      std::cout << "L'ID ou les ID de mon/mes enfant(s) est/sont: " << std::endl;
    for (std::map<std::string, boost::shared_ptr<QueryResolverSimulate> >::iterator it = this->_answerTable.begin();
      it != this->_answerTable.end(); ++it)
      it->second->dump();
  }

  void  QueryResolverSimulate::setAnswer()
  {
    boost::shared_ptr<QueryResolverSimulate> result(this);
    this->_answer = result;
  }

  void  QueryResolverSimulate::addQueryResolverSimulate(std::string alias,
    boost::shared_ptr<QueryResolverSimulate> answer)
  {
    this->_answerTable.insert(std::make_pair(alias, answer));
  }

  void  QueryResolverSimulate::setTime(int time)
  {
    this->_time = time;
  }

  int   QueryResolverSimulate::getTime()                                        const
  {
    return this->_time;
  }

  boost::shared_ptr<QueryResolverSimulate>  QueryResolverSimulate::getAnswer()  const
  {
    return this->_answer;
  }

  const std::string&  QueryResolverSimulate::getID()                            const
  {
    return this->_id;
  }

  void QueryResolverSimulate::dump()                                            const
  {
    std::cout << "My unique ID is: " << this->_id << " and my node is: " << this->_node << std::endl;
  }
}