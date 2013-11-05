#ifndef __STATEMENT_RESOLVER_H__
#define __STATEMENT_RESOLVER_H__

#include "parser/SQLParser.h"
#include "AQEngine_Intf.h"
#include "Table.h"
#include <vector>

namespace aq
{
  
/// Solve Insert Statement
void SolveInsert(aq::tnode* pNode, Settings * _pSettings, AQEngine_Intf * _aq_engine, Base& _baseDesc);
void SolveInsertAux(Table& table, size_t tableIdx, size_t colIdx, size_t packNumber,
                    std::vector<size_t>& reverseValuePos,
                    Column& nullColumn, Table& valuesToInsert, size_t startIdx, 
                    size_t endIdx, bool append,
                    Settings * _pSettings, AQEngine_Intf * _aq_engine, Base& _baseDesc);

/// Solve Update and Delete Statement
void SolveUpdateDelete(aq::tnode* pNode, Settings * _pSettings, AQEngine_Intf * _aq_engine, Base& _baseDesc);

/// Solve Union Minus Statement
void SolveUnionMinus(aq::tnode* pNode, Settings * _pSettings, AQEngine_Intf * _aq_engine, Base& _baseDesc);

/// Solve Truncate Statement
void addUnionMinusNode(int tag, std::vector<aq::tnode*>& queries, std::vector<int>& operation, aq::tnode* pNode,
                       Settings * _pSettings, AQEngine_Intf * _aq_engine, Base& _baseDesc);
void SolveTruncate(aq::tnode* pNode, Settings * _pSettings, AQEngine_Intf * _aq_engine, Base& _baseDesc);

/// Solve Create Statement
void SolveCreate(aq::tnode* pNode, Settings * _pSettings, AQEngine_Intf * _aq_engine, Base& _baseDesc);

}

#endif