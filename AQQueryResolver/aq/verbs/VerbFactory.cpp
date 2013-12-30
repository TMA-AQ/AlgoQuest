#include "VerbFactory.h"
#include <aq/Logger.h>

namespace aq {
namespace verb {
  
//------------------------------------------------------------------------------
VerbFactory::~VerbFactory()
{
  if (this->builder != nullptr)
  {
    delete this->builder;
  }
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