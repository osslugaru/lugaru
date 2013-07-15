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
#include "Terrain.h"
#include "Sprite.h"
#include <cmath>
#include <memory>
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

/* TODO : Do a proper init of fields, not this all to 0 one */
class Person : public enable_shared_from_this<Person>
{
public:
    static std::vector<std::shared_ptr<Person>> players;

    int whichpatchx = 0;
    int whichpatchz = 0;

    // animCurrent and animTarget are used to interpolate between different animations
    // (and for a bunch of other things).
    // animations interpolate with one another at various speeds.
    // animTarget seems to determine the combat state?
    int animCurrent = 0;
    int animTarget = 0;

    // frameCurrent and frameTarget are used to interpolate between the frames of an animation
    // (e.g. the crouched animation has only two frames, lerped back and forth slowly).
    // animations advance at various speeds.
    int frameCurrent = 0;
    int frameTarget = 0;

    int oldanimCurrent = 0;
    int oldanimTarget = 0;
    int oldframeCurrent = 0;
    int oldframeTarget = 0;

    int howactive = 0;

    float parriedrecently = 0;

    bool superruntoggle = 0;

    int lastattack = 0, lastattack2 = 0, lastattack3 = 0;

    XYZ currentoffset, targetoffset, offset;
    float target = 0;
    float transspeed = 0;

    XYZ realoldcoords;
    XYZ oldcoords;
    XYZ coords;
    XYZ originalcoords;
    XYZ velocity;

    XYZ proportionhead;
    XYZ proportionlegs;
    XYZ proportionarms;
    XYZ proportionbody;

    float unconscioustime = 0;

    bool immobile = 0;

    float velspeed = 0;
    float targetyaw = 0;
    float targetrot = 0;
    float rot = 0;
    float oldrot = 0;
    float lookyaw = 0;
    float lookpitch = 0;
    float yaw = 0;
    float pitch = 0;
    float lowyaw = 0;
    float tilt = 0;
    float targettilt = 0;
    float tilt2 = 0;
    float targettilt2 = 0;
    bool rabbitkickenabled = 0;

    float bloodloss = 0;
    float bleeddelay = 0;
    float skiddelay = 0;
    float skiddingdelay = 0;
    float deathbleeding = 0;
    float tempdeltav = 0;

    float damagetolerance = 0;
    float damage = 0;
    float permanentdamage = 0;
    float superpermanentdamage = 0;
    float lastcollide = 0;
    int dead = 0;

    float jumppower = 0;
    bool onground = 0;
    int madskills = 0;

    int wentforweapon = 0;

    bool calcrot = 0;

    bool backwardsanim = 0;

    XYZ facing;

    float bleeding = 0;
    float bleedx = 0, bleedy = 0;
    int direction = 0;
    float texupdatedelay = 0;

    float headyaw = 0, headpitch = 0;
    float targetheadyaw = 0, targetheadpitch = 0;

    bool onterrain = 0;
    bool pause = 0;

    float grabdelay = 0;

    std::shared_ptr<Person> victim;
    bool hasvictim = 0;

    float updatedelay = 0;
    float normalsupdatedelay = 0;

    bool jumpstart = 0;

    bool forwardkeydown = 0;
    bool forwardstogglekeydown = 0;
    bool rightkeydown = 0;
    bool leftkeydown = 0;
    bool backkeydown = 0;
    bool jumpkeydown = 0;
    bool jumptogglekeydown = 0;
    bool crouchkeydown = 0;
    bool crouchtogglekeydown = 0;
    bool drawkeydown = 0;
    bool drawtogglekeydown = 0;
    bool throwkeydown = 0;
    bool throwtogglekeydown = 0;
    bool attackkeydown = 0;
    bool feint = 0;
    bool lastfeint = 0;
    bool headless = 0;

    float crouchkeydowntime = 0;
    float jumpkeydowntime = 0;
    bool freefall = 0;

    float turnspeed = 0;

    int aitype = 0;
    float aiupdatedelay = 0;
    float losupdatedelay = 0;
    int ally = 0;
    XYZ movetarget;
    float collide = 0;
    float collided = 0;
    float avoidcollided = 0;
    bool loaded = 0;
    bool whichdirection = 0;
    float whichdirectiondelay = 0;
    bool avoidsomething = 0;
    XYZ avoidwhere;
    float blooddimamount = 0;

    float staggerdelay = 0;
    float blinkdelay = 0;
    float twitchdelay = 0;
    float twitchdelay2 = 0;
    float twitchdelay3 = 0;
    float lefthandmorphness = 0;
    float righthandmorphness = 0;
    float headmorphness = 0;
    float chestmorphness = 0;
    float tailmorphness = 0;
    float targetlefthandmorphness = 0;
    float targetrighthandmorphness = 0;
    float targetheadmorphness = 0;
    float targetchestmorphness = 0;
    float targettailmorphness = 0;
    int lefthandmorphstart = 0, lefthandmorphend = 0;
    int righthandmorphstart = 0, righthandmorphend = 0;
    int headmorphstart = 0, headmorphend = 0;
    int chestmorphstart = 0, chestmorphend = 0;
    int tailmorphstart = 0, tailmorphend = 0;

    float weaponmissdelay = 0;
    float highreversaldelay = 0;
    float lowreversaldelay = 0;
    float nocollidedelay = 0;

    int creature = 0;

    int id = 0;

    Skeleton skeleton;

    float speed = 0;
    float scale = 0;
    float power = 0;
    float speedmult = 0;

    float protectionhead = 0;
    float protectionhigh = 0;
    float protectionlow = 0;
    float armorhead = 0;
    float armorhigh = 0;
    float armorlow = 0;
    bool metalhead = 0;
    bool metalhigh = 0;
    bool metallow = 0;

    int numclothes = 0;
    char clothes[10][256];
    float clothestintr[10];
    float clothestintg[10];
    float clothestintb[10];

    bool landhard = 0;
    bool bled = 0;
    bool spurt = 0;
    bool onfire = 0;
    float onfiredelay = 0;
    float burnt = 0;
    float fireduration = 0;

    float flamedelay = 0;
    float updatestuffdelay = 0;

    int playerdetail = 0;

    int num_weapons = 0;
    int weaponids[4];
    int weaponactive = 0;
    int weaponstuck = 0;
    int weaponstuckwhere = 0;
    int weaponwhere = 0;

    int numwaypoints = 0;
    XYZ waypoints[90];
    int waypointtype[90];
    float pausetime = 0;
    bool hastempwaypoint = 0;
    XYZ tempwaypoint;

    XYZ headtarget;
    float interestdelay = 0;

    XYZ finalfinaltarget;
    XYZ finaltarget;
    int finalpathfindpoint = 0;
    int targetpathfindpoint = 0;
    int lastpathfindpoint = 0;
    int lastpathfindpoint2 = 0;
    int lastpathfindpoint3 = 0;
    int lastpathfindpoint4 = 0;
    bool onpath = 0;

    int waypoint = 0;
    bool jumppath = 0;

    XYZ lastseen;
    float lastseentime = 0;
    float lastchecktime = 0;
    float stunned = 0;
    float surprised = 0;
    float runninghowlong = 0;
    int lastoccluded = 0;
    int laststanding = 0;
    int escapednum = 0;

    float speechdelay = 0;
    float neckspurtdelay = 0;
    float neckspurtparticledelay = 0;
    float neckspurtamount = 0;

    int whichskin = 0;
    bool rabbitkickragdoll = 0;

    XYZ averageloc;
    XYZ oldaverageloc;

    Animation tempanimation;

    int occluded = 0;

    bool jumpclimb = 0;



    // convenience functions
    inline Joint& joint(int bodypart) { return skeleton.joints[skeleton.jointlabels[bodypart]]; }
    inline XYZ& jointPos(int bodypart) { return joint(bodypart).position; }
    inline XYZ& jointVel(int bodypart) { return joint(bodypart).velocity; }

    Person();

    void CheckKick();
    void CatchFire();
    void DoBlood(float howmuch, int which);
    void DoBloodBig(float howmuch, int which);
    bool DoBloodBigWhere(float howmuch, int which, XYZ where);

    bool wasIdle() { return animation_bits[animCurrent] & ab_idle; }
    bool isIdle() { return animation_bits[animTarget] & ab_idle; }
    virtual int getIdle();

    bool isSitting() { return animation_bits[animTarget] & ab_sit; }

    bool isSleeping() { return animation_bits[animTarget] & ab_sleep; }

    bool wasCrouch() { return animation_bits[animCurrent] & ab_crouch; }
    bool isCrouch() { return animation_bits[animTarget] & ab_crouch; }
    virtual int getCrouch();

    bool wasStop() { return animation_bits[animCurrent] & ab_stop; }
    bool isStop() { return animation_bits[animTarget] & ab_stop; }
    virtual int getStop();

    bool wasRun() { return animation_bits[animCurrent] & ab_run; }
    bool isRun() { return animation_bits[animTarget] & ab_run; }
    virtual int getRun();

    bool wasLanding() { return animation_bits[animCurrent] & ab_land; }
    bool isLanding() { return animation_bits[animTarget] & ab_land; }
    virtual int getLanding();

    bool wasLandhard() { return animation_bits[animCurrent] & ab_landhard; }
    bool isLandhard() { return animation_bits[animTarget] & ab_landhard; }
    virtual int getLandhard();

    bool wasFlip() { return animation_bits[animCurrent] & ab_flip; }
    bool isFlip() { return animation_bits[animTarget] & ab_flip; }

    bool isWallJump() { return animation_bits[animTarget] & ab_walljump; }
    void Reverse();
    void DoDamage(float howmuch);
    void DoHead();
    void DoMipmaps() {
        skeleton.drawmodel.textureptr.bind();
        glTexParameteri(GL_TEXTURE_2D, GL_GENERATE_MIPMAP, GL_TRUE);
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, skeleton.skinsize, skeleton.skinsize, 0, GL_RGB, GL_UNSIGNED_BYTE, &skeleton.skinText[0]);
    }

    int SphereCheck(XYZ *p1, float radius, XYZ *p, XYZ *move, float *rotate, Model *model);
    int DrawSkeleton();
    void Puff(int whichlabel);
    void FootLand(int which, float opacity);
    void DoStuff();
    void setAnimation(int);
    void DoAnimations();
    void RagDoll(bool checkcollision);

    virtual void setProps(float headprop, float bodyprop, float armprop, float legprop) = 0;
    virtual void loadSkeleton() = 0;
};

const int maxplayers = 10;

#endif
