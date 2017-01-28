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

#include "Graphic/Text.hpp"

#include "Game.hpp"

void Text::LoadFontTexture(const std::string& fileName)
{
    LOGFUNC;

    LOG(std::string("Loading font texture...") + fileName);

    FontTexture.load(fileName, false);
    if (base) {
        glDeleteLists(base, 512);
        base = 0;
    }
}

void Text::BuildFont() // Build Our Font Display List
{
    float cx; // Holds Our X Character Coord
    float cy; // Holds Our Y Character Coord
    int loop;

    LOGFUNC;

    if (base) {
        glDeleteLists(base, 512);
        base = 0;
    }

    base = glGenLists(512); // Creating 256 Display Lists
    FontTexture.bind();
    for (loop = 0; loop < 512; loop++) { // Loop Through All 256 Lists
        if (loop < 256) {
            cx = float(loop % 16) / 16.0f; // X Position Of Current Character
            cy = float(loop / 16) / 16.0f; // Y Position Of Current Character
        } else {
            cx = float((loop - 256) % 16) / 16.0f; // X Position Of Current Character
            cy = float((loop - 256) / 16) / 16.0f; // Y Position Of Current Character
        }
        glNewList(base + loop, GL_COMPILE);                  // Start Building A List
        glBegin(GL_QUADS);                                   // Use A Quad For Each Character
        glTexCoord2f(cx, 1 - cy - 0.0625f + .001);           // Texture Coord (Bottom Left)
        glVertex2i(0, 0);                                    // Vertex Coord (Bottom Left)
        glTexCoord2f(cx + 0.0625f, 1 - cy - 0.0625f + .001); // Texture Coord (Bottom Right)
        glVertex2i(16, 0);                                   // Vertex Coord (Bottom Right)
        glTexCoord2f(cx + 0.0625f, 1 - cy - .001);           // Texture Coord (Top Right)
        glVertex2i(16, 16);                                  // Vertex Coord (Top Right)
        glTexCoord2f(cx, 1 - cy - +.001);                    // Texture Coord (Top Left)
        glVertex2i(0, 16);                                   // Vertex Coord (Top Left)
        glEnd();                                             // Done Building Our Quad (Character)
        if (loop < 256) {
            glTranslated(10, 0, 0); // Move To The Right Of The Character
        } else {
            glTranslated(8, 0, 0); // Move To The Right Of The Character
        }
        glEndList(); // Done Building The Display List
    }                // Loop Until All 256 Are Built
}

void Text::_glPrint(float x, float y, const std::string& string, int set, float size, float width, float height, int start, int end, int offset) // Where The Printing Happens
{
    if (set > 1) {
        set = 1;
    }
    glTexEnvi(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE);
    FontTexture.bind();
    glDisable(GL_DEPTH_TEST);
    glDisable(GL_LIGHTING);
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    glMatrixMode(GL_PROJECTION);
    glPushMatrix();
    glLoadIdentity();
    glOrtho(0, width, 0, height, -100, 100);
    glMatrixMode(GL_MODELVIEW);
    glPushMatrix();
    glLoadIdentity();
    glTranslated(x, y, 0);
    glScalef(size, size, 1);
    glListBase(base - 32 + (128 * set) + offset);      // Choose The Font Set (0 or 1)
    glCallLists(end - start, GL_BYTE, &string[start]); // Write The Text To The Screen
    glMatrixMode(GL_PROJECTION);
    glPopMatrix();
    glMatrixMode(GL_MODELVIEW);
    glPopMatrix();
    glEnable(GL_DEPTH_TEST);
    glTexEnvi(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE);
}

void Text::glPrint(float x, float y, const std::string& string, int set, float size, float width, float height, int start, int end)
{
    if (end < 0) {
        end = string.size();
    }
    _glPrint(x, y, string, set, size, width, height, start, end, 0);
}

void Text::glPrintOutline(float x, float y, const std::string& string, int set, float size, float width, float height, int start, int end)
{
    if (end < 0) {
        end = string.size();
    }
    _glPrint(x, y, string, set, size, width, height, start, end, 256);
}

void Text::glPrintOutlined(float x, float y, const std::string& string, int set, float size, float width, float height, int start, int end)
{
    glPrintOutlined(1, 1, 1, 1, x, y, string, set, size, width, height, start, end);
}

void Text::glPrintOutlined(float r, float g, float b, float a, float x, float y, const std::string& string, int set, float size, float width, float height, int start, int end)
{
    glColor4f(0, 0, 0, a);
    glPrintOutline(x - 2 * size, y - 2 * size, string, set, size * 2.5 / 2, width, height, start, end);
    glColor4f(r, g, b, a);
    glPrint(x, y, string, set, size, width, height, start, end);
}

Text::Text()
{
    base = 0;
}

Text::~Text()
{
    if (base) {
        glDeleteLists(base, 512);
        base = 0;
    }
}
