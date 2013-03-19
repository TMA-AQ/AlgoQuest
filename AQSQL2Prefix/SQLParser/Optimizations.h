#pragma once

#include "Table.h"
#include "Verb.h"

//------------------------------------------------------------------------------
Table::Ptr solveOptimalMinMax(	VerbNode::Ptr spTree, Base& BaseDesc, 
								TProjectSettings& Settings );

//------------------------------------------------------------------------------
bool trivialSelectFromSelect( tnode* pSelect );