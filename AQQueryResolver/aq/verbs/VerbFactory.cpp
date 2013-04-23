#include "VerbFactory.h"

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
	for( size_t idx = 0; idx < this->Verbs.size(); ++idx )
		if( this->Verbs[idx]->getVerbType() == verbType )
			return this->Verbs[idx]->clone();
	return NULL;
}
