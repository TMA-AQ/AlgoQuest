#include "AQThreadRequest.h"

namespace aq
{

  AQThreadRequest::~AQThreadRequest()
  {
    for (size_t idx = 0; idx < this->_thread.size(); ++idx)
      delete this->_thread[idx];
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
    AQThread* thread = new AQThread(pNode);

    for (std::vector<AQThread*>::iterator it = this->_thread.begin(); it != this->_thread.end(); ++it)
      if ((*it)->findNodeIn((*it)->_pNode, pNode) == true && (*it)->_pNode != pNode && *it != thread)
        (*it)->addListThreadCondition(thread);

    this->_thread.push_back(thread);
  }

  void AQThreadRequest::solveThread() const
  {
    int count = 0;
    while (this->_thread.front()->threadOver() != true)
    {
      ++count; // en attendant pour prouver l'ordre des threads
      for (std::vector<AQThread*>::const_iterator it = this->_thread.begin(); it != this->_thread.end(); ++it)
        if ((*it)->isReady() == true && (*it)->threadOver() == false && (*it)->isThreading() == false)
          (*it)->start(count);
      for (std::vector<AQThread*>::const_iterator it = this->_thread.begin(); it != this->_thread.end(); ++it)
        if ((*it)->isInitialaze() == true)
          (*it)->join();
    }
  }

}