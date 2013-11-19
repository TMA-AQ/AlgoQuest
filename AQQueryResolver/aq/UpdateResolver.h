#ifndef __AQ_UPDATE_RESOLVER_H__
#define __AQ_UPDATE_RESOLVER_H__

#include "Settings.h"
#include "Base.h"
#include "AQEngine_Intf.h"
#include "parser/SQLParser.h"
#include "RowWritter_Intf.h"

namespace aq
{

  class UpdateResolver : public aq::RowWritter_Intf
  {
  public:
    UpdateResolver(aq::tnode * _statement, aq::Settings & _settings, aq::AQEngine_Intf * _aqEngine, aq::Base & _base);
    void solve();

    int process(std::vector<Row>& rows);
    RowProcess_Intf * clone();
    const std::vector<aq::Column::Ptr>& getColumns() const;
    void setColumn(std::vector<aq::Column::Ptr> _columns);
    unsigned int getTotalCount() const;

    void addColumn(aq::Column::Ptr column);

  private:

    template <typename T>
    struct col_handler_t
    {
      aq::Column::Ptr                              column;
      aq::ColumnItem<T>                            item;
      boost::shared_ptr<aq::ColumnMapper_Intf<T> > mapper;
    };
    typedef boost::variant<col_handler_t<int32_t>, col_handler_t<int64_t>, col_handler_t<double>, col_handler_t<char*> > col_handler_type_t;
    typedef std::map<std::string, col_handler_type_t> columns_values_t;

    aq::tnode         * statement;
    aq::AQEngine_Intf * aqEngine;
    aq::Settings      & settings;
    aq::Base          & base;
    aq::Table::Ptr      table;
    columns_values_t    columns;
    unsigned int        totalCount;

    std::vector<aq::Column::Ptr> cols; // FIXME
  };

}

#endif