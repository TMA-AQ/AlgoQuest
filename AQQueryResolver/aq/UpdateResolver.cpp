#include "UpdateResolver.h"
#include "parser/sql92_grm_tab.hpp"
#include "TreeUtilities.h"
#include "QueryResolver.h"
#include "ColumnMapper.h"
#include <boost/make_shared.hpp>

namespace aq
{
  
UpdateResolver::UpdateResolver(aq::tnode * _statement, aq::Settings & _settings, aq::AQEngine_Intf * _aqEngine, aq::Base & _base)
  : statement(_statement), aqEngine(_aqEngine), settings(_settings), base(_base)
{

}

void UpdateResolver::solve()
{
  //
  // get table name to update
  if ((this->statement->left->tag != K_IDENT) && (this->statement->left->getDataType() != aq::tnode::tnodeDataType::NODE_DATA_STRING))
  {
    throw aq::generic_error(aq::generic_error::INVALID_QUERY, "invalid table name");
  }
  std::string tableName = this->statement->left->getData().val_str;
  this->table = this->base.getTable(tableName);

  //
  // get columns to update and there values
  aq::tnode * setNode = this->statement->next;
  if (setNode == NULL)
  {
    throw aq::generic_error(aq::generic_error::INVALID_QUERY, "missing SET statement in UPDATE query");
  }
  std::list<aq::tnode*> set_list;
  aq::toNodeListToStdList(setNode, set_list);
  for (auto& n : set_list)
  {
    if (n->tag != K_EQ)
    {
      throw aq::generic_error(aq::generic_error::INVALID_QUERY, "only = operator is allowed in SET clause");
    }
    if ((!n->left) || (n->left->tag != K_IDENT))
    {
      throw aq::generic_error(aq::generic_error::INVALID_QUERY, "left side of operator = in SET clause should be a unique column identifier");
    }
    if (!n->right)
    {
      throw aq::generic_error(aq::generic_error::INVALID_QUERY, "right side of operator = in SET clause should be a value");
    }
    std::string column = n->left->getData().val_str;
    col_handler_t ch;
    ch.column = this->table->getColumn(column);
    ch.item = aq::GetItem(*n->right);
    bool cache = false;
    ch.mapper = aq::build_column_mapper<aq::FileMapper>(ch.column->Type, settings.dataPath.c_str(), this->table->ID, ch.column->ID, ch.column->Size, settings.packSize, cache, aq::FileMapper::mode_t::WRITE);
    if (columns.find(column) != columns.end())
    {
      throw aq::generic_error(aq::generic_error::INVALID_QUERY, "column [%s] appears several times in SET clause", column.c_str());
    }
    columns.insert(std::make_pair(column, ch));
  }

  //
  // get rows to update
  std::vector<size_t> indexes;
  aq::tnode * whereNode = setNode->next;
  if (whereNode != NULL)
  {
    aq::tnode * select = new aq::tnode(K_SELECT);
    select->left = new aq::tnode(K_PERIOD);
    select->left->left = new aq::tnode(K_IDENT);
    select->left->right = new aq::tnode(K_COLUMN);
    select->left->left->set_string_data(tableName.c_str());
    select->left->right->set_string_data(columns.begin()->first.c_str());
    select->next = new aq::tnode(K_FROM);
    select->next->left = new aq::tnode(K_IDENT);
    select->next->left->set_string_data(tableName.c_str());
    select->next->next = whereNode;

    auto handler = boost::make_shared<UpdateResolver>(*this); // ugly and risky
    boost::shared_ptr<RowWritter_Intf> rowHandler(handler);

    unsigned int id_generator = 1;
    aq::QueryResolver solver(select, &settings, aqEngine, base, id_generator);
    solver.solve(rowHandler);
  }

}

int UpdateResolver::process(std::vector<Row>& rows)
{
  for (auto& row : rows)
  {
    for (auto& index : row.indexes)
    {
      if (index != 0)
      {
        aq::Logger::getInstance().log(AQ_DEBUG, "update %s[%u]", this->table->getName().c_str(), index);
        for (auto& cv : columns)
        {
          auto& col = cv.second.column;
          auto& item = cv.second.item;
          auto& mapper = cv.second.mapper;
          aq::Logger::getInstance().log(AQ_DEBUG, "%s = %s", col->getName().c_str(), item.toString(col->Type).c_str());
          mapper->setValue(index-1, item);
        }
      }
    }
  }
  return 0;
}

RowProcess_Intf * UpdateResolver::clone()
{
  return new UpdateResolver(*this);
}

//

const std::vector<aq::Column::Ptr>& UpdateResolver::getColumns() const
{
  return cols;
}

void UpdateResolver::setColumn(std::vector<aq::Column::Ptr> _columns)
{
  (void)_columns;
}

unsigned int UpdateResolver::getTotalCount() const
{
  return totalCount;
}

}