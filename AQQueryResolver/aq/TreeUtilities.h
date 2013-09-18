#pragma once

#include "parser/SQLParser.h"
#include "parser/sql92_grm_tab.hpp"
#include "Base.h"
#include "Table.h"
#include <vector>
#include <list>
#include <exception>
#include <boost/algorithm/string.hpp>
#include <string>
#include <algorithm>
#include <aq/ParsException.h>
#include <aq/SQLPrefix.h> //  a delete

extern const int nrJoinTypes;
extern const int joinTypes[];
extern const int inverseTypes[];

namespace aq
{
 
void addAlias( aq::tnode* pNode );
void addConditionsToWhere( aq::tnode* pCond, aq::tnode* pStart );
void addInnerOuterNodes( aq::tnode* pNode, int leftTag, int rightTag );
void mark_as_deleted( aq::tnode* pNode );
void solveSelectStar(aq::tnode* pNode, 
                     Base& BaseDesc,
                     std::vector<std::string>& columnNames,
                     std::vector<std::string>& columnDisplayNames);

void  solveIdentRequest( aq::tnode* pNode, Base& BaseDesc ); ///< create trees of select
void  assignIdentRequest( aq::tnode* pNode, std::vector<aq::tnode*> tables, Base& BaseDesc ); //  used in solveBaseName
std::string checkAndName( std::string colName, std::vector<aq::tnode*> tables, Base& BaseDesc ); // check if the COLUMN exist in the IDENT COLUMN and chose the IDENT
aq::tnode*  assignSafe( aq::tnode* colRef, aq::tnode* clean ); // cut code
aq::tnode*  createPeriodColumn( std::string column, std::string period ); //  create a little tree of a period/column/ident
bool  assignFake( std::string& name, aq::tnode* table, aq::tnode* column );

void solveSelectStarExterior( aq::tnode* pInterior, aq::tnode* pExterior );
void solveOneTableInFrom( aq::tnode* pStart, Base& BaseDesc );
void moveFromJoinToWhere( aq::tnode* pStart, Base& BaseDesc );
void changeTableNames(	aq::tnode* pIntSelectAs, aq::tnode* pInteriorSelect, aq::tnode* pExteriorSelect );
void changeColumnNames(	aq::tnode* pIntSelectAs, aq::tnode* pInteriorSelect, aq::tnode* pExteriorSelect, bool keepAlias );
aq::tnode * getJoin(aq::tnode* pNode);
bool isMonoTable(aq::tnode* query, std::string& tableName);

class SolveMinMaxGroupBy
{
public:
	SolveMinMaxGroupBy();
	~SolveMinMaxGroupBy();
	bool checkAndClear( aq::tnode* pSelect );
	void modifyTmpFiles(	const char* tmpPath, 
							int selectLevel,
							Base& BaseDesc, 
							TProjectSettings& Settings );
private:
	aq::tnode* pGroupBy;
	std::vector<aq::tnode*> columns;
	bool _min;
	size_t minMaxCol;
	std::string tableName;
};

void readTmpFile( const char* filePath, std::vector<llong>& vals );
void writeTmpFile(	const char* filePath, const std::vector<llong>& vals, size_t startIdx, size_t endIdx );

void getColumnsIds(	const Table& table, std::vector<aq::tnode*>& columns, std::vector<int>& valuePos );

void eliminateAliases( aq::tnode* pSelect );

void getAllColumnNodes( aq::tnode*& pNode, std::vector<aq::tnode*>& columnNodes );

void getColumnsList( aq::tnode* pNode, std::vector<aq::tnode*>& columns );

void getTablesList( aq::tnode* pNode, std::list<std::string>& tables );

/// search a subtree for a node and return the last node that had a certain tag
aq::tnode* getLastTag( aq::tnode*& pNode, aq::tnode* pLastTag, aq::tnode* pCheckNode, int tag );

void generate_parent(aq::tnode* pNode, aq::tnode* parent = NULL);

void getColumnTypes( aq::tnode* pNode, std::vector<Column::Ptr>& columnTypes, Base& baseDesc );

void cleanQuery( aq::tnode*& pNode );
	
aq::tnode* Getnode( ColumnItem::Ptr item, ColumnType type );

aq::tnode* GetTree( Table& table );

void toNodeListToStdList(tnode* pNode, std::list<tnode*>& columns);

void findAggregateFunction(const std::list<tnode *>& columns, std::list<tnode *>& aggregateColumns);

void addEmptyGroupBy(tnode * pNode);

void addColumnsToGroupBy(tnode * pNode, const std::list<tnode *>& aggregateColumns);

void setOneColumnByTableOnSelect(tnode * pNode);

void removePartitionBy(tnode *& pNode);

/// Return 1 for true, 0 for false
int is_column_reference(const aq::tnode * pNode);

void dateNodeToBigInt(tnode * pNode);

void transformExpression(const aq::Base& baseDesc, const aq::TProjectSettings& settings, aq::tnode * tree);

}
