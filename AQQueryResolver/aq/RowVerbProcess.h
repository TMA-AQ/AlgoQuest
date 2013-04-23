#ifndef __ROW_VERB_PROCESS_H__
#define __ROW_VERB_PROCESS_H__

#include "RowProcess_Intf.h"
#include "Verb.h"

namespace aq
{

  class RowVerbProcess : public RowProcess_Intf
  {
  public:
    RowVerbProcess(VerbNode::Ptr _spTree);
    int process(row_t& row);
  private:
    VerbNode::Ptr spTree;
  };

}

#endif