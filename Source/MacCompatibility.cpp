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

#if !PLATFORM_MACOSX

/**> HEADER FILES <**/
#include "MacCompatibility.h"

#ifdef WIN32
#include <windows.h>
#endif

#include <errno.h>
#include <time.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#if PLATFORM_UNIX
#include <unistd.h>
#include <sys/time.h>
#include <sys/stat.h>
#include <assert.h>
typedef long long __int64;
typedef __int64 LARGE_INTEGER;
static int QueryPerformanceFrequency(LARGE_INTEGER *liptr)
{
    assert(sizeof (__int64) == 8);
    assert(sizeof (LARGE_INTEGER) == 8);
    *liptr = 1000;
    return(1);
}

static int QueryPerformanceCounter(LARGE_INTEGER *liptr)
{
    struct timeval tv;
    gettimeofday(&tv, NULL);
    *liptr = ( (((LARGE_INTEGER) tv.tv_sec) * 1000) +
               (((LARGE_INTEGER) tv.tv_usec) / 1000) );
    return(1);
}
#endif

class AppTime
{
public:
	AppTime()
	{
		counterRate = 1;
		baseCounter = 0;
		QueryPerformanceFrequency( (LARGE_INTEGER*)&counterRate);
		QueryPerformanceCounter( (LARGE_INTEGER*)&baseCounter);
	}
	__int64 counterRate;		// LARGE_INTEGER type has no math functions so use int64
	__int64 baseCounter;
};
static AppTime g_appTime;


void CopyCStringToPascal( const char* src, unsigned char dst[256])
{
	int len = strlen( src);
	dst[ 0] = len;
	memcpy( dst + 1, src, len);
}


void CopyPascalStringToC( const unsigned char* src, char* dst)
{
	int len = src[ 0];
	memcpy( dst, src + 1, len);
	dst[ len] = 0;
}


AbsoluteTime UpTime()
{
	__int64 counter;
	QueryPerformanceCounter( (LARGE_INTEGER*)&counter);

	counter -= g_appTime.baseCounter;

	AbsoluteTime time;
	time.lo = (unsigned long)counter;
	time.hi = (unsigned long)(counter >> 32);
	return time;
}


Duration AbsoluteDeltaToDuration( AbsoluteTime& a, AbsoluteTime& b)
{
	__int64 value = a.hi;
	value <<= 32;
	value |= a.lo;
	__int64 value2 = b.hi;
	value2 <<= 32;
	value2 |= b.lo;
	value -= value2;

	if (value <= 0)
		return durationImmediate;

	__int64 frac = value % g_appTime.counterRate;
	value /= g_appTime.counterRate;

	Duration time;

	if (value == 0)
	{
		frac *= -1000000;
		frac /= g_appTime.counterRate;
		time = (Duration)frac;
	}
	else
	{
		frac *= 1000;
		frac /= g_appTime.counterRate;
		value *= 1000;
		value += frac;
		time = (Duration)value;
	}

	return time;
}


#if PLATFORM_UNIX
#include <sys/types.h>
#include <pwd.h>
#include <fcntl.h>
#include <unistd.h>
#include <dirent.h>

// some but not all of this is code from PhysicsFS: http://icculus.org/physfs/
//  the zlib license on physfs allows this cut-and-pasting.
static int locateOneElement(char *buf)
{
    char *ptr;
    char **rc;
    char **i;
    DIR *dirp;

    //if (PHYSFS_exists(buf))
    if (access(buf, F_OK) == 0)
        return(1);  /* quick rejection: exists in current case. */

    ptr = strrchr(buf, '/');  /* find entry at end of path. */
    if (ptr == NULL)
    {
        dirp = opendir(".");
        ptr = buf;
    } /* if */
    else
    {
        *ptr = '\0';
        dirp = opendir(buf);
        *ptr = '/';
        ptr++;  /* point past dirsep to entry itself. */
    } /* else */

    struct dirent *dent;
    while ((dent = readdir(dirp)) != NULL)
    {
        if (strcasecmp(dent->d_name, ptr) == 0)
        {
            strcpy(ptr, dent->d_name); /* found a match. Overwrite with this case. */
            closedir(dirp);
            return(1);
        } /* if */
    } /* for */

    /* no match at all... */
    closedir(dirp);
    return(0);
} /* locateOneElement */


static inline const char *getUserDirByUID(void)
{
    struct passwd *pw = getpwuid(getuid());
    if (pw != NULL)
        return(pw->pw_dir);
    return(NULL);
} /* getUserDirByUID */


static inline const char *getPrefPath(void)
{
    static char *prefpath = NULL;
    if (prefpath == NULL)
    {
        const char *homedir = getenv("HOME");
        if (homedir == NULL)
            homedir = getUserDirByUID();
        if (homedir == NULL)
            homedir = ".";  // oh well.

#if (defined(__APPLE__) && defined(__MACH__))
        const char *PREFPATHNAME = "Library/Application Support/Lugaru";
#else
        const char *PREFPATHNAME = ".lugaru";
#endif
        size_t len = strlen(homedir) + strlen(PREFPATHNAME) + 2;
        prefpath = new char[len];
        snprintf(prefpath, len, "%s/%s", homedir, PREFPATHNAME);
    }
    return(prefpath);
}

static int locateCorrectCase(char *buf, bool makedirs)
{
    int rc;
    char *ptr;
    char *prevptr;

    ptr = prevptr = buf;
    while (ptr = strchr(ptr + 1, '/'))
    {
        *ptr = '\0';  /* block this path section off */
        rc = locateOneElement(buf);
        if (!rc)
        {
            if (makedirs)  /* normal if we're writing; build dirs! */
                mkdir(buf, S_IRWXU);
            else
            {
                *ptr = '/'; /* restore path separator */
                return(-2);  /* missing element in path. */
            } /* else */
        } /* if */
        *ptr = '/'; /* restore path separator */
    } /* while */

    /* check final element... */
    return(locateOneElement(buf) ? 0 : -1);
}


static int locateCorrectFile(char *buf, const char *mode)
{
    if (*buf == '\0')
        return(0);  /* Uh...I guess that's failure. */

    assert((mode[0] == 'w') || (mode[0] == 'r'));

    bool iswriting = (mode[0] == 'w');
    const char *prefpath = getPrefPath();
    size_t len = strlen(buf) + strlen(prefpath) + 2;
    char *prefpathfile = (char *) alloca(len);
    snprintf(prefpathfile, len, "%s/%s", prefpath, buf);

    int rc = locateCorrectCase(prefpathfile, iswriting);  /* favor prefpath. */
    if ( (rc == 0) || ((rc == -1) && (iswriting)) ) // found or create?
        strcpy(buf, prefpathfile);
    else if ((rc < 0) && (!iswriting))  /* not writing? Try game dir... */
        rc = locateCorrectCase(buf, iswriting);

    return(rc);
} /* locateCorrectFile */
#endif

// Dummified the function
static char g_filename[4096];
char* ConvertFileName( const char* orgfilename, const char *mode)
{
    if (orgfilename == g_filename) // recursion?
        return g_filename;

	// translate filename into proper path name
	if (orgfilename[ 0] == ':')
		orgfilename++;
	strcpy( g_filename, orgfilename);

	for (int n = 0; g_filename[ n]; n++)
	{
		if (g_filename[ n] == ':')
			g_filename[ n] = '/';

		else if (g_filename[ n] == '\\')
			g_filename[ n] = '/';
	}

    #if PLATFORM_UNIX
    locateCorrectFile(g_filename, mode);
    #endif

	return g_filename;
}

#endif


