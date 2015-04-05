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

#ifndef STEREO_H_
#define STEREO_H_

enum StereoMode {
    stereoNone,
    stereoAnaglyph,             /* red/cyan */
    stereoHorizontalInterlaced, /* some 3D monitors */
    stereoVerticalInterlaced,
    stereoHorizontalSplit,      /* cross-eyed view */
    stereoVerticalSplit,
    stereoOpenGL,               /* Whatever OpenGL does, if supported */
    stereoCount                 /* must be last element */
};


enum StereoSide {
    // Code multiplies by StereoSide to calculate camera offsets
    stereoLeft   = -1,
    stereoCenter = 0,
    stereoRight  = 1
};

extern StereoMode stereomode;
extern StereoMode newstereomode;
extern float stereoseparation;
extern bool  stereoreverse;

bool CanInitStereo(StereoMode mode);
void InitStereo(StereoMode mode);
const char* StereoModeName(StereoMode mode);

#endif