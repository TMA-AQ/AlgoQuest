#pragma once

#include "verbs/VerbNode.h"

#include <aq/Base.h>
#include <aq/Table.h>

namespace aq
{

/// \deprecated
Table::Ptr solveOptimalMinMax(aq::verb::VerbNode::Ptr spTree, 
                              Base::Ptr               BaseDesc, 
                              Settings::Ptr           Settings);

}