#ifndef __STATEMENT_RESOLVER_H__
#define __STATEMENT_RESOLVER_H__

#include "parser/SQLParser.h"
#include "AQEngine_Intf.h"
#include "Table.h"
#include <vector>

namespace aq
{
  
/// Solve Insert Statement
void SolveInsert(tnode* pNode, TProjectSettings * _pSettings, AQEngine_Intf * _aq_engine, Base& _baseDesc);
void SolveInsertAux(Table& table, size_t tableIdx, size_t colIdx, size_t packNumber,
                    std::vector<size_t>& reverseValuePos,
                    Column& nullColumn, Table& valuesToInsert, size_t startIdx, 
                    size_t endIdx, bool append,
                    TProjectSettings * _pSettings, AQEngine_Intf * _aq_engine, Base& _baseDesc);

/// Solve Update and Delete Statement
void SolveUpdateDelete(tnode* pNode, TProjectSettings * _pSettings, AQEngine_Intf * _aq_engine, Base& _baseDesc);

/// Solve Union Minus Statement
void SolveUnionMinus(tnode* pNode, TProjectSettings * _pSettings, AQEngine_Intf * _aq_engine, Base& _baseDesc);

/// Solve Truncate Statement
void addUnionMinusNode(int tag, std::vector<tnode*>& queries, std::vector<int>& operation, tnode* pNode,
                       TProjectSettings * _pSettings, AQEngine_Intf * _aq_engine, Base& _baseDesc);
void SolveTruncate(tnode* pNode, TProjectSettings * _pSettings, AQEngine_Intf * _aq_engine, Base& _baseDesc);

/// Solve Create Statement
void SolveCreate(tnode* pNode, TProjectSettings * _pSettings, AQEngine_Intf * _aq_engine, Base& _baseDesc);

}

#endif