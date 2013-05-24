#pragma once

#include "Base.h"
#include "Table.h"
#include "verbs/VerbNode.h"

namespace aq
{

//------------------------------------------------------------------------------
Table::Ptr solveOptimalMinMax(	aq::verb::VerbNode::Ptr spTree, Base& BaseDesc, 
								TProjectSettings& Settings );

//------------------------------------------------------------------------------
bool trivialSelectFromSelect( aq::tnode* pSelect );

}