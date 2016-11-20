/*
Copyright (C) 2003, 2010 - Wolfire Games

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

#ifndef _SKYBOX_H_
#define _SKYBOX_H_

#include "Quaternions.h"
#include "TGALoader.h"
#include "Quaternions.h"
#include "gamegl.h"
#include "Texture.h"

class SkyBox
{
public:
    Texture front, left, back, right, up, down;

    void load(const char *ffront, const char *fleft, const char *fback,
              const char *fright, const char *fup,   const char *fdown);
    void draw();

    SkyBox() {}
    ~SkyBox();
};

#endif
