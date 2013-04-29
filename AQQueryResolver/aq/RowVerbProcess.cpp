#include "RowVerbProcess.h"

namespace aq
{
  
    RowVerbProcess::RowVerbProcess(VerbNode::Ptr _spTree) 
      : spTree(_spTree) 
    {
    }

    int RowVerbProcess::process(Row& row)
    {
      this->spTree->addResult(row);
      return 0;
    }

}