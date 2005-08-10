// wrapper for FMOD to account for filename conversions in windows

#ifndef _FMODWRAPPER_H_
#define _FMODWRAPPER_H_

#if USE_OPENAL
#define FSOUND_3D_Listener_SetAttributes OPENAL_3D_Listener_SetAttributes
#define FSOUND_3D_SetAttributes OPENAL_3D_SetAttributes
#define FSOUND_3D_SetDopplerFactor OPENAL_3D_SetDopplerFactor
#define FSOUND_Close OPENAL_Close
#define FSOUND_GetCurrentSample OPENAL_GetCurrentSample
#define FSOUND_GetLoopMode OPENAL_GetLoopMode
#define FSOUND_GetPaused OPENAL_GetPaused
#define FSOUND_Init OPENAL_Init
#define FSOUND_IsPlaying OPENAL_IsPlaying
#define FSOUND_PlaySoundEx OPENAL_PlaySoundEx
#define FSOUND_Sample_Free OPENAL_Sample_Free
#define FSOUND_Sample_Load OPENAL_Sample_Load
#define FSOUND_Sample_SetMinMaxDistance OPENAL_Sample_SetMinMaxDistance
#define FSOUND_Sample_SetMode OPENAL_Sample_SetMode
#define FSOUND_SetFrequency OPENAL_SetFrequency
#define FSOUND_SetPaused OPENAL_SetPaused
#define FSOUND_SetSFXMasterVolume OPENAL_SetSFXMasterVolume
#define FSOUND_SetVolume OPENAL_SetVolume
#define FSOUND_StopSound OPENAL_StopSound
#define FSOUND_Stream_Close OPENAL_Stream_Close
#define FSOUND_Stream_GetSample OPENAL_Stream_GetSample
#define FSOUND_Stream_Open OPENAL_Stream_Open
#define FSOUND_Stream_PlayEx OPENAL_Stream_PlayEx
#define FSOUND_Stream_SetMode OPENAL_Stream_SetMode
#define FSOUND_Stream_Stop OPENAL_Stream_Stop
#define FSOUND_Update OPENAL_Update
#define FSOUND_SetOutput OPENAL_SetOutput
#endif

#include "fmod_header.h"
#include "MacCompatibility.h"

#if USE_OPENAL
#  undef FSOUND_Sample_Load
#  undef FSOUND_Stream_Open
#  define FSOUND_Sample_Load(a, b, c, d, e) OPENAL_Sample_Load( a, ConvertFileName( b), c, d, e)
#  define FSOUND_Stream_Open(a, b, c, d) OPENAL_Stream_Open(ConvertFileName(a), b, c, d)
#elif !PLATFORM_MACOSX
#  define FSOUND_Sample_Load(a, b, c, d, e) FSOUND_Sample_Load( a, ConvertFileName( b), c, d, e)
#  define FSOUND_Stream_Open(a, b, c, d) FSOUND_Stream_Open(ConvertFileName(a), b, c, d)
#endif

#endif

