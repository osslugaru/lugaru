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

#ifndef _OPENAL_WRAPPER_HPP_
#define _OPENAL_WRAPPER_HPP_

#include "Math/XYZ.hpp"
#include "Platform/Platform.hpp"

#ifdef __APPLE__
#include <OpenAL/al.h>
#include <OpenAL/alc.h>
#else
#include <AL/al.h>
#include <AL/alc.h>
#endif

#include <vorbis/vorbisfile.h>

#ifdef _WIN32
#include <malloc.h>
#endif

#if 0 /* this should only be enable if OPENAL doesn't provide AL_API on all platforms */
#if (!defined(WIN32) && !defined(_WIN32) && !defined(__WIN32__) && !defined(_WIN64) && !defined(_WIN32_WCE) && !defined(_XBOX)) || (defined(__GNUC__) && defined(WIN32))
#ifndef __cdecl
#define __cdecl
#endif
#ifndef __stdcall
#define __stdcall
#endif
#endif

#if defined(_WIN32_WCE)
#define AL_API _cdecl
#define F_CALLBACKAPI _cdecl
#else
#define AL_API __stdcall
#define F_CALLBACKAPI __stdcall
#endif

#ifdef DLL_EXPORTS
#define DLL_API __declspec(dllexport)
#else
#if defined(__LCC__) || defined(__MINGW32__) || defined(__CYGWIN32__)
#define DLL_API AL_API
#else
#define DLL_API
#endif /* __LCC__ ||  __MINGW32__ || __CYGWIN32__ */
#endif /* DLL_EXPORTS */
#endif /* if 0 */

typedef struct OPENAL_SAMPLE OPENAL_SAMPLE;
typedef OPENAL_SAMPLE OPENAL_STREAM;
typedef struct OPENAL_DSPUNIT OPENAL_DSPUNIT;

#define OPENAL_LOOP_OFF 0x00000001    /* For non looping samples. */
#define OPENAL_LOOP_NORMAL 0x00000002 /* For forward looping samples. */
#define OPENAL_HW3D 0x00001000        /* Attempts to make samples use 3d hardware acceleration. (if the card supports it) */
#define OPENAL_2D 0x00002000          /* Tells software (not hardware) based sample not to be included in 3d processing. */
#define OPENAL_FREE -1                /* value to play on any free channel, or to allocate a sample in a free sample slot. */
#define OPENAL_ALL -3                 /* for a channel index , this flag will affect ALL channels available!  Not supported by every function. */

#ifdef __cplusplus
extern "C" {
#endif

#undef AL_API
#define AL_API

AL_API void OPENAL_3D_Listener_SetAttributes(const float* pos, const float* vel, float fx, float fy, float fz, float tx, float ty, float tz);
AL_API signed char OPENAL_3D_SetAttributes(int channel, const float* pos);
AL_API signed char OPENAL_3D_SetAttributes_(int channel, const XYZ& pos);
AL_API signed char OPENAL_Init(int mixrate, int maxsoftwarechannels, unsigned int flags);
AL_API void OPENAL_Close();
AL_API OPENAL_SAMPLE* OPENAL_Sample_Load(int index, const char* name_or_data, unsigned int mode, int offset, int length);
AL_API void OPENAL_Sample_Free(OPENAL_SAMPLE* sptr);
AL_API signed char OPENAL_SetFrequency(int channel, bool slomo = false);
AL_API signed char OPENAL_SetVolume(int channel, int vol);
AL_API signed char OPENAL_SetPaused(int channel, signed char paused);
AL_API void OPENAL_SetSFXMasterVolume(int volume);
AL_API signed char OPENAL_StopSound(int channel);
AL_API signed char OPENAL_Stream_SetMode(OPENAL_STREAM* stream, unsigned int mode);
AL_API void OPENAL_Update();
void PlaySoundEx(int chan, OPENAL_SAMPLE* sptr, OPENAL_DSPUNIT* dsp, signed char startpaused);
void PlayStreamEx(int chan, OPENAL_SAMPLE* sptr, OPENAL_DSPUNIT* dsp, signed char startpaused);

#ifdef __cplusplus
}
#endif

#endif
