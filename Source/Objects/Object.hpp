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

#ifndef _OBJECTS_HPP_
#define _OBJECTS_HPP_

#include "Environment/Lights.hpp"
#include "Environment/Terrain.hpp"
#include "Graphic/gamegl.hpp"
#include "Graphic/Models.hpp"
#include "Graphic/Sprite.hpp"
#include "Graphic/Texture.hpp"
#include "Math/Frustum.hpp"
#include "Math/Quaternions.hpp"
#include "Utils/ImageIO.hpp"

#include <memory>
#include <vector>
//
// Model Structures
//

#define max_objects 300

#define boxtype 0
#define weirdtype 1
#define spiketype 2
#define treetrunktype 3
#define treeleavestype 4
#define bushtype 5
#define rocktype 6
#define walltype 7
#define chimneytype 8
#define platformtype 9
#define tunneltype 11
#define cooltype 12
#define firetype 13


class Object
{
public:
    static std::vector<std::unique_ptr<Object>> objects;
    static XYZ center;
    static float radius;
    static Texture boxtextureptr;
    static Texture treetextureptr;
    static Texture bushtextureptr;
    static Texture rocktextureptr;

    XYZ position;
    int type;
    float yaw;
    float pitch;
    float rotx;
    float rotxvel;
    float roty;
    float rotyvel;
    bool possible;
    Model model;
    Model displaymodel;
    float friction;
    float scale;
    float messedwith;
    float checked;
    float shadowed;
    float occluded;
    bool onfire;
    float flamedelay;

    Object();
    Object(int _type, XYZ _position, float _yaw, float _pitch, float _scale);

    static void ComputeCenter();
    static void ComputeRadius();
    static void AddObjectsToTerrain();
    static void LoadObjectsFromFile(FILE* tfile, bool skip);
    static void SphereCheckPossible(XYZ *p1, float radius);
    static void DeleteObject(int which);
    static void MakeObject(int atype, XYZ where, float ayaw, float apitch, float ascale);
    static void Draw();
    static void DoShadows();
    static void DoStuff();

private:
    void handleFire();
    void doShadows(XYZ lightloc);
    void draw();
    void drawSecondPass();
    void addToTerrain(unsigned id);
};

#endif

