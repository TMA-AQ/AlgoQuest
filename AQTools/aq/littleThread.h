#ifndef __LITTLETHREAD_H__
# define __LITTLETHREAD_H__

# include <vector>
# include <aq/TreeUtilities.h>

namespace aq
{

  class littleThread
  {
  public:
    littleThread(aq::tnode* pNode);

    bool  findNodeIn(aq::tnode* pNode, aq::tnode* tNode);
    void  addListThreadCondition(littleThread* thread);

    void  solveThread(int count);

    bool  threadOver() const;
    bool  isThreading() const;
    bool  isReady();

    aq::tnode*  _pNode;

  private:
    bool  _end;
    bool  _threading;
    bool  _ready;

    std::vector<littleThread*> _listThread;
  };

}

#endif // __LITTLETHREAD_H__