#ifndef __FIAN_NESTEDQUERIES_H__
#define __FIAN_NESTEDQUERIES_H__

#include "parser/SQLParser.h"
#include <aq/Utilities.h>
#include "AQEngine_Intf.h"
#include "Table.h"
#include "verbs/VerbNode.h"
#include <aq/Timer.h>
#include <vector>
#include <boost/date_time/posix_time/posix_time.hpp>

class QueryResolver
{
public:
	QueryResolver(tnode * _sqlStatement, TProjectSettings * _pSettings, AQEngine_Intf * _aq_engine, Base& _baseDesc, unsigned int& _id, unsigned int _level = 1);
	~QueryResolver();

	Table::Ptr solve();

	void solveAQMatriceByRows(VerbNode::Ptr spTree);
	Table::Ptr solveAQMatriceByColumns(VerbNode::Ptr spTree);

	Table::Ptr getResult() { return this->result; }
  const std::vector<Column::Ptr> getColumns() const { return this->columns; }
  const ColumnItem& getValue(size_t row, size_t column) const;

  //const char * getTableName() const;
  //size_t getNbRows() const;

private:
	/// Solve Select Statement	
	Table::Ptr SolveSelectRegular();
	void SolveSelectRecursive(	tnode*& pNode, unsigned int nSelectLevel, tnode* pLastSelect, bool inFrom, bool inIn  );
	boost::shared_ptr<QueryResolver> SolveSelectFromSelect(	tnode* pInteriorSelect, tnode* pExteriorSelect, int nSelectLevel );
  void buildTemporaryTable(tnode * pInteriorSelect);

  ////////////////////////////////////////////////////////////////////////////
	// Variables Members

  // Settings
	TProjectSettings * pSettings;
	Base& BaseDesc;
	AQEngine_Intf * aq_engine;

  // helper
	aq::Timer timer;
	char szBuffer[STR_BUF_SIZE];

  // query
	tnode *sqlStatement;
  std::vector<Column::Ptr> columns;
	Table::Ptr result;
  std::map<size_t, tnode*> values;
  std::map<std::string, boost::shared_ptr<QueryResolver> > nestedTables;
  unsigned int& id_generator;
  const unsigned int id;
  unsigned int nestedId;
  unsigned int level;
	bool nested;
	bool hasGroupBy;
	bool hasOrderBy;
	bool hasPartitionBy;
};

#endif