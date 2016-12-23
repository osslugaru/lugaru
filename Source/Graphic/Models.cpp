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

#include "Graphic/Models.hpp"

#include "Game.hpp"
#include "Utils/Folders.hpp"

extern float multiplier;
extern float viewdistance;
extern XYZ viewer;
extern float fadestart;
extern float texdetail;
extern bool decalstoggle;

int Model::LineCheck(XYZ *p1, XYZ *p2, XYZ *p, XYZ *move, float *rotate)
{
    static int j;
    static float distance;
    static float olddistance;
    static int intersecting;
    static int firstintersecting;
    static XYZ point;

    *p1 = *p1 - *move;
    *p2 = *p2 - *move;
    if (*rotate)
        *p1 = DoRotation(*p1, 0, -*rotate, 0);
    if (*rotate)
        *p2 = DoRotation(*p2, 0, -*rotate, 0);
    if (!sphere_line_intersection(p1, p2, &boundingspherecenter, &boundingsphereradius))
        return -1;
    firstintersecting = -1;

    for (j = 0; j < TriangleNum; j++) {
        intersecting = LineFacetd(p1, p2, &vertex[Triangles[j].vertex[0]], &vertex[Triangles[j].vertex[1]], &vertex[Triangles[j].vertex[2]], &facenormals[j], &point);
        distance = (point.x - p1->x) * (point.x - p1->x) + (point.y - p1->y) * (point.y - p1->y) + (point.z - p1->z) * (point.z - p1->z);
        if ((distance < olddistance || firstintersecting == -1) && intersecting) {
            olddistance = distance;
            firstintersecting = j;
            *p = point;
        }
    }

    if (*rotate)
        *p = DoRotation(*p, 0, *rotate, 0);
    *p = *p + *move;
    return firstintersecting;
}

int Model::LineCheckPossible(XYZ *p1, XYZ *p2, XYZ *p, XYZ *move, float *rotate)
{
    static int j;
    static float distance;
    static float olddistance;
    static int intersecting;
    static int firstintersecting;
    static XYZ point;

    *p1 = *p1 - *move;
    *p2 = *p2 - *move;
    if (!sphere_line_intersection(p1, p2, &boundingspherecenter, &boundingsphereradius))
        return -1;
    firstintersecting = -1;
    if (*rotate)
        *p1 = DoRotation(*p1, 0, -*rotate, 0);
    if (*rotate)
        *p2 = DoRotation(*p2, 0, -*rotate, 0);

    if (numpossible > 0 && numpossible < TriangleNum)
        for (j = 0; j < numpossible; j++) {
            if (possible[j] >= 0 && possible[j] < TriangleNum) {
                intersecting = LineFacetd(p1, p2, &vertex[Triangles[possible[j]].vertex[0]], &vertex[Triangles[possible[j]].vertex[1]], &vertex[Triangles[possible[j]].vertex[2]], &facenormals[possible[j]], &point);
                distance = (point.x - p1->x) * (point.x - p1->x) + (point.y - p1->y) * (point.y - p1->y) + (point.z - p1->z) * (point.z - p1->z);
                if ((distance < olddistance || firstintersecting == -1) && intersecting) {
                    olddistance = distance;
                    firstintersecting = possible[j];
                    *p = point;
                }
            }
        }

    if (*rotate)
        *p = DoRotation(*p, 0, *rotate, 0);
    *p = *p + *move;
    return firstintersecting;
}

int Model::LineCheckSlidePossible(XYZ *p1, XYZ *p2, XYZ *move, float *rotate)
{
    static int j;
    static float distance;
    static float olddistance;
    static int intersecting;
    static int firstintersecting;
    static XYZ point;

    *p1 = *p1 - *move;
    *p2 = *p2 - *move;
    if (!sphere_line_intersection(p1, p2, &boundingspherecenter, &boundingsphereradius))
        return -1;
    firstintersecting = -1;
    if (*rotate)
        *p1 = DoRotation(*p1, 0, -*rotate, 0);
    if (*rotate)
        *p2 = DoRotation(*p2, 0, -*rotate, 0);

    if (numpossible)
        for (j = 0; j < numpossible; j++) {
            if (possible[j] >= 0 && possible[j] < TriangleNum) {
                intersecting = LineFacetd(p1, p2, &vertex[Triangles[possible[j]].vertex[0]], &vertex[Triangles[possible[j]].vertex[1]], &vertex[Triangles[possible[j]].vertex[2]], &facenormals[possible[j]], &point);
                distance = (point.x - p1->x) * (point.x - p1->x) + (point.y - p1->y) * (point.y - p1->y) + (point.z - p1->z) * (point.z - p1->z);
                if ((distance < olddistance || firstintersecting == -1) && intersecting) {
                    olddistance = distance;
                    firstintersecting = possible[j];
                }
            }
        }

    if (firstintersecting > 0) {
        distance = abs((facenormals[firstintersecting].x * p2->x) + (facenormals[firstintersecting].y * p2->y) + (facenormals[firstintersecting].z * p2->z) - ((facenormals[firstintersecting].x * vertex[Triangles[firstintersecting].vertex[0]].x) + (facenormals[firstintersecting].y * vertex[Triangles[firstintersecting].vertex[0]].y) + (facenormals[firstintersecting].z * vertex[Triangles[firstintersecting].vertex[0]].z)));
        *p2 -= facenormals[firstintersecting] * distance;
    }

    if (*rotate)
        *p2 = DoRotation(*p2, 0, *rotate, 0);
    *p2 = *p2 + *move;
    return firstintersecting;
}

int Model::SphereCheck(XYZ *p1, float radius, XYZ *p, XYZ *move, float *rotate)
{
    static int i, j;
    static float distance;
    static float olddistance;
    static int intersecting;
    static int firstintersecting;
    static XYZ point;
    static XYZ oldp1;

    firstintersecting = -1;

    oldp1 = *p1;
    *p1 = *p1 - *move;
    if (*rotate)
        *p1 = DoRotation(*p1, 0, -*rotate, 0);
    if (distsq(p1, &boundingspherecenter) > radius * radius + boundingsphereradius * boundingsphereradius)
        return -1;

    for (i = 0; i < 4; i++) {
        for (j = 0; j < TriangleNum; j++) {
            intersecting = 0;
            distance = abs((facenormals[j].x * p1->x) + (facenormals[j].y * p1->y) + (facenormals[j].z * p1->z) - ((facenormals[j].x * vertex[Triangles[j].vertex[0]].x) + (facenormals[j].y * vertex[Triangles[j].vertex[0]].y) + (facenormals[j].z * vertex[Triangles[j].vertex[0]].z)));
            if (distance < radius) {
                point = *p1 - facenormals[j] * distance;
                if (PointInTriangle( &point, facenormals[j], &vertex[Triangles[j].vertex[0]], &vertex[Triangles[j].vertex[1]], &vertex[Triangles[j].vertex[2]]))
                    intersecting = 1;
                if (!intersecting)
                    intersecting = sphere_line_intersection(&vertex[Triangles[j].vertex[0]], &vertex[Triangles[j].vertex[1]], p1, &radius);
                if (!intersecting)
                    intersecting = sphere_line_intersection(&vertex[Triangles[j].vertex[1]], &vertex[Triangles[j].vertex[2]], p1, &radius);
                if (!intersecting)
                    intersecting = sphere_line_intersection(&vertex[Triangles[j].vertex[0]], &vertex[Triangles[j].vertex[2]], p1, &radius);
                if (intersecting) {
                    *p1 += facenormals[j] * (distance - radius);
                }
            }
            if ((distance < olddistance || firstintersecting == -1) && intersecting) {
                olddistance = distance;
                firstintersecting = j;
                *p = point;
            }
        }
    }
    if (*rotate)
        *p = DoRotation(*p, 0, *rotate, 0);
    *p = *p + *move;
    if (*rotate)
        *p1 = DoRotation(*p1, 0, *rotate, 0);
    *p1 += *move;
    return firstintersecting;
}

int Model::SphereCheckPossible(XYZ *p1, float radius, XYZ *move, float *rotate)
{
    static int j;
    static float distance;
    static float olddistance;
    static int intersecting;
    static int firstintersecting;
    static XYZ point;
    static XYZ oldp1;

    firstintersecting = -1;

    oldp1 = *p1;
    *p1 = *p1 - *move;

    numpossible = 0;

    if (*rotate)
        *p1 = DoRotation(*p1, 0, -*rotate, 0);
    if (distsq(p1, &boundingspherecenter) > radius * radius + boundingsphereradius * boundingsphereradius) {
        *p1 = oldp1;
        return -1;
    }

    for (j = 0; j < TriangleNum; j++) {
        intersecting = 0;
        distance = abs((facenormals[j].x * p1->x) + (facenormals[j].y * p1->y) + (facenormals[j].z * p1->z) - ((facenormals[j].x * vertex[Triangles[j].vertex[0]].x) + (facenormals[j].y * vertex[Triangles[j].vertex[0]].y) + (facenormals[j].z * vertex[Triangles[j].vertex[0]].z)));
        if (distance < radius) {
            point = *p1 - facenormals[j] * distance;
            if (PointInTriangle( &point, facenormals[j], &vertex[Triangles[j].vertex[0]], &vertex[Triangles[j].vertex[1]], &vertex[Triangles[j].vertex[2]]))
                intersecting = 1;
            if (!intersecting)
                intersecting = sphere_line_intersection(&vertex[Triangles[j].vertex[0]], &vertex[Triangles[j].vertex[1]], p1, &radius);
            if (!intersecting)
                intersecting = sphere_line_intersection(&vertex[Triangles[j].vertex[1]], &vertex[Triangles[j].vertex[2]], p1, &radius);
            if (!intersecting)
                intersecting = sphere_line_intersection(&vertex[Triangles[j].vertex[0]], &vertex[Triangles[j].vertex[2]], p1, &radius);
            if (intersecting) {
                possible[numpossible] = j;
                numpossible++;
            }
        }
        if ((distance < olddistance || firstintersecting == -1) && intersecting) {
            olddistance = distance;
            firstintersecting = j;
        }
    }
    if (*rotate)
        *p1 = DoRotation(*p1, 0, *rotate, 0);
    *p1 += *move;
    return firstintersecting;
}


void Model::UpdateVertexArray()
{
    if (type != normaltype && type != decalstype)
        return;
    static int i;
    static int j;
    if (!flat)
        for (i = 0; i < TriangleNum; i++) {
            j = i * 24;
            vArray[j + 0] = Triangles[i].gx[0];
            vArray[j + 1] = Triangles[i].gy[0];
            vArray[j + 2] = normals[Triangles[i].vertex[0]].x;
            vArray[j + 3] = normals[Triangles[i].vertex[0]].y;
            vArray[j + 4] = normals[Triangles[i].vertex[0]].z;
            vArray[j + 5] = vertex[Triangles[i].vertex[0]].x;
            vArray[j + 6] = vertex[Triangles[i].vertex[0]].y;
            vArray[j + 7] = vertex[Triangles[i].vertex[0]].z;

            vArray[j + 8] = Triangles[i].gx[1];
            vArray[j + 9] = Triangles[i].gy[1];
            vArray[j + 10] = normals[Triangles[i].vertex[1]].x;
            vArray[j + 11] = normals[Triangles[i].vertex[1]].y;
            vArray[j + 12] = normals[Triangles[i].vertex[1]].z;
            vArray[j + 13] = vertex[Triangles[i].vertex[1]].x;
            vArray[j + 14] = vertex[Triangles[i].vertex[1]].y;
            vArray[j + 15] = vertex[Triangles[i].vertex[1]].z;

            vArray[j + 16] = Triangles[i].gx[2];
            vArray[j + 17] = Triangles[i].gy[2];
            vArray[j + 18] = normals[Triangles[i].vertex[2]].x;
            vArray[j + 19] = normals[Triangles[i].vertex[2]].y;
            vArray[j + 20] = normals[Triangles[i].vertex[2]].z;
            vArray[j + 21] = vertex[Triangles[i].vertex[2]].x;
            vArray[j + 22] = vertex[Triangles[i].vertex[2]].y;
            vArray[j + 23] = vertex[Triangles[i].vertex[2]].z;
        }
    if (flat)
        for (i = 0; i < TriangleNum; i++) {
            j = i * 24;
            vArray[j + 0] = Triangles[i].gx[0];
            vArray[j + 1] = Triangles[i].gy[0];
            vArray[j + 2] = facenormals[i].x * -1;
            vArray[j + 3] = facenormals[i].y * -1;
            vArray[j + 4] = facenormals[i].z * -1;
            vArray[j + 5] = vertex[Triangles[i].vertex[0]].x;
            vArray[j + 6] = vertex[Triangles[i].vertex[0]].y;
            vArray[j + 7] = vertex[Triangles[i].vertex[0]].z;

            vArray[j + 8] = Triangles[i].gx[1];
            vArray[j + 9] = Triangles[i].gy[1];
            vArray[j + 10] = facenormals[i].x * -1;
            vArray[j + 11] = facenormals[i].y * -1;
            vArray[j + 12] = facenormals[i].z * -1;
            vArray[j + 13] = vertex[Triangles[i].vertex[1]].x;
            vArray[j + 14] = vertex[Triangles[i].vertex[1]].y;
            vArray[j + 15] = vertex[Triangles[i].vertex[1]].z;

            vArray[j + 16] = Triangles[i].gx[2];
            vArray[j + 17] = Triangles[i].gy[2];
            vArray[j + 18] = facenormals[i].x * -1;
            vArray[j + 19] = facenormals[i].y * -1;
            vArray[j + 20] = facenormals[i].z * -1;
            vArray[j + 21] = vertex[Triangles[i].vertex[2]].x;
            vArray[j + 22] = vertex[Triangles[i].vertex[2]].y;
            vArray[j + 23] = vertex[Triangles[i].vertex[2]].z;

        }
}

void Model::UpdateVertexArrayNoTex()
{
    if (type != normaltype && type != decalstype)
        return;
    static int i;
    static int j;
    if (!flat)
        for (i = 0; i < TriangleNum; i++) {
            j = i * 24;
            vArray[j + 2] = normals[Triangles[i].vertex[0]].x;
            vArray[j + 3] = normals[Triangles[i].vertex[0]].y;
            vArray[j + 4] = normals[Triangles[i].vertex[0]].z;
            vArray[j + 5] = vertex[Triangles[i].vertex[0]].x;
            vArray[j + 6] = vertex[Triangles[i].vertex[0]].y;
            vArray[j + 7] = vertex[Triangles[i].vertex[0]].z;

            vArray[j + 10] = normals[Triangles[i].vertex[1]].x;
            vArray[j + 11] = normals[Triangles[i].vertex[1]].y;
            vArray[j + 12] = normals[Triangles[i].vertex[1]].z;
            vArray[j + 13] = vertex[Triangles[i].vertex[1]].x;
            vArray[j + 14] = vertex[Triangles[i].vertex[1]].y;
            vArray[j + 15] = vertex[Triangles[i].vertex[1]].z;

            vArray[j + 18] = normals[Triangles[i].vertex[2]].x;
            vArray[j + 19] = normals[Triangles[i].vertex[2]].y;
            vArray[j + 20] = normals[Triangles[i].vertex[2]].z;
            vArray[j + 21] = vertex[Triangles[i].vertex[2]].x;
            vArray[j + 22] = vertex[Triangles[i].vertex[2]].y;
            vArray[j + 23] = vertex[Triangles[i].vertex[2]].z;
        }
    if (flat)
        for (i = 0; i < TriangleNum; i++) {
            j = i * 24;
            vArray[j + 2] = facenormals[i].x * -1;
            vArray[j + 3] = facenormals[i].y * -1;
            vArray[j + 4] = facenormals[i].z * -1;
            vArray[j + 5] = vertex[Triangles[i].vertex[0]].x;
            vArray[j + 6] = vertex[Triangles[i].vertex[0]].y;
            vArray[j + 7] = vertex[Triangles[i].vertex[0]].z;

            vArray[j + 10] = facenormals[i].x * -1;
            vArray[j + 11] = facenormals[i].y * -1;
            vArray[j + 12] = facenormals[i].z * -1;
            vArray[j + 13] = vertex[Triangles[i].vertex[1]].x;
            vArray[j + 14] = vertex[Triangles[i].vertex[1]].y;
            vArray[j + 15] = vertex[Triangles[i].vertex[1]].z;

            vArray[j + 18] = facenormals[i].x * -1;
            vArray[j + 19] = facenormals[i].y * -1;
            vArray[j + 20] = facenormals[i].z * -1;
            vArray[j + 21] = vertex[Triangles[i].vertex[2]].x;
            vArray[j + 22] = vertex[Triangles[i].vertex[2]].y;
            vArray[j + 23] = vertex[Triangles[i].vertex[2]].z;
        }
}

void Model::UpdateVertexArrayNoTexNoNorm()
{
    if (type != normaltype && type != decalstype)
        return;
    static int i;
    static int j;
    for (i = 0; i < TriangleNum; i++) {
        j = i * 24;
        vArray[j + 5] = vertex[Triangles[i].vertex[0]].x;
        vArray[j + 6] = vertex[Triangles[i].vertex[0]].y;
        vArray[j + 7] = vertex[Triangles[i].vertex[0]].z;

        vArray[j + 13] = vertex[Triangles[i].vertex[1]].x;
        vArray[j + 14] = vertex[Triangles[i].vertex[1]].y;
        vArray[j + 15] = vertex[Triangles[i].vertex[1]].z;

        vArray[j + 21] = vertex[Triangles[i].vertex[2]].x;
        vArray[j + 22] = vertex[Triangles[i].vertex[2]].y;
        vArray[j + 23] = vertex[Triangles[i].vertex[2]].z;
    }
}

bool Model::loadnotex(const std::string& filename)
{
    FILE *tfile;
    long i;

    type = notextype;
    color = 0;

    tfile = Folders::openMandatoryFile( Folders::getResourcePath(filename), "rb" );

    // read model settings

    fseek(tfile, 0, SEEK_SET);
    funpackf(tfile, "Bs Bs", &vertexNum, &TriangleNum);

    // read the model data
    deallocate();

    numpossible = 0;

    owner = (int*)malloc(sizeof(int) * vertexNum);
    possible = (int*)malloc(sizeof(int) * TriangleNum);
    vertex = (XYZ*)malloc(sizeof(XYZ) * vertexNum);
    Triangles = (TexturedTriangle*)malloc(sizeof(TexturedTriangle) * TriangleNum);
    vArray = (GLfloat*)malloc(sizeof(GLfloat) * TriangleNum * 24);

    for (i = 0; i < vertexNum; i++) {
        funpackf(tfile, "Bf Bf Bf", &vertex[i].x, &vertex[i].y, &vertex[i].z);
    }

    for (i = 0; i < TriangleNum; i++) {
        short vertex[ 6];
        funpackf(tfile, "Bs Bs Bs Bs Bs Bs", &vertex[ 0], &vertex[ 1], &vertex[ 2], &vertex[ 3], &vertex[ 4], &vertex[ 5]);
        Triangles[i].vertex[ 0] = vertex[ 0];
        Triangles[i].vertex[ 1] = vertex[ 2];
        Triangles[i].vertex[ 2] = vertex[ 4];
        funpackf(tfile, "Bf Bf Bf", &Triangles[i].gx[0], &Triangles[i].gx[1], &Triangles[i].gx[2]);
        funpackf(tfile, "Bf Bf Bf", &Triangles[i].gy[0], &Triangles[i].gy[1], &Triangles[i].gy[2]);
    }

    fclose(tfile);

    UpdateVertexArray();

    for (i = 0; i < vertexNum; i++) {
        owner[i] = -1;
    }

    static int j;
    boundingsphereradius = 0;
    for (i = 0; i < vertexNum; i++) {
        for (j = 0; j < vertexNum; j++) {
            if (j != i && distsq(&vertex[j], &vertex[i]) / 2 > boundingsphereradius) {
                boundingsphereradius = distsq(&vertex[j], &vertex[i]) / 2;
                boundingspherecenter = (vertex[i] + vertex[j]) / 2;
            }
        }
    }
    boundingsphereradius = fast_sqrt(boundingsphereradius);

    return true;
}


bool Model::load(const std::string& filename)
{
    FILE *tfile;
    long i;

    LOGFUNC;

    LOG(std::string("Loading model...") + filename);

    Game::LoadingScreen();

    type = normaltype;
    color = 0;

    tfile = Folders::openMandatoryFile( Folders::getResourcePath(filename), "rb" );

    // read model settings

    fseek(tfile, 0, SEEK_SET);
    funpackf(tfile, "Bs Bs", &vertexNum, &TriangleNum);

    // read the model data
    deallocate();

    numpossible = 0;

    owner = (int*)malloc(sizeof(int) * vertexNum);
    possible = (int*)malloc(sizeof(int) * TriangleNum);
    vertex = (XYZ*)malloc(sizeof(XYZ) * vertexNum);
    normals = (XYZ*)malloc(sizeof(XYZ) * vertexNum);
    facenormals = (XYZ*)malloc(sizeof(XYZ) * TriangleNum);
    Triangles = (TexturedTriangle*)malloc(sizeof(TexturedTriangle) * TriangleNum);
    vArray = (GLfloat*)malloc(sizeof(GLfloat) * TriangleNum * 24);

    for (i = 0; i < vertexNum; i++) {
        funpackf(tfile, "Bf Bf Bf", &vertex[i].x, &vertex[i].y, &vertex[i].z);
    }

    for (i = 0; i < TriangleNum; i++) {
        short vertex[ 6];
        funpackf(tfile, "Bs Bs Bs Bs Bs Bs", &vertex[ 0], &vertex[ 1], &vertex[ 2], &vertex[ 3], &vertex[ 4], &vertex[ 5]);
        Triangles[i].vertex[ 0] = vertex[ 0];
        Triangles[i].vertex[ 1] = vertex[ 2];
        Triangles[i].vertex[ 2] = vertex[ 4];
        funpackf(tfile, "Bf Bf Bf", &Triangles[i].gx[0], &Triangles[i].gx[1], &Triangles[i].gx[2]);
        funpackf(tfile, "Bf Bf Bf", &Triangles[i].gy[0], &Triangles[i].gy[1], &Triangles[i].gy[2]);
    }

    modelTexture.xsz = 0;

    fclose(tfile);

    UpdateVertexArray();

    for (i = 0; i < vertexNum; i++) {
        owner[i] = -1;
    }

    static int j;
    boundingsphereradius = 0;
    for (i = 0; i < vertexNum; i++) {
        for (j = 0; j < vertexNum; j++) {
            if (j != i && distsq(&vertex[j], &vertex[i]) / 2 > boundingsphereradius) {
                boundingsphereradius = distsq(&vertex[j], &vertex[i]) / 2;
                boundingspherecenter = (vertex[i] + vertex[j]) / 2;
            }
        }
    }
    boundingsphereradius = fast_sqrt(boundingsphereradius);

    return true;
}

bool Model::loaddecal(const std::string& filename)
{
    FILE *tfile;
    long i, j;

    LOGFUNC;

    LOG(std::string("Loading decal...") + Folders::getResourcePath(filename));

    type = decalstype;
    decals.clear();
    color = 0;

    tfile = Folders::openMandatoryFile( Folders::getResourcePath(filename), "rb" );

    // read model settings

    fseek(tfile, 0, SEEK_SET);
    funpackf(tfile, "Bs Bs", &vertexNum, &TriangleNum);

    // read the model data

    deallocate();

    numpossible = 0;

    owner = (int*)malloc(sizeof(int) * vertexNum);
    possible = (int*)malloc(sizeof(int) * TriangleNum);
    vertex = (XYZ*)malloc(sizeof(XYZ) * vertexNum);
    normals = (XYZ*)malloc(sizeof(XYZ) * vertexNum);
    facenormals = (XYZ*)malloc(sizeof(XYZ) * TriangleNum);
    Triangles = (TexturedTriangle*)malloc(sizeof(TexturedTriangle) * TriangleNum);
    vArray = (GLfloat*)malloc(sizeof(GLfloat) * TriangleNum * 24);


    for (i = 0; i < vertexNum; i++) {
        funpackf(tfile, "Bf Bf Bf", &vertex[i].x, &vertex[i].y, &vertex[i].z);
    }

    for (i = 0; i < TriangleNum; i++) {
        short vertex[ 6];
        funpackf(tfile, "Bs Bs Bs Bs Bs Bs", &vertex[ 0], &vertex[ 1], &vertex[ 2], &vertex[ 3], &vertex[ 4], &vertex[ 5]);
        Triangles[i].vertex[ 0] = vertex[ 0];
        Triangles[i].vertex[ 1] = vertex[ 2];
        Triangles[i].vertex[ 2] = vertex[ 4];
        funpackf(tfile, "Bf Bf Bf", &Triangles[i].gx[0], &Triangles[i].gx[1], &Triangles[i].gx[2]);
        funpackf(tfile, "Bf Bf Bf", &Triangles[i].gy[0], &Triangles[i].gy[1], &Triangles[i].gy[2]);
    }


    modelTexture.xsz = 0;

    fclose(tfile);

    UpdateVertexArray();

    for (i = 0; i < vertexNum; i++) {
        owner[i] = -1;
    }

    boundingsphereradius = 0;
    for (i = 0; i < vertexNum; i++) {
        for (j = 0; j < vertexNum; j++) {
            if (j != i && distsq(&vertex[j], &vertex[i]) / 2 > boundingsphereradius) {
                boundingsphereradius = distsq(&vertex[j], &vertex[i]) / 2;
                boundingspherecenter = (vertex[i] + vertex[j]) / 2;
            }
        }
    }
    boundingsphereradius = fast_sqrt(boundingsphereradius);

    return true;
}

bool Model::loadraw(const std::string& filename)
{
    FILE *tfile;
    long i;

    LOGFUNC;

    LOG(std::string("Loading raw...") + filename);

    type = rawtype;
    color = 0;

    tfile = Folders::openMandatoryFile( Folders::getResourcePath(filename), "rb" );

    // read model settings

    fseek(tfile, 0, SEEK_SET);
    funpackf(tfile, "Bs Bs", &vertexNum, &TriangleNum);

    // read the model data
    deallocate();

    numpossible = 0;

    owner = (int*)malloc(sizeof(int) * vertexNum);
    possible = (int*)malloc(sizeof(int) * TriangleNum);
    vertex = (XYZ*)malloc(sizeof(XYZ) * vertexNum);
    Triangles = (TexturedTriangle*)malloc(sizeof(TexturedTriangle) * TriangleNum);
    vArray = (GLfloat*)malloc(sizeof(GLfloat) * TriangleNum * 24);


    for (i = 0; i < vertexNum; i++) {
        funpackf(tfile, "Bf Bf Bf", &vertex[i].x, &vertex[i].y, &vertex[i].z);
    }

    for (i = 0; i < TriangleNum; i++) {
        short vertex[ 6];
        funpackf(tfile, "Bs Bs Bs Bs Bs Bs", &vertex[ 0], &vertex[ 1], &vertex[ 2], &vertex[ 3], &vertex[ 4], &vertex[ 5]);
        Triangles[i].vertex[ 0] = vertex[ 0];
        Triangles[i].vertex[ 1] = vertex[ 2];
        Triangles[i].vertex[ 2] = vertex[ 4];
        funpackf(tfile, "Bf Bf Bf", &Triangles[i].gx[0], &Triangles[i].gx[1], &Triangles[i].gx[2]);
        funpackf(tfile, "Bf Bf Bf", &Triangles[i].gy[0], &Triangles[i].gy[1], &Triangles[i].gy[2]);
    }


    fclose(tfile);

    for (i = 0; i < vertexNum; i++) {
        owner[i] = -1;
    }

    return true;
}


void Model::UniformTexCoords()
{
    static int i;
    for (i = 0; i < TriangleNum; i++) {
        Triangles[i].gy[0] = vertex[Triangles[i].vertex[0]].y;
        Triangles[i].gy[1] = vertex[Triangles[i].vertex[1]].y;
        Triangles[i].gy[2] = vertex[Triangles[i].vertex[2]].y;
        Triangles[i].gx[0] = vertex[Triangles[i].vertex[0]].x;
        Triangles[i].gx[1] = vertex[Triangles[i].vertex[1]].x;
        Triangles[i].gx[2] = vertex[Triangles[i].vertex[2]].x;
    }
    UpdateVertexArray();
}


void Model::FlipTexCoords()
{
    static int i;
    for (i = 0; i < TriangleNum; i++) {
        Triangles[i].gy[0] = -Triangles[i].gy[0];
        Triangles[i].gy[1] = -Triangles[i].gy[1];
        Triangles[i].gy[2] = -Triangles[i].gy[2];
    }
    UpdateVertexArray();
}

void Model::ScaleTexCoords(float howmuch)
{
    static int i;
    for (i = 0; i < TriangleNum; i++) {
        Triangles[i].gx[0] *= howmuch;
        Triangles[i].gx[1] *= howmuch;
        Triangles[i].gx[2] *= howmuch;
        Triangles[i].gy[0] *= howmuch;
        Triangles[i].gy[1] *= howmuch;
        Triangles[i].gy[2] *= howmuch;
    }
    UpdateVertexArray();
}

void Model::Scale(float xscale, float yscale, float zscale)
{
    static int i;
    for (i = 0; i < vertexNum; i++) {
        vertex[i].x *= xscale;
        vertex[i].y *= yscale;
        vertex[i].z *= zscale;
    }
    UpdateVertexArray();

    static int j;

    boundingsphereradius = 0;
    for (i = 0; i < vertexNum; i++) {
        for (j = 0; j < vertexNum; j++) {
            if (j != i && distsq(&vertex[j], &vertex[i]) / 2 > boundingsphereradius) {
                boundingsphereradius = distsq(&vertex[j], &vertex[i]) / 2;
                boundingspherecenter = (vertex[i] + vertex[j]) / 2;
            }
        }
    }
    boundingsphereradius = fast_sqrt(boundingsphereradius);
}

void Model::ScaleNormals(float xscale, float yscale, float zscale)
{
    if (type != normaltype && type != decalstype)
        return;
    static int i;
    for (i = 0; i < vertexNum; i++) {
        normals[i].x *= xscale;
        normals[i].y *= yscale;
        normals[i].z *= zscale;
    }
    for (i = 0; i < TriangleNum; i++) {
        facenormals[i].x *= xscale;
        facenormals[i].y *= yscale;
        facenormals[i].z *= zscale;
    }
    UpdateVertexArray();
}

void Model::Translate(float xtrans, float ytrans, float ztrans)
{
    static int i;
    for (i = 0; i < vertexNum; i++) {
        vertex[i].x += xtrans;
        vertex[i].y += ytrans;
        vertex[i].z += ztrans;
    }
    UpdateVertexArray();

    static int j;
    boundingsphereradius = 0;
    for (i = 0; i < vertexNum; i++) {
        for (j = 0; j < vertexNum; j++) {
            if (j != i && distsq(&vertex[j], &vertex[i]) / 2 > boundingsphereradius) {
                boundingsphereradius = distsq(&vertex[j], &vertex[i]) / 2;
                boundingspherecenter = (vertex[i] + vertex[j]) / 2;
            }
        }
    }
    boundingsphereradius = fast_sqrt(boundingsphereradius);
}

void Model::Rotate(float xang, float yang, float zang)
{
    static int i;
    for (i = 0; i < vertexNum; i++) {
        vertex[i] = DoRotation(vertex[i], xang, yang, zang);
    }
    UpdateVertexArray();

    static int j;
    boundingsphereradius = 0;
    for (i = 0; i < vertexNum; i++) {
        for (j = 0; j < vertexNum; j++) {
            if (j != i && distsq(&vertex[j], &vertex[i]) / 2 > boundingsphereradius) {
                boundingsphereradius = distsq(&vertex[j], &vertex[i]) / 2;
                boundingspherecenter = (vertex[i] + vertex[j]) / 2;
            }
        }
    }
    boundingsphereradius = fast_sqrt(boundingsphereradius);
}


void Model::CalculateNormals(bool facenormalise)
{
    Game::LoadingScreen();
    static int i;
    if (type != normaltype && type != decalstype)
        return;

    for (i = 0; i < vertexNum; i++) {
        normals[i].x = 0;
        normals[i].y = 0;
        normals[i].z = 0;
    }

    for (i = 0; i < TriangleNum; i++) {
        CrossProduct(vertex[Triangles[i].vertex[1]] - vertex[Triangles[i].vertex[0]], vertex[Triangles[i].vertex[2]] - vertex[Triangles[i].vertex[0]], &facenormals[i]);

        normals[Triangles[i].vertex[0]].x += facenormals[i].x;
        normals[Triangles[i].vertex[0]].y += facenormals[i].y;
        normals[Triangles[i].vertex[0]].z += facenormals[i].z;

        normals[Triangles[i].vertex[1]].x += facenormals[i].x;
        normals[Triangles[i].vertex[1]].y += facenormals[i].y;
        normals[Triangles[i].vertex[1]].z += facenormals[i].z;

        normals[Triangles[i].vertex[2]].x += facenormals[i].x;
        normals[Triangles[i].vertex[2]].y += facenormals[i].y;
        normals[Triangles[i].vertex[2]].z += facenormals[i].z;
        if (facenormalise)
            Normalise(&facenormals[i]);
    }
    for (i = 0; i < vertexNum; i++) {
        Normalise(&normals[i]);
        normals[i] *= -1;
    }
    UpdateVertexArrayNoTex();
}

void Model::drawimmediate()
{
    textureptr.bind();
    glBegin(GL_TRIANGLES);
    for (int i = 0; i < TriangleNum; i++) {
        glTexCoord2f(Triangles[i].gx[0], Triangles[i].gy[0]);
        if (color)
            glColor3f(normals[Triangles[i].vertex[0]].x, normals[Triangles[i].vertex[0]].y, normals[Triangles[i].vertex[0]].z);
        if (!color && !flat)
            glNormal3f(normals[Triangles[i].vertex[0]].x, normals[Triangles[i].vertex[0]].y, normals[Triangles[i].vertex[0]].z);
        if (!color && flat)
            glNormal3f(facenormals[i].x, facenormals[i].y, facenormals[i].y);
        glVertex3f(vertex[Triangles[i].vertex[0]].x, vertex[Triangles[i].vertex[0]].y, vertex[Triangles[i].vertex[0]].z);

        glTexCoord2f(Triangles[i].gx[1], Triangles[i].gy[1]);
        if (color)
            glColor3f(normals[Triangles[i].vertex[1]].x, normals[Triangles[i].vertex[1]].y, normals[Triangles[i].vertex[1]].z);
        if (!color && !flat)
            glNormal3f(normals[Triangles[i].vertex[1]].x, normals[Triangles[i].vertex[1]].y, normals[Triangles[i].vertex[1]].z);
        if (!color && flat)
            glNormal3f(facenormals[i].x, facenormals[i].y, facenormals[i].y);
        glVertex3f(vertex[Triangles[i].vertex[1]].x, vertex[Triangles[i].vertex[1]].y, vertex[Triangles[i].vertex[1]].z);

        glTexCoord2f(Triangles[i].gx[2], Triangles[i].gy[2]);
        if (color)
            glColor3f(normals[Triangles[i].vertex[2]].x, normals[Triangles[i].vertex[2]].y, normals[Triangles[i].vertex[2]].z);
        if (!color && !flat)
            glNormal3f(normals[Triangles[i].vertex[2]].x, normals[Triangles[i].vertex[2]].y, normals[Triangles[i].vertex[2]].z);
        if (!color && flat)
            glNormal3f(facenormals[i].x, facenormals[i].y, facenormals[i].y);
        glVertex3f(vertex[Triangles[i].vertex[2]].x, vertex[Triangles[i].vertex[2]].y, vertex[Triangles[i].vertex[2]].z);
    }
    glEnd();
}

void Model::draw()
{
    if (type != normaltype && type != decalstype)
        return;

    glEnableClientState(GL_NORMAL_ARRAY);
    glEnableClientState(GL_VERTEX_ARRAY);
    glEnableClientState(GL_TEXTURE_COORD_ARRAY);

    if (!color)
        glInterleavedArrays( GL_T2F_N3F_V3F, 8 * sizeof(GLfloat), &vArray[0]);
    if (color)
        glInterleavedArrays( GL_T2F_C3F_V3F, 8 * sizeof(GLfloat), &vArray[0]);
    textureptr.bind();

    glDrawArrays(GL_TRIANGLES, 0, TriangleNum * 3);

    if (!color)
        glDisableClientState(GL_NORMAL_ARRAY);
    if (color)
        glDisableClientState(GL_COLOR_ARRAY);
    glDisableClientState(GL_VERTEX_ARRAY);
    glDisableClientState(GL_TEXTURE_COORD_ARRAY);
}

//TODO: phase out in favor of Texture
void Model::drawdifftex(GLuint texture)
{
    glEnableClientState(GL_NORMAL_ARRAY);
    glEnableClientState(GL_VERTEX_ARRAY);
    glEnableClientState(GL_TEXTURE_COORD_ARRAY);
    if (!color)
        glInterleavedArrays( GL_T2F_N3F_V3F, 8 * sizeof(GLfloat), &vArray[0]);
    if (color)
        glInterleavedArrays( GL_T2F_C3F_V3F, 8 * sizeof(GLfloat), &vArray[0]);

    glBindTexture(GL_TEXTURE_2D, (unsigned long)texture);
    glTexParameterf( GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT );
    glTexParameterf( GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT );

    glDrawArrays(GL_TRIANGLES, 0, TriangleNum * 3);

    if (!color)
        glDisableClientState(GL_NORMAL_ARRAY);
    if (color)
        glDisableClientState(GL_COLOR_ARRAY);
    glDisableClientState(GL_VERTEX_ARRAY);
    glDisableClientState(GL_TEXTURE_COORD_ARRAY);
}

void Model::drawdifftex(Texture texture)
{
    glEnableClientState(GL_NORMAL_ARRAY);
    glEnableClientState(GL_VERTEX_ARRAY);
    glEnableClientState(GL_TEXTURE_COORD_ARRAY);
    if (!color)
        glInterleavedArrays( GL_T2F_N3F_V3F, 8 * sizeof(GLfloat), &vArray[0]);
    if (color)
        glInterleavedArrays( GL_T2F_C3F_V3F, 8 * sizeof(GLfloat), &vArray[0]);

    texture.bind();
    glTexParameterf( GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT );
    glTexParameterf( GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT );

    glDrawArrays(GL_TRIANGLES, 0, TriangleNum * 3);

    if (!color)
        glDisableClientState(GL_NORMAL_ARRAY);
    if (color)
        glDisableClientState(GL_COLOR_ARRAY);
    glDisableClientState(GL_VERTEX_ARRAY);
    glDisableClientState(GL_TEXTURE_COORD_ARRAY);
}

void Model::drawdecals(Texture shadowtexture, Texture bloodtexture, Texture bloodtexture2, Texture breaktexture)
{
    if (decalstoggle) {
        if (type != decalstype)
            return;
        static int lasttype;
        static bool blend;

        blend = 1;

        lasttype = -1;
        glEnable(GL_BLEND);
        glDisable(GL_LIGHTING);
        glDisable(GL_CULL_FACE);
        glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
        glDepthMask(0);
        for (unsigned int i = 0; i < decals.size(); i++) {
            if (decals[i].type == blooddecalfast && decals[i].alivetime < 2)
                decals[i].alivetime = 2;

            if (decals[i].type != lasttype) {
                if (decals[i].type == shadowdecal) {
                    shadowtexture.bind();
                    if (!blend) {
                        blend = 1;
                        glAlphaFunc(GL_GREATER, 0.0001);
                        glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
                    }
                }
                if (decals[i].type == breakdecal) {
                    breaktexture.bind();
                    if (!blend) {
                        blend = 1;
                        glAlphaFunc(GL_GREATER, 0.0001);
                        glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
                    }
                }
                if (decals[i].type == blooddecal || decals[i].type == blooddecalslow) {
                    bloodtexture.bind();
                    if (blend) {
                        blend = 0;
                        glAlphaFunc(GL_GREATER, 0.15);
                        glBlendFunc(GL_ONE, GL_ZERO);
                    }
                }
                if (decals[i].type == blooddecalfast) {
                    bloodtexture2.bind();
                    if (blend) {
                        blend = 0;
                        glAlphaFunc(GL_GREATER, 0.15);
                        glBlendFunc(GL_ONE, GL_ZERO);
                    }
                }
            }
            if (decals[i].type == shadowdecal) {
                glColor4f(1, 1, 1, decals[i].opacity);
            }
            if (decals[i].type == breakdecal) {
                glColor4f(1, 1, 1, decals[i].opacity);
                if (decals[i].alivetime > 58)
                    glColor4f(1, 1, 1, decals[i].opacity * (60 - decals[i].alivetime) / 2);
            }
            if ((decals[i].type == blooddecal || decals[i].type == blooddecalfast || decals[i].type == blooddecalslow)) {
                glColor4f(1, 1, 1, decals[i].opacity);
                if (decals[i].alivetime < 4)
                    glColor4f(1, 1, 1, decals[i].opacity*decals[i].alivetime*.25);
                if (decals[i].alivetime > 58)
                    glColor4f(1, 1, 1, decals[i].opacity * (60 - decals[i].alivetime) / 2);
            }
            lasttype = decals[i].type;
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP);

            glMatrixMode(GL_MODELVIEW);
            glPushMatrix();
            glBegin(GL_TRIANGLES);
            for (int j = 0; j < 3; j++) {
                glTexCoord2f(decals[i].texcoords[j][0], decals[i].texcoords[j][1]);
                glVertex3f(decals[i].vertex[j].x, decals[i].vertex[j].y, decals[i].vertex[j].z);
            }
            glEnd();
            glPopMatrix();
        }
        for (int i = decals.size() - 1; i >= 0; i--) {
            decals[i].alivetime += multiplier;
            if (decals[i].type == blooddecalslow)
                decals[i].alivetime -= multiplier * 2 / 3;
            if (decals[i].type == blooddecalfast)
                decals[i].alivetime += multiplier * 4;
            if (decals[i].type == shadowdecal)
                DeleteDecal(i);
            if ((decals[i].type == blooddecal || decals[i].type == blooddecalfast || decals[i].type == blooddecalslow) && decals[i].alivetime >= 60)
                DeleteDecal(i);
        }
        glAlphaFunc(GL_GREATER, 0.0001);
        glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    }
}

void Model::DeleteDecal(int which)
{
    if (decalstoggle) {
        if (type != decalstype)
            return;
        decals.erase(decals.begin() + which);
    }
}

void Model::MakeDecal(decal_type atype, XYZ *where, float *size, float *opacity, float *rotation)
{
    if (decalstoggle) {
        if (type != decalstype)
            return;

        static XYZ rot;
        static float distance;

        if (*opacity > 0)
            if (distsq(where, &boundingspherecenter) < (boundingsphereradius + *size) * (boundingsphereradius + *size))
                for (int i = 0; i < TriangleNum; i++) {
                    if (facenormals[i].y < -.1 && (vertex[Triangles[i].vertex[0]].y < where->y || vertex[Triangles[i].vertex[1]].y < where->y || vertex[Triangles[i].vertex[2]].y < where->y)) {
                        distance = abs(((facenormals[i].x * where->x) + (facenormals[i].y * where->y) + (facenormals[i].z * where->z) - ((facenormals[i].x * vertex[Triangles[i].vertex[0]].x) + (facenormals[i].y * vertex[Triangles[i].vertex[0]].y) + (facenormals[i].z * vertex[Triangles[i].vertex[0]].z))) / facenormals[i].y);

                        if (*opacity - distance / 10 > 0) {
                            Decal decal(*where, atype, *opacity - distance / 10, *rotation, *size, *this, i, 0);

                            if (!(decal.texcoords[0][0] < 0 && decal.texcoords[1][0] < 0 && decal.texcoords[2][0] < 0))
                                if (!(decal.texcoords[0][1] < 0 && decal.texcoords[1][1] < 0 && decal.texcoords[2][1] < 0))
                                    if (!(decal.texcoords[0][0] > 1 && decal.texcoords[1][0] > 1 && decal.texcoords[2][0] > 1))
                                        if (!(decal.texcoords[0][1] > 1 && decal.texcoords[1][1] > 1 && decal.texcoords[2][1] > 1)) {
                                            if (decal.rotation) {
                                                for (int j = 0; j < 3; j++) {
                                                    rot.y = 0;
                                                    rot.x = decal.texcoords[j][0] - .5;
                                                    rot.z = decal.texcoords[j][1] - .5;
                                                    rot = DoRotation(rot, 0, -decal.rotation, 0);
                                                    decal.texcoords[j][0] = rot.x + .5;
                                                    decal.texcoords[j][1] = rot.z + .5;
                                                }
                                            }
                                            if (decals.size() < max_model_decals - 1) {
                                                decals.push_back(decal);
                                            }
                                        }
                        }
                    }
                }
    }
}

void Model::MakeDecal(decal_type atype, XYZ where, float size, float opacity, float rotation)
{
    if (decalstoggle) {
        if (type != decalstype)
            return;

        static XYZ rot;
        static float distance;

        if (opacity > 0)
            if (distsq(&where, &boundingspherecenter) < (boundingsphereradius + size) * (boundingsphereradius + size))
                for (int i = 0; i < TriangleNum; i++) {
                    distance = abs(((facenormals[i].x * where.x) + (facenormals[i].y * where.y) + (facenormals[i].z * where.z) - ((facenormals[i].x * vertex[Triangles[i].vertex[0]].x) + (facenormals[i].y * vertex[Triangles[i].vertex[0]].y) + (facenormals[i].z * vertex[Triangles[i].vertex[0]].z))));
                    if (distance < .02 && abs(facenormals[i].y) > abs(facenormals[i].x) && abs(facenormals[i].y) > abs(facenormals[i].z)) {
                        if (opacity - distance / 10 > 0) {
                            Decal decal(where, atype, opacity - distance / 10, rotation, size, *this, i, 0);

                            if (!(decal.texcoords[0][0] < 0 && decal.texcoords[1][0] < 0 && decal.texcoords[2][0] < 0))
                                if (!(decal.texcoords[0][1] < 0 && decal.texcoords[1][1] < 0 && decal.texcoords[2][1] < 0))
                                    if (!(decal.texcoords[0][0] > 1 && decal.texcoords[1][0] > 1 && decal.texcoords[2][0] > 1))
                                        if (!(decal.texcoords[0][1] > 1 && decal.texcoords[1][1] > 1 && decal.texcoords[2][1] > 1)) {
                                            if (decal.rotation) {
                                                for (int j = 0; j < 3; j++) {
                                                    rot.y = 0;
                                                    rot.x = decal.texcoords[j][0] - .5;
                                                    rot.z = decal.texcoords[j][1] - .5;
                                                    rot = DoRotation(rot, 0, -decal.rotation, 0);
                                                    decal.texcoords[j][0] = rot.x + .5;
                                                    decal.texcoords[j][1] = rot.z + .5;
                                                }
                                            }
                                            if (decals.size() < max_model_decals - 1) {
                                                decals.push_back(decal);
                                            }
                                        }
                        }
                    } else if (distance < .02 && abs(facenormals[i].x) > abs(facenormals[i].y) && abs(facenormals[i].x) > abs(facenormals[i].z)) {
                        if (opacity - distance / 10 > 0) {
                            Decal decal(where, atype, opacity - distance / 10, rotation, size, *this, i, 1);

                            if (!(decal.texcoords[0][0] < 0 && decal.texcoords[1][0] < 0 && decal.texcoords[2][0] < 0))
                                if (!(decal.texcoords[0][1] < 0 && decal.texcoords[1][1] < 0 && decal.texcoords[2][1] < 0))
                                    if (!(decal.texcoords[0][0] > 1 && decal.texcoords[1][0] > 1 && decal.texcoords[2][0] > 1))
                                        if (!(decal.texcoords[0][1] > 1 && decal.texcoords[1][1] > 1 && decal.texcoords[2][1] > 1)) {
                                            if (decal.rotation) {
                                                for (int j = 0; j < 3; j++) {
                                                    rot.y = 0;
                                                    rot.x = decal.texcoords[j][0] - .5;
                                                    rot.z = decal.texcoords[j][1] - .5;
                                                    rot = DoRotation(rot, 0, -decal.rotation, 0);
                                                    decal.texcoords[j][0] = rot.x + .5;
                                                    decal.texcoords[j][1] = rot.z + .5;
                                                }
                                            }
                                            if (decals.size() < max_model_decals - 1) {
                                                decals.push_back(decal);
                                            }
                                        }
                        }
                    } else if (distance < .02 && abs(facenormals[i].z) > abs(facenormals[i].y) && abs(facenormals[i].z) > abs(facenormals[i].x)) {
                        if (opacity - distance / 10 > 0) {
                            Decal decal(where, atype, opacity - distance / 10, rotation, size, *this, i, 2);

                            if (!(decal.texcoords[0][0] < 0 && decal.texcoords[1][0] < 0 && decal.texcoords[2][0] < 0))
                                if (!(decal.texcoords[0][1] < 0 && decal.texcoords[1][1] < 0 && decal.texcoords[2][1] < 0))
                                    if (!(decal.texcoords[0][0] > 1 && decal.texcoords[1][0] > 1 && decal.texcoords[2][0] > 1))
                                        if (!(decal.texcoords[0][1] > 1 && decal.texcoords[1][1] > 1 && decal.texcoords[2][1] > 1)) {
                                            if (decal.rotation) {
                                                for (int j = 0; j < 3; j++) {
                                                    rot.y = 0;
                                                    rot.x = decal.texcoords[j][0] - .5;
                                                    rot.z = decal.texcoords[j][1] - .5;
                                                    rot = DoRotation(rot, 0, -decal.rotation, 0);
                                                    decal.texcoords[j][0] = rot.x + .5;
                                                    decal.texcoords[j][1] = rot.z + .5;
                                                }
                                            }
                                            if (decals.size() < max_model_decals - 1) {
                                                decals.push_back(decal);
                                            }
                                        }
                        }
                    }
                }
    }
}

void Model::deleteDeadDecals()
{
    for (int i = decals.size() - 1; i >= 0; i--) {
        if ((decals[i].type == blooddecal || decals[i].type == blooddecalslow) && decals[i].alivetime < 2) {
            DeleteDecal(i);
        }
    }
}

Model::~Model()
{
    deallocate();
}

void Model::deallocate()
{
    if (owner)
        free(owner);
    owner = 0;

    if (possible)
        free(possible);
    possible = 0;

    if (vertex)
        free(vertex);
    vertex = 0;

    if (normals)
        free(normals);
    normals = 0;

    if (facenormals)
        free(facenormals);
    facenormals = 0;

    if (Triangles)
        free(Triangles);
    Triangles = 0;

    if (vArray)
        free(vArray);
    vArray = 0;
}

Model::Model()
  : vertexNum(0), TriangleNum(0),
    hastexture(0),
    type(0), oldtype(0),
    possible(0),
    owner(0),
    vertex(0),
    normals(0),
    facenormals(0),
    Triangles(0),
    vArray(0)
{
    memset(&modelTexture, 0, sizeof(modelTexture));
    numpossible = 0;
    color = 0;

    boundingspherecenter = 0;
    boundingsphereradius = 0;

    flat = 0;

    type = nothing;
}
