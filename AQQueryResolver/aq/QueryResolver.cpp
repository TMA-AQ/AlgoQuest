#include "QueryResolver.h"
#include "SQLPrefix.h"
#include "QueryAnalyzer.h"
#include "parser/sql92_grm_tab.hpp"
#include "Base.h"
#include "Table.h"

#include "RowProcesses.h"
#include "RowWritter.h"
#include "RowVerbProcess.h"
#include "RowTemporaryWritter.h"
#include "RowSolver.h"

#include "verbs/Verb.h"
#include "parser/JeqParser.h"
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

namespace aq
{

//------------------------------------------------------------------------------
QueryResolver::QueryResolver(aq::tnode * _sqlStatement, TProjectSettings * _pSettings, AQEngine_Intf * _aq_engine, Base& _baseDesc, unsigned int& _id, unsigned int _level)
	:	sqlStatement(_sqlStatement),
		pSettings(_pSettings), 
		aq_engine(_aq_engine), 
		BaseDesc(_baseDesc), 
    nestedId(0),
    level(_level),
    id_generator(_id),
    id(_id),
    nested(id > 1)
{
  this->sqlStatement->to_upper();
	memset(szBuffer, 0, STR_BUF_SIZE);
  timer.start();
}

//------------------------------------------------------------------------------
QueryResolver::~QueryResolver()
{
	aq::Logger::getInstance().log(AQ_INFO, "Query Resolver: Time elapsed = %s\n", aq::Timer::getString(timer.getTimeElapsed()).c_str());
}

//-------------------------------------------------------------------------------
Table::Ptr QueryResolver::solve()
{
  aq::generate_parent(this->sqlStatement, NULL);
	aq::tnode *pNode = this->sqlStatement;
	if( !pNode )
		throw aq::generic_error(aq::generic_error::INVALID_QUERY, "");

	assert( this->sqlStatement->tag == K_SELECT );
	
  bool hasWhere = find_main_node(this->sqlStatement, K_WHERE) != NULL;
	this->hasGroupBy = find_main_node(this->sqlStatement, K_GROUP) != NULL;
	this->hasOrderBy = find_main_node(this->sqlStatement, K_ORDER) != NULL;
	this->hasPartitionBy = find_main_node(this->sqlStatement, K_OVER) != NULL;
  
  aq::addAlias(this->sqlStatement->left);
  if (!this->hasGroupBy && hasWhere)
  {
    std::list<tnode*> columns, aggregateColumns;
    aq::selectToList(this->sqlStatement, columns);
    aq::findAggregateFunction(columns, aggregateColumns);
    if (!aggregateColumns.empty())
    {
      // aq::addColumnsToGroupBy(this->sqlStatement, aggregateColumns);
      aq::addEmptyGroupBy(this->sqlStatement);
      this->hasGroupBy = true;
    }
  }

	this->SolveSelectRecursive( this->sqlStatement, this->level, NULL, false, false );
	this->result = this->SolveSelectRegular();

  return this->result;
}

//-------------------------------------------------------------------------------
Table::Ptr QueryResolver::SolveSelectRegular()
{
  aq::Logger::getInstance().log(AQ_DEBUG, "solve query\n");
	Table::Ptr table;

  this->changeTemporaryTableName(this->sqlStatement);

#ifdef OUTPUT_NESTED_QUERIES
	std::string str;
	aq::syntax_tree_to_prefix_form( this->sqlStatement, str );
	
	aq::SaveFile( pSettings->szOutputFN, str.c_str() );
	aq::MakeBackupFile( pSettings->szOutputFN, aq::backup_type_t::Before, this->level, this->id );
#endif

#ifdef _DEBUG
  std::ostringstream oss;
  oss << *this->sqlStatement << std::endl;
  std::cout << oss.str() << std::endl;
  std::string stmp1 = oss.str();
#endif
  
  //
  // processing order of main verb of the request depends of the resolution mode ('by row' or 'by column')
  std::vector<unsigned int> categories_order;
  if (pSettings->useRowResolver)
  {
    categories_order.push_back( K_FROM );
    categories_order.push_back( K_WHERE );
    categories_order.push_back( K_SELECT );
    categories_order.push_back( K_GROUP );
    categories_order.push_back( K_HAVING );
    categories_order.push_back( K_ORDER );
  }
  else
  {
    categories_order.push_back( K_FROM );
    categories_order.push_back( K_WHERE );
    categories_order.push_back( K_GROUP );
    categories_order.push_back( K_HAVING );
    categories_order.push_back( K_SELECT );
    categories_order.push_back( K_ORDER );
  }
  
	//
	// Query Pre Processing (TODO : optimize tree by detecting identical subtrees)
	timer.start();
	aq::verb::VerbNode::Ptr spTree = aq::verb::VerbNode::BuildVerbsTree( this->sqlStatement, categories_order, this->BaseDesc, this->pSettings );
	spTree->changeQuery();
  
#ifdef _DEBUG
  {
    std::cout << *this->sqlStatement << std::endl;
    aq::verb::VerbNode::dump(std::cout, spTree);
    DumpVisitor printer;
    spTree->accept(&printer);
    std::cout << std::endl << printer.getQuery() << std::endl;
  }
#endif
  
  std::set<aq::tnode*> nodes;
  checkTree( this->sqlStatement, nodes);
	aq::cleanQuery( this->sqlStatement );
	aq::Logger::getInstance().log(AQ_INFO, "Query Preprocessing: Time elapsed = %s\n", aq::Timer::getString(timer.getTimeElapsed()).c_str());
  
	//
	// Solve Optimal Min/Max : FIXME
	timer.start();
	table = solveOptimalMinMax( spTree, BaseDesc, *pSettings );
	aq::Logger::getInstance().log(AQ_INFO, "Solve Optimal Min/Max: Time elapsed = %s\n", aq::Timer::getString(timer.getTimeElapsed()).c_str());
	
	if( !table )
  {

    //vector<VerbNode::Ptr> preorderVerbTree;
    //PreorderTraversal( spStart, preorderVerbTree );
    //for( int idx = (int) preorderVerbTree.size() - 1; idx >= 0; --idx )
    //	preorderVerbTree[idx]->changeQuery();

    aq_engine->call( this->sqlStatement, this->nested ? AQEngine_Intf::NESTED_1 : AQEngine_Intf::REGULAR, this->id );

    if (pSettings->computeAnswer)
    {

      //
      if (pSettings->useRowResolver) // && !this->nested && !this->hasOrderBy && !this->hasPartitionBy)
      {
        this->solveAQMatriceByRows(spTree);
        table = this->result;
      }
      else
      {
        table = this->solveAQMatriceByColumns(spTree);
      }
      spTree = NULL; //debug13 - force delete to see if it causes an error
    }
  }

	return table;
}

//------------------------------------------------------------------------------
//solve all selects found in the main select
void QueryResolver::SolveSelectRecursive(	aq::tnode*& pNode, unsigned int nSelectLevel, aq::tnode* pLastSelect, bool inFrom, bool inIn )
{
	if( pNode == NULL )
		return;
	if( pNode->tag == K_DELETED )
		return;
	aq::tnode* pNewLastSelect = pLastSelect;
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

      aq::analyze::type_t type = aq::analyze::analyze_query(pNode);
      switch (type)
      {
      case aq::analyze::type_t::TEMPORARY_TABLE:
        this->buildTemporaryTable(pNode);
        break;
      case aq::analyze::type_t::FOLD_UP_QUERY:
        this->SolveSelectFromSelect(pNode, pLastSelect, ++this->nestedId);
        break;
      }

    }
    else // if ( inIn )
    {
      // Resolve SubQuery
      bool resolverMode = this->pSettings->useRowResolver; // FIXME: pSettings should be clone
      this->pSettings->useRowResolver = false;
      QueryResolver queryResolver(pNode, this->pSettings, this->aq_engine, this->BaseDesc, nSelectLevel, ++this->nestedId);
      queryResolver.solve();
      Table::Ptr table = queryResolver.getResult();
      this->pSettings->useRowResolver = resolverMode;

      //delete old subtree and add new subtree containing answer
      delete_subtree( pNode );
      if( inIn )
      {
        pNode = new aq::tnode( K_IN_VALUES );
        pNode->left = aq::GetTree( *table );
      }
      else
      {
        pNode = aq::GetTree( *table );
        if (pNode == NULL)
        {
          throw aq::generic_error(aq::generic_error::NOT_IMPLEMENED, "empty nested result not supported");
        }
      }
    }
    //else
    //{
    //  throw aq::generic_error(aq::generic_error::INVALID_QUERY, "SELECT must be in FROM or WHERE clause");
    //}
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
      pNode = new aq::tnode( K_FALSE );
    }
  }

}

//------------------------------------------------------------------------------
void QueryResolver::buildTemporaryTable(aq::tnode * pNode)
{

  std::string alias;
  aq::tnode * as = pNode->parent;
  if ((as != NULL) && (as->tag == K_AS) && (as->right != NULL) && (as->right->getDataType() == aq::NODE_DATA_STRING))
  {
    alias = pNode->parent->right->getData().val_str;
  }
  else
  {
    throw aq::generic_error(aq::generic_error::INVALID_QUERY, "bad nested query: missing as keyword");
  }

  // build table
  this->id_generator += 1;
  boost::shared_ptr<QueryResolver> interiorQuery(new QueryResolver(aq::clone_subtree(pNode), pSettings, aq_engine, BaseDesc, this->id_generator, this->level + 1));
  interiorQuery->solve();
  this->nestedTables.insert(std::make_pair(alias, interiorQuery));
  
  // update base desc
  this->BaseDesc.getTables().push_back(interiorQuery->result);

  // update node tree
  aq::delete_subtree(as->left);
  aq::delete_subtree(as->right);
  as->tag = K_IDENT;
  as->set_string_data(alias.c_str());
  as->left = NULL;
  as->right = NULL;
  as->next = NULL;
}

//------------------------------------------------------------------------------
boost::shared_ptr<QueryResolver> QueryResolver::SolveSelectFromSelect(	aq::tnode* pInteriorSelect, aq::tnode* pExteriorSelect, int nSelectLevel )
{
	if( !pInteriorSelect || !pExteriorSelect )
  {
    throw aq::generic_error(aq::generic_error::INVALID_QUERY, "");
  }
  
  this->id_generator += 1;
  boost::shared_ptr<QueryResolver> query(new QueryResolver(pInteriorSelect, this->pSettings, this->aq_engine, this->BaseDesc, this->id_generator, this->level + 1));

#if _DEBUG
  std::cout << *pInteriorSelect << std::endl;
#endif

#ifdef OUTPUT_NESTED_QUERIES
	std::string str;
	aq::syntax_tree_to_prefix_form( pExteriorSelect, str );
	aq::SaveFile( pSettings->szOutputFN, str.c_str() );
  aq::MakeBackupFile( pSettings->szOutputFN, aq::backup_type_t::Exterior_Before, this->level, this->id );
#endif

	aq::solveSelectStar( pInteriorSelect, BaseDesc );
	aq::solveOneTableInFrom( pInteriorSelect, BaseDesc );

	aq::tnode* pIntSelectAs = aq::getLastTag( pExteriorSelect, NULL, pInteriorSelect, K_AS );
	if( !pIntSelectAs || !pIntSelectAs->right || (pIntSelectAs->right->tag != K_IDENT) )
		throw aq::generic_error(aq::generic_error::INVALID_QUERY, "");

	aq::SolveMinMaxGroupBy solveMinMaxGroupBy;
	bool minMaxGroupBy = solveMinMaxGroupBy.checkAndClear( pInteriorSelect );

	aq::changeTableNames( pIntSelectAs, pInteriorSelect, pExteriorSelect );
	aq::changeColumnNames( pIntSelectAs, pInteriorSelect, pExteriorSelect->left, true );
	aq::changeColumnNames( pIntSelectAs, pInteriorSelect, pExteriorSelect->next, false );
	aq::solveSelectStarExterior( pInteriorSelect, pExteriorSelect );

  if( trivialSelectFromSelect(pInteriorSelect) )
	{
		aq::syntax_tree_to_prefix_form( pInteriorSelect, str );
		aq::SaveFile( pSettings->szOutputFN, str.c_str() );
		aq::MakeBackupFile( pSettings->szOutputFN, aq::backup_type_t::Empty, this->level, this->id );
		aq::mark_as_deleted( pIntSelectAs );
		return query;
	}

	aq::eliminateAliases( pInteriorSelect );

	// copy conditions from inner select WHERE into outer select WHERE
	aq::tnode* intWhereNode = find_main_node( pInteriorSelect, K_WHERE );
	if( intWhereNode )
	{
    // keep only join
    aq::tnode * joinNode = aq::clone_subtree(intWhereNode->left);
    joinNode = aq::getJoin(joinNode);
		aq::addConditionsToWhere( joinNode, pExteriorSelect );
	}
	aq::addInnerOuterNodes( intWhereNode->left, K_INNER, K_INNER );

#ifdef OUTPUT_NESTED_QUERIES
	aq::syntax_tree_to_prefix_form( pExteriorSelect, str );
	aq::SaveFile( pSettings->szOutputFN, str.c_str() );
	aq::MakeBackupFile( pSettings->szOutputFN, aq::backup_type_t::Exterior, this->level, this->id );
#endif
  
#if _DEBUG
  std::string queryInterior;
  std::string queryExterior;
  std::cout << "--- Interior select ---" << std::endl;
  std::cout << aq::syntax_tree_to_prefix_form(pInteriorSelect, queryInterior) << std::endl;
  std::cout << "--- Exterior select ---" << std::endl;
  std::cout << aq::syntax_tree_to_prefix_form(pExteriorSelect, queryExterior) << std::endl;
#endif

  std::vector<unsigned int> categories_order;
  if (pSettings->useRowResolver)
  {
    categories_order.push_back( K_FROM );
    categories_order.push_back( K_WHERE );
    categories_order.push_back( K_SELECT );
    categories_order.push_back( K_GROUP );
    categories_order.push_back( K_HAVING );
    categories_order.push_back( K_ORDER );
  }
  else
  {
    categories_order.push_back( K_FROM );
    categories_order.push_back( K_WHERE );
    categories_order.push_back( K_GROUP );
    categories_order.push_back( K_HAVING );
    categories_order.push_back( K_SELECT );
    categories_order.push_back( K_ORDER );
  }
	aq::verb::VerbNode::Ptr spTree = aq::verb::VerbNode::BuildVerbsTree( pInteriorSelect, categories_order, this->BaseDesc, this->pSettings );
	spTree->changeQuery();
	aq::cleanQuery( pInteriorSelect );

  aq_engine->call(pInteriorSelect, AQEngine_Intf::NESTED_2, nSelectLevel);
	// aq_engine->call(	pInteriorSelect, minMaxGroupBy ? 0 : 1, nSelectLevel );
	// solveMinMaxGroupBy.modifyTmpFiles( pSettings->szTempPath2, nSelectLevel, BaseDesc, *pSettings );

#ifdef OUTPUT_NESTED_QUERIES
	aq::MakeBackupFile( pSettings->szOutputFN, aq::backup_type_t::Empty, this->level, this->id );
#endif
	
	aq::mark_as_deleted( pIntSelectAs );
  
  return query;
}

//------------------------------------------------------------------------------
void QueryResolver::solveAQMatriceByRows(aq::verb::VerbNode::Ptr spTree)
{	
	assert(pSettings->useRowResolver);
	aq::Timer timer;

	// Prepare Columns
	std::vector<Column::Ptr> columnTypes;
	aq::getColumnTypes( this->sqlStatement, columnTypes, this->BaseDesc );
	
#ifdef OUTPUT_NESTED_QUERIES
	aq::MakeBackupFile( pSettings->szOutputFN, aq::backup_type_t::Empty, this->level, this->id );
	aq::MakeBackupFile( pSettings->szAnswerFN, aq::backup_type_t::Before, this->level, this->id );
#endif

  // build process to apply on each row
  boost::shared_ptr<aq::RowProcesses> processes(new aq::RowProcesses);
  
  std::vector<aq::tnode**> columnNodes;
	if (this->hasGroupBy)
	{
		aq::tnode * nodeGroup = find_main_node(this->sqlStatement, K_GROUP);
		aq::getAllColumnNodes(nodeGroup, columnNodes);
	}

  boost::shared_ptr<aq::RowVerbProcess> rowVerbProcess(new aq::RowVerbProcess(spTree));
	processes->addProcess(rowVerbProcess);

  boost::shared_ptr<aq::RowWritter> rowWritter;
  if (this->nested)
  {
    std::string path = this->pSettings->szTempRootPath + "/" + this->pSettings->queryIdent;
    rowWritter.reset(new aq::RowTemporaryWritter(static_cast<unsigned>(BaseDesc.getTables().size() + 1), path.c_str(), pSettings->packSize));
    processes->addProcess(rowWritter);
  }
  else
  {
    rowWritter.reset(new aq::RowWritter(pSettings->output == "stdout" ? pSettings->output : pSettings->szAnswerFN));
    rowWritter->setColumn(columnTypes);
    processes->addProcess(rowWritter);
  }

  // build result from aq matrix
	timer.start();
	aq::solveAQMatrix(*(aq_engine->getAQMatrix()), aq_engine->getTablesIDs(), columnTypes, columnNodes, *pSettings, BaseDesc, processes, this->hasGroupBy );
	aq::Logger::getInstance().log(AQ_INFO, "Load From Answer: Time Elapsed = %s\n", aq::Timer::getString(timer.getTimeElapsed()).c_str());
  
  const std::vector<Column::Ptr>& columnsWritter = rowWritter->getColumns();
  std::copy(columnsWritter.begin(), columnsWritter.end(), std::back_inserter(this->columns));
  
  // build result : FIXME : shouldn't be done in solveRegular ??
  this->result.reset(new Table(this->resultName, static_cast<unsigned>(this->BaseDesc.getTables().size() + 1), true));
  for (std::vector<Column::Ptr>::const_iterator it = this->columns.begin(); it != this->columns.end(); ++it)
  {
    Column::Ptr column = (*it);
    column->setTableName(this->resultName);
    this->result->Columns.push_back(column);
  }
  this->result->TotalCount = rowWritter->getTotalCount();
}

//------------------------------------------------------------------------------
Table::Ptr QueryResolver::solveAQMatriceByColumns(aq::verb::VerbNode::Ptr spTree)
{
	aq::Timer timer;

	// get select columns from query
	std::vector<Column::Ptr> columnTypes;
	aq::getColumnTypes( this->sqlStatement, columnTypes, this->BaseDesc );
	
	//
	// result
	Table::Ptr table = new Table();

	timer.start();
	table->loadFromTableAnswerByColumn(*(aq_engine->getAQMatrix()), aq_engine->getTablesIDs(), columnTypes, *pSettings, BaseDesc );
	aq::Logger::getInstance().log(AQ_INFO, "Load From Answer: Time Elapsed = %s\n", aq::Timer::getString(timer.getTimeElapsed()).c_str());

#ifdef OUTPUT_NESTED_QUERIES
	aq::MakeBackupFile( pSettings->szOutputFN, aq::backup_type_t::Empty, this->level, this->id );
	aq::MakeBackupFile( pSettings->szAnswerFN, aq::backup_type_t::Before, this->level, this->id );
#endif

	if( !table->NoAnswer )
	{
		timer.start();
		spTree->changeResult( table );
		aq::Logger::getInstance().log(AQ_INFO, "Change Result: Time Elapsed = %s\n", aq::Timer::getString(timer.getTimeElapsed()).c_str());
	}
	
	//timer.start();
	//table->cleanRedundantColumns();
	//table->groupBy();
	//aq::Logger::getInstance().log(AQ_INFO, "Group By: Time Elapsed = %s\n", aq::Timer::getString(timer.getTimeElapsed()).c_str());

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
void QueryResolver::changeTemporaryTableName(aq::tnode * pNode)
{
  if (pNode == NULL) return;
  if (pNode->tag == K_PERIOD)
  {
    aq::tnode * table = pNode->left;
    aq::tnode * column = pNode->right;
    assert((table != NULL) && (table->tag == K_IDENT) && (table->getDataType() == aq::NODE_DATA_STRING));
    assert((column != NULL) && (table->tag == K_IDENT) && (table->getDataType() == aq::NODE_DATA_STRING));
    for (std::map<std::string, boost::shared_ptr<QueryResolver> >::const_iterator it = this->nestedTables.begin(); it != this->nestedTables.end(); ++it) 
    {
      if (it->first == table->getData().val_str)
      {
        Table::Ptr tmp = it->second->getResult();
        for (std::vector<Column::Ptr>::iterator itCol = tmp->Columns.begin(); itCol != tmp->Columns.end(); ++itCol)
        {
          if ((*itCol)->getName() == column->getData().val_str)
          {
            char * buf = static_cast<char*>(malloc(128 * sizeof(char)));
            std::string type_str = columnTypeToStr((*itCol)->Type);
            sprintf(buf, "C%.4u%s%.4u", (*itCol)->ID, type_str.c_str(), (*itCol)->Size);
            column->set_string_data(buf);
            free(buf);
          }
        }
      }
    }
    changeTemporaryTableName(table);
    return;
  }
  else if ((pNode->tag == K_IDENT) && (pNode->getDataType() == aq::NODE_DATA_STRING))
  {
    for (std::map<std::string, boost::shared_ptr<QueryResolver> >::const_iterator it = this->nestedTables.begin(); it != this->nestedTables.end(); ++it) 
    {
      if (it->first == pNode->getData().val_str)
      {
        char * buf = static_cast<char*>(malloc(128 * sizeof(char)));
        sprintf(buf, "TMP%.4uSIZE%.10u", it->second->getResult()->ID, it->second->getResult()->TotalCount);
        pNode->set_string_data(buf);
        free(buf);
      }
    }
  }
  changeTemporaryTableName(pNode->left);
  changeTemporaryTableName(pNode->right);
  changeTemporaryTableName(pNode->next);
  
  // change table and column name in BaseDesc
  if (pNode == this->sqlStatement)
  {
    for (std::map<std::string, boost::shared_ptr<QueryResolver> >::const_iterator itTemp = this->nestedTables.begin(); itTemp != this->nestedTables.end(); ++itTemp)
    {
      char * name = static_cast<char*>(malloc(128 * sizeof(char)));
      sprintf(name, "TMP%.4uSIZE%.10u", itTemp->second->result->ID, itTemp->second->result->TotalCount);
      itTemp->second->result->setName(name);
      free(name);

      Table::Ptr tmp = itTemp->second->getResult();
      for (std::vector<Column::Ptr>::iterator itCol = tmp->Columns.begin(); itCol != tmp->Columns.end(); ++itCol)
      {
        char * buf = static_cast<char*>(malloc(128 * sizeof(char)));
        std::string type_str = columnTypeToStr((*itCol)->Type);
        sprintf(buf, "C%.4u%s%.4u", (*itCol)->ID, type_str.c_str(), (*itCol)->Size);
        (*itCol)->setName(buf);
        free(buf);
      }
    }
  }
}

}