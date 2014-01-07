#include "RowVerbProcess.h"

namespace aq
{
  
    RowVerbProcess::RowVerbProcess(aq::verb::VerbNode::Ptr _spTree, 
      std::vector<aq::verb::VerbNode::Ptr>& _selectVerbs) 
      : spTree(_spTree), verbs(_selectVerbs)
    {
      this->applyRowVisitor.reset(new aq::ApplyRowVisitor);
    }
    
    RowVerbProcess::RowVerbProcess(const RowVerbProcess& o) 
      : spTree(o.spTree)
    {
      this->applyRowVisitor.reset(new aq::ApplyRowVisitor);
      for (auto& verb : o.verbs) 
      {
        aq::verb::VerbNode::Ptr v = aq::verb::VerbFactory::GetInstance().getVerb(verb->getVerbType());
        v->cloneSubtree(verb);
        this->verbs.push_back(v);
      }
    }

    int RowVerbProcess::process(std::vector<Row>& rows)
    {
      this->applyRowVisitor->rows = &rows; // shoulb be done only the first time
      for (auto& verb : this->verbs)
      {
        verb->apply(this->applyRowVisitor.get());
      }
      return 0;
    }

}