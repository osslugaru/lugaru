/*
Copyright (C) 2003, 2010 - Wolfire Games

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

#ifndef _WEAPONS_H_
#define _WEAPONS_H_

/**> HEADER FILES <**/

#include "gamegl.h"
#include "Quaternions.h"
#include "Skeleton.h"
#include "Models.h"
#include "Constants.h"
#include "Terrain.h"
#include "Sprites.h"
#include "Person.h"
#include <cmath>

#define max_weapons 30
#define max_weaponinstances 20

#define knife 1
#define sword 2
#define staff 3

class Weapons
{
public:

	float rotation1[max_weaponinstances];
	float rotation2[max_weaponinstances];
	float rotation3[max_weaponinstances];
	float bigrotation[max_weaponinstances];
	float bigtilt[max_weaponinstances];
	float bigtilt2[max_weaponinstances];
	float smallrotation[max_weaponinstances];
	float smallrotation2[max_weaponinstances];

	int numweapons;
	float damage[max_weaponinstances];
	XYZ tippoint[max_weaponinstances];
	XYZ oldtippoint[max_weaponinstances];
	XYZ position[max_weaponinstances];
	float lastmult[max_weaponinstances];
	XYZ oldposition[max_weaponinstances];
	XYZ velocity[max_weaponinstances];
	XYZ tipvelocity[max_weaponinstances];
	int type[max_weaponinstances];
	int oldowner[max_weaponinstances];
	int owner[max_weaponinstances];
	int bloody[max_weaponinstances];
	float blooddrip[max_weaponinstances];
	float blooddripdelay[max_weaponinstances];
	bool onfire[max_weaponinstances];
	float flamedelay[max_weaponinstances];
	bool missed[max_weaponinstances];
	float mass[max_weaponinstances];
	float tipmass[max_weaponinstances];
	float length[max_weaponinstances];
	float freetime[max_weaponinstances];
	bool firstfree[max_weaponinstances];
	bool physics[max_weaponinstances];
	float drawhowmany[max_weaponinstances];
	bool hitsomething[max_weaponinstances];

	XYZ lastdrawnposition[max_weaponinstances];
	XYZ lastdrawntippoint[max_weaponinstances];
	float lastdrawnrotation1[max_weaponinstances];
	float lastdrawnrotation2[max_weaponinstances];
	float lastdrawnrotation3[max_weaponinstances];
	float lastdrawnbigrotation[max_weaponinstances];
	float lastdrawnbigtilt[max_weaponinstances];
	float lastdrawnbigtilt2[max_weaponinstances];
	float lastdrawnsmallrotation[max_weaponinstances];
	float lastdrawnsmallrotation2[max_weaponinstances];
	int lastdrawnanim[max_weaponinstances];

	Model throwingknifemodel;
	GLuint knifetextureptr;
	GLuint lightbloodknifetextureptr;
	GLuint bloodknifetextureptr;

	Model swordmodel;
	GLuint swordtextureptr;
	GLuint lightbloodswordtextureptr;
	GLuint bloodswordtextureptr;

	Model staffmodel;
	GLuint stafftextureptr;

	int Draw();
	void DoStuff();

	Weapons();

	~Weapons();
};

#endif
