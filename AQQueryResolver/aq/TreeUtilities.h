#pragma once

#include "parser/SQLParser.h"
#include "parser/sql92_grm_tab.hpp"
#include "Table.h"
#include <vector>
#include <list>

extern const int nrJoinTypes;
extern const int joinTypes[];
extern const int inverseTypes[];

namespace aq
{
 
void addAlias( tnode* pNode );
void addConditionsToWhere( tnode* pCond, tnode* pStart );
void addInnerOuterNodes( tnode* pNode, int leftTag, int rightTag );
void mark_as_deleted( tnode* pNode );
void solveSelectStar(	tnode* pNode, 
            Base& BaseDesc,
						std::vector<std::string>& columnNames = std::vector<std::string>(),
						std::vector<std::string>& columnDisplayNames = std::vector<std::string>() );
void solveSelectStarExterior( tnode* pInterior, tnode* pExterior );
void solveOneTableInFrom( tnode* pStart, Base& BaseDesc );
void moveFromJoinToWhere( tnode* pStart, Base& BaseDesc );
void changeTableNames(	tnode* pIntSelectAs, tnode* pInteriorSelect, tnode* pExteriorSelect );
void changeColumnNames(	tnode* pIntSelectAs, tnode* pInteriorSelect, tnode* pExteriorSelect, bool keepAlias );
tnode * getJoin(tnode* pNode);
bool isMonoTable(tnode* query, std::string& tableName);

class SolveMinMaxGroupBy
{
public:
	SolveMinMaxGroupBy();
	~SolveMinMaxGroupBy();
	bool checkAndClear( tnode* pSelect );
	void modifyTmpFiles(	const char* tmpPath, 
							int selectLevel,
							Base& BaseDesc, 
							TProjectSettings& Settings );
private:
	tnode* pGroupBy;
	std::vector<tnode*> columns;
	bool _min;
	size_t minMaxCol;
	std::string tableName;
};

void readTmpFile( const char* filePath, std::vector<llong>& vals );
void writeTmpFile(	const char* filePath, const std::vector<llong>& vals, size_t startIdx, size_t endIdx );

void getColumnsIds(	const Table& table, std::vector<tnode*>& columns, std::vector<int>& valuePos );

void eliminateAliases( tnode* pSelect );

void getAllColumnNodes( tnode*& pNode, std::vector<tnode**>& columnNodes );

void getColumnsList( tnode* pNode, std::vector<tnode*>& columns );

void getTablesList( tnode* pNode, std::list<std::string>& tables );

/// search a subtree for a node and return the last node that had a certain tag
tnode* getLastTag( tnode*& pNode, tnode* pLastTag, tnode* pCheckNode, int tag );

void generate_parent(tnode* pNode, tnode* parent);

void getColumnTypes( tnode* pNode, std::vector<Column::Ptr>& columnTypes, Base& baseDesc );

void cleanQuery( tnode*& pNode );
	
tnode* GetNode( ColumnItem::Ptr item, ColumnType type );

tnode* GetTree( Table& table );

}