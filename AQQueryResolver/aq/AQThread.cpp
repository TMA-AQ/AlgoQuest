#include "AQThread.h"

#include <boost/timer.hpp> // pour les tests
#include <boost/version.hpp> // pour les tests
#if BOOST_VERSION < 105000 // pour les tests
#define TIME_UTC_ TIME_UTC
#endif // pour les tests

#include <aq/TreeUtilities.h> // pour les tests
#include <aq/parser/SQLParser.h> // pour les tests
#include <aq/SQLPrefix.h> // pour les tests

namespace aq
{

  // INITIALIZATION PART
  AQThread::AQThread(aq::tnode* pNode)
    : _pNode(pNode), _end(false), _threading(false), _ready(false), _initialaze(false)
  {}

  bool  AQThread::findNodeIn(aq::tnode* pNode, aq::tnode* tNode)
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

  void  AQThread::addListThreadCondition(AQThread* thread)
  {
    this->_listThread.push_back(thread);
  }

  // THREAD PART
  void  AQThread::start(int count)
  {
    this->_initialaze = true;
    this->_thread = boost::thread(&AQThread::solveThread, this, count);
  }

  void  AQThread::join()
  {
    this->_thread.join();
  }

  void  AQThread::solveThread(int count)
  {
    boost::xtime xt; // a virer, c'est pour les tests
    boost::xtime_get(&xt, boost::TIME_UTC_); // a virer, c'est pour les tests
    xt.sec += count * 2; // a virer, c'est pour les tests
    boost::thread::sleep(xt); // a virer, c'est pour les tests

    if (this->_threading == true)
      return;
    this->_threading = true;

    // blabla thread debut
    std::string newQuery;
    aq::generate_parent(this->_pNode, NULL);
    aq::syntax_tree_to_sql_form(this->_pNode, newQuery);
    std::cout << "Turn number: [" << count << "] & Select part is:" << std::endl << "[" << newQuery << "]" << std::endl;
    // blabla thread fin

    this->_threading = false;
    this->_end = true;
    this->_initialaze = false;
  }

  // CHECK PART
  bool  AQThread::threadOver()    const
  {
    return this->_end;
  }

  bool  AQThread::isThreading()   const
  {
    return this->_threading;
  }

  bool  AQThread::isInitialaze()  const
  {
    return this->_initialaze;
  }

  bool  AQThread::isReady()
  {
    this->_ready = true;
    for (std::vector<AQThread*>::iterator it = this->_listThread.begin(); it != this->_listThread.end(); ++it)
      if ((*it)->threadOver() == false || (*it)->isThreading() == true)
        this->_ready = false;
    return this->_ready;
  }

}