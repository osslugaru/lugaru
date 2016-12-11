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

#include "Game.h"
#include "Environment/Terrain.h"
#include "Objects/Objects.h"
#include "Utils/Folders.h"

extern XYZ viewer;
extern float viewdistance;
extern float fadestart;
extern int environment;
extern float texscale;
extern Light light;
extern float multiplier;
extern FRUSTUM frustum;
extern float texdetail;
extern int detail;
extern bool decals;
extern float blurness;
extern float targetblurness;
extern Objects objects;
extern bool visibleloading;
extern bool skyboxtexture;
extern int tutoriallevel;

//Functions

int Terrain::lineTerrain(XYZ p1, XYZ p2, XYZ *p)
{
    static int i, j, k;
    static float distance;
    static float olddistance;
    static int intersecting;
    static int firstintersecting;
    static XYZ point;
    static int startx, starty;
    static int endx, endy;
    static float highest, lowest;

    firstintersecting = -1;
    olddistance = 10000;
    distance = 1;

    XYZ triangles[3];

    p1 /= scale;
    p2 /= scale;

    startx = p1.x;
    starty = p1.z;
    endx = p2.x;
    endy = p2.z;

    if (startx > endx) {
        i = endx;
        endx = startx;
        startx = i;
    }
    if (starty > endy) {
        i = endy;
        endy = starty;
        starty = i;
    }

    if (startx < 0)
        startx = 0;
    if (starty < 0)
        starty = 0;
    if (endx > size - 1)
        endx = size - 1;
    if (endy > size - 1)
        endy = size - 1;

    for (i = startx; i <= endx; i++) {
        for (j = starty; j <= endy; j++) {
            highest = -1000;
            lowest = 1000;
            for (k = 0; k < 2; k++) {
                if (heightmap[i + k][j] > highest)
                    highest = heightmap[i + k][j];
                if (heightmap[i + k][j] < lowest)
                    lowest = heightmap[i + k][j];
                if (heightmap[i + k][j + 1] > highest)
                    highest = heightmap[i + k][j + 1];
                if (heightmap[i + k][j + 1] < lowest)
                    lowest = heightmap[i + k][j + 1];
            }
            if ((p1.y <= highest || p2.y <= highest) && (p1.y >= lowest || p2.y >= lowest)) {
                triangles[0].x = i;
                triangles[0].y = heightmap[i][j];
                triangles[0].z = j;

                triangles[1].x = i;
                triangles[1].y = heightmap[i][j + 1];
                triangles[1].z = j + 1;

                triangles[2].x = i + 1;
                triangles[2].y = heightmap[i + 1][j];
                triangles[2].z = j;

                intersecting = LineFacet(p1, p2, triangles[0], triangles[1], triangles[2], &point);
                distance = distsq(&p1, &point);
                if ((distance < olddistance || firstintersecting == -1) && intersecting == 1) {
                    olddistance = distance;
                    firstintersecting = 1;
                    *p = point;
                }

                triangles[0].x = i + 1;
                triangles[0].y = heightmap[i + 1][j];
                triangles[0].z = j;

                triangles[1].x = i;
                triangles[1].y = heightmap[i][j + 1];
                triangles[1].z = j + 1;

                triangles[2].x = i + 1;
                triangles[2].y = heightmap[i + 1][j + 1];
                triangles[2].z = j + 1;

                intersecting = LineFacet(p1, p2, triangles[0], triangles[1], triangles[2], &point);
                distance = distsq(&p1, &point);
                if ((distance < olddistance || firstintersecting == -1) && intersecting == 1) {
                    olddistance = distance;
                    firstintersecting = 1;
                    *p = point;
                }
            }
        }
    }
    return firstintersecting;
}

void Terrain::UpdateTransparency(int whichx, int whichy)
{
    static XYZ vertex;
    static int i, j, a, b, c, d, patch_size, stepsize;
    static float distance;

    static float viewdistsquared;

    viewdistsquared = viewdistance * viewdistance;
    patch_size = size / subdivision;

    stepsize = 1;
    c = whichx * patch_elements + whichy * patch_elements * subdivision;

    for (i = patch_size * whichx; i < patch_size * (whichx + 1) + 1; i += stepsize) {
        for (j = patch_size * whichy; j < patch_size * (whichy + 1) + 1; j += stepsize) {
            if (i < size && j < size) {
                vertex.x = i * scale;
                vertex.z = j * scale;
                vertex.y = heightmap[i][j] * scale;
                distance = distsq(&viewer, &vertex);
                if (distance > viewdistsquared)
                    distance = viewdistsquared;
                colors[i][j][3] = (viewdistsquared - (distance - (viewdistsquared * fadestart)) * (1 / (1 - fadestart))) / viewdistsquared;
            }
        }
    }

    for (i = patch_size * whichx; i < patch_size * (whichx + 1); i += stepsize) {
        for (j = patch_size * whichy; j < patch_size * (whichy + 1); j += stepsize) {
            a = (i - (patch_size * whichx)) / stepsize;
            b = (j - (patch_size * whichy)) / stepsize;
            d = (a * 54) + (b * 54 * patch_size / stepsize);
            vArray[d + c + 6] = colors[i][j][3];

            vArray[d + c + 15] = colors[i][j + stepsize][3];

            vArray[d + c + 24] = colors[i + stepsize][j][3];

            vArray[d + c + 33] = colors[i + stepsize][j][3];

            vArray[d + c + 42] = colors[i][j + stepsize][3];

            vArray[d + c + 51] = colors[i + stepsize][j + stepsize][3];
        }
    }
}

void Terrain::UpdateTransparencyother(int whichx, int whichy)
{
    static int i, j, a, b, c, d, patch_size, stepsize;

    patch_size = size / subdivision;

    stepsize = 1;
    c = whichx * patch_elements + whichy * patch_elements * subdivision;

    for (i = patch_size * whichx; i < patch_size * (whichx + 1); i += stepsize) {
        for (j = patch_size * whichy; j < patch_size * (whichy + 1); j += stepsize) {
            a = (i - (patch_size * whichx)) / stepsize;
            b = (j - (patch_size * whichy)) / stepsize;
            d = (a * 54) + (b * 54 * patch_size / stepsize);
            vArray[d + c + 6] = colors[i][j][3] * opacityother[i][j];

            vArray[d + c + 15] = colors[i][j + stepsize][3] * opacityother[i][j + stepsize];

            vArray[d + c + 24] = colors[i + stepsize][j][3] * opacityother[i + stepsize][j];

            vArray[d + c + 33] = colors[i + stepsize][j][3] * opacityother[i + stepsize][j];

            vArray[d + c + 42] = colors[i][j + stepsize][3] * opacityother[i][j + stepsize];

            vArray[d + c + 51] = colors[i + stepsize][j + stepsize][3] * opacityother[i + stepsize][j + stepsize];
        }
    }
}

void Terrain::UpdateTransparencyotherother(int whichx, int whichy)
{
    static XYZ vertex;
    static int i, j, a, b, c, d, patch_size, stepsize;
    static float distance;

    static float viewdistsquared;

    viewdistsquared = viewdistance * viewdistance;
    patch_size = size / subdivision;

    stepsize = 1;
    c = whichx * patch_elements + whichy * patch_elements * subdivision;

    for (i = patch_size * whichx; i < patch_size * (whichx + 1) + 1; i += stepsize) {
        for (j = patch_size * whichy; j < patch_size * (whichy + 1) + 1; j += stepsize) {
            if (i < size && j < size) {
                vertex.x = i * scale;
                vertex.z = j * scale;
                vertex.y = heightmap[i][j] * scale;
                distance = distsq(&viewer, &vertex);
                if (distance > viewdistsquared)
                    distance = viewdistsquared;
                colors[i][j][3] = (viewdistsquared - (distance - (viewdistsquared * fadestart)) * (1 / (1 - fadestart))) / viewdistsquared;
            }
        }
    }

    for (i = patch_size * whichx; i < patch_size * (whichx + 1); i += stepsize) {
        for (j = patch_size * whichy; j < patch_size * (whichy + 1); j += stepsize) {
            a = (i - (patch_size * whichx)) / stepsize;
            b = (j - (patch_size * whichy)) / stepsize;
            d = (a * 54) + (b * 54 * patch_size / stepsize);
            vArray[d + c + 6] = colors[i][j][3];

            vArray[d + c + 15] = colors[i][j + stepsize][3];

            vArray[d + c + 24] = colors[i + stepsize][j][3];

            vArray[d + c + 33] = colors[i + stepsize][j][3];

            vArray[d + c + 42] = colors[i][j + stepsize][3];

            vArray[d + c + 51] = colors[i + stepsize][j + stepsize][3];
        }
    }
}

void Terrain::UpdateVertexArray(int whichx, int whichy)
{
    static int i, j, a, b, c, patch_size, stepsize;


    numtris[whichx][whichy] = 0;

    patch_size = size / subdivision;

    stepsize = 1;
    c = whichx * patch_elements + whichy * patch_elements * subdivision;
    for (i = patch_size * whichx; i < patch_size * (whichx + 1); i += stepsize) {
        for (j = patch_size * whichy; j < patch_size * (whichy + 1); j += stepsize) {
            a = (i - ((float)size / subdivision * (float)whichx)) / stepsize;
            b = (j - ((float)size / subdivision * (float)whichy)) / stepsize;
            vArray[(a * 54) + (b * 54 * patch_size / stepsize) + c + 0] = i * scale;
            vArray[(a * 54) + (b * 54 * patch_size / stepsize) + c + 1] = heightmap[i][j] * scale;
            vArray[(a * 54) + (b * 54 * patch_size / stepsize) + c + 2] = j * scale;
            vArray[(a * 54) + (b * 54 * patch_size / stepsize) + c + 3] = colors[i][j][0];
            vArray[(a * 54) + (b * 54 * patch_size / stepsize) + c + 4] = colors[i][j][1];
            vArray[(a * 54) + (b * 54 * patch_size / stepsize) + c + 5] = colors[i][j][2];
            vArray[(a * 54) + (b * 54 * patch_size / stepsize) + c + 6] = colors[i][j][3];
            vArray[(a * 54) + (b * 54 * patch_size / stepsize) + c + 7] = i * scale * texscale + texoffsetx[i][j];
            vArray[(a * 54) + (b * 54 * patch_size / stepsize) + c + 8] = j * scale * texscale + texoffsety[i][j];

            vArray[(a * 54) + (b * 54 * patch_size / stepsize) + c + 9] = i * scale;
            vArray[(a * 54) + (b * 54 * patch_size / stepsize) + c + 10] = heightmap[i][j + stepsize] * scale;
            vArray[(a * 54) + (b * 54 * patch_size / stepsize) + c + 11] = j * scale + stepsize * scale;
            vArray[(a * 54) + (b * 54 * patch_size / stepsize) + c + 12] = colors[i][j + stepsize][0];
            vArray[(a * 54) + (b * 54 * patch_size / stepsize) + c + 13] = colors[i][j + stepsize][1];
            vArray[(a * 54) + (b * 54 * patch_size / stepsize) + c + 14] = colors[i][j + stepsize][2];
            vArray[(a * 54) + (b * 54 * patch_size / stepsize) + c + 15] = colors[i][j + stepsize][3];
            vArray[(a * 54) + (b * 54 * patch_size / stepsize) + c + 16] = i * scale * texscale + texoffsetx[i][j + stepsize];
            vArray[(a * 54) + (b * 54 * patch_size / stepsize) + c + 17] = j * scale * texscale + stepsize * scale * texscale + texoffsety[i][j + stepsize];

            vArray[(a * 54) + (b * 54 * patch_size / stepsize) + c + 18] = i * scale + stepsize * scale;
            vArray[(a * 54) + (b * 54 * patch_size / stepsize) + c + 19] = heightmap[i + stepsize][j] * scale;
            vArray[(a * 54) + (b * 54 * patch_size / stepsize) + c + 20] = j * scale;
            vArray[(a * 54) + (b * 54 * patch_size / stepsize) + c + 21] = colors[i + stepsize][j][0];
            vArray[(a * 54) + (b * 54 * patch_size / stepsize) + c + 22] = colors[i + stepsize][j][1];
            vArray[(a * 54) + (b * 54 * patch_size / stepsize) + c + 23] = colors[i + stepsize][j][2];
            vArray[(a * 54) + (b * 54 * patch_size / stepsize) + c + 24] = colors[i + stepsize][j][3];
            vArray[(a * 54) + (b * 54 * patch_size / stepsize) + c + 25] = i * scale * texscale + stepsize * scale * texscale + texoffsetx[i + stepsize][j];
            vArray[(a * 54) + (b * 54 * patch_size / stepsize) + c + 26] = j * scale * texscale + texoffsety[i + stepsize][j];

            vArray[(a * 54) + (b * 54 * patch_size / stepsize) + c + 27] = i * scale + stepsize * scale;
            vArray[(a * 54) + (b * 54 * patch_size / stepsize) + c + 28] = heightmap[i + stepsize][j] * scale;
            vArray[(a * 54) + (b * 54 * patch_size / stepsize) + c + 29] = j * scale;
            vArray[(a * 54) + (b * 54 * patch_size / stepsize) + c + 30] = colors[i + stepsize][j][0];
            vArray[(a * 54) + (b * 54 * patch_size / stepsize) + c + 31] = colors[i + stepsize][j][1];
            vArray[(a * 54) + (b * 54 * patch_size / stepsize) + c + 32] = colors[i + stepsize][j][2];
            vArray[(a * 54) + (b * 54 * patch_size / stepsize) + c + 33] = colors[i + stepsize][j][3];
            vArray[(a * 54) + (b * 54 * patch_size / stepsize) + c + 34] = i * scale * texscale + stepsize * scale * texscale + texoffsetx[i + stepsize][j];
            vArray[(a * 54) + (b * 54 * patch_size / stepsize) + c + 35] = j * scale * texscale + texoffsety[i + stepsize][j];

            vArray[(a * 54) + (b * 54 * patch_size / stepsize) + c + 36] = i * scale;
            vArray[(a * 54) + (b * 54 * patch_size / stepsize) + c + 37] = heightmap[i][j + stepsize] * scale;
            vArray[(a * 54) + (b * 54 * patch_size / stepsize) + c + 38] = j * scale + stepsize * scale;
            vArray[(a * 54) + (b * 54 * patch_size / stepsize) + c + 39] = colors[i][j + stepsize][0];
            vArray[(a * 54) + (b * 54 * patch_size / stepsize) + c + 40] = colors[i][j + stepsize][1];
            vArray[(a * 54) + (b * 54 * patch_size / stepsize) + c + 41] = colors[i][j + stepsize][2];
            vArray[(a * 54) + (b * 54 * patch_size / stepsize) + c + 42] = colors[i][j + stepsize][3];
            vArray[(a * 54) + (b * 54 * patch_size / stepsize) + c + 43] = i * scale * texscale + texoffsetx[i][j + stepsize];
            vArray[(a * 54) + (b * 54 * patch_size / stepsize) + c + 44] = j * scale * texscale + stepsize * scale * texscale + texoffsety[i][j + stepsize];

            vArray[(a * 54) + (b * 54 * patch_size / stepsize) + c + 45] = i * scale + stepsize * scale;
            vArray[(a * 54) + (b * 54 * patch_size / stepsize) + c + 46] = heightmap[i + stepsize][j + stepsize] * scale;
            vArray[(a * 54) + (b * 54 * patch_size / stepsize) + c + 47] = j * scale + stepsize * scale;
            vArray[(a * 54) + (b * 54 * patch_size / stepsize) + c + 48] = colors[i + stepsize][j + stepsize][0];
            vArray[(a * 54) + (b * 54 * patch_size / stepsize) + c + 49] = colors[i + stepsize][j + stepsize][1];
            vArray[(a * 54) + (b * 54 * patch_size / stepsize) + c + 50] = colors[i + stepsize][j + stepsize][2];
            vArray[(a * 54) + (b * 54 * patch_size / stepsize) + c + 51] = colors[i + stepsize][j + stepsize][3];
            vArray[(a * 54) + (b * 54 * patch_size / stepsize) + c + 52] = i * scale * texscale + stepsize * scale * texscale + texoffsetx[i + stepsize][j + stepsize];
            vArray[(a * 54) + (b * 54 * patch_size / stepsize) + c + 53] = j * scale * texscale + stepsize * scale * texscale + texoffsety[i + stepsize][j + stepsize];
            numtris[whichx][whichy] += 2;
        }
    }

    maxypatch[whichx][whichy] = -10000;
    minypatch[whichx][whichy] = 10000;
    for (a = 0; a < size / subdivision; a++) {
        for (b = 0; b < size / subdivision; b++) {
            if (heightmap[(size / subdivision)*whichx + a][(size / subdivision)*whichy + b]*scale > maxypatch[whichx][whichy])
                maxypatch[whichx][whichy] = heightmap[(size / subdivision) * whichx + a][(size / subdivision) * whichy + b] * scale;
            if (heightmap[(size / subdivision)*whichx + a][(size / subdivision)*whichy + b]*scale < minypatch[whichx][whichy])
                minypatch[whichx][whichy] = heightmap[(size / subdivision) * whichx + a][(size / subdivision) * whichy + b] * scale;
        }
    }
    heightypatch[whichx][whichy] = (maxypatch[whichx][whichy] - minypatch[whichx][whichy]);
    if (heightypatch[whichx][whichy] < size / subdivision * scale)
        heightypatch[whichx][whichy] = size / subdivision * scale;
    avgypatch[whichx][whichy] = (minypatch[whichx][whichy] + maxypatch[whichx][whichy]) / 2;

    for (i = whichx * size / subdivision; i < (whichx + 1)*size / subdivision - 1; i++) {
        for (j = whichy * size / subdivision; j < (whichy + 1)*size / subdivision - 1; j++) {
            triangles[(i * (size - 1) * 2) + (j * 2)][0].x = i * scale;
            triangles[(i * (size - 1) * 2) + (j * 2)][0].y = heightmap[i][j] * scale;
            triangles[(i * (size - 1) * 2) + (j * 2)][0].z = j * scale;

            triangles[(i * (size - 1) * 2) + (j * 2)][1].x = i * scale;
            triangles[(i * (size - 1) * 2) + (j * 2)][1].y = heightmap[i][j + 1] * scale;
            triangles[(i * (size - 1) * 2) + (j * 2)][1].z = j * scale + scale;

            triangles[(i * (size - 1) * 2) + (j * 2)][2].x = i * scale + 1 * scale;
            triangles[(i * (size - 1) * 2) + (j * 2)][2].y = heightmap[i + 1][j] * scale;
            triangles[(i * (size - 1) * 2) + (j * 2)][2].z = j * scale;

            triangles[(i * (size - 1) * 2) + (j * 2) + 1][0].x = i * scale + 1 * scale;
            triangles[(i * (size - 1) * 2) + (j * 2) + 1][0].y = heightmap[i + 1][j] * scale;
            triangles[(i * (size - 1) * 2) + (j * 2) + 1][0].z = j * scale;

            triangles[(i * (size - 1) * 2) + (j * 2) + 1][1].x = i * scale;
            triangles[(i * (size - 1) * 2) + (j * 2) + 1][1].y = heightmap[i][j + 1] * scale;
            triangles[(i * (size - 1) * 2) + (j * 2) + 1][1].z = j * scale + 1 * scale;

            triangles[(i * (size - 1) * 2) + (j * 2) + 1][2].x = i * scale + 1 * scale;
            triangles[(i * (size - 1) * 2) + (j * 2) + 1][2].y = heightmap[i + 1][j + 1] * scale;
            triangles[(i * (size - 1) * 2) + (j * 2) + 1][2].z = j * scale + 1 * scale;
        }
    }

}


bool Terrain::load(const std::string& fileName)
{
    static long i, j;
    static long x, y;
    static float patch_size;

    float temptexdetail = texdetail;

    ImageRec texture;

    //Load Image
    if (!load_image(Folders::getResourcePath(fileName).c_str(), texture)) {
        return false;
    }

    //Is it valid?
    if (texture.bpp > 24) {
        int bytesPerPixel = texture.bpp / 8;

        int tempnum = 0;
        for (i = 0; i < (long)(texture.sizeY * texture.sizeX * bytesPerPixel); i++) {
            if ((i + 1) % 4) {
                texture.data[tempnum] = texture.data[i];
                tempnum++;
            }
        }
    }
    texture.bpp = 24;
    if (visibleloading)
        Game::LoadingScreen();

    texdetail = temptexdetail;

    size = texture.sizeX;

    for (i = 0; i < size; i++) {
        for (j = 0; j < size; j++) {
            heightmap[size - 1 - i][j] = (float)((texture.data[(i + (j * size)) * texture.bpp / 8])) / 5;
        }
    }

    if (visibleloading)
        Game::LoadingScreen();

    float slopeness;

    for (i = 0; i < subdivision; i++) {
        for (j = 0; j < subdivision; j++) {
            textureness[i][j] = -1;
        }
    }
    if (visibleloading)
        Game::LoadingScreen();


    for (i = 0; i < size; i++) {
        for (j = 0; j < size; j++) {
            heightmap[i][j] *= .5;

            texoffsetx[i][j] = (float)abs(Random() % 100) / 1200 / scale * 3;
            texoffsety[i][j] = (float)abs(Random() % 100) / 1200 / scale * 3;

            slopeness = 0;
            if (environment == snowyenvironment) {
                if (j != 0 && heightmap[i][j] - heightmap[i][j - 1] > slopeness) {
                    slopeness = heightmap[i][j] - heightmap[i][j - 1];
                }
                opacityother[i][j] = slopeness * slopeness * 2;
                if (opacityother[i][j] > 1)
                    opacityother[i][j] = 1;
                opacityother[i][j] -= (float)abs(Random() % 100) / 300;
            }
            if (environment == desertenvironment) {
                if (j != 0 && heightmap[i][j] - heightmap[i][j - 1] > slopeness) {
                    slopeness = heightmap[i][j] - heightmap[i][j - 1];
                }
                opacityother[i][j] = slopeness * slopeness * 2;
                if (opacityother[i][j] > 1)
                    opacityother[i][j] = 1;
                opacityother[i][j] -= (float)abs(Random() % 100) / 300;
            }
            if (environment == grassyenvironment) {
                if (i != 0 && heightmap[i][j] - heightmap[i - 1][j] > slopeness) {
                    slopeness = heightmap[i][j] - heightmap[i - 1][j];
                }
                if (j != 0 && heightmap[i][j] - heightmap[i][j - 1] > slopeness) {
                    slopeness = heightmap[i][j] - heightmap[i][j - 1];
                }
                if (i < size - 1 && heightmap[i][j] - heightmap[i + 1][j] > slopeness) {
                    slopeness = heightmap[i][j] - heightmap[i + 1][j];
                }
                if (j < size - 1 && heightmap[i][j] - heightmap[i][j + 1] > slopeness) {
                    slopeness = heightmap[i][j] - heightmap[i][j + 1];
                }
                opacityother[i][j] = slopeness * slopeness * 10;
                if (opacityother[i][j] > 1)
                    opacityother[i][j] = 1;
                opacityother[i][j] -= (float)abs(Random() % 100) / 100;
            }
        }
    }
    if (visibleloading)
        Game::LoadingScreen();

    for (i = 0; i < size; i++) {
        for (j = 0; j < size; j++) {
            if (environment == snowyenvironment) {
                heightmap[i][j] -= opacityother[i][j];
            }
            if (environment == desertenvironment) {
                heightmap[i][j] -= opacityother[i][j];
            }
        }
    }
    if (visibleloading)
        Game::LoadingScreen();

    for (i = 0; i < size; i++) {
        for (j = 0; j < size; j++) {
            if (opacityother[i][j] < .1)
                opacityother[i][j] = 0;
            if (textureness[(int)(i * subdivision / size)][(int)(j * subdivision / size)] == -1) {
                if (!opacityother[i][j])
                    textureness[(int)(i * subdivision / size)][(int)(j * subdivision / size)] = allfirst;
                if (opacityother[i][j] == 1)
                    textureness[(int)(i * subdivision / size)][(int)(j * subdivision / size)] = allsecond;
            }
            if (opacityother[i][j] && textureness[(int)(i * subdivision / size)][(int)(j * subdivision / size)] == allfirst)
                textureness[(int)(i * subdivision / size)][(int)(j * subdivision / size)] = mixed;
            if (opacityother[i][j] != 1 && textureness[(int)(i * subdivision / size)][(int)(j * subdivision / size)] == allsecond)
                textureness[(int)(i * subdivision / size)][(int)(j * subdivision / size)] = mixed;

            x = i;
            y = j;
            if (i > 0) {
                i--;
                if (opacityother[x][y] && textureness[(int)(i * subdivision / size)][(int)(j * subdivision / size)] == allfirst)
                    textureness[(int)(i * subdivision / size)][(int)(j * subdivision / size)] = mixed;
                if (opacityother[x][y] != 1 && textureness[(int)(i * subdivision / size)][(int)(j * subdivision / size)] == allsecond)
                    textureness[(int)(i * subdivision / size)][(int)(j * subdivision / size)] = mixed;
                if (opacityother[i][j] && textureness[(int)(x * subdivision / size)][(int)(y * subdivision / size)] == allfirst)
                    textureness[(int)(x * subdivision / size)][(int)(y * subdivision / size)] = mixed;
                if (opacityother[i][j] != 1 && textureness[(int)(x * subdivision / size)][(int)(y * subdivision / size)] == allsecond)
                    textureness[(int)(x * subdivision / size)][(int)(y * subdivision / size)] = mixed;

                if (j > 0) {
                    j--;
                    if (opacityother[x][y] && textureness[(int)(i * subdivision / size)][(int)(j * subdivision / size)] == allfirst)
                        textureness[(int)(i * subdivision / size)][(int)(j * subdivision / size)] = mixed;
                    if (opacityother[x][y] != 1 && textureness[(int)(i * subdivision / size)][(int)(j * subdivision / size)] == allsecond)
                        textureness[(int)(i * subdivision / size)][(int)(j * subdivision / size)] = mixed;
                    if (opacityother[i][j] && textureness[(int)(x * subdivision / size)][(int)(y * subdivision / size)] == allfirst)
                        textureness[(int)(x * subdivision / size)][(int)(y * subdivision / size)] = mixed;
                    if (opacityother[i][j] != 1 && textureness[(int)(x * subdivision / size)][(int)(y * subdivision / size)] == allsecond)
                        textureness[(int)(x * subdivision / size)][(int)(y * subdivision / size)] = mixed;
                    j++;
                }

                if (j < size - 1) {
                    j++;
                    if (opacityother[x][y] && textureness[(int)(i * subdivision / size)][(int)(j * subdivision / size)] == allfirst)
                        textureness[(int)(i * subdivision / size)][(int)(j * subdivision / size)] = mixed;
                    if (opacityother[x][y] != 1 && textureness[(int)(i * subdivision / size)][(int)(j * subdivision / size)] == allsecond)
                        textureness[(int)(i * subdivision / size)][(int)(j * subdivision / size)] = mixed;
                    if (opacityother[i][j] && textureness[(int)(x * subdivision / size)][(int)(y * subdivision / size)] == allfirst)
                        textureness[(int)(x * subdivision / size)][(int)(y * subdivision / size)] = mixed;
                    if (opacityother[i][j] != 1 && textureness[(int)(x * subdivision / size)][(int)(y * subdivision / size)] == allsecond)
                        textureness[(int)(x * subdivision / size)][(int)(y * subdivision / size)] = mixed;
                    j--;
                }
                i++;
            }

            if (i < size - 1) {
                i++;
                if (opacityother[x][y] && textureness[(int)(i * subdivision / size)][(int)(j * subdivision / size)] == allfirst)
                    textureness[(int)(i * subdivision / size)][(int)(j * subdivision / size)] = mixed;
                if (opacityother[x][y] != 1 && textureness[(int)(i * subdivision / size)][(int)(j * subdivision / size)] == allsecond)
                    textureness[(int)(i * subdivision / size)][(int)(j * subdivision / size)] = mixed;
                if (opacityother[i][j] && textureness[(int)(x * subdivision / size)][(int)(y * subdivision / size)] == allfirst)
                    textureness[(int)(x * subdivision / size)][(int)(y * subdivision / size)] = mixed;
                if (opacityother[i][j] != 1 && textureness[(int)(x * subdivision / size)][(int)(y * subdivision / size)] == allsecond)
                    textureness[(int)(x * subdivision / size)][(int)(y * subdivision / size)] = mixed;

                if (j > 0) {
                    j--;
                    if (opacityother[x][y] && textureness[(int)(i * subdivision / size)][(int)(j * subdivision / size)] == allfirst)
                        textureness[(int)(i * subdivision / size)][(int)(j * subdivision / size)] = mixed;
                    if (opacityother[x][y] != 1 && textureness[(int)(i * subdivision / size)][(int)(j * subdivision / size)] == allsecond)
                        textureness[(int)(i * subdivision / size)][(int)(j * subdivision / size)] = mixed;
                    if (opacityother[i][j] && textureness[(int)(x * subdivision / size)][(int)(y * subdivision / size)] == allfirst)
                        textureness[(int)(x * subdivision / size)][(int)(y * subdivision / size)] = mixed;
                    if (opacityother[i][j] != 1 && textureness[(int)(x * subdivision / size)][(int)(y * subdivision / size)] == allsecond)
                        textureness[(int)(x * subdivision / size)][(int)(y * subdivision / size)] = mixed;
                    j++;
                }

                if (j < size - 1) {
                    j++;
                    if (opacityother[x][y] && textureness[(int)(i * subdivision / size)][(int)(j * subdivision / size)] == allfirst)
                        textureness[(int)(i * subdivision / size)][(int)(j * subdivision / size)] = mixed;
                    if (opacityother[x][y] != 1 && textureness[(int)(i * subdivision / size)][(int)(j * subdivision / size)] == allsecond)
                        textureness[(int)(i * subdivision / size)][(int)(j * subdivision / size)] = mixed;
                    if (opacityother[i][j] && textureness[(int)(x * subdivision / size)][(int)(y * subdivision / size)] == allfirst)
                        textureness[(int)(x * subdivision / size)][(int)(y * subdivision / size)] = mixed;
                    if (opacityother[i][j] != 1 && textureness[(int)(x * subdivision / size)][(int)(y * subdivision / size)] == allsecond)
                        textureness[(int)(x * subdivision / size)][(int)(y * subdivision / size)] = mixed;
                    j--;
                }
                i--;
            }

            if (j > 0) {
                j--;
                if (opacityother[x][y] && textureness[(int)(i * subdivision / size)][(int)(j * subdivision / size)] == allfirst)
                    textureness[(int)(i * subdivision / size)][(int)(j * subdivision / size)] = mixed;
                if (opacityother[x][y] != 1 && textureness[(int)(i * subdivision / size)][(int)(j * subdivision / size)] == allsecond)
                    textureness[(int)(i * subdivision / size)][(int)(j * subdivision / size)] = mixed;
                if (opacityother[i][j] && textureness[(int)(x * subdivision / size)][(int)(y * subdivision / size)] == allfirst)
                    textureness[(int)(x * subdivision / size)][(int)(y * subdivision / size)] = mixed;
                if (opacityother[i][j] != 1 && textureness[(int)(x * subdivision / size)][(int)(y * subdivision / size)] == allsecond)
                    textureness[(int)(x * subdivision / size)][(int)(y * subdivision / size)] = mixed;
                j++;
            }

            if (j < size - 1) {
                j++;
                if (opacityother[x][y] && textureness[(int)(i * subdivision / size)][(int)(j * subdivision / size)] == allfirst)
                    textureness[(int)(i * subdivision / size)][(int)(j * subdivision / size)] = mixed;
                if (opacityother[x][y] != 1 && textureness[(int)(i * subdivision / size)][(int)(j * subdivision / size)] == allsecond)
                    textureness[(int)(i * subdivision / size)][(int)(j * subdivision / size)] = mixed;
                if (opacityother[i][j] && textureness[(int)(x * subdivision / size)][(int)(y * subdivision / size)] == allfirst)
                    textureness[(int)(x * subdivision / size)][(int)(y * subdivision / size)] = mixed;
                if (opacityother[i][j] != 1 && textureness[(int)(x * subdivision / size)][(int)(y * subdivision / size)] == allsecond)
                    textureness[(int)(x * subdivision / size)][(int)(y * subdivision / size)] = mixed;
                j--;

            }
        }
    }
    if (visibleloading)
        Game::LoadingScreen();

    patch_size = size / subdivision;
    patch_elements = (patch_size) * (patch_size) * 54;
    CalculateNormals();

    return true;
}

void Terrain::CalculateNormals()
{
    static int i, j;
    static XYZ facenormal;
    static XYZ p, q, a, b, c;

    for (i = 0; i < size; i++) {
        for (j = 0; j < size; j++) {
            normals[i][j].x = 0;
            normals[i][j].y = 0;
            normals[i][j].z = 0;
        }
    }

    for (i = 0; i < size - 1; i++) {
        for (j = 0; j < size - 1; j++) {
            a.x = i;
            a.y = heightmap[i][j];
            a.z = j;
            b.x = i;
            b.y = heightmap[i][j + 1];
            b.z = j + 1;
            c.x = i + 1;
            c.y = heightmap[i + 1][j];
            c.z = j;

            p.x = b.x - a.x;
            p.y = b.y - a.y;
            p.z = b.z - a.z;
            q.x = c.x - a.x;
            q.y = c.y - a.y;
            q.z = c.z - a.z;

            CrossProduct(&p, &q, &facenormal);

            facenormals[i][j] = facenormal;

            normals[i][j] = normals[i][j] + facenormal;
            normals[i][j + 1] = normals[i][j + 1] + facenormal;
            normals[i + 1][j] = normals[i + 1][j] + facenormal;


            a.x = i + 1;
            a.y = heightmap[i + 1][j];
            a.z = j;
            b.x = i;
            b.y = heightmap[i][j + 1];
            b.z = j + 1;
            c.x = i + 1;
            c.y = heightmap[i + 1][j + 1];
            c.z = j + 1;

            p.x = b.x - a.x;
            p.y = b.y - a.y;
            p.z = b.z - a.z;
            q.x = c.x - a.x;
            q.y = c.y - a.y;
            q.z = c.z - a.z;

            CrossProduct(&p, &q, &facenormal);

            normals[i + 1][j + 1] = normals[i + 1][j + 1] + facenormal;
            normals[i][j + 1] = normals[i][j + 1] + facenormal;
            normals[i + 1][j] = normals[i + 1][j] + facenormal;

            Normalise(&facenormals[i][j]);
        }
    }

    for (i = 0; i < size; i++) {
        for (j = 0; j < size; j++) {
            Normalise(&normals[i][j]);
        }
    }
}

void Terrain::drawpatch(int whichx, int whichy, float opacity)
{
    if (opacity >= 1)
        glDisable(GL_BLEND);
    if (opacity < 1) {
        glEnable(GL_BLEND);
        UpdateTransparency(whichx, whichy);
    }
    glColor4f(1, 1, 1, 1);
    //Set up vertex array
    glEnableClientState(GL_VERTEX_ARRAY);
    glEnableClientState(GL_COLOR_ARRAY);
    glEnableClientState(GL_TEXTURE_COORD_ARRAY);
    glVertexPointer(3, GL_FLOAT, 9 * sizeof(GLfloat), &vArray[0 + whichx * patch_elements + whichy * patch_elements * subdivision]);
    glColorPointer(4, GL_FLOAT, 9 * sizeof(GLfloat), &vArray[3 + whichx * patch_elements + whichy * patch_elements * subdivision]);
    glTexCoordPointer(2, GL_FLOAT, 9 * sizeof(GLfloat), &vArray[7 + whichx * patch_elements + whichy * patch_elements * subdivision]);

    //Draw
    glDrawArrays(GL_TRIANGLES, 0, numtris[whichx][whichy] * 3);

    glDisableClientState(GL_VERTEX_ARRAY);
    glDisableClientState(GL_COLOR_ARRAY);
    glDisableClientState(GL_TEXTURE_COORD_ARRAY);
}

void Terrain::drawpatchother(int whichx, int whichy, float opacity)
{
    glEnable(GL_BLEND);
    if (opacity < 1) {
        UpdateTransparency(whichx, whichy);
    }
    UpdateTransparencyother(whichx, whichy);
    glColor4f(1, 1, 1, 1);
    //Set up vertex array
    glEnableClientState(GL_VERTEX_ARRAY);
    glEnableClientState(GL_COLOR_ARRAY);
    glEnableClientState(GL_TEXTURE_COORD_ARRAY);
    glVertexPointer(3, GL_FLOAT, 9 * sizeof(GLfloat), &vArray[0 + whichx * patch_elements + whichy * patch_elements * subdivision]);
    glColorPointer(4, GL_FLOAT, 9 * sizeof(GLfloat), &vArray[3 + whichx * patch_elements + whichy * patch_elements * subdivision]);
    glTexCoordPointer(2, GL_FLOAT, 9 * sizeof(GLfloat), &vArray[7 + whichx * patch_elements + whichy * patch_elements * subdivision]);

    //Draw
    glDrawArrays(GL_TRIANGLES, 0, numtris[whichx][whichy] * 3);

    glDisableClientState(GL_VERTEX_ARRAY);
    glDisableClientState(GL_COLOR_ARRAY);
    glDisableClientState(GL_TEXTURE_COORD_ARRAY);
}

void Terrain::drawpatchotherother(int whichx, int whichy, float opacity)
{
    glEnable(GL_BLEND);
    UpdateTransparencyotherother(whichx, whichy);

    glMatrixMode(GL_TEXTURE);
    glPushMatrix();
    glScalef(6, 6, 6);

    glColor4f(1, 1, 1, 1);

    //Set up vertex array
    glEnableClientState(GL_VERTEX_ARRAY);
    glEnableClientState(GL_COLOR_ARRAY);
    glEnableClientState(GL_TEXTURE_COORD_ARRAY);
    glVertexPointer(3, GL_FLOAT, 9 * sizeof(GLfloat), &vArray[0 + whichx * patch_elements + whichy * patch_elements * subdivision]);
    glColorPointer(4, GL_FLOAT, 9 * sizeof(GLfloat), &vArray[3 + whichx * patch_elements + whichy * patch_elements * subdivision]);
    glTexCoordPointer(2, GL_FLOAT, 9 * sizeof(GLfloat), &vArray[7 + whichx * patch_elements + whichy * patch_elements * subdivision]);

    //Draw
    glDrawArrays(GL_TRIANGLES, 0, numtris[whichx][whichy] * 3);

    glDisableClientState(GL_VERTEX_ARRAY);
    glDisableClientState(GL_COLOR_ARRAY);
    glDisableClientState(GL_TEXTURE_COORD_ARRAY);

    glPopMatrix();
    glMatrixMode(GL_MODELVIEW);
}


float Terrain::getHeight(float pointx, float pointz)
{
    static int tilex, tiley;
    static XYZ startpoint, endpoint, intersect, triangle[3];

    pointx /= scale;
    pointz /= scale;

    if (pointx >= size - 1 || pointz >= size - 1 || pointx <= 0 || pointz <= 0)
        return 0;

    startpoint.x = pointx;
    startpoint.y = -1000;
    startpoint.z = pointz;

    endpoint = startpoint;
    endpoint.y = 1000;

    tilex = pointx;
    tiley = pointz;

    triangle[0].x = tilex;
    triangle[0].z = tiley;
    triangle[0].y = heightmap[tilex][tiley];

    triangle[1].x = tilex + 1;
    triangle[1].z = tiley;
    triangle[1].y = heightmap[tilex + 1][tiley];

    triangle[2].x = tilex;
    triangle[2].z = tiley + 1;
    triangle[2].y = heightmap[tilex][tiley + 1];

    if (!LineFacetd(&startpoint, &endpoint, &triangle[0], &triangle[1], &triangle[2], &intersect)) {
        triangle[0].x = tilex + 1;
        triangle[0].z = tiley;
        triangle[0].y = heightmap[tilex + 1][tiley];

        triangle[1].x = tilex + 1;
        triangle[1].z = tiley + 1;
        triangle[1].y = heightmap[tilex + 1][tiley + 1];

        triangle[2].x = tilex;
        triangle[2].z = tiley + 1;
        triangle[2].y = heightmap[tilex][tiley + 1];
        LineFacetd(&startpoint, &endpoint, &triangle[0], &triangle[1], &triangle[2], &intersect);
    }
    return intersect.y * scale + getOpacity(pointx * scale, pointz * scale) / 8;
}

float Terrain::getOpacity(float pointx, float pointz)
{
    static float height1, height2;
    static int tilex, tiley;

    pointx /= scale;
    pointz /= scale;

    if (pointx >= size - 1 || pointz >= size - 1 || pointx <= 0 || pointz <= 0)
        return 0;

    tilex = pointx;
    tiley = pointz;

    height1 = opacityother[tilex][tiley] * (1 - (pointx - tilex)) + opacityother[tilex + 1][tiley] * (pointx - tilex);
    height2 = opacityother[tilex][tiley + 1] * (1 - (pointx - tilex)) + opacityother[tilex + 1][tiley + 1] * (pointx - tilex);

    return height1 * (1 - (pointz - tiley)) + height2 * (pointz - tiley);
}

XYZ Terrain::getNormal(float pointx, float pointz)
{
    static XYZ height1, height2, total;
    static int tilex, tiley;

    pointx /= scale;
    pointz /= scale;

    height1 = 0;
    if (pointx >= size - 1 || pointz >= size - 1 || pointx <= 0 || pointz <= 0)
        return height1;
    tilex = pointx;
    tiley = pointz;

    height1 = normals[tilex][tiley] * (1 - (pointx - tilex)) + normals[tilex + 1][tiley] * (pointx - tilex);
    height2 = normals[tilex][tiley + 1] * (1 - (pointx - tilex)) + normals[tilex + 1][tiley + 1] * (pointx - tilex);
    total = height1 * (1 - (pointz - tiley)) + height2 * (pointz - tiley);
    Normalise(&total);
    return total;
}

XYZ Terrain::getLighting(float pointx, float pointz)
{
    static XYZ height1, height2;
    static int tilex, tiley;

    pointx /= scale;
    pointz /= scale;

    height1 = 0;
    if (pointx >= size - 1 || pointz >= size - 1 || pointx <= 0 || pointz <= 0)
        return height1;
    tilex = pointx;
    tiley = pointz;

    height1.x = colors[tilex][tiley][0] * (1 - (pointx - tilex)) + colors[tilex + 1][tiley][0] * (pointx - tilex);
    height1.y = colors[tilex][tiley][1] * (1 - (pointx - tilex)) + colors[tilex + 1][tiley][1] * (pointx - tilex);
    height1.z = colors[tilex][tiley][2] * (1 - (pointx - tilex)) + colors[tilex + 1][tiley][2] * (pointx - tilex);
    height2.x = colors[tilex][tiley + 1][0] * (1 - (pointx - tilex)) + colors[tilex + 1][tiley + 1][0] * (pointx - tilex);
    height2.y = colors[tilex][tiley + 1][1] * (1 - (pointx - tilex)) + colors[tilex + 1][tiley + 1][1] * (pointx - tilex);
    height2.z = colors[tilex][tiley + 1][2] * (1 - (pointx - tilex)) + colors[tilex + 1][tiley + 1][2] * (pointx - tilex);

    return height1 * (1 - (pointz - tiley)) + height2 * (pointz - tiley);
}

void Terrain::draw(int layer)
{
    static int i, j;
    static float opacity;
    static XYZ terrainpoint;
    static float distance[subdivision][subdivision];

    static int beginx, endx;
    static int beginz, endz;

    static float patch_size = size / subdivision * scale;
    static float viewdistsquared;

    viewdistsquared = viewdistance * viewdistance;

    //Only nearby blocks
    beginx = (viewer.x - viewdistance) / (patch_size) - 1;
    if (beginx < 0)
        beginx = 0;
    beginz = (viewer.z - viewdistance) / (patch_size) - 1;
    if (beginz < 0)
        beginz = 0;

    endx = (viewer.x + viewdistance) / (patch_size) + 1;
    if (endx > subdivision)
        endx = subdivision;
    endz = (viewer.z + viewdistance) / (patch_size) + 1;
    if (endz > subdivision)
        endz = subdivision;

    if (!layer) {
        for (i = beginx; i < endx; i++) {
            for (j = beginz; j < endz; j++) {
                terrainpoint.x = i * patch_size + (patch_size) / 2;
                terrainpoint.y = viewer.y; //heightmap[i][j]*scale;
                terrainpoint.z = j * patch_size + (patch_size) / 2;
                distance[i][j] = distsq(&viewer, &terrainpoint);
            }
        }
    }
    for (i = beginx; i < endx; i++) {
        for (j = beginz; j < endz; j++) {
            if (distance[i][j] < (viewdistance + patch_size) * (viewdistance + patch_size)) {
                opacity = 1;
                if (distance[i][j] > viewdistsquared * fadestart - viewdistsquared)
                    opacity = 0;
                if (opacity == 1 && i != subdivision)
                    if (distance[i + 1][j] > viewdistsquared * fadestart - viewdistsquared)
                        opacity = 0;
                if (opacity == 1 && j != subdivision)
                    if (distance[i][j + 1] > viewdistsquared * fadestart - viewdistsquared)
                        opacity = 0;
                if (opacity == 1 && j != subdivision && i != subdivision)
                    if (distance[i + 1][j + 1] > viewdistsquared * fadestart - viewdistsquared)
                        opacity = 0;
                glMatrixMode(GL_MODELVIEW);
                glPushMatrix();
                if (frustum.CubeInFrustum(i * patch_size + patch_size * .5, avgypatch[i][j], j * patch_size + patch_size * .5, heightypatch[i][j] / 2)) {
                    if (environment == desertenvironment && distance[i][j] > viewdistsquared / 4)
                        glTexEnvf( GL_TEXTURE_FILTER_CONTROL_EXT, GL_TEXTURE_LOD_BIAS_EXT, blurness);
                    else if (environment == desertenvironment)
                        glTexEnvf( GL_TEXTURE_FILTER_CONTROL_EXT, GL_TEXTURE_LOD_BIAS_EXT, 0 );
                    if (!layer && textureness[i][j] != allsecond)
                        drawpatch(i, j, opacity);
                    if (layer == 1 && textureness[i][j] != allfirst)
                        drawpatchother(i, j, opacity);
                    if (layer == 2 && textureness[i][j] != allfirst)
                        drawpatchotherother(i, j, opacity);
                }
                glPopMatrix();
            }
        }
    }
    if (environment == desertenvironment)
        glTexEnvf( GL_TEXTURE_FILTER_CONTROL_EXT, GL_TEXTURE_LOD_BIAS_EXT, 0 );
}

void Terrain::drawdecals()
{
    if (decals) {
        static int i;
        static float distancemult;
        static int lasttype;

        static float viewdistsquared;
        static bool blend;

        viewdistsquared = viewdistance * viewdistance;
        blend = 1;

        lasttype = -1;
        glEnable(GL_BLEND);
        glDisable(GL_LIGHTING);
        glDisable(GL_CULL_FACE);
        glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
        glDepthMask(0);
        for (i = 0; i < numdecals; i++) {
            if (decaltype[i] == blooddecalfast && decalalivetime[i] < 2)
                decalalivetime[i] = 2;
            if ((decaltype[i] == shadowdecal || decaltype[i] == shadowdecalpermanent) && decaltype[i] != lasttype) {
                shadowtexture.bind();
                if (!blend) {
                    blend = 1;
                    glAlphaFunc(GL_GREATER, 0.0001);
                    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
                }
            }
            if (decaltype[i] == footprintdecal && decaltype[i] != lasttype) {
                footprinttexture.bind();
                if (!blend) {
                    blend = 1;
                    glAlphaFunc(GL_GREATER, 0.0001);
                    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
                }
            }
            if (decaltype[i] == bodyprintdecal && decaltype[i] != lasttype) {
                bodyprinttexture.bind();
                if (!blend) {
                    blend = 1;
                    glAlphaFunc(GL_GREATER, 0.0001);
                    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
                }
            }
            if ((decaltype[i] == blooddecal || decaltype[i] == blooddecalslow) && decaltype[i] != lasttype) {
                bloodtexture.bind();
                if (blend) {
                    blend = 0;
                    glAlphaFunc(GL_GREATER, 0.15);
                    glBlendFunc(GL_ONE, GL_ZERO);
                }
            }
            if ((decaltype[i] == blooddecalfast) && decaltype[i] != lasttype) {
                bloodtexture2.bind();
                if (blend) {
                    blend = 0;
                    glAlphaFunc(GL_GREATER, 0.15);
                    glBlendFunc(GL_ONE, GL_ZERO);
                }
            }
            if (decaltype[i] == shadowdecal || decaltype[i] == shadowdecalpermanent) {
                distancemult = (viewdistsquared - (distsq(&viewer, &decalposition[i]) - (viewdistsquared * fadestart)) * (1 / (1 - fadestart))) / viewdistsquared;
                if (distancemult >= 1)
                    glColor4f(1, 1, 1, decalopacity[i]);
                if (distancemult < 1)
                    glColor4f(1, 1, 1, decalopacity[i]*distancemult);
            }
            if (decaltype[i] == footprintdecal || decaltype[i] == bodyprintdecal) {
                distancemult = (viewdistsquared - (distsq(&viewer, &decalposition[i]) - (viewdistsquared * fadestart)) * (1 / (1 - fadestart))) / viewdistsquared;
                if (distancemult >= 1) {
                    glColor4f(1, 1, 1, decalopacity[i]);
                    if (decalalivetime[i] > 3)
                        glColor4f(1, 1, 1, decalopacity[i] * (5 - decalalivetime[i]) / 2);
                }
                if (distancemult < 1) {
                    glColor4f(1, 1, 1, decalopacity[i]*distancemult);
                    if (decalalivetime[i] > 3)
                        glColor4f(1, 1, 1, decalopacity[i] * (5 - decalalivetime[i]) / 2 * distancemult);
                }
            }
            if ((decaltype[i] == blooddecal || decaltype[i] == blooddecalfast || decaltype[i] == blooddecalslow)) {
                distancemult = (viewdistsquared - (distsq(&viewer, &decalposition[i]) - (viewdistsquared * fadestart)) * (1 / (1 - fadestart))) / viewdistsquared;
                if (distancemult >= 1) {
                    glColor4f(decalbrightness[i], decalbrightness[i], decalbrightness[i], decalopacity[i]);
                    if (decalalivetime[i] < 4)
                        glColor4f(decalbrightness[i], decalbrightness[i], decalbrightness[i], decalopacity[i]*decalalivetime[i]*.25);
                    if (decalalivetime[i] > 58)
                        glColor4f(decalbrightness[i], decalbrightness[i], decalbrightness[i], decalopacity[i] * (60 - decalalivetime[i]) / 2);
                }
                if (distancemult < 1) {
                    glColor4f(decalbrightness[i], decalbrightness[i], decalbrightness[i], decalopacity[i]*distancemult);
                    if (decalalivetime[i] < 4)
                        glColor4f(decalbrightness[i], decalbrightness[i], decalbrightness[i], decalopacity[i]*decalalivetime[i]*distancemult * .25);
                    if (decalalivetime[i] > 58)
                        glColor4f(decalbrightness[i], decalbrightness[i], decalbrightness[i], decalopacity[i] * (60 - decalalivetime[i]) / 2 * distancemult);
                }
            }
            lasttype = decaltype[i];
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP);

            glMatrixMode(GL_MODELVIEW);
            glPushMatrix();
            glBegin(GL_TRIANGLES);
            for (int j = 0; j < 3; j++) {
                glTexCoord2f(decaltexcoords[i][j][0], decaltexcoords[i][j][1]);
                glVertex3f(decalvertex[i][j].x, decalvertex[i][j].y, decalvertex[i][j].z);
            }
            glEnd();
            glPopMatrix();
        }
        for (i = numdecals - 1; i >= 0; i--) {
            decalalivetime[i] += multiplier;
            if (decaltype[i] == blooddecalslow)
                decalalivetime[i] -= multiplier * 2 / 3;
            if (decaltype[i] == blooddecalfast)
                decalalivetime[i] += multiplier * 4;
            if (decaltype[i] == shadowdecal)
                DeleteDecal(i);
            if (decaltype[i] == footprintdecal && decalalivetime[i] >= 5)
                DeleteDecal(i);
            if (decaltype[i] == bodyprintdecal && decalalivetime[i] >= 5)
                DeleteDecal(i);
            if ((decaltype[i] == blooddecal || decaltype[i] == blooddecalfast || decaltype[i] == blooddecalslow) && decalalivetime[i] >= 60)
                DeleteDecal(i);
        }
        glAlphaFunc(GL_GREATER, 0.0001);
    }
}

void Terrain::AddObject(XYZ where, float radius, int id)
{
    bool done;
    int i, j;
    XYZ points[4];
    if (id >= 0 && id < 10000)
        for (i = 0; i < subdivision; i++) {
            for (j = 0; j < subdivision; j++) {
                if (patchobjectnum[i][j] < 300 - 1) {
                    done = 0;
                    points[0].x = (size / subdivision) * i;
                    points[0].z = (size / subdivision) * j;
                    points[0].y = heightmap[(int)points[0].x][(int)points[0].z];
                    points[1].x = (size / subdivision) * (i + 1);
                    points[1].z = (size / subdivision) * j;
                    points[1].y = heightmap[(int)points[1].x][(int)points[1].z];
                    points[2].x = (size / subdivision) * (i + 1);
                    points[2].z = (size / subdivision) * (j + 1);
                    points[2].y = heightmap[(int)points[2].x][(int)points[2].z];
                    points[3].x = (size / subdivision) * i;
                    points[3].z = (size / subdivision) * (j + 1);
                    points[3].y = heightmap[(int)points[3].x][(int)points[3].z];
                    points[0] *= scale;
                    points[1] *= scale;
                    points[2] *= scale;
                    points[3] *= scale;
                    if (!done && where.x + radius > points[0].x && where.x - radius < points[2].x && where.z + radius > points[0].z && where.z - radius < points[2].z) {
                        patchobjects[i][j][patchobjectnum[i][j]] = id;
                        patchobjectnum[i][j]++;
                        done = 1;
                    }
                }
            }
        }
}

void Terrain::DeleteDecal(int which)
{
    if (decals) {
        decaltype[which] = decaltype[numdecals - 1];
        decalposition[which] = decalposition[numdecals - 1];
        for (int i = 0; i < 3; i++) {
            decalvertex[which][i] = decalvertex[numdecals - 1][i];
            decaltexcoords[which][i][0] = decaltexcoords[numdecals - 1][i][0];
            decaltexcoords[which][i][1] = decaltexcoords[numdecals - 1][i][1];
        }
        decalrotation[which] = decalrotation[numdecals - 1];
        decalalivetime[which] = decalalivetime[numdecals - 1];
        decalopacity[which] = decalopacity[numdecals - 1];
        decalbrightness[which] = decalbrightness[numdecals - 1];
        numdecals--;
    }
}

void Terrain::MakeDecal(int type, XYZ where, float size, float opacity, float rotation)
{
    if (decals) {
        if (opacity > 0 && size > 0) {
            static int patchx[4];
            static int patchy[4];

            decaltexcoords[numdecals][0][0] = 1;
            decaltexcoords[numdecals][0][1] = 0;

            patchx[0] = (where.x + size) / scale;
            patchx[1] = (where.x - size) / scale;
            patchx[2] = (where.x - size) / scale;
            patchx[3] = (where.x + size) / scale;

            patchy[0] = (where.z - size) / scale;
            patchy[1] = (where.z - size) / scale;
            patchy[2] = (where.z + size) / scale;
            patchy[3] = (where.z + size) / scale;

            if ((patchx[0] != patchx[1] || patchy[0] != patchy[1]) && (patchx[0] != patchx[2] || patchy[0] != patchy[2]) && (patchx[0] != patchx[3] || patchy[0] != patchy[3])) {
                MakeDecalLock(type, where, patchx[0], patchy[0], size, opacity, rotation);
            }

            if ((patchx[1] != patchx[2] || patchy[1] != patchy[2]) && (patchx[1] != patchx[3] || patchy[1] != patchy[3])) {
                MakeDecalLock(type, where, patchx[1], patchy[1], size, opacity, rotation);
            }

            if ((patchx[2] != patchx[3] || patchy[2] != patchy[3])) {
                MakeDecalLock(type, where, patchx[2], patchy[2], size, opacity, rotation);
            }
            MakeDecalLock(type, where, patchx[3], patchy[3], size, opacity, rotation);
        }
    }
    //}
}

void Terrain::MakeDecalLock(int type, XYZ where, int whichx, int whichy, float size, float opacity, float rotation)
{
    if (decals) {
        static float placex, placez;
        static XYZ rot;

        float decalbright;

        rot = getLighting(where.x, where.z);
        decalbrightness[numdecals] = (rot.x + rot.y + rot.z) / 3;
        if (decalbrightness[numdecals] < .4)
            decalbrightness[numdecals] = .4;

        if (environment == grassyenvironment) {
            decalbrightness[numdecals] *= .6;
        }

        if (decalbrightness[numdecals] > 1)
            decalbrightness[numdecals] = 1;
        decalbright = decalbrightness[numdecals];

        decalposition[numdecals] = where;
        decaltype[numdecals] = type;
        decalopacity[numdecals] = opacity;
        decalrotation[numdecals] = rotation;
        decalalivetime[numdecals] = 0;

        placex = (float)whichx * scale + scale;
        placez = (float)whichy * scale;

        decaltexcoords[numdecals][0][0] = (placex - where.x) / size / 2 + .5;
        decaltexcoords[numdecals][0][1] = (placez - where.z) / size / 2 + .5;

        decalvertex[numdecals][0].x = placex;
        decalvertex[numdecals][0].z = placez;
        decalvertex[numdecals][0].y = heightmap[whichx + 1][whichy] * scale + .01;


        placex = (float)whichx * scale + scale;
        placez = (float)whichy * scale + scale;

        decaltexcoords[numdecals][1][0] = (placex - where.x) / size / 2 + .5;
        decaltexcoords[numdecals][1][1] = (placez - where.z) / size / 2 + .5;

        decalvertex[numdecals][1].x = placex;
        decalvertex[numdecals][1].z = placez;
        decalvertex[numdecals][1].y = heightmap[whichx + 1][whichy + 1] * scale + .01;


        placex = (float)whichx * scale;
        placez = (float)whichy * scale + scale;

        decaltexcoords[numdecals][2][0] = (placex - where.x) / size / 2 + .5;
        decaltexcoords[numdecals][2][1] = (placez - where.z) / size / 2 + .5;

        decalvertex[numdecals][2].x = placex;
        decalvertex[numdecals][2].z = placez;
        decalvertex[numdecals][2].y = heightmap[whichx][whichy + 1] * scale + .01;

        if (decalrotation[numdecals]) {
            for (int i = 0; i < 3; i++) {
                rot.y = 0;
                rot.x = decaltexcoords[numdecals][i][0] - .5;
                rot.z = decaltexcoords[numdecals][i][1] - .5;
                rot = DoRotation(rot, 0, -decalrotation[numdecals], 0);
                decaltexcoords[numdecals][i][0] = rot.x + .5;
                decaltexcoords[numdecals][i][1] = rot.z + .5;
            }
        }

        if (!(decaltexcoords[numdecals][0][0] < 0 && decaltexcoords[numdecals][1][0] < 0 && decaltexcoords[numdecals][2][0] < 0))
            if (!(decaltexcoords[numdecals][0][1] < 0 && decaltexcoords[numdecals][1][1] < 0 && decaltexcoords[numdecals][2][1] < 0))
                if (!(decaltexcoords[numdecals][0][0] > 1 && decaltexcoords[numdecals][1][0] > 1 && decaltexcoords[numdecals][2][0] > 1))
                    if (!(decaltexcoords[numdecals][0][1] > 1 && decaltexcoords[numdecals][1][1] > 1 && decaltexcoords[numdecals][2][1] > 1))
                        if (numdecals < max_decals - 1)
                            numdecals++;

        decalbrightness[numdecals] = decalbright;

        decalposition[numdecals] = where;
        decaltype[numdecals] = type;
        decalopacity[numdecals] = opacity;
        decalrotation[numdecals] = rotation;
        decalalivetime[numdecals] = 0;

        placex = (float)whichx * scale + scale;
        placez = (float)whichy * scale;

        decaltexcoords[numdecals][0][0] = (placex - where.x) / size / 2 + .5;
        decaltexcoords[numdecals][0][1] = (placez - where.z) / size / 2 + .5;

        decalvertex[numdecals][0].x = placex;
        decalvertex[numdecals][0].z = placez;
        decalvertex[numdecals][0].y = heightmap[whichx + 1][whichy] * scale + .01;


        placex = (float)whichx * scale;
        placez = (float)whichy * scale;

        decaltexcoords[numdecals][1][0] = (placex - where.x) / size / 2 + .5;
        decaltexcoords[numdecals][1][1] = (placez - where.z) / size / 2 + .5;

        decalvertex[numdecals][1].x = placex;
        decalvertex[numdecals][1].z = placez;
        decalvertex[numdecals][1].y = heightmap[whichx][whichy] * scale + .01;


        placex = (float)whichx * scale;
        placez = (float)whichy * scale + scale;

        decaltexcoords[numdecals][2][0] = (placex - where.x) / size / 2 + .5;
        decaltexcoords[numdecals][2][1] = (placez - where.z) / size / 2 + .5;

        decalvertex[numdecals][2].x = placex;
        decalvertex[numdecals][2].z = placez;
        decalvertex[numdecals][2].y = heightmap[whichx][whichy + 1] * scale + .01;

        if (decalrotation[numdecals]) {
            for (int i = 0; i < 3; i++) {
                rot.y = 0;
                rot.x = decaltexcoords[numdecals][i][0] - .5;
                rot.z = decaltexcoords[numdecals][i][1] - .5;
                rot = DoRotation(rot, 0, -decalrotation[numdecals], 0);
                decaltexcoords[numdecals][i][0] = rot.x + .5;
                decaltexcoords[numdecals][i][1] = rot.z + .5;
            }
        }

        if (!(decaltexcoords[numdecals][0][0] < 0 && decaltexcoords[numdecals][1][0] < 0 && decaltexcoords[numdecals][2][0] < 0))
            if (!(decaltexcoords[numdecals][0][1] < 0 && decaltexcoords[numdecals][1][1] < 0 && decaltexcoords[numdecals][2][1] < 0))
                if (!(decaltexcoords[numdecals][0][0] > 1 && decaltexcoords[numdecals][1][0] > 1 && decaltexcoords[numdecals][2][0] > 1))
                    if (!(decaltexcoords[numdecals][0][1] > 1 && decaltexcoords[numdecals][1][1] > 1 && decaltexcoords[numdecals][2][1] > 1))
                        if (numdecals < max_decals - 1)
                            numdecals++;
    }
}

void Terrain::DoShadows()
{
    static int i, j, k, l, todivide;
    static float brightness, total;
    static XYZ testpoint, testpoint2, terrainpoint, lightloc, col;
    lightloc = light.location;
    if (!skyboxtexture) {
        lightloc.x = 0;
        lightloc.z = 0;
    }
    if (skyboxtexture && tutoriallevel) {
        lightloc.x *= .4;
        lightloc.z *= .4;
    }
    int patchx, patchz;
    float shadowed;
    Normalise(&lightloc);
    //Calculate shadows
    for (i = 0; i < size; i++) {
        for (j = 0; j < size; j++) {
            terrainpoint.x = (float)(i) * scale;
            terrainpoint.z = (float)(j) * scale;
            terrainpoint.y = heightmap[i][j] * scale;

            shadowed = 0;
            patchx = (float)(i) * subdivision / size;
            patchz = (float)(j) * subdivision / size;
            if (patchobjectnum[patchx][patchz]) {
                for (k = 0; k < patchobjectnum[patchx][patchz]; k++) {
                    l = patchobjects[patchx][patchz][k];
                    if (objects.type[l] != treetrunktype) {
                        testpoint = terrainpoint;
                        testpoint2 = terrainpoint + lightloc * 50 * (1 - shadowed);
                        if (objects.model[l].LineCheck(&testpoint, &testpoint2, &col, &objects.position[l], &objects.yaw[l]) != -1) {
                            shadowed = 1 - (findDistance(&terrainpoint, &col) / 50);
                        }
                    }
                }
                if (visibleloading)
                    Game::LoadingScreen();
            }
            brightness = dotproduct(&lightloc, &normals[i][j]);
            if (shadowed)
                brightness *= 1 - shadowed;

            if (brightness > 1)
                brightness = 1;
            if (brightness < 0)
                brightness = 0;

            colors[i][j][0] = light.color[0] * brightness + light.ambient[0];
            colors[i][j][1] = light.color[1] * brightness + light.ambient[1];
            colors[i][j][2] = light.color[2] * brightness + light.ambient[2];

            if (colors[i][j][0] > 1) colors[i][j][0] = 1;
            if (colors[i][j][1] > 1) colors[i][j][1] = 1;
            if (colors[i][j][2] > 1) colors[i][j][2] = 1;
            if (colors[i][j][0] < 0) colors[i][j][0] = 0;
            if (colors[i][j][1] < 0) colors[i][j][1] = 0;
            if (colors[i][j][2] < 0) colors[i][j][2] = 0;
        }
    }

    if (visibleloading)
        Game::LoadingScreen();

    //Smooth shadows
    for (i = 0; i < size; i++) {
        for (j = 0; j < size; j++) {
            for (k = 0; k < 3; k++) {
                total = 0;
                todivide = 0;
                if (i != 0) {
                    total += colors[j][i - 1][k];
                    todivide++;
                }
                if (i != size - 1) {
                    total += colors[j][i + 1][k];
                    todivide++;
                }
                if (j != 0) {
                    total += colors[j - 1][i][k];
                    todivide++;
                }
                if (j != size - 1) {
                    total += colors[j + 1][i][k];
                    todivide++;
                }
                if (i != 0 && j != 0) {
                    total += colors[j - 1][i - 1][k];
                    todivide++;
                }
                if (i != size - 1 && j != 0) {
                    total += colors[j - 1][i + 1][k];
                    todivide++;
                }
                if (j != size - 1 && i != size - 1) {
                    total += colors[j + 1][i + 1][k];
                    todivide++;
                }
                if (j != size - 1 && i != 0) {
                    total += colors[j + 1][i - 1][k];
                    todivide++;
                }
                total += colors[j][i][k];
                todivide++;

                colors[j][i][k] = total / todivide;
            }
        }
    }

    for (i = 0; i < subdivision; i++) {
        for (j = 0; j < subdivision; j++) {
            UpdateVertexArray(i, j);
        }
    }
}

Terrain::Terrain()
{
    size = 0;

    memset(patchobjectnum, 0, sizeof(patchobjectnum));
    memset(patchobjects, 0, sizeof(patchobjects));

    scale = 1.0f;
    type = 0;
    memset(heightmap, 0, sizeof(heightmap));
    memset(normals, 0, sizeof(normals));
    memset(facenormals, 0, sizeof(facenormals));
    memset(triangles, 0, sizeof(triangles));
    memset(colors, 0, sizeof(colors));
    memset(opacityother, 0, sizeof(opacityother));
    memset(texoffsetx, 0, sizeof(texoffsetx));
    memset(texoffsety, 0, sizeof(texoffsety));
    memset(numtris, 0, sizeof(numtris));
    memset(textureness, 0, sizeof(textureness));

    memset(vArray, 0, sizeof(vArray));

    memset(visible, 0, sizeof(visible));
    memset(avgypatch, 0, sizeof(avgypatch));
    memset(maxypatch, 0, sizeof(maxypatch));
    memset(minypatch, 0, sizeof(minypatch));
    memset(heightypatch, 0, sizeof(heightypatch));

    patch_elements = 0;

    memset(decaltexcoords, 0, sizeof(decaltexcoords));
    memset(decalvertex, 0, sizeof(decalvertex));
    memset(decaltype, 0, sizeof(decaltype));
    memset(decalopacity, 0, sizeof(decalopacity));
    memset(decalrotation, 0, sizeof(decalrotation));
    memset(decalalivetime, 0, sizeof(decalalivetime));
    memset(decalbrightness, 0, sizeof(decalbrightness));
    memset(decalposition, 0, sizeof(decalposition));
    numdecals = 0;
}
Terrain::~Terrain()
{
    terraintexture.destroy();
    shadowtexture.destroy();
    bodyprinttexture.destroy();
    footprinttexture.destroy();
    bloodtexture.destroy();
    bloodtexture2.destroy();
    breaktexture.destroy();
}

