#ifndef __AQTHREAD_H__
# define __AQTHREAD_H__

# include <vector>
# include <aq/TreeUtilities.h>
# include <boost/thread/thread.hpp>

namespace aq
{

  class AQThread
  {
  public:
    AQThread(aq::tnode* pNode);

    bool  findNodeIn(aq::tnode* pNode, aq::tnode* tNode);
    void  addListThreadCondition(AQThread* thread);

    void  solveThread(int count);

    void  start(int count);
    void  join();

    bool  threadOver()    const;
    bool  isThreading()   const;
    bool  isInitialaze()  const;
    bool  isReady();

    aq::tnode*  _pNode;

  private:
    bool  _end;
    bool  _threading;
    bool  _ready;
    bool  _initialaze;

    boost::thread _thread;

    std::vector<AQThread*> _listThread;
  };

}

#endif // __AQTHREAD_H__