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

#ifndef _AWARDS_HPP_
#define _AWARDS_HPP_

enum bonus_types
{
#define DECLARE_BONUS(id, ...) id,
#include "Bonuses.def"
#undef DECLARE_BONUS
    bonus_count
};

extern const char* bonus_names[bonus_count];

extern int bonus;
extern int bonusvalue;
extern int bonustotal;
extern float bonustime;
extern int startbonustotal;
extern float bonusnum[100];

extern void award_bonus(int playerid, int bonusid, int alt_value = 0);

enum award_types
{
#define DECLARE_AWARD(id, name) id,
#include "Awards.def"
#undef DECLARE_AWARD
    award_count
};

extern const char* award_names[award_count];

extern int award_awards(int*);

extern float damagetaken;
extern int numfalls;
extern int numflipfail;
extern int numseen;
extern int numresponded;
extern int numstaffattack;
extern int numswordattack;
extern int numknifeattack;
extern int numunarmedattack;
extern int numescaped;
extern int numflipped;
extern int numwallflipped;
extern int numthrowkill;
extern int numafterkill;
extern int numreversals;
extern int numattacks;
extern int maxalarmed;
#endif
