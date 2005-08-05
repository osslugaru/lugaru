#if !PLATFORM_MACOSX

/**> HEADER FILES <**/
#include "MacCompatibility.h"
#include <windows.h>
#include <errno.h>
#include <time.h>
#include <stdio.h>

#if PLATFORM_UNIX
typedef long long __int64;
typedef __int64 LARGE_INTEGER;
static int QueryPerformanceFrequency(LARGE_INTEGER *liptr)
{
    assert(sizeof (__int64) == 8);
    assert(sizeof (LARGE_INTEGER) == 8);
    *liptr = 1000;
    return(1);
}

static void QueryPerformanceCounter(LARGE_INTEGER *liptr)
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


static char g_filename[ 256];
char* ConvertFileName( const char* orgfilename)
{
	// translate filename into proper path name
	if (orgfilename[ 0] == ':')
		orgfilename++;
	strcpy( g_filename, orgfilename);

	for (int n = 0; g_filename[ n]; n++)
	{
		if (g_filename[ n] == ':')
			g_filename[ n] = '/';
	}

	return g_filename;
}

#endif


