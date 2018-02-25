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

#ifndef _OBJECTS_HPP_
#define _OBJECTS_HPP_

#include "Environment/Lights.hpp"
#include "Environment/Terrain.hpp"
#include "Graphic/Models.hpp"
#include "Graphic/Sprite.hpp"
#include "Graphic/Texture.hpp"
#include "Graphic/gamegl.hpp"
#include "Math/Frustum.hpp"
#include "Math/XYZ.hpp"
#include "Utils/ImageIO.hpp"

#include <memory>
#include <vector>
#include <json/value.h>
//
// Model Structures
//

#define max_objects 300

enum object_type
{
    boxtype = 0,
    weirdtype = 1,
    spiketype = 2,
    treetrunktype = 3,
    treeleavestype = 4,
    bushtype = 5,
    rocktype = 6,
    walltype = 7,
    chimneytype = 8,
    platformtype = 9,
    tunneltype = 11,
    cooltype = 12,
    firetype = 13
};

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
    object_type type;
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
    Object(object_type _type, XYZ _position, float _yaw, float _pitch, float _scale);
    Object(Json::Value, float);

    static void ComputeCenter();
    static void ComputeRadius();
    static void AddObjectsToTerrain();
    static void LoadObjectsFromFile(FILE* tfile, bool skip);
    static void LoadObjectsFromJson(Json::Value);
    static void SphereCheckPossible(XYZ* p1, float radius);
    static void DeleteObject(int which);
    static void MakeObject(int atype, XYZ where, float ayaw, float apitch, float ascale);
    static void Draw();
    static void DoShadows();
    static void DoStuff();
    static int checkcollide(XYZ startpoint, XYZ endpoint);
    static int checkcollide(XYZ startpoint, XYZ endpoint, int what);

    operator Json::Value();

private:
    void handleFire();
    void handleRot(int divide);
    void doShadows(XYZ lightloc);
    void draw();
    void drawSecondPass();
    void addToTerrain(unsigned id);
    static int checkcollide(XYZ startpoint, XYZ endpoint, int what, float minx, float miny, float minz, float maxx, float maxy, float maxz);
};

#endif
