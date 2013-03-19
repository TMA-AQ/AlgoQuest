#pragma once

#include "SQLParser.h"
#include "sql92_grm_tab.h"
#include <vector>
#include "Table.h"

void addConditionsToWhere( tnode* pCond, tnode* pStart );
void addInnerOuterNodes( tnode* pNode, int leftTag, int rightTag );
extern const int nrJoinTypes;
extern const int joinTypes[];
extern const int inverseTypes[];
void mark_as_deleted( tnode* pNode );
void solveSelectStar(	tnode* pNode, 
            Base& BaseDesc,
						std::vector<std::string>& columnNames = std::vector<std::string>(),
						std::vector<std::string>& columnDisplayNames = std::vector<std::string>() );
void solveSelectStarExterior( tnode* pInterior, tnode* pExterior );
void solveOneTableInFrom( tnode* pStart, Base& BaseDesc );
void moveFromJoinToWhere( tnode* pStart, Base& BaseDesc );
void changeTableNames(	tnode* pIntSelectAs, tnode* pInteriorSelect, 
						tnode* pExteriorSelect );
void changeColumnNames(	tnode* pIntSelectAs, tnode* pInteriorSelect, 
						tnode* pExteriorSelect, bool keepAlias );
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
	bool min;
	size_t minMaxCol;
	std::string tableName;
};

void readTmpFile( const char* filePath, std::vector<llong>& vals );
void writeTmpFile(	const char* filePath, const std::vector<llong>& vals,
					size_t startIdx, size_t endIdx );

void getColumnsIds(	const Table& table, std::vector<tnode*>& columns, 
					std::vector<int>& valuePos );

void eliminateAliases( tnode* pSelect );