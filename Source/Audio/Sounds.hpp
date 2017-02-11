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

#ifndef _SOUNDS_HPP_
#define _SOUNDS_HPP_

#include "Math/XYZ.hpp"

enum sound_type
{
#define DECLARE_SOUND(id, filename) id,
#include "Sounds.def"
#undef DECLARE_SOUND
    sounds_count
};

extern struct OPENAL_SAMPLE* samp[sounds_count];
extern int channels[];

extern void loadAllSounds();

extern void addEnvSound(XYZ coords, float vol = 16, float life = .4);

extern void emit_sound_at(int soundid, const XYZ& pos = XYZ(), float vol = 256.f);
extern void emit_sound_np(int soundid, float vol = 256.f);
extern void emit_stream_at(int soundid, const XYZ& pos = XYZ(), float vol = 256.f);
extern void emit_stream_np(int soundid, float vol = 256.f);
extern void resume_stream(int soundid);
extern void pause_sound(int soundid);

extern int footstepsound, footstepsound2, footstepsound3, footstepsound4;
#endif
