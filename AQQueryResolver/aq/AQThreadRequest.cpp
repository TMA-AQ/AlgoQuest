#include "AQThreadRequest.h"

namespace aq
{

  AQThreadRequest::AQThreadRequest(int nbrThread, aq::TProjectSettings& settings, aq::AQEngine_Intf* aqEngine, aq::Base& baseDesc)
    : _nbrThread(nbrThread), _settings(settings), _aqEngine(aqEngine), _baseDesc(baseDesc)
  {
    srand(static_cast<unsigned int>(time(NULL))); //temporaire
  }

  AQThreadRequest::~AQThreadRequest()
  {
    for (size_t idx = 0; idx < this->_thread.size(); ++idx)
      delete this->_thread[idx];
  }

  void AQThreadRequest::solveRequest(aq::tnode* pNode)
  {
    this->parseAndCutThread(pNode);
    this->determinatePatern();
    this->solveThread();
  }

  void AQThreadRequest::parseAndCutThread(aq::tnode* pNode)
  {
    if (pNode == NULL)
      return;

    if (pNode->getTag() == K_SELECT)
      this->addThreadToList(pNode);

    this->parseAndCutThread(pNode->left);
    this->parseAndCutThread(pNode->right);
    this->parseAndCutThread(pNode->next);
  }

  void AQThreadRequest::addThreadToList(aq::tnode* pNode)
  {
    QueryResolverSimulate*  resolver = new QueryResolverSimulate(pNode);
    AQThread<QueryResolverSimulate>* thread = new AQThread<QueryResolverSimulate>(pNode, resolver);

    for (std::vector<AQThread<QueryResolverSimulate>*>::iterator it = this->_thread.begin();
      it != this->_thread.end(); ++it)
      if ((*it)->findNodeIn((*it)->getPNode(), pNode) == true && (*it)->getPNode() != pNode && *it != thread)
        (*it)->addListThreadCondition(thread);

    this->_thread.push_back(thread);
  }

  void AQThreadRequest::determinatePatern()
  {
    this->deleteOverList();
    this->assignPatern();
  }

  void AQThreadRequest::deleteOverList()
  {
    this->_thread.front()->purgeOverList();
  }

  void AQThreadRequest::assignPatern()
  {
    this->_thread.front()->assignPatern(NULL);
  }

  void AQThreadRequest::solveThread() const
  {
    int count = 0;
    int nbrThread;
    while (this->_thread.front()->threadOver() != true)
    {
      ++count;
      nbrThread = 0;
      for (std::vector<AQThread<QueryResolverSimulate>*>::const_iterator it = this->_thread.begin();
        it != this->_thread.end(); ++it)
        if ((*it)->isReady() == true && (*it)->threadOver() == false && (*it)->isThreading() == false && nbrThread < this->_nbrThread)
        {
          (*it)->start(count, (rand() % 5));
          ++nbrThread;
        }
      for (std::vector<AQThread<QueryResolverSimulate>*>::const_iterator it = this->_thread.begin();
        it != this->_thread.end(); ++it)
        if ((*it)->isInitialaze() == true)
          (*it)->join();
    }
  }

  void AQThreadRequest::dump()        const
  {
    this->_thread.front()->dump(0);
  }

}