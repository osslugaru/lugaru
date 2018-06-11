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

#ifndef _QUATERNIONS_HPP_
#define _QUATERNIONS_HPP_

#include "Graphic/gamegl.hpp"

#include <math.h>
#include <json/value.h>

class XYZ
{
public:
    float x;
    float y;
    float z;
    XYZ()
        : x(0.0f)
        , y(0.0f)
        , z(0.0f)
    {
    }
    XYZ(Json::Value v)
        : x(v[0].asFloat())
        , y(v[1].asFloat())
        , z(v[2].asFloat())
    {
    }
    inline XYZ operator+(XYZ add);
    inline XYZ operator-(XYZ add);
    inline XYZ operator*(float add);
    inline XYZ operator*(XYZ add);
    inline XYZ operator/(float add);
    inline void operator+=(XYZ add);
    inline void operator-=(XYZ add);
    inline void operator*=(float add);
    inline void operator*=(XYZ add);
    inline void operator/=(float add);
    inline void operator=(float add);
    inline bool operator==(XYZ add);

    operator Json::Value();
};

inline void CrossProduct(XYZ* P, XYZ* Q, XYZ* V);
inline void CrossProduct(XYZ P, XYZ Q, XYZ* V);
inline void Normalise(XYZ* vectory);
inline float normaldotproduct(XYZ point1, XYZ point2);
inline float fast_sqrt(float arg);
bool PointInTriangle(XYZ* p, XYZ normal, XYZ* p1, XYZ* p2, XYZ* p3);
bool LineFacet(XYZ p1, XYZ p2, XYZ pa, XYZ pb, XYZ pc, XYZ* p);
float LineFacetd(XYZ p1, XYZ p2, XYZ pa, XYZ pb, XYZ pc, XYZ* p);
float LineFacetd(XYZ p1, XYZ p2, XYZ pa, XYZ pb, XYZ pc, XYZ n, XYZ* p);
float LineFacetd(XYZ* p1, XYZ* p2, XYZ* pa, XYZ* pb, XYZ* pc, XYZ* n, XYZ* p);
float LineFacetd(XYZ* p1, XYZ* p2, XYZ* pa, XYZ* pb, XYZ* pc, XYZ* p);
inline void ReflectVector(XYZ* vel, const XYZ* n);
inline void ReflectVector(XYZ* vel, const XYZ& n);
inline XYZ DoRotation(XYZ thePoint, float xang, float yang, float zang);
inline XYZ DoRotationRadian(XYZ thePoint, float xang, float yang, float zang);
inline float findDistance(XYZ* point1, XYZ* point2);
inline float findLength(XYZ* point1);
inline float findLengthfast(XYZ* point1);
inline float distsq(XYZ* point1, XYZ* point2);
inline float distsq(XYZ point1, XYZ point2);
inline float distsqflat(XYZ* point1, XYZ* point2);
inline float dotproduct(const XYZ* point1, const XYZ* point2);
bool sphere_line_intersection(
    float x1, float y1, float z1,
    float x2, float y2, float z2,
    float x3, float y3, float z3, float r);
bool sphere_line_intersection(
    XYZ* p1, XYZ* p2, XYZ* p3, float* r);
inline bool DistancePointLine(XYZ* Point, XYZ* LineStart, XYZ* LineEnd, float* Distance, XYZ* Intersection);

inline void Normalise(XYZ* vectory)
{
    static float d;
    d = fast_sqrt(vectory->x * vectory->x + vectory->y * vectory->y + vectory->z * vectory->z);
    if (d == 0) {
        return;
    }
    vectory->x /= d;
    vectory->y /= d;
    vectory->z /= d;
}

inline XYZ XYZ::operator+(XYZ add)
{
    static XYZ ne;
    ne = add;
    ne.x += x;
    ne.y += y;
    ne.z += z;
    return ne;
}

inline XYZ XYZ::operator-(XYZ add)
{
    static XYZ ne;
    ne = add;
    ne.x = x - ne.x;
    ne.y = y - ne.y;
    ne.z = z - ne.z;
    return ne;
}

inline XYZ XYZ::operator*(float add)
{
    static XYZ ne;
    ne.x = x * add;
    ne.y = y * add;
    ne.z = z * add;
    return ne;
}

inline XYZ XYZ::operator*(XYZ add)
{
    static XYZ ne;
    ne.x = x * add.x;
    ne.y = y * add.y;
    ne.z = z * add.z;
    return ne;
}

inline XYZ XYZ::operator/(float add)
{
    static XYZ ne;
    ne.x = x / add;
    ne.y = y / add;
    ne.z = z / add;
    return ne;
}

inline void XYZ::operator+=(XYZ add)
{
    x += add.x;
    y += add.y;
    z += add.z;
}

inline void XYZ::operator-=(XYZ add)
{
    x = x - add.x;
    y = y - add.y;
    z = z - add.z;
}

inline void XYZ::operator*=(float add)
{
    x = x * add;
    y = y * add;
    z = z * add;
}

inline void XYZ::operator*=(XYZ add)
{
    x = x * add.x;
    y = y * add.y;
    z = z * add.z;
}

inline void XYZ::operator/=(float add)
{
    x = x / add;
    y = y / add;
    z = z / add;
}

inline void XYZ::operator=(float add)
{
    x = add;
    y = add;
    z = add;
}

inline bool XYZ::operator==(XYZ add)
{
    if (x == add.x && y == add.y && z == add.z)
        return 1;
    return 0;
}

inline void CrossProduct(XYZ* P, XYZ* Q, XYZ* V)
{
    V->x = P->y * Q->z - P->z * Q->y;
    V->y = P->z * Q->x - P->x * Q->z;
    V->z = P->x * Q->y - P->y * Q->x;
}

inline void CrossProduct(XYZ P, XYZ Q, XYZ* V)
{
    V->x = P.y * Q.z - P.z * Q.y;
    V->y = P.z * Q.x - P.x * Q.z;
    V->z = P.x * Q.y - P.y * Q.x;
}

inline float fast_sqrt(float arg)
{
    return sqrtf(arg);
}

inline float normaldotproduct(XYZ point1, XYZ point2)
{
    static GLfloat returnvalue;
    Normalise(&point1);
    Normalise(&point2);
    returnvalue = (point1.x * point2.x + point1.y * point2.y + point1.z * point2.z);
    return returnvalue;
}

inline void ReflectVector(XYZ* vel, const XYZ* n)
{
    ReflectVector(vel, *n);
}

inline void ReflectVector(XYZ* vel, const XYZ& n)
{
    static XYZ vn;
    static XYZ vt;
    static float dotprod;

    dotprod = dotproduct(&n, vel);
    vn.x = n.x * dotprod;
    vn.y = n.y * dotprod;
    vn.z = n.z * dotprod;

    vt.x = vel->x - vn.x;
    vt.y = vel->y - vn.y;
    vt.z = vel->z - vn.z;

    vel->x = vt.x - vn.x;
    vel->y = vt.y - vn.y;
    vel->z = vt.z - vn.z;
}

inline float dotproduct(const XYZ* point1, const XYZ* point2)
{
    static GLfloat returnvalue;
    returnvalue = (point1->x * point2->x + point1->y * point2->y + point1->z * point2->z);
    return returnvalue;
}

inline float findDistance(XYZ* point1, XYZ* point2)
{
    return (fast_sqrt((point1->x - point2->x) * (point1->x - point2->x) + (point1->y - point2->y) * (point1->y - point2->y) + (point1->z - point2->z) * (point1->z - point2->z)));
}

inline float findLength(XYZ* point1)
{
    return (fast_sqrt((point1->x) * (point1->x) + (point1->y) * (point1->y) + (point1->z) * (point1->z)));
}

inline float findLengthfast(XYZ* point1)
{
    return ((point1->x) * (point1->x) + (point1->y) * (point1->y) + (point1->z) * (point1->z));
}

inline float distsq(XYZ* point1, XYZ* point2)
{
    return ((point1->x - point2->x) * (point1->x - point2->x) + (point1->y - point2->y) * (point1->y - point2->y) + (point1->z - point2->z) * (point1->z - point2->z));
}

inline float distsq(XYZ point1, XYZ point2)
{
    return ((point1.x - point2.x) * (point1.x - point2.x) + (point1.y - point2.y) * (point1.y - point2.y) + (point1.z - point2.z) * (point1.z - point2.z));
}

inline float distsqflat(XYZ* point1, XYZ* point2)
{
    return ((point1->x - point2->x) * (point1->x - point2->x) + (point1->z - point2->z) * (point1->z - point2->z));
}

inline XYZ DoRotation(XYZ thePoint, float xang, float yang, float zang)
{
    static XYZ newpoint;
    if (xang) {
        xang *= 6.283185f;
        xang /= 360;
    }
    if (yang) {
        yang *= 6.283185f;
        yang /= 360;
    }
    if (zang) {
        zang *= 6.283185f;
        zang /= 360;
    }

    if (yang) {
        newpoint.z = thePoint.z * cosf(yang) - thePoint.x * sinf(yang);
        newpoint.x = thePoint.z * sinf(yang) + thePoint.x * cosf(yang);
        thePoint.z = newpoint.z;
        thePoint.x = newpoint.x;
    }

    if (zang) {
        newpoint.x = thePoint.x * cosf(zang) - thePoint.y * sinf(zang);
        newpoint.y = thePoint.y * cosf(zang) + thePoint.x * sinf(zang);
        thePoint.x = newpoint.x;
        thePoint.y = newpoint.y;
    }

    if (xang) {
        newpoint.y = thePoint.y * cosf(xang) - thePoint.z * sinf(xang);
        newpoint.z = thePoint.y * sinf(xang) + thePoint.z * cosf(xang);
        thePoint.z = newpoint.z;
        thePoint.y = newpoint.y;
    }

    return thePoint;
}

inline float square(float f)
{
    return (f * f);
}

inline bool sphere_line_intersection(
    float x1, float y1, float z1,
    float x2, float y2, float z2,
    float x3, float y3, float z3, float r)
{

    // x1,y1,z1  P1 coordinates (point of line)
    // x2,y2,z2  P2 coordinates (point of line)
    // x3,y3,z3, r  P3 coordinates and radius (sphere)
    // x,y,z   intersection coordinates
    //
    // This function returns a pointer array which first index indicates
    // the number of intersection point, followed by coordinate pairs.

    //~ static float x , y , z;
    static float a, b, c, /*mu,*/ i;

    if (x1 > x3 + r && x2 > x3 + r)
        return (0);
    if (x1 < x3 - r && x2 < x3 - r)
        return (0);
    if (y1 > y3 + r && y2 > y3 + r)
        return (0);
    if (y1 < y3 - r && y2 < y3 - r)
        return (0);
    if (z1 > z3 + r && z2 > z3 + r)
        return (0);
    if (z1 < z3 - r && z2 < z3 - r)
        return (0);
    a = square(x2 - x1) + square(y2 - y1) + square(z2 - z1);
    b = 2 * ((x2 - x1) * (x1 - x3) + (y2 - y1) * (y1 - y3) + (z2 - z1) * (z1 - z3));
    c = square(x3) + square(y3) +
        square(z3) + square(x1) +
        square(y1) + square(z1) -
        2 * (x3 * x1 + y3 * y1 + z3 * z1) - square(r);
    i = b * b - 4 * a * c;

    if (i < 0.0) {
        // no intersection
        return (0);
    }
    return (1);
}

inline bool sphere_line_intersection(
    XYZ* p1, XYZ* p2, XYZ* p3, float* r)
{

    // x1,p1->y,p1->z  P1 coordinates (point of line)
    // p2->x,p2->y,p2->z  P2 coordinates (point of line)
    // p3->x,p3->y,p3->z, r  P3 coordinates and radius (sphere)
    // x,y,z   intersection coordinates
    //
    // This function returns a pointer array which first index indicates
    // the number of intersection point, followed by coordinate pairs.

    //~ static float x , y , z;
    static float a, b, c, /*mu,*/ i;

    if (p1->x > p3->x + *r && p2->x > p3->x + *r)
        return (0);
    if (p1->x < p3->x - *r && p2->x < p3->x - *r)
        return (0);
    if (p1->y > p3->y + *r && p2->y > p3->y + *r)
        return (0);
    if (p1->y < p3->y - *r && p2->y < p3->y - *r)
        return (0);
    if (p1->z > p3->z + *r && p2->z > p3->z + *r)
        return (0);
    if (p1->z < p3->z - *r && p2->z < p3->z - *r)
        return (0);
    a = square(p2->x - p1->x) + square(p2->y - p1->y) + square(p2->z - p1->z);
    b = 2 * ((p2->x - p1->x) * (p1->x - p3->x) + (p2->y - p1->y) * (p1->y - p3->y) + (p2->z - p1->z) * (p1->z - p3->z));
    c = square(p3->x) + square(p3->y) +
        square(p3->z) + square(p1->x) +
        square(p1->y) + square(p1->z) -
        2 * (p3->x * p1->x + p3->y * p1->y + p3->z * p1->z) - square(*r);
    i = b * b - 4 * a * c;

    if (i < 0.0) {
        // no intersection
        return (0);
    }
    return (1);
}

inline XYZ DoRotationRadian(XYZ thePoint, float xang, float yang, float zang)
{
    static XYZ newpoint;
    static XYZ oldpoint;

    oldpoint = thePoint;

    if (yang != 0) {
        newpoint.z = oldpoint.z * cosf(yang) - oldpoint.x * sinf(yang);
        newpoint.x = oldpoint.z * sinf(yang) + oldpoint.x * cosf(yang);
        oldpoint.z = newpoint.z;
        oldpoint.x = newpoint.x;
    }

    if (zang != 0) {
        newpoint.x = oldpoint.x * cosf(zang) - oldpoint.y * sinf(zang);
        newpoint.y = oldpoint.y * cosf(zang) + oldpoint.x * sinf(zang);
        oldpoint.x = newpoint.x;
        oldpoint.y = newpoint.y;
    }

    if (xang != 0) {
        newpoint.y = oldpoint.y * cosf(xang) - oldpoint.z * sinf(xang);
        newpoint.z = oldpoint.y * sinf(xang) + oldpoint.z * cosf(xang);
        oldpoint.z = newpoint.z;
        oldpoint.y = newpoint.y;
    }

    return oldpoint;
}

inline bool DistancePointLine(XYZ* Point, XYZ* LineStart, XYZ* LineEnd, float* Distance, XYZ* Intersection)
{
    float LineMag;
    float U;

    LineMag = findDistance(LineEnd, LineStart);

    U = (((Point->x - LineStart->x) * (LineEnd->x - LineStart->x)) +
         ((Point->y - LineStart->y) * (LineEnd->y - LineStart->y)) +
         ((Point->z - LineStart->z) * (LineEnd->z - LineStart->z))) /
        (LineMag * LineMag);

    if (U < 0.0f || U > 1.0f)
        return 0; // closest point does not fall within the line segment

    Intersection->x = LineStart->x + U * (LineEnd->x - LineStart->x);
    Intersection->y = LineStart->y + U * (LineEnd->y - LineStart->y);
    Intersection->z = LineStart->z + U * (LineEnd->z - LineStart->z);

    *Distance = findDistance(Point, Intersection);

    return 1;
}

#endif
