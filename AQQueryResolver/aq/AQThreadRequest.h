#ifndef __AQTHREADREQUEST_H__
# define __AQTHREADREQUEST_H__

# include <vector>

# include <aq/AQThread.h>
# include <aq/TreeUtilities.h>
# include <aq/AQEngine_Intf.h>

namespace aq
{

  template <class T>
  class AQThreadRequest
  {
  public:

    // creator
    //--------------------------------------------------------------------------------------------

    AQThreadRequest(int nbrThread, aq::TProjectSettings* settings, aq::AQEngine_Intf* aqEngine, aq::Base& baseDesc)
      : _nbrThread(nbrThread), _settings(settings), _aqEngine(aqEngine), _baseDesc(baseDesc)
    {
      srand(static_cast<unsigned int>(time(NULL))); //temporaire
    }

    ~AQThreadRequest() {
      for (size_t idx = 0; idx < this->_thread.size(); ++idx)
        delete this->_thread[idx];
    }
    
    //--------------------------------------------------------------------------------------------


    // solve the request / cut / thread
    //--------------------------------------------------------------------------------------------

    void  solveRequest(aq::tnode* pNode) {
      this->parseAndCutThread(pNode);
      this->determinatePatern();
      this->solveThread();
    }

    void  parseAndCutThread(aq::tnode* pNode) {
      if (pNode == NULL)
        return;

      if (pNode->getTag() == K_SELECT)
        this->addThreadToList(pNode);

      this->parseAndCutThread(pNode->left);
      this->parseAndCutThread(pNode->right);
      this->parseAndCutThread(pNode->next);
    }

    void  addThreadToList(aq::tnode* pNode) {
      //-----------------------------------------------
      // ICI EST INSTANCIE LE QUERYRESOLVER
      //-----------------------------------------------
      T*  resolver = new T(pNode, this->_settings, this->_aqEngine, this->_baseDesc);
      AQThread<T>* thread = new AQThread<T>(pNode, resolver);
      //-----------------------------------------------

      for (typename std::vector<AQThread<T>*>::iterator it = this->_thread.begin();
        it != this->_thread.end(); ++it)
        if ((*it)->findNodeIn((*it)->getPNode(), pNode) == true && (*it)->getPNode() != pNode && *it != thread)
          (*it)->addListThreadCondition(thread);

      this->_thread.push_back(thread);
    }
    
    //--------------------------------------------------------------------------------------------


    // determinate the patern of each thread
    //--------------------------------------------------------------------------------------------

    void  determinatePatern() {
      this->deleteOverList();
      this->assignPatern();
    }

    void  deleteOverList() {
      this->_thread.front()->purgeOverList();
    }

    void  assignPatern() {
      this->_thread.front()->assignPatern(NULL);
    }
     
    //--------------------------------------------------------------------------------------------


    // lunch and solve each thread
    //--------------------------------------------------------------------------------------------

    void  solveThread() const {
      int count = 0;
      int nbrThread;
      while (this->_thread.front()->threadOver() != true)
      {
        ++count;
        nbrThread = 0;
        for (auto& th : this->_thread)
        {
          if (th->isReady() && !th->threadOver() && !th->isThreading() && (nbrThread < this->_nbrThread))
          {
            th->start(count, (rand() % 5));
            ++nbrThread;
          }
        }
        for (auto& th : this->_thread)
          if (th->isInitialaze() == true)
            th->join();
      }
    }
     
    //--------------------------------------------------------------------------------------------


    // dump
    //--------------------------------------------------------------------------------------------

    void  dump(std::ostream& os)        const {
      this->_thread.front()->dump(os, "  ");
    }
     
    //--------------------------------------------------------------------------------------------

  private:
    
    // attribute part
    //--------------------------------------------------------------------------------------------

    int                                           _nbrThread;
    aq::TProjectSettings*                         _settings;
    aq::AQEngine_Intf*                            _aqEngine;
    aq::Base&                                     _baseDesc;

    std::vector<AQThread<T>*> _thread;
     
    //--------------------------------------------------------------------------------------------

  };

}

#endif // __AQTHREADREQUEST_H__
