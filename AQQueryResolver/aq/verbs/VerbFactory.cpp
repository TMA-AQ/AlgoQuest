#include "VerbFactory.h"
#include <aq/Logger.h>

namespace aq {
namespace verb {

//------------------------------------------------------------------------------
VerbFactory& VerbFactory::GetInstance()
{
	static VerbFactory Instance;
	return Instance;
}

//------------------------------------------------------------------------------
void VerbFactory::addVerb( VerbNode::Ptr verb )
{
	if( verb )
		this->Verbs.push_back( verb );
}

//------------------------------------------------------------------------------
VerbNode::Ptr VerbFactory::getVerb( int verbType ) const
{
	for (auto it = this->Verbs.begin(); it != this->Verbs.end(); ++it)
  {
		if ((*it)->getVerbType() == verbType)
    {
			return (*it)->clone();
    }
  }
	return NULL;
}

}
}