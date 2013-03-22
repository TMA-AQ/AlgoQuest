#include "TreeUtilities.h"
#include <assert.h>
#include "sql92_grm_tab.h"
#include "Table.h"
#include <aq/Exceptions.h>
#include <algorithm>
#include <boost/scoped_array.hpp>
#include <direct.h>

using namespace aq;
using namespace std;

//------------------------------------------------------------------------------
const int nrJoinTypes = 7;
const int joinTypes[] = { K_JEQ, K_JAUTO, K_JNEQ, K_JINF, K_JIEQ, K_JSUP, K_JSEQ };
const int inverseTypes[] = { K_JEQ, K_JAUTO, K_JNEQ, K_JSUP, K_JSEQ, K_JINF, K_JIEQ };

//------------------------------------------------------------------------------
void addConditionsToWhere( tnode* pCond, tnode* pStart )
{
	assert( pCond && pStart );
	//eliminate K_JNO from conditions
	tnode* pCondClone = clone_subtree( pCond );
	vector<tnode*> nodes;
	andListToNodeArray( pCondClone, nodes );
	vector<tnode*> newNodes;
	for( size_t idx = 0; idx < nodes.size(); ++idx )
		if( nodes[idx] && (nodes[idx]->tag != K_JNO) )
			newNodes.push_back( nodes[idx] );
		else
			delete_node( nodes[idx] );
	if( newNodes.size() == 0 )
		return;
	pCondClone = nodeArrayToAndList( newNodes );

	tnode* pWhere = find_main_node( pStart, K_WHERE );
	if( !pWhere )
	{
		//create node
		tnode* fromNode = find_main_node( pStart, K_FROM );
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
		tnode* pLastAnd, *pAux;
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
void addInnerOuterNodes( tnode* pNode, int leftTag, int rightTag )
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
		tnode* pAux = pNode->left;
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
void mark_as_deleted( tnode* pNode )
{
	if( !pNode )
		return;
	pNode->tag = K_DELETED;
	mark_as_deleted( pNode->left );
	mark_as_deleted( pNode->right );
	mark_as_deleted( pNode->next );
}

//------------------------------------------------------------------------------
void solveSelectStar(	tnode* pNode,
            Base& BaseDesc,
						std::vector<std::string>& columnNames,
						std::vector<std::string>& columnDisplayNames )
{
	assert( pNode->tag == K_SELECT && pNode->left );
	if( pNode->left->tag != K_STAR )
		return;
	//get all columns from all tables and return
	//the other verbs will have the columns they need
	tnode* fromNode = find_main_node( pNode, K_FROM );
	vector<tnode*> tables;
	commaListToNodeArray( fromNode->left, tables );
	vector<tnode*> colRefs;
	for( size_t idx = 0; idx < tables.size(); ++idx )
	{
		assert( tables[idx] && tables[idx]->tag == K_IDENT );
		int tableIdx = BaseDesc.getTableIdx( tables[idx]->data.val_str );
		if( tableIdx < 0 )
			throw generic_error(generic_error::INVALID_TABLE, "");
		std::vector<Column::Ptr>& columns = BaseDesc.Tables[tableIdx].Columns;
		for( size_t idx2 = 0; idx2 < columns.size(); ++idx2 )
		{
			tnode* colRef = new_node( K_PERIOD );
			colRef->left = new_node( K_IDENT );
			char* tableName = tables[idx]->data.val_str;
			set_string_data( colRef->left, tableName );
			colRef->right = new_node( K_COLUMN );
			set_string_data( colRef->right, columns[idx2]->getName().c_str() );
			colRefs.push_back( colRef );
			columnNames.push_back( string(tableName) + "." + columns[idx2]->getName() );
			columnDisplayNames.push_back( string(tableName) + "." + columns[idx2]->getOriginalName() );
		}
	}
	delete_subtree( pNode->left );
	pNode->left = nodeArrayToCommaList( colRefs );
}

//------------------------------------------------------------------------------
void solveSelectStarExterior( tnode* pInterior, tnode* pExterior )
{
	assert( pInterior && pExterior && pExterior->left );
	if( pExterior->left->tag != K_STAR )
		return;

	delete_subtree( pExterior->left );
	pExterior->left = clone_subtree( pInterior->left );
}

//------------------------------------------------------------------------------
void solveOneTableInFrom( tnode* pStart, Base& BaseDesc )
{
	assert( pStart && pStart->tag == K_SELECT );
	tnode* pNode = find_main_node( pStart, K_FROM );
	/*vector<tnode*> tables;
	commaListToNodeArray( pNode->left, tables );
	for( size_t idx = 0; idx < tables.size(); ++idx )
		if( tables[idx] && tables[idx]->tag == K_AS )
		{
			replaceTableIdent( pStart, tables[idx]->right->data.val_str, tables[idx]->left->data.val_str );
			*tables[idx] = *tables[idx]->left; //no memory leaks
		}*/
	vector<tnode*> tables;
	commaListToNodeArray( pNode->left, tables );
	/*tnode* oldTables = pNode->left;
	vector<tnode*> newTables;
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
	int tIdx = BaseDesc.getTableIdx( tName );
	if( BaseDesc.Tables[tIdx].Columns.size() == 0 )
		return;
	Column::Ptr col = BaseDesc.Tables[tIdx].Columns[0];
	if( !col )
		return;
	
	tnode* pWhere = find_main_node( pStart, K_WHERE );
	if( !pWhere )
	{
		pWhere = new_node( K_WHERE );
		pWhere->next = pNode->next;
		pNode->next = pWhere;
	}
	else
	{
		tnode* pAnd = new_node( K_AND );
		pAnd->right = pWhere->left;
		pWhere->left = pAnd;
		pWhere = pAnd;
	}
	
	tnode* newNode = new_node( K_JNO );
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
void moveFromJoinToWhere( tnode* pStart, Base& BaseDesc )
{	
	assert( pStart && pStart->tag == K_SELECT );

	tnode* pNode = find_main_node( pStart, K_FROM );

	//
	// Get All the tables in inner clause
	tnode* pInner = NULL;
	do
	{
		if ((pInner = find_deeper_node( pNode, K_INNER )) == NULL)
			return;

		assert(pInner->left && (pInner->left->tag == K_JOIN));
		assert(pInner->left->next && (pInner->left->next->tag == K_ON));
		tnode* tablesNodes = pInner->left;
		tnode* condNodes = pInner->left->next->left;
		
		assert(tablesNodes->left && ((tablesNodes->left->tag == K_IDENT) || (tablesNodes->left->tag == K_COMMA)));
		assert(tablesNodes->right && ((tablesNodes->right->tag == K_IDENT) || (tablesNodes->right->tag == K_COMMA)));

		pInner->tag = K_COMMA;
		pInner->left = tablesNodes->left;
		pInner->right = tablesNodes->right;
		
		addInnerOuterNodes( condNodes, K_INNER, K_INNER );
		condNodes->inf = 1;

		tnode* pWhere = find_main_node( pStart, K_WHERE );
		if( !pWhere )
		{
			pWhere = new_node( K_WHERE );
			pWhere->left = condNodes;
			pNode->next = pWhere;
		}
		else
		{
			tnode* pAnd = new_node( K_AND );
			pAnd->left = condNodes;
			pAnd->right = pWhere->left;
			pWhere->left = pAnd;
			pWhere = pAnd;
		}
	} while (pInner != NULL);
}

//------------------------------------------------------------------------------
void getAllColumnNodes( tnode*& pNode, vector<tnode**>& columnNodes )
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
tnode* findColumn(  string columnName, std::vector<tnode*>& interiorColumns,
					bool keepAlias)
{
	strtoupr( columnName );
	for( size_t idx = 0; idx < interiorColumns.size(); ++idx )
	{
		if( !interiorColumns[idx] )
			continue;
		string column2(interiorColumns[idx]->right->data.val_str);
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
void changeTableNames(	tnode* pIntSelectAs, tnode* pInteriorSelect, 
						tnode* pExteriorSelect )
{
	string tableName = pIntSelectAs->right->data.val_str;

	tnode* intFromNode = find_main_node( pInteriorSelect, K_FROM );
	vector<tnode*> intTables;
	commaListToNodeArray( intFromNode->left, intTables );

	tnode* extFromNode = find_main_node( pExteriorSelect, K_FROM );
	vector<tnode*> extTables;
	commaListToNodeArray( extFromNode->left, extTables );
	
	vector<tnode*> newExtTables;
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
	tnode* intSelectAs = (*pIntSelectAs)->next;
	(*pIntSelectAs)->left = NULL;*/

	/*
	//extract interior select definition from the exterior select
	tnode** pComma = getLastTag( pExteriorSelect, NULL, *pIntSelectAs, K_COMMA );
	tnode* intSelectAs = *pIntSelectAs;
	if( pComma )
	{
		assert( *pComma );
		tnode* pAux = (*pComma)->left;
		if( pAux == *pIntSelectAs )
			pAux = (*pComma)->right;
		tnode* pAux2 = *pComma;
		*pComma = pAux;
		delete_node( pAux2 );
	}*/
}

//------------------------------------------------------------------------------
void changeColumnNames(	tnode* pIntSelectAs, tnode* pInteriorSelect, 
						tnode* pExteriorSelect, bool keepAlias )
{
	string tableName = pIntSelectAs->right->data.val_str;

	std::vector<tnode**> exteriorColumns;
	getAllColumnNodes( pExteriorSelect, exteriorColumns );
	std::vector<tnode*> interiorColumns;
	getColumnsList( pInteriorSelect->left, interiorColumns );
	for( size_t idx = 0; idx < exteriorColumns.size(); ++idx )
	{
		if( !exteriorColumns[idx] || !*exteriorColumns[idx] )
			continue;
		tnode*& extCol = *exteriorColumns[idx];
		switch( extCol->tag )
		{
		case K_PERIOD:
			{
				assert( extCol->left && extCol->left->tag == K_IDENT && 
					extCol->right->tag == K_COLUMN );
				if( tableName != string(extCol->left->data.val_str) )
					continue;
				string columnName = extCol->right->data.val_str;
				tnode* replCol = findColumn( columnName, interiorColumns, keepAlias );
				if( replCol )
				{
					delete_subtree( extCol );
					extCol = clone_subtree( replCol );
				}
			}
			break;
		case K_COLUMN:
			{
				string columnName = extCol->data.val_str;
				tnode* replCol = findColumn( columnName, interiorColumns, keepAlias );
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
bool isMonoTable(tnode * query, std::string& tableName)
{
	//
	// Get From
	tnode * fromNode = find_main_node(query, K_FROM);
	if (fromNode)
	{
		std::vector<tnode*> tables;
		commaListToNodeArray( fromNode->left, tables );
		if (tables.size() == 1)
		{
			tnode * tmp = *tables.begin();
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
SolveMinMaxGroupBy::SolveMinMaxGroupBy(): pGroupBy(NULL), min(true)
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
bool SolveMinMaxGroupBy::checkAndClear( tnode* pSelect )
{
	vector<tnode*> auxColumns;
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
		min = auxColumns[idx]->left->tag == K_MIN;
		++nrMinMaxCols;
	}
	if( nrMinMaxCols != 1 )
		return false;

	tnode* pGroup = pSelect;
	while( pGroup->next && pGroup->next->tag != K_GROUP )
		pGroup = pGroup->next;
	if( !pGroup->next )
		return false;

	tnode* pFrom = find_main_node( pSelect, K_FROM );
	if( !pFrom )
		return false;
	vector<tnode*> tables;
	commaListToNodeArray( pFrom->left, tables );
	
	if( tables.size() != 1 )
		return false; //no min+group by or max+group by + from 1 table

	this->tableName = tables[0]->data.val_str;

	this->pGroupBy = pGroup->next;
	pGroup->next = pGroup->next->next;

	tnode* pAux = auxColumns[minMaxCol]->left;
	auxColumns[minMaxCol]->left = auxColumns[minMaxCol]->left->left;
	delete_node( pAux );

	//make copies of the column nodes because they belong to the query
	//and the query might get modified before modifyTmpFiles is called
	for( size_t idx = 0; idx < auxColumns.size(); ++idx )
		this->columns.push_back( clone_subtree(auxColumns[idx]) );

	return true;
}

//------------------------------------------------------------------------------
void readTmpFile( const char* filePath, vector<llong>& vals )
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
void writeTmpFile(	const char* filePath, const vector<llong>& vals,
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
void readPosFile( const char* filePath, vector<llong>& vals )
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

	/*vector<string> files;
	if( GetFiles( tmpPath, files ) != 0 )
		throw generic_error(generic_error::INVALID_FILE, "");*/

	int tIdx1 = BaseDesc.getTableIdx( this->tableName );
	/*vector<llong> rows;
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

	Table& templateTable = BaseDesc.Tables[tIdx1];
	vector<int> colIds;
	getColumnsIds( templateTable, this->columns, colIds );

	Table table;
	vector<Column::Ptr> columnTypes;
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

		vector<char*> fields;
		splitLine( myRecord, Settings.fieldSeparator, fields, false );
		for( size_t idx2 = 0; idx2 < colIds.size(); ++idx2 )
			table.Columns[idx2]->Items.push_back( 
				new ColumnItem(fields[colIds[idx2]], table.Columns[idx2]->Type) );
	}*/
  std::vector<std::string> answerFiles;
  answerFiles.push_back(Settings.szAnswerFN);

	aq::AQMatrix aqMatrix;
	vector<llong> tableIDs;
	for (std::vector<std::string>::const_iterator it = answerFiles.begin(); it != answerFiles.end(); ++it)
	{
		aqMatrix.load((*it).c_str(), Settings.fieldSeparator, tableIDs);
	}

	table.loadFromTableAnswerByColumn( aqMatrix, tableIDs, columnTypes, Settings, BaseDesc );

	TablePartition::Ptr partition = new TablePartition();
	vector<size_t> index;
	vector<Column::Ptr> orderColumns;
	for( size_t idx = 0; idx < table.Columns.size(); ++idx )
		if( idx != minMaxCol )
			orderColumns.push_back( table.Columns[idx] );
	table.orderBy(orderColumns, partition, index);
	assert( partition->Rows.size() > 0 );
	vector<llong> finalRows;
	for( size_t idx = 0; idx < partition->Rows.size() - 1; ++idx )
	{
		size_t selIdx = partition->Rows[idx];
		for( size_t idx2 = partition->Rows[idx] + 1; idx2 < partition->Rows[idx+1]; ++idx2 )
			if( this->min )
			{
				if( lessThan(	table.Columns[minMaxCol]->Items[idx2].get(),
								table.Columns[minMaxCol]->Items[selIdx].get(),
								table.Columns[minMaxCol]->Type ) )
					selIdx = idx2;
			}
			else
			{
				if( lessThan(	table.Columns[minMaxCol]->Items[selIdx].get(),
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
	char path[_MAX_PATH];
	sprintf( path, "%s_%d", Settings.szTempPath1, selectLevel );
	mkdir( path );
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
void getColumnsIds(	const Table& table, vector<tnode*>& columns, 
					vector<int>& valuePos )
{
	vector<string> columnsStr;
	for( size_t idx = 0; idx < columns.size(); ++idx )
	{
		if( columns[idx]->tag == K_PERIOD )
			columnsStr.push_back( string(columns[idx]->right->data.val_str) );
		else if( columns[idx]->tag == K_AS )
			columnsStr.push_back( string(columns[idx]->left->right->data.val_str) );
		else if( columns[idx]->tag == K_COLUMN )
			columnsStr.push_back( string(columns[idx]->data.val_str) );
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
void eliminateAliases( tnode* pSelect )
{
	assert( pSelect );
	vector<tnode*> columns;
	commaListToNodeArray( pSelect->left, columns );
	vector<tnode*> newColumns;
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
void getColumnsList( tnode* pNode,std::vector<tnode*>& columns )
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
