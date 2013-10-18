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
#include <boost/logic/tribool.hpp>

namespace aq
{
  
/// Solve Select Statement	
class QueryResolver
{
public:
	QueryResolver(aq::tnode * _sqlStatement, TProjectSettings * _pSettings, AQEngine_Intf * _aq_engine, Base& _baseDesc, unsigned int& _id, unsigned int _level = 1);
	~QueryResolver();

  /// main entry
	Table::Ptr solve();

  /// 
  void preProcess();
  aq::verb::VerbNode::Ptr postProcess();
  void resolve(aq::verb::VerbNode::Ptr);

  /// \{
  /// Once aq engine has been called, aq engine matrix must be solve
  /// There is three case:

  /// 1: parse the full aq matrix and apply some verb
	void solveAQMatrix(aq::verb::VerbNode::Ptr spTree);

  /// 2: apply some rules on the result and generate a TEMPORARY table based on REGULAR/BASE table
  void generateTemporaryTable();

  /// 3: just rename .t and .s files
  void renameResultTable();

  /// \}

  size_t getNbRows();
	Table::Ptr getResult() { return this->result; }
  const std::vector<Column::Ptr> getColumns() const { return this->columns; }
  const ColumnItem& getValue(size_t row, size_t column) const;

  void setResultName(const char * value, const char * base) { 
    this->resultTables.clear();
    this->resultTables.push_back(std::make_pair(value, base)); 
  }

  const std::vector<std::pair<std::string, std::string> >& getResultTables() const {return this->resultTables; }

  void setOuterSelect(tnode * select) { this->outerSelect = select; }

  void setInWereClause() { this->inWhereClause = true; }

  void addInnerQuery(const char * id, boost::shared_ptr<QueryResolver> qr);

private:
  /// solve all selects found in the main select
  void solveNested();
	void solveNested(aq::tnode*& pNode, unsigned int nSelectLevel, aq::tnode* pLastSelect, bool inFrom, bool inIn);
  void changeTemporaryTableName(aq::tnode * pNode);
  bool isCompressable();
  boost::tribool isCompressable() const { return this->compressable; }
  std::string getOriginalColumn(const std::string& alias) const;
  void executeNested(aq::tnode * pInteriorSelect);

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
	aq::tnode * sqlStatement;
	aq::tnode * originalSqlStatement;
	aq::tnode * outerSelect;
  std::vector<Column::Ptr> columns;
  std::vector<std::vector<tnode*> > partitions;
  std::vector<tnode*> groupBy;
  std::vector<tnode*> orderBy;
  std::map<std::string, tnode*> aliases;
  std::vector<std::pair<std::string, std::string> > resultTables;
	Table::Ptr result;
  std::map<size_t, aq::tnode*> values;
  std::map<std::string, boost::shared_ptr<QueryResolver> > nestedTables;
  unsigned int& id_generator;
  const unsigned int id;
  unsigned int nestedId;
  unsigned int level;
	bool nested;
  bool inWhereClause;
	bool hasGroupBy;
	bool hasOrderBy;
	bool hasPartitionBy;
  boost::tribool compressable;
};

}

#endif
