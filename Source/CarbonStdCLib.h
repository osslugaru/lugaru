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
