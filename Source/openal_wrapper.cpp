
#if USE_OPENAL

#include "MacCompatibility.h"
#include "fmod.h"

//#include "al.h"

void F_API FSOUND_3D_Listener_SetAttributes(const float *pos, const float *vel, float fx, float fy, float fz, float tx, float ty, float tz)
{
}

signed char F_API FSOUND_3D_SetAttributes(int channel, const float *pos, const float *vel)
{
    return 0;
}

void F_API FSOUND_3D_SetDopplerFactor(float scale)
{
}

signed char F_API FSOUND_Init(int mixrate, int maxsoftwarechannels, unsigned int flags)
{
    return 0;
}

void F_API FSOUND_Close()
{
}

FSOUND_SAMPLE *F_API FSOUND_GetCurrentSample(int channel)
{
    return 0;
}

signed char F_API FSOUND_GetPaused(int channel)
{
    return 0;
}

unsigned int F_API FSOUND_GetLoopMode(int channel)
{
    return 0;
}

signed char F_API FSOUND_IsPlaying(int channel)
{
    return 0;
}

int F_API FSOUND_PlaySoundEx(int channel, FSOUND_SAMPLE *sptr, FSOUND_DSPUNIT *dsp, signed char startpaused)
{
    return 0;
}

#undef FSOUND_Sample_Load
FSOUND_SAMPLE * F_API FSOUND_Sample_Load(int index, const char *name_or_data, unsigned int mode, int offset, int length)
{
    return 0;
}

void F_API FSOUND_Sample_Free(FSOUND_SAMPLE *sptr)
{
}

signed char F_API FSOUND_Sample_SetMode(FSOUND_SAMPLE *sptr, unsigned int mode)
{
    return 0;
}

signed char F_API FSOUND_Sample_SetMinMaxDistance(FSOUND_SAMPLE *sptr, float min, float max)
{
    return 0;
}

signed char F_API FSOUND_SetFrequency(int channel, int freq)
{
    return 0;
}

signed char F_API FSOUND_SetVolume(int channel, int vol)
{
    return 0;
}

signed char F_API FSOUND_SetPaused(int channel, signed char paused)
{
    return 0;
}

void F_API FSOUND_SetSFXMasterVolume(int volume)
{
}

signed char F_API FSOUND_StopSound(int channel)
{
    return 0;
}

FSOUND_STREAM * F_API FSOUND_Stream_Open(const char *name_or_data, unsigned int mode, int offset, int length)
{
    return 0;
}

signed char F_API FSOUND_Stream_Close(FSOUND_STREAM *stream)
{
    return 0;
}

FSOUND_SAMPLE * F_API FSOUND_Stream_GetSample(FSOUND_STREAM *stream)
{
    return 0;
}

int F_API FSOUND_Stream_PlayEx(int channel, FSOUND_STREAM *stream, FSOUND_DSPUNIT *dsp, signed char startpaused)
{
    return 0;
}

signed char F_API FSOUND_Stream_Stop(FSOUND_STREAM *stream)
{
    return 0;
}

signed char F_API FSOUND_Stream_SetMode(FSOUND_STREAM *stream, unsigned int mode)
{
    return 0;
}

void F_API FSOUND_Update()
{
}

#endif

