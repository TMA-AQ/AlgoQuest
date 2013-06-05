#include "RowVerbProcess.h"

namespace aq
{
  
    RowVerbProcess::RowVerbProcess(aq::verb::VerbNode::Ptr _spTree, 
      boost::shared_ptr<aq::ApplyRowVisitor> _applyRowVisitor,
      std::vector<aq::verb::VerbNode::Ptr>& _selectVerbs) 
      : spTree(_spTree), applyRowVisitor(_applyRowVisitor), verbs(_selectVerbs)
    {
    }

    int RowVerbProcess::process(std::vector<Row>& rows)
    {
      // this->spTree->addResultOnChild(row);
      // this->applyRowVisitor->inc();
      // if (this->applyRowVisitor->full())
      // {
        // this->applyRowVisitor->clear();

      if (this->verbs.size() >= 0)
      {
        std::for_each(this->verbs.begin(), this->verbs.end(), [&] (aq::verb::VerbNode::Ptr verb) {
          verb->apply(this->applyRowVisitor.get());
        });
      }
      else
      {
        if (rows[0].flush)
          this->applyRowVisitor->rows[0].flush = rows[0].flush; // FIXME
        this->spTree->apply(this->applyRowVisitor.get());
      }

        // return 0;
      // }
      // return -1;
        return 0;
    }

}