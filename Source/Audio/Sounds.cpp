/*
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

#include "Audio/Sounds.hpp"

#include "Audio/openal_wrapper.hpp"
#include "Utils/Folders.hpp"

struct OPENAL_SAMPLE* samp[sounds_count];

extern XYZ envsound[30];
extern float envsoundvol[30];
extern int numenvsounds;
extern float envsoundlife[30];

int footstepsound, footstepsound2, footstepsound3, footstepsound4;

int channels[100];

static const char* sound_data[sounds_count] = {
#define DECLARE_SOUND(id, filename) filename,
#include "Sounds.def"
#undef DECLARE_SOUND
};

// FIXME: dimensionality is not a property of the sound sample.
// This should be decided at the time of playback
static int snd_mode(int snd)
{
    switch (snd) {
        case alarmsound:
        case consolefailsound:
        case consolesuccesssound:
        case firestartsound:
        case fireendsound:
            return OPENAL_2D;
        default:
            return OPENAL_HW3D;
    }
}

void loadAllSounds()
{
    for (int i = 0; i < sounds_count; i++) {
        std::string buf = std::string("Sounds/") + sound_data[i];
        samp[i] = OPENAL_Sample_Load(OPENAL_FREE,
                                     Folders::getResourcePath(buf).c_str(),
                                     snd_mode(i),
                                     0, 0);
    }
    footstepsound = footstepsn1;
    footstepsound2 = footstepsn2;
    footstepsound3 = footstepst1;
    footstepsound4 = footstepst2;
    // Huh?
    // OPENAL_Sample_SetMode(samp[whooshsound], OPENAL_LOOP_NORMAL);
    for (int i = stream_firesound; i <= stream_menutheme; i++) {
        OPENAL_Stream_SetMode(samp[i], OPENAL_LOOP_NORMAL);
    }
}

void addEnvSound(XYZ coords, float vol, float life)
{
    envsound[numenvsounds] = coords;
    envsoundvol[numenvsounds] = vol;
    envsoundlife[numenvsounds] = life;
    numenvsounds++;
}

void emit_sound_at(int soundid, const XYZ& pos, float vol)
{
    PlaySoundEx(soundid, samp[soundid], NULL, true);
    OPENAL_3D_SetAttributes_(channels[soundid], pos);
    OPENAL_SetVolume(channels[soundid], vol);
    OPENAL_SetPaused(channels[soundid], false);
}

void emit_sound_np(int soundid, float vol)
{
    PlaySoundEx(soundid, samp[soundid], NULL, true);
    OPENAL_SetVolume(channels[soundid], vol);
    OPENAL_SetPaused(channels[soundid], false);
}

void emit_stream_at(int soundid, const XYZ& pos, float vol)
{
    PlayStreamEx(soundid, samp[soundid], NULL, true);
    OPENAL_3D_SetAttributes_(channels[soundid], pos);
    OPENAL_SetVolume(channels[soundid], vol);
    OPENAL_SetPaused(channels[soundid], false);
}

void emit_stream_np(int soundid, float vol)
{
    PlayStreamEx(soundid, samp[soundid], NULL, true);
    OPENAL_SetVolume(channels[soundid], vol);
    OPENAL_SetPaused(channels[soundid], false);
}

void resume_stream(int soundid)
{
    OPENAL_SetPaused(channels[soundid], false);
}

void pause_sound(int soundid)
{
    OPENAL_SetPaused(channels[soundid], true);
}
