#include "AQThread.h"
#include <aq/QueryResolverSimulate.h>

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
  template <class T>
  AQThread<T>::AQThread(aq::tnode* pNode, T resolver)
    : _pNode(pNode), _queryResolver(resolver) _patern(NULL), _end(false), _threading(false), _ready(false), _initialaze(false)
  {}

  template <class T>
  bool  AQThread<T>::findNodeIn(aq::tnode* pNode, aq::tnode* tNode)
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

  template <class T>
  void  AQThread<T>::addListThreadCondition(AQThread<T>* thread)
  {
    this->_listThread.push_back(thread);
  }

  // THREAD PART
  template <class T>
  void  AQThread<T>::start(int count, int time)
  {
    this->_initialaze = true;
    this->_thread = boost::thread(&AQThread<T>::solveThread, this, count, time);
  }

  template <class T>
  void  AQThread<T>::join()
  {
    this->_thread.join();
  }

  template <class T>
  void  AQThread<T>::solveThread(int count, int time)
  {
    boost::xtime xt; // a virer, c'est pour les tests
    boost::xtime_get(&xt, boost::TIME_UTC_); // a virer, c'est pour les tests
    xt.sec += (time * 2); // a virer, c'est pour les tests
    boost::thread::sleep(xt); // a virer, c'est pour les tests

    if (this->_threading == true)
      return;
    this->_threading = true;

    // blabla thread debut
    std::string newQuery;
    aq::generate_parent(this->_pNode, NULL);
    aq::syntax_tree_to_sql_form(this->_pNode, newQuery);
    std::cout << "Turn number: [" << count << "] with timer at [" << (time * 2)
              << "] & Select part is:" << std::endl << "[" << newQuery << "]" << std::endl;
    // blabla thread fin

    this->_threading = false;
    this->_end = true;
    this->_initialaze = false;


    //launch the patern
    if (this->_patern != NULL)
      if (this->_patern->isReady() == true && this->_patern->threadOver() == false && this->_patern->isThreading() == false)
        this->_patern->solveThread(++count, rand() % 5);
  }

  // CHECK PART
  template <class T>
  bool  AQThread<T>::threadOver()    const
  {
    return this->_end;
  }

  template <class T>
  bool  AQThread<T>::isThreading()   const
  {
    return this->_threading;
  }

  template <class T>
  bool  AQThread<T>::isInitialaze()  const
  {
    return this->_initialaze;
  }

  template <class T>
  bool  AQThread<T>::isReady()
  {
    this->_ready = true;
    for (std::vector<AQThread<T>*>::iterator it = this->_listThread.begin(); it != this->_listThread.end(); ++it)
      if ((*it)->threadOver() == false || (*it)->isThreading() == true)
        this->_ready = false;
    return this->_ready;
  }

  // algo for determinate the patern

  template <class T>
  bool        AQThread<T>::existanceThread(AQThread<T>* aqthread) const
  {
    for (std::vector<AQThread<T>*>::const_iterator it = this->_listThread.begin(); it != this->_listThread.end(); ++it)
      if (aqthread == (*it))
        return true;

    for (std::vector<AQThread<T>*>::const_iterator it = this->_listThread.begin(); it != this->_listThread.end(); ++it)
      return (*it)->existanceThread(aqthread);

    return false;
  }

  template <class T>
  void        AQThread<T>::purgeOverList()
  {
    for (std::vector<AQThread<T>*>::iterator it = this->_listThread.begin(); it != this->_listThread.end(); ++it)
      if (this->existanceThread(*it) == true)
        it = this->_listThread.erase(it);
  }

  template <class T>
  void        AQThread<T>::assignPatern(AQThread<T>* patern)
  {
    this->setPatern(patern);
    for (std::vector<AQThread<T>*>::iterator it = this->_listThread.begin(); it != this->_listThread.end(); ++it)
      (*it)->assignPatern(this);
  }

  // getter / setter
  template <class T>
  aq::tnode*  AQThread<T>::getPNode()  const
  {
    return this->_pNode;
  }

  template <class T>
  void        AQThread<T>::setPatern(AQThread<T>* patern)
  {
    this->_patern = patern;
  }

  // dump
  template <class T>
  void        AQThread<T>::dump()      const
  {
    std::string newQuery;
    aq::generate_parent(this->_pNode, NULL);
    if (this->_patern != NULL)
    {
      aq::syntax_tree_to_sql_form(this->_patern->_pNode, newQuery);
      std::cout << "My father is [" << newQuery << "] and:" << std::endl;
    }
    newQuery = "";
    aq::syntax_tree_to_sql_form(this->_pNode, newQuery);
    std::cout << "I'm [" << newQuery << "]";
    if (this->_listThread.size() == 0)
      std::cout << std::endl;
    else
      std::cout << " and my son(s) is(are) : -->" << std::endl;
    for (std::vector<AQThread<T>*>::const_iterator it = this->_listThread.begin(); it != this->_listThread.end(); ++it)
      (*it)->dump();
    if (this->_listThread.size() > 0)
      std::cout << "<--" << std::endl;
  }

  void TemporaryFunction ()
  {
    aq::QueryResolverSimulate tmp;
    aq::AQThread<aq::QueryResolverSimulate> TempObj(NULL, tmp);
  }
}

