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

#ifndef _STEREO_HPP_
#define _STEREO_HPP_

#include <string>

enum StereoMode
{
    stereoNone,
    stereoAnaglyph,             /* red/cyan */
    stereoHorizontalInterlaced, /* some 3D monitors */
    stereoVerticalInterlaced,
    stereoHorizontalSplit, /* cross-eyed view */
    stereoVerticalSplit,
    stereoOpenGL, /* Whatever OpenGL does, if supported */
    stereoCount   /* must be last element */
};

enum StereoSide
{
    // Code multiplies by StereoSide to calculate camera offsets
    stereoLeft = -1,
    stereoCenter = 0,
    stereoRight = 1
};

extern StereoMode stereomode;
extern StereoMode newstereomode;
extern float stereoseparation;
extern bool stereoreverse;

bool CanInitStereo(StereoMode mode);
void InitStereo(StereoMode mode);
const std::string StereoModeName(StereoMode mode);

#endif
