// wrapper for FMOD to account for filename conversions in windows

#ifndef _FMODWRAPPER_H_
#define _FMODWRAPPER_H_

#include "fmod_header.h"
#include "MacCompatibility.h"


#ifdef WIN32

#define FSOUND_Sample_Load( a, b, c, d) FSOUND_Sample_Load( a, ConvertFileName( b), c, d, 0);

#endif


#endif

