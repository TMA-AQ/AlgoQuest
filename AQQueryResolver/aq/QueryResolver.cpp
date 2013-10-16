#include "QueryResolver.h"
#include "SQLPrefix.h"
#include "QueryAnalyzer.h"
#include "parser/sql92_grm_tab.hpp"
#include "Base.h"
#include "Table.h"

#include "RowProcesses.h"
#include "RowWritter.h"
#include "RowTableWritter.h"
#include "RowBinaryWritter.h"
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
    originalSqlStatement(NULL),
    outerSelect(NULL),
		pSettings(_pSettings), 
		aq_engine(_aq_engine), 
		BaseDesc(_baseDesc), 
    nestedId(0),
    level(_level),
    id_generator(_id),
    id(_id),
    nested(id > 1),
    inWhereClause(false),
    hasGroupBy(false),
    hasOrderBy(false),
    hasPartitionBy(false),
    compressable(boost::none)
{
  this->sqlStatement->to_upper();
  this->originalSqlStatement = aq::clone_subtree(this->sqlStatement);
	memset(szBuffer, 0, STR_BUF_SIZE);
  timer.start();
}

//------------------------------------------------------------------------------
QueryResolver::~QueryResolver()
{
  aq::delete_subtree(this->originalSqlStatement);
  for (auto& v : this->aliases) { aq::delete_subtree(v.second); }
	aq::Logger::getInstance().log(AQ_INFO, "Query Resolver: Time elapsed = %s\n", aq::Timer::getString(timer.getTimeElapsed()).c_str());
}

//-------------------------------------------------------------------------------
Table::Ptr QueryResolver::solve()
{

  this->preProcess();

	this->solveNested(this->sqlStatement, this->level, NULL, false, false);
  // this->solveNested();

  aq::verb::VerbNode::Ptr spTree = this->postProcess();

	// if post Process solve the result we don't need to call AQ Engine
  if (!this->result)
  {
    this->resolve(spTree);
  }

	return this->result;
}

//-------------------------------------------------------------------------------
void QueryResolver::preProcess()
{
  if (!this->sqlStatement || (this->sqlStatement->tag != K_SELECT))
  {
		throw aq::generic_error(aq::generic_error::INVALID_QUERY, "");
  }
  
  // post processing before solving nested queries
	aq::solveIdentRequest(this->sqlStatement, BaseDesc);
  aq::generate_parent(this->sqlStatement, NULL);
  aq::addAlias(this->sqlStatement->left);

#if defined(_DEBUG) && defined(_TRACE)
  std::string sql_query;
  std::cout << *this->sqlStatement << std::endl;
  std::cout << aq::syntax_tree_to_sql_form(this->sqlStatement, sql_query) << std::endl;
#endif

  bool hasWhere = find_main_node(this->sqlStatement, K_WHERE) != NULL;
	this->hasGroupBy = find_main_node(this->sqlStatement, K_GROUP) != NULL;
	this->hasOrderBy = find_main_node(this->sqlStatement, K_ORDER) != NULL;

  std::list<tnode*> columns;
  aq::toNodeListToStdList(this->sqlStatement, columns);
  for (auto& col : columns)
  {
    this->aliases.insert(std::make_pair(col->right->getData().val_str, aq::clone_subtree(col->left)));
  }

  if (this->hasGroupBy)
  {
    tnode * grpNode = find_main_node(this->sqlStatement, K_GROUP);
    std::vector<tnode*> cl;
    aq::getColumnsList(grpNode->left->left, cl);
    for (auto& n : cl) 
    {
      this->groupBy.push_back(aq::clone_subtree(n));
    }

    cl.clear();
    aq::getColumnsList(this->sqlStatement->left, cl);
    for (auto& n : cl) 
    {
      if ((n->tag == K_AS) && (n->left->tag == K_MIN)) // TODO : for all aggregate function
      {
        // FIXME : this suppose that the min aggregate function is applied to only one column
        this->orderBy.push_back(aq::clone_subtree(n->left->left));
      }
    }
  }

  if (this->hasOrderBy)
  {
    tnode * grpNode = find_main_node(this->sqlStatement, K_ORDER);
    std::vector<tnode*> cl;
    aq::getColumnsList(grpNode->left->left, cl);
    for (auto& n : cl) 
    {
      this->orderBy.push_back(aq::clone_subtree(n));
    }
  }

  std::vector<tnode*> partitionsNodes;
	find_nodes(this->sqlStatement->left, K_PARTITION, partitionsNodes);
  if (!partitionsNodes.empty())
  {
    // to solve a partition by we must perform an order on result
    // so we get all the column defining the partition
    this->hasPartitionBy = true;
    for (auto& n : partitionsNodes)
    {
      this->partitions.push_back(std::vector<tnode*>());
      std::vector<tnode*>& v = *(this->partitions.rbegin());
      std::vector<tnode*> cl;
      aq::getColumnsList(n->left->left, cl);
      for (auto& n : cl) 
      {
        v.push_back(aq::clone_subtree(n));
      }
    }
    // add order
    tnode * orderNode = find_first_node(this->sqlStatement->left, K_ORDER);
    std::vector<tnode*> orderByNodes;
    aq::getColumnsList(orderNode->left->left, orderByNodes);
    for (auto& n : orderByNodes)
    {
      this->orderBy.push_back(aq::clone_subtree(n));
    }
  }
  aq::removePartitionBy(this->sqlStatement);
  
  if (!this->hasGroupBy && !this->hasPartitionBy/* && hasWhere*/) // FIXME : why where is needed ?
  {
    std::list<tnode*> aggregateColumns;
    aq::findAggregateFunction(columns, aggregateColumns);
    if (!aggregateColumns.empty())
    {
      aq::addEmptyGroupBy(this->sqlStatement);
      this->hasGroupBy = true;
    }
  }
}

//-------------------------------------------------------------------------------
aq::verb::VerbNode::Ptr QueryResolver::postProcess()
{
  aq::verb::VerbNode::Ptr spTree;

#if defined(_DEBUG) && defined(_TRACE)
	sql_query = "";
  std::cout << *this->sqlStatement << std::endl;
  std::cout << aq::multiline_query(aq::syntax_tree_to_sql_form(this->sqlStatement, sql_query)) << std::endl;
#endif

  aq::dateNodeToBigInt(this->sqlStatement);
  aq::transformExpression(this->BaseDesc, *this->pSettings, this->sqlStatement);

#if defined(_DEBUG) && defined(_TRACE)
	sql_query = "";
  std::cout << *this->sqlStatement << std::endl;
  std::cout << aq::multiline_query(aq::syntax_tree_to_sql_form(this->sqlStatement, sql_query)) << std::endl;
#endif

  this->changeTemporaryTableName(this->sqlStatement);
  for (auto& n : this->groupBy) 
  {
    this->changeTemporaryTableName(n);
  }
  for (auto& n : this->orderBy) 
  {
    this->changeTemporaryTableName(n);
  }
  for (auto& v : this->partitions) 
  {
    for (auto& n : v)
    {
      this->changeTemporaryTableName(n);
    }
  }
  
#if defined(_DEBUG) && defined(_TRACE)
  sql_query = "";
  std::cout << "---" << std::endl;
  std::cout << aq::multiline_query(aq::syntax_tree_to_sql_form(this->sqlStatement, sql_query)) << std::endl;
  std::cout << "---" << std::endl;
#endif

// #ifdef OUTPUT_NESTED_QUERIES
	//std::string str;
	//aq::syntax_tree_to_prefix_form( this->sqlStatement, str );
	//aq::SaveFile( pSettings->szOutputFN, str.c_str() );
	//aq::MakeBackupFile( pSettings->szOutputFN, aq::backup_type_t::Before, this->level, this->id );
// #endif

#if defined(_DEBUG) && defined(_TRACE)
  std::ostringstream oss;
  oss << *this->sqlStatement << std::endl;
  std::cout << oss.str() << std::endl;
  std::string stmp1 = oss.str();
#endif
  
  //
  // processing order of main verb of the request depends of the resolution mode ('by row' or 'by column')
  boost::array<uint32_t, 6> categories_order =  { K_FROM, K_WHERE, K_SELECT, K_GROUP, K_HAVING, K_ORDER };
  if (!pSettings->useRowResolver)
  {
    boost::array<uint32_t, 6> new_order =  { K_FROM, K_WHERE, K_SELECT, K_GROUP, K_HAVING, K_ORDER };
    categories_order = new_order;
  }
  
	//
	// Query Pre Processing (TODO : optimize tree by detecting identical subtrees)
	timer.start();
	spTree = aq::verb::VerbNode::BuildVerbsTree( this->sqlStatement, categories_order, this->BaseDesc, this->pSettings );
	spTree->changeQuery();
  
#if defined(_DEBUG) && defined(_TRACE)
  std::cout << "nodes tree:" << std::endl;
  std::cout << *this->sqlStatement << std::endl;
  std::cout << "verbs tree:" << std::endl;
#endif
  
  std::set<aq::tnode*> nodes;
  checkTree( this->sqlStatement, nodes);
	aq::cleanQuery( this->sqlStatement );
	aq::Logger::getInstance().log(AQ_INFO, "Query Preprocessing: Time elapsed = %s\n", aq::Timer::getString(timer.getTimeElapsed()).c_str());
  
	//
	// Solve Optimal Min/Max : FIXME
	timer.start();
	this->result = solveOptimalMinMax( spTree, BaseDesc, *pSettings );
  if (this->result)
  {
    aq::Logger::getInstance().log(AQ_INFO, "Solve Optimal Min/Max: Time elapsed = %s\n", aq::Timer::getString(timer.getTimeElapsed()).c_str());
  }

  return spTree;
}

//-------------------------------------------------------------------------------
void QueryResolver::resolve(aq::verb::VerbNode::Ptr spTree)
{   
  AQEngine_Intf::mode_t mode;
  analyze::type_t type;
  if (!this->nested || this->inWhereClause)
  {
    mode = AQEngine_Intf::mode_t::REGULAR;
    type = analyze::type_t::REGULAR;
  }
  else
  {
    if (this->hasGroupBy || this->hasPartitionBy)
    {
      mode = AQEngine_Intf::mode_t::NESTED_1;
      type = this->isCompressable() ? analyze::type_t::TEMPORARY_TABLE : analyze::type_t::TEMPORARY_COLUMN;
    }
    else
    {
      mode = AQEngine_Intf::mode_t::NESTED_2;
      type = analyze::type_t::FOLD_UP_QUERY;
    }
  }

  // Generate AQEngine Query
  std::string query;
  std::string group;
  std::string order;
  std::string group_and_order;
  aq::syntax_tree_to_aql_form(this->sqlStatement, query);

  std::string::size_type posOrder = query.find("ORDER");
  std::string::size_type posGroup = query.find("GROUP");
  if ((posOrder != std::string::npos) || (posGroup != std::string::npos))
  {
    std::string::size_type pos = std::min(posOrder, posGroup);
    group_and_order = query.substr(pos);
    query = query.substr(0, pos);
  }
  ParseJeq(query);

  if (!this->groupBy.empty())
  {
    group = "GROUP ";
    for (size_t i = 0; i < this->groupBy.size() - 1; ++i)
      group += " , ";
    for (auto it = this->groupBy.begin(); it != this->groupBy.end(); ++it)
    {
      aq::syntax_tree_to_aql_form(*it, group);
    }
  }

  if (!this->orderBy.empty())
  {
    order = "ORDER ";
    for (size_t i = 0; i < this->orderBy.size() - 1; ++i)
      order += " , ";
    for (auto it = this->orderBy.begin(); it != this->orderBy.end(); ++it)
    {
      aq::syntax_tree_to_aql_form(*it, order);
    }
  }

  if (!this->partitions.empty() && !this->partitions[0].empty() && (group == ""))
  {
    group = "GROUP ";
    for (size_t i = 0; i < this->partitions[0].size() - 1; ++i)
      group += " , ";
    for (auto it = this->partitions[0].begin(); it != this->partitions[0].end(); ++it)
    {
      aq::syntax_tree_to_aql_form(*it, group);
    }
  }

  boost::algorithm::trim(query);
  boost::algorithm::trim(group);
  boost::algorithm::trim(order);
  if (!group.empty())
  {
    query += "\n" + group;
  }
  if (!order.empty())
  {
    query += "\n" + order;
  }

  // Call AQEngine
  aq_engine->call(query, mode);
  aq::MakeBackupFile(pSettings->szOutputFN, aq::backup_type_t::Empty, this->level, this->id);

  // parse result
  if (pSettings->computeAnswer)
  {
    timer.start();
    switch (type)
    {
    case analyze::type_t::REGULAR:
    case analyze::type_t::TEMPORARY_COLUMN:
      this->solveAQMatriceByRows(spTree);
      aq::Logger::getInstance().log(AQ_INFO, "solve aq matrice in %s", aq::Timer::getString(timer.getTimeElapsed()).c_str());
      break;
    case analyze::type_t::FOLD_UP_QUERY:
      this->resultTables.clear();
      this->renameResultTable();
      aq::Logger::getInstance().log(AQ_INFO, "result table renamed in %s", aq::Timer::getString(timer.getTimeElapsed()).c_str());
      break;
    case analyze::type_t::TEMPORARY_TABLE:
      this->resultTables.clear();
      this->generateTemporaryTable();
      aq::Logger::getInstance().log(AQ_INFO, "generate temporary table in %s", aq::Timer::getString(timer.getTimeElapsed()).c_str());
      break;
    }
  }
}

//------------------------------------------------------------------------------
void QueryResolver::addInnerQuery(const char * id, boost::shared_ptr<QueryResolver> qr)
{
  this->nestedTables[id] = qr;
}

//------------------------------------------------------------------------------
void QueryResolver::solveNested()
{

  // solve from nested first
  aq::tnode * from = aq::find_main_node(this->sqlStatement, K_FROM);
  std::list<aq::tnode*> l;
  toNodeListToStdList(from, l);
  for (auto& f : l)
  {
    if ((f->tag == K_AS) && (f->left && (f->left->tag == K_SELECT)))
    {
      std::string alias = f->right->to_string();
      this->executeNested(f->left);
    }
  }

  // then solve where nested  
  aq::tnode * where = aq::find_main_node(this->sqlStatement, K_WHERE);
  // todo
}

//------------------------------------------------------------------------------
void QueryResolver::solveNested(aq::tnode*& pNode, unsigned int nSelectLevel, aq::tnode* pLastSelect, bool inFrom, bool inIn)
{
	if((pNode == NULL) || (pNode->tag == K_DELETED))
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
	default:
    break;
	}

  if ((pNode->tag == K_SELECT) && (nSelectLevel > this->level))
  {
    if( inFrom )
    {
      this->executeNested(pNode);
    }
    else // if ( inIn )
    {
      // Resolve SubQuery
      // bool resolverMode = this->pSettings->useRowResolver; // FIXME: pSettings should be clone
      // this->pSettings->useRowResolver = false;
      QueryResolver queryResolver(pNode, this->pSettings, this->aq_engine, this->BaseDesc, nSelectLevel, ++this->nestedId);
      queryResolver.setInWereClause();
      queryResolver.solve();
      Table::Ptr table = queryResolver.getResult();
      // this->pSettings->useRowResolver = resolverMode;

      //delete old subtree and add new subtree containing answer
      delete_subtree( pNode );
      if( inIn )
      {
        pNode = new aq::tnode( K_IN_VALUES );
        pNode->left = aq::GetTree( *table );
      }
      else
      {
        // table->load(this->pSettings->szTempPath1, this->pSettings->packSize);
        pNode = aq::GetTree( *table );
        if (pNode == NULL)
        {
          throw aq::generic_error(aq::generic_error::NOT_IMPLEMENED, "empty nested result not supported");
        }
      }
    }
  }
  else
  {
    if ( pNode->tag == K_SELECT)
      ++nSelectLevel;

    solveNested(pNode->left, nSelectLevel, pNewLastSelect, newInFrom, newInIn);

    solveNested(pNode->right, nSelectLevel, pNewLastSelect, newInFrom, newInIn);

    if( pNode->tag == K_FROM )
      newInFrom = false;

    solveNested(pNode->next, nSelectLevel, pNewLastSelect, newInFrom, newInIn);

    if( (pNode->tag == K_IN) && (pNode->right == NULL) )
    {
      /* If subquery evaluates to an empty set, IN evaluates to FALSE. */
      delete_subtree( pNode );
      pNode = new aq::tnode( K_FALSE );
    }
  }

}

//------------------------------------------------------------------------------
void QueryResolver::executeNested(aq::tnode * pNode)
{

  std::string alias;
  aq::tnode * as = pNode->parent;
  if ((as != NULL) && (as->tag == K_AS) && (as->right != NULL) && (as->right->getDataType() == aq::tnode::tnodeDataType::NODE_DATA_STRING))
  {
    alias = pNode->parent->right->getData().val_str;
  }
  else
  {
#if defined(_DEBUG) && defined(_TRACE)
    std::cout << *pNode << std::endl;
#endif
    throw aq::generic_error(aq::generic_error::INVALID_QUERY, "bad nested query: missing as keyword");
  }

  // build table
  this->id_generator += 1;
  boost::shared_ptr<QueryResolver> interiorQuery(new QueryResolver(aq::clone_subtree(pNode), pSettings, aq_engine, BaseDesc, this->id_generator, this->level + 1));
  interiorQuery->setOuterSelect(this->sqlStatement);
  interiorQuery->setResultName(alias.c_str(), ""); // FIXME
  interiorQuery->solve();
  this->nestedTables.insert(std::make_pair(alias, interiorQuery));
  
  // update base desc
  if (interiorQuery->result)
  {
    this->BaseDesc.getTables().push_back(interiorQuery->result);
  }

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
void getSelectVerbs(aq::verb::VerbNode::Ptr spTree, std::vector<aq::verb::VerbNode::Ptr>& selectVerbs)
{
  aq::verb::SelectVerb::Ptr select = boost::dynamic_pointer_cast<aq::verb::SelectVerb>(spTree);
  while ((select == 0) && (spTree->getBrother()))
  {
    spTree = spTree->getBrother();
    select = boost::dynamic_pointer_cast<aq::verb::SelectVerb>(spTree);
  }
  spTree = spTree->getLeftChild();
  while (spTree)
  {
    if (boost::dynamic_pointer_cast<aq::verb::CommaVerb>(spTree) != 0)
    {
      selectVerbs.push_back(spTree->getRightChild());
      spTree = spTree->getLeftChild();
      assert(spTree);
    }
    else
    {
      selectVerbs.push_back(spTree);
      spTree = 0;
    }
  }
}

//------------------------------------------------------------------------------
void QueryResolver::solveAQMatriceByRows(aq::verb::VerbNode::Ptr spTree)
{	
	assert(pSettings->useRowResolver);
	aq::Timer timer;

	// Prepare Columns
	std::vector<Column::Ptr> columnTypes;
	aq::getColumnTypes( this->sqlStatement, columnTypes, this->BaseDesc );
	
  // build process to apply on each row
  boost::shared_ptr<aq::RowProcesses> processes(new aq::RowProcesses);
  
  assert(!(this->hasGroupBy && this->hasPartitionBy));
  std::vector<aq::tnode*> columnNodes;
	if (this->hasGroupBy)
	{
		aq::tnode * nodeGroup = find_main_node(this->sqlStatement, K_GROUP);
		aq::getAllColumnNodes(nodeGroup, columnNodes);
	}
  else if (this->hasPartitionBy)
  {
    std::copy(this->partitions[0].begin(), this->partitions[0].end(), std::back_inserter(columnNodes));
    assert(!this->partitions.empty());
    assert(!this->partitions[0].empty());
  }

  //
  // Aggregate Verbs Processing
  std::vector<aq::verb::VerbNode::Ptr> aggregateVerbs;
  getSelectVerbs(spTree, aggregateVerbs);
  boost::shared_ptr<aq::RowVerbProcess> aggregateVerbProcess(new aq::RowVerbProcess(spTree, aggregateVerbs));
	processes->addProcess(aggregateVerbProcess);

  //
  // Algebric Verbs Processing
  std::vector<aq::verb::VerbNode::Ptr> algebricVerbs;
  boost::shared_ptr<aq::RowVerbProcess> algebricVerbProcess(new aq::RowVerbProcess(spTree, algebricVerbs));
	processes->addProcess(algebricVerbProcess);

  ////
  //// Output Processing
  boost::shared_ptr<aq::RowWritter_Intf> rowWritter;
  if (this->nested)
  {
    if (this->inWhereClause)
    {
      this->result.reset(new Table("tmp", static_cast<unsigned>(this->BaseDesc.getTables().size() + 1), true));
      rowWritter.reset(new aq::RowTableWritter(this->result));
      processes->addProcess(rowWritter);
    }
    else
    {
      std::string path = this->pSettings->szTempRootPath + "/" + this->pSettings->queryIdent;
      rowWritter.reset(new aq::RowTemporaryWritter(static_cast<unsigned>(BaseDesc.getTables().size() + 1), path.c_str(), pSettings->packSize));
      processes->addProcess(rowWritter);
    }
  }
  else
  {
    rowWritter.reset(new aq::RowWritter(pSettings->output == "stdout" ? pSettings->output : pSettings->szAnswerFN));
    rowWritter->setColumn(columnTypes);
    processes->addProcess(rowWritter);
  }

  //
  // build result from aq matrix
  timer.start();
  aq::solveAQMatrix(aq_engine->getAQMatrix(), columnTypes, columnNodes, *pSettings, BaseDesc, processes, pSettings->process_thread, this->hasGroupBy || this->hasPartitionBy );
  aq::Logger::getInstance().log(AQ_INFO, "build result from aq matrix: Time Elapsed = %s\n", aq::Timer::getString(timer.getTimeElapsed()).c_str());

  //
  // build table result (need by nested query)
  if (this->nested && !this->inWhereClause)
  {
    const std::vector<Column::Ptr>& columnsWritter = rowWritter->getColumns();
    std::copy(columnsWritter.begin(), columnsWritter.end(), std::back_inserter(this->columns));

    std::string name = (this->resultTables.size() == 1) ? this->resultTables[0].first : "tmp";
    this->result.reset(new Table(name, static_cast<unsigned>(this->BaseDesc.getTables().size() + 1), true));
    for (std::vector<Column::Ptr>::const_iterator it = this->columns.begin(); it != this->columns.end(); ++it)
    {
      Column::Ptr column = (*it);
      column->setTableName(name);
      this->result->Columns.push_back(column);
    }
    this->result->TotalCount = rowWritter->getTotalCount();
  }
}

//------------------------------------------------------------------------------
void QueryResolver::generateTemporaryTable()
{
  boost::shared_ptr<AQMatrix> matrix = aq_engine->getAQMatrix();
  matrix->compress();
  matrix->writeTemporaryTable();

  // the table is compress and refer to a physical table so the alias table must be rename by real table name
  assert(this->resultTables.empty());

  const AQMatrix::matrix_t& m = matrix->getMatrix();
  assert(m.size() == 1);
  for (auto& c : m) 
  {
    this->resultTables.push_back(std::make_pair(c.tableName, c.baseTableName));
  }

  aq::Table::Ptr table(new Table(this->resultTables[0].first, static_cast<unsigned>(this->BaseDesc.getTables().size() + 1), true));
  table->setReferenceTable(this->resultTables[0].second);

  // TODO : add column according to query
  aq::Table::Ptr refTable = this->BaseDesc.getTable(this->resultTables[0].second);
  for (auto& column : refTable->Columns) 
  {
    table->Columns.push_back(new aq::Column(*column));
  }

  //unsigned int columnId = 1;
  //for (auto& a : this->aliases) 
  //{
  //  // FIXME
  //  std::string columnName = a.first;
  //  std::string::size_type pos = columnName.find(".");
  //  if (pos != std::string::npos)
  //  {
  //    columnName = columnName.substr(pos + 1);
  //  }
  //  aq::Column::Ptr c(new aq::Column(columnName, columnId++, 0, aq::ColumnType::COL_TYPE_INT));
  //  table->Columns.push_back(c);
  //}

  this->BaseDesc.getTables().push_back(table);
}

//------------------------------------------------------------------------------
void QueryResolver::renameResultTable()
{

  aq_engine->renameResult(this->id, this->resultTables);

  for (auto& e : this->resultTables) 
  {
    aq::Table::Ptr table(new Table(e.first, 1, true));
    table->setReferenceTable(e.second);
    
    aq::Table::Ptr refTable = this->BaseDesc.getTable(e.second);
    for (auto& column : refTable->Columns) 
    {
      table->Columns.push_back(new aq::Column(*column));
    }

    this->BaseDesc.getTables().push_back(table);
  }

  
	// copy WHERE's conditions to outer select WHERE
	aq::tnode* whereNode = find_main_node(this->sqlStatement, K_WHERE);
	if (whereNode)
	{
    // keep only join
    aq::tnode * joinNode = aq::clone_subtree(whereNode->left); // FIXME : possible memory leak
    joinNode = aq::getJoin(joinNode);
    if (joinNode)
    {
      // rename table in join by temporary table

      // look for K_IDENT (should be K_TABLE)
      std::vector<tnode*> ltables;
      aq::find_nodes(joinNode, K_IDENT, ltables);
      for (auto& n : ltables) 
      {
        aq::Table::Ptr table = this->BaseDesc.getTable(n->getData().val_str);
        for (auto& rt : this->resultTables)
        {
          if (rt.second == table->getName())
          {
            n->set_string_data(rt.first.c_str());
          }
        }
      }

      aq::addConditionsToWhere(joinNode, this->outerSelect);
    }
	}
	// aq::addInnerOuterNodes(whereNode->left, K_INNER, K_INNER);
}

//------------------------------------------------------------------------------
void QueryResolver::changeTemporaryTableName(aq::tnode * pNode)
{
  if (pNode == NULL) return;
  if (pNode->tag == K_PERIOD)
  {
    aq::tnode * table = pNode->left;
    aq::tnode * column = pNode->right;
    assert((table != NULL) && (table->tag == K_IDENT) && (table->getDataType() == aq::tnode::tnodeDataType::NODE_DATA_STRING));
    assert((column != NULL) && (table->tag == K_IDENT) && (table->getDataType() == aq::tnode::tnodeDataType::NODE_DATA_STRING));

    // !!!!!!!!!!! FIXME !!!!!!!!!!!!! Hacking for testing (DEMO July 4 2013) !!!!!!!!!!!!!!!
    //if ((strcmp("B", table->getData().val_str) == 0) &&
    //  (strcmp("MIN_SEM_COURT", column->getData().val_str) == 0))
    //{
    //  column->set_string_data("SEQ_SEM_COURT");
    //  table->set_string_data("B001REG0001TMP0003P000000000007");
    //}

    for (auto& nestedTable : this->nestedTables) 
    {
      if (nestedTable.first == table->getData().val_str)
      {
        Table::Ptr tmp = nestedTable.second->getResult();
        if (tmp)
        {
          for (auto& col : tmp->Columns)
          {
            if (col->getName() == column->getData().val_str)
            {
              char * buf = static_cast<char*>(malloc(128 * sizeof(char)));
              std::string type_str = columnTypeToStr(col->Type);
              sprintf(buf, "C%.4u%s%.4u", col->ID, type_str.c_str(), col->Size);
              aq::Logger::getInstance().log(AQ_DEBUG, "change column name '%s' by '%s'\n", column->getData().val_str, buf);
              column->set_string_data(buf);
              free(buf);
              changeTemporaryTableName(table); // FIXME
            }
          }
        }
        else
        {
          assert(nestedTable.second->isCompressable());
          if (!nestedTable.second->isCompressable())
            throw aq::generic_error(aq::generic_error::INVALID_TABLE, "empty result on a non compressable nested result");
          for (auto& baseTable : nestedTable.second->getResultTables())
          {
            aq::Table::Ptr tableBase = this->BaseDesc.getTable(baseTable.first);
            aq::Table::Ptr tableDesc = this->BaseDesc.getTable(baseTable.first);

            //while (tableBase->isTemporary())
            //{
            //  tableBase = this->BaseDesc.getTable(tableBase->getReferenceTable());
            //}

            // check if the column belong to the table
            bool isPresent = false;
            for (auto& col : tableDesc->Columns)
            {
              if (col->getName() == column->getData().val_str)
              {
                // nothing to do
                isPresent = true;
                break;
              }
            }

            // if the column doesn't belong to the table, it should be an alias
            if (!isPresent)
            {
              // find the original column and change alias name
              std::string new_column_name = nestedTable.second->getOriginalColumn(column->getData().val_str);
              if (new_column_name != "")
              {
                isPresent = true;
                column->set_string_data(new_column_name.c_str());
              }
            }

            if (isPresent)
            {
              if (tableDesc->isTemporary())
              {
                // even is the column isPresent, it can be an alias
                std::string new_column_name = nestedTable.second->getOriginalColumn(column->getData().val_str);
                if (new_column_name != "")
                {
                  isPresent = true;
                  column->set_string_data(new_column_name.c_str());
                }
              }
              table->set_string_data(baseTable.first.c_str());
            }

          }
        }
      }
    }
    assert(!pNode->next);
    return;
  }
  else if ((pNode->tag == K_IDENT) && (pNode->getDataType() == aq::tnode::tnodeDataType::NODE_DATA_STRING))
  {
    for (auto& nestedTable : this->nestedTables) 
    {
      if (nestedTable.first == pNode->getData().val_str)
      {
        if (nestedTable.second->isCompressable())
        {
          const auto& v = nestedTable.second->getResultTables();
          assert(!v.empty());
          if (v.size() == 1)
          {
            pNode->set_string_data(v[0].first.c_str());
          }
          else
          {
            pNode->set_string_data(v[0].first.c_str());
            tnode * comma = pNode->parent;
            tnode * p = comma->left;
            assert(comma->tag == K_COMMA);
            for (auto itTableFrom = v.begin() + 1; itTableFrom != v.end(); ++itTableFrom)
            {
              tnode * n = new aq::tnode(K_COMMA);
              n->right = new aq::tnode(K_IDENT);
              n->right->set_string_data(itTableFrom->first.c_str());
              n->left = p;
              p = n;
            }
            assert(p);
            comma->left = p;
          }
        }
        else
        {
          char * buf = static_cast<char*>(malloc(128 * sizeof(char)));
          sprintf(buf, "TMP%.4uSIZE%.10u", nestedTable.second->getResult()->ID, nestedTable.second->getResult()->TotalCount);
          aq::Logger::getInstance().log(AQ_DEBUG, "change table name '%s' by '%s'\n", pNode->getData().val_str, buf);
          pNode->set_string_data(buf);
          free(buf);
        }
      }
    }
  }
  changeTemporaryTableName(pNode->left);
  changeTemporaryTableName(pNode->right);
  changeTemporaryTableName(pNode->next);
  
  // change table and column name in BaseDesc
  if (pNode == this->sqlStatement)
  {
    for (auto& nestedTable : this->nestedTables)
    {
      if (!nestedTable.second->result)
      {
        assert(nestedTable.second->isCompressable());
        if (!nestedTable.second->isCompressable())
        {
          throw aq::generic_error(aq::generic_error::INVALID_TABLE, "");
        }
        continue;
      }

      char * name = static_cast<char*>(malloc(128 * sizeof(char)));
      sprintf(name, "TMP%.4uSIZE%.10u", nestedTable.second->result->ID, nestedTable.second->result->TotalCount);
      nestedTable.second->result->setName(name);
      free(name);

      Table::Ptr tmp = nestedTable.second->getResult();
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

bool isColumn(const tnode * const n) // TODO : TO COMPLETE
{
  return (n->tag == K_PERIOD) || (n->tag == K_COLUMN) || ((n->tag == K_AS) && (isColumn(n->left)));
}

bool isMinMax(const tnode * const n) // TODO : TO COMPLETE
{
  return (n->tag == K_AS) && ((n->left->tag == K_MIN) || (n->left->tag == K_MAX));
}

bool QueryResolver::isCompressable()
{
  // check function select
  // if only column table and min/max occur, the AQMatrix of this query can be compressed
  if (this->compressable.indeterminate_value) // already compute
  {
    if (this->hasPartitionBy)
    {
      this->compressable = false;
    }
    else
    {
      bool c = true;
      tnode * select = aq::find_main_node(this->originalSqlStatement, K_SELECT);
      std::vector<tnode*> nodes;
      aq::getColumnsList(select->left, nodes);
      for (auto it = nodes.begin(); (it != nodes.end()) && c; ++it)
      {
        c = isColumn(*it) || isMinMax(*it);
      }
      this->compressable = c;
    }
  }
  return this->compressable;
}

std::string QueryResolver::getOriginalColumn(const std::string& alias) const
{
  for (auto it = this->aliases.begin(); it != this->aliases.end(); ++it)
  {
    if (it->first == alias)
    {
      // TODO : check if tnode structure refer to only one column
      tnode * n = aq::find_deeper_node(it->second, K_COLUMN);
      assert(n);
      if (strcmp(n->getData().val_str, alias.c_str()) != 0)
      {
        return n->getData().val_str;
      }
      else
      {
        return "";
      }
    }
  }
  std::string res = "";
  for (auto it = this->nestedTables.begin(); (it != this->nestedTables.end()) && (res == ""); ++it)
  {
    res = it->second->getOriginalColumn(alias);
  }
  return res;
}

//------------------------------------------------------------------------------
// DEPRECATED
boost::shared_ptr<QueryResolver> QueryResolver::SolveSelectFromSelect(	aq::tnode* pInteriorSelect, aq::tnode* pExteriorSelect, int nSelectLevel )
{
	if( !pInteriorSelect || !pExteriorSelect )
  {
    throw aq::generic_error(aq::generic_error::INVALID_QUERY, "");
  }
  
  this->id_generator += 1;
  boost::shared_ptr<QueryResolver> query(new QueryResolver(pInteriorSelect, this->pSettings, this->aq_engine, this->BaseDesc, this->id_generator, this->level + 1));

#if defined(_DEBUG) && defined(_TRACE)
  std::cout << *pInteriorSelect << std::endl;
#endif

  std::string str;
  aq::syntax_tree_to_aql_form( pExteriorSelect, str );
  aq::SaveFile( pSettings->szOutputFN, str.c_str() );
  aq::MakeBackupFile( pSettings->szOutputFN, aq::backup_type_t::Exterior_Before, this->level, this->id );

  std::vector<std::string> dummy1;
  std::vector<std::string> dummy2;
  aq::solveSelectStar( pInteriorSelect, BaseDesc, dummy1, dummy2 );
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
		aq::syntax_tree_to_aql_form( pInteriorSelect, str );
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

// #ifdef OUTPUT_NESTED_QUERIES
	aq::syntax_tree_to_aql_form( pExteriorSelect, str );
	aq::SaveFile( pSettings->szOutputFN, str.c_str() );
	aq::MakeBackupFile( pSettings->szOutputFN, aq::backup_type_t::Exterior, this->level, this->id );
// #endif
  
#if defined(_DEBUG) && defined(_TRACE)
  std::string queryInterior;
  std::string queryExterior;
  std::cout << "--- Interior select ---" << std::endl;
  std::cout << aq::syntax_tree_to_aql_form(pInteriorSelect, queryInterior) << std::endl;
  std::cout << "--- Exterior select ---" << std::endl;
  std::cout << aq::syntax_tree_to_aql_form(pExteriorSelect, queryExterior) << std::endl;
#endif

  boost::array<uint32_t, 6> categories_order = { K_FROM, K_WHERE, K_SELECT, K_GROUP, K_HAVING, K_ORDER };
  if (!pSettings->useRowResolver)
  {
    boost::array<uint32_t, 6> new_order = { K_FROM, K_WHERE, K_GROUP, K_HAVING, K_SELECT, K_ORDER };
    categories_order = new_order;
  }
	aq::verb::VerbNode::Ptr spTree = aq::verb::VerbNode::BuildVerbsTree( pInteriorSelect, categories_order, this->BaseDesc, this->pSettings );
	spTree->changeQuery();
	aq::cleanQuery( pInteriorSelect );

  aq_engine->call(pInteriorSelect, AQEngine_Intf::NESTED_2, nSelectLevel);
	// aq_engine->call(	pInteriorSelect, minMaxGroupBy ? 0 : 1, nSelectLevel );
	// solveMinMaxGroupBy.modifyTmpFiles( pSettings->szTempPath2, nSelectLevel, BaseDesc, *pSettings );

// #ifdef OUTPUT_NESTED_QUERIES
	aq::MakeBackupFile( pSettings->szOutputFN, aq::backup_type_t::Empty, this->level, this->id );
// #endif
	
	aq::mark_as_deleted( pIntSelectAs );
  
  return query;
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

// #ifdef OUTPUT_NESTED_QUERIES
	aq::MakeBackupFile( pSettings->szOutputFN, aq::backup_type_t::Empty, this->level, this->id );
	aq::MakeBackupFile( pSettings->szAnswerFN, aq::backup_type_t::Before, this->level, this->id );
// #endif

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

}
