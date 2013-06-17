#include "QueryResolver.h"
#include "SQLPrefix.h"
#include "QueryAnalyzer.h"
#include "parser/sql92_grm_tab.hpp"
#include "Base.h"
#include "Table.h"

#include "RowProcesses.h"
#include "RowWritter.h"
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
  this->originalSqlStatement = aq::clone_subtree(this->sqlStatement);
	memset(szBuffer, 0, STR_BUF_SIZE);
  timer.start();
}

//------------------------------------------------------------------------------
QueryResolver::~QueryResolver()
{
  aq::delete_subtree(this->originalSqlStatement);
  std::for_each(this->aliases.begin(), this->aliases.end(), [] (std::pair<std::string, tnode*> v) { aq::delete_subtree(v.second); });
	aq::Logger::getInstance().log(AQ_INFO, "Query Resolver: Time elapsed = %s\n", aq::Timer::getString(timer.getTimeElapsed()).c_str());
}

//-------------------------------------------------------------------------------
Table::Ptr QueryResolver::solve()
{
  if (!this->sqlStatement || (this->sqlStatement->tag != K_SELECT))
  {
		throw aq::generic_error(aq::generic_error::INVALID_QUERY, "");
  }

	aq::solveIdentRequest(this->sqlStatement, BaseDesc);
  aq::generate_parent(this->sqlStatement, NULL);
  // aq::addAlias(this->sqlStatement->left);
	
  bool hasWhere = find_main_node(this->sqlStatement, K_WHERE) != NULL;
	this->hasGroupBy = find_main_node(this->sqlStatement, K_GROUP) != NULL;
	this->hasOrderBy = find_main_node(this->sqlStatement, K_ORDER) != NULL;
	this->hasPartitionBy = find_main_node(this->sqlStatement, K_OVER) != NULL;

  std::list<tnode*> columns;
  aq::selectToList(this->sqlStatement, columns);
  for (auto it = columns.begin(); it != columns.end(); ++it)
  {
    this->aliases.insert(std::make_pair((*it)->right->getData().val_str, aq::clone_subtree((*it)->left)));
  }

  if (!this->hasGroupBy && hasWhere)
  {
    std::list<tnode*> aggregateColumns;
    aq::findAggregateFunction(columns, aggregateColumns);
    if (!aggregateColumns.empty())
    {
      aq::addEmptyGroupBy(this->sqlStatement);
      this->hasGroupBy = true;
    }
  }

	this->solveNested(this->sqlStatement, this->level, NULL, false, false);

#ifdef _DEBUG
	std::string sql_query;
	aq::syntax_tree_to_sql_form( this->sqlStatement, sql_query );
  std::cout << sql_query << std::endl;
#endif

  this->changeTemporaryTableName(this->sqlStatement);
  
#ifdef _DEBUG
  sql_query = "";
	aq::syntax_tree_to_sql_form( this->sqlStatement, sql_query );
  std::cout << sql_query << std::endl;
#endif

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
    std::cout << "nodes tree:" << std::endl;
    std::cout << *this->sqlStatement << std::endl;
    std::cout << "verbs tree:" << std::endl;
    aq::verb::VerbNode::dump(std::cout, spTree);
    DumpVisitor printer;
    // spTree->apply(&printer);
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
	this->result = solveOptimalMinMax( spTree, BaseDesc, *pSettings );
  if (this->result)
  {
    aq::Logger::getInstance().log(AQ_INFO, "Solve Optimal Min/Max: Time elapsed = %s\n", aq::Timer::getString(timer.getTimeElapsed()).c_str());
  }
  else
  {
    AQEngine_Intf::mode_t mode;
    analyze::type_t type;
    if (!this->nested)
    {
      mode = AQEngine_Intf::mode_t::REGULAR;
      type = analyze::type_t::REGULAR;
    }
    else
    {
      if (this->hasGroupBy)
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

    aq_engine->call( this->sqlStatement, mode, this->id );

    if (pSettings->computeAnswer)
    {
      timer.start();
      switch (type)
      {
      case analyze::type_t::REGULAR:
      case analyze::type_t::TEMPORARY_COLUMN:
        this->solveAQMatriceByRows(spTree);
        aq::Logger::getInstance().log(AQ_INFO, "solve aq matrice in %s", aq::Timer::getString(timer.getTimeElapsed()));
        break;
      case analyze::type_t::FOLD_UP_QUERY:
        this->resultTables.clear();
        this->renameResultTable();
        aq::Logger::getInstance().log(AQ_INFO, "result table renamed in %s", aq::Timer::getString(timer.getTimeElapsed()));
        break;
      case analyze::type_t::TEMPORARY_TABLE:
        this->resultTables.clear();
        this->generateTemporaryTable();
        aq::Logger::getInstance().log(AQ_INFO, "generate temporary table in %s", aq::Timer::getString(timer.getTimeElapsed()));
        break;
      }
      spTree = NULL; //debug13 - force delete to see if it causes an error
    }
  }

	return this->result;
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
        table->load(this->pSettings->szTempPath1, this->pSettings->packSize);
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
  if ((as != NULL) && (as->tag == K_AS) && (as->right != NULL) && (as->right->getDataType() == aq::NODE_DATA_STRING))
  {
    alias = pNode->parent->right->getData().val_str;
  }
  else
  {
    std::cout << *pNode << std::endl;
    throw aq::generic_error(aq::generic_error::INVALID_QUERY, "bad nested query: missing as keyword");
  }

  // build table
  this->id_generator += 1;
  boost::shared_ptr<QueryResolver> interiorQuery(new QueryResolver(aq::clone_subtree(pNode), pSettings, aq_engine, BaseDesc, this->id_generator, this->level + 1));
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

  //
  // Verbs Processing
  std::vector<aq::verb::VerbNode::Ptr> selectVerbs;
  getSelectVerbs(spTree, selectVerbs);
  boost::shared_ptr<aq::ApplyRowVisitor> applyRowVisitor(new aq::ApplyRowVisitor);
  boost::shared_ptr<aq::RowVerbProcess> rowVerbProcess(new aq::RowVerbProcess(spTree, applyRowVisitor, selectVerbs));
	processes->addProcess(rowVerbProcess);

  ////
  //// Output Processing
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

  //
  // build result from aq matrix
  timer.start();
  aq::solveAQMatrix_V2(
    *(aq_engine->getAQMatrix()), aq_engine->getTablesIDs(), columnTypes, columnNodes, 
    *pSettings, BaseDesc, 
    processes, applyRowVisitor->rows, 
    this->hasGroupBy );
  aq::Logger::getInstance().log(AQ_INFO, "build result from aq matrix: Time Elapsed = %s\n", aq::Timer::getString(timer.getTimeElapsed()).c_str());

  //
  // build table result (need by nested query)
  if (this->nested)
  {
    const std::vector<Column::Ptr>& columnsWritter = rowWritter->getColumns();
    std::copy(columnsWritter.begin(), columnsWritter.end(), std::back_inserter(this->columns));

    assert(this->resultTables.size() == 1);
    this->result.reset(new Table(this->resultTables[0].first, static_cast<unsigned>(this->BaseDesc.getTables().size() + 1), true));
    for (std::vector<Column::Ptr>::const_iterator it = this->columns.begin(); it != this->columns.end(); ++it)
    {
      Column::Ptr column = (*it);
      column->setTableName(this->resultTables[0].first);
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
  // this->resultTables.push_back(std::make_pair("B001REG0001TMP0001P000000000007", this->BaseDesc.getTables()[matrix->getTableId(0) - 1]->getName()));

  const AQMatrix::matrix_t& m = matrix->getMatrix();
  std::for_each(m.begin(), m.end(), [&] (const AQMatrix::matrix_t::value_type& c) {
    this->resultTables.push_back(std::make_pair(c.tableName, c.baseTableName));
  });

  aq::Table::Ptr table(new Table(this->resultTables[0].first, static_cast<unsigned>(this->BaseDesc.getTables().size() + 1), true));
  table->setReferenceTable(this->BaseDesc.getTables()[matrix->getTableId(0) - 1]->getName());
  this->BaseDesc.getTables().push_back(table);
}

//------------------------------------------------------------------------------
void QueryResolver::renameResultTable()
{

  std::vector<std::string> files;
  if(aq::GetFiles(this->pSettings->szTempPath1, files) != 0)
    throw aq::generic_error(aq::generic_error::COULD_NOT_OPEN_FILE, "");

  for (auto it = files.begin(); it != files.end(); ++it)
  {
    // BxxxTxxxxTPNxxxxPxxxxxxxxxx.[st]
    if (((*it).length() == 32)
      && ((*it)[0] == 'B')
      && ((*it)[4] == 'T')
      && ((*it)[9] == 'T')
      && ((*it)[10] == 'P')
      && ((*it)[11] == 'N')
      && ((*it)[17] == 'P')
      && ((*it)[30] == '.')
      && (((*it)[31] == 's') || ((*it)[31] == 't')))
    {
      std::string oldFile = std::string(this->pSettings->szTempPath1) + "/" + *it;

      try
      {
        char newFile[_MAX_PATH];
        uint64_t reg = boost::lexical_cast<unsigned>((*it).substr(5, 4));
        uint64_t packet = boost::lexical_cast<unsigned>((*it).substr(18, 12));
        sprintf(newFile, "%s/B001REG%.4uTMP%.4uP%.12u.TMP", this->pSettings->szTempPath1, reg, this->id, packet);
        ::remove(newFile);
        ::rename(oldFile.c_str(), newFile);

        Table::Ptr table = this->BaseDesc.getTable(reg);
        packet = table->TotalCount / this->pSettings->packSize;
        sprintf(newFile, "B001REG%.4uTMP%.4uP%.12u", reg, this->id, packet + 1);
        std::pair<std::string, std::string> p(newFile, table->getName());
        if (std::find(this->resultTables.begin(), this->resultTables.end(), p) == this->resultTables.end())
        {
          this->resultTables.push_back(p);
        }
      }
      catch (const boost::bad_lexical_cast&)
      {
        throw aq::generic_error(aq::generic_error::COULD_NOT_OPEN_FILE, "invalid result file '%s'", oldFile.c_str());
      }
    }
  }

  std::for_each(this->resultTables.begin(), this->resultTables.end(), [&] (const std::pair<std::string, std::string>& e) {
    aq::Table::Ptr table(new Table(e.first, 1, true));
    table->setReferenceTable(e.second);
    this->BaseDesc.getTables().push_back(table);
  });
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
    for (auto it = this->nestedTables.begin(); it != this->nestedTables.end(); ++it) 
    {
      if (it->first == table->getData().val_str)
      {
        Table::Ptr tmp = it->second->getResult();
        if (tmp)
        {
          for (auto itCol = tmp->Columns.begin(); itCol != tmp->Columns.end(); ++itCol)
          {
            if ((*itCol)->getName() == column->getData().val_str)
            {
              char * buf = static_cast<char*>(malloc(128 * sizeof(char)));
              std::string type_str = columnTypeToStr((*itCol)->Type);
              sprintf(buf, "C%.4u%s%.4u", (*itCol)->ID, type_str.c_str(), (*itCol)->Size);
              aq::Logger::getInstance().log(AQ_DEBUG, "change column name '%s' by '%s'\n", column->getData().val_str, buf);
              column->set_string_data(buf);
              free(buf);
            }
          }
        }
        else
        {
          assert(it->second->isCompressable());
          if (!it->second->isCompressable())
            throw aq::generic_error(aq::generic_error::INVALID_TABLE, "empty result on a non compressable nested result");
          auto baseTables = it->second->getResultTables();
          for (auto itTable = baseTables.begin(); itTable != baseTables.end(); ++itTable)
          {
            aq::Table::Ptr tableDesc = this->BaseDesc.getTable(itTable->first);
            // check if the column belong to the table
            bool isPresent = false;
            for (auto itCol = tableDesc->Columns.begin(); (itCol != tableDesc->Columns.end()) && !isPresent; ++itCol)
            {
              if ((*itCol)->getName() == column->getData().val_str)
              {
                // nothing to do
                isPresent = true;
              }
            }
            // if the column doesn't belong to the table, it should be an alias
            if (!isPresent)
            {
              // find the original column and change alias name
              std::string new_column_name = it->second->getOriginalColumn(column->getData().val_str);
              if (new_column_name != "")
              {
                isPresent = true;
                column->set_string_data(new_column_name.c_str());
              }
            }
            if (isPresent)
            {
              table->set_string_data(itTable->first.c_str());
            }
          }
        }
      }
    }
    // changeTemporaryTableName(table);
    assert(!pNode->next);
    return;
  }
  else if ((pNode->tag == K_IDENT) && (pNode->getDataType() == aq::NODE_DATA_STRING))
  {
    for (std::map<std::string, boost::shared_ptr<QueryResolver> >::const_iterator it = this->nestedTables.begin(); it != this->nestedTables.end(); ++it) 
    {
      if (it->first == pNode->getData().val_str)
      {
        if (it->second->isCompressable())
        {
          const std::vector<std::pair<std::string, std::string> >& v = it->second->getResultTables();
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
          sprintf(buf, "TMP%.4uSIZE%.10u", it->second->getResult()->ID, it->second->getResult()->TotalCount);
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
    for (std::map<std::string, boost::shared_ptr<QueryResolver> >::const_iterator itTemp = this->nestedTables.begin(); itTemp != this->nestedTables.end(); ++itTemp)
    {
      if (!itTemp->second->result)
      {
        assert(itTemp->second->isCompressable());
        if (!itTemp->second->isCompressable())
        {
          throw aq::generic_error(aq::generic_error::INVALID_TABLE, "");
        }
        continue;
      }

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
  if (this->compressable) // already compute
    return compressable;
  
  bool c = true;
  tnode * select = aq::find_main_node(this->originalSqlStatement, K_SELECT);
  std::vector<tnode*> nodes;
  aq::getColumnsList(select->left, nodes);
  for (auto it = nodes.begin(); (it != nodes.end()) && c; ++it)
  {
    c = isColumn(*it) || isMinMax(*it);
  }
  this->compressable = c;
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

void QueryResolver::changeTemporaryTableName(std::list<aq::tnode*>& columns)
{
  std::for_each(columns.begin(), columns.end(), [&] (aq::tnode* c) {

  });
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

}