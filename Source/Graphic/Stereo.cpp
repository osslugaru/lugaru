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

#include "Graphic/Stereo.hpp"

#include "Game.hpp"

extern int kContextWidth;
extern int kContextHeight;

bool CanInitStereo(StereoMode mode)
{
    GLint stencilbits = 0;

    switch (mode) {
        case stereoNone:
        case stereoAnaglyph:
            return true;
            break;
        case stereoHorizontalInterlaced:
        case stereoVerticalInterlaced:
            glGetIntegerv(GL_STENCIL_BITS, &stencilbits);
            if (stencilbits < 1) {
                fprintf(stderr, "Failed to get a stencil buffer, interlaced stereo not available.\n");
                return false;
            } else {
                fprintf(stderr, "Stencil buffer has %i bits, good.\n", stencilbits);
            }
            return true;
            break;
        default:
            return false;
    }
}

void InitStereo(StereoMode mode)
{
    switch (mode) {
        default:
        case stereoNone:
        case stereoAnaglyph:
            glDisable(GL_STENCIL_TEST);
            return;
        case stereoHorizontalInterlaced:
        case stereoVerticalInterlaced:
            fprintf(stderr, "Screen width is %i, height is %i\n", kContextWidth, kContextHeight);

            // Setup stencil buffer
            glDisable(GL_DEPTH_TEST);
            glDisable(GL_CULL_FACE);
            glDisable(GL_LIGHTING);
            glDisable(GL_TEXTURE_2D);

            glEnable(GL_STENCIL_TEST);
            glClearStencil(0);
            glClear(GL_STENCIL_BUFFER_BIT);
            glStencilFunc(GL_ALWAYS, 0x1, 0x1);
            glStencilOp(GL_REPLACE, GL_REPLACE, GL_REPLACE);

            // Setup viewport
            glViewport(0, 0, kContextWidth, kContextHeight);
            glMatrixMode(GL_PROJECTION);
            glPushMatrix();
            glLoadIdentity();
            glOrtho((GLdouble)0, (GLdouble)kContextWidth, (GLdouble)kContextHeight, 0, -1, 1);
            glMatrixMode(GL_MODELVIEW);
            glPushMatrix();
            glLoadIdentity();
            glColorMask(GL_FALSE, GL_FALSE, GL_FALSE, GL_FALSE);
            glDisable(GL_LINE_SMOOTH);

            // Add 0.5 to the coordinates, because OpenGL considers a pixel should be
            // turned on when a line passes through the center of it.
            if (mode == stereoHorizontalInterlaced) {
                for (int y = 0; y < kContextHeight; y += 2) {
                    glBegin(GL_LINES);
                    glVertex3f(0.5, y + 0.5, 0);
                    glVertex3f(kContextWidth + 0.5, y + 0.5, 0);
                    glEnd();
                }
            } else {
                for (int x = 0; x < kContextWidth; x += 2) {
                    glBegin(GL_LINES);
                    glVertex3f(x + 0.5, 0.5, 0);
                    glVertex3f(x + 0.5, kContextHeight + 0.5, 0);
                    glEnd();
                }
            }

            glColorMask(GL_TRUE, GL_TRUE, GL_TRUE, GL_TRUE);

            glPopMatrix();
            glMatrixMode(GL_PROJECTION);
            glPopMatrix();

            glStencilFunc(GL_NOTEQUAL, 0x01, 0x01);
            glStencilOp(GL_KEEP, GL_KEEP, GL_KEEP);
            glEnable(GL_DEPTH_TEST);
            glEnable(GL_CULL_FACE);
            glEnable(GL_LIGHTING);
            glEnable(GL_TEXTURE_2D);
    }
}

const std::string StereoModeName(StereoMode mode)
{
    switch (mode) {
        case stereoNone:
            return "None";
            break;
        case stereoAnaglyph:
            return "Anaglyph";
            break;
        case stereoHorizontalInterlaced:
            return "Horizontal interlacing";
            break;
        case stereoVerticalInterlaced:
            return "Vertical interlacing";
            break;
        case stereoHorizontalSplit:
            return "Horizontal split";
            break;
        case stereoVerticalSplit:
            return "Vertical split";
            break;
        case stereoOpenGL:
            return "OpenGL";
            break;
        default:
            return "(error)";
            break;
    }
}
