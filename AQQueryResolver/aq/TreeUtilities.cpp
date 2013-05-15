#include "TreeUtilities.h"
#include "parser/sql92_grm_tab.hpp"
#include <cassert>
#include <algorithm>
#include <aq/Exceptions.h>
#include <aq/Logger.h>
#include <boost/scoped_array.hpp>
#include <boost/filesystem.hpp>

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
    name = pNode->left->data.val_str;
    name += ".";
    name += pNode->right->data.val_str;
  }
  else
  {
    name = pNode->data.val_str;
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
      aq::tnode * as_node = new_node(K_AS);
      as_node->left = pNode->right;
      as_node->right = new_node(K_IDENT);
      as_node->right->eNodeDataType = NODE_DATA_STRING;
      std::string name;
      getRowItemName(as_node->left, name);
      set_string_data(as_node->right, name.c_str());
      pNode->right = as_node;
    }
    assert(pNode->left != NULL);
    if ((pNode->left->tag != K_COMMA) && (pNode->left->tag != K_AS))
    {
      aq::tnode * as_node = new_node(K_AS);
      as_node->left = pNode->left;
      as_node->right = new_node(K_IDENT);
      as_node->right->eNodeDataType = NODE_DATA_STRING;
      std::string name;
      getRowItemName(as_node->left, name);
      set_string_data(as_node->right, name.c_str());
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
      aq::tnode * left_as_node = clone_subtree(pNode);
      pNode->tag = K_AS;
      delete_subtree(pNode->left);
      delete_subtree(pNode->right);
      pNode->left = left_as_node;
      pNode->right = new_node(K_IDENT);
      pNode->right->eNodeDataType = NODE_DATA_STRING;
      std::string name;
      getRowItemName(pNode->left, name);
      set_string_data(pNode->right, name.c_str());
    }
  }
}

//------------------------------------------------------------------------------
void addConditionsToWhere( aq::tnode* pCond, aq::tnode* pStart )
{
	assert( pCond && pStart );
	//eliminate K_JNO from conditions
	aq::tnode* pCondClone = clone_subtree( pCond );
	std::vector<aq::tnode*> nodes;
	andListToNodeArray( pCondClone, nodes );
	std::vector<aq::tnode*> newNodes;
	for( size_t idx = 0; idx < nodes.size(); ++idx )
		if( nodes[idx] && (nodes[idx]->tag != K_JNO) )
			newNodes.push_back( nodes[idx] );
		else
			delete_node( nodes[idx] );
	if( newNodes.size() == 0 )
		return;
	pCondClone = nodeArrayToAndList( newNodes );

	aq::tnode* pWhere = find_main_node( pStart, K_WHERE );
	if( !pWhere )
	{
		//create node
		aq::tnode* fromNode = find_main_node( pStart, K_FROM );
		assert( fromNode );
		pWhere = new_node( K_WHERE );
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
			pLastAnd->left = new_node( K_AND );
			pLastAnd = pLastAnd->left;
		}
		else
		{
			pAux = pWhere->left;
			pWhere->left = new_node( K_AND );
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
		pNode->left = new_node( leftTag );
		pNode->left->left = pAux;

		pAux = pNode->right;
		pNode->right = new_node( rightTag );
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
void solveSelectStar(	aq::tnode* pNode,
            Base& BaseDesc,
						std::vector<std::string>& columnNames,
						std::vector<std::string>& columnDisplayNames )
{
	assert( pNode->tag == K_SELECT && pNode->left );
	if( pNode->left->tag != K_STAR )
		return;
	//get all columns from all tables and return
	//the other verbs will have the columns they need
	aq::tnode* fromNode = find_main_node( pNode, K_FROM );
	std::vector<aq::tnode*> tables;
	commaListToNodeArray( fromNode->left, tables );
	std::vector<aq::tnode*> colRefs;
	for( size_t idx = 0; idx < tables.size(); ++idx )
	{
		assert( tables[idx] && tables[idx]->tag == K_IDENT );
		size_t tableIdx = BaseDesc.getTableIdx( tables[idx]->data.val_str );
		std::vector<Column::Ptr>& columns = BaseDesc.Tables[tableIdx]->Columns;
		for( size_t idx2 = 0; idx2 < columns.size(); ++idx2 )
		{
			aq::tnode* colRef = new_node( K_PERIOD );
			colRef->left = new_node( K_IDENT );
			char* tableName = tables[idx]->data.val_str;
			set_string_data( colRef->left, tableName );
			colRef->right = new_node( K_COLUMN );
			set_string_data( colRef->right, columns[idx2]->getName().c_str() );
			colRefs.push_back( colRef );
			columnNames.push_back( std::string(tableName) + "." + columns[idx2]->getName() );
			columnDisplayNames.push_back( std::string(tableName) + "." + columns[idx2]->getOriginalName() );
		}
	}
	delete_subtree( pNode->left );
	pNode->left = nodeArrayToCommaList( colRefs );
}

//------------------------------------------------------------------------------
void solveSelectStarExterior( aq::tnode* pInterior, aq::tnode* pExterior )
{
	assert( pInterior && pExterior && pExterior->left );
	if( pExterior->left->tag != K_STAR )
		return;

	delete_subtree( pExterior->left );
	pExterior->left = clone_subtree( pInterior->left );
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
	char* tName = tables[0]->data.val_str;
	size_t tIdx = BaseDesc.getTableIdx( tName );
	if( BaseDesc.Tables[tIdx]->Columns.size() == 0 )
		return;
	Column::Ptr col = BaseDesc.Tables[tIdx]->Columns[0];
	if( !col )
		return;
	
	aq::tnode* pWhere = find_main_node( pStart, K_WHERE );
	if( !pWhere )
	{
		pWhere = new_node( K_WHERE );
		pWhere->next = pNode->next;
		pNode->next = pWhere;
	}
	else
	{
		aq::tnode* pAnd = new_node( K_AND );
		pAnd->right = pWhere->left;
		pWhere->left = pAnd;
		pWhere = pAnd;
	}
	
	aq::tnode* newNode = new_node( K_JNO );
	pWhere->left = newNode;
	newNode->left = new_node( K_INNER );
	newNode = newNode->left;
	newNode->left = new_node( K_PERIOD );
	newNode->left->left = new_node( K_IDENT );
	set_string_data( newNode->left->left, tName );
	newNode->left->right = new_node( K_COLUMN );
	set_string_data( newNode->left->right, col->getName().c_str() );
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
			pWhere = new_node( K_WHERE );
			pWhere->left = condNodes;
			pNode->next = pWhere;
		}
		else
		{
			aq::tnode* pAnd = new_node( K_AND );
			pAnd->left = condNodes;
			pAnd->right = pWhere->left;
			pWhere->left = pAnd;
			pWhere = pAnd;
		}
	} while (pInner != NULL);
}

//------------------------------------------------------------------------------
void getAllColumnNodes( aq::tnode*& pNode, std::vector<aq::tnode**>& columnNodes )
{
	if( !pNode )
		return;
	if( pNode->tag == K_COLUMN || pNode->tag == K_PERIOD )
	{
		columnNodes.push_back( &pNode );
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
		std::string column2(interiorColumns[idx]->right->data.val_str);
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
	std::string tableName = pIntSelectAs->right->data.val_str;

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
		newExtTables.push_back( clone_subtree( intTables[idx] ) );
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
	std::string tableName = pIntSelectAs->right->data.val_str;

	std::vector<aq::tnode**> exteriorColumns;
	getAllColumnNodes( pExteriorSelect, exteriorColumns );
	std::vector<aq::tnode*> interiorColumns;
	getColumnsList( pInteriorSelect->left, interiorColumns );
	for( size_t idx = 0; idx < exteriorColumns.size(); ++idx )
	{
		if( !exteriorColumns[idx] || !*exteriorColumns[idx] )
			continue;
		aq::tnode*& extCol = *exteriorColumns[idx];
		switch( extCol->tag )
		{
		case K_PERIOD:
			{
				assert( extCol->left && extCol->left->tag == K_IDENT && 
					extCol->right->tag == K_COLUMN );
				if( tableName != std::string(extCol->left->data.val_str) )
					continue;
				std::string columnName = extCol->right->data.val_str;
				aq::tnode* replCol = findColumn( columnName, interiorColumns, keepAlias );
				if( replCol )
				{
					delete_subtree( extCol );
					extCol = clone_subtree( replCol );
				}
			}
			break;
		case K_COLUMN:
			{
				std::string columnName = extCol->data.val_str;
				aq::tnode* replCol = findColumn( columnName, interiorColumns, keepAlias );
				if( replCol )
				{
					delete_subtree( extCol );
					extCol = clone_subtree( replCol );
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
  else if (!((pNode->tag == K_JEQ) || (pNode->tag == K_JAUTO)))
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
			if ((tmp->tag == K_IDENT) && (tmp->eNodeDataType == NODE_DATA_STRING))
			{
				tableName = tmp->data.val_str;
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

	this->tableName = tables[0]->data.val_str;

	this->pGroupBy = pGroup->next;
	pGroup->next = pGroup->next->next;

	aq::tnode* pAux = auxColumns[minMaxCol]->left;
	auxColumns[minMaxCol]->left = auxColumns[minMaxCol]->left->left;
	delete_node( pAux );

	//make copies of the column nodes because they belong to the query
	//and the query might get modified before modifyTmpFiles is called
	for( size_t idx = 0; idx < auxColumns.size(); ++idx )
		this->columns.push_back( clone_subtree(auxColumns[idx]) );

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
	startIdx = max( startIdx, (size_t) 0 );
	endIdx = min( endIdx, vals.size() );
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
void SolveMinMaxGroupBy::modifyTmpFiles(	const char* tmpPath, 
											int selectLevel,
											Base& BaseDesc, 
											TProjectSettings& Settings )
{
	if( !this->pGroupBy )
		return;

	/*std::vector<string> files;
	if( GetFiles( tmpPath, files ) != 0 )
		throw generic_error(generic_error::INVALID_FILE, "");*/

	size_t tIdx1 = BaseDesc.getTableIdx( this->tableName );
	/*std::vector<llong> rows;
	/* read tmp file
	for( size_t idx = 0; idx < files.size(); ++idx )
	{
		string valstr = files[idx].substr( 5, 4 );
		int tIdx2 = atoi( valstr.c_str() );
		if( (tIdx1 + 1) != tIdx2 )
			continue;
		string filePath = tmpPath;
		filePath += files[idx];
		readTmpFile( filePath.c_str(), rows );
	}*/
	/*for( size_t idx = 0; idx < files.size(); ++idx )
	{
		string filePath = tmpPath;
		filePath += "\\";
		filePath += files[idx];
		readPosFile( filePath.c_str(), rows );
	}
	if( rows.size() < 1 )
		return; //nothing to modify
	sort( rows.begin(), rows.end() );*/

	Table& templateTable = *BaseDesc.Tables[tIdx1];
	std::vector<int> colIds;
	getColumnsIds( templateTable, this->columns, colIds );

	Table table;
	std::vector<Column::Ptr> columnTypes;
	for( size_t idx = 0; idx < colIds.size(); ++idx )
	{
		columnTypes.push_back( new Column(*templateTable.Columns[colIds[idx]]) );
		columnTypes[idx]->setTableName( this->tableName );
	}

	//load table from disk
	/*char* myRecord = new char[Settings.maxRecordSize];
	boost::scoped_array<char> myRecordDel( myRecord );

	string tablePath = Settings.szRootPath;
	tablePath += "data_orga\\tables\\" + templateTable.getOriginalName() + ".txt";
	FILE* fIn = fopenUTF8( tablePath.c_str(), "rt" );
	if( !fIn )
		throw generic_error(generic_error::COULD_NOT_OPEN_FILE, "");;

	int idxRows = 0;
	for( llong idx = 0; idx <= rows[rows.size() - 1]; ++idx )
	{
		fgets( myRecord, Settings.maxRecordSize, fIn );
		if( idx != rows[idxRows] )
			continue;
		++idxRows;

		std::vector<char*> fields;
		splitLine( myRecord, Settings.fieldSeparator, fields, false );
		for( size_t idx2 = 0; idx2 < colIds.size(); ++idx2 )
			table.Columns[idx2]->Items.push_back( 
				new ColumnItem(fields[colIds[idx2]], table.Columns[idx2]->Type) );
	}*/
  std::vector<std::string> answerFiles;
  answerFiles.push_back(Settings.szAnswerFN);

	aq::AQMatrix aqMatrix(Settings);
	std::vector<llong> tableIDs;
	for (std::vector<std::string>::const_iterator it = answerFiles.begin(); it != answerFiles.end(); ++it)
	{
		aqMatrix.load((*it).c_str(), Settings.fieldSeparator, tableIDs);
	}

	table.loadFromTableAnswerByColumn( aqMatrix, tableIDs, columnTypes, Settings, BaseDesc );

	TablePartition::Ptr partition = new TablePartition();
	std::vector<size_t> index;
	std::vector<Column::Ptr> orderColumns;
	for( size_t idx = 0; idx < table.Columns.size(); ++idx )
		if( idx != minMaxCol )
			orderColumns.push_back( table.Columns[idx] );
	table.orderBy(orderColumns, partition, index);
	assert( partition->Rows.size() > 0 );
	std::vector<llong> finalRows;
	for( size_t idx = 0; idx < partition->Rows.size() - 1; ++idx )
	{
		size_t selIdx = partition->Rows[idx];
		for( size_t idx2 = partition->Rows[idx] + 1; idx2 < partition->Rows[idx+1]; ++idx2 )
			if( this->_min )
			{
				if( ColumnItem::lessThan(	table.Columns[minMaxCol]->Items[idx2].get(),
								table.Columns[minMaxCol]->Items[selIdx].get(),
								table.Columns[minMaxCol]->Type ) )
					selIdx = idx2;
			}
			else
			{
				if( ColumnItem::lessThan(	table.Columns[minMaxCol]->Items[selIdx].get(),
								table.Columns[minMaxCol]->Items[idx2].get(),
								table.Columns[minMaxCol]->Type ) )
					selIdx = idx2;
			}

		finalRows.push_back( index[selIdx] );
	}

	int nrPacks = (int) (finalRows.size() / Settings.packSize);
	if( finalRows.size() % Settings.packSize != 0 )
		++nrPacks;
	--selectLevel;
	char buf[_MAX_PATH];
	sprintf( buf, "%s_%d", Settings.szTempPath1, selectLevel );

  boost::filesystem::path path(buf);
  if (!boost::filesystem::create_directory(path))
  {
    throw aq::generic_error(aq::generic_error::COULD_NOT_OPEN_FILE, "cannot create directory '%s'", buf);
  }

  for( size_t idx = 0; idx < nrPacks; ++idx )
	{
		char file[_MAX_PATH];
		//BxxxTxxxxPxxxxxxxxxx
		sprintf( file, "%s\\B001T%04dP%010d.tmp", path, 
			selectLevel, tIdx1 + 1, idx + 1 );
		writeTmpFile( file, finalRows, idx * Settings.packSize, 
			(idx + 1) * Settings.packSize );
	}
}

//------------------------------------------------------------------------------
void getColumnsIds(	const Table& table, std::vector<aq::tnode*>& columns, std::vector<int>& valuePos )
{
	std::vector<std::string> columnsStr;
	for( size_t idx = 0; idx < columns.size(); ++idx )
	{
		if( columns[idx]->tag == K_PERIOD )
			columnsStr.push_back( std::string(columns[idx]->right->data.val_str) );
		else if( columns[idx]->tag == K_AS )
			columnsStr.push_back( std::string(columns[idx]->left->right->data.val_str) );
		else if( columns[idx]->tag == K_COLUMN )
			columnsStr.push_back( std::string(columns[idx]->data.val_str) );
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
			newColumns.push_back( clone_subtree(columns[idx]) );
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
	} else if( pNode->tag == K_COMMA )
	{
		getColumnsList( pNode->left, columns );
		getColumnsList( pNode->right, columns );
	} else
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
      tables.push_back( pNode->data.val_str );
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

	if( pNode->tag != K_COMMA )
		return;

	if( pNode->left != NULL && pNode->right != NULL )
		return; //no null to delete
	if( pNode->left == NULL && pNode->right == NULL )
	{
		//delete entire node
		delete_node( pNode );
		pNode = NULL;
		return;
	}
	//delete left null
	aq::tnode* newNode = pNode->left;
	if( pNode->right != NULL )
		newNode = pNode->right; //or right null
	delete_node( pNode );
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
		size_t tableIdx = BaseDesc.getTableIdx( colNode->left->data.val_str );
		assert( tableIdx < BaseDesc.Tables.size() ); //debug13 possible wrong query formatting, should throw
		Table& table = *BaseDesc.Tables[tableIdx];
		bool found = false;
		Column auxCol;
		auxCol.setName(colNode->right->data.val_str);
		for( size_t idx = 0; idx < table.Columns.size(); ++idx )
			if( table.Columns[idx]->getName() == auxCol.getName() )
			{
				Column::Ptr column = new Column(*table.Columns[idx]);
        column->setTableName(table.getName());
				//column->Type = table.Columns[idx]->Type;
				//column->setTableName( table.getName() );
				//column->setName( table.Columns[idx]->getName() );
				//column->Size = table.Columns[idx]->Size;
				//column->ID = table.Columns[idx]->ID;
				columnTypes.push_back( column );
				found = true;
				break;
			}
		assert( found ); // tma: FIXME: raise an exception
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
	case COL_TYPE_DATE1:
	case COL_TYPE_DATE2:
	case COL_TYPE_DATE3:
		pNode = new_node( K_INTEGER );
		set_int_data( pNode, (llong) item->numval );
		break;
	case COL_TYPE_DOUBLE:
		pNode = new_node( K_REAL );
		set_double_data( pNode, item->numval );
		break;
	default:
		pNode = new_node( K_STRING );
		set_string_data( pNode, item->strval.c_str() );
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
	pNode = new_node( K_COMMA );
	pStart = pNode;
	size_t size = column.Items.size();
	for( size_t idx = 0; idx < size - 2; ++idx )
	{
		pNode->right = new_node( K_COMMA );
		pNode->left = Getnode(column.Items[idx], column.Type);
		pNode = pNode->right;
	}
	pNode->left = Getnode(column.Items[size - 2], column.Type);
	pNode->right = Getnode(column.Items[size - 1], column.Type);
	
	return pStart;
}

}