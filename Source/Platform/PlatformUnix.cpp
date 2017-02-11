/*
Copyright (C) 2003, 2010 - Wolfire Games
Copyright (C) 2010-2017 - Lugaru contributors (see AUTHORS file)

This file is part of Lugaru.

Lugaru is free software; you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation; either version 2 of the License, or
(at your option) any later version.

Lugaru is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with Lugaru.  If not, see <http://www.gnu.org/licenses/>.
*/

#ifdef PLATFORM_UNIX

#include "Platform/Platform.hpp"

#include <assert.h>
#include <cerrno>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <ctime>
#include <sys/stat.h>
#include <sys/time.h>
#include <unistd.h>

typedef long long __int64;
typedef __int64 LARGE_INTEGER;
static int QueryPerformanceFrequency(LARGE_INTEGER* liptr)
{
    assert(sizeof(__int64) == 8);
    assert(sizeof(LARGE_INTEGER) == 8);
    *liptr = 1000;
    return (1);
}

static int QueryPerformanceCounter(LARGE_INTEGER* liptr)
{
    struct timeval tv;
    gettimeofday(&tv, NULL);
    *liptr = ((((LARGE_INTEGER)tv.tv_sec) * 1000) +
              (((LARGE_INTEGER)tv.tv_usec) / 1000));
    return (1);
}

class AppTime
{
public:
    AppTime()
    {
        counterRate = 1;
        baseCounter = 0;
        QueryPerformanceFrequency((LARGE_INTEGER*)&counterRate);
        QueryPerformanceCounter((LARGE_INTEGER*)&baseCounter);
    }
    __int64 counterRate; // LARGE_INTEGER type has no math functions so use int64
    __int64 baseCounter;
};
static AppTime g_appTime;

AbsoluteTime UpTime()
{
    __int64 counter;
    QueryPerformanceCounter((LARGE_INTEGER*)&counter);

    counter -= g_appTime.baseCounter;

    AbsoluteTime time;
    time.lo = (unsigned long)counter;
    time.hi = (unsigned long)(counter >> 32);
    return time;
}

Duration AbsoluteDeltaToDuration(AbsoluteTime& a, AbsoluteTime& b)
{
    __int64 value = a.hi;
    value <<= 32;
    value |= a.lo;
    __int64 value2 = b.hi;
    value2 <<= 32;
    value2 |= b.lo;
    value -= value2;

    if (value <= 0) {
        return durationImmediate;
    }

    __int64 frac = value % g_appTime.counterRate;
    value /= g_appTime.counterRate;

    Duration time;

    if (value == 0) {
        frac *= -1000000;
        frac /= g_appTime.counterRate;
        time = (Duration)frac;
    } else {
        frac *= 1000;
        frac /= g_appTime.counterRate;
        value *= 1000;
        value += frac;
        time = (Duration)value;
    }

    return time;
}

#endif // PLATFORM_UNIX
