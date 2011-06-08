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
#include "Terrain.h"
#include "Sprite.h"
#include "Person.h"
#include "Texture.h"
#include <cmath>

#define max_weapons 30
#define max_weaponinstances 20

#define knife 1
#define sword 2
#define staff 3

class Weapon {
	public:
		Weapon(int type, int owner);
			
		static Model throwingknifemodel;
		static Texture knifetextureptr;
		static Texture lightbloodknifetextureptr;
		static Texture bloodknifetextureptr;

		static Model swordmodel;
		static Texture swordtextureptr;
		static Texture lightbloodswordtextureptr;
		static Texture bloodswordtextureptr;

		static Model staffmodel;
		static Texture stafftextureptr;
	
		void Draw();
		void DoStuff(int);
		
		int getType() { return type; }
		void setType(int);
		
		int owner;
		XYZ position;
		XYZ tippoint;
		XYZ velocity;
		XYZ tipvelocity;
		bool missed;
		bool hitsomething;
		float freetime;
		bool firstfree;
		bool physics;
		
		float damage;
		int bloody;
		float blooddrip;
		float blooddripdelay;
		
		float rotation1;
		float rotation2;
		float rotation3;
		float bigrotation;
		float bigtilt;
		float bigtilt2;
		float smallrotation;
		float smallrotation2;
	private:
		int type;
		
		XYZ oldtippoint;
		float lastmult;
		XYZ oldposition;
		int oldowner;
		bool onfire;
		float flamedelay;
		float mass;
		float tipmass;
		float length;
		float drawhowmany;

		XYZ lastdrawnposition;
		XYZ lastdrawntippoint;
		float lastdrawnrotation1;
		float lastdrawnrotation2;
		float lastdrawnrotation3;
		float lastdrawnbigrotation;
		float lastdrawnbigtilt;
		float lastdrawnbigtilt2;
		float lastdrawnsmallrotation;
		float lastdrawnsmallrotation2;
		int lastdrawnanim;
};

class Weapons : public std::vector<Weapon>
{
public:
	Weapons();
	~Weapons();
	
	int Draw();
	void DoStuff();
};

extern Weapons weapons;
#endif
