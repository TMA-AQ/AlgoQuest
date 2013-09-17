#ifndef __AQTHREAD_H__
# define __AQTHREAD_H__

# include <vector>
# include <aq/TreeUtilities.h>
# include <boost/thread/thread.hpp>

#include <aq/TreeUtilities.h> // pour les tests
#include <aq/parser/SQLParser.h> // pour les tests
#include <aq/SQLPrefix.h> // pour les tests

namespace aq
{
  template <class T>
  class AQThread
  {
  public:
    
    // creator
    //--------------------------------------------------------------------------------------------

    AQThread(aq::tnode* pNode, T* resolver) {
      this->_pNode = pNode;
      this->_queryResolver = resolver;
      this->_patern = NULL;
      this->_end = false;
      this->_threading = false;
      this->_ready = false; 
      this->_initialaze = false;
    }
    
    //--------------------------------------------------------------------------------------------


    // initialization
    //--------------------------------------------------------------------------------------------

    bool  findNodeIn(aq::tnode* pNode, aq::tnode* tNode) {
      if (pNode == NULL)
        return false;

      if (pNode == tNode)
        return true;

      if (this->findNodeIn(pNode->next, tNode) == true)
        return true;
      if (this->findNodeIn(pNode->left, tNode) == true)
        return true;
      if (this->findNodeIn(pNode->right, tNode) == true)
        return true;

      return false;
    }

    void  addListThreadCondition(AQThread* thread) {
      this->_listThread.push_back(thread);
    }
    
    //--------------------------------------------------------------------------------------------


    // resolve patern and sons
    //--------------------------------------------------------------------------------------------

    bool  existanceThread(AQThread* aqthread, int level) const {
      if (level == 1)
        for (typename std::vector<AQThread*>::const_iterator it = this->_listThread.begin(); it != this->_listThread.end(); ++it)
          if (aqthread->getPNode() == (*it)->getPNode())
            return true;

      for (typename std::vector<AQThread*>::const_iterator it = this->_listThread.begin(); it != this->_listThread.end(); ++it)
        if ((*it)->existanceThread(aqthread, 1) == true)
          return true;

      return false;
    }

    void  purgeOverList() {
      typename std::vector<AQThread*>::iterator it = this->_listThread.begin();
      while (it != this->_listThread.end())
        if (this->existanceThread(*it, 0) == true)
          it = this->_listThread.erase(it);
        else
          ++it;

      for (it = this->_listThread.begin(); it != this->_listThread.end(); ++it)
        (*it)->purgeOverList();
    }

    void  assignPatern(AQThread* patern) {
      this->setPatern(patern);
      for (typename std::vector<AQThread*>::iterator it = this->_listThread.begin(); it != this->_listThread.end(); ++it)
        (*it)->assignPatern(this);
    }

    void  setPatern(AQThread* patern) {
      this->_patern = patern;
    }
    
    //--------------------------------------------------------------------------------------------


    // thread part
    //--------------------------------------------------------------------------------------------

    void  solveThread(int count, int time) {

      if (this->_threading == true)
        return;
      this->_threading = true;

      //test template queryresolverSimulate
      this->assignAnswer();
      this->_queryResolver->setTime(time);
      this->_queryResolver->solve();

      this->setTime(time * 2);
      this->setGlobalTime();


      // blabla thread debut
      std::cout << "Turn number: [" << count << "] with timer at [" << (time * 2)
        << "] & Select part is:" << std::endl << "[" << this->_pNode << "]" << std::endl;
      // blabla thread fin

      this->_threading = false;
      this->_end = true;
      this->_initialaze = false;


      //launch the patern
      if (this->_patern != NULL)
        if (this->_patern->isReady() == true && this->_patern->threadOver() == false && this->_patern->isThreading() == false)
        {
          std::cout << "--->  I call my patern to solveThread and:" <<std::endl;
          this->_queryResolver->dump();
          this->_patern->solveThread(count + 1, rand() % 5);
        }
    }

    void  start(int count, int time) {
      this->_initialaze = true;
      this->_thread = boost::thread(&AQThread::solveThread, this, count, time);
    }

    void  join()  {
      this->_thread.join();
    }

    void  assignAnswer() {
      for (typename std::vector<AQThread*>::iterator it = this->_listThread.begin(); it != this->_listThread.end(); ++it)
        this->_queryResolver->addQueryResolverSimulate((*it)->_queryResolver->getID(), (*it)->_queryResolver->getAnswer());
    }
    
    //--------------------------------------------------------------------------------------------


    // bool
    //--------------------------------------------------------------------------------------------

    bool  threadOver()      const {
      return this->_end;
    }

    bool  isThreading()     const {
      return this->_threading;
    }

    bool  isInitialaze()    const {
      return this->_initialaze;
    }

    bool  isReady() {
      this->_ready = true;
      for (typename std::vector<AQThread*>::iterator it = this->_listThread.begin(); it != this->_listThread.end(); ++it)
        if ((*it)->threadOver() == false || (*it)->isThreading() == true)
          this->_ready = false;
      return this->_ready;
    }
    
    //--------------------------------------------------------------------------------------------


    // getter / setter
    //--------------------------------------------------------------------------------------------

    aq::tnode*  getPNode()          const {
      return this->_pNode;
    }
    
    int         getTime()           const {
      return this->_time;
    }

    int         getGlobalTime()     const {
      return this->_globalTime;
    }

    void        setGlobalTime() {
      int globalTime = 0;
      for (const auto& it : this->_listThread)
        if (it->getGlobalTime() > globalTime)
          globalTime = it->getGlobalTime();
      this->_globalTime = this->_time + globalTime;
    }

    void        setTime(int time) {
      this->_time = time;
    }

    //--------------------------------------------------------------------------------------------


    // dump part
    //--------------------------------------------------------------------------------------------

    void  secretModule(std::string& str, const std::string& indent)                      const {
      str.insert(0, indent);
      for (size_t i = 0; i < str.size(); ++i)
        if (str.at(i) == '\n')
          str.insert(i + 1, indent);
    }

    void  presentation(std::ostream& os, std::string indent)  const {
      os << indent << "<globalTime> #- " << this->getGlobalTime() << " -# </globalTime>" << std::endl;
      os << indent << "<time> #- " << this->getTime() << " -# </time>" << std::endl;
      os << indent << "<adress> #- [" << this->_pNode << "] -# </adress>" << std::endl;
      std::string str;
      aq::generate_parent(this->_pNode, NULL);
      aq::syntax_tree_to_sql_form(this->_pNode, str);
      aq::multiline_query(str);
      this->secretModule(str, indent);
      os << indent << "<request>" << std::endl << str << std::endl << indent << "</request>" << std::endl;
    }

    void  dump(std::ostream& os, std::string indent)          const {
      os << indent << "<query>" << std::endl;
      this->presentation(os, indent);
      if (this->_listThread.size() > 0)
        os << indent << "<child>" << std::endl;
      for (typename std::vector<AQThread*>::const_iterator it = this->_listThread.begin(); it != this->_listThread.end(); ++it)
        (*it)->dump(os, indent + "  ");
      if (this->_listThread.size() > 0)
        os << indent << "</child>" << std::endl;
      os << indent << "</query>" << std::endl;
    }

    //--------------------------------------------------------------------------------------------

  private:

    // attribute part
    //--------------------------------------------------------------------------------------------

    T*                      _queryResolver;

    bool                    _end;
    bool                    _threading;
    bool                    _ready;
    bool                    _initialaze;

    int                     _time;
    int                     _globalTime;

    boost::thread           _thread;

    aq::tnode*              _pNode;

    AQThread*               _patern;
    std::vector<AQThread*>  _listThread;
    
    //--------------------------------------------------------------------------------------------

  };

}

#endif // __AQTHREAD_H__
