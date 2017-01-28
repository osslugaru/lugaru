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

#include "Environment/Lights.hpp"

void SetUpLight(Light* whichsource, int whichlight)
{
    static float qattenuation[] = { 0.0002f };

    //Initialize lights
    if (whichlight == 0) {
        GLfloat LightAmbient[] = { whichsource->ambient[0], whichsource->ambient[1], whichsource->ambient[2], 1.0f };
        GLfloat LightDiffuse[] = { whichsource->color[0], whichsource->color[1], whichsource->color[2], 1.0f };
        GLfloat LightPosition[] = { whichsource->location.x, whichsource->location.y, whichsource->location.z, 0.0f };

        glLightfv(GL_LIGHT0, GL_POSITION, LightPosition);
        glLightfv(GL_LIGHT0, GL_AMBIENT, LightAmbient);
        glLightfv(GL_LIGHT0, GL_DIFFUSE, LightDiffuse);
        glEnable(GL_LIGHT0);
    } else {
        GLenum lightselect = GL_LIGHT1;
        switch (whichlight) {
            case 2:
                lightselect = GL_LIGHT2;
                break;
            case 3:
                lightselect = GL_LIGHT3;
                break;
            case 4:
                lightselect = GL_LIGHT4;
                break;
            case 5:
                lightselect = GL_LIGHT5;
                break;
            case 6:
                lightselect = GL_LIGHT6;
                break;
            case 7:
                lightselect = GL_LIGHT7;
                break;
        }

        GLfloat LightAmbient[] = { 0, 0, 0, 1.0f };
        GLfloat LightDiffuse[] = { whichsource->color[0], whichsource->color[1], whichsource->color[2], 1.0f };
        GLfloat LightPosition[] = { whichsource->location.x, whichsource->location.y, whichsource->location.z, 1.0f };

        glLightfv(lightselect, GL_QUADRATIC_ATTENUATION, qattenuation);
        glLightfv(lightselect, GL_POSITION, LightPosition);
        glLightfv(lightselect, GL_AMBIENT, LightAmbient);
        glLightfv(lightselect, GL_DIFFUSE, LightDiffuse);
        glEnable(lightselect);
    }
}
