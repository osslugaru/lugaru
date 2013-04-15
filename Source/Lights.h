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

#ifndef _LIGHTS_H_
#define _LIGHTS_H_


/**> HEADER FILES <**/
#include "gamegl.h"
#include "Quaternions.h"

class Light
{
public:
    GLint type;
    GLfloat color[3];
    GLfloat ambient[3];
    int attach;
    XYZ location;
    inline void setColors(GLfloat cr, GLfloat cg, GLfloat cb,
                          GLfloat ar, GLfloat ag, GLfloat ab) {
        color[0] = cr;
        color[1] = cg;
        color[2] = cb;
        ambient[0] = ar;
        ambient[1] = ag;
        ambient[2] = ab;
    }
};

void SetUpMainLight(Light* whichsource, int whichlight, float ambientr, float ambientg, float ambientb);
void SetUpLight(Light* whichsource, int whichlight);

#endif
