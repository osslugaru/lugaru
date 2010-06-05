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

/**> HEADER FILES <**/
#include "WinDefs.h"
#include <windows.h>
#include <errno.h>
#include <time.h>
#include <stdio.h>


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
	dst[0] = len;
	memcpy( dst + 1, src, len);
}


void CopyPascalStringToC( const unsigned char* src, char* dst)
{
	int len = src[0];
	memcpy( dst, src + 1, len);
	dst[len] = 0;
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

char* ConvertFileName( const char* orgfilename, const char* junk)
{
	return ConvertFileName(orgfilename);
}
