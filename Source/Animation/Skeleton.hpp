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

#ifndef _SKELETON_HPP_
#define _SKELETON_HPP_

#include "Animation/Animation.hpp"
#include "Animation/Joint.hpp"
#include "Animation/Muscle.hpp"
#include "Graphic/Models.hpp"
#include "Graphic/Sprite.hpp"
#include "Graphic/gamegl.hpp"
#include "Math/XYZ.hpp"
#include "Objects/Object.hpp"
#include "Utils/binio.h"

const int max_joints = 50;

class Skeleton
{
public:
    std::vector<Joint> joints;

    std::vector<Muscle> muscles;

    int selected;

    int forwardjoints[3];
    XYZ forward;

    int id;

    int lowforwardjoints[3];
    XYZ lowforward;

    XYZ specialforward[5];
    int jointlabels[max_joints];

    Model model[7];
    Model modellow;
    Model modelclothes;
    int num_models;

    Model drawmodel;
    Model drawmodellow;
    Model drawmodelclothes;

    bool clothes;
    bool spinny;

    GLubyte skinText[512 * 512 * 3];
    int skinsize;

    float checkdelay;

    float longdead;
    bool broken;

    int free;
    int oldfree;
    float freetime;
    bool freefall;

    void FindForwards();
    float DoConstraints(XYZ* coords, float* scale);
    void DoGravity(float* scale);
    void FindRotationJoint(int which);
    void FindRotationJointSameTwist(int which);
    void FindRotationMuscle(int which, int animation);
    void Load(const std::string& fileName, const std::string& lowfileName, const std::string& clothesfileName, const std::string& modelfileName, const std::string& model2fileName, const std::string& model3fileName, const std::string& model4fileName, const std::string& model5fileNamee, const std::string& model6fileName, const std::string& model7fileName, const std::string& modellowfileName, const std::string& modelclothesfileName, bool aclothes);

    Skeleton();

private:
    // convenience functions
    // only for Skeleton.cpp
    inline Joint& joint(int bodypart) { return joints[jointlabels[bodypart]]; }
    inline XYZ& jointPos(int bodypart) { return joint(bodypart).position; }
    inline XYZ& jointVel(int bodypart) { return joint(bodypart).velocity; }
};

#endif
