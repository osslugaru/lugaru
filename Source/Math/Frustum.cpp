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

#include "Math/Frustum.hpp"

#include "Graphic/gamegl.hpp"

#include <math.h>

void FRUSTUM::
    GetFrustum()
{
    static float projmatrix[16];
    static float mvmatrix[16];
    static float clip[16];

    glGetFloatv(GL_PROJECTION_MATRIX, projmatrix);
    glGetFloatv(GL_MODELVIEW_MATRIX, mvmatrix);

    // Combine the matrices
    clip[0] = mvmatrix[0] * projmatrix[0] + mvmatrix[1] * projmatrix[4] + mvmatrix[2] * projmatrix[8] + mvmatrix[3] * projmatrix[12];
    clip[1] = mvmatrix[0] * projmatrix[1] + mvmatrix[1] * projmatrix[5] + mvmatrix[2] * projmatrix[9] + mvmatrix[3] * projmatrix[13];
    clip[2] = mvmatrix[0] * projmatrix[2] + mvmatrix[1] * projmatrix[6] + mvmatrix[2] * projmatrix[10] + mvmatrix[3] * projmatrix[14];
    clip[3] = mvmatrix[0] * projmatrix[3] + mvmatrix[1] * projmatrix[7] + mvmatrix[2] * projmatrix[11] + mvmatrix[3] * projmatrix[15];

    clip[4] = mvmatrix[4] * projmatrix[0] + mvmatrix[5] * projmatrix[4] + mvmatrix[6] * projmatrix[8] + mvmatrix[7] * projmatrix[12];
    clip[5] = mvmatrix[4] * projmatrix[1] + mvmatrix[5] * projmatrix[5] + mvmatrix[6] * projmatrix[9] + mvmatrix[7] * projmatrix[13];
    clip[6] = mvmatrix[4] * projmatrix[2] + mvmatrix[5] * projmatrix[6] + mvmatrix[6] * projmatrix[10] + mvmatrix[7] * projmatrix[14];
    clip[7] = mvmatrix[4] * projmatrix[3] + mvmatrix[5] * projmatrix[7] + mvmatrix[6] * projmatrix[11] + mvmatrix[7] * projmatrix[15];

    clip[8] = mvmatrix[8] * projmatrix[0] + mvmatrix[9] * projmatrix[4] + mvmatrix[10] * projmatrix[8] + mvmatrix[11] * projmatrix[12];
    clip[9] = mvmatrix[8] * projmatrix[1] + mvmatrix[9] * projmatrix[5] + mvmatrix[10] * projmatrix[9] + mvmatrix[11] * projmatrix[13];
    clip[10] = mvmatrix[8] * projmatrix[2] + mvmatrix[9] * projmatrix[6] + mvmatrix[10] * projmatrix[10] + mvmatrix[11] * projmatrix[14];
    clip[11] = mvmatrix[8] * projmatrix[3] + mvmatrix[9] * projmatrix[7] + mvmatrix[10] * projmatrix[11] + mvmatrix[11] * projmatrix[15];

    clip[12] = mvmatrix[12] * projmatrix[0] + mvmatrix[13] * projmatrix[4] + mvmatrix[14] * projmatrix[8] + mvmatrix[15] * projmatrix[12];
    clip[13] = mvmatrix[12] * projmatrix[1] + mvmatrix[13] * projmatrix[5] + mvmatrix[14] * projmatrix[9] + mvmatrix[15] * projmatrix[13];
    clip[14] = mvmatrix[12] * projmatrix[2] + mvmatrix[13] * projmatrix[6] + mvmatrix[14] * projmatrix[10] + mvmatrix[15] * projmatrix[14];
    clip[15] = mvmatrix[12] * projmatrix[3] + mvmatrix[13] * projmatrix[7] + mvmatrix[14] * projmatrix[11] + mvmatrix[15] * projmatrix[15];

    // Right plane
    frustum[0][0] = clip[3] - clip[0];
    frustum[0][1] = clip[7] - clip[4];
    frustum[0][2] = clip[11] - clip[8];
    frustum[0][3] = clip[15] - clip[12];

    // Left plane
    frustum[1][0] = clip[3] + clip[0];
    frustum[1][1] = clip[7] + clip[4];
    frustum[1][2] = clip[11] + clip[8];
    frustum[1][3] = clip[15] + clip[12];

    // Bottom plane
    frustum[2][0] = clip[3] + clip[1];
    frustum[2][1] = clip[7] + clip[5];
    frustum[2][2] = clip[11] + clip[9];
    frustum[2][3] = clip[15] + clip[13];

    // Top plane
    frustum[3][0] = clip[3] - clip[1];
    frustum[3][1] = clip[7] - clip[5];
    frustum[3][2] = clip[11] - clip[9];
    frustum[3][3] = clip[15] - clip[13];

    // Far plane
    frustum[4][0] = clip[3] - clip[2];
    frustum[4][1] = clip[7] - clip[6];
    frustum[4][2] = clip[11] - clip[10];
    frustum[4][3] = clip[15] - clip[14];

    // Near plane
    frustum[5][0] = clip[3] + clip[2];
    frustum[5][1] = clip[7] + clip[6];
    frustum[5][2] = clip[11] + clip[10];
    frustum[5][3] = clip[15] + clip[14];
}

int FRUSTUM::
    CubeInFrustum(float x, float y, float z, float size)
{
    static int c, c2;

    for (int i = 0; i < 6; i++) {
        c = 0;
        if (frustum[i][0] * (x - size) + frustum[i][1] * (y - size) + frustum[i][2] * (z - size) + frustum[i][3] > 0) {
            c++;
        }
        if (frustum[i][0] * (x + size) + frustum[i][1] * (y - size) + frustum[i][2] * (z - size) + frustum[i][3] > 0) {
            c++;
        }
        if (frustum[i][0] * (x - size) + frustum[i][1] * (y + size) + frustum[i][2] * (z - size) + frustum[i][3] > 0) {
            c++;
        }
        if (frustum[i][0] * (x + size) + frustum[i][1] * (y + size) + frustum[i][2] * (z - size) + frustum[i][3] > 0) {
            c++;
        }
        if (frustum[i][0] * (x - size) + frustum[i][1] * (y - size) + frustum[i][2] * (z + size) + frustum[i][3] > 0) {
            c++;
        }
        if (frustum[i][0] * (x + size) + frustum[i][1] * (y - size) + frustum[i][2] * (z + size) + frustum[i][3] > 0) {
            c++;
        }
        if (frustum[i][0] * (x - size) + frustum[i][1] * (y + size) + frustum[i][2] * (z + size) + frustum[i][3] > 0) {
            c++;
        }
        if (frustum[i][0] * (x + size) + frustum[i][1] * (y + size) + frustum[i][2] * (z + size) + frustum[i][3] > 0) {
            c++;
        }
        if (c == 0) {
            return 0;
        }
        if (c == 8) {
            c2++;
        }
    }
    if (c2 >= 6) {
        return 2;
    } else {
        return 1;
    }
}

int FRUSTUM::
    CubeInFrustum(float x, float y, float z, float size, float height)
{
    static int c, c2;

    for (int i = 0; i < 6; i++) {
        c = 0;
        if (frustum[i][0] * (x - size) + frustum[i][1] * (y - height) + frustum[i][2] * (z - size) + frustum[i][3] > 0) {
            c++;
        }
        if (frustum[i][0] * (x + size) + frustum[i][1] * (y - height) + frustum[i][2] * (z - size) + frustum[i][3] > 0) {
            c++;
        }
        if (frustum[i][0] * (x - size) + frustum[i][1] * (y + height) + frustum[i][2] * (z - size) + frustum[i][3] > 0) {
            c++;
        }
        if (frustum[i][0] * (x + size) + frustum[i][1] * (y + height) + frustum[i][2] * (z - size) + frustum[i][3] > 0) {
            c++;
        }
        if (frustum[i][0] * (x - size) + frustum[i][1] * (y - height) + frustum[i][2] * (z + size) + frustum[i][3] > 0) {
            c++;
        }
        if (frustum[i][0] * (x + size) + frustum[i][1] * (y - height) + frustum[i][2] * (z + size) + frustum[i][3] > 0) {
            c++;
        }
        if (frustum[i][0] * (x - size) + frustum[i][1] * (y + height) + frustum[i][2] * (z + size) + frustum[i][3] > 0) {
            c++;
        }
        if (frustum[i][0] * (x + size) + frustum[i][1] * (y + height) + frustum[i][2] * (z + size) + frustum[i][3] > 0) {
            c++;
        }
        if (c == 0) {
            return 0;
        }
        if (c == 8) {
            c2++;
        }
    }
    if (c2 >= 6) {
        return 2;
    } else {
        return 1;
    }
}

int FRUSTUM::
    SphereInFrustum(float x, float y, float z, float radius)
{
    static int c2;

    for (int i = 0; i < 6; i++) {
        if (frustum[i][0] * x + frustum[i][1] * y + frustum[i][2] * z + frustum[i][3] > -1 * radius) {
            c2++;
        } else {
            return 0;
        }
    }
    if (c2 >= 6) {
        return 2;
    } else {
        return 1;
    }
}
