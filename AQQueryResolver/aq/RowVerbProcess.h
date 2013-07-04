#ifndef __ROW_VERB_PROCESS_H__
#define __ROW_VERB_PROCESS_H__

#include "RowProcess_Intf.h"
#include "ApplyRowVisitor.h"
#include "verbs/VerbNode.h"

namespace aq
{

  class RowVerbProcess : public RowProcess_Intf
  {
  public:
    RowVerbProcess(aq::verb::VerbNode::Ptr _spTree, 
      std::vector<aq::verb::VerbNode::Ptr>& _selectVerbs);
    RowVerbProcess(const RowVerbProcess& o);
    int process(std::vector<Row>& rows);
    RowVerbProcess * clone()
    {
      return new RowVerbProcess(*this);
    }
  private:
    aq::verb::VerbNode::Ptr spTree;
    std::vector<aq::verb::VerbNode::Ptr> verbs;
    boost::shared_ptr<aq::ApplyRowVisitor> applyRowVisitor;
  };

}

#endif