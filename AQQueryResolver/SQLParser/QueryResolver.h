#ifndef __FIAN_NESTEDQUERIES_H__
#define __FIAN_NESTEDQUERIES_H__

#include "SQLParser.h"
#include <aq/Utilities.h>
#include "AQEngine_Intf.h"
#include "Table.h"
#include "Verb.h"
#include <aq/Timer.h>
#include <vector>
#include <boost/date_time/posix_time/posix_time.hpp>

//------------------------------------------------------------------------------
/* Executes nested queries and replaces them with the result. */
class QueryResolver
{
public:
	QueryResolver(tnode * _sqlStatement, TProjectSettings * _pSettings, AQEngine_Intf * _aq_engine, Base& _baseDesc, unsigned int _level = 1, unsigned int _id = 1);
	~QueryResolver();

	int SolveSQLStatement();
	Table::Ptr solveAQMatriceByRows(VerbNode::Ptr spTree);
	Table::Ptr solveAQMatriceByColumns(VerbNode::Ptr spTree);
	// Table::Ptr solveAQMatriceV2(VerbNode::Ptr spTree, tnode * pNode);
	Table::Ptr getResult() { return this->result; }

	static VerbNode::Ptr BuildVerbsTree( tnode* pStart, Base& baseDesc, TProjectSettings * settings );
	static void getColumnTypes( tnode* pNode, std::vector<Column::Ptr>& columnTypes, Base& baseDesc );
	static void cleanQuery( tnode*& pNode );
	
private:
	static VerbNode::Ptr BuildVerbsSubtree(	tnode* pSelect, tnode* pStart, tnode* pStartOriginal, int context, Base& BaseDesc, TProjectSettings *pSettings );
	void addUnionMinusNode(	int tag, std::vector<tnode*>& queries, std::vector<int>& operation, tnode* pNode );
						
	/// Solve Select Statement	
	Table::Ptr SolveSelect();
	Table::Ptr SolveSelectRegular();
	
	void SolveSelectFromSelect(	tnode* pInteriorSelect, tnode* pExteriorSelect, int nSelectLevel );
	void SolveSelectRecursive(	tnode*& pNode, unsigned int nSelectLevel, tnode* pLastSelect, bool inFrom, bool inIn  );

	/// Solve Insert Statement
	void SolveInsert(	tnode* pNode );
	void SolveInsertAux(	
    Table& table, size_t tableIdx, size_t colIdx, size_t packNumber,
    std::vector<size_t>& reverseValuePos,
    Column& nullColumn, Table& valuesToInsert, size_t startIdx, 
    size_t endIdx, bool append );

	/// Solve Update and Delete Statement
	void SolveUpdateDelete(	tnode* pNode );

	/// Solve Union Minus Statement
	void SolveUnionMinus(	tnode* pNode );

	/// Solve Truncate Statement
	void SolveTruncate(	tnode* pNode );

	/// Solve Create Statement
	void SolveCreate(	tnode* pNode );
	
  /// backup
  enum backup_type_t
  {
    Empty = 0,
    Before,
    After,
    Exterior_Before,
    Exterior
  };
  int MakeBackupFile( char *pszPath, backup_type_t type ) const;

	/// write a record
	void FileWriteEnreg( aq::ColumnType col_type, const int col_size, char *my_field, FILE *fcol );

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
	Table::Ptr result;
  std::map<size_t, tnode*> values;
  unsigned int id;
  unsigned int nestedId;
  unsigned int level;
	bool nested;
	bool hasGroupBy;
	bool hasOrderBy;
	bool hasPartitionBy;
};

#endif /* __FIAN_NESTEDQUERIES_H__ */