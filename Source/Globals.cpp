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

#include "SDL.h"

#include "gamegl.h"
#include "Quaternions.h"
#include "Lights.h"
#include "Skeleton.h"
#include "Terrain.h"
#include "Sprite.h"
#include "Frustum.h"
#include "Objects.h"
#include "Weapons.h"
#include "Person.h"
#include "TGALoader.h"
#include "openal_wrapper.h"
#include "Stereo.h"

#include "Animation.h"

bool visibleloading = 0;

float volume = 0;
bool ismotionblur = false;
float usermousesensitivity = 0;
bool floatjump = false;
bool cellophane = false;
bool autoslomo = false;
bool decals = false;
bool invertmouse = false;
bool texttoggle = false;
float blurness = 0;
float targetblurness = 0;
float windvar = 0;
float precipdelay = 0;
float gamespeed = 0;
float oldgamespeed = 0;
float tintr = 0,tintg = 0,tintb = 0;
int difficulty = 0;
float multiplier = 0;
float realmultiplier = 0;
float screenwidth = 0,screenheight = 0;
float viewdistance = 0;
XYZ viewer;
XYZ viewerfacing;
XYZ lightlocation;
float fadestart = 0;
int environment = 0;
float texscale = 0;
float gravity = 0;
Light light;
Animation animation[animation_count];
Skeleton testskeleton;
int numsounds = 0;
Terrain terrain;
float sps = 0;

SDL_Surface *sdlscreen;

int kTextureSize = 0;
int detail = 0;
FRUSTUM frustum;
float texdetail = 0;
float realtexdetail = 0;
float playerdist = 0;
Objects objects;
int slomo = 0;
float slomodelay = 0;
GLubyte bloodText[512*512*3] = {0};
GLubyte wolfbloodText[512*512*3] = {0};
float colors[3] = {0};
int bloodtoggle = 0;
bool osx = false;
float camerashake = 0;
float woozy = 0;
float blackout = 0;
bool foliage = false;
bool musictoggle = false;
bool trilinear;
Weapons weapons;
bool damageeffects = false;
int numplayers = 0;
bool ambientsound = false;
bool mousejump = false;
bool freeze = false;
bool winfreeze = false;
float flashamount = 0,flashr = 0,flashg = 0,flashb = 0;
int flashdelay = 0;
bool vblsync = false;
float motionbluramount = 0;
bool keyboardfrozen = false;
bool loadingstuff = false;
bool stillloading = false;
bool showpoints = false;
bool showdamagebar = false;
bool alwaysblur = false;
bool immediate = false;
bool velocityblur = false;
int test = 0;
XYZ windvector;
short vRefNum = 0;
long dirID = 0;
int mainmenu = 0;
int whichjointstartarray[26] = {0};
int whichjointendarray[26] = {0};
int kBitsPerPixel = 0;

int numhotspots = 0;
XYZ hotspot[40];
int hotspottype[40] = {0};
float hotspotsize[40] = {0};
char hotspottext[40][256] = {0};
int currenthotspot = 0;	
int killhotspot = 0;

float menupulse = 0;

float smoketex = 0;

float slomospeed = 0;
float slomofreq = 0;

int tutoriallevel = 0;
int tutorialstage = 0;
float tutorialstagetime = 0;
float tutorialmaxtime = 0;
float tutorialsuccess = 0;

bool againbonus = false;

float damagedealt = 0;

int maptype = 0;

int editoractive = 0;
int editorpathtype = 0;

bool reversaltrain = false;
bool cananger = false;
bool canattack = false;

bool skyboxtexture = false;
float skyboxr = 0;
float skyboxg = 0;
float skyboxb = 0;
float skyboxlightr = 0;
float skyboxlightg = 0;
float skyboxlightb = 0;

int hostile = 0;
float hostiletime = 0;

XYZ envsound[30]; // = {0};
float envsoundvol[30] = {0};
float envsoundlife[30] = {0};
int numenvsounds;


bool tilt2weird = false;
bool tiltweird = false;
bool midweird = false;
bool proportionweird = false;
bool vertexweird[6] = {0};
TGAImageRec texture;
bool debugmode = false;

bool won = false;


bool campaign = false;

bool gamestarted = false;

//TextureList textures;

StereoMode stereomode =  stereoNone;
StereoMode newstereomode = stereoNone;
float stereoseparation = 0.05;
bool  stereoreverse = false;
