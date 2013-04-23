#pragma once

#ifndef __FIAN_Object_H__
#define __FIAN_Object_H__

#include <boost/intrusive_ptr.hpp>

//--------------------------------------------------------------------------
//Abstract object
class Object
{
	unsigned int RefCount;
public:
	//public methods
	typedef boost::intrusive_ptr<Object> Ptr;
	//constructor
	Object() : RefCount(0){}
	Object(const Object&) : RefCount(0){}
	//destructor
	virtual ~Object(){ assert( RefCount == 0 ); }
	//assign
	Object& operator=(const Object &source){ return *this; }
	//smart pointers helpers
	inline void addRef() { RefCount++; }
	inline void releaseRef()
	{
		assert( RefCount );
		RefCount--;
		if( RefCount == 0 )
			delete this;
	}
};
//------------------------------------------------------------------------------
namespace boost
{
	inline void	intrusive_ptr_add_ref( Object* pObj )
	{
		if( pObj ) 
			pObj->addRef();
	}
	inline void	intrusive_ptr_release( Object* pObj )
	{
		if( pObj )
			pObj->releaseRef();
	}
}//namespace boost
//------------------------------------------------------------------------------
//macro
#define OBJECT_DECLARE( class_name )\
	public:\
	typedef boost::intrusive_ptr<class_name> Ptr;

#endif	//__FIAN_Object_H__
