#ifndef __AQTHREADREQUEST_H__
# define __AQTHREADREQUEST_H__

# include <vector>

# include "littleThread.h"
# include <aq/TreeUtilities.h>

namespace aq
{

  class AQThreadRequest
  {
  public:
    ~AQThreadRequest() {} // a faire -> delete _thread;

    void parseAndCutThread(aq::tnode* pNode);
    void addThreadToList(aq::tnode* pNode);
    void solveThread(); // const?

  private:
    std::vector<littleThread*> _thread;
  };

}

#endif // __AQTHREADREQUEST_H__