#ifndef __VERB_FACTORY_H__
#define __VERB_FACTORY_H__

#include "VerbNode.h"

namespace aq {
namespace verb {

//--------------------------------------------------------------------------
struct Builder_Intf
{
  virtual VerbNode::Ptr build(unsigned int type) const = 0;
};

//--------------------------------------------------------------------------
class VerbFactory
{
public:
  void setBuilder(Builder_Intf const * _builder) { builder = _builder; }
	VerbNode::Ptr getVerb( int verbType ) const;
	static VerbFactory& GetInstance();
private:
	VerbFactory(){};
	VerbFactory(const VerbFactory& source);
	~VerbFactory(){};
	VerbFactory& operator=( const VerbFactory& source );

	Builder_Intf const * builder;
};

}
}

#endif