
#if USE_OPENAL

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

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
} OPENAL_Channels;

typedef struct FSOUND_SAMPLE
{
    char *name;
    ALuint bid;  // buffer id.
    int mode;
    int is2d;
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


void F_API OPENAL_3D_Listener_SetAttributes(const float *pos, const float *vel, float fx, float fy, float fz, float tx, float ty, float tz)
{
    if (!initialized) return;
    if (pos != NULL)
        alListener3f(AL_POSITION, pos[0], pos[1], -pos[2]);
    ALfloat vec[6] = { fx, fy, -fz, tz, ty, -tz };
    alListenerfv(AL_ORIENTATION, vec);

    // we ignore velocity, since doppler's broken in the Linux AL at the moment...
}

signed char F_API OPENAL_3D_SetAttributes(int channel, const float *pos, const float *vel)
{
    if (!initialized) return FALSE;
    if ((channel < 0) || (channel >= num_channels)) return FALSE;

    if ((pos != NULL) && (!channels[channel].sample->is2d))
        alSource3f(channels[channel].sid, AL_POSITION, pos[0], pos[1], -pos[2]);

    // we ignore velocity, since doppler's broken in the Linux AL at the moment...
    return TRUE;
}

void F_API OPENAL_3D_SetDopplerFactor(float scale)
{
    if (!initialized) return;
    // unimplemented...looks like init routines just call this with scale == 0.0f anyhow.
}

signed char F_API OPENAL_Init(int mixrate, int maxsoftwarechannels, unsigned int flags)
{
    if (initialized) return FALSE;
    if (maxsoftwarechannels == 0) return FALSE;

    if (flags != 0)  // unsupported.
        return FALSE;

    if (!lookup_all_alsyms("./openal.so"))  // !!! FIXME: linux specific lib name
    {
        if (!lookup_all_alsyms("openal.so.1"))  // !!! FIXME: linux specific lib name
        {
            if (!lookup_all_alsyms("openal.so"))  // !!! FIXME: linux specific lib name
                return FALSE;
        }
    }

    ALCdevice *dev = alcOpenDevice(NULL);
    if (!dev)
        return FALSE;

    ALint caps[] = { ALC_FREQUENCY, mixrate, 0 };
    ALCcontext *ctx = alcCreateContext(dev, caps);
    if (!ctx)
    {
        alcCloseDevice(dev);
        return FALSE;
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
    return TRUE;
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
    if (!initialized) return FALSE;
    if ((channel < 0) || (channel >= num_channels)) return FALSE;
    if (channels[channel].startpaused)
        return(TRUE);

    ALint state = 0;
    alGetSourceiv(channels[channel].sid, AL_SOURCE_STATE, &state);
    return((state == AL_PAUSED) ? TRUE : FALSE);
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
    if (!initialized) return FALSE;
    if ((channel < 0) || (channel >= num_channels)) return FALSE;
    ALint state = 0;
    alGetSourceiv(channels[channel].sid, AL_SOURCE_STATE, &state);
    return((state == AL_PLAYING) ? TRUE : FALSE);
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
    alSourcei(channels[channel].sid, AL_SOURCE_RELATIVE, (sptr->is2d) ? AL_TRUE : AL_FALSE);
    alSource3f(channels[channel].sid, AL_POSITION, 0.0f, 0.0f, 0.0f);

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
    if (!initialized) return FALSE;
    if ((mode != FSOUND_LOOP_NORMAL) && (mode != FSOUND_LOOP_OFF)) return FALSE;
    if (!sptr) return FALSE;
    sptr->mode = mode;
    return TRUE;
}

signed char F_API OPENAL_Sample_SetMinMaxDistance(FSOUND_SAMPLE *sptr, float min, float max)
{
    if (!initialized) return FALSE;
    if (sptr == NULL) return FALSE;
    // !!! FIXME: write me
    return 0;
}

signed char F_API OPENAL_SetFrequency(int channel, int freq)
{
    if (!initialized) return FALSE;
    if (channel == FSOUND_ALL)
    {
        for (int i = 0; i < num_channels; i++)
            OPENAL_SetFrequency(i, freq);
        return TRUE;
    }

    if ((channel < 0) || (channel >= num_channels)) return FALSE;
    if (freq == 8012)  // hack
        alSourcef(channels[channel].sid, AL_PITCH, 8012.0f / 44100.0f);
    else
        alSourcef(channels[channel].sid, AL_PITCH, 1.0f);
    return TRUE;
}

signed char F_API OPENAL_SetVolume(int channel, int vol)
{
    if (!initialized) return FALSE;

    if (channel == FSOUND_ALL)
    {
        for (int i = 0; i < num_channels; i++)
            OPENAL_SetVolume(i, vol);
        return TRUE;
    }

    if ((channel < 0) || (channel >= num_channels)) return FALSE;

    if (vol < 0) vol = 0;
    else if (vol > 255) vol = 255;
    ALfloat gain = ((ALfloat) vol) / 255.0f;
    alSourcef(channels[channel].sid, AL_GAIN, gain);
    return TRUE;
}

signed char F_API OPENAL_SetPaused(int channel, signed char paused)
{
    if (!initialized) return FALSE;

    if (channel == FSOUND_ALL)
    {
        for (int i = 0; i < num_channels; i++)
            OPENAL_SetPaused(i, paused);
        return TRUE;
    }

    if ((channel < 0) || (channel >= num_channels)) return FALSE;

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
    return TRUE;
}

void F_API OPENAL_SetSFXMasterVolume(int volume)
{
    if (!initialized) return;
    ALfloat gain = ((ALfloat) volume) / 255.0f;
    alListenerf(AL_GAIN, gain);
}

signed char F_API OPENAL_StopSound(int channel)
{
    if (!initialized) return FALSE;

    if (channel == FSOUND_ALL)
    {
        for (int i = 0; i < num_channels; i++)
            OPENAL_StopSound(i);
        return TRUE;
    }

    if ((channel < 0) || (channel >= num_channels)) return FALSE;
    alSourceStop(channels[channel].sid);
    channels[channel].startpaused = false;
    return TRUE;
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
    if (!initialized) return FALSE;
    for (int i = 0; i < num_channels; i++)
    {
        if (channels[i].sample == (FSOUND_SAMPLE *) stream)
        {
            alSourceStop(channels[i].sid);
            channels[i].startpaused = false;
        }
    }
    return TRUE;
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
    return TRUE;
}

#endif

