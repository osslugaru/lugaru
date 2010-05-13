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


#if USE_OPENAL

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#ifdef _MSC_VER
#include <alloca.h>
#endif

#include "MacCompatibility.h"
#include "fmod.h"

#include "AL/al.h"
#include "AL/alc.h"

#include "ogg/ogg.h"
#include "vorbis/vorbisfile.h"

// NOTE:
// FMOD uses a Left Handed Coordinate system, OpenAL uses a Right Handed
//  one...so we just need to flip the sign on the Z axis when appropriate.


#define DYNAMIC_LOAD_OPENAL 0

#if DYNAMIC_LOAD_OPENAL

#include <dlfcn.h>

#define AL_FUNC(t,ret,fn,params,call,rt) \
    extern "C" { \
        static ret ALAPIENTRY (*p##fn) params = NULL; \
        ret ALAPIENTRY fn params { rt p##fn call; } \
    }
#include "alstubs.h"
#undef AL_FUNC

static void *aldlhandle = NULL;

static bool lookup_alsym(const char *funcname, void **func, const char *libname)
{
    if (!aldlhandle)
        return false;

    *func = dlsym(aldlhandle, funcname);
    if (*func == NULL)
    {
        fprintf(stderr, "Failed to find OpenAL symbol \"%s\" in \"%s\"\n",
                 funcname, libname);
        return false;
    }
    return true;
}

static void unload_alsyms(void)
{
    #define AL_FUNC(t,ret,fn,params,call,rt) p##fn = NULL;
    #include "alstubs.h"
    #undef AL_FUNC
    if (aldlhandle)
    {
        dlclose(aldlhandle);
        aldlhandle = NULL;
    }
}

static bool lookup_all_alsyms(const char *libname)
{
    if (!aldlhandle)
    {
        if ( (aldlhandle = dlopen(libname, RTLD_GLOBAL | RTLD_NOW)) == NULL )
            return false;
    }

    bool retval = true;
    #define AL_FUNC(t,ret,fn,params,call,rt) \
        if (!lookup_alsym(#fn, (void **) &p##fn, libname)) retval = false;
    #include "alstubs.h"
    #undef AL_FUNC

    if (!retval)
        unload_alsyms();

    return retval;
}
#else
#define lookup_all_alsyms(x) (true)
#define unload_alsyms()
#endif


typedef struct
{
    ALuint sid;
    FSOUND_SAMPLE *sample;
    bool startpaused;
    float position[3];
} OPENAL_Channels;

typedef struct FSOUND_SAMPLE
{
    char *name;
    ALuint bid;  // buffer id.
    int mode;
    int is2d;
    float min_distance;
} FSOUND_SAMPLE;

typedef struct FSOUND_STREAM
{
    char *name;
    ALuint bid;  // buffer id.
    int mode;
    int is2d;
} FSOUND_STREAM;

static size_t num_channels = 0;
static OPENAL_Channels *channels = NULL;
static bool initialized = false;
static float listener_position[3];


static inline bool source_too_close(const int channel)
{
    const OPENAL_Channels *chan = &channels[channel];
    const float *pos = chan->position;
    const float distance = sqrtf(powf((pos[0] - listener_position[0]), 2.0f) +
                                 powf((pos[1] - listener_position[1]), 2.0f) +
                                 powf((pos[2] - listener_position[2]), 2.0f));
    return (distance <= chan->sample->min_distance);
}


static void set_channel_position(const int channel, const float x,
                                 const float y, const float z)
{
    OPENAL_Channels *chan = &channels[channel];

    chan->position[0] = x;
    chan->position[1] = y;
    chan->position[2] = z;

    FSOUND_SAMPLE *sptr = chan->sample;
    if (sptr == NULL)
        return;

    const ALuint sid = chan->sid;
    const bool no_attenuate = ((sptr->is2d) || (source_too_close(channel)));

    if (no_attenuate)
    {
        alSourcei(sid, AL_SOURCE_RELATIVE, AL_TRUE);
        alSource3f(sid, AL_POSITION, 0.0f, 0.0f, 0.0f);
    }
    else
    {
        alSourcei(sid, AL_SOURCE_RELATIVE, AL_FALSE);
        alSource3f(sid, AL_POSITION, x, y, z);
    }
}


void F_API OPENAL_3D_Listener_SetAttributes(const float *pos, const float *vel, float fx, float fy, float fz, float tx, float ty, float tz)
{
    if (!initialized) return;
    if (pos != NULL)
    {
        alListener3f(AL_POSITION, pos[0], pos[1], -pos[2]);
        listener_position[0] = pos[0];
        listener_position[1] = pos[1];
        listener_position[2] = -pos[2];
    }

    ALfloat vec[6] = { fx, fy, -fz, tz, ty, -tz };
    alListenerfv(AL_ORIENTATION, vec);

    // we ignore velocity, since doppler's broken in the Linux AL at the moment...

    // adjust existing positions...
    for (int i = 0; i < num_channels; i++)
    {
        const float *p = channels[i].position;
        set_channel_position(i, p[0], p[1], p[2]);
    }
}

signed char F_API OPENAL_3D_SetAttributes(int channel, const float *pos, const float *vel)
{
    if (!initialized) return false;
    if ((channel < 0) || (channel >= num_channels)) return false;

    if (pos != NULL)
        set_channel_position(channel, pos[0], pos[1], -pos[2]);

    // we ignore velocity, since doppler's broken in the Linux AL at the moment...

    return true;
}

void F_API OPENAL_3D_SetDopplerFactor(float scale)
{
    if (!initialized) return;
    // unimplemented...looks like init routines just call this with scale == 0.0f anyhow.
}

signed char F_API OPENAL_Init(int mixrate, int maxsoftwarechannels, unsigned int flags)
{
    if (initialized) return false;
    if (maxsoftwarechannels == 0) return false;

    if (flags != 0)  // unsupported.
        return false;

    if (!lookup_all_alsyms("./openal.so"))  // !!! FIXME: linux specific lib name
    {
        if (!lookup_all_alsyms("openal.so.1"))  // !!! FIXME: linux specific lib name
        {
            if (!lookup_all_alsyms("openal.so"))  // !!! FIXME: linux specific lib name
                return false;
        }
    }

    ALCdevice *dev = alcOpenDevice(NULL);
    if (!dev)
        return false;

    ALint caps[] = { ALC_FREQUENCY, mixrate, 0 };
    ALCcontext *ctx = alcCreateContext(dev, caps);
    if (!ctx)
    {
        alcCloseDevice(dev);
        return false;
    }

    alcMakeContextCurrent(ctx);
    alcProcessContext(ctx);

    bool cmdline(const char *cmd);
    if (cmdline("openalinfo"))
    {
        printf("AL_VENDOR: %s\n", (char *) alGetString(AL_VENDOR));
        printf("AL_RENDERER: %s\n", (char *) alGetString(AL_RENDERER));
        printf("AL_VERSION: %s\n", (char *) alGetString(AL_VERSION));
        printf("AL_EXTENSIONS: %s\n", (char *) alGetString(AL_EXTENSIONS));
    }

    num_channels = maxsoftwarechannels;
    channels = new OPENAL_Channels[maxsoftwarechannels];
    memset(channels, '\0', sizeof (OPENAL_Channels) * num_channels);
    for (int i = 0; i < num_channels; i++)
        alGenSources(1, &channels[i].sid);  // !!! FIXME: verify this didn't fail!

    initialized = true;
    return true;
}

void F_API OPENAL_Close()
{
    if (!initialized) return;

    ALCcontext *ctx = alcGetCurrentContext();
    if (ctx)
    {
        for (int i = 0; i < num_channels; i++)
        {
            alSourceStop(channels[i].sid);
            alSourcei(channels[i].sid, AL_BUFFER, 0);
            alDeleteSources(1, &channels[i].sid);
        }
        ALCdevice *dev = alcGetContextsDevice(ctx);
        alcMakeContextCurrent(NULL);
        alcSuspendContext(ctx);
        alcDestroyContext(ctx);
        alcCloseDevice(dev);
    }

    num_channels = 0;
    delete[] channels;
    channels = NULL;

    unload_alsyms();
    initialized = false;
}

FSOUND_SAMPLE *F_API OPENAL_GetCurrentSample(int channel)
{
    if (!initialized) return NULL;
    if ((channel < 0) || (channel >= num_channels)) return NULL;
    return channels[channel].sample;
}

signed char F_API OPENAL_GetPaused(int channel)
{
    if (!initialized) return false;
    if ((channel < 0) || (channel >= num_channels)) return false;
    if (channels[channel].startpaused)
        return(true);

    ALint state = 0;
    alGetSourceiv(channels[channel].sid, AL_SOURCE_STATE, &state);
    return((state == AL_PAUSED) ? true : false);
}

unsigned int F_API OPENAL_GetLoopMode(int channel)
{
    if (!initialized) return 0;
    if ((channel < 0) || (channel >= num_channels)) return 0;
    ALint loop = 0;
    alGetSourceiv(channels[channel].sid, AL_LOOPING, &loop);
    if (loop)
        return(FSOUND_LOOP_NORMAL);
    return FSOUND_LOOP_OFF;
}

signed char F_API OPENAL_IsPlaying(int channel)
{
    if (!initialized) return false;
    if ((channel < 0) || (channel >= num_channels)) return false;
    ALint state = 0;
    alGetSourceiv(channels[channel].sid, AL_SOURCE_STATE, &state);
    return((state == AL_PLAYING) ? true : false);
}

int F_API OPENAL_PlaySoundEx(int channel, FSOUND_SAMPLE *sptr, FSOUND_DSPUNIT *dsp, signed char startpaused)
{
    if (!initialized) return -1;
    if (sptr == NULL) return -1;
    if (dsp != NULL) return -1;
    if (channel == FSOUND_FREE)
    {
        for (int i = 0; i < num_channels; i++)
        {
            ALint state = 0;
            alGetSourceiv(channels[i].sid, AL_SOURCE_STATE, &state);
            if ((state != AL_PLAYING) && (state != AL_PAUSED))
            {
                channel = i;
                break;
            }
        }
    }

    if ((channel < 0) || (channel >= num_channels)) return -1;
    alSourceStop(channels[channel].sid);
    channels[channel].sample = sptr;
    alSourcei(channels[channel].sid, AL_BUFFER, sptr->bid);
    alSourcei(channels[channel].sid, AL_LOOPING, (sptr->mode == FSOUND_LOOP_OFF) ? AL_FALSE : AL_TRUE);
    set_channel_position(channel, 0.0f, 0.0f, 0.0f);

    channels[channel].startpaused = ((startpaused) ? true : false);
    if (!startpaused)
        alSourcePlay(channels[channel].sid);
    return channel;
}


static void *decode_to_pcm(const char *_fname, ALenum &format, ALsizei &size, ALuint &freq)
{
#ifdef __POWERPC__
    const int bigendian = 1;
#else
    const int bigendian = 0;
#endif

    // !!! FIXME: if it's not Ogg, we don't have a decoder. I'm lazy.  :/
    char *fname = (char *) alloca(strlen(_fname) + 16);
    strcpy(fname, _fname);
    char *ptr = strchr(fname, '.');
    if (ptr) *ptr = NULL;
    strcat(fname, ".ogg");

    // just in case...
    #undef fopen
    FILE *io = fopen(fname, "rb");
    if (io == NULL)
        return NULL;

    ALubyte *retval = NULL;

    #if 0  // untested, so disable this!
    // Can we just feed it to the AL compressed?
    if (alIsExtensionPresent((const ALubyte *) "AL_EXT_vorbis"))
    {
        format = alGetEnumValue((const ALubyte *) "AL_FORMAT_VORBIS_EXT");
        freq = 44100;
        fseek(io, 0, SEEK_END);
        size = ftell(io);
        fseek(io, 0, SEEK_SET);
        retval = (ALubyte *) malloc(size);
        size_t rc = fread(retval, size, 1, io);
        fclose(io);
        if (rc != 1)
        {
            free(retval);
            return NULL;
        }
        return retval;
    }
    #endif

    // Uncompress and feed to the AL.
    OggVorbis_File vf;
    memset(&vf, '\0', sizeof (vf));
    if (ov_open(io, &vf, NULL, 0) == 0)
    {
        int bitstream = 0;
        vorbis_info *info = ov_info(&vf, -1);
        size = 0;
        format = (info->channels == 1) ? AL_FORMAT_MONO16 : AL_FORMAT_STEREO16;
        freq = info->rate;

        if ((info->channels != 1) && (info->channels != 2))
        {
            ov_clear(&vf);
            return NULL;
        }

        char buf[1024 * 16];
        long rc = 0;
        size_t allocated = 64 * 1024;
        retval = (ALubyte *) malloc(allocated);
        while ( (rc = ov_read(&vf, buf, sizeof (buf), bigendian, 2, 1, &bitstream)) != 0 )
        {
            if (rc > 0)
            {
                size += rc;
                if (size >= allocated)
                {
                    allocated *= 2;
                    ALubyte *tmp = (ALubyte *) realloc(retval, allocated);
                    if (tmp == NULL)
                    {
                        free(retval);
                        retval = NULL;
                        break;
                    }
                    retval = tmp;
                }
                memcpy(retval + (size - rc), buf, rc);
            }
        }
        ov_clear(&vf);
        return retval;
    }

    fclose(io);
    return NULL;
}


FSOUND_SAMPLE * F_API OPENAL_Sample_Load(int index, const char *name_or_data, unsigned int mode, int offset, int length)
{
    if (!initialized) return NULL;
    if (index != FSOUND_FREE) return NULL;  // this is all the game does...
    if (offset != 0) return NULL;  // this is all the game does...
    if (length != 0) return NULL;  // this is all the game does...
    if ((mode != FSOUND_HW3D) && (mode != FSOUND_2D)) return NULL;  // this is all the game does...

    FSOUND_SAMPLE *retval = NULL;
    ALuint bufferName = 0;
    ALenum format = AL_NONE;
    ALsizei size = 0;
    ALuint frequency = 0;
    void *data = decode_to_pcm(name_or_data, format, size, frequency);
    if (data == NULL)
        return NULL;

    ALuint bid = 0;
    alGetError();
    alGenBuffers(1, &bid);
    if (alGetError() == AL_NO_ERROR)
    {
        alBufferData(bid, format, data, size, frequency);
        retval = new FSOUND_SAMPLE;
        retval->bid = bid;
        retval->mode = FSOUND_LOOP_OFF;
        retval->is2d = (mode == FSOUND_2D);
        retval->name = new char[strlen(name_or_data) + 1];
        if (retval->name)
            strcpy(retval->name, name_or_data);
    }

    free(data);
    return(retval);
}

void F_API OPENAL_Sample_Free(FSOUND_SAMPLE *sptr)
{
    if (!initialized) return;
    if (sptr)
    {
        for (int i = 0; i < num_channels; i++)
        {
            if (channels[i].sample == sptr)
            {
                alSourceStop(channels[i].sid);
                alSourcei(channels[i].sid, AL_BUFFER, 0);
                channels[i].sample = NULL;
            }
        }
        alDeleteBuffers(1, &sptr->bid);
        delete[] sptr->name;
        delete sptr;
    }
}

signed char F_API OPENAL_Sample_SetMode(FSOUND_SAMPLE *sptr, unsigned int mode)
{
    if (!initialized) return false;
    if ((mode != FSOUND_LOOP_NORMAL) && (mode != FSOUND_LOOP_OFF)) return false;
    if (!sptr) return false;
    sptr->mode = mode;
    return true;
}

signed char F_API OPENAL_Sample_SetMinMaxDistance(FSOUND_SAMPLE *sptr, float mindist, float maxdist)
{
    if (!initialized) return false;
    if (sptr == NULL) return false;
    sptr->min_distance = mindist;
    // we ignore maxdist. It's not really important to this game, and the
    //  FMOD docs suggest that it's worthless anyhow.

    // recalc sources to see if we need to adjust attenuation.
    for (int i = 0; i < num_channels; i++)
    {
        if (channels[i].sample == sptr)
        {
            const float *p = channels[i].position;
            set_channel_position(i, p[0], p[1], p[2]);
        }
    }

    return 0;
}

signed char F_API OPENAL_SetFrequency(int channel, int freq)
{
    if (!initialized) return false;
    if (channel == FSOUND_ALL)
    {
        for (int i = 0; i < num_channels; i++)
            OPENAL_SetFrequency(i, freq);
        return true;
    }

    if ((channel < 0) || (channel >= num_channels)) return false;
    if (freq == 8012)  // hack
        alSourcef(channels[channel].sid, AL_PITCH, 8012.0f / 44100.0f);
    else
        alSourcef(channels[channel].sid, AL_PITCH, 1.0f);
    return true;
}

signed char F_API OPENAL_SetVolume(int channel, int vol)
{
    if (!initialized) return false;

    if (channel == FSOUND_ALL)
    {
        for (int i = 0; i < num_channels; i++)
            OPENAL_SetVolume(i, vol);
        return true;
    }

    if ((channel < 0) || (channel >= num_channels)) return false;

    if (vol < 0) vol = 0;
    else if (vol > 255) vol = 255;
    ALfloat gain = ((ALfloat) vol) / 255.0f;
    alSourcef(channels[channel].sid, AL_GAIN, gain);
    return true;
}

signed char F_API OPENAL_SetPaused(int channel, signed char paused)
{
    if (!initialized) return false;

    if (channel == FSOUND_ALL)
    {
        for (int i = 0; i < num_channels; i++)
            OPENAL_SetPaused(i, paused);
        return true;
    }

    if ((channel < 0) || (channel >= num_channels)) return false;

    ALint state = 0;
    if (channels[channel].startpaused)
        state = AL_PAUSED;
    else
        alGetSourceiv(channels[channel].sid, AL_SOURCE_STATE, &state);

    if ((paused) && (state == AL_PLAYING))
        alSourcePause(channels[channel].sid);
    else if ((!paused) && (state == AL_PAUSED))
    {
        alSourcePlay(channels[channel].sid);
        channels[channel].startpaused = false;
    }
    return true;
}

void F_API OPENAL_SetSFXMasterVolume(int volume)
{
    if (!initialized) return;
    ALfloat gain = ((ALfloat) volume) / 255.0f;
    alListenerf(AL_GAIN, gain);
}

signed char F_API OPENAL_StopSound(int channel)
{
    if (!initialized) return false;

    if (channel == FSOUND_ALL)
    {
        for (int i = 0; i < num_channels; i++)
            OPENAL_StopSound(i);
        return true;
    }

    if ((channel < 0) || (channel >= num_channels)) return false;
    alSourceStop(channels[channel].sid);
    channels[channel].startpaused = false;
    return true;
}

FSOUND_STREAM * F_API OPENAL_Stream_Open(const char *name_or_data, unsigned int mode, int offset, int length)
{
    return (FSOUND_STREAM *) OPENAL_Sample_Load(FSOUND_FREE, name_or_data, mode, offset, length);
}

signed char F_API OPENAL_Stream_Close(FSOUND_STREAM *stream)
{
    OPENAL_Sample_Free((FSOUND_SAMPLE *) stream);
}

FSOUND_SAMPLE * F_API OPENAL_Stream_GetSample(FSOUND_STREAM *stream)
{
    if (!initialized) return NULL;
    return (FSOUND_SAMPLE *) stream;
}

int F_API OPENAL_Stream_PlayEx(int channel, FSOUND_STREAM *stream, FSOUND_DSPUNIT *dsp, signed char startpaused)
{
    return OPENAL_PlaySoundEx(channel, (FSOUND_SAMPLE *) stream, dsp, startpaused);
}

signed char F_API OPENAL_Stream_Stop(FSOUND_STREAM *stream)
{
    if (!initialized) return false;
    for (int i = 0; i < num_channels; i++)
    {
        if (channels[i].sample == (FSOUND_SAMPLE *) stream)
        {
            alSourceStop(channels[i].sid);
            channels[i].startpaused = false;
        }
    }
    return true;
}

signed char F_API OPENAL_Stream_SetMode(FSOUND_STREAM *stream, unsigned int mode)
{
    return OPENAL_Sample_SetMode((FSOUND_SAMPLE *) stream, mode);
}

void F_API OPENAL_Update()
{
    if (!initialized) return;
    alcProcessContext(alcGetCurrentContext());
}

signed char F_API OPENAL_SetOutput(int outputtype)
{
    return true;
}

#endif

