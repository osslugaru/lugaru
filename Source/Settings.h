#ifndef SETTINGS_H_
#define SETTINGS_H_

#include "Game.h"

extern float usermousesensitivity;
extern bool ismotionblur;
extern bool floatjump;
extern bool mousejump;
extern bool ambientsound;
extern int bloodtoggle;
extern bool autoslomo;
extern bool foliage;
extern bool musictoggle;
extern bool trilinear;
extern bool decals;
extern bool invertmouse;
extern float gamespeed;
extern float oldgamespeed;
extern int difficulty;
extern bool damageeffects;
extern bool texttoggle;
extern bool debugmode;
extern bool vblsync;
extern bool showpoints;
extern bool showdamagebar;
extern bool alwaysblur;
extern bool immediate;
extern bool velocityblur;
extern float volume;
extern int detail;
extern int kBitsPerPixel;
extern int kContextWidth;
extern int kContextHeight;
extern float screenwidth,screenheight;

void DefaultSettings();
void SaveSettings();
bool LoadSettings();


#endif
