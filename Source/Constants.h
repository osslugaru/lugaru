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

#ifndef _CONSTANTS_H_
#define _CONSTANTS_H_

// cat src/Constants.h | sed -e 's/#define/const int/' -e 's/ [0-9]*$/ =&;/'
// chew on that --Jookia

const int awardklutz = 0;
const int awardflawless = 1;
const int awardalldead = 2;
const int awardnodead = 3;
const int awardstealth = 4;
const int awardswordsman = 5;
const int awardkungfu = 6;
const int awardknifefighter = 7;
const int awardcoward = 8;
const int awardevasion = 9;
const int awardacrobat = 10;
const int awardlongrange = 11;
const int awardbrutal = 12;
const int awardhyper = 13;
const int awardaikido = 14;
const int awardrambo = 15;
const int awardfast = 16;
const int awardrealfast = 17;
const int awarddamnfast = 18;
const int awardstrategy = 19;
const int awardbojutsu = 20;

const int mapkilleveryone = 0;
const int mapgosomewhere = 1;
const int mapkillsomeone = 2;
const int mapkillmost = 3;

enum pathtypes {wpkeepwalking, wppause};

static const char *pathtypenames[] = {"keepwalking", "pause"};

enum editortypes {typeactive, typesitting, typesittingwall, typesleeping,
		  typedead1, typedead2, typedead3, typedead4};

static const char *editortypenames[] = {
  "active", "sitting", "sitting wall", "sleeping",
  "dead1", "dead2", "dead3", "dead4"
};

const int tracheotomy = 1;
const int backstab = 2;
const int spinecrusher = 3;
const int ninja = 4;
const int style = 5;
const int cannon = 6;
const int aimbonus = 7;
const int deepimpact = 8;
const int touchofdeath = 9;
const int swordreversebonus = 10;
const int staffreversebonus = 11;
const int reverseko = 12;
const int solidhit = 13;
const int twoxcombo = 14;
const int threexcombo = 15;
const int fourxcombo = 16;
const int megacombo = 17;
const int Reversal = 18;
const int Stabbonus = 19;
const int Slicebonus = 20;
const int Bullseyebonus = 21;
const int Slashbonus = 22;
const int Wolfbonus = 23;
const int FinishedBonus = 24;
const int TackleBonus = 25;
const int AboveBonus = 26;

const int boneconnect = 0;
const int constraint = 1;
const int muscle = 2;

const int head = 0; // DO NOT CONFLICT WITH ZLIB's variable head
const int neck = 1;
const int leftshoulder = 2;
const int leftelbow = 3;
const int leftwrist = 4;
const int lefthand = 5;
const int rightshoulder = 6;
const int rightelbow = 7;
const int rightwrist = 8;
const int righthand = 9;
const int abdomen = 10;
const int lefthip = 11;
const int righthip = 12;
const int groin = 13;
const int leftknee = 14;
const int leftankle = 15;
const int leftfoot = 16;
const int rightknee = 17;
const int rightankle = 18;
const int rightfoot = 19;

const int max_joints = 50;
const int max_frames = 50;
const int max_muscles = 100;

const int animation_count = 140;

const int runanim = 0;
const int bounceidleanim = 1;
const int stopanim = 2;
const int jumpupanim = 3;
const int jumpdownanim = 4;
const int landanim = 5;
const int climbanim = 6;
const int hanganim = 7;
const int spinkickanim = 8;
const int tempanim = 9;
const int getupfromfrontanim = 10;
const int getupfrombackanim = 11;
const int crouchanim = 12;
const int sneakanim = 13;
const int rollanim = 14;
const int flipanim = 15;
const int spinkickreversedanim = 16;
const int spinkickreversalanim = 17;
const int lowkickanim = 18;
const int sweepanim = 19;
const int sweepreversedanim = 20;
const int sweepreversalanim = 21;
const int rabbitkickanim = 22;
const int rabbitkickreversedanim = 23;
const int rabbitkickreversalanim = 24;
const int upunchanim = 25;
const int staggerbackhighanim = 26;
const int upunchreversedanim = 27;
const int upunchreversalanim = 28;
const int hurtidleanim = 29;
const int backhandspringanim = 30;
const int fightidleanim = 31;
const int walkanim = 32;
const int fightsidestep = 33;
const int killanim = 34;
const int sneakattackanim = 35;
const int sneakattackedanim = 36;
const int drawrightanim = 37;
const int knifeslashstartanim = 38;
const int crouchstabanim = 39;
const int crouchdrawrightanim = 40;
const int knifefollowanim = 41;
const int knifefollowedanim = 42;
const int knifethrowanim = 43;
const int removeknifeanim = 44;
const int crouchremoveknifeanim = 45;
const int jumpreversedanim = 46;
const int jumpreversalanim = 47;
const int landhardanim = 48;
const int staggerbackhardanim = 49;
const int dropkickanim = 50;
const int winduppunchanim = 51;
const int winduppunchblockedanim = 52;
const int blockhighleftanim = 53;
const int blockhighleftstrikeanim = 54;
const int walljumpfrontanim = 55;
const int walljumpbackanim = 56;
const int walljumpleftanim = 57;
const int walljumprightanim = 58;
const int backflipanim = 59;
const int leftflipanim = 60;
const int rightflipanim = 61;
const int walljumprightkickanim = 62;
const int walljumpleftkickanim = 63;
const int knifefightidleanim = 64;
const int knifesneakattackanim = 65;
const int knifesneakattackedanim = 66;
const int swordstabanim = 67;
const int swordslashleftanim = 68;
const int swordslashrightanim = 69;
const int swordfightidleanim = 70;
const int swordsneakattackanim = 71;
const int swordsneakattackedanim = 72;
const int drawleftanim = 73;
const int swordslashanim = 74;
const int swordgroundstabanim = 75;
const int dodgebackanim = 76;
const int swordslashreversedanim = 77;
const int swordslashreversalanim = 78;
const int knifeslashreversedanim = 79;
const int knifeslashreversalanim = 80;
const int swordfightidlebothanim = 81;
const int swordslashparryanim = 82;
const int sworddisarmanim = 83;
const int swordslashparriedanim = 84;
const int wolfidle = 85;
const int wolffightidle = 86;
const int wolfswordidle = 87;
const int wolfhurtidle = 88;
const int wolfcrouchanim = 89;
const int wolfsneakanim = 90;
const int wolfrunanim = 91;
const int wolfstopanim = 92;
const int wolfclawanim = 93;
const int wolflandanim = 94;
const int wolflandhardanim = 95;
const int wolfrunninganim = 96;
const int rabbitrunninganim = 97;
const int frontflipanim = 98;
const int rabbittackleanim = 99;
const int rabbittacklinganim = 100;
const int rabbittackledfrontanim = 101;
const int rabbittackledbackanim = 102;
const int rabbittacklereversal = 103;
const int rabbittacklereversed = 104;
const int wolftackleanim = 105;
const int wolftacklinganim = 106;
const int wolftackledfrontanim = 107;
const int wolftackledbacanim = 108;
const int wolftacklereversal = 109;
const int wolftacklereversed = 110;
const int wolfslapanim = 111;
const int wolfbashanim = 112;
const int staffhitanim = 113;
const int staffgroundsmashanim = 114;
const int staffspinhitanim = 115;
const int staffhitreversedanim = 116;
const int staffhitreversalanim = 117;
const int staffspinhitreversedanim = 118;
const int staffspinhitreversalanim = 119;
const int sleepanim = 120;
const int sitanim = 121;
const int talkidleanim = 122;
const int sitwallanim = 123;
const int dead1anim = 124;
const int dead2anim = 125;
const int dead3anim = 126;
const int dead4anim = 127;

const int max_dialogues = 20;
const int max_dialoguelength = 20;

const int max_model_vertex = 3000;		// maximum number of vertexs
const int max_textured_triangle = 3000;		// maximum number of texture-filled triangles in a model

const int stream_music1desert = 0;
const int stream_music1grass = 1;
const int stream_music1snow = 2;
const int stream_music2 = 3;
const int stream_music3 = 4;
const int stream_music4 = 5;
const int stream_menumusic = 6;
const int stream_desertambient = 7;
const int stream_firesound = 8;
const int stream_wind = 9;

//const int music1desert = 0;
//const int music1grass = 1;
//const int music1snow = 2;
//const int music2 = 3;
//const int music3 = 4;
//const int music4 = 5;
//const int menumusic = 6;
//const int desertambient = 7;
//const int firesound = 8;
//const int wind = 9;
const int footstepsound = 10;
const int footstepsound2 = 11;
const int footstepsound3 = 12;
const int footstepsound4 = 13;
const int jumpsound = 14;
const int landsound = 15;
const int whooshsound = 16;
const int hawksound = 17;
const int landsound1 = 18;
const int landsound2 = 19;
const int breaksound = 20;
const int lowwhooshsound = 21;
const int heavyimpactsound = 22;
const int firestartsound = 23;
const int fireendsound = 24;
const int breaksound2 = 25;
const int knifedrawsound = 26;
const int knifesheathesound = 27;
const int knifeswishsound = 28;
const int knifeslicesound = 29;
const int skidsound = 30;
const int snowskidsound = 31;
const int bushrustle = 32;
const int midwhooshsound = 33;
const int highwhooshsound = 34;
const int movewhooshsound = 35;
const int thudsound = 36;
const int whooshhitsound = 37;
const int clank1sound = 38;
const int clank2sound = 39;
const int clank3sound = 40;
const int clank4sound = 41;
const int consolefailsound = 42;
const int consolesuccesssound = 43;
const int swordslicesound = 44;
const int metalhitsound = 45;
const int clawslicesound = 46;
const int splattersound = 47;
const int growlsound = 48;
const int growl2sound = 49;
const int barksound = 50;
const int snarlsound = 51;
const int snarl2sound = 52;
const int barkgrowlsound = 53;
const int bark2sound = 54;
const int bark3sound = 55;
const int rabbitattacksound = 56;
const int rabbitattack2sound = 57;
const int rabbitattack3sound = 58;
const int rabbitattack4sound = 59;
const int rabbitpainsound = 60;
const int rabbitpain1sound = 61;
const int rabbitpain2sound = 62;
const int rabbitchitter = 63;
const int rabbitchitter2 = 64;
const int fleshstabsound = 65;
const int fleshstabremovesound = 66;
const int swordstaffsound = 67;
const int staffbodysound = 68;
const int staffheadsound = 69;
const int alarmsound = 70;
const int staffbreaksound = 71;

const int normalmode = 0;
const int motionblurmode = 1;
const int radialzoommode = 2;
const int realmotionblurmode = 3;
const int doublevisionmode = 4;
const int glowmode = 5;

const int maxplayers = 10;
#endif
