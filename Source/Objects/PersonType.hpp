/*
Copyright (C) 2016-2017 - Lugaru contributors (see AUTHORS file)

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

#ifndef _PERSONTYPE_HPP_
#define _PERSONTYPE_HPP_

#include "Animation/Animation.hpp"
#include "Audio/Sounds.hpp"
#include "Math/XYZ.hpp"

enum person_type
{
    rabbittype = 0,
    wolftype = 1
};

class PersonType
{
public:
    // head, body, arms, legs
    XYZ proportions[4];
    animation_type animRun;
    animation_type animRunning;
    animation_type animCrouch;
    animation_type animStop;
    animation_type animLanding;
    animation_type animLandingHard;
    animation_type animFightIdle;
    animation_type animBounceIdle;
    animation_type animTalkIdle;
    animation_type animHurtIdle;

    sound_type soundsAttack[4];
    sound_type soundsTalk[2];

    std::string figureFileName;
    std::string lowFigureFileName;
    std::string clothesFileName;
    std::string modelFileNames[7];
    std::string lowModelFileName;
    std::string modelClothesFileName;

    std::vector<std::string> skins;

    float power;
    unsigned int defaultDamageTolerance;
    float defaultScale;
    bool hasClaws;
    bool clothes;
    unsigned int maxRunSpeed;
    unsigned int knifeCatchingType;

    GLubyte bloodText[512 * 512 * 3] = { 0 };

    PersonType();
    bool hasAnimTalkIdle();
    bool hasAnimHurtIdle();

    static std::vector<PersonType> types;
    static void Load();
};

#endif
