/*
Copyright (C) 2010 - Lugaru authors

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

#include "Quaternions.h"
#include "Sounds.h"
#include "openal_wrapper.h"

struct OPENAL_SAMPLE *samp[sounds_count];

int footstepsound, footstepsound2, footstepsound3, footstepsound4;

int channels[100];

static const char *sound_data[sounds_count] = {
#define DECLARE_SOUND(id, filename) filename,
#include "Sounds.def"
#undef DECLARE_SOUND
};

// FIXME: dimensionality is not a property of the sound sample.
// This should be decided at the time of playback
static int snd_mode(int snd)
{
  switch (snd)
    {
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
  for (int i = 0; i < sounds_count; i++)
    {
      char buf[64];
      snprintf(buf, 64, ":Data:Sounds:%s", sound_data[i]);
      samp[i] = OPENAL_Sample_Load(OPENAL_FREE,
				   ConvertFileName(buf),
				   snd_mode(i),
				   0, 0);
    }
  footstepsound = footstepsn1;
  footstepsound2 = footstepsn2;
  footstepsound3 = footstepst1;
  footstepsound4 = footstepst2;
  // Huh?
  // OPENAL_Sample_SetMode(samp[whooshsound], OPENAL_LOOP_NORMAL);
  for (int i = stream_firesound; i <= stream_music3; i++)
    OPENAL_Stream_SetMode(samp[i], OPENAL_LOOP_NORMAL);
}

void
emit_sound_at(int soundid, const XYZ &pos, float vol)
{
  PlaySoundEx (soundid, samp[soundid], NULL, true);
  OPENAL_3D_SetAttributes_ (channels[soundid], pos, NULL);
  OPENAL_SetVolume (channels[soundid], vol);
  OPENAL_SetPaused (channels[soundid], false);
}

void
emit_sound_np(int soundid, float vol)
{
  PlaySoundEx (soundid, samp[soundid], NULL, true);
  OPENAL_SetVolume (channels[soundid], vol);
  OPENAL_SetPaused (channels[soundid], false);
}

void
emit_stream_at(int soundid, const XYZ &pos, float vol)
{
  PlayStreamEx (soundid, samp[soundid], NULL, true);
  OPENAL_3D_SetAttributes_ (channels[soundid], pos, NULL);
  OPENAL_SetVolume (channels[soundid], vol);
  OPENAL_SetPaused (channels[soundid], false);
}

void
emit_stream_np(int soundid, float vol)
{
  PlayStreamEx (soundid, samp[soundid], NULL, true);
  OPENAL_SetVolume (channels[soundid], vol);
  OPENAL_SetPaused (channels[soundid], false);
}

