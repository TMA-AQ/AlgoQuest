#include "littleThread.h"

#include <aq/TreeUtilities.h> // pour les tests
#include <aq/parser/SQLParser.h> // pour les tests
#include <aq/SQLPrefix.h> // pour les tests

namespace aq
{

  // INITIALIZATION PART
  littleThread::littleThread(aq::tnode* pNode)
    : _pNode(pNode), _end(false), _threading(false), _ready(false)
  {}

  bool  littleThread::findNodeIn(aq::tnode* pNode, aq::tnode* tNode)
  {
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

  void  littleThread::addListThreadCondition(littleThread* thread)
  {
    this->_listThread.push_back(thread);
  }

  // THREAD PART
  void  littleThread::solveThread(int count)
  {
    if (this->_threading == true)
      return;
    this->_threading = true;

    // blabla thread
    std::string newQuery;
    aq::generate_parent(this->_pNode, NULL);
    aq::syntax_tree_to_sql_form(this->_pNode, newQuery);

    std::cout << "Turn number: [" << count << "] & Select part is: [" << newQuery << "]" << std::endl;

    this->_threading = false;
    this->_end = true;
  }

  // CHECK PART
  bool  littleThread::threadOver() const
  {
    return this->_end;
  }

  bool  littleThread::isThreading() const
  {
    return this->_threading;
  }
  
  bool  littleThread::isReady()
  {
    this->_ready = true;
    for (std::vector<littleThread*>::iterator it = this->_listThread.begin(); it != this->_listThread.end(); ++it)
      if ((*it)->threadOver() == false || (*it)->isThreading() == true)
        this->_ready = false;
    return this->_ready;
  }

}