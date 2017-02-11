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

#ifndef _PLATFORM_HPP_
#define _PLATFORM_HPP_

#include "Math/Random.hpp"

#include <float.h>
#include <math.h>
#include <stdio.h>

#if defined(WIN32) && !defined(strcasecmp)
#define strcasecmp(a, b) stricmp(a, b)
#endif

struct Point
{
    short v;
    short h;
};

typedef signed char SInt8;
typedef unsigned int UInt32;

typedef struct AbsoluteTime
{
    unsigned long hi;
    unsigned long lo;
} AbsoluteTime;

/* Returns time since the app started, not system start. */
AbsoluteTime UpTime();

typedef long Duration;

enum
{
    durationMicrosecond = -1,
    durationMillisecond = 1,
    durationSecond = 1000,
    durationMinute = 1000 * 60,
    durationHour = 1000 * 60 * 60,
    durationDay = 1000 * 60 * 60 * 24,
    durationForever = 0x7FFFFFFF,
    durationImmediate = 0,
};

Duration AbsoluteDeltaToDuration(AbsoluteTime& a, AbsoluteTime& b);

/* Workaround missing math stuff on MSVC
 * FIXME: Check that it is still necessary nowadays.
 */
#ifdef _MSC_VER
inline bool isnormal(double x)
{
    int ret = _fpclass(x);
    return (ret == _FPCLASS_NN || ret == _FPCLASS_PN);
}

inline float abs(float f)
{
    if (f < 0)
        return -f;
    return f;
}

inline double abs(double d)
{
    if (d < 0)
        return -d;
    return d;
}
#else
#include <stdint.h>
#endif // _MSC_VER

#endif // _PLATFORM_HPP_
