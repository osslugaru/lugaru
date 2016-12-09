/*
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

#include "Animation/Skeleton.h"
#include "Animation/Animation.h"
#include "Utils/Folders.h"
#include "Game.h"

extern bool visibleloading;

//~ struct animation_data_elt {
    //~ const std::string& filename;
    //~ int height;
    //~ int attack;
//~ };

//~ static animation_data_elt animation_data[animation_count] = {
//~ #define DECLARE_ANIM(id, file, height, attack, ...) {file, height, attack},
//~ #include "Animation.def"
//~ #undef DECLARE_ANIM
//~ };
std::vector<Animation> Animation::animations;

void Animation::loadAll()
{
    int i = 0;
#define DECLARE_ANIM(id, file, height, attack, ...) if (i++ < loadable_anim_end) animations.emplace_back(file, height, attack);
#include "Animation.def"
#undef DECLARE_ANIM
}


Animation::Animation():
    numframes(0),
    height(0),
    attack(0),
    joints(0),
    weapontargetnum(0),

    position(0),
    twist(0),
    twist2(0),
    speed(0),
    onground(0),
    forward(0),
    label(0),
    weapontarget(0)
{
}

/* EFFECT
 * load an animation from file
 */
Animation::Animation(const std::string& filename, int aheight, int aattack):
    Animation()
{
    FILE *tfile;
    int i, j;
    XYZ endoffset;

    LOGFUNC;

    // Changing the filename into something the OS can understand
    std::string filepath = Folders::getResourcePath("Animations/"+filename);

    LOG(std::string("Loading animation...") + filepath);

    height = aheight;
    attack = aattack;

    if (visibleloading)
        Game::LoadingScreen();

    // read file in binary mode
    tfile = Folders::openMandatoryFile( filepath, "rb" );

    // read numframes, joints to know how much memory to allocate
    funpackf(tfile, "Bi Bi", &numframes, &joints);

    // allocate memory for everything

    position = (XYZ**)malloc(sizeof(XYZ*) * joints);
    for (i = 0; i < joints; i++)
        position[i] = (XYZ*)malloc(sizeof(XYZ) * numframes);

    twist = (float**)malloc(sizeof(float*) * joints);
    for (i = 0; i < joints; i++)
        twist[i] = (float*)malloc(sizeof(float) * numframes);

    twist2 = (float**)malloc(sizeof(float*) * joints);
    for (i = 0; i < joints; i++)
        twist2[i] = (float*)malloc(sizeof(float) * numframes);

    speed = (float*)malloc(sizeof(float) * numframes);

    onground = (bool**)malloc(sizeof(bool*) * joints);
    for (i = 0; i < joints; i++)
        onground[i] = (bool*)malloc(sizeof(bool) * numframes);

    forward = (XYZ*)malloc(sizeof(XYZ) * numframes);
    weapontarget = (XYZ*)malloc(sizeof(XYZ) * numframes);
    label = (int*)malloc(sizeof(int) * numframes);

    // read binary data as animation

    // for each frame...
    for (i = 0; i < numframes; i++) {
        // for each joint in the skeleton...
        for (j = 0; j < joints; j++) {
            // read joint position
            funpackf(tfile, "Bf Bf Bf", &position[j][i].x, &position[j][i].y, &position[j][i].z);
        }
        for (j = 0; j < joints; j++) {
            // read twist
            funpackf(tfile, "Bf", &twist[j][i]);
        }
        for (j = 0; j < joints; j++) {
            // read onground (boolean)
            unsigned char uch;
            funpackf(tfile, "Bb", &uch);
            onground[j][i] = (uch != 0);
        }
        // read frame speed (?)
        funpackf(tfile, "Bf", &speed[i]);
    }
    // read twist2 for whole animation
    for (i = 0; i < numframes; i++) {
        for (j = 0; j < joints; j++) {
            funpackf(tfile, "Bf", &twist2[j][i]);
        }
    }
    // read label for each frame
    for (i = 0; i < numframes; i++) {
        funpackf(tfile, "Bf", &label[i]);
    }
    // read weapontargetnum
    funpackf(tfile, "Bi", &weapontargetnum);
    // read weapontarget positions for each frame
    for (i = 0; i < numframes; i++) {
        funpackf(tfile, "Bf Bf Bf", &weapontarget[i].x, &weapontarget[i].y, &weapontarget[i].z);
    }

    fclose(tfile);

    endoffset = 0;
    // find average position of certain joints on last frames
    // and save in endoffset
    // (not sure what exactly this accomplishes. the y < 1 test confuses me.)
    for (j = 0; j < joints; j++) {
        if (position[j][numframes - 1].y < 1)
            endoffset += position[j][numframes - 1];
    }
    endoffset /= joints;
    offset = endoffset;
    offset.y = 0;
}

Animation::~Animation()
{
    //~ deallocate();
}

void Animation::deallocate()
{
    int i = 0;

    if (position) {
        for (i = 0; i < joints; i++)
            free(position[i]);

        free(position);
    }
    position = 0;

    if (twist) {
        for (i = 0; i < joints; i++)
            free(twist[i]);

        free(twist);
    }
    twist = 0;

    if (twist2) {
        for (i = 0; i < joints; i++)
            free(twist2[i]);

        free(twist2);
    }
    twist2 = 0;

    if (onground) {
        for (i = 0; i < joints; i++)
            free(onground[i]);

        free(onground);
    }
    onground = 0;

    if (speed)
        free(speed);
    speed = 0;

    if (forward)
        free(forward);
    forward = 0;

    if (weapontarget)
        free(weapontarget);
    weapontarget = 0;

    if (label)
        free(label);
    label = 0;

    joints = 0;
}

Animation & Animation::operator = (const Animation & ani)
{
    int i = 0;

    bool allocate = ((ani.numframes != numframes) || (ani.joints != joints));

    if (allocate)
        deallocate();

    numframes = ani.numframes;
    height = ani.height;
    attack = ani.attack;
    joints = ani.joints;
    weapontargetnum = ani.weapontargetnum;
    offset = ani.offset;

    if (allocate)
        position = (XYZ**)malloc(sizeof(XYZ*)*ani.joints);
    for (i = 0; i < ani.joints; i++) {
        if (allocate)
            position[i] = (XYZ*)malloc(sizeof(XYZ) * ani.numframes);
        memcpy(position[i], ani.position[i], sizeof(XYZ)*ani.numframes);
    }

    if (allocate)
        twist = (float**)malloc(sizeof(float*)*ani.joints);
    for (i = 0; i < ani.joints; i++) {
        if (allocate)
            twist[i] = (float*)malloc(sizeof(float) * ani.numframes);
        memcpy(twist[i], ani.twist[i], sizeof(float)*ani.numframes);
    }

    if (allocate)
        twist2 = (float**)malloc(sizeof(float*)*ani.joints);
    for (i = 0; i < ani.joints; i++) {
        if (allocate)
            twist2[i] = (float*)malloc(sizeof(float) * ani.numframes);
        memcpy(twist2[i], ani.twist2[i], sizeof(float)*ani.numframes);
    }

    if (allocate)
        speed = (float*)malloc(sizeof(float) * ani.numframes);
    memcpy(speed, ani.speed, sizeof(float)*ani.numframes);

    if (allocate)
        onground = (bool**)malloc(sizeof(bool*)*ani.joints);
    for (i = 0; i < ani.joints; i++) {
        if (allocate)
            onground[i] = (bool*)malloc(sizeof(bool) * ani.numframes);
        memcpy(onground[i], ani.onground[i], sizeof(bool)*ani.numframes);
    }

    if (allocate)
        forward = (XYZ*)malloc(sizeof(XYZ) * ani.numframes);
    memcpy(forward, ani.forward, sizeof(XYZ)*ani.numframes);

    if (allocate)
        weapontarget = (XYZ*)malloc(sizeof(XYZ) * ani.numframes);
    memcpy(weapontarget, ani.weapontarget, sizeof(XYZ)*ani.numframes);

    if (allocate)
        label = (int*)malloc(sizeof(int) * ani.numframes);
    memcpy(label, ani.label, sizeof(int)*ani.numframes);

    return (*this);
}
