#include "TreeUtilities.h"
#include "ExprTransform.h"
#include "parser/sql92_grm_tab.hpp"
#include "parser/ID2Str.h"
#include "parser/JeqParser.h"
#include <cassert>
#include <algorithm>
#include <aq/DateConversion.h>
#include <aq/Exceptions.h>
#include <aq/Logger.h>
#include <aq/FileCloser.h>
#include <boost/bind.hpp>
#include <boost/scoped_array.hpp>
#include <boost/filesystem.hpp>
#include <boost/array.hpp>
#include <aq/FileMapper.h>

namespace aq {
namespace util {
  
//------------------------------------------------------------------------------
namespace helper {

  void markAsDeleted(aq::tnode* pNode)
  {	
    if( !pNode )
      return;
    pNode->tag = K_DELETED;
    markAsDeleted(pNode->left);
    markAsDeleted(pNode->right);
    markAsDeleted(pNode->next);
  }

}

//------------------------------------------------------------------------------
void getRowItemName(aq::tnode* pNode, std::string& name)
{
  if (pNode->tag == K_PERIOD)
  {
    //name = pNode->left->getData().val_str;
    //name += ".";
    name += pNode->right->getData().val_str;
  }
  else
  {
    if (pNode->getDataType() == aq::tnode::tnodeDataType::NODE_DATA_STRING)
    {
      name = pNode->getData().val_str;
    }
    else
    {
      name = aq::id_to_string(pNode->getTag());
    }
  }
}

//------------------------------------------------------------------------------
void addConditionsToWhere( aq::tnode* pCond, aq::tnode* pStart )
{
	assert( pCond && pStart );
	//eliminate K_JNO from conditions
	aq::tnode* pCondClone = pCond->clone_subtree();
	std::vector<aq::tnode*> nodes;
	pCondClone->treeListToNodeArray(nodes, K_AND);
	std::vector<aq::tnode*> newNodes;
	for( size_t idx = 0; idx < nodes.size(); ++idx )
		if( nodes[idx] && (nodes[idx]->tag != K_JNO) )
			newNodes.push_back( nodes[idx] );
		else
			delete nodes[idx] ;
	if( newNodes.size() == 0 )
		return;
	pCondClone = aq::tnode::nodeArrayToTreeList( newNodes, K_AND );

	aq::tnode* pWhere = pStart->find_main(K_WHERE);
	if( !pWhere )
	{
		//create node
		aq::tnode* fromNode = pStart->find_main(K_FROM);
		assert( fromNode );
		pWhere = new aq::tnode( K_WHERE );
		pWhere->next = fromNode->next;
		fromNode->next = pWhere;
		pWhere->left = pCondClone;
	}
	else
	{
		//create AND node and add it to exterior tree
		//because we will need it now
		assert( pWhere->left );
		aq::tnode* pLastAnd, *pAux;
		if( pWhere->left->tag == K_AND )
		{
			pLastAnd = pWhere;
			while( pLastAnd->left && pLastAnd->left->tag == K_AND )
				pLastAnd = pLastAnd->left;
			assert( pLastAnd->tag == K_AND );
			pAux = pLastAnd->left;
			pLastAnd->left = new aq::tnode( K_AND );
			pLastAnd = pLastAnd->left;
		}
		else
		{
			pAux = pWhere->left;
			pWhere->left = new aq::tnode( K_AND );
			pLastAnd = pWhere->left;
		}
		if( pCond->tag == K_AND )
		{
			pLastAnd->left = pCondClone;
			pLastAnd->right = pAux;
		}
		else
		{
			pLastAnd->right = pCondClone;
			pLastAnd->left = pAux;
		}
	}
}

//------------------------------------------------------------------------------
void addInnerOuterNodes( aq::tnode* pNode, aq::tnode::tag_t leftTag, aq::tnode::tag_t rightTag )
{
	if( !pNode || !pNode->left )
		return;
	if( pNode->left->tag == K_INNER || pNode->left->tag == K_OUTER )
		return;
	//K_IN_VALUES can have a very deep sub-tree, stack overflow danger
	//the subtree does not contain any conditions that require K_INNER/K_OUTER
	if( pNode->tag == K_IN_VALUES )
		return;
	bool join = false;
	for( unsigned int idx = 0; idx < aq::parser::nrJoinTypes; ++idx )
		if( pNode->tag == aq::parser::joinTypes[idx] )
			join = true;
	if( join )
	{
		aq::tnode* pAux = pNode->left;
		pNode->left = new aq::tnode( leftTag );
		pNode->left->left = pAux;

		pAux = pNode->right;
		pNode->right = new aq::tnode( rightTag );
		pNode->right->left = pAux;
	}
	addInnerOuterNodes( pNode->left, leftTag, rightTag );
	addInnerOuterNodes( pNode->right, leftTag, rightTag );
	addInnerOuterNodes( pNode->next, leftTag, rightTag );
}

//------------------------------------------------------------------------------
void addInnerOuterNodes( aq::tnode* pNode, aq::tnode::tag_t tag, const std::vector<std::string>& tables )
{
	if( !pNode || !pNode->left )
		return;
	//K_IN_VALUES can have a very deep sub-tree, stack overflow danger
	//the subtree does not contain any conditions that require K_INNER/K_OUTER
	if( pNode->tag == K_IN_VALUES )
		return;
	bool join = false;
	for( unsigned int idx = 0; (idx < aq::parser::nrJoinTypes) && !join; ++idx )
		if( pNode->tag == aq::parser::joinTypes[idx] )
			join = true;
	if( join )
	{
    auto n = pNode->left;
    if ((n->tag != K_INNER) && (n->tag != K_OUTER))
    {
      bool find = false;
      for (auto& t : tables)
      {
        if (strcmp(n->left->getData().val_str, t.c_str()) == 0)
          find = true;
      }
      if (find)
      {
        aq::tnode* pAux = pNode->left;
        pNode->left= new aq::tnode(tag);
        pNode->left->left = pAux;
      }
    }

    n = pNode->right;
    if ((n->tag != K_INNER) && (n->tag != K_OUTER))
    {
      bool find = false;
      for (auto& t : tables)
      {
        if (strcmp(n->left->getData().val_str, t.c_str()) == 0)
          find = true;
      }
      if (find)
      {
        aq::tnode* pAux = pNode->right;
        pNode->right = new aq::tnode(tag);
        pNode->right->left = pAux;
      }
    }
	}
	addInnerOuterNodes( pNode->left, tag, tables );
	addInnerOuterNodes( pNode->right, tag, tables );
	addInnerOuterNodes( pNode->next, tag, tables );
}

//------------------------------------------------------------------------------
void moveFromJoinToWhere( aq::tnode* pStart, Base::Ptr BaseDesc )
{	
	assert( pStart && pStart->tag == K_SELECT );

	aq::tnode* pNode = pStart->find_main(K_FROM);

	//
	// Get All the tables in inner clause
	aq::tnode* pInner = nullptr;
	do
	{
    pInner = pNode->find_deeper(K_INNER); // K_OUTER is not managed here
		if (pInner == nullptr)
      return;

		assert(pInner->left && (pInner->left->tag == K_JOIN));
		assert(pInner->left->next && (pInner->left->next->tag == K_ON));
		aq::tnode* tablesNodes = pInner->left;
		aq::tnode* condNodes = pInner->left->next->left;
		
		assert(tablesNodes->left && ((tablesNodes->left->tag == K_IDENT) || (tablesNodes->left->tag == K_COMMA)));
		assert(tablesNodes->right && ((tablesNodes->right->tag == K_IDENT) || (tablesNodes->right->tag == K_COMMA)));

		pInner->tag = K_COMMA;
		pInner->left = tablesNodes->left;
		pInner->right = tablesNodes->right;
		
    // std::swap(condNodes->left, condNodes->right);
		addInnerOuterNodes( condNodes, K_INNER, K_INNER );
		condNodes->inf = 1;

		aq::tnode* pWhere = pStart->find_main(K_WHERE);
		if( !pWhere )
		{
			pWhere = new aq::tnode( K_WHERE );
			pWhere->left = condNodes;
			pNode->next = pWhere;
		}
		else
		{
			aq::tnode* pAnd = new aq::tnode( K_AND );
			pAnd->left = condNodes;
			pAnd->right = pWhere->left;
			pWhere->left = pAnd;
			pWhere = pAnd;
		}
	} while (pInner != nullptr);
}

//------------------------------------------------------------------------------
void addAlias( aq::tnode* pNode )
{
  if (pNode == nullptr) 
    return;
  if (pNode->tag == K_COMMA)
  {
    assert(pNode->right != nullptr);
    if ((pNode->right->tag != K_AS) && (pNode->tag != K_STAR))
    {
      aq::tnode * as_node = new aq::tnode(K_AS);
      as_node->left = pNode->right;
      std::string name;
      getRowItemName(as_node->left, name);
      as_node->right = new aq::tnode(K_IDENT);
      as_node->right->set_string_data(name.c_str());
      pNode->right = as_node;
    }
    assert(pNode->left != nullptr);
    if ((pNode->left->tag != K_COMMA) && (pNode->left->tag != K_AS) && (pNode->tag != K_STAR))
    {
      aq::tnode * as_node = new aq::tnode(K_AS);
      as_node->left = pNode->left;
      std::string name;
      getRowItemName(as_node->left, name);
      as_node->right = new aq::tnode(K_IDENT);
      as_node->right->set_string_data(name.c_str());
      pNode->left = as_node;
    }
    else
    {
      addAlias(pNode->left);
    }
  }
  else
  {
    if ((pNode->tag != K_AS) && (pNode->tag != K_STAR))
    {
      aq::tnode * left_as_node = pNode->clone_subtree();
      pNode->tag = K_AS;
      aq::tnode::delete_subtree(pNode->left);
      aq::tnode::delete_subtree(pNode->right);
      pNode->left = left_as_node;
      std::string name;
      getRowItemName(pNode->left, name);
      pNode->right = new aq::tnode(K_IDENT);
      pNode->right->set_string_data(name.c_str());
    }
  }
}

//------------------------------------------------------------------------------
void solveSelectStar(aq::tnode* pNode, Base::Ptr BaseDesc, std::vector<std::string>& columnNames, std::vector<std::string>& columnDisplayNames )
{
  if ( !pNode || ( pNode->tag == K_SELECT && !pNode->left ) )
    throw aq::parsException( "pNode->left is empty in { void solveSelectStar }, this exception should be throw in { int SQLParse }" );
  if( pNode->left->tag != K_STAR )
    return;
  //get all columns from all tables and return
  //the other verbs will have the columns they need
  aq::tnode* fromNode = pNode->find_main(K_FROM);
  std::vector<aq::tnode*> tables, tables2;
  fromNode->left->treeListToNodeArray(tables, K_COMMA);
  for (auto& n : tables)
  {
    n->joinlistToNodeArray(tables2);
  }
  if (!tables2.empty())
  {
    tables.clear();
  }
  std::set<std::string> tables_names;
  for (auto& n : tables2)
  {
    std::string tname = syntax_tree_to_sql_form(n);
    boost::trim(tname);
    boost::to_upper(tname);
    if (tables_names.find(tname) == tables_names.end())
    {
      tables_names.insert(tname);
      tables.push_back(n);
    }
  }
  std::vector<aq::tnode*> colRefs;
  aq::tnode *column;
  aq::tnode *nextFrom;
  for( size_t idx = 0; idx < tables.size(); ++idx )
  {
    if ((column = tables[idx]->find_first(K_SELECT)) == nullptr || (nextFrom = column->find_main(K_FROM)) == nullptr)
      nextFrom = tables[idx];

    if ( !tables[idx] || ( tables[idx]->getTag() == K_SELECT && ( column = tables[idx]->find_first(K_AS)) == nullptr) || !(column = nextFrom->find_first(K_IDENT)))
      throw aq::parsException( "STAR failed, no AS found", pNode, true );
    
    aq::tnode *column2;
    aq::tnode *alias;
    aq::tnode* colRef;
    if ( nextFrom != tables[idx] && ( column2 = tables[idx]->find_first(K_SELECT)) != nullptr && column2->left )
    {
      std::vector<aq::tnode*> select;
      column2->left->treeListToNodeArray(select, K_COMMA);
      for ( size_t idx2 = 0; idx2 < select.size(); ++idx2 )
      {
        if ((column = select[idx2]->find_first(K_COLUMN)) == nullptr)
          throw aq::parsException( "pNode->left is empty in { void solveSelectStar }, this exception should be throw in { int SQLParse } -> ", tables[idx], true );
        if ( ( alias = tables[idx]->find_first(K_AS)) != nullptr && alias->right && alias->right->getTag() == K_IDENT )
          colRef = createPeriodColumn( column->getData().val_str, alias->right->getData().val_str );
        else
          throw aq::parsException( "No alias found for ", tables[idx], true );
        colRefs.push_back( colRef );
        columnNames.push_back( std::string( alias->right->getData().val_str ) + "." + std::string( column->getData().val_str ) );
        columnDisplayNames.push_back( std::string( alias->right->getData().val_str ) + "." + std::string( column->getData().val_str ) );
      }
    }
    else
    {
      std::vector<Column::Ptr>& columns = BaseDesc->getTable( column->getData().val_str )->Columns;
      for( size_t idx2 = 0; idx2 < columns.size(); ++idx2 )
      {
        if ( ( alias = tables[idx]->find_first(K_AS)) != nullptr &&
          alias->right && alias->right->getTag() == K_IDENT )
          colRef = createPeriodColumn( columns[idx2]->getName().c_str(), alias->right->getData().val_str );
        else
          colRef = createPeriodColumn( columns[idx2]->getName().c_str(), column->getData().val_str );
        colRefs.push_back( colRef );
        columnNames.push_back( std::string( column->getData().val_str ) + "." + columns[idx2]->getName() );
        columnDisplayNames.push_back( std::string( column->getData().val_str ) + "." + columns[idx2]->getOriginalName() );
      }
    }
  }
  pNode->left = assignSafe( aq::tnode::nodeArrayToTreeList( colRefs, K_COMMA ), pNode->left );
}

//------------------------------------------------------------------------------
void solveIdentRequest(aq::tnode* pNode, Base::Ptr BaseDesc)
{
  if ( !pNode || ( pNode->tag == K_SELECT && !pNode->left ) )
    throw aq::parsException( "SELECT EMPTY", pNode, true );
  generateParent(pNode);
  aq::tnode*  fromNode = pNode->find_main(K_FROM);
  if ( !fromNode )
    throw aq::parsException( "NO FROM FOUND", pNode, true );
  std::vector<aq::tnode*> tables;
  fromNode->left->treeListToNodeArray(tables, K_COMMA);
  std::reverse(tables.begin(), tables.end());
  aq::tnode*  found;
  for ( size_t idx = 0; idx < tables.size(); ++idx )
  {
    if ((found = tables[idx]->find_first(K_SELECT)))
      solveIdentRequest( found, BaseDesc );
  }
  bool fake = true;
  if ( pNode->left->tag == K_STAR )
  {
    fake = false;
    std::vector<std::string> dummy1;
    std::vector<std::string> dummy2;
    solveSelectStar( pNode, BaseDesc, dummy1, dummy2 );
  }

  aq::tnode * assign = nullptr;
  if (fake != false)
  {
    if ((assign = pNode->find_first(K_WHEN)))
      while (assign)
      {
        assignIdentRequest(assign, tables, BaseDesc);
        assign = assign->next;
      }
      assignIdentRequest(pNode, tables, BaseDesc);
  }
  if ((assign = pNode->find_main(K_WHERE)))
  {
    aq::tnode*  assign2;
    if ((assign2 = assign->find_first(K_SELECT)))
      solveIdentRequest(assign2, BaseDesc);
    else
      assignIdentRequest(assign, tables, BaseDesc);
  }
  if ((assign = pNode->find_main(K_GROUP)))
    assignIdentRequest( assign, tables, BaseDesc);
  if ((assign = pNode->find_main(K_ORDER)))
    assignIdentRequest(assign, tables, BaseDesc);
}

//------------------------------------------------------------------------------
void assignIdentRequest( aq::tnode* pNode, std::vector<aq::tnode*> tables, Base::Ptr BaseDesc )
{
  if ( !pNode )
    throw aq::parsException( "pNode is empty in { void assignIdentRequest }, this exception shouldn't be normal" );
  std::vector<aq::tnode*> leftTab;
  aq::tnode*  nNode = pNode->clone_subtree();
  generateParent( nNode, nullptr );
  if ( nNode->getTag() != K_WHEN )
  {
    // pNode->left->commaListToNodeArraySecond(leftTab);
    pNode->left->treeListToNodeArray(leftTab, K_COMMA);
    aq::tnode::delete_subtree(pNode->left);
    delete pNode->left;
  }
  else
  {
    // nNode->right->commaListToNodeArraySecond(leftTab);
    nNode->right->treeListToNodeArray(leftTab, K_COMMA);
    aq::tnode::delete_subtree(pNode->right);
    delete pNode->right;
  }
  if ( nNode->next )
    aq::tnode::delete_subtree(nNode->next);
  if ( nNode->getTag() != K_WHEN )
    aq::tnode::delete_subtree(nNode->right);
  else
    aq::tnode::delete_subtree(nNode->left);
  delete nNode;
  aq::tnode*  column;
  std::string ident;
  for ( size_t idx = 0; idx < leftTab.size(); ++idx )
  {
    while ((column = leftTab[idx]->aq::tnode::find_first(K_COLUMN, K_PERIOD)))
    {
      aq::tnode*  colRef;
      try
      {
        ident = checkAndName( column->getData().val_str, tables, BaseDesc );
      }
      catch ( const std::exception & )
      {
        colRef = aq::tnode::nodeArrayToTreeList( leftTab, K_COMMA );
        aq::tnode::delete_subtree(colRef);
        throw;
      }
      colRef = createPeriodColumn( column->getData().val_str, ident );
      if ( column->parent && column != leftTab[idx] )
      {
        column = column->parent;
        if ( column->left && column->left->getTag() == K_COLUMN )
          column->left = assignSafe( colRef, column->left );
        else
          column->right = assignSafe( colRef, column->right );
      }
      else
        leftTab[idx] = assignSafe( colRef, column );
      generateParent( leftTab[idx], nullptr );
    }
  }
  std::reverse( leftTab.begin(), leftTab.end() );
  if ( pNode->getTag() != K_WHEN )
    pNode->left = assignSafe( aq::tnode::nodeArrayToTreeList( leftTab, K_COMMA ), pNode->left );
  else
    pNode->right = assignSafe( aq::tnode::nodeArrayToTreeList( leftTab, K_COMMA ), pNode->right );
}

//------------------------------------------------------------------------------
aq::tnode * assignSafe( aq::tnode* colRef, aq::tnode* clean )
{
  aq::tnode::delete_subtree(clean);
  return colRef;
}

//------------------------------------------------------------------------------
std::string checkAndName( std::string colName, std::vector<aq::tnode*> tables, Base::Ptr BaseDesc )
{
  aq::tnode* column;
  bool fake = false;
  std::vector<std::string>  list;
  std::string name;
  boost::to_upper( colName );
  for ( size_t idx = 0; idx < tables.size(); ++idx )
  {
    // rajouter une méthode pour trouver le IDENT le plus profond si il y a un AS
    if ((column = tables[idx]->find_first(K_SELECT)))
    {
      if (column->find_first(colName))
        fake = assignFake( name, tables[idx], nullptr );
    }
    else
    {
      if ( !tables[idx] || !( column = tables[idx]->find_first(K_IDENT)))
        throw aq::parsException( "False FROM: There are no IDENT in this FROM: ", tables[idx] );
      if ( list.size() == 0 || std::find( list.begin(), list.end(), column->getData().val_str ) == list.end() )
      {
        try
        {
          list.push_back( column->getData().val_str );
          std::vector<Column::Ptr>& columns = BaseDesc->getTable( column->getData().val_str )->Columns;
          for( size_t idx2 = 0; idx2 < columns.size(); ++idx2 )
          {
            if ( columns[idx2]->getName().c_str() == colName )
            {
              if ( fake == true )
                throw aq::parsException( "Ambigue", list, colName );
              else
                fake = assignFake( name, tables[idx], column );
            }
          }
        }
        catch ( const std::exception & )
        {
          throw;
        }
      }
    }
  }
  try
  {
    if ( fake == false )
    {
      std::string error = "Existance: this COLUMN: " + colName + " doesn't exist";
      throw parsException( error.c_str() );
    }
  }
  catch ( const std::exception & )
  {
    throw;
  }
  return name;
}

//------------------------------------------------------------------------------
bool assignFake( std::string& name, aq::tnode* table, aq::tnode* column )
{
  if ( table->getTag() == K_AS && table->right && table->right->getTag() == K_IDENT )
    name = table->right->getData().val_str;
  else if ( column == nullptr )
    throw parsException( "SELECT WITHOUT ALIAS: ", table );
  else
    name = column->getData().val_str;
  return true;
}

//------------------------------------------------------------------------------
aq::tnode *createPeriodColumn( std::string column, std::string ident )
{
  aq::tnode *colRef = new aq::tnode( K_PERIOD );
	colRef->right = new aq::tnode( K_COLUMN );
	colRef->right->set_string_data( column.c_str() );
	colRef->left = new aq::tnode( K_IDENT );
  colRef->left->set_string_data( ident.c_str() );
  return colRef;
}

//------------------------------------------------------------------------------
void solveSelectStarExterior( aq::tnode* pInterior, aq::tnode* pExterior )
{
	assert( pInterior && pExterior && pExterior->left );
	if( pExterior->left->tag != K_STAR )
		return;

	aq::tnode::delete_subtree(pExterior->left);
	pExterior->left = pInterior->left->clone_subtree();
}

//------------------------------------------------------------------------------
void getAllColumnNodes( aq::tnode*& pNode, std::vector<aq::tnode*>& columnNodes )
{
	if( !pNode )
		return;
	if( pNode->tag == K_COLUMN || pNode->tag == K_PERIOD )
	{
		columnNodes.push_back(pNode);
		return;
	}
	getAllColumnNodes( pNode->left, columnNodes );
	getAllColumnNodes( pNode->right, columnNodes );
	getAllColumnNodes( pNode->next, columnNodes );
}

//------------------------------------------------------------------------------
aq::tnode* findColumn(  std::string columnName, std::vector<aq::tnode*>& interiorColumns, bool keepAlias)
{
	boost::to_upper(columnName);
	for( size_t idx = 0; idx < interiorColumns.size(); ++idx )
	{
		if( !interiorColumns[idx] )
			continue;
		std::string column2(interiorColumns[idx]->right->getData().val_str);
		boost::to_upper(column2);

		switch( interiorColumns[idx]->tag )
		{
		case K_PERIOD:
			assert( interiorColumns[idx]->right );
			assert( interiorColumns[idx]->right->tag == K_COLUMN );
			if( columnName == column2 )
				return interiorColumns[idx];
			break;
		case K_AS:
			assert( interiorColumns[idx]->right );
			assert( interiorColumns[idx]->right->tag == K_IDENT );
			if( columnName == column2 )
      {
				if( keepAlias )
					return interiorColumns[idx];
				else
					return interiorColumns[idx]->left;
      }
			break;
		default:
			throw generic_error(generic_error::INVALID_QUERY, "");
		}
	}
	return nullptr;
}

//------------------------------------------------------------------------------
void changeTableNames(	aq::tnode* pIntSelectAs, aq::tnode* pInteriorSelect, aq::tnode* pExteriorSelect )
{
	std::string tableName = pIntSelectAs->right->getData().val_str;

	aq::tnode* intFromNode = pInteriorSelect->find_main(K_FROM);
	std::vector<aq::tnode*> intTables;
	intFromNode->left->treeListToNodeArray(intTables, K_COMMA);

	aq::tnode* extFromNode = pExteriorSelect->find_main(K_FROM);
	std::vector<aq::tnode*> extTables;
	extFromNode->left->treeListToNodeArray(extTables, K_COMMA);
	
	std::vector<aq::tnode*> newExtTables;
	for( size_t idx = 0; idx < extTables.size(); ++idx )
		if( extTables[idx] != pIntSelectAs )
			newExtTables.push_back( extTables[idx] );
	for( size_t idx = 0; idx < intTables.size(); ++idx )
		newExtTables.push_back(intTables[idx]->clone_subtree());
	extFromNode->left = aq::tnode::nodeArrayToTreeList( newExtTables, K_COMMA );

	//debug13 - remember that comma nodes aren't deleted, possible memory leaks

}

//------------------------------------------------------------------------------
void changeColumnNames(	aq::tnode* pIntSelectAs, aq::tnode* pInteriorSelect, aq::tnode* pExteriorSelect, bool keepAlias )
{
	std::string tableName = pIntSelectAs->right->getData().val_str;

	std::vector<aq::tnode*> exteriorColumns;
	getAllColumnNodes( pExteriorSelect, exteriorColumns );
	std::vector<aq::tnode*> interiorColumns;
	getColumnsList( pInteriorSelect->left, interiorColumns );
	for( size_t idx = 0; idx < exteriorColumns.size(); ++idx )
	{
		if( !exteriorColumns[idx] || !exteriorColumns[idx] )
			continue;
		aq::tnode*& extCol = exteriorColumns[idx];
		switch( extCol->tag )
		{
		case K_PERIOD:
			{
				assert( extCol->left && extCol->left->tag == K_IDENT && 
					extCol->right->tag == K_COLUMN );
				if( tableName != std::string(extCol->left->getData().val_str) )
					continue;
				std::string columnName = extCol->right->getData().val_str;
				aq::tnode* replCol = findColumn( columnName, interiorColumns, keepAlias );
				if( replCol )
				{
					aq::tnode::delete_subtree(extCol);
					extCol = replCol->clone_subtree();
				}
			}
			break;
		case K_COLUMN:
			{
				std::string columnName = extCol->getData().val_str;
				aq::tnode* replCol = findColumn( columnName, interiorColumns, keepAlias );
				if( replCol )
				{
					aq::tnode::delete_subtree(extCol);
					extCol = replCol->aq::tnode::clone_subtree();
				}
			}
			break;
		default:
			assert( 0 );
		}
	}
}

//------------------------------------------------------------------------------
aq::tnode * getJoin(aq::tnode* pNode)
{

  //if (!((pNode->left != nullptr) && (pNode->left->left->tag == K_PERIOD) && (pNode->left->right->tag == K_PERIOD)))
  //{
  //  delete_subtree(pNode->left);
  //}

  if (pNode->tag == K_AND)
  {

    aq::tnode * left = getJoin(pNode->left);
    aq::tnode * right = getJoin(pNode->right);

    if ((left == nullptr) && (right == nullptr))
    {
      delete pNode;
      pNode = nullptr;
    }
    else if (left != nullptr)
    {
      delete pNode;
      pNode = left;
    }
    else if (right != nullptr)
    {
      delete pNode;
      pNode = right;
    }
  }
  else if (!((pNode->tag == K_JEQ) || (pNode->tag == K_JAUTO) || 
    (pNode->tag == K_JIEQ) || (pNode->tag == K_JSEQ))) // TODO : some join type are missing
  {
    delete pNode;
    pNode = nullptr;
  }

  return pNode;
}

//------------------------------------------------------------------------------
void solveOneTableInFrom( aq::tnode* pStart, Base::Ptr BaseDesc )
{
	assert( pStart && pStart->tag == K_SELECT );
	aq::tnode* pNode = pStart->find_main(K_FROM);

	std::vector<aq::tnode*> tables;
	pNode->left->treeListToNodeArray(tables, K_COMMA);

	if( tables.size() != 1 || !tables[0] || tables[0]->tag != K_IDENT )
		return;
	char* tName = tables[0]->getData().val_str;
	if( BaseDesc->getTable( tName )->Columns.size() == 0 )
		return;
	Column::Ptr col = BaseDesc->getTable( tName )->Columns[0];
	if( !col )
		return;
	
	aq::tnode* pWhere = pStart->find_main(K_WHERE);
	if( !pWhere )
	{
		pWhere = new aq::tnode( K_WHERE );
		pWhere->next = pNode->next;
		pNode->next = pWhere;
	}
	else
	{
		aq::tnode* pAnd = new aq::tnode( K_AND );
		pAnd->right = pWhere->left;
		pWhere->left = pAnd;
		pWhere = pAnd;
	}
	
	aq::tnode* newNode = new aq::tnode( K_JNO );
	pWhere->left = newNode;
	newNode->left = new aq::tnode( K_INNER );
	newNode = newNode->left;
	newNode->left = new aq::tnode( K_PERIOD );
	newNode->left->left = new aq::tnode( K_IDENT );
	newNode->left->left->set_string_data( tName );
	newNode->left->right = new aq::tnode( K_COLUMN );
	newNode->left->right->set_string_data( col->getName().c_str() );
}

//------------------------------------------------------------------------------
void getColumnsList( aq::tnode* pNode,std::vector<aq::tnode*>& columns )
{
	if( (pNode->tag == K_PERIOD) || (pNode->inf == 1) )
	{
		columns.push_back( pNode );
	} 
  else if( pNode->tag == K_COMMA )
	{
		getColumnsList( pNode->left, columns );
		getColumnsList( pNode->right, columns );
	} 
  else
	{
		// 		pNode->tag = K_DELETED;
		// 		columns.push_back( nullptr );
		columns.push_back( pNode );
	}
}

//------------------------------------------------------------------------------
void getTablesList( aq::tnode* pNode, std::list<std::string>& tables )
{
  if (pNode != nullptr)
  {
    if (pNode->tag == K_IDENT)
    {
      tables.push_back( pNode->getData().val_str );
    } 
    else // if (pNode->tag == K_COMMA)
    {
      getTablesList( pNode->left, tables );
      getTablesList( pNode->right, tables );
    }
  }
}

//------------------------------------------------------------------------------
aq::tnode* getLastTag( aq::tnode*& pNode, aq::tnode* pLastTag, aq::tnode* pCheckNode, aq::tnode::tag_t tag )
{
	if( !pNode )
		return nullptr;
	if( pNode == pCheckNode )
		return pLastTag;
	aq::tnode* pNewLastTag = pLastTag;
	if( pNode->tag == tag )
		pNewLastTag = pNode;
	aq::tnode* res = getLastTag( pNode->left, pNewLastTag, pCheckNode, tag );
	if( res )
		return res;
	res = getLastTag( pNode->right, pNewLastTag, pCheckNode, tag );
	if( res )
		return res;
	res = getLastTag( pNode->next, pNewLastTag, pCheckNode, tag );
	return res;
}

//------------------------------------------------------------------------------
void generateParent(aq::tnode* pNode, aq::tnode* parent)
{
  pNode->parent = parent;
  if (pNode->right) generateParent(pNode->right, pNode);
  if (pNode->left) generateParent(pNode->left, pNode);
  if (pNode->next) generateParent(pNode->next, nullptr);
}

//------------------------------------------------------------------------------
void cleanQuery( aq::tnode*& pNode )
{
	if( !pNode )
		return;
  
	if( pNode->tag == K_DELETED )
	{
    //if (pNode->parent->left == pNode)
    //{
    //  pNode->parent->left = nullptr;
    //}
    //else if (pNode->parent->right == pNode)
    //{
    //  pNode->parent->right = nullptr;
    //}
    //else if (pNode->parent->next == pNode)
    //{
    //  pNode->parent->next = nullptr;
    //}
		aq::tnode::delete_subtree(pNode);
		pNode = nullptr;
		return;
	}

	//K_IN_VALUES can have a very deep sub-tree, stack overflow danger
	if( pNode->tag == K_IN_VALUES )
		return;
  
	cleanQuery( pNode->next );
	cleanQuery( pNode->left );
	cleanQuery( pNode->right );

	if( pNode && (pNode->tag != K_COMMA) )
		return;

	if( pNode->left != nullptr && pNode->right != nullptr )
		return; //no null to delete
	if( pNode->left == nullptr && pNode->right == nullptr )
	{
		//delete entire node
    //if (pNode->parent->left == pNode)
    //{
    //  pNode->parent->left = nullptr;
    //}
    //else if (pNode->parent->right == pNode)
    //{
    //  pNode->parent->right = nullptr;
    //}
    //else if (pNode->parent->next == pNode)
    //{
    //  pNode->parent->next = nullptr;
    //}
		delete pNode ;
		pNode = nullptr;
		return;
	}
	//delete left null
	aq::tnode* newNode = pNode->left;
	if( pNode->right != nullptr )
		newNode = pNode->right; //or right null
	delete pNode ;
	pNode = newNode;
}

//------------------------------------------------------------------------------
void getColumnTypes( aq::tnode* pNode, std::vector<Column::Ptr>& columnTypes, Base::Ptr BaseDesc )
{
	if (!pNode || !pNode->left)
		return;

	while ((pNode->left->tag == K_PERIOD) || (pNode->left->tag == K_COMMA))
	{
		pNode = pNode->left;
		aq::tnode* colNode;
		if (pNode->tag == K_PERIOD)
    {
      colNode = pNode;
    }
    else
		{
			colNode = pNode->right;
			assert (colNode->tag == K_PERIOD);
		}
		assert(colNode);
		assert(colNode->left && (colNode->left->tag == K_IDENT));
		assert(colNode->right && (colNode->right->tag == K_COLUMN));
		bool found = false;
		const auto& table = BaseDesc->getTable(colNode->left->getData().val_str);
    const auto& column = table->getColumn(colNode->right->getData().val_str);
    Column::Ptr c(new Column(*column));
    c->setTableName(table->getName());
    columnTypes.push_back(c);
	}
	reverse(columnTypes.begin(), columnTypes.end());
}

//------------------------------------------------------------------------------
aq::tnode* GetTree( Table& table )
{
	// aq::tnode	*pNode = nullptr;
  aq::tnode	*pStart = nullptr;

//	if( table.Columns.size() < 1 )
//		return nullptr;
//	Column& column = *table.Columns[0];
//	if( column.Items.size() < 1 )
//		return nullptr;
//	if( column.Items.size() < 2 )
//		return Getnode(column.Items[0], column.Type);
//
//	//we have a list
//	pNode = new aq::tnode( K_COMMA );
//	pStart = pNode;
//	size_t size = column.Items.size();
//	for( size_t idx = 0; idx < size - 2; ++idx )
//	{
//		pNode->right = new aq::tnode( K_COMMA );
//		pNode->left = Getnode(column.Items[idx], column.Type);
//		pNode = pNode->right;
//	}
//	pNode->left = Getnode(column.Items[size - 2], column.Type);
//	pNode->right = Getnode(column.Items[size - 1], column.Type);
	
	return pStart;
}

//------------------------------------------------------------------------------
void toNodeListToStdList(tnode* pNode, std::list<tnode*>& columns)
{	
  if( !pNode || !pNode->left )
		return;
  if (pNode->left->tag == K_COMMA)
  {
    assert(pNode->left->right);
    columns.push_back(pNode->left->right);
    toNodeListToStdList(pNode->left, columns);
  }
  else
  {
    columns.push_back(pNode->left);
  }
}

//------------------------------------------------------------------------------
void findAggregateFunction(const std::list<tnode *>& columns, std::list<tnode *>& aggregateColumns)
{
  for (std::list<tnode *>::const_iterator it = columns.begin(); it != columns.end(); ++it)
  {
    if ((*it)->tag == K_AS)
    {
      tnode * n = (*it)->left;
      if ((n->tag == K_MAX) || (n->tag == K_MIN) || (n->tag == K_SUM) || (n->tag == K_AVG) || (n->tag == K_COUNT))
      {
        assert(n->left);
        if (std::find_if(aggregateColumns.begin(), aggregateColumns.end(), boost::bind(&tnode::cmp, _1, n->left)) == aggregateColumns.end())
        {
          aggregateColumns.push_back(n->left);
        }
      }
    }
  }
}

//------------------------------------------------------------------------------
void addEmptyGroupBy(tnode * pNode)
{
  tnode * from = pNode->find_main(K_FROM);
  tnode * where = pNode->find_main(K_WHERE);
  tnode * group = new aq::tnode(K_GROUP);
  tnode * n = where ? where : from;
  group->next = n->next;
  n->next = group;
}

//------------------------------------------------------------------------------
void addColumnsToGroupBy(tnode * pNode, const std::list<tnode *>& columns)
{
  tnode * from = pNode->find_main(K_FROM);
  tnode * where = pNode->find_main(K_WHERE);
  tnode * group = new aq::tnode(K_GROUP);

  tnode * n = where ? where : from;
  group->next = n->next;
  n->next = group;

  pNode = group;
  for (std::list<tnode *>::const_iterator it = columns.begin(); it != columns.end();)
  {
    tnode * n = *it; // aq::find_first_node(*it, K_PERIOD);
    ++it;
    if (it == columns.end())
    {
      pNode->left = n->clone_subtree();
    }
    else
    {
      pNode->left = new aq::tnode(K_COMMA);
      pNode->right = n->clone_subtree();
      pNode = pNode->left;
    }
  }

}

//------------------------------------------------------------------------------
void removePartitionByFromSelect(tnode *& pNode)
{
  if (pNode != nullptr)
  {
    if ((pNode->tag == K_ORDER) || (pNode->tag == K_PARTITION))
    {
      tnode * n = pNode->find_deeper(K_FRAME);
      n = n->clone_subtree();
      aq::tnode::delete_subtree(pNode);
      pNode = n;
    }
    else
    {
      removePartitionByFromSelect(pNode->left);
      removePartitionByFromSelect(pNode->right);
    }
  }
}

//------------------------------------------------------------------------------
void removePartitionBy(tnode *& pNode)
{
  if (pNode->tag == K_SELECT)
  {
    removePartitionByFromSelect(pNode->left);
  }
  else
  {
    aq::Logger::getInstance().log(AQ_WARNING, "cannot remove partition by on a no SELECT node");
  }
}

//------------------------------------------------------------------------------
bool isColumnReference(const aq::tnode *pNode) 
{
	if ( pNode == nullptr )
		return false;
	if ( pNode->tag != K_PERIOD )
		return false;
	if ( pNode->left == nullptr )
		return false;
	if ( pNode->right == nullptr )
		return false;
	if ( pNode->left->tag != K_IDENT )
		return false;
	if ( pNode->right->tag != K_COLUMN )
		return false;
	return true;
}

//------------------------------------------------------------------------------
void dateNodeToBigInt(tnode * pNode)
{
  if (pNode != nullptr)
  {
    if (pNode->tag == K_TO_DATE)
    {
      assert(pNode->left != nullptr);
      DateConversion dateConverter;
      long long value;
      if (pNode->right != nullptr)
      {
        assert(pNode->right->getDataType() == tnode::tnodeDataType::NODE_DATA_STRING);
        dateConverter.setInputFormat(pNode->right->getData().val_str);
      }
      value = dateConverter.dateToBigInt(pNode->left->getData().val_str);
      pNode->tag = K_INTEGER;
      pNode->set_int_data(value);
      aq::tnode::delete_subtree(pNode->left);
      aq::tnode::delete_subtree(pNode->right);
      aq::tnode::delete_subtree(pNode->next);
    }
    dateNodeToBigInt(pNode->left);
    dateNodeToBigInt(pNode->right);
    dateNodeToBigInt(pNode->next);
  }
}

//------------------------------------------------------------------------------
void transformExpression(const aq::Base::Ptr baseDesc, const aq::Settings::Ptr settings, aq::tnode * tree)
{
  aq::tnode * whereNode = tree->find_main(K_WHERE);
  if (whereNode)
  {
    unsigned int tags[] = { K_EQ, K_NEQ, K_LT, K_LEQ, K_GT, K_GEQ, K_BETWEEN, K_NOT_BETWEEN, K_LIKE, K_NOT_LIKE };
    for (auto& tag : tags)
    {
      aq::tnode * cmpNode = nullptr;
      while ((cmpNode = whereNode->find_first(tag)) != nullptr)
      {
        aq::expression_transform::transform<FileMapper>(baseDesc, settings->dataPath, settings->packSize, cmpNode);
      }
    }
  }
}

//------------------------------------------------------------------------------
void getAllColumns(aq::tnode* pNode, std::vector<aq::tnode*>& columns)
{
	if( !pNode || ((pNode->inf == 1) && (pNode->tag != K_COMMA)) || (pNode->tag == K_JNO) )
		return;
	if( pNode->tag == K_PERIOD )
	{
		//only if the column name is unique
		bool found = false;
		for( size_t idx = 0; idx < columns.size(); ++idx )
		{
			if( !columns[idx] )
				continue;

			std::string table1(columns[idx]->left->getData().val_str);
			boost::to_upper(table1);
			std::string table2(pNode->left->getData().val_str);
			boost::to_upper(table2);
			std::string col1(columns[idx]->right->getData().val_str);
			boost::to_upper(col1);
			std::string col2(pNode->right->getData().val_str);
			boost::to_upper(col2);

			if( columns[idx]->tag == K_PERIOD &&
				table1 == table2 && col1 == col2 )
			{
				found = true;
				break;
			}
		}
		if( !found )
		{
			columns.push_back(pNode->clone_subtree());
			pNode = nullptr;
		}
		return;
	}
	getAllColumns( pNode->left, columns );
	getAllColumns( pNode->right, columns );
	getAllColumns( pNode->next, columns );
}

//------------------------------------------------------------------------------
void extractName( aq::tnode* pNode, std::string& name )
{
	if( !pNode )
		return;
	if( pNode->tag == K_AS )
	{
		name += pNode->right->getData().val_str;
	}
	else if( pNode->tag == K_PERIOD )
	{
		if( name != "" )
			name += " ";
		name += pNode->left->getData().val_str;
		name += ".";
		name += pNode->right->getData().val_str;
	}
	else if( pNode->tag == K_COLUMN )
	{
		name += pNode->getData().val_str;
	}
	else
	{
		std::string idstr = std::string( id_to_string( pNode->tag ) );
		if( idstr != "" )
		{
			if( name != "" )
				name += " ";
			name += idstr;
		}
		extractName( pNode->left, name );
		extractName( pNode->right, name );
	}
}

//------------------------------------------------------------------------------
void getTableAndColumnName(const aq::tnode& n, std::string& table, std::string& column) 
{
	if (n.tag == K_COLUMN)
	{
		column = n.getData().val_str;
	}
	else if ((n.tag == K_PERIOD) &&
					 ((n.left != nullptr) && (n.right != nullptr)) &&
					 (n.left->tag == K_IDENT) &&
					 ((n.right->tag == K_IDENT) || (n.right->tag == K_COLUMN)))
	{
		table = n.left->getData().val_str;
		column = n.right->getData().val_str;
	}
}

//------------------------------------------------------------------------------
void processNot( aq::tnode*& pNode, bool applyNot )
{
	if( !pNode )
		return;
	switch( pNode->tag )
	{
	case K_NOT:
		{
			aq::tnode* auxNode = pNode;
			pNode = pNode->left;
			auxNode->left = nullptr;
			delete auxNode ;
			processNot( pNode, !applyNot );
		}
		break;
	case K_AND:
		{
			if( applyNot )
				pNode->tag = K_OR;
			processNot( pNode->left, applyNot );
			processNot( pNode->right, applyNot );
		}
		break;
	case K_OR:
		{
			if( applyNot )
				pNode->tag = K_AND;
			processNot( pNode->left, applyNot );
			processNot( pNode->right, applyNot );
		}
		break;
	case K_LT:
		if( applyNot )
			pNode->tag = K_GEQ;
		break;
	case K_LEQ:
		if( applyNot )
			pNode->tag = K_GT;
		break;
	case K_GT:
		if( applyNot )
			pNode->tag = K_LEQ;
		break;
	case K_GEQ:
		if( applyNot )
			pNode->tag = K_LT;
		break;
	case K_JSEQ:
		if( applyNot )
			pNode->tag = K_JINF;
		break;
	case K_JSUP:
		if( applyNot )
			pNode->tag = K_JIEQ;
		break;
	case K_JINF:
		if( applyNot )
			pNode->tag = K_JSEQ;
		break;
	case K_JIEQ:
		if( applyNot )
			pNode->tag = K_JSUP;
		break;
	case K_JEQ:
		if( applyNot )
			pNode->tag = K_JNEQ;
		break;
	case K_JAUTO:
		if( applyNot )
			pNode->tag = K_JAUTO;
		break;
	case K_JNEQ:
		if( applyNot )
			pNode->tag = K_JEQ;
		break;
	case K_EQ:
		if( applyNot )
			pNode->tag = K_NEQ;
		break;
	case K_NEQ:
		if( applyNot )
			pNode->tag = K_EQ;
		break;
	case K_BETWEEN:
		if( applyNot )
			pNode->tag = K_NOT_BETWEEN;
		break;
	case K_NOT_BETWEEN:
		if( applyNot )
			pNode->tag = K_BETWEEN;
		break;
	case K_LIKE:
		if( applyNot )
			pNode->tag = K_NOT_LIKE;
		break;
	case K_NOT_LIKE:
		if( applyNot )
			pNode->tag = K_LIKE;
		break;
	case K_IN:
		if( applyNot )
			pNode->tag = K_NOT_IN;
		break;
	case K_NOT_IN:
		if( applyNot )
			pNode->tag = K_IN;
		break;
	case K_JNO:
		break;
	case K_IS:
		if( !pNode->right )
			throw generic_error(generic_error::NOT_IMPLEMENTED, "");
		if( (pNode->right->tag == K_NOT) && pNode->right->left && (pNode->right->left->tag == K_NULL) )
		{
			aq::tnode* auxNode = pNode->right;
			pNode->right = pNode->right->left;
			delete auxNode ;
		}
		else if( pNode->right->tag == K_NULL )
		{
			aq::tnode* auxNode = new aq::tnode( K_NOT );
			auxNode->left = pNode->right;
			pNode->right = auxNode;
		}
		else throw generic_error(generic_error::NOT_IMPLEMENTED, "");
		break;
	default:
		throw generic_error(generic_error::NOT_IMPLEMENTED, "operator doesn't support NOT");
	}	
}

//------------------------------------------------------------------------------
void tnodeToSelectStatement(const aq::tnode& tree, aq::core::SelectStatement& ss)
{
  std::cout << tree << std::endl;

  const aq::tnode * selectNode = tree.find_main(K_SELECT);
  const aq::tnode * fromNode = tree.find_main(K_FROM);
  const aq::tnode * whereNode = tree.find_main(K_WHERE);
  const aq::tnode * groupNode = tree.find_main(K_GROUP);
  const aq::tnode * orderNode = tree.find_main(K_ORDER);

  if (selectNode != nullptr)
  {
    assert(selectNode->left);
    std::vector<tnode*> columns;
    aq::util::getAllColumns(selectNode->left, columns);
    std::string tname, cname;
    for (auto& c : columns)
    {
      aq::core::ColumnReference cr;
      cr.table.name = c->left->getData().val_str;
      cr.name = c->right->getData().val_str;
      boost::to_upper(cr.table.name);
      boost::to_upper(cr.name);
      ss.selectedTables.push_back(cr);
      aq::tnode::delete_subtree(c);
    }
  }

  if (fromNode != nullptr)
  {
    assert(fromNode->left);
    std::vector<tnode*> nodes;
    
    //
    fromNode->joinlistToNodeArray(nodes);
    for (const auto& table : nodes)
    {
      if (table->getTag() == K_IDENT)
      {
        assert(table->getDataType() == tnode::tnodeDataType::NODE_DATA_STRING);
        aq::core::TableReference tr;
        tr.name = table->getData().val_str;
        boost::to_upper(tr.name);
        ss.fromTables.push_back(tr);
      }
    }
    std::reverse(ss.fromTables.begin(), ss.fromTables.end());

    //
    nodes.clear();
    std::vector<const tnode*> from_nodes;
    fromNode->find_nodes(K_JOIN, from_nodes);
    for (const auto& join : nodes)
    {
      std::cout << *join << std::endl;
      aq::core::JoinCondition jc;
      
      aq::tnode * on = join->next;
      assert(on->getTag() == K_ON);
      assert(on->left != nullptr);

      jc.op = aq::id_to_kstring(on->left->getTag());
      
      aq::tnode * jtype = join->parent;
      assert(jtype != nullptr);
      if (jtype->getTag() == K_INNER)
      {
        jc.jt_left = jc.jt_right = "K_INNER";
      }
      else if (jtype->getTag() == K_OUTER)
      {
        aq::tnode * jway = jtype->parent;
        assert(jway != nullptr);
        switch (jway->getTag())
        {
        case K_LEFT:
          jc.jt_left = "K_OUTER";
          jc.jt_right = "K_INNER";
          break;
        case K_RIGHT:
          jc.jt_left = "K_INNER";
          jc.jt_right = "k_OUTER";
          break;
        case K_FULL:
          jc.jt_left = "K_OUTER";
          jc.jt_right = "k_OUTER";
          break;
        default:
          assert(false);
        }
      }
      else
      {
        assert(false);
      }

      aq::tnode * n = on->left;
      jc.left.table.name = n->left->left->getData().val_str;
      jc.left.name = n->left->right->getData().val_str;
      jc.right.table.name = n->right->left->getData().val_str;
      jc.right.name = n->right->right->getData().val_str;
      
      boost::to_upper(jc.left.table.name);
      boost::to_upper(jc.left.name);
      boost::to_upper(jc.right.table.name);
      boost::to_upper(jc.right.name);
      ss.joinConditions.push_back(jc);
    }

    //
    nodes.clear();
    fromNode->left->treeListToNodeArray(nodes, K_COMMA);
    for (const auto& table : nodes)
    {
      if (table->getTag() == K_IDENT)
      {
        assert(table->getDataType() == tnode::tnodeDataType::NODE_DATA_STRING);
        aq::core::TableReference tr;
        tr.name = table->getData().val_str;
        boost::to_upper(tr.name);
        ss.fromTables.push_back(tr);
      }
    }
    
  }

  if (whereNode != nullptr)
  {
    std::vector<aq::tnode*> conds;
    whereNode->left->treeListToNodeArray(conds, K_AND);
    for (const auto& cond : conds)
    {
      if (aq::tnode::isJoinTag(cond->getTag()))
      {
        aq::core::JoinCondition jc;
        jc.op = aq::id_to_kstring(cond->getTag());
        jc.jt_left = jc.jt_right = "K_INNER";
        jc.left.table.name = cond->left->left->getData().val_str;
        jc.left.name = cond->left->right->getData().val_str;
        jc.right.table.name = cond->right->left->getData().val_str;
        jc.right.name = cond->right->right->getData().val_str;
        boost::to_upper(jc.left.table.name);
        boost::to_upper(jc.left.name);
        boost::to_upper(jc.right.table.name);
        boost::to_upper(jc.right.name);
        ss.joinConditions.push_back(jc);
      }
      else
      {
        aq::core::InCondition ic;
        ic.column.table.name = cond->left->left->getData().val_str;
        ic.column.name = cond->left->right->getData().val_str;
        boost::to_upper(ic.column.table.name);
        boost::to_upper(ic.column.name);
        std::vector<tnode*> values;
        cond->right->treeListToNodeArray(values, K_COMMA);
        std::ostringstream value;
        for (const auto& v : values)
        {
          value.str("");
          switch (v->getDataType())
          {
          case tnode::tnodeDataType::NODE_DATA_INT:
            value << v->getData().val_int;
            break;
          case tnode::tnodeDataType::NODE_DATA_NUMBER:
            value << v->getData().val_number;
            break;
          case tnode::tnodeDataType::NODE_DATA_STRING:
            assert(v->getTag() == K_INTEGER);
            value << v->getData().val_str;
            break;
          }
          ic.values.push_back(value.str());
        }
        ss.inConditions.push_back(ic);
      }
    }
  }

  if (groupNode != nullptr)
  {
    std::vector<tnode*> columns;
    groupNode->left->treeListToNodeArray(columns, K_COMMA);
    for (const auto& v : columns)
    {
      aq::core::ColumnReference cr;
      cr.table.name = v->left->getData().val_str;
      cr.name = v->right->getData().val_str;
      boost::to_upper(cr.table.name);
      boost::to_upper(cr.name);
      ss.groupedColumns.push_back(cr);
    }
  }

  if (orderNode != nullptr)
  {
    std::vector<tnode*> columns;
    orderNode->left->treeListToNodeArray(columns, K_COMMA);
    for (const auto& v : columns)
    {
      aq::core::ColumnReference cr;
      cr.table.name = v->left->getData().val_str;
      cr.name = v->right->getData().val_str;
      boost::to_upper(cr.table.name);
      boost::to_upper(cr.name);
      ss.orderedColumns.push_back(cr);
    }
  }

}

}
}
