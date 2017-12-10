/*
Copyright (C) 2003, 2010 - Wolfire Games
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

#include "Graphic/Stereo.hpp"
#include "Math/XYZ.hpp"
#include "Objects/Weapons.hpp"

#include <SDL.h>
#include <string>

bool visibleloading = false;

float volume = 0;
bool ismotionblur = false;
float usermousesensitivity = 0;
bool floatjump = false;
bool cellophane = false;
bool autoslomo = false;
bool decalstoggle = false;
bool invertmouse = false;
bool texttoggle = false;
float blurness = 0;
float targetblurness = 0;
float windvar = 0;
float precipdelay = 0;
float gamespeed = 0;
float oldgamespeed = 0;
int difficulty = 0;
float multiplier = 0;
float realmultiplier = 0;
float screenwidth = 0, screenheight = 0;
float minscreenwidth = 640, minscreenheight = 480;
float maxscreenwidth = 3000, maxscreenheight = 3000;
bool fullscreen = 0;
float viewdistance = 0;
XYZ viewer;
XYZ viewerfacing;
float fadestart = 0;
int environment = 0;
float texscale = 0;
float gravity = 0;
Light light;
Terrain terrain;

SDL_Window* sdlwindow;

int kTextureSize = 0;
int detail = 0;
FRUSTUM frustum;
float texdetail = 0;
float realtexdetail = 0;
float playerdist = 0;
int slomo = 0;
float slomodelay = 0;
int bloodtoggle = 0;
float camerashake = 0;
float woozy = 0;
float blackout = 0;
bool foliage = false;
bool musictoggle = false;
bool trilinear;
Weapons weapons;
bool damageeffects = false;
bool ambientsound = false;
bool mousejump = false;
bool freeze = false;
bool winfreeze = false;
float flashamount = 0, flashr = 0, flashg = 0, flashb = 0;
int flashdelay = 0;
float motionbluramount = 0;
bool stillloading = false;
bool showpoints = false;
bool showdamagebar = false;
bool alwaysblur = false;
bool immediate = false;
bool velocityblur = false;
XYZ windvector;
int mainmenu = 0;
int whichjointstartarray[26] = { 0 };
int whichjointendarray[26] = { 0 };

float smoketex = 0;

float slomospeed = 0;
float slomofreq = 0;

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

XYZ envsound[30];
float envsoundvol[30] = { 0 };
float envsoundlife[30] = { 0 };
int numenvsounds;

bool devtools = false;

bool gamestarted = false;

StereoMode stereomode = stereoNone;
StereoMode newstereomode = stereoNone;
float stereoseparation = 0.05;
bool stereoreverse = false;
