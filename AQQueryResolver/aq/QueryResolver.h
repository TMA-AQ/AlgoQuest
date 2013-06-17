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
  
/// Solve Select Statement	
class QueryResolver
{
public:
	QueryResolver(aq::tnode * _sqlStatement, TProjectSettings * _pSettings, AQEngine_Intf * _aq_engine, Base& _baseDesc, unsigned int& _id, unsigned int _level = 1);
	~QueryResolver();

  /// main entry
	Table::Ptr solve();

  /// \{
  /// Once aq engine has been called, aq engine matrix must be solve
  /// There is three case:

  /// 1: parse the full aq matrix and apply some verb
	void solveAQMatriceByRows(aq::verb::VerbNode::Ptr spTree);

  /// 2: apply some rules on the result and generate a TEMPORARY table based on REGULAR/BASE table
  void generateTemporaryTable();

  /// 3: just rename .t and .s files
  void renameResultTable();

  /// \}

  /// deprecated
	Table::Ptr solveAQMatriceByColumns(aq::verb::VerbNode::Ptr spTree);

	Table::Ptr getResult() { return this->result; }
  const std::vector<Column::Ptr> getColumns() const { return this->columns; }
  const ColumnItem& getValue(size_t row, size_t column) const;

  void setResultName(const char * value, const char * base) { 
    this->resultTables.clear();
    this->resultTables.push_back(std::make_pair(value, base)); 
  }

  const std::vector<std::pair<std::string, std::string> >& getResultTables() const {return this->resultTables; }

private:
  /// solve all selects found in the main select
	void solveNested(aq::tnode*& pNode, unsigned int nSelectLevel, aq::tnode* pLastSelect, bool inFrom, bool inIn);
  void changeTemporaryTableName(aq::tnode * pNode);
  void changeTemporaryTableName(std::list<aq::tnode*>& column);
  bool isCompressable();
  boost::optional<bool> isCompressable() const { return this->compressable; }
  std::string getOriginalColumn(const std::string& alias) const;
  void executeNested(aq::tnode * pInteriorSelect);

  /// deprecated
	boost::shared_ptr<QueryResolver> SolveSelectFromSelect(	aq::tnode* pInteriorSelect, aq::tnode* pExteriorSelect, int nSelectLevel );

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
  std::vector<Column::Ptr> columns;
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
	bool hasGroupBy;
	bool hasOrderBy;
	bool hasPartitionBy;
  boost::optional<bool> compressable;
};

}

#endif