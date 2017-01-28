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

#include "Animation/Muscle.hpp"

#include "Utils/binio.h"

extern float multiplier;
extern bool freeze;

Muscle::Muscle()
    : length(0)
    , targetlength(0)
    , parent1(0)
    , parent2(0)
    , maxlength(0)
    , minlength(0)
    , type(boneconnect)
    , visible(false)
    , rotate1(0)
    , rotate2(0)
    , rotate3(0)
    , lastrotate1(0)
    , lastrotate2(0)
    , lastrotate3(0)
    , oldrotate1(0)
    , oldrotate2(0)
    , oldrotate3(0)
    , newrotate1(0)
    , newrotate2(0)
    , newrotate3(0)
    ,

    strength(0)
{
}

void Muscle::load(FILE* tfile, int vertexNum, std::vector<Joint>& joints)
{
    int numvertices, vertice, parentID;

    // read info
    funpackf(tfile, "Bf Bf Bf Bf Bf Bi Bi", &length, &targetlength, &minlength, &maxlength, &strength, &type, &numvertices);

    // read vertices
    for (int j = 0; j < numvertices; j++) {
        funpackf(tfile, "Bi", &vertice);
        if (vertice < vertexNum) {
            vertices.push_back(vertice);
        }
    }

    // read more info
    funpackf(tfile, "Bb Bi", &visible, &parentID);
    parent1 = &joints[parentID];
    funpackf(tfile, "Bi", &parentID);
    parent2 = &joints[parentID];
}

void Muscle::loadVerticesLow(FILE* tfile, int vertexNum)
{
    int numvertices, vertice;

    // read numvertices
    funpackf(tfile, "Bi", &numvertices);

    // read vertices
    for (int j = 0; j < numvertices; j++) {
        funpackf(tfile, "Bi", &vertice);
        if (vertice < vertexNum) {
            verticeslow.push_back(vertice);
        }
    }
}

void Muscle::loadVerticesClothes(FILE* tfile, int vertexNum)
{
    int numvertices, vertice;

    // read numvertices
    funpackf(tfile, "Bi", &numvertices);

    // read vertices
    for (int j = 0; j < numvertices; j++) {
        funpackf(tfile, "Bi", &vertice);
        if (vertice < vertexNum) {
            verticesclothes.push_back(vertice);
        }
    }
}

/* EFFECT
 * sets strength, length,
 *      parent1->position, parent2->position,
 *      parent1->velocity, parent2->velocity
 * used for ragdolls?
 *
 * USES:
 * Skeleton::DoConstraints
 */
void Muscle::DoConstraint(bool spinny)
{
    // FIXME: relaxlength shouldn't be static, but may not always be set
    // so I don't want to change the existing behavior even though it's probably a bug
    static float relaxlength;

    float oldlength = length;

    if (type != boneconnect) {
        relaxlength = findDistance(&parent1->position, &parent2->position);
    }

    if (type == boneconnect) {
        strength = 1;
    }
    if (type == constraint) {
        strength = 0;
    }

    // clamp strength
    if (strength < 0) {
        strength = 0;
    }
    if (strength > 1) {
        strength = 1;
    }

    length -= (length - relaxlength) * (1 - strength) * multiplier * 10000;
    length -= (length - targetlength) * strength * multiplier * 10000;
    if (strength == 0) {
        length = relaxlength;
    }

    if ((relaxlength - length > 0 && relaxlength - oldlength < 0) || (relaxlength - length < 0 && relaxlength - oldlength > 0)) {
        length = relaxlength;
    }

    // clamp length
    if (length < minlength) {
        length = minlength;
    }
    if (length > maxlength) {
        length = maxlength;
    }

    if (length == relaxlength) {
        return;
    }

    // relax muscle?

    //Find midpoint
    XYZ midp = (parent1->position * parent1->mass + parent2->position * parent2->mass) / (parent1->mass + parent2->mass);

    //Find vector from midpoint to second vector
    XYZ vel = parent2->position - midp;

    //Change to unit vector
    Normalise(&vel);

    //Apply velocity change
    XYZ newpoint1 = midp - vel * length * (parent2->mass / (parent1->mass + parent2->mass));
    XYZ newpoint2 = midp + vel * length * (parent1->mass / (parent1->mass + parent2->mass));
    if (!freeze && spinny) {
        parent1->velocity = parent1->velocity + (newpoint1 - parent1->position) / multiplier / 4;
        parent2->velocity = parent2->velocity + (newpoint2 - parent2->position) / multiplier / 4;
    } else {
        parent1->velocity = parent1->velocity + (newpoint1 - parent1->position);
        parent2->velocity = parent2->velocity + (newpoint2 - parent2->position);
    }

    //Move child point to within certain distance of parent point
    parent1->position = newpoint1;
    parent2->position = newpoint2;
}
