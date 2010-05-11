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

/*
 * Copyright (c) 1999-2002 Apple Computer, Inc. All rights reserved.
 *
 * @APPLE_LICENSE_HEADER_START@
 * 
 * Portions Copyright (c) 1999-2002 Apple Computer, Inc.  All Rights
 * Reserved.  This file contains Original Code and/or Modifications of
 * Original Code as defined in and that are subject to the Apple Public
 * Source License Version 1.1 (the "License").  You may not use this file
 * except in compliance with the License.  Please obtain a copy of the
 * License at http://www.apple.com/publicsource and read it before using
 * this file.
 * 
 * The Original Code and all software distributed under the License are
 * distributed on an "AS IS" basis, WITHOUT WARRANTY OF ANY KIND, EITHER
 * EXPRESS OR IMPLIED, AND APPLE HEREBY DISCLAIMS ALL SUCH WARRANTIES,
 * INCLUDING WITHOUT LIMITATION, ANY WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE OR NON- INFRINGEMENT.  Please see the
 * License for the specific language governing rights and limitations
 * under the License.
 * 
 * @APPLE_LICENSE_HEADER_END@
 *
 * Modified: $Date: 2002/04/27 20:52:48 $
 * Revision: $Id: LinkedList.h,v 1.3 2002/04/27 20:52:48 lane Exp $
 */


/*
	File:		LinkedList.h

	Contains:

*/

#ifndef __LINKEDLIST__
#define __LINKEDLIST__


//we cant just use "macintosh_build" - the mac macho_build is designated as "posix_build" but may 
//include open transport in some cases
#ifndef __OPENTRANSPORT__
//#if (!macintosh_build)

#include <stddef.h>		//for OTGetLinkObject replacement

#include "OPUtils.h"	//for true/false

#if (windows_build)

// ECF 010928 win32 syncronization routines for safe critical sections.  We can't simply use TryEnterCriticalSection()
// to return a true/false value because it's not available in win95 and 98
class OSCriticalSection
{
	public:
		NMBoolean locked;
		OSCriticalSection() {locked = false; InitializeCriticalSection(&theCriticalSection);}
		~OSCriticalSection() {DeleteCriticalSection(&theCriticalSection);}
		NMBoolean	acquire()	{	if (locked == true) return false; //dont have to wait in this case	
								EnterCriticalSection(&theCriticalSection);
								if (locked) {LeaveCriticalSection(&theCriticalSection); return false;}
								else {locked = true; LeaveCriticalSection(&theCriticalSection); return true;}}
		void	release()	{	EnterCriticalSection(&theCriticalSection);
								locked = false; LeaveCriticalSection(&theCriticalSection);}

	CRITICAL_SECTION theCriticalSection;
};
#endif //windows_build

#if (posix_build)
#include <pthread.h>
class OSCriticalSection
{
	public:
		OSCriticalSection() {pthread_mutex_init(&theMutex,NULL);}
		~OSCriticalSection() {pthread_mutex_destroy(&theMutex);}
		NMBoolean	acquire()	{	int error = pthread_mutex_trylock(&theMutex);
									if (error) return false; else return true;}
		void	release()	{	pthread_mutex_unlock(&theMutex);}

		pthread_mutex_t theMutex;
};

#endif //(posix_build)

/*	-------------------------------------------------------------------------
	** OTLIFO
	**
	** These are functions to implement a LIFO list that is interrupt-safe.
	** The only function which is not is OTReverseList.  Normally, you create
	** a LIFO list, populate it at interrupt time, and then use OTLIFOStealList
	** to atomically remove the list, and OTReverseList to flip the list so that
	** it is a FIFO list, which tends to be more useful.
	------------------------------------------------------------------------- */

	class OTLink;
	class OTLIFO;

	class OTLink
	{
		public:
			OTLink*	fNext;
			void	Init()
						{ fNext = NULL; }
	};

	class OTLIFO
	{
		public:
			OSCriticalSection theLock;
			OTLink*	fHead;
		
			void	Init()	
					{ fHead = NULL; }
		
			void	Enqueue(OTLink* link)
							{ 
								while (true) {if (theLock.acquire()) break;}
								link->fNext = fHead;
								fHead = link;	
								theLock.release();		
							}
					

			OTLink*	Dequeue()
							{
								while (true) {if (theLock.acquire()) break;}						
								OTLink *origHead = fHead;
								fHead = fHead->fNext;
								theLock.release();									
								return origHead;
							}

						
			OTLink*	StealList()
							{	
								while (true) {if (theLock.acquire()) break;}
								OTLink *origHead = fHead;
								fHead = NULL;
								theLock.release();
								return origHead;
							}
						
						
			NMBoolean	IsEmpty()
							{
								return fHead == NULL;
							}
	};

/*	-------------------------------------------------------------------------
	** OTList
	**
	** An OTList is a non-interrupt-safe list, but has more features than the
	** OTLIFO list. It is a standard singly-linked list.
	------------------------------------------------------------------------- */

	typedef struct OTList	OTList;

		typedef NMBoolean (*OTListSearchProcPtr)(const void* ref, OTLink* linkToCheck);
		//
		// Remove the last link from the list
		//
	extern OTLink*		OTRemoveLast(OTList* pList);
		//
		// Return the first link from the list
		//
	extern OTLink*		OTGetFirst(OTList* pList);
		//
		// Return the last link from the list
		//
	extern OTLink*		OTGetLast(OTList* pList);
		//
		// Return true if the link is present in the list
		//
	extern NMBoolean		OTIsInList(OTList* pList, OTLink* link);
		//
		// Find a link in the list which matches the search criteria
		// established by the search proc and the refPtr.  This is done
		// by calling the search proc, passing it the refPtr and each
		// link in the list, until the search proc returns true.
		// NULL is returned if the search proc never returned true.
		//
	extern OTLink*		OTFindLink(OTList* pList, OTListSearchProcPtr proc, const void* refPtr);
		//
		// Remove the specified link from the list, returning true if it was found
		//
	extern NMBoolean		OTRemoveLink(OTList*, OTLink*);
		//
		// Similar to OTFindLink, but it also removes it from the list.
		//
	extern OTLink*		OTFindAndRemoveLink(OTList* pList, OTListSearchProcPtr proc, const void* refPtr);
		//
		// Return the "index"th link in the list
		//
	extern OTLink*		OTGetIndexedLink(OTList* pList, size_t index);

	struct OTList
	{
		OTLink*		fHead;
		
		void		Init()	
						{ fHead = NULL; }
		
		NMBoolean		IsEmpty()
						{ return fHead == NULL; }
						
		void		AddFirst(OTLink* link)
							{ 
								link->fNext = fHead;
								fHead = link;			
							}

		void		AddLast(OTLink* link)
							{
								if (fHead == NULL)
									fHead = link->fNext;
								else
								{
									OTLink *current = fHead;
									
									while (current->fNext != NULL)
										current = current->fNext;
										
									current->fNext = link;
								}
								
								
								link->fNext = fHead;
								fHead = link;			
							}

		
		OTLink*		GetFirst()
						{ return OTGetFirst(this); }
		
		OTLink*		GetLast()
						{ return OTGetLast(this); }
		
		OTLink*		RemoveFirst()
							{
								OTLink *origHead = fHead;
								fHead = fHead->fNext;
								return origHead;
							}
								
		OTLink*		RemoveLast()
						{ return OTRemoveLast(this); }
						
		NMBoolean		IsInList(OTLink* link)
						{ return OTIsInList(this, link); }
						
		OTLink*		FindLink(OTListSearchProcPtr proc, const void* ref)
						{ return OTFindLink(this, proc, ref); }
						
		NMBoolean		RemoveLink(OTLink* link)
						{ return OTRemoveLink(this, link); }
						
		OTLink*		RemoveLink(OTListSearchProcPtr proc, const void* ref)
						{ return OTFindAndRemoveLink(this, proc, ref); }
						
		OTLink*		GetIndexedLink(size_t index)
						{ return OTGetIndexedLink(this, index); }
	};
	
	
//FIXME!!  this is a recursive function and will crash and burn on large lists
static OTLink* OTReverseList(OTLink *headRef)
{
	OTLink	*first;
	OTLink	*rest;

	if (headRef == NULL) return NULL;
	
	first = headRef;
	rest = (OTLink *) first->fNext;
	
	if (rest == NULL) return headRef;
	
	rest = OTReverseList(rest);
	
	first->fNext->fNext = first;
	first->fNext = NULL;
	
	return rest;
}
	

	#define OTGetLinkObject(link, struc, field)	\
		((struc*)((char*)(link) - offsetof(struc, field)))

#endif //!macintosh_build
#endif	/* __LINKEDLIST__ */


