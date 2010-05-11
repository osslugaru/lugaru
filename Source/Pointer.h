/*
Copyright (C) 2003, 2010 - Wolfire Games

This file is part of Lugaru.

Lugaru is free software; you can redistribute it and/or
modify it under the terms of the GNU General Public License
as published by the Free Software Foundation; either version 2
of the License, or (at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  

See the GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program; if not, write to the Free Software
Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
*/

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

