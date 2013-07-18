#include "TreeUtilities.h"
#include "ExprTransform.h"
#include "parser/sql92_grm_tab.hpp"
#include <cassert>
#include <algorithm>
#include <aq/WIN32FileMapper.h>
#include <aq/DateConversion.h>
#include <aq/Exceptions.h>
#include <aq/Logger.h>
#include <boost/bind.hpp>
#include <boost/scoped_array.hpp>
#include <boost/filesystem.hpp>
#include <boost/array.hpp>

//------------------------------------------------------------------------------
const int nrJoinTypes = 7;
const int joinTypes[] = { K_JEQ, K_JAUTO, K_JNEQ, K_JINF, K_JIEQ, K_JSUP, K_JSEQ };
const int inverseTypes[] = { K_JEQ, K_JAUTO, K_JNEQ, K_JSUP, K_JSEQ, K_JINF, K_JIEQ };

namespace aq
{
  
//------------------------------------------------------------------------------
void getRowItemName( aq::tnode* pNode, std::string& name)
{
  if (pNode->tag == K_PERIOD)
  {
    name = pNode->left->getData().val_str;
    name += ".";
    name += pNode->right->getData().val_str;
  }
  else
  {
    name = pNode->getData().val_str;
  }
}

//------------------------------------------------------------------------------
void addAlias( aq::tnode* pNode )
{
  if (pNode == NULL) 
    return;
  if (pNode->tag == K_COMMA)
  {
    assert(pNode->right != NULL);
    if (pNode->right->tag != K_AS)
    {
      aq::tnode * as_node = new aq::tnode(K_AS);
      as_node->left = pNode->right;
      std::string name;
      getRowItemName(as_node->left, name);
      as_node->right = new aq::tnode(K_IDENT);
      as_node->right->set_string_data(name.c_str());
      pNode->right = as_node;
    }
    assert(pNode->left != NULL);
    if ((pNode->left->tag != K_COMMA) && (pNode->left->tag != K_AS))
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
    if (pNode->tag != K_AS)
    {
      aq::tnode * left_as_node = aq::clone_subtree(pNode);
      pNode->tag = K_AS;
      delete_subtree(pNode->left);
      delete_subtree(pNode->right);
      pNode->left = left_as_node;
      std::string name;
      getRowItemName(pNode->left, name);
      pNode->right = new aq::tnode(K_IDENT);
      pNode->right->set_string_data(name.c_str());
    }
  }
}

//------------------------------------------------------------------------------
void addConditionsToWhere( aq::tnode* pCond, aq::tnode* pStart )
{
	assert( pCond && pStart );
	//eliminate K_JNO from conditions
	aq::tnode* pCondClone = aq::clone_subtree(pCond);
	std::vector<aq::tnode*> nodes;
	andListToNodeArray( pCondClone, nodes );
	std::vector<aq::tnode*> newNodes;
	for( size_t idx = 0; idx < nodes.size(); ++idx )
		if( nodes[idx] && (nodes[idx]->tag != K_JNO) )
			newNodes.push_back( nodes[idx] );
		else
			delete nodes[idx] ;
	if( newNodes.size() == 0 )
		return;
	pCondClone = nodeArrayToAndList( newNodes );

	aq::tnode* pWhere = find_main_node( pStart, K_WHERE );
	if( !pWhere )
	{
		//create node
		aq::tnode* fromNode = find_main_node( pStart, K_FROM );
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
void addInnerOuterNodes( aq::tnode* pNode, int leftTag, int rightTag )
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
	for( int idx = 0; idx < nrJoinTypes; ++idx )
		if( pNode->tag == joinTypes[idx] )
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
void mark_as_deleted( aq::tnode* pNode )
{
	if( !pNode )
		return;
	pNode->tag = K_DELETED;
	mark_as_deleted( pNode->left );
	mark_as_deleted( pNode->right );
	mark_as_deleted( pNode->next );
}

//------------------------------------------------------------------------------
void solveSelectStar(aq::tnode* pNode,
                     Base& BaseDesc,
                     std::vector<std::string>& columnNames,
                     std::vector<std::string>& columnDisplayNames )
{
  if ( !pNode || ( pNode->tag == K_SELECT && !pNode->left ) )
    throw aq::parsException( "pNode->left is empty in { void solveSelectStar }, this exception should be throw in { int SQLParse }" );
  if( pNode->left->tag != K_STAR )
    return;
  //get all columns from all tables and return
  //the other verbs will have the columns they need
  aq::tnode* fromNode = find_main_node( pNode, K_FROM );
  std::vector<aq::tnode*> tables;
  commaListToNodeArray( fromNode->left, tables );
  std::vector<aq::tnode*> colRefs;
  aq::tnode *column;
  aq::tnode *nextFrom;
  for( size_t idx = 0; idx < tables.size(); ++idx )
  {
    if ( ( column = find_first_node( tables[idx], K_SELECT ) ) == NULL
      || ( nextFrom = find_main_node( column, K_FROM ) ) == NULL )
      nextFrom = tables[idx];
    if ( !tables[idx] || ( tables[idx]->getTag() == K_SELECT && ( column = find_first_node( tables[idx], K_AS ) ) == NULL )
      || !( column = find_first_node( nextFrom, K_IDENT ) ) )
      throw aq::parsException( "STAR failed, no AS found", pNode, true );
    aq::tnode *column2;
    aq::tnode *alias;
    aq::tnode* colRef;
    if ( nextFrom != tables[idx] && ( column2 = find_first_node( tables[idx], K_SELECT ) ) != NULL && column2->left )
    {
      std::vector<aq::tnode*> select;
      commaListToNodeArray( column2->left, select );
      for ( size_t idx2 = 0; idx2 < select.size(); ++idx2 )
      {
        if ( ( column = find_first_node( select[idx2], K_COLUMN ) ) == NULL )
          throw aq::parsException( "pNode->left is empty in { void solveSelectStar }, this exception should be throw in { int SQLParse } -> ", tables[idx], true );
        if ( ( alias = find_first_node( tables[idx], K_AS ) ) != NULL &&
          alias->right && alias->right->getTag() == K_IDENT )
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
      std::vector<Column::Ptr>& columns = BaseDesc.getTable( column->getData().val_str )->Columns;
      for( size_t idx2 = 0; idx2 < columns.size(); ++idx2 )
      {
        if ( ( alias = find_first_node( tables[idx], K_AS ) ) != NULL &&
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
  pNode->left = assignSafe( nodeArrayToCommaList( colRefs ), pNode->left );
}

//------------------------------------------------------------------------------

void solveIdentRequest( aq::tnode* pNode, Base& BaseDesc )
{
  try
  {
    if ( !pNode || ( pNode->tag == K_SELECT && !pNode->left ) )
      throw aq::parsException( "SELECT EMPTY", pNode, true );
    generate_parent(pNode, NULL);
    aq::tnode*  fromNode = find_main_node( pNode, K_FROM );
    if ( !fromNode )
      throw aq::parsException( "NO FROM FOUND", pNode, true );
    std::vector<aq::tnode*> tables;
    commaListToNodeArray( fromNode->left, tables );
    std::reverse(tables.begin(), tables.end());
    aq::tnode*  found;
    for ( size_t idx = 0; idx < tables.size(); ++idx )
    {
      if ( ( found = find_first_node( tables[idx], K_SELECT ) ) )
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

    aq::tnode*  assign = NULL;
    if ( fake != false )
    {
      if ( ( assign = find_first_node( pNode, K_WHEN ) ) )
        while ( assign )
        {
          assignIdentRequest( assign, tables, BaseDesc );
          assign = assign->next;
        }
      assignIdentRequest( pNode, tables, BaseDesc );
    }
    if ( ( assign = find_main_node( pNode, K_WHERE ) ) )
    {
      aq::tnode*  assign2;
      if ( ( assign2 = find_first_node( assign, K_SELECT ) ) )
        solveIdentRequest( assign2, BaseDesc );
      else
        assignIdentRequest( assign, tables, BaseDesc );
    }
    if ( ( assign = find_main_node( pNode, K_GROUP ) ) )
      assignIdentRequest( assign, tables, BaseDesc );
    if ( ( assign = find_main_node( pNode, K_ORDER ) ) )
      assignIdentRequest( assign, tables, BaseDesc );
  }
  catch ( const std::exception & )
  {
    throw;
  }
}

void  assignIdentRequest( aq::tnode* pNode, std::vector<aq::tnode*> tables, Base& BaseDesc )
{
  if ( !pNode )
    throw aq::parsException( "pNode is empty in { void assignIdentRequest }, this exception shouldn't be normal" );
  std::vector<aq::tnode*> leftTab;
  aq::tnode*  nNode = aq::clone_subtree( pNode );
  generate_parent( nNode, NULL );
  if ( nNode->getTag() != K_WHEN )
    commaListToNodeArraySecond( nNode->left, leftTab );
  else
    commaListToNodeArraySecond( nNode->right, leftTab );
  if ( nNode->next )
    delete_subtree( nNode->next );
  if ( nNode->getTag() != K_WHEN )
    delete_subtree( nNode->right );
  else
    delete_subtree( nNode->left );
  delete nNode;
  aq::tnode*  column;
  std::string ident;
  for ( size_t idx = 0; idx < leftTab.size(); ++idx )
  {
    while ( ( column = find_first_node_diffTag( leftTab[idx], K_COLUMN, K_PERIOD ) ) )
    {
      aq::tnode*  colRef;
      try
      {
        ident = checkAndName( column->getData().val_str, tables, BaseDesc );
      }
      catch ( const std::exception & )
      {
        colRef = nodeArrayToCommaList( leftTab );
        delete_subtree( colRef );
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
      generate_parent( leftTab[idx], NULL );
    }
  }
  std::reverse( leftTab.begin(), leftTab.end() );
  if ( pNode->getTag() != K_WHEN )
    pNode->left = assignSafe( nodeArrayToCommaList( leftTab ), pNode->left );
  else
    pNode->right = assignSafe( nodeArrayToCommaList( leftTab ), pNode->right );
}

aq::tnode*  assignSafe( aq::tnode* colRef, aq::tnode* clean )
{
  delete_subtree( clean );
  return colRef;
}

std::string checkAndName( std::string colName, std::vector<aq::tnode*> tables, Base& BaseDesc )
{
  aq::tnode* column;
  bool fake = false;
  std::vector<std::string>  list;
  std::string name;
  boost::to_upper( colName );
  for ( size_t idx = 0; idx < tables.size(); ++idx )
  {
    // rajouter une méthode pour trouver le IDENT le plus profond si il y a un AS
    if ( ( column = find_first_node( tables[idx], K_SELECT ) ) )
    {
      if ( findOut_IDENT( column, colName ) )
        fake = assignFake( name, tables[idx], NULL );
    }
    else
    {
      if ( !tables[idx] || !( column = find_first_node( tables[idx], K_IDENT ) ) )
        throw aq::parsException( "False FROM: There are no IDENT in this FROM: ", tables[idx] );
      if ( list.size() == 0 || std::find( list.begin(), list.end(), column->getData().val_str ) == list.end() )
      {
        try
        {
          list.push_back( column->getData().val_str );
          std::vector<Column::Ptr>& columns = BaseDesc.getTable( column->getData().val_str )->Columns;
          for( size_t idx2 = 0; idx2 < columns.size(); ++idx2 )
            if ( columns[idx2]->getName().c_str() == colName )
              if ( fake == true )
                throw aq::parsException( "Ambigue", list, colName );
              else
                fake = assignFake( name, tables[idx], column );
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

bool  assignFake( std::string& name, aq::tnode* table, aq::tnode* column )
{
  if ( table->getTag() == K_AS && table->right && table->right->getTag() == K_IDENT )
    name = table->right->getData().val_str;
  else if ( column == NULL )
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

	delete_subtree( pExterior->left );
	pExterior->left = aq::clone_subtree(pInterior->left);
}

//------------------------------------------------------------------------------
void solveOneTableInFrom( aq::tnode* pStart, Base& BaseDesc )
{
	assert( pStart && pStart->tag == K_SELECT );
	aq::tnode* pNode = find_main_node( pStart, K_FROM );
	/*std::vector<aq::tnode*> tables;
	commaListToNodeArray( pNode->left, tables );
	for( size_t idx = 0; idx < tables.size(); ++idx )
		if( tables[idx] && tables[idx]->tag == K_AS )
		{
			replaceTableIdent( pStart, tables[idx]->right->data.val_str, tables[idx]->left->data.val_str );
			*tables[idx] = *tables[idx]->left; //no memory leaks
		}*/
	std::vector<aq::tnode*> tables;
	commaListToNodeArray( pNode->left, tables );
	/*aq::tnode* oldTables = pNode->left;
	std::vector<aq::tnode*> newTables;
	for( size_t idx = 0; idx < tables.size(); ++idx )
		if( tables[idx] && tables[idx]->tag == K_IDENT )
			newTables.push_back( new_node(tables[idx]) );
	pNode->left = nodeArrayToCommaList( newTables );
	delete_subtree( oldTables );
	if( tables.size() != 1 || !newTables[0] || newTables[0]->tag != K_IDENT )
		return false;
	char* tName = newTables[0]->data.val_str;*/
	if( tables.size() != 1 || !tables[0] || tables[0]->tag != K_IDENT )
		return;
	char* tName = tables[0]->getData().val_str;
	if( BaseDesc.getTable( tName )->Columns.size() == 0 )
		return;
	Column::Ptr col = BaseDesc.getTable( tName )->Columns[0];
	if( !col )
		return;
	
	aq::tnode* pWhere = find_main_node( pStart, K_WHERE );
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
void moveFromJoinToWhere( aq::tnode* pStart, Base& BaseDesc )
{	
	assert( pStart && pStart->tag == K_SELECT );

	aq::tnode* pNode = find_main_node( pStart, K_FROM );

	//
	// Get All the tables in inner clause
	aq::tnode* pInner = NULL;
	do
	{
		if ((pInner = find_deeper_node( pNode, K_INNER )) == NULL)
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
		
		addInnerOuterNodes( condNodes, K_INNER, K_INNER );
		condNodes->inf = 1;

		aq::tnode* pWhere = find_main_node( pStart, K_WHERE );
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
	} while (pInner != NULL);
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
aq::tnode* findColumn(  std::string columnName, std::vector<aq::tnode*>& interiorColumns,
					bool keepAlias)
{
	strtoupr( columnName );
	for( size_t idx = 0; idx < interiorColumns.size(); ++idx )
	{
		if( !interiorColumns[idx] )
			continue;
		std::string column2(interiorColumns[idx]->right->getData().val_str);
		strtoupr( column2 );

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
				if( keepAlias )
					return interiorColumns[idx];
				else
					return interiorColumns[idx]->left;
			break;
		default:
			throw generic_error(generic_error::INVALID_QUERY, "");
		}
	}
	return NULL;
}

//------------------------------------------------------------------------------
void changeTableNames(	aq::tnode* pIntSelectAs, aq::tnode* pInteriorSelect, aq::tnode* pExteriorSelect )
{
	std::string tableName = pIntSelectAs->right->getData().val_str;

	aq::tnode* intFromNode = find_main_node( pInteriorSelect, K_FROM );
	std::vector<aq::tnode*> intTables;
	commaListToNodeArray( intFromNode->left, intTables );

	aq::tnode* extFromNode = find_main_node( pExteriorSelect, K_FROM );
	std::vector<aq::tnode*> extTables;
	commaListToNodeArray( extFromNode->left, extTables );
	
	std::vector<aq::tnode*> newExtTables;
	for( size_t idx = 0; idx < extTables.size(); ++idx )
		if( extTables[idx] != pIntSelectAs )
			newExtTables.push_back( extTables[idx] );
	for( size_t idx = 0; idx < intTables.size(); ++idx )
		newExtTables.push_back( aq::clone_subtree(intTables[idx]) );
	extFromNode->left = nodeArrayToCommaList( newExtTables );

	//debug13 - remember that comma nodes aren't deleted, possible memory leaks

	/*
	//replace interior select definition from exterior select with K_DELETED node
	delete_node( (*pIntSelectAs)->right );
	(*pIntSelectAs)->right = NULL;
	(*pIntSelectAs)->tag = K_DELETED;
	aq::tnode* intSelectAs = (*pIntSelectAs)->next;
	(*pIntSelectAs)->left = NULL;*/

	/*
	//extract interior select definition from the exterior select
	aq::tnode** pComma = getLastTag( pExteriorSelect, NULL, *pIntSelectAs, K_COMMA );
	aq::tnode* intSelectAs = *pIntSelectAs;
	if( pComma )
	{
		assert( *pComma );
		aq::tnode* pAux = (*pComma)->left;
		if( pAux == *pIntSelectAs )
			pAux = (*pComma)->right;
		aq::tnode* pAux2 = *pComma;
		*pComma = pAux;
		delete_node( pAux2 );
	}*/
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
					delete_subtree( extCol );
					extCol = aq::clone_subtree(replCol);
				}
			}
			break;
		case K_COLUMN:
			{
				std::string columnName = extCol->getData().val_str;
				aq::tnode* replCol = findColumn( columnName, interiorColumns, keepAlias );
				if( replCol )
				{
					delete_subtree( extCol );
					extCol = aq::clone_subtree(replCol);
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

  //if (!((pNode->left != NULL) && (pNode->left->left->tag == K_PERIOD) && (pNode->left->right->tag == K_PERIOD)))
  //{
  //  delete_subtree(pNode->left);
  //}

  if (pNode->tag == K_AND)
  {

    aq::tnode * left = getJoin(pNode->left);
    aq::tnode * right = getJoin(pNode->right);

    if ((left == NULL) && (right == NULL))
    {
      delete pNode;
      pNode = NULL;
    }
    else if (left != NULL)
    {
      delete pNode;
      pNode = left;
    }
    else if (right != NULL)
    {
      delete pNode;
      pNode = right;
    }
  }
  else if (!((pNode->tag == K_JEQ) || (pNode->tag == K_JAUTO) || 
    (pNode->tag == K_JIEQ) || (pNode->tag == K_JSEQ))) // TODO : some join type are missing
  {
    delete pNode;
    pNode = NULL;
  }

  return pNode;
}

//------------------------------------------------------------------------------
bool isMonoTable(aq::tnode * query, std::string& tableName)
{
	//
	// Get From
	aq::tnode * fromNode = find_main_node(query, K_FROM);
	if (fromNode)
	{
		std::vector<aq::tnode*> tables;
		commaListToNodeArray( fromNode->left, tables );
		if (tables.size() == 1)
		{
			aq::tnode * tmp = *tables.begin();
			if ((tmp->tag == K_IDENT) && (tmp->getDataType() == aq::tnode::tnodeDataType::NODE_DATA_STRING))
			{
				tableName = tmp->getData().val_str;
				return true;
			}
		}
	}
	return false;
}

//------------------------------------------------------------------------------
SolveMinMaxGroupBy::SolveMinMaxGroupBy(): pGroupBy(NULL), _min(true)
{
}

//------------------------------------------------------------------------------
SolveMinMaxGroupBy::~SolveMinMaxGroupBy()
{
	delete_subtree( this->pGroupBy );
	for( size_t idx = 0; idx < this->columns.size(); ++idx )
		delete_subtree( this->columns[idx] );
}

//------------------------------------------------------------------------------
bool SolveMinMaxGroupBy::checkAndClear( aq::tnode* pSelect )
{
	std::vector<aq::tnode*> auxColumns;
	commaListToNodeArray( pSelect->left, auxColumns );
	int nrMinMaxCols = 0;
	for( size_t idx = 0; idx < auxColumns.size(); ++idx )
	{
		if( !auxColumns[idx] )
			return false;
		if( auxColumns[idx]->tag == K_PERIOD )
			continue;
		if( auxColumns[idx]->tag != K_AS )
			return false;
		if( auxColumns[idx]->left->tag == K_PERIOD )
			continue;
		if( !(auxColumns[idx]->left->tag == K_MIN || 
			auxColumns[idx]->left->tag == K_MAX) )
			return false;
		minMaxCol = idx;
		_min = auxColumns[idx]->left->tag == K_MIN;
		++nrMinMaxCols;
	}
	if( nrMinMaxCols != 1 )
		return false;

	aq::tnode* pGroup = pSelect;
	while( pGroup->next && pGroup->next->tag != K_GROUP )
		pGroup = pGroup->next;
	if( !pGroup->next )
		return false;

	aq::tnode* pFrom = find_main_node( pSelect, K_FROM );
	if( !pFrom )
		return false;
	std::vector<aq::tnode*> tables;
	commaListToNodeArray( pFrom->left, tables );
	
	if( tables.size() != 1 )
		return false; //no min+group by or max+group by + from 1 table

	this->tableName = tables[0]->getData().val_str;

	this->pGroupBy = pGroup->next;
	pGroup->next = pGroup->next->next;

	aq::tnode* pAux = auxColumns[minMaxCol]->left;
	auxColumns[minMaxCol]->left = auxColumns[minMaxCol]->left->left;
	delete pAux ;

	//make copies of the column nodes because they belong to the query
	//and the query might get modified before modifyTmpFiles is called
	for( size_t idx = 0; idx < auxColumns.size(); ++idx )
		this->columns.push_back( aq::clone_subtree(auxColumns[idx]) );

	return true;
}

//------------------------------------------------------------------------------
void readTmpFile( const char* filePath, std::vector<llong>& vals )
{
	FILE *pFIn = fopenUTF8( filePath, "rb" );
	FileCloser fileCloser( pFIn );
	if ( pFIn == NULL )
		throw generic_error( generic_error::COULD_NOT_OPEN_FILE, "" );
	llong val1;
	llong val2;
	int auxval;
	if( fread( &auxval, sizeof(int), 1, pFIn ) != 1 )
		throw generic_error( generic_error::INVALID_FILE, "" );

	while(	fread( &val1, sizeof(llong), 1, pFIn ) == 1 &&
			fread( &val2, sizeof(llong), 1, pFIn ) == 1 )
			vals.push_back( val2 );
}

//------------------------------------------------------------------------------
void writeTmpFile(	const char* filePath, const std::vector<llong>& vals,
					size_t startIdx, size_t endIdx )
{
	FILE *pFOut = fopen( filePath, "wb" );
	FileCloser fileCloser( pFOut );
	if ( pFOut == NULL )
		throw generic_error( generic_error::COULD_NOT_OPEN_FILE, "" );
	int intval = 0;
	fwrite( &intval, sizeof(int), 1, pFOut );
	llong auxval = 0;
	startIdx = std::max( startIdx, (size_t) 0 );
	endIdx = std::min( endIdx, vals.size() );
	for( size_t idx = startIdx; idx < endIdx; ++idx )
	{	
		fwrite( &auxval, sizeof(llong), 1, pFOut );
		fwrite( &vals[idx], sizeof(llong), 1, pFOut );
	}
}

//------------------------------------------------------------------------------
void readPosFile( const char* filePath, std::vector<llong>& vals )
{
	FILE *pFIn = fopen( filePath, "rb" );
	FileCloser fileCloser( pFIn );
	if ( pFIn == NULL )
		throw generic_error( generic_error::COULD_NOT_OPEN_FILE, "" );
	
	int val;
	while(	fread( &val, sizeof(int), 1, pFIn ) == 1 )
		vals.push_back( val );
}

//------------------------------------------------------------------------------
void getColumnsIds(	const Table& table, std::vector<aq::tnode*>& columns, std::vector<int>& valuePos )
{
	std::vector<std::string> columnsStr;
	for( size_t idx = 0; idx < columns.size(); ++idx )
	{
		if( columns[idx]->tag == K_PERIOD )
			columnsStr.push_back( std::string(columns[idx]->right->getData().val_str) );
		else if( columns[idx]->tag == K_AS )
			columnsStr.push_back( std::string(columns[idx]->left->right->getData().val_str) );
		else if( columns[idx]->tag == K_COLUMN )
			columnsStr.push_back( std::string(columns[idx]->getData().val_str) );
		else
			throw generic_error(generic_error::INVALID_QUERY, "");
		strtoupr(columnsStr[idx]);
	}
	for( size_t idx = 0; idx < columnsStr.size(); ++idx )
	{
		bool found = false;
		for( size_t idx2 = 0; idx2 < table.Columns.size(); ++idx2 )
			if( columnsStr[idx] == table.Columns[idx2]->getName() )
			{
				found = true;
				valuePos.push_back( (int) idx2 );
			}
		if( !found )
			throw generic_error(generic_error::INVALID_QUERY, "");
	}
}

//------------------------------------------------------------------------------
void eliminateAliases( aq::tnode* pSelect )
{
	assert( pSelect );
	std::vector<aq::tnode*> columns;
	commaListToNodeArray( pSelect->left, columns );
	std::vector<aq::tnode*> newColumns;
	for( int idx = 0; idx < columns.size(); ++idx )
		if( columns[idx]->tag == K_PERIOD )
			newColumns.push_back( aq::clone_subtree(columns[idx]) );
		else if( columns[idx]->tag == K_AS )
			newColumns.push_back( clone_subtree(columns[idx]->left) );
		else
			throw generic_error(generic_error::INVALID_QUERY, "");

	delete_subtree( pSelect->left );
	pSelect->left = nodeArrayToCommaList( newColumns );
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
		// 		columns.push_back( NULL );
		columns.push_back( pNode );
	}
}

//------------------------------------------------------------------------------
void getTablesList( aq::tnode* pNode, std::list<std::string>& tables )
{
  if (pNode != NULL)
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
aq::tnode* getLastTag( aq::tnode*& pNode, aq::tnode* pLastTag, aq::tnode* pCheckNode, int tag )
{
	if( !pNode )
		return NULL;
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
void generate_parent(aq::tnode* pNode, aq::tnode* parent)
{
  pNode->parent = parent;
  if (pNode->right) generate_parent(pNode->right, pNode);
  if (pNode->left) generate_parent(pNode->left, pNode);
  if (pNode->next) generate_parent(pNode->next, NULL);
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
    //  pNode->parent->left = NULL;
    //}
    //else if (pNode->parent->right == pNode)
    //{
    //  pNode->parent->right = NULL;
    //}
    //else if (pNode->parent->next == pNode)
    //{
    //  pNode->parent->next = NULL;
    //}
		delete_subtree( pNode );
		pNode = NULL;
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

	if( pNode->left != NULL && pNode->right != NULL )
		return; //no null to delete
	if( pNode->left == NULL && pNode->right == NULL )
	{
		//delete entire node
    //if (pNode->parent->left == pNode)
    //{
    //  pNode->parent->left = NULL;
    //}
    //else if (pNode->parent->right == pNode)
    //{
    //  pNode->parent->right = NULL;
    //}
    //else if (pNode->parent->next == pNode)
    //{
    //  pNode->parent->next = NULL;
    //}
		delete pNode ;
		pNode = NULL;
		return;
	}
	//delete left null
	aq::tnode* newNode = pNode->left;
	if( pNode->right != NULL )
		newNode = pNode->right; //or right null
	delete pNode ;
	pNode = newNode;
}

//------------------------------------------------------------------------------
void getColumnTypes( aq::tnode* pNode, std::vector<Column::Ptr>& columnTypes, Base& BaseDesc )
{
	if( !pNode || !pNode->left )
		return;
//	assert( pNode->left ); //debug13 not necessarily true, I should really start to handle this case
	while( pNode->left->tag == K_PERIOD || pNode->left->tag == K_COMMA )
	{
		pNode = pNode->left;
		aq::tnode* colNode;
		if( pNode->tag == K_PERIOD )
			colNode = pNode;
		else
		{
			colNode = pNode->right;
			assert( colNode->tag == K_PERIOD );
		}
		assert( colNode );
		assert( colNode->left && colNode->left->tag == K_IDENT );
		assert( colNode->right && colNode->right->tag == K_COLUMN );
		Table& table = *BaseDesc.getTable( colNode->left->getData().val_str );
		bool found = false;
		Column auxCol;
		auxCol.setName(colNode->right->getData().val_str);
		for(auto& c : table.Columns)
    {
			if (c->getName() == auxCol.getName())
			{
				Column::Ptr column = new Column(*c);
        column->setTableName(table.getName());
				columnTypes.push_back( column );
				found = true;
				break;
			}
    }
    assert(found);
    if (!found)
    {
      throw aq::generic_error(aq::generic_error::INVALID_QUERY, "");
    }
	}
	reverse(columnTypes.begin(), columnTypes.end());
}

//------------------------------------------------------------------------------
aq::tnode* Getnode( ColumnItem::Ptr item, ColumnType type )
{
	aq::tnode	*pNode = NULL;
	if( !item )
		return pNode;
	switch( type )
	{
	case COL_TYPE_INT:
	case COL_TYPE_DATE:
		pNode = new aq::tnode( K_INTEGER );
		pNode->set_int_data( (llong) item->numval );
		break;
	case COL_TYPE_DOUBLE:
		pNode = new aq::tnode( K_REAL );
		pNode->set_double_data( item->numval );
		break;
	default:
		pNode = new aq::tnode( K_STRING );
		pNode->set_string_data( item->strval.c_str() );
	}
	return pNode;
}

//------------------------------------------------------------------------------
aq::tnode* GetTree( Table& table )
{
	aq::tnode	*pNode = NULL;
	aq::tnode	*pStart = NULL;

	if( table.Columns.size() < 1 )
		return NULL;
	Column& column = *table.Columns[0];
	if( column.Items.size() < 1 )
		return NULL;
	if( column.Items.size() < 2 )
		return Getnode(column.Items[0], column.Type);

	//we have a list
	pNode = new aq::tnode( K_COMMA );
	pStart = pNode;
	size_t size = column.Items.size();
	for( size_t idx = 0; idx < size - 2; ++idx )
	{
		pNode->right = new aq::tnode( K_COMMA );
		pNode->left = Getnode(column.Items[idx], column.Type);
		pNode = pNode->right;
	}
	pNode->left = Getnode(column.Items[size - 2], column.Type);
	pNode->right = Getnode(column.Items[size - 1], column.Type);
	
	return pStart;
}

void selectToList(tnode* pNode, std::list<tnode*>& columns)
{	
  if( !pNode || !pNode->left )
		return;
  if (pNode->left->tag == K_COMMA)
  {
    assert(pNode->left->right);
    columns.push_back(pNode->left->right);
    selectToList(pNode->left, columns);
  }
  else
  {
    columns.push_back(pNode->left);
  }
}

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

void addEmptyGroupBy(tnode * pNode)
{
  tnode * from = find_main_node(pNode, K_FROM);
  tnode * where = find_main_node(pNode, K_WHERE);
  tnode * group = new aq::tnode(K_GROUP);
  tnode * n = where ? where : from;
  group->next = n->next;
  n->next = group;
}

void addColumnsToGroupBy(tnode * pNode, const std::list<tnode *>& columns)
{
  tnode * from = find_main_node(pNode, K_FROM);
  tnode * where = find_main_node(pNode, K_WHERE);
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
      pNode->left = aq::clone_subtree(n);
    }
    else
    {
      pNode->left = new aq::tnode(K_COMMA);
      pNode->right = aq::clone_subtree(n);
      pNode = pNode->left;
    }
  }

}

void setOneColumnByTableOnSelect(tnode * select)
{
  if (select->tag != K_SELECT)
  {
    aq::Logger::getInstance().log(AQ_ERROR, "cannot perform operation on select node");
    return;
  }


  // fill columns list
  std::vector<tnode*> columns;
  std::vector<tnode*> uniqueColumnsTable;
  getAllColumnNodes(select->left, columns);
  for (auto it1 = columns.begin(); it1 != columns.end(); ++it1) 
  {
    tnode *& n = *it1;
    bool b = false;
    for (auto it2 = uniqueColumnsTable.begin(); !b && (it2 != uniqueColumnsTable.end()); ++it2) 
    {
      if (strcmp(n->left->getData().val_str, (*it2)->left->getData().val_str) == 0)
      {
        b = true;
      }
    }
    if (!b)
      uniqueColumnsTable.push_back(aq::clone_subtree(n));
  }

  // replace select node
  aq::delete_subtree(select->left);
  tnode * n = select;
  for (auto it = uniqueColumnsTable.begin(); it != uniqueColumnsTable.end();)
  {
    tnode *& n1 = *it;
     ++it;
     if (it == uniqueColumnsTable.end())
     {
       n->left = n1;
     }
     else
     {
       n->right = n1;
       n->left = new aq::tnode(K_COMMA);
       n = n->left;
     }
  }

}

void removePartitionByFromSelect(tnode *& pNode)
{
  if (pNode != NULL)
  {
    if ((pNode->tag == K_ORDER) || (pNode->tag == K_PARTITION))
    {
      tnode * n = find_deeper_node(pNode, K_FRAME);
      n = aq::clone_subtree(n);
      aq::delete_subtree(pNode);
      pNode = n;
    }
    else
    {
      removePartitionByFromSelect(pNode->left);
      removePartitionByFromSelect(pNode->right);
    }
  }
}

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
// Return 1 for true, 0 for false//
int is_column_reference(const aq::tnode *pNode) 
{
	if ( pNode == NULL )
		return 0;
	if ( pNode->tag != K_PERIOD )
		return 0;
	if ( pNode->left == NULL )
		return 0;
	if ( pNode->right == NULL )
		return 0;
	if ( pNode->left->tag != K_IDENT )
		return 0;
	if ( pNode->right->tag != K_COLUMN )
		return 0;
	return 1;
}

void dateNodeToBigInt(tnode * pNode)
{
  if (pNode != NULL)
  {
    if (pNode->tag == K_TO_DATE)
    {
      assert(pNode->left != NULL);
      DateConversion dateConverter;
      long long value;
      if (pNode->right != NULL)
      {
        assert(pNode->right->getDataType() == tnode::tnodeDataType::NODE_DATA_STRING);
        dateConverter.setInputFormat(pNode->right->getData().val_str);
      }
      value = dateConverter.dateToBigInt(pNode->left->getData().val_str);
      pNode->tag = K_INTEGER;
      pNode->set_int_data(value);
      aq::delete_subtree(pNode->left);
      aq::delete_subtree(pNode->right);
      aq::delete_subtree(pNode->next);
    }
    dateNodeToBigInt(pNode->left);
    dateNodeToBigInt(pNode->right);
    dateNodeToBigInt(pNode->next);
  }
}

void transformExpression(const aq::Base& baseDesc, const aq::TProjectSettings& settings, aq::tnode * tree)
{
  aq::tnode * whereNode = aq::find_main_node(tree, K_WHERE);
  if (whereNode)
  {
    unsigned int tags[] = { K_LT, K_LEQ, K_GT, K_GEQ, K_BETWEEN, K_NOT_BETWEEN, K_LIKE, K_NOT_LIKE };
    for (auto& tag : tags)
    {
      aq::tnode * cmpNode = NULL;
      while ((cmpNode = aq::find_first_node(whereNode, tag)) != NULL)
      {
        aq::expression_transform::transform<aq::WIN32FileMapper>(baseDesc, settings, cmpNode);
      }
    }
  }
}

}
