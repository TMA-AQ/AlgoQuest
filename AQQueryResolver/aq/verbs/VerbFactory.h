#ifndef __VERB_FACTORY_H__
#define __VERB_FACTORY_H__

#include "VerbNode.h"

namespace aq {
namespace verb {

/// \brief user must define this interface to build verb corresponding to a tag
struct Builder_Intf
{
  virtual ~Builder_Intf() {}

  /// \brief build a verb
  /// \param tag the type of verb to build, tag refer to tnode tag (see bison/flex parser)
  /// \return the built verb
  virtual VerbNode::Ptr build(aq::tnode::tag_t tag) const = 0;
};

/// \deprecated
/// \brief use this singleton factory to build verb. 
/// this is an historical class used for conveniance builder interface should be sufficiant.
class VerbFactory
{
public:
  void setBuilder(boost::shared_ptr<const Builder_Intf> _builder) { builder = _builder; }
	VerbNode::Ptr getVerb( int verbType ) const;
	static VerbFactory& GetInstance();
private:
	VerbFactory(){};
	VerbFactory(const VerbFactory& source);
	~VerbFactory();
	VerbFactory& operator=( const VerbFactory& source );

	boost::shared_ptr<const Builder_Intf> builder;
};

}
}

#endif
