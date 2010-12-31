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

#ifndef _PERSON_H_
#define _PERSON_H_

/**> HEADER FILES <**/

#include "gamegl.h"
#include "Quaternions.h"
#include "Skeleton.h"
#include "Models.h"
#include "Constants.h"
#include "Terrain.h"
#include "Sprite.h"
#include <cmath>
#include "Weapons.h"
#include "Animation.h"

#define passivetype 0
#define guardtype 1
#define searchtype 2
#define attacktype 3
#define attacktypecutoff 4
#define playercontrolled 5
#define gethelptype 6
#define getweapontype 7
#define pathfindtype 8

#define rabbittype 0
#define wolftype 1

class Person
{
	public:
		Person();
		~Person();

		int whichpatchx;
		int whichpatchz;
		
		int currentframe;
		int targetframe;
		int currentanimation;
		int targetanimation;
		int oldcurrentframe;
		int oldtargetframe;
		int oldcurrentanimation;
		int oldtargetanimation;
		
		int howactive;
		
		float parriedrecently;
		
		bool superruntoggle;
		
		int lastattack,lastattack2,lastattack3;
		
		XYZ currentoffset,targetoffset,offset;
		float target;
		float transspeed;
		
		XYZ realoldcoords;
		XYZ oldcoords;
		XYZ coords;
		XYZ originalcoords;
		XYZ velocity;
		
		XYZ proportionhead;
		XYZ proportionlegs;
		XYZ proportionarms;
		XYZ proportionbody;
		
		float heightleft;
		float heightright;
		
		float unconscioustime;
		
		bool immobile;
		
		float velspeed;
		float targetrotation;
		float targetrot;
		float rot;
		float oldrot;
		float lookrotation;
		float lookrotation2;
		float rotation;
		float rotation2;
		float lowrotation;
		float tilt;
		float targettilt;
		float tilt2;
		float targettilt2;
		bool rabbitkickenabled;
		
		float bloodloss;
		float bleeddelay;
		float skiddelay;
		float skiddingdelay;
		float deathbleeding;
		float tempdeltav;
		
		float damagetolerance;
		float damage;
		float permanentdamage;
		float superpermanentdamage;		float lastcollide;
		int dead;
		
		float jumppower;
		bool onground;
		int madskills;
		
		int wentforweapon;
		
		bool calcrot;
		
		bool backwardsanim;
		
		XYZ facing;
		
		float bleeding;
		float bleedx,bleedy;
		int direction;
		float texupdatedelay;
		
		float headrotation,headrotation2;
		float targetheadrotation,targetheadrotation2;
		
		bool onterrain;
		bool pause;
		
		float grabdelay;
		
		Person *victim;
		bool hasvictim;
		
		float updatedelay;
		float normalsupdatedelay;
		
		bool jumpstart;
		
		bool forwardkeydown;
		bool forwardstogglekeydown;
		bool rightkeydown;
		bool leftkeydown;
		bool backkeydown;
		bool jumpkeydown;
		bool jumptogglekeydown;
		bool crouchkeydown;
		bool crouchtogglekeydown;
		bool drawkeydown;
		bool drawtogglekeydown;
		bool throwkeydown;
		bool throwtogglekeydown;
		bool attackkeydown;
		bool feint;
		bool lastfeint;
		bool headless;
		
		float crouchkeydowntime;
		float jumpkeydowntime;
		bool freefall;
		
		
		float turnspeed;
		
		int aitype;
		int aitarget;
		float aiupdatedelay;
		float losupdatedelay;
		int ally;
		XYZ movetarget;
		float collide;
		float collided;
		float avoidcollided;
		bool loaded;
		bool whichdirection;
		float whichdirectiondelay;
		bool avoidsomething;		XYZ avoidwhere;		
		float blooddimamount;
		
		float staggerdelay;
		float blinkdelay;
		float twitchdelay;
		float twitchdelay2;
		float twitchdelay3;
		float lefthandmorphness;
		float righthandmorphness;
		float headmorphness;
		float chestmorphness;
		float tailmorphness;
		float targetlefthandmorphness;
		float targetrighthandmorphness;
		float targetheadmorphness;
		float targetchestmorphness;
		float targettailmorphness;
		int lefthandmorphstart,lefthandmorphend;
		int righthandmorphstart,righthandmorphend;
		int headmorphstart,headmorphend;
		int chestmorphstart,chestmorphend;
		int tailmorphstart,tailmorphend;
		
		float weaponmissdelay;
		float highreversaldelay;
		float lowreversaldelay;
		float nocollidedelay;
		
		int creature;
		
		int id;
		
		Skeleton skeleton;
		
		float speed;
		float scale;
		float power;
		float speedmult;
		
		float protectionhead;
		float protectionhigh;
		float protectionlow;
		float armorhead;
		float armorhigh;
		float armorlow;
		bool metalhead;
		bool metalhigh;
		bool metallow;
		
		int numclothes;
		char clothes[10][256];
		float clothestintr[10];
		float clothestintg[10];
		float clothestintb[10];
		
		bool landhard;
		bool bled;
		bool spurt;
		bool onfire;
		float onfiredelay;		float burnt;
		float fireduration;
		
		float flamedelay;
		float updatestuffdelay;
		
		int playerdetail;
		
		int num_weapons;
		int weaponids[4];
		int weaponactive;
		int weaponstuck;
		int weaponstuckwhere;
		int weaponwhere;
		
		int numwaypoints;
		XYZ waypoints[90];
		int waypointtype[90];
		float pausetime;
		bool hastempwaypoint;
		XYZ tempwaypoint;
		
		XYZ headtarget;
		float interestdelay;
		
		XYZ finalfinaltarget;
		XYZ finaltarget;
		int finalpathfindpoint;
		int targetpathfindpoint;
		int lastpathfindpoint;
		int lastpathfindpoint2;
		int lastpathfindpoint3;
		int lastpathfindpoint4;
		bool onpath;
		
		int waypoint;
		bool jumppath;
		
		XYZ lastseen;
		float lastseentime;
		float lastchecktime;
		float stunned;
		float surprised;
		float runninghowlong;		int lastoccluded;
		int laststanding;
		int escapednum;
		
		float speechdelay;
		float neckspurtdelay;
		float neckspurtparticledelay;
		float neckspurtamount;
		
		int whichskin;
		bool rabbitkickragdoll;
		
		XYZ averageloc;
		XYZ oldaverageloc;
		
		Animation tempanimation;
		
		float occluded;
		
		void CheckKick();
		void CatchFire();
		void DoBlood(float howmuch, int which);
		void DoBloodBig(float howmuch, int which);
		bool DoBloodBigWhere(float howmuch, int which, XYZ where);
		
		bool wasIdle()
		{
		  return animation_bits[currentanimation] & ab_idle;
		}
		bool isIdle()
		{
		  return animation_bits[targetanimation] & ab_idle;
		}
		int getIdle();
		
		bool isSitting()
		{
		  return animation_bits[targetanimation] & ab_sit;
		}

		bool isSleeping()
		{
		  return animation_bits[targetanimation] & ab_sleep;
		}

		bool wasCrouch()
		{
		  return animation_bits[currentanimation] & ab_crouch;
		}
		bool isCrouch()
		{
		  return animation_bits[targetanimation] & ab_crouch;
		}
		int getCrouch();
		
		bool wasStop()
		{
		  return animation_bits[currentanimation] & ab_stop;
		}
		bool isStop()
		{
		  return animation_bits[targetanimation] & ab_stop;
		}
		int getStop();
		
		bool wasSneak();
		bool isSneak();
		int getSneak();
		
		bool wasRun()
		{
		  return animation_bits[currentanimation] & ab_run;
		}
		bool isRun()
		{
		  return animation_bits[targetanimation] & ab_run;
		}
		int getRun();

		bool wasLanding()
		{
		  return animation_bits[currentanimation] & ab_land;
		}
		bool isLanding()
		{
		  return animation_bits[targetanimation] & ab_land;
		}
		int getLanding();
		
		bool wasLandhard();
		bool isLandhard();
		int getLandhard();
		
		bool isFlip();
		bool wasFlip();
		
		bool jumpclimb;
		
		bool isWallJump();
		void Reverse();
		void DoDamage(float howmuch);
		void DoHead();
		void DoMipmaps(int howmanylevels, float startx, float endx, float starty, float endy);
		int SphereCheck(XYZ *p1,float radius, XYZ *p, XYZ *move, float *rotate, Model *model);
		int DrawSkeleton();
		void Puff(int whichlabel);
		void FootLand(int which, float opacity);
		void DoStuff();
		void DoAnimations();
		void RagDoll(bool checkcollision);
};

extern Person player[maxplayers];
#endif
