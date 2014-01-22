#pragma once

#include "parser/SQLParser.h"
#include "parser/JeqParser.h"
#include "parser/sql92_grm_tab.hpp"
#include "Settings.h"

#include <aq/Base.h>
#include <aq/ColumnItem.h>
#include <aq/ParsException.h>
#include <aq/SQLPrefix.h> //  a delete
#include <aq/AQLQuery.h>
#include <aq/Exceptions.h>

#include <vector>
#include <list>
#include <string>
#include <algorithm>

#include <boost/algorithm/string.hpp>

namespace aq {
namespace util {

  namespace {

  }



  /// \defgroup conditions_process Pre-process joins conditions
  /// \{

  /// \brief Add a condition in Where clause
  /// \param pCond
  /// \param pStart
  void addConditionsToWhere(aq::tnode* pCond, aq::tnode* pStart);

  /// \brief Add K_INNER/K_OUTER node a condition.
  /// \param pNode the input and output result tree
  /// \param leftTag K_INNER/K_OUTER on left
  /// \param rightTag K_INNER/K_OUTER on right
  ///
  /// K_INNER and K_OUTER node are needed by AQ_Engine. There AQ equivalent to sql [inner/left outer/right outer/full outer] join in from clause
  void addInnerOuterNodes(aq::tnode* pNode, aq::tnode::tag_t leftTag, aq::tnode::tag_t rightTag);

  /// \brief
  /// \param pNode
  /// \param tag
  /// \param tables
  void addInnerOuterNodes(aq::tnode* pNode, aq::tnode::tag_t tag, const std::vector<std::string>& tables);
  
  /// \brief
  /// \param pStart
  /// \param BaseDesc
  void moveFromJoinToWhere(aq::tnode* pStart, Base::Ptr BaseDesc);

  /// \}



  /// \defgroup select_process Pre-process of select clause
  /// \{

  /// \brief Add an alias on each select item
  /// \param pNode the list of item to process
  void addAlias(aq::tnode* pNode);

  /// \brief Replace * in select clause by all columns resulting of the from clause
  /// \param pNode the input and output resulting tree
  /// \param BaseDesc Database Description with current temporary table of the query beeing processed
  /// \param columnNames the resulting columnNames
  /// \param columnDisplayNames the name of the columns
  void solveSelectStar(aq::tnode* pNode, Base::Ptr BaseDesc, std::vector<std::string>& columnNames, std::vector<std::string>& columnDisplayNames);
  
  /// \brief Create trees of select
  /// \param pNode
  /// \param BaseDesc
  void solveIdentRequest(aq::tnode* pNode, Base::Ptr BaseDesc);

  /// \brief
  /// \param pNode
  /// \param tables
  /// \param BaseDesc
  void assignIdentRequest(aq::tnode* pNode, std::vector<aq::tnode*> tables, Base::Ptr BaseDesc);

  /// \brief Check if the COLUMN exist in the IDENT COLUMN and chose the IDENT
  /// \param colName
  /// \param tables
  /// \param BaseDesc
  std::string checkAndName(std::string colName, std::vector<aq::tnode*> tables, Base::Ptr BaseDesc);

  /// \brief
  /// \param colRef
  /// \param clean
  aq::tnode * assignSafe(aq::tnode* colRef, aq::tnode* clean);

  /// \brief Create a little tree of a period/column/ident
  /// \param column
  /// \param period
  aq::tnode * createPeriodColumn(std::string column, std::string period);

  /// \brief
  /// \param name
  /// \param table
  /// \param column
  bool assignFake(std::string& name, aq::tnode* table, aq::tnode* column);
  
  /// \brief
  /// \param pInterior
  /// \param pExterior
  void solveSelectStarExterior(aq::tnode* pInterior, aq::tnode* pExterior);

  /// \brief
  /// \param pIntSelectAs
  /// \param pInteriorSelect
  /// \param pExteriorSelect
  void changeTableNames(aq::tnode* pIntSelectAs, aq::tnode* pInteriorSelect, aq::tnode* pExteriorSelect);

  /// \brief
  /// \param pIntSelectAs
  /// \param pInteriorSelect
  /// \param pExteriorSelect
  /// \param keepAlias
  void changeColumnNames(aq::tnode* pIntSelectAs, aq::tnode* pInteriorSelect, aq::tnode* pExteriorSelect, bool keepAlias);

  /// \}


  
  /// \brief
  /// \param pNode
  aq::tnode * getJoin(aq::tnode* pNode);
  
  /// \brief
  /// \param pNode
  /// \param BaseDesc
  void solveOneTableInFrom(aq::tnode* pNode, Base::Ptr BaseDesc);

  /// \brief
  /// \param pNode
  /// \param columnNodes
  void getAllColumnNodes(aq::tnode*& pNode, std::vector<aq::tnode*>& columnNodes);
  
  /// \brief
  /// \param pNode
  /// \param columns
  void getColumnsList(aq::tnode* pNode, std::vector<aq::tnode*>& columns);

  /// \brief
  /// \param pNode
  /// \param tables
  void getTablesList(aq::tnode* pNode, std::list<std::string>& tables);

  /// \brief Search in a tree for the last node matching tag
  /// \param pNode
  /// \param pLastTag
  /// \param pCheckNode
  /// \param tag 
  aq::tnode* getLastTag(aq::tnode*& pNode, aq::tnode* pLastTag, aq::tnode* pCheckNode, aq::tnode::tag_t tag);

  /// \brief
  /// \param pNode
  /// \param parent
  void generateParent(aq::tnode* pNode, aq::tnode* parent = nullptr);

  /// \brief
  /// \param pNode
  /// \param columnTypes
  /// \param baseDesc
  void getColumnTypes(aq::tnode* pNode, std::vector<Column::Ptr>& columnTypes, Base::Ptr baseDesc);

  /// \brief
  /// \param pNode
  void cleanQuery(aq::tnode*& pNode);

  /// \brief
  /// \param pNode
  template <typename T> aq::ColumnItem<T> GetItem(const aq::tnode& pNode)
  {
    aq::ColumnItem<T> item;
    item.setValue(static_cast<T>(pNode.getData().val_int));
    return item;
  }

  //template <> aq::ColumnItem<char*> GetItem(const aq::tnode& n)
  //{
  //  aq::ColumnItem<char*> item;
  //  char * value = new char[strlen(n.getData().val_str) + 1];
  //  strcpy(value, n.getData().val_str);
  //  item.setValue(value);
  //  return item;
  //}

  /// \brief
  /// \param item
  template <typename T> aq::tnode * GetNode(const aq::ColumnItem<T>& item)
  {
    aq::tnode	* n = new aq::tnode(K_INTEGER);
    n->set_int_data(item.getValue());
    return n;
  }

  template <>
  inline aq::tnode * GetNode<double>(const aq::ColumnItem<double>& item)
  {
    aq::tnode * n = new aq::tnode(K_REAL);
    n->set_double_data(item.getValue());
    return n;
  }

  template <>
  inline aq::tnode * GetNode<char*>(const aq::ColumnItem<char*>& item)
  {
    aq::tnode * n = new aq::tnode(K_STRING);
    n->set_string_data(item.getValue());
    return n;
  }

  /// \brief
  /// \param table
  /// \todo to implement
  aq::tnode* GetTree(Table& table);

  /// \brief
  /// \param pNode
  /// \param columns
  void toNodeListToStdList(tnode* pNode, std::list<tnode*>& columns);

  /// \brief
  /// \param columns
  /// \param aggregateColumns
  void findAggregateFunction(const std::list<tnode *>& columns, std::list<tnode *>& aggregateColumns);

  /// \brief
  /// \param pNode
  void addEmptyGroupBy(tnode * pNode);

  /// \brief
  /// \param pNode
  /// \param aggregateColumns
  void addColumnsToGroupBy(tnode * pNode, const std::list<tnode *>& aggregateColumns);

  /// \brief
  /// \param pNode
  void removePartitionBy(tnode *& pNode);

  /// \brief
  /// \param pNode
  bool isColumnReference(const aq::tnode * pNode);
  
  /// \brief
  /// \param pNode
  void dateNodeToBigInt(tnode * pNode);
  
  /// \brief
  /// \param baseDesc
  /// \param settings
  /// \param pNode
  /// \deprecated
  void transformExpression(const aq::Base::Ptr baseDesc, const aq::Settings::Ptr settings, aq::tnode * pNode);

  /// \brief
  /// \param pNode
  /// \param columns
  void getAllColumns(aq::tnode* pNode, std::vector<aq::tnode*>& columns);
  
  /// \brief
  /// \param pNode
  /// \param name
  void extractName(aq::tnode* pNode, std::string& name);
  
  /// \brief get table and column name if n is a column reference
  /// \param[int] n the input column node
  /// \param[out] table output table name
  /// \param[out[ column output column name
  void getTableAndColumnName(const aq::tnode& n, std::string& table, std::string& column);

  /// \brief traverse subtree recursively and negate operators when needed
  ///
  /// \param pNode
  /// \param applyNot
  ///
  /// if there is a NOT to be applied and the current node is
  /// NOT: delete this node and make the child node this node
  ///      if no NOT applies, apply NOT to child node
  ///      if a NOT already applies, do not apply NOT to child node
  /// <, <=, >, >=, =, <>, BETWEEN, LIKE : change into the inverse version
  /// OR : change to AND, apply NOT on children
  /// AND: change to OR, apply NOT on children
  void processNot(aq::tnode*& pNode, bool applyNot);

  /// \brief 
  /// \param pNode
  /// \param stmt
  void tnodeToSelectStatement(const aq::tnode& pNode, aq::core::SelectStatement& stmt);

}
}
