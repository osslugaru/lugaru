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

#ifndef _JOINT_HPP_
#define _JOINT_HPP_

#include "Math/XYZ.hpp"

#include <vector>

enum bodypart
{
    head,
    neck,
    leftshoulder,
    leftelbow,
    leftwrist,
    lefthand,
    rightshoulder,
    rightelbow,
    rightwrist,
    righthand,
    abdomen,
    lefthip,
    righthip,
    groin,
    leftknee,
    leftankle,
    leftfoot,
    rightknee,
    rightankle,
    rightfoot
};

class Joint
{
public:
    XYZ position;
    XYZ oldposition;
    XYZ realoldposition;
    XYZ velocity;
    XYZ oldvelocity;
    XYZ startpos;
    float blurred;
    float length;
    float mass;
    bool lower;
    bool hasparent;
    bool locked;
    int modelnum;
    bool visible;
    Joint* parent;
    bool sametwist;
    bodypart label;
    int hasgun;
    float delay;
    XYZ velchange;

    Joint();
    void load(FILE* tfile, std::vector<Joint>& joints);
};

#endif
