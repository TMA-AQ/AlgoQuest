#ifndef __AQTHREADREQUEST_H__
# define __AQTHREADREQUEST_H__

# include <vector>

# include <aq/AQThread.h>
# include <aq/TreeUtilities.h>

namespace aq
{

  class AQThreadRequest
  {
  public:
    ~AQThreadRequest();

    void parseAndCutThread(aq::tnode* pNode);
    void addThreadToList(aq::tnode* pNode);
    void solveThread() const;

  private:
    std::vector<AQThread*> _thread;
  };

}

#endif // __AQTHREADREQUEST_H__