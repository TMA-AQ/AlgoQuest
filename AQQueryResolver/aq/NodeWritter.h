#ifndef __AQ_NODE_WRITTER_H__
#define __AQ_NODE_WRITTER_H__

#include "RowWritter_Intf.h"
#include "parser/SQLParser.h"

namespace aq
{

  class NodeWritter : public RowWritter_Intf
  {
  public:
    NodeWritter(aq::tnode& _result);
    virtual ~NodeWritter();
    
    int process(std::vector<Row>& rows);
    RowProcess_Intf * clone();

    const std::vector<Column::Ptr>& getColumns() const;
    void setColumn(std::vector<Column::Ptr> _columns);
    unsigned int getTotalCount() const;

  private:
    std::vector<Column::Ptr> columns;
    aq::tnode& result;
    aq::tnode * cur;
  };

}

#endif