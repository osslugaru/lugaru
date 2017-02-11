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

#ifndef _MUSCLE_HPP_
#define _MUSCLE_HPP_

#include "Animation/Joint.hpp"

#include <vector>

enum muscle_type
{
    boneconnect,
    constraint,
    muscle
};

class Muscle
{
public:
    std::vector<int> vertices;
    std::vector<int> verticeslow;
    std::vector<int> verticesclothes;
    float length;
    float targetlength;
    Joint* parent1;
    Joint* parent2;
    float maxlength;
    float minlength;
    muscle_type type;
    bool visible;
    float rotate1, rotate2, rotate3;
    float lastrotate1, lastrotate2, lastrotate3;
    float oldrotate1, oldrotate2, oldrotate3;
    float newrotate1, newrotate2, newrotate3;

    float strength;

    Muscle();
    void load(FILE* tfile, int vertexNum, std::vector<Joint>& joints);
    void loadVerticesLow(FILE* tfile, int vertexNum);
    void loadVerticesClothes(FILE* tfile, int vertexNum);
    void DoConstraint(bool spinny);
};

#endif
