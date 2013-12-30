#pragma once

#include "verbs/VerbNode.h"

#include <aq/Base.h>
#include <aq/Table.h>

namespace aq
{

///
Table::Ptr solveOptimalMinMax(aq::verb::VerbNode::Ptr   spTree, 
                              Base                    & BaseDesc, 
                              Settings        & Settings);

}