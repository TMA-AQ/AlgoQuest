#ifndef __AQ_NESTEDQUERIES_H__
#define __AQ_NESTEDQUERIES_H__

#include "parser/SQLParser.h"
#include <aq/Utilities.h>
#include "AQEngine_Intf.h"
#include "Table.h"
#include "verbs/VerbNode.h"
#include <aq/Timer.h>
#include <vector>
#include <boost/date_time/posix_time/posix_time.hpp>

namespace aq
{

class QueryResolver
{
public:
	QueryResolver(aq::tnode * _sqlStatement, TProjectSettings * _pSettings, AQEngine_Intf * _aq_engine, Base& _baseDesc, unsigned int& _id, unsigned int _level = 1);
	~QueryResolver();

	Table::Ptr solve();

	void solveAQMatriceByRows(aq::verb::VerbNode::Ptr spTree);
	Table::Ptr solveAQMatriceByColumns(aq::verb::VerbNode::Ptr spTree);

	Table::Ptr getResult() { return this->result; }
  const std::vector<Column::Ptr> getColumns() const { return this->columns; }
  const ColumnItem& getValue(size_t row, size_t column) const;

  void setResultName(const char * value) { this->resultName = value; }
  const char * getResultName() const { return this->resultName.c_str(); }

  //const char * getTableName() const;
  //size_t getNbRows() const;

private:
	/// Solve Select Statement	
	Table::Ptr SolveSelectRegular();
	void SolveSelectRecursive(	aq::tnode*& pNode, unsigned int nSelectLevel, aq::tnode* pLastSelect, bool inFrom, bool inIn  );
	boost::shared_ptr<QueryResolver> SolveSelectFromSelect(	aq::tnode* pInteriorSelect, aq::tnode* pExteriorSelect, int nSelectLevel );
  void buildTemporaryTable(aq::tnode * pInteriorSelect);
  void changeTemporaryTableName(aq::tnode * pNode);

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
	aq::tnode *sqlStatement;
  std::vector<Column::Ptr> columns;
  std::string resultName;
	Table::Ptr result;
  std::map<size_t, aq::tnode*> values;
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

}

#endif