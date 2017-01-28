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

#include "Environment/Skybox.hpp"

#include "Game.hpp"

extern float viewdistance;
extern float blurness;
extern int environment;
extern bool skyboxtexture;
extern float skyboxr;
extern float skyboxg;
extern float skyboxb;

void SkyBox::load(const std::string& ffront, const std::string& fleft, const std::string& fback,
                  const std::string& fright, const std::string& fup, const std::string& fdown)
{
    front.load(ffront, true);
    left.load(fleft, true);
    back.load(fback, true);
    right.load(fright, true);
    up.load(fup, true);
    down.load(fdown, true);
}

void SkyBox::draw()
{
    static float size = viewdistance / 4;
    glPushMatrix();
    static GLfloat M[16];
    glGetFloatv(GL_MODELVIEW_MATRIX, M);
    M[12] = 0;
    M[13] = 0;
    M[14] = 0;
    glLoadMatrixf(M);
    if (environment == desertenvironment) {
        glScalef(1 + blurness / 1000, 1, 1 + blurness / 1000);
        glColor3f(1 * skyboxr, .95 * skyboxg, .95 * skyboxb);
    } else {
        glColor3f(.85 * skyboxr, .85 * skyboxg, .95 * skyboxb);
    }

    if (!skyboxtexture) {
        glDisable(GL_TEXTURE_2D);
        glColor3f(skyboxr * .8, skyboxg * .8, skyboxb * .8);
    }
    glDepthMask(0);
    glDisable(GL_CULL_FACE);
    glEnable(GL_BLEND);
    glDisable(GL_LIGHTING);
    if (skyboxtexture) {
        glEnable(GL_TEXTURE_2D);
    }
    glTexEnvi(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE);
    front.bind();
    glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glBegin(GL_QUADS);
    glNormal3f(0.0f, 0.0f, -1);
    glTexCoord2f(0, 0);
    glVertex3f(-size, -size, size);
    glTexCoord2f(1, 0);
    glVertex3f(size, -size, size);
    glTexCoord2f(1, 1);
    glVertex3f(size, size, size);
    glTexCoord2f(0, 1);
    glVertex3f(-size, size, size);
    glEnd();
    back.bind();
    glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glBegin(GL_QUADS);
    glNormal3f(0.0f, 0.0f, 1);
    glTexCoord2f(1, 0);
    glVertex3f(-size, -size, -size);
    glTexCoord2f(1, 1);
    glVertex3f(-size, size, -size);
    glTexCoord2f(0, 1);
    glVertex3f(size, size, -size);
    glTexCoord2f(0, 0);
    glVertex3f(size, -size, -size);
    glEnd();
    up.bind();
    glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glBegin(GL_QUADS);
    glNormal3f(0.0f, -1.0f, 0);
    glTexCoord2f(0, 1);
    glVertex3f(-size, size, -size);
    glTexCoord2f(0, 0);
    glVertex3f(-size, size, size);
    glTexCoord2f(1, 0);
    glVertex3f(size, size, size);
    glTexCoord2f(1, 1);
    glVertex3f(size, size, -size);
    glEnd();
    down.bind();
    glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glBegin(GL_QUADS);
    glNormal3f(0.0f, 1.0f, 0);

    glTexCoord2f(0, 0);
    glVertex3f(-size, -size, -size);
    glTexCoord2f(1, 0);
    glVertex3f(size, -size, -size);
    glTexCoord2f(1, 1);
    glVertex3f(size, -size, size);
    glTexCoord2f(0, 1);
    glVertex3f(-size, -size, size);
    glEnd();
    right.bind();
    glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glBegin(GL_QUADS);
    glNormal3f(-1.0f, 0.0f, 0);
    glTexCoord2f(1, 0);
    glVertex3f(size, -size, -size);
    glTexCoord2f(1, 1);
    glVertex3f(size, size, -size);
    glTexCoord2f(0, 1);
    glVertex3f(size, size, size);
    glTexCoord2f(0, 0);
    glVertex3f(size, -size, size);
    glEnd();
    left.bind();
    glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glBegin(GL_QUADS);
    glNormal3f(1.0f, 0.0f, 0);
    glTexCoord2f(0, 0);
    glVertex3f(-size, -size, -size);
    glTexCoord2f(1, 0);
    glVertex3f(-size, -size, size);
    glTexCoord2f(1, 1);
    glVertex3f(-size, size, size);
    glTexCoord2f(0, 1);
    glVertex3f(-size, size, -size);
    glEnd();
    glEnable(GL_CULL_FACE);
    glDepthMask(1);
    glPopMatrix();
}
