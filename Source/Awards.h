/*
Copyright (C) 2010 - Lugaru authors

This file is part of Lugaru.

Lugaru is free software; you can redistribute it and/or
modify it under the terms of the GNU General Public License
as published by the Free Software Foundation; either version 2
of the License, or (at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.

See the GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program; if not, write to the Free Software
Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
*/

#ifndef AWARDS_H
#define AWARDS_H

enum bonus_types {
#define DECLARE_BONUS(id, name) id,
#include "Bonuses.def"
#undef DECLARE_BONUS
bonus_count
};

static const char *bonus_names[bonus_count] = {
#define DECLARE_BONUS(id, name) name,
#include "Bonuses.def"
#undef DECLARE_BONUS
};

enum award_types {
#define DECLARE_AWARD(id, name) id,
#include "Awards.def"
#undef DECLARE_AWARD
award_count
};

static const char *award_names[award_count] = {
#define DECLARE_AWARD(id, name) name,
#include "Awards.def"
#undef DECLARE_AWARD
};

extern int award_awards(int *);

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

