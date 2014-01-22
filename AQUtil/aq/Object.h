#pragma once

#ifndef __AQ_OBJECT_H__
#define __AQ_OBJECT_H__

#include <boost/intrusive_ptr.hpp>

/// \brief AlgoQuest Object to hold intrusive pointer
/// \deprecated don't need this class. use of shared_ptr are more than enough for our need.
template <class T>
class Object
{
private:
	unsigned int RefCount;

public:
  typedef boost::intrusive_ptr<T> Ptr;
	
  Object() : RefCount(0)
  {
  }
	
  Object(const Object&) : RefCount(0)
  {
  }
	
  virtual ~Object()
  { 
    assert( RefCount == 0 ); 
  }
	
  Object& operator=(const Object &source)
  { 
    return *this; 
  }
	
  inline void addRef() 
  { 
    RefCount++; 
  }
	
  inline void releaseRef()
	{
		assert( RefCount );
		RefCount--;
		if( RefCount == 0 )
			delete this;
	}
};

template <class T>
inline void	intrusive_ptr_add_ref(Object<T> * pObj)
{
  if (pObj) 
    pObj->addRef();
}

template <class T>
inline void	intrusive_ptr_release(Object<T> * pObj)
{
  if (pObj)
    pObj->releaseRef();
}

#endif
