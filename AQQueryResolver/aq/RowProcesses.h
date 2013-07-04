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
    RowProcesses(const RowProcesses& o) {
      for (auto it = o.processes.begin(); it != o.processes.end(); ++it)
      {
        this->processes.push_back(boost::shared_ptr<aq::RowProcess_Intf>((*it)->clone()));
      }
    }
    void addProcess(boost::shared_ptr<aq::RowProcess_Intf> _process) { this->processes.push_back(_process); }
    int process(std::vector<Row>& rows) 
    {
      int rc = 0;
      for (auto it = processes.begin(); it != processes.end(); ++it) 
      {
        rc = (*it)->process(rows);
        if (rc == -1)
          break;
      }
      return rc;
    }
    virtual RowProcesses * clone()
    {
      return new RowProcesses(*this);
    }
  private:
    std::list<boost::shared_ptr<aq::RowProcess_Intf> > processes;
  };

}

#endif