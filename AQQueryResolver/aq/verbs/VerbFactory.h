#ifndef __VERB_FACTORY_H__
#define __VERB_FACTORY_H__

#include "VerbNode.h"

//--------------------------------------------------------------------------
class VerbFactory
{
public:
	void addVerb( VerbNode::Ptr verb );
	VerbNode::Ptr getVerb( int verbType ) const;
	static VerbFactory& GetInstance();
private:
	//disable creation
	VerbFactory(){};
	VerbFactory(const VerbFactory& source){};
	VerbFactory& operator=( const VerbFactory& source ){ return *this; }

	std::vector<VerbNode::Ptr> Verbs;
};

//------------------------------------------------------------------------------
//macro
#define VERB_DECLARE( class_name )\
	OBJECT_DECLARE( class_name );\
	private:\
	class FactoryRegister##class_name\
	{\
	public:\
		FactoryRegister##class_name()\
		{\
			VerbFactory::GetInstance().addVerb( new class_name() );\
		}\
	};\
	static FactoryRegister##class_name FactoryRegisterObj##class_name;\
	public:\
	class_name();\
	virtual VerbNode* clone() const { return new class_name(*this); }

//------------------------------------------------------------------------------
#define VERB_IMPLEMENT( class_name )\
	class_name::FactoryRegister##class_name class_name::FactoryRegisterObj##class_name;

#endif