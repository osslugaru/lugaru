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

#include "Level/Awards.hpp"

#include "Game.hpp"
#include "Objects/Person.hpp"

int bonus;
int bonusvalue;
int bonustotal;
int startbonustotal;
float bonustime;
float bonusnum[100];

const char* bonus_names[bonus_count] = {
#define DECLARE_BONUS(id, name, ...) name,
#include "Bonuses.def"
#undef DECLARE_BONUS
};

const char* award_names[award_count] = {
#define DECLARE_AWARD(id, name) name,
#include "Awards.def"
#undef DECLARE_AWARD
};

static const int bonus_values[bonus_count] = {
#define DECLARE_BONUS(id, name, value) value,
#include "Bonuses.def"
#undef DECLARE_BONUS
};

void award_bonus(int playerid, int bonusid, int alt_value)
{
    if (playerid != 0) {
        return;
    }
    bonus = bonusid;
    bonustime = 0;
    bonusvalue = alt_value ? alt_value : bonus_values[bonusid];
}

// FIXME: make these per-player
float damagetaken;
int numfalls;
int numflipfail;
int numseen;
int numresponded;
int numstaffattack;
int numswordattack;
int numknifeattack;
int numunarmedattack;
int numescaped;
int numflipped;
int numwallflipped;
int numthrowkill;
int numafterkill;
int numreversals;
int numattacks;
int maxalarmed;

int award_awards(int* awards)
{
    int numawards = 0;
    if (damagetaken == 0 && Person::players[0]->bloodloss == 0) {
        awards[numawards] = awardflawless;
        numawards++;
    }
    bool alldead = true;
    for (unsigned i = 1; i < Person::players.size(); i++) {
        if (Person::players[i]->dead != 2) {
            alldead = 0;
        }
    }
    if (alldead) {
        awards[numawards] = awardalldead;
        numawards++;
    }
    alldead = 1;
    for (unsigned i = 1; i < Person::players.size(); i++) {
        if (Person::players[i]->dead != 1) {
            alldead = 0;
        }
    }
    if (alldead) {
        awards[numawards] = awardnodead;
        numawards++;
    }
    if (numresponded == 0 && !numthrowkill) {
        awards[numawards] = awardstealth;
        numawards++;
    }
    if (numattacks == numstaffattack && numattacks > 0) {
        awards[numawards] = awardbojutsu;
        numawards++;
    }
    if (numattacks == numswordattack && numattacks > 0) {
        awards[numawards] = awardswordsman;
        numawards++;
    }
    if (numattacks == numknifeattack && numattacks > 0) {
        awards[numawards] = awardknifefighter;
        numawards++;
    }
    if (numattacks == numunarmedattack && numthrowkill == 0 && weapons.size() > 0) {
        awards[numawards] = awardkungfu;
        numawards++;
    }
    if (numescaped > 0) {
        awards[numawards] = awardevasion;
        numawards++;
    }
    if (numflipfail == 0 && numflipped + numwallflipped * 2 > 20) {
        awards[numawards] = awardacrobat;
        numawards++;
    }
    if (numthrowkill == (int(Person::players.size()) - 1)) {
        awards[numawards] = awardlongrange;
        numawards++;
    }
    alldead = 1;
    for (unsigned i = 1; i < Person::players.size(); i++) {
        if (Person::players[i]->dead != 2) {
            alldead = 0;
        }
    }
    if (numafterkill > 0 && alldead) {
        awards[numawards] = awardbrutal;
        numawards++;
    }
    if (numreversals > ((float)numattacks) * .8 && numreversals > 3) {
        awards[numawards] = awardaikido;
        numawards++;
    }
    if (maxalarmed == 1 && Person::players.size() > 2) {
        awards[numawards] = awardstrategy;
        numawards++;
    }
    if (numflipfail > 3) {
        awards[numawards] = awardklutz;
        numawards++;
    }
    return numawards;
}
