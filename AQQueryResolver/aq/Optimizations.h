#pragma once

#include "Table.h"
#include "verbs/VerbNode.h"

//------------------------------------------------------------------------------
Table::Ptr solveOptimalMinMax(	VerbNode::Ptr spTree, Base& BaseDesc, 
								TProjectSettings& Settings );

//------------------------------------------------------------------------------
bool trivialSelectFromSelect( aq::tnode* pSelect );