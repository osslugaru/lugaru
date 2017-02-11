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

#ifndef _TERRAIN_HPP_
#define _TERRAIN_HPP_

#include "Environment/Lights.hpp"
#include "Graphic/Decal.hpp"
#include "Graphic/Texture.hpp"
#include "Graphic/gamegl.hpp"
#include "Math/Frustum.hpp"
#include "Math/XYZ.hpp"
#include "Utils/ImageIO.hpp"

#define max_terrain_size 256
#define curr_terrain_size size
#define subdivision 64
#define max_patch_elements (max_terrain_size / subdivision) * (max_terrain_size / subdivision) * 54

#define allfirst 0
#define mixed 1
#define allsecond 2

#define max_decals 1000

#define snowyenvironment 0
#define grassyenvironment 1
#define desertenvironment 2
//
// Model Structures
//

class Terrain
{
public:
    Texture bloodtexture;
    Texture bloodtexture2;
    Texture shadowtexture;
    Texture footprinttexture;
    Texture bodyprinttexture;
    Texture breaktexture;
    Texture terraintexture;
    short size;

    std::vector<unsigned int> patchobjects[subdivision][subdivision];

    float scale;
    int type;
    float heightmap[max_terrain_size + 1][max_terrain_size + 1];
    XYZ normals[max_terrain_size][max_terrain_size];
    XYZ facenormals[max_terrain_size][max_terrain_size];
    XYZ triangles[(max_terrain_size - 1) * (max_terrain_size - 1) * 2][3];
    float colors[max_terrain_size][max_terrain_size][4];
    float opacityother[max_terrain_size][max_terrain_size];
    float texoffsetx[max_terrain_size][max_terrain_size];
    float texoffsety[max_terrain_size][max_terrain_size];
    int numtris[subdivision][subdivision];
    int textureness[subdivision][subdivision];

    GLfloat vArray[(max_patch_elements)*subdivision * subdivision];

    bool visible[subdivision][subdivision];
    float avgypatch[subdivision][subdivision];
    float maxypatch[subdivision][subdivision];
    float minypatch[subdivision][subdivision];
    float heightypatch[subdivision][subdivision];

    int patch_elements;

    std::vector<Decal> decals;

    void AddObject(XYZ where, float radius, int id);
    void DeleteObject(unsigned int id);
    void DeleteDecal(int which);
    void MakeDecal(decal_type type, XYZ where, float size, float opacity, float rotation);
    void MakeDecalLock(decal_type type, XYZ where, int whichx, int whichy, float size, float opacity, float rotation);
    int lineTerrain(XYZ p1, XYZ p2, XYZ* p);
    float getHeight(float pointx, float pointz);
    float getOpacity(float pointx, float pointz);
    XYZ getLighting(float pointx, float pointz);
    XYZ getNormal(float pointx, float pointz);
    void UpdateVertexArray(int whichx, int whichy);
    bool load(const std::string& fileName);
    void CalculateNormals();
    void drawdecals();
    void draw(int layer);
    void DoShadows();
    void deleteDeadDecals();

    Terrain();

private:
    void drawpatch(int whichx, int whichy, float opacity);
    void drawpatchother(int whichx, int whichy, float opacity);
    void drawpatchotherother(int whichx, int whichy);
    void UpdateTransparency(int whichx, int whichy);
    void UpdateTransparencyother(int whichx, int whichy);
    void UpdateTransparencyotherother(int whichx, int whichy);
};

#endif
