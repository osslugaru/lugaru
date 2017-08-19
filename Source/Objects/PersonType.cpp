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

#include "Objects/PersonType.hpp"

std::vector<PersonType> PersonType::types;

PersonType::PersonType()
{
    animTalkIdle = tempanim;
    animHurtIdle = tempanim;
}

bool PersonType::hasAnimTalkIdle()
{
    return (animTalkIdle != tempanim);
}

bool PersonType::hasAnimHurtIdle()
{
    return (animHurtIdle != tempanim);
}

void PersonType::Load()
{
    types.resize(2);

    /* Wolf */
    types[wolftype].proportions[0] = 1.1;
    types[wolftype].proportions[1] = 1.1;
    types[wolftype].proportions[2] = 1.1;
    types[wolftype].proportions[3] = 1.1;

    types[wolftype].animRun = wolfrunanim;
    types[wolftype].animRunning = wolfrunninganim;
    types[wolftype].animCrouch = wolfcrouchanim;
    types[wolftype].animStop = wolfstopanim;
    types[wolftype].animLanding = wolflandanim;
    types[wolftype].animLandingHard = wolflandhardanim;
    types[wolftype].animFightIdle = wolfidle;
    types[wolftype].animBounceIdle = wolfidle;

    types[wolftype].soundsAttack[0] = barksound;
    types[wolftype].soundsAttack[1] = bark2sound;
    types[wolftype].soundsAttack[2] = bark3sound;
    types[wolftype].soundsAttack[3] = barkgrowlsound;
    types[wolftype].soundsTalk[0] = growlsound;
    types[wolftype].soundsTalk[1] = growl2sound;

    types[wolftype].figureFileName = "Skeleton/BasicFigureWolf";
    types[wolftype].lowFigureFileName = "Skeleton/BasicFigureWolfLow";
    types[wolftype].clothesFileName = "Skeleton/RabbitBelt";
    types[wolftype].modelFileNames[0] = "Models/Wolf.solid";
    types[wolftype].modelFileNames[1] = "Models/Wolf2.solid";
    types[wolftype].modelFileNames[2] = "Models/Wolf3.solid";
    types[wolftype].modelFileNames[3] = "Models/Wolf4.solid";
    types[wolftype].modelFileNames[4] = "Models/Wolf5.solid";
    types[wolftype].modelFileNames[5] = "Models/Wolf6.solid";
    types[wolftype].modelFileNames[6] = "Models/Wolf7.solid";
    types[wolftype].lowModelFileName = "Models/WolfLow.solid";
    types[wolftype].modelClothesFileName = "Models/Belt.solid";

    types[wolftype].skins.resize(3);
    types[wolftype].skins[0] = "Textures/FurWolfGrey.jpg";
    types[wolftype].skins[1] = "Textures/FurWolfDark.jpg";
    types[wolftype].skins[2] = "Textures/FurWolfSnow.jpg";

    types[wolftype].power = 2.5;
    types[wolftype].defaultDamageTolerance = 300;
    types[wolftype].defaultScale = .23;
    types[wolftype].hasClaws = true;
    types[wolftype].clothes = false;
    types[wolftype].maxRunSpeed = 75;
    types[wolftype].knifeCatchingType = 1;

    /* Rabbit */
    types[rabbittype].proportions[0] = 1.2;
    types[rabbittype].proportions[1] = 1.05;
    types[rabbittype].proportions[2] = 1;
    types[rabbittype].proportions[3] = 1.1;
    types[rabbittype].proportions[3].y = 1.05;

    types[rabbittype].animRun = runanim;
    types[rabbittype].animRunning = rabbitrunninganim;
    types[rabbittype].animCrouch = crouchanim;
    types[rabbittype].animStop = stopanim;
    types[rabbittype].animLanding = landanim;
    types[rabbittype].animLandingHard = landhardanim;
    types[rabbittype].animFightIdle = fightidleanim;
    types[rabbittype].animBounceIdle = bounceidleanim;
    types[rabbittype].animTalkIdle = talkidleanim;
    types[rabbittype].animHurtIdle = hurtidleanim;

    types[rabbittype].soundsAttack[0] = rabbitattacksound;
    types[rabbittype].soundsAttack[1] = rabbitattack2sound;
    types[rabbittype].soundsAttack[2] = rabbitattack3sound;
    types[rabbittype].soundsAttack[3] = rabbitattack4sound;
    types[rabbittype].soundsTalk[0] = rabbitchitter;
    types[rabbittype].soundsTalk[1] = rabbitchitter2;

    types[rabbittype].figureFileName = "Skeleton/BasicFigure";
    types[rabbittype].lowFigureFileName = "Skeleton/BasicFigureLow";
    types[rabbittype].clothesFileName = "Skeleton/RabbitBelt";
    types[rabbittype].modelFileNames[0] = "Models/Body.solid";
    types[rabbittype].modelFileNames[1] = "Models/Body2.solid";
    types[rabbittype].modelFileNames[2] = "Models/Body3.solid";
    types[rabbittype].modelFileNames[3] = "Models/Body4.solid";
    types[rabbittype].modelFileNames[4] = "Models/Body5.solid";
    types[rabbittype].modelFileNames[5] = "Models/Body6.solid";
    types[rabbittype].modelFileNames[6] = "Models/Body7.solid";
    types[rabbittype].lowModelFileName = "Models/BodyLow.solid";
    types[rabbittype].modelClothesFileName = "Models/Belt.solid";

    types[rabbittype].skins.resize(10);
    types[rabbittype].skins[0] = "Textures/FurBrown.jpg";
    types[rabbittype].skins[1] = "Textures/FurWhite.jpg";
    types[rabbittype].skins[2] = "Textures/FurBlack.jpg";
    types[rabbittype].skins[3] = "Textures/FurLynx.jpg";
    types[rabbittype].skins[4] = "Textures/FurOtter.jpg";
    types[rabbittype].skins[5] = "Textures/FurOpal.jpg";
    types[rabbittype].skins[6] = "Textures/FurSable.jpg";
    types[rabbittype].skins[7] = "Textures/FurChocolate.jpg";
    types[rabbittype].skins[8] = "Textures/FurBlackWhite.jpg";
    types[rabbittype].skins[9] = "Textures/FurBrownWhite.jpg";

    types[rabbittype].power = 1;
    types[rabbittype].defaultDamageTolerance = 200;
    types[rabbittype].defaultScale = .2;
    types[rabbittype].hasClaws = false;
    types[rabbittype].clothes = true;
    types[rabbittype].maxRunSpeed = 55;
    types[rabbittype].knifeCatchingType = 0;
}
