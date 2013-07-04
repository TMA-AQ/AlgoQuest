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
        for (std::vector<AQThread*>::const_iterator it = this->_listThread.begin(); it != this->_listThread.end(); ++it)
          if (aqthread->getPNode() == (*it)->getPNode())
            return true;

      for (std::vector<AQThread*>::const_iterator it = this->_listThread.begin(); it != this->_listThread.end(); ++it)
        if ((*it)->existanceThread(aqthread, 1) == true)
          return true;

      return false;
    }

    void  purgeOverList() {
      std::vector<AQThread*>::iterator it = this->_listThread.begin();
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
      for (std::vector<AQThread*>::iterator it = this->_listThread.begin(); it != this->_listThread.end(); ++it)
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
      for (std::vector<AQThread*>::iterator it = this->_listThread.begin(); it != this->_listThread.end(); ++it)
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
      for (std::vector<AQThread*>::iterator it = this->_listThread.begin(); it != this->_listThread.end(); ++it)
        if ((*it)->threadOver() == false || (*it)->isThreading() == true)
          this->_ready = false;
      return this->_ready;
    }
    
    //--------------------------------------------------------------------------------------------


    // getter
    //--------------------------------------------------------------------------------------------

    aq::tnode*  getPNode()  const {
      return this->_pNode;
    }
    
    //--------------------------------------------------------------------------------------------


    // dump part
    //--------------------------------------------------------------------------------------------

    void  dump(int level)    const {
      if (this->_patern != NULL)
      {
        for (int i = 0; i < level; ++i)
          std::cout << "  ";
        std::cout << "My father is [" << this->_patern->_pNode << "] and:" << std::endl;
      }
      for (int i = 0; i < level; ++i)
        std::cout << "  ";
      std::cout << "I'm [" << this->_pNode << "]";
      if (this->_listThread.size() == 0)
        std::cout << std::endl;
      else
      {
        std::cout << " and my son(s) is(are) : " << std::endl;
        for (int i = 0; i < level; ++i)
          std::cout << "  ";
        std::cout << "-->" << std::endl;
      }
      for (std::vector<AQThread*>::const_iterator it = this->_listThread.begin(); it != this->_listThread.end(); ++it)
        (*it)->dump(level + 1);
      if (this->_listThread.size() > 0)
      {
        for (int i = 0; i < level; ++i)
          std::cout << "  ";
        std::cout << "<--" << std::endl;
      }
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

    boost::thread           _thread;

    aq::tnode*              _pNode;

    AQThread*               _patern;
    std::vector<AQThread*>  _listThread;
    
    //--------------------------------------------------------------------------------------------

  };

}

#endif // __AQTHREAD_H__