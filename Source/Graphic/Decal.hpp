/*
Copyright (C) 2003, 2010 - Wolfire Games
Copyright (C) 2010-2016 - Lugaru contributors (see AUTHORS file)

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

#ifndef _DECAL_HPP_
#define _DECAL_HPP_

class Terrain;

#include "Math/Quaternions.hpp"

class Decal
{
public:
    XYZ position;
    int type;
    float opacity;
    float rotation;
    float alivetime;
    float brightness;

    float texcoords[3][2];
    XYZ vertex[3];

    Decal();
    Decal(XYZ position, int type, float opacity, float rotation, float brightness, int whichx, int whichy, float size, const Terrain& terrain, bool first);
};

#endif
