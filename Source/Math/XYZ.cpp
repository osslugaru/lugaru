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

#include "Math/XYZ.hpp"

bool PointInTriangle(XYZ* p, XYZ normal, XYZ* p1, XYZ* p2, XYZ* p3)
{
    static float u0, u1, u2;
    static float v0, v1, v2;
    static float a, b;
    static float max;
    static int i, j;
    static bool bInter = 0;
    static float pointv[3];
    static float p1v[3];
    static float p2v[3];
    static float p3v[3];
    static float normalv[3];

    bInter = 0;

    pointv[0] = p->x;
    pointv[1] = p->y;
    pointv[2] = p->z;

    p1v[0] = p1->x;
    p1v[1] = p1->y;
    p1v[2] = p1->z;

    p2v[0] = p2->x;
    p2v[1] = p2->y;
    p2v[2] = p2->z;

    p3v[0] = p3->x;
    p3v[1] = p3->y;
    p3v[2] = p3->z;

    normalv[0] = normal.x;
    normalv[1] = normal.y;
    normalv[2] = normal.z;

#define ABS(X) (((X) < 0.f) ? -(X) : (X))
#define MAX(A, B) (((A) < (B)) ? (B) : (A))
    max = MAX(MAX(ABS(normalv[0]), ABS(normalv[1])), ABS(normalv[2]));
#undef MAX
    if (max == ABS(normalv[0])) {
        i = 1; // y, z
        j = 2;
    }
    if (max == ABS(normalv[1])) {
        i = 0; // x, z
        j = 2;
    }
    if (max == ABS(normalv[2])) {
        i = 0; // x, y
        j = 1;
    }
#undef ABS

    u0 = pointv[i] - p1v[i];
    v0 = pointv[j] - p1v[j];
    u1 = p2v[i] - p1v[i];
    v1 = p2v[j] - p1v[j];
    u2 = p3v[i] - p1v[i];
    v2 = p3v[j] - p1v[j];

    if (u1 > -1.0e-05f && u1 < 1.0e-05f) { // == 0.0f)
        b = u0 / u2;
        if (0.0f <= b && b <= 1.0f) {
            a = (v0 - b * v2) / v1;
            if ((a >= 0.0f) && ((a + b) <= 1.0f)) {
                bInter = 1;
            }
        }
    } else {
        b = (v0 * u1 - u0 * v1) / (v2 * u1 - u2 * v1);
        if (0.0f <= b && b <= 1.0f) {
            a = (u0 - b * u2) / u1;
            if ((a >= 0.0f) && ((a + b) <= 1.0f)) {
                bInter = 1;
            }
        }
    }

    return bInter;
}

bool LineFacet(XYZ p1, XYZ p2, XYZ pa, XYZ pb, XYZ pc, XYZ* p)
{
    static float d;
    static float denom, mu;
    static XYZ n;

    //Calculate the parameters for the plane
    n.x = (pb.y - pa.y) * (pc.z - pa.z) - (pb.z - pa.z) * (pc.y - pa.y);
    n.y = (pb.z - pa.z) * (pc.x - pa.x) - (pb.x - pa.x) * (pc.z - pa.z);
    n.z = (pb.x - pa.x) * (pc.y - pa.y) - (pb.y - pa.y) * (pc.x - pa.x);
    Normalise(&n);
    d = -n.x * pa.x - n.y * pa.y - n.z * pa.z;

    //Calculate the position on the line that intersects the plane
    denom = n.x * (p2.x - p1.x) + n.y * (p2.y - p1.y) + n.z * (p2.z - p1.z);
    if (fabs(denom) < 0.0000001) { // Line and plane don't intersect
        return 0;
    }
    mu = -(d + n.x * p1.x + n.y * p1.y + n.z * p1.z) / denom;
    p->x = p1.x + mu * (p2.x - p1.x);
    p->y = p1.y + mu * (p2.y - p1.y);
    p->z = p1.z + mu * (p2.z - p1.z);
    if (mu < 0 || mu > 1) { // Intersection not along line segment
        return 0;
    }

    if (!PointInTriangle(p, n, &pa, &pb, &pc)) {
        return 0;
    }

    return 1;
}

float LineFacetd(XYZ p1, XYZ p2, XYZ pa, XYZ pb, XYZ pc, XYZ* p)
{
    static float d;
    static float denom, mu;
    static XYZ n;

    //Calculate the parameters for the plane
    n.x = (pb.y - pa.y) * (pc.z - pa.z) - (pb.z - pa.z) * (pc.y - pa.y);
    n.y = (pb.z - pa.z) * (pc.x - pa.x) - (pb.x - pa.x) * (pc.z - pa.z);
    n.z = (pb.x - pa.x) * (pc.y - pa.y) - (pb.y - pa.y) * (pc.x - pa.x);
    Normalise(&n);
    d = -n.x * pa.x - n.y * pa.y - n.z * pa.z;

    //Calculate the position on the line that intersects the plane
    denom = n.x * (p2.x - p1.x) + n.y * (p2.y - p1.y) + n.z * (p2.z - p1.z);
    if (fabs(denom) < 0.0000001) { // Line and plane don't intersect
        return 0;
    }
    mu = -(d + n.x * p1.x + n.y * p1.y + n.z * p1.z) / denom;
    p->x = p1.x + mu * (p2.x - p1.x);
    p->y = p1.y + mu * (p2.y - p1.y);
    p->z = p1.z + mu * (p2.z - p1.z);
    if (mu < 0 || mu > 1) { // Intersection not along line segment
        return 0;
    }

    if (!PointInTriangle(p, n, &pa, &pb, &pc)) {
        return 0;
    }

    return 1;
}

float LineFacetd(XYZ p1, XYZ p2, XYZ pa, XYZ pb, XYZ pc, XYZ n, XYZ* p)
{
    static float d;
    static float denom, mu;

    //Calculate the parameters for the plane
    d = -n.x * pa.x - n.y * pa.y - n.z * pa.z;

    //Calculate the position on the line that intersects the plane
    denom = n.x * (p2.x - p1.x) + n.y * (p2.y - p1.y) + n.z * (p2.z - p1.z);
    if (fabs(denom) < 0.0000001) { // Line and plane don't intersect
        return 0;
    }
    mu = -(d + n.x * p1.x + n.y * p1.y + n.z * p1.z) / denom;
    p->x = p1.x + mu * (p2.x - p1.x);
    p->y = p1.y + mu * (p2.y - p1.y);
    p->z = p1.z + mu * (p2.z - p1.z);
    if (mu < 0 || mu > 1) { // Intersection not along line segment
        return 0;
    }

    if (!PointInTriangle(p, n, &pa, &pb, &pc)) {
        return 0;
    }
    return 1;
}

float LineFacetd(XYZ* p1, XYZ* p2, XYZ* pa, XYZ* pb, XYZ* pc, XYZ* p)
{
    static float d;
    static float denom, mu;
    static XYZ n;

    //Calculate the parameters for the plane
    n.x = (pb->y - pa->y) * (pc->z - pa->z) - (pb->z - pa->z) * (pc->y - pa->y);
    n.y = (pb->z - pa->z) * (pc->x - pa->x) - (pb->x - pa->x) * (pc->z - pa->z);
    n.z = (pb->x - pa->x) * (pc->y - pa->y) - (pb->y - pa->y) * (pc->x - pa->x);
    Normalise(&n);
    d = -n.x * pa->x - n.y * pa->y - n.z * pa->z;

    //Calculate the position on the line that intersects the plane
    denom = n.x * (p2->x - p1->x) + n.y * (p2->y - p1->y) + n.z * (p2->z - p1->z);
    if (fabs(denom) < 0.0000001) { // Line and plane don't intersect
        return 0;
    }
    mu = -(d + n.x * p1->x + n.y * p1->y + n.z * p1->z) / denom;
    p->x = p1->x + mu * (p2->x - p1->x);
    p->y = p1->y + mu * (p2->y - p1->y);
    p->z = p1->z + mu * (p2->z - p1->z);
    if (mu < 0 || mu > 1) { // Intersection not along line segment
        return 0;
    }

    if (!PointInTriangle(p, n, pa, pb, pc)) {
        return 0;
    }
    return 1;
}

float LineFacetd(XYZ* p1, XYZ* p2, XYZ* pa, XYZ* pb, XYZ* pc, XYZ* n, XYZ* p)
{
    static float d;
    static float denom, mu;

    //Calculate the parameters for the plane
    d = -n->x * pa->x - n->y * pa->y - n->z * pa->z;

    //Calculate the position on the line that intersects the plane
    denom = n->x * (p2->x - p1->x) + n->y * (p2->y - p1->y) + n->z * (p2->z - p1->z);
    if (fabs(denom) < 0.0000001) { // Line and plane don't intersect
        return 0;
    }
    mu = -(d + n->x * p1->x + n->y * p1->y + n->z * p1->z) / denom;
    p->x = p1->x + mu * (p2->x - p1->x);
    p->y = p1->y + mu * (p2->y - p1->y);
    p->z = p1->z + mu * (p2->z - p1->z);
    if (mu < 0 || mu > 1) { // Intersection not along line segment
        return 0;
    }

    if (!PointInTriangle(p, *n, pa, pb, pc)) {
        return 0;
    }
    return 1;
}

XYZ::operator Json::Value()
{
    Json::Value xyz;

    xyz[0] = x;
    xyz[1] = y;
    xyz[2] = z;

    return xyz;
}
