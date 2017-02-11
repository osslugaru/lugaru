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

#ifndef _MODELS_HPP_
#define _MODELS_HPP_

#include "Environment/Terrain.hpp"
#include "Graphic/Texture.hpp"
#include "Graphic/gamegl.hpp"
#include "Math/XYZ.hpp"
#include "Utils/binio.h"

#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <vector>

//
// Textures List
//
typedef struct
{
    long xsz, ysz;
    GLubyte* txt;
} ModelTexture;

//
// Model Structures
//

class TexturedTriangle
{
public:
    short vertex[3];
    float gx[3], gy[3];
    XYZ facenormal;
};

#define max_model_decals 300

enum ModelType
{
    nothing = 0,
    notextype = 1,
    rawtype = 2,
    decalstype = 3,
    normaltype = 4
};

class Model
{
public:
    short vertexNum;

    ModelType type;

    int* owner;
    XYZ* vertex;
    XYZ* normals;
    std::vector<TexturedTriangle> Triangles;
    GLfloat* vArray;

    /*
    int owner[max_textured_triangle];
    XYZ vertex[max_model_vertex];
    XYZ normals[max_model_vertex];
    GLfloat vArray[max_textured_triangle*24];*/

    Texture textureptr;
    ModelTexture modelTexture;
    bool color;

    XYZ boundingspherecenter;
    float boundingsphereradius;

    std::vector<Decal> decals;

    bool flat;

    Model();
    ~Model();
    void DeleteDecal(int which);
    void MakeDecal(decal_type atype, XYZ* where, float* size, float* opacity, float* rotation);
    void MakeDecal(decal_type atype, XYZ where, float size, float opacity, float rotation);
    const XYZ& getTriangleVertex(unsigned triangleId, unsigned vertexId) const;
    void drawdecals(Texture shadowtexture, Texture bloodtexture, Texture bloodtexture2, Texture breaktexture);
    int SphereCheck(XYZ* p1, float radius, XYZ* p, XYZ* move, float* rotate);
    int SphereCheckPossible(XYZ* p1, float radius, XYZ* move, float* rotate);
    int LineCheck(XYZ* p1, XYZ* p2, XYZ* p, XYZ* move, float* rotate);
    int LineCheckPossible(XYZ* p1, XYZ* p2, XYZ* p, XYZ* move, float* rotate);
    int LineCheckSlidePossible(XYZ* p1, XYZ* p2, XYZ* move, float* rotate);
    void UpdateVertexArray();
    void UpdateVertexArrayNoTex();
    void UpdateVertexArrayNoTexNoNorm();
    bool loadnotex(const std::string& filename);
    bool loadraw(const std::string& filename);
    bool load(const std::string& filename);
    bool loaddecal(const std::string& filename);
    void Scale(float xscale, float yscale, float zscale);
    void FlipTexCoords();
    void UniformTexCoords();
    void ScaleTexCoords(float howmuch);
    void ScaleNormals(float xscale, float yscale, float zscale);
    void Translate(float xtrans, float ytrans, float ztrans);
    void CalculateNormals(bool facenormalise);
    void draw();
    void drawdifftex(Texture texture);
    void drawimmediate();
    void Rotate(float xang, float yang, float zang);
    void deleteDeadDecals();

private:
    void deallocate();
    /* indices of triangles that might collide */
    std::vector<unsigned int> possible;
};

#endif
