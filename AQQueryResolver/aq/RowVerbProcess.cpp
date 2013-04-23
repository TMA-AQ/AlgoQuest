#include "RowVerbProcess.h"

namespace aq
{
  
    RowVerbProcess::RowVerbProcess(VerbNode::Ptr _spTree) 
      : spTree(_spTree) 
    {
    }

    int RowVerbProcess::process(row_t& row)
    {
      this->spTree->addResult(row);
      return 0;
    }

}