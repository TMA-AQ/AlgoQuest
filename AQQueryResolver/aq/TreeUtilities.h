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
 
void addAlias( aq::tnode* pNode );
void addConditionsToWhere( aq::tnode* pCond, aq::tnode* pStart );
void addInnerOuterNodes( aq::tnode* pNode, aq::tnode::tag_t leftTag, aq::tnode::tag_t rightTag );
void addInnerOuterNodes( aq::tnode* pNode, aq::tnode::tag_t tag, const std::vector<std::string>& tables );
void mark_as_deleted( aq::tnode* pNode );
void solveSelectStar(aq::tnode* pNode, 
                     Base& BaseDesc,
                     std::vector<std::string>& columnNames,
                     std::vector<std::string>& columnDisplayNames);

void  solveIdentRequest( aq::tnode* pNode, Base& BaseDesc ); ///< create trees of select
void  assignIdentRequest( aq::tnode* pNode, std::vector<aq::tnode*> tables, Base& BaseDesc ); //  used in solveBaseName
std::string checkAndName( std::string colName, std::vector<aq::tnode*> tables, Base& BaseDesc ); // check if the COLUMN exist in the IDENT COLUMN and chose the IDENT
aq::tnode*  assignSafe( aq::tnode* colRef, aq::tnode* clean ); // cut code
aq::tnode*  createPeriodColumn( std::string column, std::string period ); //  create a little tree of a period/column/ident
bool  assignFake( std::string& name, aq::tnode* table, aq::tnode* column );

void solveSelectStarExterior( aq::tnode* pInterior, aq::tnode* pExterior );
void solveOneTableInFrom( aq::tnode* pStart, Base& BaseDesc );
void moveFromJoinToWhere( aq::tnode* pStart, Base& BaseDesc );
void changeTableNames(	aq::tnode* pIntSelectAs, aq::tnode* pInteriorSelect, aq::tnode* pExteriorSelect );
void changeColumnNames(	aq::tnode* pIntSelectAs, aq::tnode* pInteriorSelect, aq::tnode* pExteriorSelect, bool keepAlias );
aq::tnode * getJoin(aq::tnode* pNode);
bool isMonoTable(aq::tnode* query, std::string& tableName);

void readTmpFile( const char* filePath, std::vector<llong>& vals );
void writeTmpFile(	const char* filePath, const std::vector<llong>& vals, size_t startIdx, size_t endIdx );

void getColumnsIds(	const Table& table, std::vector<aq::tnode*>& columns, std::vector<int>& valuePos );

void eliminateAliases( aq::tnode* pSelect );

void getAllColumnNodes( aq::tnode*& pNode, std::vector<aq::tnode*>& columnNodes );

void getColumnsList( aq::tnode* pNode, std::vector<aq::tnode*>& columns );

void getTablesList( aq::tnode* pNode, std::list<std::string>& tables );

/// search a subtree for a node and return the last node that had a certain tag
aq::tnode* getLastTag( aq::tnode*& pNode, aq::tnode* pLastTag, aq::tnode* pCheckNode, aq::tnode::tag_t tag );

void generate_parent(aq::tnode* pNode, aq::tnode* parent = nullptr);

void getColumnTypes( aq::tnode* pNode, std::vector<Column::Ptr>& columnTypes, Base& baseDesc );

void cleanQuery( aq::tnode*& pNode );

//------------------------------------------------------------------------------
template <typename T> aq::ColumnItem<T> GetItem(const aq::tnode& n)
{
  aq::ColumnItem<T> item;
  item.setValue(static_cast<T>(n.getData().val_int));
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

//------------------------------------------------------------------------------
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

aq::tnode* GetTree( Table& table );

void toNodeListToStdList(tnode* pNode, std::list<tnode*>& columns);

void findAggregateFunction(const std::list<tnode *>& columns, std::list<tnode *>& aggregateColumns);

void addEmptyGroupBy(tnode * pNode);

void addColumnsToGroupBy(tnode * pNode, const std::list<tnode *>& aggregateColumns);

void setOneColumnByTableOnSelect(tnode * pNode);

void removePartitionBy(tnode *& pNode);

/// Return 1 for true, 0 for false
int is_column_reference(const aq::tnode * pNode);

void dateNodeToBigInt(tnode * pNode);

void transformExpression(const aq::Base& baseDesc, const aq::Settings& settings, aq::tnode * tree);

void getAllColumns(aq::tnode* pNode, std::vector<aq::tnode*>& columns);

void extractName(aq::tnode* pNode, std::string& name);

/// \brief traverse subtree recursively and negate operators when needed
///
/// if there is a NOT to be applied and the current node is
/// NOT: delete this node and make the child node this node
///      if no NOT applies, apply NOT to child node
///      if a NOT already applies, do not apply NOT to child node
/// <, <=, >, >=, =, <>, BETWEEN, LIKE : change into the inverse version
/// OR : change to AND, apply NOT on children
/// AND: change to OR, apply NOT on children
void processNot(aq::tnode*& pNode, bool applyNot);

void tnodeToSelectStatement(aq::tnode& tree, aq::core::SelectStatement& ss);

}
}
