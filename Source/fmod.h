// wrapper for FMOD to account for filename conversions in windows

#ifndef _FMODWRAPPER_H_
#define _FMODWRAPPER_H_

#include "fmod_header.h"
#include "MacCompatibility.h"


#if !PLATFORM_MACOSX
#define FSOUND_Sample_Load( a, b, c, d, e) FSOUND_Sample_Load( a, ConvertFileName( b), c, d, e);
#endif

#endif

