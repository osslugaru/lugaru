/*
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

#include "Animation/Animation.hpp"

#include "Animation/Skeleton.hpp"
#include "Game.hpp"
#include "Utils/Folders.hpp"

std::vector<Animation> Animation::animations;

void Animation::loadAll()
{
#define DECLARE_ANIM(id, file, height, attack, ...) \
    if (id < loadable_anim_end)                     \
        animations.emplace_back(file, height, attack);
#include "Animation.def"
#undef DECLARE_ANIM
}

void AnimationFrame::loadBaseInfo(FILE* tfile)
{
    // for each joint in the skeleton...
    for (unsigned j = 0; j < joints.size(); j++) {
        // read joint position
        funpackf(tfile, "Bf Bf Bf", &joints[j].position.x, &joints[j].position.y, &joints[j].position.z);
    }
    for (unsigned j = 0; j < joints.size(); j++) {
        // read twist
        funpackf(tfile, "Bf", &joints[j].twist);
    }
    for (unsigned j = 0; j < joints.size(); j++) {
        // read onground (boolean)
        unsigned char uch;
        funpackf(tfile, "Bb", &uch);
        joints[j].onground = (uch != 0);
    }
    // read frame speed (?)
    funpackf(tfile, "Bf", &speed);
}

void AnimationFrame::loadTwist2(FILE* tfile)
{
    for (unsigned j = 0; j < joints.size(); j++) {
        funpackf(tfile, "Bf", &joints[j].twist2);
    }
}

void AnimationFrame::loadLabel(FILE* tfile)
{
    funpackf(tfile, "Bf", &label);
}

void AnimationFrame::loadWeaponTarget(FILE* tfile)
{
    funpackf(tfile, "Bf Bf Bf", &weapontarget.x, &weapontarget.y, &weapontarget.z);
}

Animation::Animation()
    : height(lowheight)
    , attack(neutral)
    , numjoints(0)
{
}

/* EFFECT
 * load an animation from file
 */
Animation::Animation(const std::string& filename, anim_height_type aheight, anim_attack_type aattack)
    : Animation()
{
    FILE* tfile;
    int numframes;
    unsigned i;

    LOGFUNC;

    // Changing the filename into something the OS can understand
    std::string filepath = Folders::getResourcePath("Animations/" + filename);

    LOG(std::string("Loading animation...") + filepath);

    height = aheight;
    attack = aattack;

    Game::LoadingScreen();

    // read file in binary mode
    tfile = Folders::openMandatoryFile(filepath, "rb");

    // read numframes, joints to know how much memory to allocate
    funpackf(tfile, "Bi Bi", &numframes, &numjoints);

    // allocate memory for everything

    frames.resize(numframes);

    // read binary data as animation

    // for each frame...
    for (i = 0; i < frames.size(); i++) {
        frames[i].joints.resize(numjoints);
        frames[i].loadBaseInfo(tfile);
    }
    // read twist2 for whole animation
    for (i = 0; i < frames.size(); i++) {
        frames[i].loadTwist2(tfile);
    }
    // read label for each frame
    for (i = 0; i < frames.size(); i++) {
        frames[i].loadLabel(tfile);
    }
    // read unused weapontargetnum
    int weapontargetnum;
    funpackf(tfile, "Bi", &weapontargetnum);
    // read weapontarget positions for each frame
    for (i = 0; i < frames.size(); i++) {
        frames[i].loadWeaponTarget(tfile);
    }

    fclose(tfile);

    XYZ endoffset;
    endoffset = 0;
    // find average position of certain joints on last frames
    // and save in endoffset
    // (not sure what exactly this accomplishes. the y < 1 test confuses me.)
    for (i = 0; i < frames.back().joints.size(); i++) {
        if (frames.back().joints[i].position.y < 1) {
            endoffset += frames.back().joints[i].position;
        }
    }
    endoffset /= numjoints;
    offset = endoffset;
    offset.y = 0;
}
