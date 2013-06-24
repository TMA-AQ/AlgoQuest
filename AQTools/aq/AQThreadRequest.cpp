#include "AQThreadRequest.h"

namespace aq
{

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
    littleThread* thread = new littleThread(pNode);

    for (std::vector<littleThread*>::iterator it = this->_thread.begin(); it != this->_thread.end(); ++it)
      if ((*it)->findNodeIn((*it)->_pNode, pNode) == true && (*it)->_pNode != pNode && *it != thread)
        (*it)->addListThreadCondition(thread);

    this->_thread.push_back(thread);
  }

  void AQThreadRequest::solveThread()
  {
    int count = 0;
    while (this->_thread.front()->threadOver() != true)
    {
      ++count;
      for (std::vector<littleThread*>::iterator it = this->_thread.begin(); it != this->_thread.end(); ++it)
        if ((*it)->isReady() == true && (*it)->threadOver() == false && (*it)->isThreading() == false)
          (*it)->solveThread(count);
    }
  }

}