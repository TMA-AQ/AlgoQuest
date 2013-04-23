#include "QueryResolver.h"
#include "SQLPrefix.h"
#include "sql92_grm_tab.h"
#include "Table.h"

#include "RowProcesses.h"
#include "RowProcessing.h"
#include "RowVerbProcess.h"

#include "verbs/Verb.h"
#include "JeqParser.h"
#include "Column2Table.h"
#include "ExprTransform.h"
#include "TreeUtilities.h"
#include "Optimizations.h"
#include "AQEngine_Intf.h"
#include "ColumnMapper.h"
#include "DumpVisitor.h"

#include <aq/Exceptions.h>
#include <aq/Logger.h>
#include <aq/DateConversion.h>

#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <utility>

#include <deque>
#include <algorithm>
#include <boost/scoped_array.hpp>

using namespace aq;
using namespace std;

//------------------------------------------------------------------------------
tnode* GetNode( ColumnItem::Ptr item, ColumnType type )
{
	tnode	*pNode = NULL;
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
tnode* GetTree( Table& table )
{
	tnode	*pNode = NULL;
	tnode	*pStart = NULL;

	if( table.Columns.size() < 1 )
		return NULL;
	Column& column = *table.Columns[0];
	if( column.Items.size() < 1 )
		return NULL;
	if( column.Items.size() < 2 )
		return GetNode(column.Items[0], column.Type);

	//we have a list
	pNode = new_node( K_COMMA );
	pStart = pNode;
	size_t size = column.Items.size();
	for( size_t idx = 0; idx < size - 2; ++idx )
	{
		pNode->right = new_node( K_COMMA );
		pNode->left = GetNode(column.Items[idx], column.Type);
		pNode = pNode->right;
	}
	pNode->left = GetNode(column.Items[size - 2], column.Type);
	pNode->right = GetNode(column.Items[size - 1], column.Type);
	
	return pStart;
}

//------------------------------------------------------------------------------
//------------------------------------------------------------------------------

//------------------------------------------------------------------------------
QueryResolver::QueryResolver(tnode * _sqlStatement, TProjectSettings * _pSettings, AQEngine_Intf * _aq_engine, Base& _baseDesc, unsigned int _level, unsigned int _id)
	:	sqlStatement(_sqlStatement),
		pSettings(_pSettings), 
		aq_engine(_aq_engine), 
		BaseDesc(_baseDesc), 
    id(_id),
    nestedId(0),
    level(_level),
		nested(id != 1)
{
	memset(szBuffer, 0, STR_BUF_SIZE);
}

//------------------------------------------------------------------------------
QueryResolver::~QueryResolver()
{
	aq::Logger::getInstance().log(AQ_INFO, "Query Resolver: Time elapsed = %s\n", aq::Timer::getString(timer.getTimeElapsed()).c_str());
}

/*
//------------------------------------------------------------------------------
void PreorderTraversal(	VerbNode::Ptr spNode, 
					   vector<VerbNode::Ptr>& preorderVerbTree )
{
	preorderVerbTree.push_back( spNode );
	for( size_t idx = 0; idx < spNode->getChildren().size(); ++idx )
		PreorderTraversal( spNode->getChildren()[idx], preorderVerbTree );
}*/

//------------------------------------------------------------------------------
void QueryResolver::cleanQuery( tnode*& pNode )
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
	tnode* newNode = pNode->left;
	if( pNode->right != NULL )
		newNode = pNode->right; //or right null
	delete_node( pNode );
	pNode = newNode;
}

//------------------------------------------------------------------------------
void QueryResolver::getColumnTypes( tnode* pNode, vector<Column::Ptr>& columnTypes, Base& BaseDesc )
{
	if( !pNode || !pNode->left )
		return;
//	assert( pNode->left ); //debug13 not necessarily true, I should really start to handle this case
	while( pNode->left->tag == K_PERIOD || pNode->left->tag == K_COMMA )
	{
		pNode = pNode->left;
		tnode* colNode;
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
		Table& table = BaseDesc.Tables[tableIdx];
		bool found = false;
		Column auxCol;
		auxCol.setName(colNode->right->data.val_str);
		for( size_t idx = 0; idx < table.Columns.size(); ++idx )
			if( table.Columns[idx]->getName() == auxCol.getName() )
			{
				Column::Ptr column = new Column();
				column->Type = table.Columns[idx]->Type;
				column->setTableName( table.getName() );
				column->setName( table.getName() + "." + table.Columns[idx]->getName() );
				column->Size = table.Columns[idx]->Size;
				column->ID = table.Columns[idx]->ID;
				columnTypes.push_back( column );
				found = true;
				break;
			}
		assert( found ); // tma: FIXME: raise an exception
	}
	reverse(columnTypes.begin(), columnTypes.end());
}

//------------------------------------------------------------------------------
void QueryResolver::addUnionMinusNode(	int tag, vector<tnode*>& queries, vector<int>& operation, 
						tnode* pNode )
{
	assert( pNode );
	if( tag == K_SEL_MINUS )
	{
		operation.push_back( 2 );
		assert( pNode->tag == K_SELECT );
		queries.push_back( pNode );
	}
	else
	{
		assert( tag == K_UNION );
		if( pNode->tag == K_ALL )
		{
			operation.push_back( 1 );
			assert( pNode->left );
			assert( pNode->left->tag == K_SELECT );
			queries.push_back( pNode->left );
		}
		else
		{
			operation.push_back( 0 );
			assert( pNode->tag == K_SELECT );
			queries.push_back( pNode );
		}
	}
}

//-------------------------------------------------------------------------------
Table::Ptr QueryResolver::SolveSelect()
{
	assert( this->sqlStatement->tag == K_SELECT );
	
	this->hasGroupBy = find_main_node(this->sqlStatement, K_GROUP) != NULL;
	this->hasOrderBy = find_main_node(this->sqlStatement, K_ORDER) != NULL;
	this->hasPartitionBy = find_main_node(this->sqlStatement, K_OVER) != NULL;

	SolveSelectRecursive( this->sqlStatement, this->level, NULL, false, false );
	return SolveSelectRegular();
}

//-------------------------------------------------------------------------------
Table::Ptr QueryResolver::SolveSelectRegular()
{
	aq::Timer timer;
	Table::Ptr table;

#ifdef OUTPUT_NESTED_QUERIES
	std::string str;
	syntax_tree_to_prefix_form( this->sqlStatement, str );
	
	SaveFile( pSettings->szOutputFN, str.c_str() );
	MakeBackupFile( pSettings->szOutputFN, backup_type_t::Before );
#endif
  
#ifdef _DEBUG
  std::ostringstream oss;
  oss << *this->sqlStatement << std::endl;
  std::cout << oss.str() << std::endl;
  std::string stmp1 = oss.str();
#endif

	//
	// Query Pre Processing (TODO : optimize tree by detecting identical subtrees)
	timer.start();
	VerbNode::Ptr spTree = VerbNode::BuildVerbsTree( this->sqlStatement, this->BaseDesc, this->pSettings );
	spTree->changeQuery();

#ifdef _DEBUG
  {
    DumpVisitor printer;
    spTree->acceptLeftToRight(&printer);
    std::cout << printer.getQuery() << std::endl;
  }
#endif

	QueryResolver::cleanQuery( this->sqlStatement );
	aq::Logger::getInstance().log(AQ_INFO, "Query Preprocessing: Time elapsed = %s\n", aq::Timer::getString(timer.getTimeElapsed()).c_str());
  
#ifdef _DEBUG
  oss.str() = "";
  oss << *this->sqlStatement << std::endl;
  std::string stmp2 = oss.str();
  DumpVisitor printer;
  spTree->acceptLeftToRight(&printer);
  std::cout << oss.str() << std::endl << printer.getQuery() << std::endl;
#endif

	//
	// Solve Optimal Min/Max : FIXME
	timer.start();
	table = solveOptimalMinMax( spTree, BaseDesc, *pSettings );
	aq::Logger::getInstance().log(AQ_INFO, "Solve Optimal Min/Max: Time elapsed = %s\n", aq::Timer::getString(timer.getTimeElapsed()).c_str());
	
	if( table )
		return table;

	//vector<VerbNode::Ptr> preorderVerbTree;
	//PreorderTraversal( spStart, preorderVerbTree );
	//for( int idx = (int) preorderVerbTree.size() - 1; idx >= 0; --idx )
	//	preorderVerbTree[idx]->changeQuery();
	
	aq_engine->call( this->sqlStatement, 0, this->id );

	if (pSettings->computeAnswer)
	{
	
		//
		if (pSettings->useRowResolver && !this->nested && !this->hasOrderBy && !this->hasPartitionBy)
		{
			table = this->solveAQMatriceByRows(spTree);
		}
		else
		{
			table = this->solveAQMatriceByColumns(spTree);
		}
		spTree = NULL; //debug13 - force delete to see if it causes an error
	}

	return table;
}

//------------------------------------------------------------------------------
boost::shared_ptr<QueryResolver> QueryResolver::SolveSelectFromSelect(	tnode* pInteriorSelect, tnode* pExteriorSelect, int nSelectLevel )
{
	if( !pInteriorSelect || !pExteriorSelect )
  {
    throw aq::generic_error(aq::generic_error::INVALID_QUERY, "");
  }
  
  boost::shared_ptr<QueryResolver> query(new QueryResolver(pInteriorSelect, this->pSettings, this->aq_engine, this->BaseDesc));

#if _DEBUG
  std::cout << *pInteriorSelect << std::endl;
#endif

#ifdef OUTPUT_NESTED_QUERIES
	std::string str;
	syntax_tree_to_prefix_form( pExteriorSelect, str );
	SaveFile( pSettings->szOutputFN, str.c_str() );
  MakeBackupFile( pSettings->szOutputFN, backup_type_t::Exterior_Before );
#endif

	solveSelectStar( pInteriorSelect, BaseDesc );
	solveOneTableInFrom( pInteriorSelect, BaseDesc );

	tnode* pIntSelectAs = getLastTag( pExteriorSelect, NULL, pInteriorSelect, K_AS );
	if( !pIntSelectAs || !pIntSelectAs->right || (pIntSelectAs->right->tag != K_IDENT) )
		throw generic_error(generic_error::INVALID_QUERY, "");

	SolveMinMaxGroupBy solveMinMaxGroupBy;
	bool minMaxGroupBy = solveMinMaxGroupBy.checkAndClear( pInteriorSelect );

	changeTableNames( pIntSelectAs, pInteriorSelect, pExteriorSelect );
	changeColumnNames( pIntSelectAs, pInteriorSelect, pExteriorSelect->left, true );
	changeColumnNames( pIntSelectAs, pInteriorSelect, pExteriorSelect->next, false );
	solveSelectStarExterior( pInteriorSelect, pExteriorSelect );

  if( trivialSelectFromSelect(pInteriorSelect) )
	{
		syntax_tree_to_prefix_form( pInteriorSelect, str );
		SaveFile( pSettings->szOutputFN, str.c_str() );
		MakeBackupFile( pSettings->szOutputFN, backup_type_t::Empty );
		mark_as_deleted( pIntSelectAs );
		return query;
	}

	eliminateAliases( pInteriorSelect );

	// copy conditions from inner select WHERE into outer select WHERE
	tnode* intWhereNode = find_main_node( pInteriorSelect, K_WHERE );
	if( intWhereNode )
	{
    // keep only join
    tnode * joinNode = clone_subtree(intWhereNode->left);
    joinNode = getJoin(joinNode);
		addConditionsToWhere( joinNode, pExteriorSelect );
	}
	addInnerOuterNodes( intWhereNode->left, K_INNER, K_INNER );

#ifdef OUTPUT_NESTED_QUERIES
	syntax_tree_to_prefix_form( pExteriorSelect, str );
	SaveFile( pSettings->szOutputFN, str.c_str() );
	MakeBackupFile( pSettings->szOutputFN, backup_type_t::Exterior );
#endif
  
#if _DEBUG
  std::string queryInterior;
  std::string queryExterior;
  std::cout << "--- Interior select ---" << std::endl;
  std::cout << syntax_tree_to_prefix_form(pInteriorSelect, queryInterior) << std::endl;
  std::cout << "--- Exterior select ---" << std::endl;
  std::cout << syntax_tree_to_prefix_form(pExteriorSelect, queryExterior) << std::endl;
#endif

	VerbNode::Ptr spTree = VerbNode::BuildVerbsTree( pInteriorSelect, this->BaseDesc, this->pSettings );
	spTree->changeQuery();
	QueryResolver::cleanQuery( pInteriorSelect );

	aq_engine->call(	pInteriorSelect, minMaxGroupBy ? 0 : 1, nSelectLevel );

	solveMinMaxGroupBy.modifyTmpFiles( pSettings->szTempPath2, nSelectLevel, BaseDesc, *pSettings );

#ifdef OUTPUT_NESTED_QUERIES
	MakeBackupFile( pSettings->szOutputFN, backup_type_t::Empty );
#endif
	
	mark_as_deleted( pIntSelectAs );
  
  return query;
}

//------------------------------------------------------------------------------
//void QueryResolver::SolveSelectFromSelect(	tnode* pInteriorSelect, 
//							tnode* pExteriorSelect,
//							int nSelectLevel)
//{
//	if( !pInteriorSelect || !pExteriorSelect )
//		return;
//
//#ifdef OUTPUT_NESTED_QUERIES
//	//nQueryIdx;
//	//if( nSelectLevel < 2 )
//	//	nQueryIdx = -1;
//	std::string str;
//	syntax_tree_to_prefix_form( pExteriorSelect, str );
//	SaveFile( pSettings->szOutputFN, str.c_str() );
//  MakeBackupFile( pSettings->szOutputFN, backup_type_t::Exterior_Before );
//#endif
//
//	solveSelectStar( pInteriorSelect, BaseDesc );
//	solveOneTableInFrom( pInteriorSelect, BaseDesc );
//
//	tnode* pIntSelectAs = getLastTag( pExteriorSelect, NULL, pInteriorSelect, K_AS );
//	if( !pIntSelectAs || !pIntSelectAs->right || (pIntSelectAs->right->tag != K_IDENT) )
//		throw generic_error(generic_error::INVALID_QUERY, "");
//
//	SolveMinMaxGroupBy solveMinMaxGroupBy;
//	bool minMaxGroupBy = solveMinMaxGroupBy.checkAndClear( pInteriorSelect );
//
//	changeTableNames( pIntSelectAs, pInteriorSelect, pExteriorSelect );
//	changeColumnNames( pIntSelectAs, pInteriorSelect, pExteriorSelect->left, true );
//	changeColumnNames( pIntSelectAs, pInteriorSelect, pExteriorSelect->next, false );
//	solveSelectStarExterior( pInteriorSelect, pExteriorSelect );
//	if( trivialSelectFromSelect(pInteriorSelect) )
//	{
//		syntax_tree_to_prefix_form( pInteriorSelect, str );
//		SaveFile( pSettings->szOutputFN, str.c_str() );
//		MakeBackupFile( pSettings->szOutputFN, backup_type_t::Empty );
//		mark_as_deleted( pIntSelectAs );
//		return;
//	}
//	eliminateAliases( pInteriorSelect );
//
//	//copy conditions from inner select WHERE into outer select WHERE
//	tnode* intWhereNode = find_main_node( pInteriorSelect, K_WHERE );
//	if( intWhereNode )
//	{
//		addConditionsToWhere( intWhereNode->left, pExteriorSelect );
//	}
//	addInnerOuterNodes( intWhereNode->left, K_INNER, K_INNER );
//
//#ifdef OUTPUT_NESTED_QUERIES
//	syntax_tree_to_prefix_form( pExteriorSelect, str );
//	SaveFile( pSettings->szOutputFN, str.c_str() );
//	MakeBackupFile( pSettings->szOutputFN, backup_type_t::Exterior );
//#endif
//
//	VerbNode::Ptr spTree = QueryResolver::BuildVerbsTree( pInteriorSelect, this->BaseDesc, this->pSettings );
//	spTree->changeQuery();
//	QueryResolver::cleanQuery( pInteriorSelect );
//
//	aq_engine->call(	pInteriorSelect, minMaxGroupBy ? 0 : 1, nSelectLevel );
//
//	solveMinMaxGroupBy.modifyTmpFiles(	pSettings->szTempPath2, nSelectLevel, 
//										BaseDesc, *pSettings );
//
//#ifdef OUTPUT_NESTED_QUERIES
//	MakeBackupFile( pSettings->szOutputFN, backup_type_t::Empty );
//#endif
//	
//	mark_as_deleted( pIntSelectAs );
//}

//------------------------------------------------------------------------------
//solve all selects found in the main select
void QueryResolver::SolveSelectRecursive(	tnode*& pNode, unsigned int nSelectLevel, tnode* pLastSelect, bool inFrom, bool inIn )
{
	if( pNode == NULL )
		return;
	if( pNode->tag == K_DELETED )
		return;
	tnode* pNewLastSelect = pLastSelect;
	bool newInFrom = inFrom;
	bool newInIn = inIn;
	switch( pNode->tag )
	{
	case K_SELECT:
		pNewLastSelect = pNode;
		newInFrom = false;
		newInIn = false;
		break;
	case K_FROM:
		newInFrom = true;
		break;
	case K_IN:
		newInIn = true;
		break;
	default:;
	}

  if ((pNode->tag == K_SELECT) && (nSelectLevel > this->level))
  {
    if( inFrom )
    {
      // throw generic_error(generic_error::NOT_IMPLEMENED, "'SELECT FROM SELECT' are not implemented yet");
      // There is two type of nested query in FROM that can be solve in two way:
      //   - Fold Up => pNode will be modified.
      //   - Temporary Table => create a table used to perform join on it, this is a complex task.
      std::string alias;
      if ((pNode->parent != NULL) && (pNode->parent->tag == K_AS) && (pNode->parent->right != NULL) && (pNode->parent->right->eNodeDataType == NODE_DATA_STRING))
      {
        alias = pNode->parent->right->data.val_str;
      }
      this->nestedTables.insert(std::make_pair(alias, this->SolveSelectFromSelect(pNode, pLastSelect, nSelectLevel)));
    }
    else if ( inIn )
    {
      // Resolve SubQuery
      QueryResolver queryResolver(pNode, this->pSettings, this->aq_engine, this->BaseDesc, nSelectLevel, ++this->nestedId);
      queryResolver.SolveSQLStatement();
      Table::Ptr table = queryResolver.getResult();

      //delete old subtree and add new subtree containing answer
      delete_subtree( pNode );
      if( inIn )
      {
        pNode = new_node( K_IN_VALUES );
        pNode->left = GetTree( *table );
      }
      else
        pNode = GetTree( *table );
    }
    else
    {
      throw aq::generic_error(aq::generic_error::INVALID_QUERY, "SELECT must be in FROM or WHERE clause");
    }
  }
  else
  {
    if ( pNode->tag == K_SELECT)
      ++nSelectLevel;

    SolveSelectRecursive( pNode->left, nSelectLevel, pNewLastSelect, newInFrom, newInIn );
    if( !pNode ) 
      return; // FIXME: why it is needed ?

    SolveSelectRecursive( pNode->right, nSelectLevel, pNewLastSelect, newInFrom, newInIn );
    if( !pNode ) 
      return; // FIXME: why it is needed ?

    if( pNode->tag == K_FROM )
      newInFrom = false;

    SolveSelectRecursive( pNode->next, nSelectLevel, pNewLastSelect, newInFrom, newInIn );
    if( !pNode ) 
      return; // FIXME: why it is needed ?

    if( (pNode->tag == K_IN) && (pNode->right == NULL) )
    {
      /* If subquery evaluates to an empty set, IN evaluates to FALSE. */
      delete_subtree( pNode );
      pNode = new_node( K_FALSE );
    }
  }

}

//-------------------------------------------------------------------------------
void QueryResolver::FileWriteEnreg( aq::ColumnType col_type, const int col_size, char *my_field, FILE *fcol )
{
	int dum_int;
	int * my_int = & dum_int;

	double dum_double; // 2009/09/01 
	double * my_double = &dum_double; // 2009/09/01 

	long long int dum_long_long;
	long long int *my_long_long = &dum_long_long;

	if ( (int) strlen ( my_field ) >= col_size ) my_field[ col_size ] = 0 ;

	switch (  col_type )
	{
	case COL_TYPE_INT :
		if ( strcmp ( my_field, "NULL")  ==   0 )  *my_int = 'NULL'; // ****
		else  *my_int = atoi ( my_field );
		fwrite( my_int , sizeof(int), 1, fcol  );
		break;

	case COL_TYPE_BIG_INT :
		if ( strcmp ( my_field, "NULL")  ==   0 )  *my_long_long  = 'NULL'; // ****  
#ifdef WIN32
		else  *my_long_long  = _atoi64 (my_field );   
#else
		else  *my_long_long  = atoll (my_field );   
#endif
		fwrite( my_long_long , sizeof(long long), 1, fcol );
		break;

	case COL_TYPE_DOUBLE :
		if (  strcmp ( my_field, "NULL")  ==   0 )  *my_double = 'NULL'; // ****  
		else
		{
			// step 1 convert ',' in '.'
			ChangeCommaToDot (  my_field );
			// step 2 : use strtod
			*my_double =     strtod ( my_field, NULL );  // atof  ( field );
		}
		fwrite( my_double, sizeof(double), 1, fcol );
		break;

	case COL_TYPE_DATE1 :
	case COL_TYPE_DATE2 :
	case COL_TYPE_DATE3 :
		{
			char *dateBuf;
			DateType dateType;
			switch( col_type )
			{
			case COL_TYPE_DATE1 :
				dateBuf = "DD/MM/YYYY HH:MM:SS";
				dateType = DDMMYYYY_HHMMSS;
				break;
			case COL_TYPE_DATE2 :
				dateBuf = "DD/MM/YYYY";
				dateType = DDMMYYYY;
				break;
			case COL_TYPE_DATE3 :
				dateBuf = "DD/MM/YY";
				dateType = DDMMYY;
				break;
			default:
				throw generic_error(generic_error::TYPE_MISMATCH, "");
			}
			if ( (strcmp ( my_field, "NULL")  ==   0) ||	(strcmp( my_field, "" ) == 0) )  *my_long_long  = 'NULL'; // ****
			else
			{
				if ( dateToBigInt(my_field, dateType, my_long_long) )
				{
					fwrite( my_long_long , sizeof(long long), 1, fcol );
				}
				else
				{
					//sprintf ( a_message, "Champ DATE invalide. Format attendu: %s. Champ: %s.", dateBuf, my_field );
					throw generic_error(generic_error::TYPE_MISMATCH, "");
				}
			}
		}
		break;

	case COL_TYPE_VARCHAR :
		// check my_field size
		if ( (int) strlen ( my_field ) >= col_size ) my_field[ col_size ] = 0 ;
		// clean all space at the end
		CleanSpaceAtEnd (my_field );
		// write string record and go to next
		fwrite(my_field, sizeof(char), strlen( my_field ) , fcol );
		fwrite("\0",sizeof(char),1, fcol );
		break;

	default:
		throw generic_error(generic_error::TYPE_MISMATCH, "");
		break;
	}
}

//------------------------------------------------------------------------------
void QueryResolver::SolveInsertAux(	Table& table, size_t tableIdx, size_t colIdx, size_t packNumber,
						vector<size_t>& reverseValuePos,
						Column& nullColumn, Table& valuesToInsert, size_t startIdx, 
						size_t endIdx, bool append )
{
	sprintf( szBuffer, "%s%sB001T%.4uC%.4uP%.12u", pSettings->szRootPath.c_str(), 
		"data_orga\\columns\\", table.ID, table.Columns[colIdx]->ID, packNumber );
	if( reverseValuePos[colIdx] < 0 )
		nullColumn.saveToFile( szBuffer, startIdx, endIdx, append );
	else
	{
		Column& col = *valuesToInsert.Columns[reverseValuePos[colIdx]];
		col.saveToFile( szBuffer, startIdx, endIdx, append );
	}

	sprintf( szBuffer, "\"%s %s %u %u %u\"\n", 
		pSettings->szLoaderPath,pSettings->iniFile.c_str(), tableIdx + 1, colIdx + 1, packNumber );
	system ( szBuffer );
}

//------------------------------------------------------------------------------
void QueryResolver::SolveInsert(	tnode* pNode )
{
	if( !pNode || pNode->tag != K_INSERT )
		return;
	size_t tableIdx = BaseDesc.getTableIdx( pNode->left->data.val_str );
	vector<tnode*> columns;
	commaListToNodeArray( pNode->right->left, columns );
	reverse( columns.begin(), columns.end() );
	if( columns.size() == 0 )
		throw generic_error(generic_error::GENERIC, "");

	vector<int> valuePos;
	Table& table = BaseDesc.Tables[tableIdx];
	getColumnsIds( table, columns, valuePos );
	
	Table::Ptr valuesToInsert;
	if( pNode->right->right->tag == K_SELECT )
	{
		QueryResolver queryResolver(pNode->right->right, this->pSettings, this->aq_engine, this->BaseDesc, true);
		queryResolver.SolveSQLStatement();
		valuesToInsert = queryResolver.getResult();
		//check that the result is compatible with the insert columns
		if( (valuesToInsert->Columns.size() - 1) != valuePos.size() )
			throw generic_error(generic_error::INVALID_QUERY, "");
		for( size_t idx = 0; idx < valuesToInsert->Columns.size()-1; ++idx )
			if( !compatibleTypes(	valuesToInsert->Columns[idx]->Type,
									table.Columns[valuePos[idx]]->Type) )
				throw generic_error(generic_error::INVALID_QUERY, "");
	}
	else
	{
		vector<tnode*> values;
		commaListToNodeArray( pNode->right->right, values );
		reverse( values.begin(), values.end() );
		if( columns.size() != values.size() )
			throw generic_error(generic_error::INVALID_QUERY, "");
		//create table from values
		valuesToInsert = new Table();
		for( size_t idx = 0; idx < values.size(); ++idx )
		{
			Column::Ptr column = new Column( *table.Columns[valuePos[idx]] );
			switch( column->Type )
			{
			case COL_TYPE_INT:
			case COL_TYPE_BIG_INT:
			case COL_TYPE_DATE1:
			case COL_TYPE_DATE2:
			case COL_TYPE_DATE3:
			case COL_TYPE_DOUBLE:
				column->Items.push_back( new ColumnItem(static_cast<double>(pNode->data.val_int)) );
				break;
			case COL_TYPE_VARCHAR:
				column->Items.push_back( new ColumnItem(pNode->data.val_str) );
				break;
			}
			valuesToInsert->Columns.push_back( column );
		}
	}

	int newRowsNr = (int) valuesToInsert->TotalCount;
	int nrColumns = (int) table.Columns.size();
	//write new base struct
	table.TotalCount += newRowsNr;
	BaseDesc.saveToRawFile( pSettings->szDBDescFN );

	//write rows to table file
	vector<size_t> reverseValuePos;
	reverseValuePos.resize( nrColumns, -1 );
	for( size_t idx = 0; idx < valuePos.size(); ++idx )
		reverseValuePos[valuePos[idx]] = idx;

	std::string tablePath = pSettings->szRootPath;
	tablePath += "data_orga\\tables\\" + table.getOriginalName() + ".txt";
	FILE* file = fopen(tablePath.c_str(), "a");
	if( !file )
		throw generic_error(generic_error::COULD_NOT_OPEN_FILE, "");
	
	for( int idx = 0; idx < (int) valuesToInsert->Columns[0]->Items.size(); ++idx )
	{
		int nrRepetitions = 1;
		if( valuesToInsert->HasCount )
			nrRepetitions = (int) valuesToInsert->Columns[nrColumns - 1]->Items[idx]->numval;
		for( int countIdx = 0; countIdx < nrRepetitions; ++countIdx )
		{
			std::string row;
			for( int idx2 = 0; idx2 < nrColumns; ++idx2 )
			{
				if( pSettings->csvFormat )
					row += "\"";
				//if( reverseValuePos[idx2] < 0  ) // this is not possible
				//	row += "NULL";
				else
				{
					Column& column = *valuesToInsert->Columns[reverseValuePos[idx2]];
					column.Items[idx]->toString( szBuffer, column.Type );
					row += szBuffer;
				}
				if( pSettings->csvFormat )
					row += "\"";
				if( idx2 < nrColumns - 1)
					if( pSettings->csvFormat )
						row += ",";
					else
						row += ";";
			}
			row += "\n";
			fputs( row.c_str(), file );
		}
	}
	fclose( file );

	//write values to column values and call cut_in_col
	int lastOldPackSize = (table.TotalCount - newRowsNr) % pSettings->packSize;
	int firstPackSize = pSettings->packSize - lastOldPackSize;
	if( newRowsNr < firstPackSize )
		firstPackSize = newRowsNr;
	int packNumber = (int) (table.TotalCount - newRowsNr) / pSettings->packSize;
	if( firstPackSize > 0 )
	{
		Column nullColumn;
		nullColumn.Items.resize(firstPackSize, NULL);
		for( int idx = 0; idx < nrColumns; ++idx )
			SolveInsertAux( table, tableIdx, idx, packNumber, reverseValuePos, nullColumn, 
				*valuesToInsert, 0, firstPackSize, true );
		++packNumber;
	}
	
	int leftOverRows = (int) newRowsNr - firstPackSize;
	int nrPacks = leftOverRows / pSettings->packSize;
	if( leftOverRows % pSettings->packSize > 0 )
		++nrPacks;
	Column nullColumn;
	nullColumn.Items.resize(pSettings->packSize, NULL);
	for( int idx = 0; idx < nrPacks; ++idx )
	{
		int firstIdx = firstPackSize + idx * pSettings->packSize;
		int endIdx = firstIdx + pSettings->packSize;
		if( idx == nrPacks - 1 )
		{
			int lastPackSize = table.TotalCount % pSettings->packSize;
			nullColumn.Items.resize(lastPackSize, NULL);
			endIdx = firstIdx + lastPackSize;
		}
		for( int idx2 = 0; idx2 < nrColumns; ++idx2 )
			SolveInsertAux( table, tableIdx, idx2, packNumber + idx, reverseValuePos, 
				nullColumn, *valuesToInsert, firstIdx, endIdx, false );
	}
}

//------------------------------------------------------------------------------
void QueryResolver::SolveUpdateDelete(	tnode* pNode )
{
	size_t tableIdx = BaseDesc.getTableIdx( pNode->left->data.val_str );
	vector<Column::Ptr>& columns = BaseDesc.Tables[tableIdx].Columns;
	Table& table = BaseDesc.Tables[tableIdx];

	std::string tablePath = pSettings->szRootPath;
	tablePath += "data_orga\\tables\\" + table.getOriginalName() + ".txt";
	FILE* fOldTable = fopenUTF8( tablePath.c_str(), "rt" );
	if( fOldTable == NULL )
		throw generic_error(generic_error::COULD_NOT_OPEN_FILE, "");

	std::string newTablePath = tablePath + "a"; //form unique table name
	FILE* fNewTable = fopen( newTablePath.c_str(), "wt" );
	if( fNewTable == NULL )
		throw generic_error(generic_error::COULD_NOT_OPEN_FILE, "");

	vector<tnode*> updates;
	vector<size_t> updateToTableMap;
	vector<size_t> tableToUpdateMap;
	if( pNode->tag == K_UPDATE )
	{
		commaListToNodeArray( pNode->right->left, updates );
		updateToTableMap.resize( updates.size(), -1 );
		tableToUpdateMap.resize( columns.size(), -1 );

		for( size_t idx = 0; idx < updates.size(); ++idx )
		{
			std::string str(updates[idx]->left->data.val_str);
			strtoupr( str );
			for( size_t idx2 = 0; idx2 < columns.size(); ++idx2 )
				if( str == columns[idx2]->getName() )
				{
					updateToTableMap[idx] = idx2;
					tableToUpdateMap[idx2] = idx;

					break;
				}
				if( updateToTableMap[idx] < 0 )
					throw generic_error( generic_error::INVALID_QUERY, "" );
		}
	}
	else
	{	
		for( size_t idx = 0; idx < columns.size(); ++idx )
		{
			updates.push_back( NULL );
			updateToTableMap.push_back( idx );
			tableToUpdateMap.push_back( idx );
		}
	}

	char* myRecord = new char[pSettings->maxRecordSize];
	boost::scoped_array<char> myRecordDel( myRecord );

	Table::Ptr condTable = new Table();
	tnode* conditionsRoot;
	if( pNode->tag == K_UPDATE )
		conditionsRoot = pNode->right->right;
	else
		conditionsRoot = pNode->right;
	if( conditionsRoot->tag == K_IN )
	{
		vector<int> valuePos;
		vector<tnode*> conditions;
		commaListToNodeArray( conditionsRoot->left, conditions );
		reverse( conditions.begin(), conditions.end() );
		getColumnsIds( table, conditions, valuePos );

		assert( conditionsRoot->right->tag == K_SELECT );
		
		QueryResolver queryResolver(pNode->right->right, this->pSettings, this->aq_engine, this->BaseDesc, true);
		queryResolver.SolveSQLStatement();
		condTable = queryResolver.getResult();
		if( !condTable || condTable->Columns.size() == 0 )
			return; //no conditions
		//check that the result is compatible with the columns
		if( condTable->HasCount )
			condTable->Columns.pop_back();
		if( (condTable->Columns.size()) != valuePos.size() )
			throw generic_error(generic_error::INVALID_QUERY, "");
		for( size_t idx = 0; idx < condTable->Columns.size(); ++idx )
		{
			Column& col = *table.Columns[valuePos[idx]];
			if( !compatibleTypes(	condTable->Columns[idx]->Type,
									col.Type) )
				throw generic_error(generic_error::INVALID_QUERY, "");
			condTable->Columns[idx]->Type = col.Type;
			condTable->Columns[idx]->ID = col.ID;
		}
	}
	else
	{
		vector<tnode*> conditions;
		commaListToNodeArray( conditionsRoot, conditions );
		for( size_t idx = 0; idx < conditions.size(); ++idx )
		{
			std::string str(conditions[idx]->left->data.val_str);
			strtoupr( str );
			bool found = false;
			for( size_t idx2 = 0; idx2 < columns.size(); ++idx2 )
				if( str == columns[idx2]->getName() )
				{
					Column::Ptr newCol = new Column(*columns[idx2]);

					switch( newCol->Type )
					{
					case COL_TYPE_INT:
					case COL_TYPE_BIG_INT:
					case COL_TYPE_DATE1:
					case COL_TYPE_DATE2:
					case COL_TYPE_DATE3:
					case COL_TYPE_DOUBLE:
						newCol->Items.push_back( new ColumnItem(static_cast<double>(conditions[idx]->right->data.val_int)) );
						break;
					case COL_TYPE_VARCHAR:
						newCol->Items.push_back( new ColumnItem(conditions[idx]->right->data.val_str) );
						break;
					}

					condTable->Columns.push_back( newCol );
					found = true;
					break;
				}
			if( !found )
				throw generic_error( generic_error::INVALID_QUERY, "" );
		}
	}

	llong nrOfPacks = table.TotalCount / pSettings->packSize + 1;
	if( table.TotalCount % pSettings->packSize == 0 )
		++nrOfPacks;
	for( size_t packNr = 0; packNr < (size_t) nrOfPacks; ++packNr )
	{
		vector<bool> updatedRows;
		llong nrItems = pSettings->packSize;
		if( packNr == nrOfPacks - 1 )
		{
			nrItems = BaseDesc.Tables[tableIdx].TotalCount % pSettings->packSize;
			if( nrItems == 0 )
				nrItems = pSettings->packSize;
		}
		updatedRows.resize( (size_t) nrItems, true );
		
		for( size_t idx = 0; idx < condTable->Columns.size(); ++idx )
		{
			Column::Ptr condCol = condTable->Columns[idx];
			Column::Ptr column( new Column(*condCol) );
			std::string columnPath = pSettings->szRootPath;
			sprintf( szBuffer, "B001T%.4uC%.4uP%.12u", table.ID, 
				column->ID, packNr );
			columnPath += "data_orga\\columns\\";
			columnPath += szBuffer;
			column->loadFromFile( columnPath );
			for( size_t idx2 = 0; idx2 < column->Items.size(); ++idx2 )
			{
				bool found = false;
				for( size_t idx3 = 0; idx3 < condCol->Items.size(); ++idx3 )
					if( equal(	column->Items[idx2].get(), 
								condCol->Items[idx3].get(), 
								column->Type) )
					{
						found = true;
						break;
					}
				if( !found )
					updatedRows[idx2] = false;
			}
		}
		bool atLeastOneUpdate = false;
		for( size_t idx = 0; idx < updatedRows.size(); ++idx )
			if( updatedRows[idx] )
			{
				atLeastOneUpdate = true;
				break;
			}

		if( atLeastOneUpdate )
		{
			for( size_t idx = 0; idx < updates.size(); ++idx )
			{
				Column::Ptr tColumn = table.Columns[updateToTableMap[idx]];
				Column::Ptr column( new Column(*tColumn) );
				std::string columnPath = pSettings->szRootPath;
				sprintf( szBuffer, "B001T%.4uC%.4uP%.12u", table.ID, 
					tColumn->ID, packNr );
				columnPath += "data_orga\\columns\\";
				columnPath += szBuffer;
				tnode* pValNode = NULL;
				if( pNode->tag == K_UPDATE )
					pValNode = updates[idx]->right;
				column->loadFromFile( columnPath );
				for( size_t idx2 = 0; idx2 < updatedRows.size(); ++idx2 )
					if( updatedRows[idx2] )
						if( pValNode )
							switch( pValNode->eNodeDataType )
						{
							case NODE_DATA_INT:
								column->Items[idx2]->numval = (double) pValNode->data.val_int;
								break;
							case NODE_DATA_NUMBER:
								column->Items[idx2]->numval = pValNode->data.val_number;
								break;
							case NODE_DATA_STRING:
								column->Items[idx2]->strval = pValNode->data.val_str;
								break;
							default:
								assert( 0 );
						}
						else
							column->Items[idx2] = NULL;
				column->saveToFile( columnPath );
				sprintf( szBuffer, "\"%s %s %d %d %u\"\n", 
					pSettings->szLoaderPath, pSettings->iniFile.c_str(), tableIdx + 1, 
					updateToTableMap[idx] + 1, packNr );
				system ( szBuffer );
			}
		}

		for( size_t idx = 0; idx < updatedRows.size(); ++idx )
			if( updatedRows[idx] )
			{
				fgets( myRecord, pSettings->maxRecordSize, fOldTable );
				vector<char*> fields;
				splitLine( myRecord, pSettings->fieldSeparator, fields, true );
				std::string strval;
				for( size_t idx2 = 0; idx2 < fields.size(); ++idx2 )
				{
					if( pNode->tag == K_DELETE )
						fwrite( "NULL", sizeof(char), strlen("NULL"), fNewTable );
					else if( tableToUpdateMap[idx2] < 0 )
						fwrite( fields[idx2], sizeof(char), strlen(fields[idx2]), fNewTable );
					else
					{
						std::string str = to_string(updates[tableToUpdateMap[idx2]]->right);
						fwrite( str.c_str(), sizeof(char), str.size(), fNewTable );
					}
					if( idx2 + 1 < fields.size() )
						fputc( ';', fNewTable );
				}
				fputc( '\n', fNewTable );
			}
			else
			{
				fgets( myRecord, pSettings->maxRecordSize, fOldTable );
				fputs( myRecord, fNewTable );				
			}
	}
	fclose( fOldTable );
	fclose( fNewTable );

	remove( tablePath.c_str() );
	rename( newTablePath.c_str(), tablePath.c_str() );
}

//------------------------------------------------------------------------------
void QueryResolver::SolveUnionMinus(	tnode* pNode )
{
	vector<tnode*> queries;
	vector<int> operation;
	int lastTag = -1;
	while( pNode->tag == K_UNION || pNode->tag == K_SEL_MINUS )
	{
		addUnionMinusNode( pNode->tag, queries, operation, pNode->right );
		lastTag = pNode->tag;
		pNode = pNode->left;
	}
	addUnionMinusNode( lastTag, queries, operation, pNode );
	reverse( queries.begin(), queries.end() );
	reverse( operation.begin(), operation.end() );
	Table::Ptr totalTable = NULL;
	vector<size_t> deletedRows;
	for( size_t idx = 0; idx < queries.size(); ++idx )
	{
		QueryResolver queryResolver(queries[idx], this->pSettings, this->aq_engine, this->BaseDesc, true);
		queryResolver.SolveSQLStatement();
		Table::Ptr table = queryResolver.getResult();
		if( table->Columns.size() == 0 )
			throw generic_error(generic_error::INVALID_TABLE, "");
		if( totalTable )
		{
			//check for compatibility
			if( totalTable->Columns.size() != table->Columns.size() )
				throw generic_error(generic_error::INVALID_TABLE, "");
			for( size_t idx2 = 0; idx2 < table->Columns.size(); ++idx2 )
				if( !compatibleTypes( totalTable->Columns[idx2]->Type,
					table->Columns[idx2]->Type) )
					throw generic_error(generic_error::INVALID_TABLE, "");
			size_t totalTableSize = totalTable->Columns[0]->Items.size();
			if( operation[idx] == 1 ) //UNION ALL
				totalTable->TotalCount += table->TotalCount;
			for( size_t idx2 = 0; idx2 < table->Columns[0]->Items.size(); ++idx2 )
			{
				if( operation[idx] == 1 ) //UNION ALL
				{
					for( size_t idx3 = 0; idx3 < table->Columns.size(); ++idx3 )
						totalTable->Columns[idx3]->Items.push_back( table->Columns[idx3]->Items[idx2] );
				}
				else
				{
					size_t nrColumns = table->Columns.size();
					if( table->HasCount )
						--nrColumns;
					bool found = false;
					for( size_t idx4 = 0; idx4 < totalTableSize; ++idx4 )
					{
						bool allEqual = true;
						for( size_t idx3 = 0; idx3 < nrColumns; ++idx3 )
							if( !equal(	table->Columns[idx3]->Items[idx2].get(),
										totalTable->Columns[idx3]->Items[idx4].get(), 
										table->Columns[idx3]->Type ) )
							{
								allEqual = false;
								break;
							}
						if( allEqual )
						{
							if( operation[idx] == 0 )
							{
								found = true;
								break;
							}
							else
							{
								//MINUS
								deletedRows.push_back( idx4 );
								for( size_t idx3 = 0; idx3 < nrColumns; ++idx3 )
                  totalTable->Columns[idx3]->Items[idx4] = NULL;
							}
						}
					}
					if( (operation[idx] == 0) && !found ) //UNION
					{
						if( table->HasCount )
						{
							size_t countCol = table->Columns.size()-1;
							Column& count = *table->Columns[countCol];
							ColumnItem& item = *count.Items[idx2];
							totalTable->TotalCount += (llong) item.numval;
						}
						else
							totalTable->TotalCount += 1;
						for( size_t idx3 = 0; idx3 < table->Columns.size(); ++idx3 )
						{
							ColumnItem::Ptr item = table->Columns[idx3]->Items[idx2];
							totalTable->Columns[idx3]->Items.push_back( item );
						}
					}
				}
			}
			totalTable->groupBy();
		}
		else
			totalTable = table;
	}
	totalTable->saveToAnswer(	pSettings->szAnswerFN, pSettings->fieldSeparator,
								deletedRows );
};

//------------------------------------------------------------------------------
void QueryResolver::SolveTruncate(	tnode* pNode )
{
	size_t tableIdx = -1;
	Table::Ptr table = new Table();
	table->setName( pNode->left->data.val_str );
	bool found = true;
	for( size_t idx = 0; idx < BaseDesc.Tables.size(); ++idx )
		if( table->getName() == BaseDesc.Tables[idx].getName() )
			tableIdx = idx;
	if( tableIdx < 0 )
		throw generic_error(generic_error::INVALID_TABLE, "");

	BaseDesc.Tables[tableIdx].TotalCount = 0;

	//write to disk
	BaseDesc.saveToRawFile( pSettings->szDBDescFN );

	//delete table related files?
	std::string tablePath = pSettings->szRootPath;
	tablePath += "data_orga\\tables\\" + table->getOriginalName() + ".txt";
	//..
}

//------------------------------------------------------------------------------
void QueryResolver::SolveCreate(	tnode* pNode )
{
	Table::Ptr table = new Table();
	table->setName( pNode->left->data.val_str );
	for( size_t idx = 0; idx < BaseDesc.Tables.size(); ++idx )
		if( table->getName() == BaseDesc.Tables[idx].getName() )
			throw generic_error(generic_error::TABLE_ALREADY_EXISTS, "");

	QueryResolver queryResolver( pNode->right, this->pSettings, this->aq_engine, this->BaseDesc, true);
	queryResolver.SolveSQLStatement();
	table = queryResolver.getResult();
	assert( table );
	size_t ID = 0;
	if( BaseDesc.Tables.size() > 0 )
	{
		ID = BaseDesc.Tables[0].ID;
		for( size_t idx = 1; idx < BaseDesc.Tables.size(); ++idx )
			ID = max(ID, (size_t) BaseDesc.Tables[idx].ID);
	}
	++ID;
	table->ID = ID;
	table->setName( pNode->left->data.val_str );
	BaseDesc.Tables.push_back( *table );
	//write to disk
	BaseDesc.saveToRawFile( pSettings->szDBDescFN );
	std::string tablePath = pSettings->szRootPath;
	tablePath += "data_orga\\tables\\" + table->getOriginalName() + ".txt";
	table->saveToAnswer( tablePath.c_str(), pSettings->fieldSeparator, false );
	//apply cut in col on the new columns
	size_t nrColumns = table->Columns.size();
	if( table->HasCount )
		--nrColumns;
	for( size_t idx = 0; idx < nrColumns; ++idx )
	{
    // FIXME
		//sprintf( szBuffer, "cmd /s /c \"%s %s \"%u\" \"%u\"\"", pSettings->szCutInColPath, pSettings->iniFile.c_str(), BaseDesc.Tables.size(), idx + 1 ); //debug13 - not portable
		//system( szBuffer );
	}
}

//------------------------------------------------------------------------------
int QueryResolver::SolveSQLStatement()
{
  aq::generate_parent(this->sqlStatement, NULL);
	tnode *pNode = this->sqlStatement;
	if( !pNode )
		throw generic_error(generic_error::INVALID_QUERY, "");

	switch( pNode->tag )
	{
	case K_SELECT:
		{
#ifdef _DEBUG
			std::string query;
			// syntax_tree_to_sql_form(this->sqlStatement, query);
			std::cerr << query << std::endl;
#endif
			this->result = SolveSelect();
		}
		break;
	case K_CREATE:
		{
			SolveCreate( pNode );
		}
		break;
	case K_INSERT:
		{
			SolveInsert( pNode );
		}
		break;
	case K_DELETE:
	case K_UPDATE:
		{
			SolveUpdateDelete( pNode );
		}
		break;
	case K_TRUNCATE:
		{
			SolveTruncate( pNode );
		}
		break;
	case K_SEL_MINUS:
	case K_UNION:
		{
			SolveUnionMinus( pNode );
		}
		break;
	default:
		throw generic_error(generic_error::NOT_IMPLEMENED, "");
	}
	return 0; //debug13 should throw instead of returning
}

//------------------------------------------------------------------------------
Table::Ptr QueryResolver::solveAQMatriceByRows(VerbNode::Ptr spTree)
{	
	assert(pSettings->useRowResolver);
	aq::Timer timer;

	// Prepare Columns
	std::vector<Column::Ptr> columnTypes;
	this->getColumnTypes( this->sqlStatement, columnTypes, this->BaseDesc );
	
	std::map<size_t, std::vector<std::pair<size_t, aq::ColumnType> > > columnsByTableId;
	for (std::vector<Column::Ptr>::const_iterator it = columnTypes.begin(); it != columnTypes.end(); ++it)
	{
		(*it)->TableID = BaseDesc.getTableIdx((*it)->getTableName());
		(*it)->TableID = BaseDesc.Tables[(*it)->TableID].ID;
		columnsByTableId[(*it)->TableID].push_back(std::make_pair((*it)->ID, (*it)->Type));
	}

	std::vector<aq::ColumnMapper::Ptr> columnsMapper;
	for (std::map<size_t, std::vector<std::pair<size_t, aq::ColumnType> > >::const_iterator it_t = columnsByTableId.begin(); it_t != columnsByTableId.end(); ++it_t)
	{
		for (std::vector<std::pair<size_t, aq::ColumnType> >::const_iterator it_c = it_t->second.begin(); it_c != it_t->second.end(); ++it_c)
		{
			ColumnMapper::Ptr cm(new ColumnMapper(aq::ColumnMapper(pSettings->szThesaurusPath, it_t->first, it_c->first, it_c->second, pSettings->packSize)));
			columnsMapper.push_back(cm);
		}
	}

#ifdef OUTPUT_NESTED_QUERIES
	MakeBackupFile( pSettings->szOutputFN, backup_type_t::Empty );
	MakeBackupFile( pSettings->szAnswerFN, backup_type_t::Before );
#endif

  boost::shared_ptr<aq::RowProcesses> processes(new aq::RowProcesses);

	if (this->hasGroupBy)
	{
		std::vector<Column::Ptr> columnGroupBy;
		tnode * nodeGroup = find_main_node(this->sqlStatement, K_GROUP);
		std::vector<tnode**> columnNodes;
		getAllColumnNodes(nodeGroup, columnNodes);

		// aq_engine->getAQMatrix()->groupBy(columnsMapper); // FIXME : the group by must be ordered according to the group by clause
	}

  boost::shared_ptr<aq::RowVerbProcess> rowVerbProcess(new aq::RowVerbProcess(spTree));
	processes->addProcess(rowVerbProcess);

	boost::shared_ptr<aq::RowWritter> rowWritter(new aq::RowWritter(pSettings->output == "stdout" ? pSettings->output : pSettings->szAnswerFN));
	rowWritter->setColumn(columnTypes); // fixme
  processes->addProcess(rowWritter);

	timer.start();
	Table::Ptr table = new Table();
	table->loadFromTableAnswerByRow(*(aq_engine->getAQMatrix()), aq_engine->getTablesIDs(), columnsMapper, columnTypes, *pSettings, BaseDesc, processes );
	aq::Logger::getInstance().log(AQ_INFO, "Load From Answer: Time Elapsed = %s\n", aq::Timer::getString(timer.getTimeElapsed()).c_str());
	return Table::Ptr(); // empty
}

//------------------------------------------------------------------------------
Table::Ptr QueryResolver::solveAQMatriceByColumns(VerbNode::Ptr spTree)
{
	aq::Timer timer;

	// get select columns from query
	vector<Column::Ptr> columnTypes;
	this->getColumnTypes( this->sqlStatement, columnTypes, this->BaseDesc );
	
	//
	// result
	Table::Ptr table = new Table();

	timer.start();
	table->loadFromTableAnswerByColumn(*(aq_engine->getAQMatrix()), aq_engine->getTablesIDs(), columnTypes, *pSettings, BaseDesc );
	aq::Logger::getInstance().log(AQ_INFO, "Load From Answer: Time Elapsed = %s\n", aq::Timer::getString(timer.getTimeElapsed()).c_str());

#ifdef OUTPUT_NESTED_QUERIES
	MakeBackupFile( pSettings->szOutputFN, backup_type_t::Empty );
	MakeBackupFile( pSettings->szAnswerFN, backup_type_t::Before );
#endif

	if( !table->NoAnswer )
	{
		timer.start();
		spTree->changeResult( table );
		aq::Logger::getInstance().log(AQ_INFO, "Change Result: Time Elapsed = %s\n", aq::Timer::getString(timer.getTimeElapsed()).c_str());
	}
	
	timer.start();
	table->cleanRedundantColumns();
	table->groupBy();
	aq::Logger::getInstance().log(AQ_INFO, "Group By: Time Elapsed = %s\n", aq::Timer::getString(timer.getTimeElapsed()).c_str());

	if( table->GroupByApplied && table->HasCount )
	{
		timer.start();
		Column::Ptr count = table->Columns[table->Columns.size() - 1];
		for( size_t idx = 0; idx < count->Items.size(); ++idx )
			count->Items[idx]->numval = 1;
		table->TotalCount = count->Items.size();
		aq::Logger::getInstance().log(AQ_INFO, "Change Count: Time Elapsed = %s\n", aq::Timer::getString(timer.getTimeElapsed()).c_str());
	}

	this->result = table;
	return table;
}


//------------------------------------------------------------------------------
//Table::Ptr QueryResolver::solveAQMatriceV2(VerbNode::Ptr spTree)
//{
//
//	aq::Logger::getInstance().log(AQ_INFO, "processing answer file '%s'\n", pSettings->szAnswerFN);
//
//	//
//	// get answer files
//	std::vector<std::string> filesStr;
//	//if (multipleAnswerFiles)
//	//{
//	//	std::vector<fs::path> files;
//	//	fs::path answerPath(answerPathStr);
//	//	if (fs::exists(answerPath) && fs::is_directory(answerPath))
//	//	{
//	//		for (fs::directory_iterator it = fs::directory_iterator(answerPath); it != fs::directory_iterator(); ++it)
//	//		{
//	//			fs::path f(*it);
//	//			std::string s(f.filename().string().substr(0, 7));
//	//			if (fs::is_regular_file(*it) && ((s == "answer_") || s == "answer."))
//	//				filesStr.push_back(f.string());
//	//		}
//	//		std::sort(filesStr.begin(), filesStr.end());
//	//	}
//	//}
//	//else
//	//{
//	filesStr.push_back(pSettings->szAnswerFN); // FIXME
//	//}
//
//	//
//	// get select columns from query
//	std::vector<Column::Ptr> columnTypes;
//	QueryResolver::getColumnTypes(this->sqlStatement, columnTypes, this->BaseDesc);
//
//	//
//	// compute answer
//	Table::Ptr table = new Table();
//	table->loadFromTableAnswerByColumn(*aq_engine->getAQMatrix(), aq_engine->getTablesIDs(), columnTypes, *pSettings, BaseDesc);
//
//	if(!table->NoAnswer)
//		spTree->changeResult(table);
//
//	table->cleanRedundantColumns();
//	table->groupBy();
//
//	return table;
//}

//------------------------------------------------------------------------------
int QueryResolver::MakeBackupFile( char *pszPath, backup_type_t type ) const
{
	char szBuffer[STR_BUF_SIZE];
	memset(szBuffer, 0, STR_BUF_SIZE);
	char		szDstPath[_MAX_PATH];
	size_t	len = 0;
	strcpy( szBuffer, pszPath );
	len = strlen(szBuffer);
	if( len < 3 )
	{
		aq::Logger::getInstance().log(AQ_DEBUG, "MakeBackupFile : Invalid filename %s !", szBuffer );
		return -1;
	}
	szBuffer[len - 4] = '\0';
	char *typeChar = "";
	switch( type )
	{
  case backup_type_t::Empty: break;
  case backup_type_t::Before: typeChar = "_Before"; break;
	case backup_type_t::After: typeChar = "_After"; break;
	case backup_type_t::Exterior_Before: typeChar = "_Exterior_Before"; break;
	case backup_type_t::Exterior: typeChar = "_Exterior"; break;
	default: ;
	}
	sprintf( szDstPath, "%s_%.2d_%.2d%s.%s", szBuffer, this->level, this->id, typeChar, &pszPath[len - 3] );
	if( FileRename( pszPath, szDstPath ) != 0 )
	{
		aq::Logger::getInstance().log(AQ_DEBUG, "MakeBackupFile : Error renaming file %s to %s !", pszPath, szDstPath );
		return -1;
	}
	return 0;
}
