// Pointer.h: interface for the Pointer class.
//
//////////////////////////////////////////////////////////////////////

#ifndef _POINTER_H
#define _POINTER_H

template <class T>
class Pointer 
{
private:
	void Destroy()
	{
		p = NULL;
		GC::Collect();		
	}

public:
    T* p;

    Pointer( T* p_ = NULL ) : p( p_ ) 
	{
	}

    ~Pointer() 
	{ 
		GC::SetTotalBytesAllocated( GC::GetTotalBytesAllocated() - sizeof( *p ) );

		p->~T(); // Explicitely call the destructor
		
		Destroy();

	}
	
    Pointer& operator = ( Pointer<T>& p_ )
	{
		return operator = ( ( T* ) p_);
	}
    
	Pointer& operator = ( T* p_ ) 
	{     
		Destroy();
		p = p_; 
		return *this;
    }

    operator T*() 
	{ 
		return p; 
	}
    
	T& operator*() 
	{ 
		return *p; 
	}
    
	T* operator->() 
	{ 
		return p; 
	}
    
// For automatic type conversion during new call
	operator void**()
	{
		return ( void** ) & p;
	}
};


#endif 

