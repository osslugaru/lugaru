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

/************************************************************

    CarbonStdCLib.h
    Assorted helper functions associated with CarbonStdClib.o.

    Copyright Apple Computer,Inc.  2001
    All rights reserved

 * Warning:  This interface is NOT a part of the ANSI C standard.
 *           This header file is not POSIX compliant.
 *           For portable code, don't use this interface.

************************************************************/


#ifndef __CARBONSTDCLIB__
#define __CARBONSTDCLIB__

#ifndef __CONDITIONALMACROS__
#include <ConditionalMacros.h>
#endif

#if defined (__powerc) || defined (powerc) || defined (__CFM68K__)
    #pragma import on
#endif

#if __cplusplus
extern "C" {
#endif  /* __cplusplus */

#if !(CALL_NOT_IN_CARBON || __MPWINTERNAL__)

extern void InitCarbonStdCLib( void ) ;
extern void TermCarbonStdCLib( void ) ;

extern void ConvertUnixPathToHFSPath(const char *unixPath, char *hfsPath) ;
extern void ConvertHFSPathToUnixPath(const char *hfsPath, char *unixPath) ;

/* BSD functions */

int bsd_chdir(const char *path) ;
int bsd_chmod(const char *path , int mode) ;

#endif

#if __cplusplus
}
#endif  /* __cplusplus */

#if defined (__powerc) || defined (powerc) || defined (__CFM68K__)
    #pragma import off
#endif

#endif  /* __CARBONSTDCLIB__ */

