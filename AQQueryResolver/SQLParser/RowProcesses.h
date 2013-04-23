#ifndef __ROW_PROCESSES_H__
#define __ROW_PROCESSES_H__

#include "RowProcess_Intf.h"
#include <list>

namespace aq
{
  
  class RowProcesses : public RowProcess_Intf
  {
  public:
    RowProcesses() {}
    void addProcess(boost::shared_ptr<aq::RowProcess_Intf> _process) { this->processes.push_back(_process); }
    int process(row_t& row) 
    {
      std::for_each(processes.begin(), processes.end(), [&] (boost::shared_ptr<aq::RowProcess_Intf> process) {
        process->process(row);
      });
      return 0;
    }
  private:
    std::list<boost::shared_ptr<aq::RowProcess_Intf> > processes;
  };

}

#endif