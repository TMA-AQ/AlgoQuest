#include "VerbFactory.h"
#include <aq/Logger.h>

namespace aq {
namespace verb {
  
//------------------------------------------------------------------------------
VerbFactory::~VerbFactory()
{
}

//------------------------------------------------------------------------------
VerbFactory& VerbFactory::GetInstance()
{
	static VerbFactory Instance;
	return Instance;
}

//------------------------------------------------------------------------------
VerbNode::Ptr VerbFactory::getVerb(int verbType) const
{
	return this->builder->build(verbType);
}

}
}