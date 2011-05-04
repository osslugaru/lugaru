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

#if PLATFORM_UNIX
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>
#else
#include <direct.h>
#endif

#include <ctime>
#include "Game.h"
#include "openal_wrapper.h"
#include "Settings.h"
#include "Input.h"
#include "Animation.h"
#include "Awards.h"

using namespace std;

// Added more evilness needed for MSVC
#ifdef _MSC_VER
	#define strncasecmp(s1, s2, n) _strnicmp(s1, s2, n)
	#define snprintf(buf, size, format, ...) _sprintf_p(buf, size, format)
#endif


extern float multiplier;
extern XYZ viewer;
extern int environment;
extern Terrain terrain;
extern float screenwidth,screenheight;
extern float gravity;
extern int detail;
extern float texdetail;
extern Objects objects;
extern int slomo;
extern float slomodelay;
extern bool floatjump;
extern float volume;
extern Light light;
extern float camerashake;
extern float woozy;
extern float blackout;
extern bool cellophane;
extern bool musictoggle;
extern int difficulty;
extern int bloodtoggle;
extern bool invertmouse;
extern float windvar;
extern float precipdelay;
extern XYZ viewerfacing;
extern bool ambientsound;
extern bool mousejump;
extern float viewdistance;
extern bool freeze;
extern bool keyboardfrozen;
extern bool loadingstuff;
extern XYZ windvector;
extern bool debugmode;
static int music1;
extern int mainmenu;
extern bool visibleloading;
extern int loadscreencolor;
extern XYZ envsound[30];
extern float envsoundvol[30];
extern int numenvsounds;
extern float envsoundlife[30];
extern float usermousesensitivity;
extern bool ismotionblur;
extern bool showdamagebar; // (des)activate the damage bar
extern bool decals;
extern float tintr,tintg,tintb;
extern bool skyboxtexture;
extern float skyboxr;
extern float skyboxg;
extern float skyboxb;
extern float skyboxlightr;
extern float skyboxlightg;
extern float skyboxlightb;
extern float fadestart;
extern float slomospeed;
extern float slomofreq;
extern int tutoriallevel;
extern float smoketex;
extern float tutorialstagetime;
extern int tutorialstage;
extern float tutorialmaxtime;
extern float tutorialsuccess;
extern bool againbonus;
extern bool reversaltrain;
extern bool canattack;
extern bool cananger;
extern float damagedealt;
extern int maptype;
extern int editoractive;
extern int editorpathtype;

extern float hostiletime;

extern bool gamestarted;

extern int numhotspots;
extern int winhotspot;
extern int windialogue;
extern int killhotspot;
extern XYZ hotspot[40];
extern int hotspottype[40];
extern float hotspotsize[40];
extern char hotspottext[40][256];
extern int currenthotspot;

extern int hostile;

extern bool stillloading;
extern bool winfreeze;

extern bool campaign;

static const char *rabbitskin[] = {
":Data:Textures:Fur3.jpg",
":Data:Textures:Fur.jpg",
":Data:Textures:Fur2.jpg",
":Data:Textures:Lynx.jpg",
":Data:Textures:Otter.jpg",
":Data:Textures:Opal.jpg",
":Data:Textures:Sable.jpg",
":Data:Textures:Chocolate.jpg",
":Data:Textures:BW2.jpg",
":Data:Textures:WB2.jpg"
};

static const char *wolfskin[] = {
":Data:Textures:Wolf.jpg",
":Data:Textures:Darkwolf.jpg",
":Data:Textures:Snowwolf.jpg"
};

#define STATIC_ASSERT(x) extern int s_a_dummy[2 * (!!(x)) - 1];
STATIC_ASSERT (rabbittype == 0 && wolftype == 1)

static const char **creatureskin[] = {rabbitskin, wolfskin};

/* Return true if PFX is a prefix of STR (case-insensitive).  */
static bool stripfx(const char *str, const char *pfx)
{
  return !strncasecmp(str, pfx, strlen(pfx));
}

static const char *cmd_names[] = {
#define DECLARE_COMMAND(cmd) #cmd " ",
#include "ConsoleCmds.h"
#undef  DECLARE_COMMAND
};

typedef void (*console_handler)(Game *game, const char *args);

#define DECLARE_COMMAND(cmd) static void ch_##cmd(Game *game, const char *args);
#include "ConsoleCmds.h"
#undef  DECLARE_COMMAND

static console_handler cmd_handlers[] = {
#define DECLARE_COMMAND(cmd) ch_##cmd,
#include "ConsoleCmds.h"
#undef  DECLARE_COMMAND
};



// added utility functions -sf17k =============================================================

//TODO: try to hide these variables completely with a better interface
inline void setAnimation(int playerid,int animation){
    player[playerid].targetanimation=animation;
    player[playerid].targetframe=0;
    player[playerid].target=0;
}

//TODO: this is incorrect but I'm afraid to change it and break something,
//probably causes quirky behavior that I might want to preserve
inline float roughDirection(XYZ vec){
    Normalise(&vec);
    float angle=-asin(-vec.x)*180/M_PI;
    if(vec.z<0)
        angle=180-angle;
    return angle;
}
inline float roughDirectionTo(XYZ start, XYZ end){
    return roughDirection(end-start);
}

//TODO: gotta be a better way
inline float pitch(XYZ vec){
    Normalise(&vec);
    return -asin(vec.y)*180/M_PI;
}
inline float pitchTo(XYZ start, XYZ end){
    return pitch(end-start);
}

//change these to a Person method
inline Joint& playerJoint(int playerid, int bodypart){
    return player[playerid].skeleton.joints[player[playerid].skeleton.jointlabels[bodypart]]; }
inline Joint& playerJoint(Person* pplayer, int bodypart){
    return pplayer->skeleton.joints[pplayer->skeleton.jointlabels[bodypart]]; }

inline float sq(float n){ return n*n; }

inline float stepTowardf(float from, float to, float by){
    if(fabs(from-to)<by) return to;
    else if(from>to) return from-by;
    else return from+by;
}

void playdialogueboxsound(){
    XYZ temppos;
    temppos=player[participantfocus[whichdialogue][indialogue]].coords;
    temppos=temppos-viewer;
    Normalise(&temppos);
    temppos+=viewer;

    int sound=-1;
    switch(dialogueboxsound[whichdialogue][indialogue]){
        case -6: sound=alarmsound; break;
        case -4: sound=consolefailsound; break;
        case -3: sound=consolesuccesssound; break;
        case -2: sound=firestartsound; break;
        case -1: sound=fireendsound; break;
        case 1: sound=rabbitchitter; break;
        case 2: sound=rabbitchitter2; break;
        case 3: sound=rabbitpainsound; break;
        case 4: sound=rabbitpain1sound; break;
        case 5: sound=rabbitattacksound; break;
        case 6: sound=rabbitattack2sound; break;
        case 7: sound=rabbitattack3sound; break;
        case 8: sound=rabbitattack4sound; break;
        case 9: sound=growlsound; break;
        case 10: sound=growl2sound; break;
        case 11: sound=snarlsound; break;
        case 12: sound=snarl2sound; break;
        case 13: sound=barksound; break;
        case 14: sound=bark2sound; break;
        case 15: sound=bark3sound; break;
        case 16: sound=barkgrowlsound; break;
        default: break;
    }
    if(sound!=-1)
        emit_sound_at(sound, temppos);
}

// end added utility functions ================================================================



static void ch_quit(Game *game, const char *args)
{
  game->tryquit = 1;
}

static void ch_map(Game *game, const char *args)
{
  game->Loadlevel(args);
  game->whichlevel = -2;
  campaign = 0;
}

static void ch_save(Game *game, const char *args)
{
  char buf[64];
  int i, j, k, l, m, templength;
  float headprop, bodyprop, armprop, legprop;
  snprintf(buf, 63, ":Data:Maps:%s", args);


  int mapvers = 12;;

  FILE			*tfile;
  tfile=fopen( ConvertFileName(buf), "wb" );
  fpackf(tfile, "Bi", mapvers);
  fpackf(tfile, "Bi", maptype);
  fpackf(tfile, "Bi", hostile);
  fpackf(tfile, "Bf Bf", viewdistance, fadestart);
  fpackf(tfile, "Bb Bf Bf Bf", skyboxtexture, skyboxr, skyboxg, skyboxb);
  fpackf(tfile, "Bf Bf Bf", skyboxlightr, skyboxlightg, skyboxlightb);
  fpackf(tfile, "Bf Bf Bf Bf Bf Bi", player[0].coords.x, player[0].coords.y, player[0].coords.z, player[0].rotation, player[0].targetrotation, player[0].num_weapons);
  if(player[0].num_weapons>0&&player[0].num_weapons<5)
    for(int j=0;j<player[0].num_weapons;j++){
      fpackf(tfile, "Bi", weapons.type[player[0].weaponids[j]]);
    }

  fpackf(tfile, "Bf Bf Bf", player[0].armorhead, player[0].armorhigh, player[0].armorlow);
  fpackf(tfile, "Bf Bf Bf", player[0].protectionhead, player[0].protectionhigh, player[0].protectionlow);
  fpackf(tfile, "Bf Bf Bf", player[0].metalhead, player[0].metalhigh, player[0].metallow);
  fpackf(tfile, "Bf Bf", player[0].power, player[0].speedmult);

  fpackf(tfile, "Bi", player[0].numclothes);

  fpackf(tfile, "Bi Bi", player[0].whichskin, player[0].creature);

  fpackf(tfile, "Bi", numdialogues);
	for(int k=0;k<numdialogues;k++){
		fpackf(tfile, "Bi", numdialogueboxes[k]);
		fpackf(tfile, "Bi", dialoguetype[k]);
		for(int l=0;l<10;l++){
			fpackf(tfile, "Bf Bf Bf", participantlocation[k][l].x, participantlocation[k][l].y, participantlocation[k][l].z);
			fpackf(tfile, "Bf", participantrotation[k][l]);
		}
		for(int l=0;l<numdialogueboxes[k];l++){
			fpackf(tfile, "Bi", dialogueboxlocation[k][l]);
			fpackf(tfile, "Bf", dialogueboxcolor[k][l][0]);
			fpackf(tfile, "Bf", dialogueboxcolor[k][l][1]);
			fpackf(tfile, "Bf", dialogueboxcolor[k][l][2]);
			fpackf(tfile, "Bi", dialogueboxsound[k][l]);

			templength=strlen(dialoguetext[k][l]);
			fpackf(tfile, "Bi",(templength));
			for(int m=0;m<templength;m++){
				fpackf(tfile, "Bb", dialoguetext[k][l][m]);
				if(dialoguetext[k][l][m]=='\0')break;
			}

			templength=strlen(dialoguename[k][l]);
			fpackf(tfile, "Bi",templength);
			for(int m=0;m<templength;m++){
				fpackf(tfile, "Bb", dialoguename[k][l][m]);
				if(dialoguename[k][l][m]=='\0')break;
			}

			fpackf(tfile, "Bf Bf Bf", dialoguecamera[k][l].x, dialoguecamera[k][l].y, dialoguecamera[k][l].z);
			fpackf(tfile, "Bi", participantfocus[k][l]);
			fpackf(tfile, "Bi", participantaction[k][l]);

			for(int m=0;m<10;m++)
				fpackf(tfile, "Bf Bf Bf", participantfacing[k][l][m].x, participantfacing[k][l][m].y, participantfacing[k][l][m].z);

			fpackf(tfile, "Bf Bf",dialoguecamerarotation[k][l],dialoguecamerarotation2[k][l]);
		}
	}

	for(int k=0;k<player[0].numclothes;k++){
		templength=strlen(player[0].clothes[k]);
		fpackf(tfile, "Bi", templength);
		for(int l=0;l<templength;l++)
			fpackf(tfile, "Bb", player[0].clothes[k][l]);
		fpackf(tfile, "Bf Bf Bf", player[0].clothestintr[k], player[0].clothestintg[k], player[0].clothestintb[k]);
	}

  fpackf(tfile, "Bi", environment);

  fpackf(tfile, "Bi", objects.numobjects);

    for(int k=0;k<objects.numobjects;k++){
      fpackf(tfile, "Bi Bf Bf Bf Bf Bf Bf", objects.type[k], objects.rotation[k], objects.rotation2[k], objects.position[k].x, objects.position[k].y, objects.position[k].z, objects.scale[k]);
    }

  fpackf(tfile, "Bi", numhotspots);
	for(int i=0;i<numhotspots;i++){
		fpackf(tfile, "Bi Bf Bf Bf Bf", hotspottype[i],hotspotsize[i],hotspot[i].x,hotspot[i].y,hotspot[i].z);
		templength=strlen(hotspottext[i]);
		fpackf(tfile, "Bi",templength);
		for(int l=0;l<templength;l++)
			fpackf(tfile, "Bb", hotspottext[i][l]);
	}

  fpackf(tfile, "Bi", numplayers);
  if(numplayers<maxplayers)
    for(int j=1;j<numplayers;j++){
		fpackf(tfile, "Bi Bi Bf Bf Bf Bi Bi Bf Bb Bf", player[j].whichskin, player[j].creature, player[j].coords.x, player[j].coords.y, player[j].coords.z, player[j].num_weapons, player[j].howactive, player[j].scale, player[j].immobile, player[j].rotation);
		if(player[j].num_weapons<5)
			for(int k=0;k<player[j].num_weapons;k++){
				fpackf(tfile, "Bi", weapons.type[player[j].weaponids[k]]);
			}
		if(player[j].numwaypoints<30){
			fpackf(tfile, "Bi", player[j].numwaypoints);
			for(int k=0;k<player[j].numwaypoints;k++){
				fpackf(tfile, "Bf", player[j].waypoints[k].x);
				fpackf(tfile, "Bf", player[j].waypoints[k].y);
				fpackf(tfile, "Bf", player[j].waypoints[k].z);
				fpackf(tfile, "Bi", player[j].waypointtype[k]);
			}
			fpackf(tfile, "Bi", player[j].waypoint);
		} else {
			player[j].numwaypoints=0;
			player[j].waypoint=0;
			fpackf(tfile, "Bi Bi Bi", player[j].numwaypoints, player[j].waypoint, player[j].waypoint);
		}

		fpackf(tfile, "Bf Bf Bf", player[j].armorhead, player[j].armorhigh, player[j].armorlow);
		fpackf(tfile, "Bf Bf Bf", player[j].protectionhead, player[j].protectionhigh, player[j].protectionlow);
		fpackf(tfile, "Bf Bf Bf", player[j].metalhead, player[j].metalhigh, player[j].metallow);
		fpackf(tfile, "Bf Bf", player[j].power, player[j].speedmult);

		if(player[j].creature==wolftype) {
			headprop=player[j].proportionhead.x/1.1;
			bodyprop=player[j].proportionbody.x/1.1;
			armprop=player[j].proportionarms.x/1.1;
			legprop=player[j].proportionlegs.x/1.1;
		} else if(player[j].creature==rabbittype){
			headprop=player[j].proportionhead.x/1.2;
			bodyprop=player[j].proportionbody.x/1.05;
			armprop=player[j].proportionarms.x/1.00;
			legprop=player[j].proportionlegs.x/1.1;
		}

		fpackf(tfile, "Bf Bf Bf Bf", headprop, bodyprop, armprop, legprop);

		fpackf(tfile, "Bi", player[j].numclothes);
		if(player[j].numclothes)
			for(int k=0;k<player[j].numclothes;k++){
				int templength;
				templength=strlen(player[j].clothes[k]);
				fpackf(tfile, "Bi", templength);
				for(int l=0;l<templength;l++)
					fpackf(tfile, "Bb", player[j].clothes[k][l]);
				fpackf(tfile, "Bf Bf Bf", player[j].clothestintr[k], player[j].clothestintg[k], player[j].clothestintb[k]);
			}
    }

  fpackf(tfile, "Bi", game->numpathpoints);
	for(int j=0;j<game->numpathpoints;j++){
		fpackf(tfile, "Bf Bf Bf Bi", game->pathpoint[j].x, game->pathpoint[j].y, game->pathpoint[j].z, game->numpathpointconnect[j]);
		for(int k=0;k<game->numpathpointconnect[j];k++){
			fpackf(tfile, "Bi", game->pathpointconnect[j][k]);
		}
	}

  fpackf(tfile, "Bf Bf Bf Bf", game->mapcenter.x, game->mapcenter.y, game->mapcenter.z, game->mapradius);

  fclose(tfile);
}

static void ch_cellar(Game *game, const char *args)
{
  game->LoadTextureSave(":Data:Textures:Furdarko.jpg",&player[0].skeleton.drawmodel.textureptr,1,&player[0].skeleton.skinText[0],&player[0].skeleton.skinsize);
}

static void ch_tint(Game *game, const char *args)
{
  sscanf(args, "%f%f%f", &tintr, &tintg, &tintb);
}

static void ch_tintr(Game *game, const char *args)
{
  tintr = atof(args);
}

static void ch_tintg(Game *game, const char *args)
{
  tintg = atof(args);
}

static void ch_tintb(Game *game, const char *args)
{
  tintb = atof(args);
}

static void ch_speed(Game *game, const char *args)
{
  player[0].speedmult = atof(args);
}

static void ch_strength(Game *game, const char *args)
{
  player[0].power = atof(args);
}

static void ch_power(Game *game, const char *args)
{
  player[0].power = atof(args);
}

static void ch_size(Game *game, const char *args)
{
  player[0].scale = atof(args) * .2;
}

static int find_closest()
{
  int closest = 0;
  float closestdist = 1.0/0.0;

  for (int i = 1; i < numplayers; i++) {
    float distance;
    distance = findDistancefast(&player[i].coords,&player[0].coords);
    if (distance < closestdist) {
      closestdist = distance;
      closest = i;
    }
  }
  return closest;
}

static void ch_sizenear(Game *game, const char *args)
{
  int closest = find_closest();

  if (closest)
    player[closest].scale = atof(args) * .2;
}

static void set_proportion(int pnum, const char *args)
{
  float headprop,bodyprop,armprop,legprop;

  sscanf(args, "%f%f%f%f", &headprop, &bodyprop, &armprop, &legprop);

  if(player[pnum].creature==wolftype){
    player[pnum].proportionhead=1.1*headprop;
    player[pnum].proportionbody=1.1*bodyprop;
    player[pnum].proportionarms=1.1*armprop;
    player[pnum].proportionlegs=1.1*legprop;
  } else if(player[pnum].creature==rabbittype){
    player[pnum].proportionhead=1.2*headprop;
    player[pnum].proportionbody=1.05*bodyprop;
    player[pnum].proportionarms=1.00*armprop;
    player[pnum].proportionlegs=1.1*legprop;
    player[pnum].proportionlegs.y=1.05*legprop;
  }
}

static void ch_proportion(Game *game, const char *args)
{
  set_proportion(0, args);
}

static void ch_proportionnear(Game *game, const char *args)
{
  int closest = find_closest();
  if (closest)
    set_proportion(closest, args);
}

static void set_protection(int pnum, const char *args)
{
  float head, high, low;
  sscanf(args, "%f%f%f", &head, &high, &low);

  player[pnum].protectionhead = head;
  player[pnum].protectionhigh = high;
  player[pnum].protectionlow  = low;
}

static void ch_protection(Game *game, const char *args)
{
  set_protection(0, args);
}

static void ch_protectionnear(Game *game, const char *args)
{
  int closest = find_closest();
  if (closest)
    set_protection(closest, args);
}

static void set_armor(int pnum, const char *args)
{
  float head, high, low;
  sscanf(args, "%f%f%f", &head, &high, &low);

  player[pnum].armorhead = head;
  player[pnum].armorhigh = high;
  player[pnum].armorlow  = low;
}

static void ch_armor(Game *game, const char *args)
{
  set_armor(0, args);
}

static void ch_armornear(Game *game, const char *args)
{
  int closest = find_closest();
  if (closest)
    set_armor(closest, args);
}

static void ch_protectionreset(Game *game, const char *args)
{
  set_protection(0, "1 1 1");
  set_armor(0, "1 1 1");
}

static void set_metal(int pnum, const char *args)
{
  float head, high, low;
  sscanf(args, "%f%f%f", &head, &high, &low);

  player[pnum].metalhead = head;
  player[pnum].metalhigh = high;
  player[pnum].metallow  = low;
}

static void ch_metal(Game *game, const char *args)
{
  set_metal(0, args);
}

static void set_noclothes(int pnum, Game *game, const char *args)
{
  player[pnum].numclothes = 0;
  game->LoadTextureSave(creatureskin[player[pnum].creature][player[pnum].whichskin],
			&player[pnum].skeleton.drawmodel.textureptr,1,
			&player[pnum].skeleton.skinText[0],&player[pnum].skeleton.skinsize);
}

static void ch_noclothes(Game *game, const char *args)
{
  set_noclothes(0, game, args);
}

static void ch_noclothesnear(Game *game, const char *args)
{
  int closest = find_closest();
  if (closest)
    set_noclothes(closest, game, args);
}


static void set_clothes(int pnum, Game *game, const char *args)
{
  char buf[64];
  snprintf(buf, 63, ":Data:Textures:%s.png", args);

  if (!game->AddClothes(buf,0,1,&player[pnum].skeleton.skinText[pnum],&player[pnum].skeleton.skinsize))
    return;

  player[pnum].DoMipmaps();
  strcpy(player[pnum].clothes[player[pnum].numclothes],buf);
  player[pnum].clothestintr[player[pnum].numclothes]=tintr;
  player[pnum].clothestintg[player[pnum].numclothes]=tintg;
  player[pnum].clothestintb[player[pnum].numclothes]=tintb;
  player[pnum].numclothes++;
}

static void ch_clothes(Game *game, const char *args)
{
  set_clothes(0, game, args);
}

static void ch_clothesnear(Game *game, const char *args)
{
  int closest = find_closest();
  if (closest)
    set_clothes(closest, game, args);
}

static void ch_belt(Game *game, const char *args)
{
  player[0].skeleton.clothes = !player[0].skeleton.clothes;
}


static void ch_cellophane(Game *game, const char *args)
{
  cellophane = !cellophane;
  float mul = cellophane ? 0 : 1;

  for (int i = 0; i < numplayers; i++) {
    player[i].proportionhead.z = player[i].proportionhead.x * mul;
    player[i].proportionbody.z = player[i].proportionbody.x * mul;
    player[i].proportionarms.z = player[i].proportionarms.x * mul;
    player[i].proportionlegs.z = player[i].proportionlegs.x * mul;
  }
}

static void ch_funnybunny(Game *game, const char *args)
{
  player[0].skeleton.id=0;
  player[0].skeleton.Load(":Data:Skeleton:Basic Figure",":Data:Skeleton:Basic Figurelow",
			  ":Data:Skeleton:Rabbitbelt",":Data:Models:Body.solid",
			  ":Data:Models:Body2.solid",":Data:Models:Body3.solid",
			  ":Data:Models:Body4.solid",":Data:Models:Body5.solid",
			  ":Data:Models:Body6.solid",":Data:Models:Body7.solid",
			  ":Data:Models:Bodylow.solid",":Data:Models:Belt.solid",1);
  game->LoadTextureSave(":Data:Textures:fur3.jpg",&player[0].skeleton.drawmodel.textureptr,1,
			&player[0].skeleton.skinText[0],&player[0].skeleton.skinsize);
  player[0].creature=rabbittype;
  player[0].scale=.2;
  player[0].headless=0;
  player[0].damagetolerance=200;
  set_proportion(0, "1 1 1 1");
}

static void ch_wolfie(Game *game, const char *args)
{
  player[0].skeleton.id=0;
  player[0].skeleton.Load(":Data:Skeleton:Basic Figure Wolf",":Data:Skeleton:Basic Figure Wolf Low",
			  ":Data:Skeleton:Rabbitbelt",":Data:Models:Wolf.solid",
			  ":Data:Models:Wolf2.solid",":Data:Models:Wolf3.solid",
			  ":Data:Models:Wolf4.solid",":Data:Models:Wolf5.solid",
			  ":Data:Models:Wolf6.solid",":Data:Models:Wolf7.solid",
			  ":Data:Models:Wolflow.solid",":Data:Models:Belt.solid",0);
  game->LoadTextureSave(":Data:Textures:Wolf.jpg",&player[0].skeleton.drawmodel.textureptr,1,
			&player[0].skeleton.skinText[0],&player[0].skeleton.skinsize);
  player[0].creature=wolftype;
  player[0].damagetolerance=300;
  set_proportion(0, "1 1 1 1");
}

static void ch_wolfieisgod(Game *game, const char *args)
{
  ch_wolfie(game, args);
}

static void ch_wolf(Game *game, const char *args)
{
  game->LoadTextureSave(":Data:Textures:Wolf.jpg",&player[0].skeleton.drawmodel.textureptr,1,
			&player[0].skeleton.skinText[0],&player[0].skeleton.skinsize);
}

static void ch_snowwolf(Game *game, const char *args)
{
  game->LoadTextureSave(":Data:Textures:SnowWolf.jpg",&player[0].skeleton.drawmodel.textureptr,1,
			&player[0].skeleton.skinText[0],&player[0].skeleton.skinsize);
}

static void ch_darkwolf(Game *game, const char *args)
{
  game->LoadTextureSave(":Data:Textures:DarkWolf.jpg",&player[0].skeleton.drawmodel.textureptr,1,
			&player[0].skeleton.skinText[0],&player[0].skeleton.skinsize);
}

static void ch_lizardwolf(Game *game, const char *args)
{
  game->LoadTextureSave(":Data:Textures:Lizardwolf.jpg",&player[0].skeleton.drawmodel.textureptr,1,
			&player[0].skeleton.skinText[0],&player[0].skeleton.skinsize);
}

static void ch_white(Game *game, const char *args)
{
  game->LoadTextureSave(":Data:Textures:fur.jpg",&player[0].skeleton.drawmodel.textureptr,1,
			&player[0].skeleton.skinText[0],&player[0].skeleton.skinsize);
}

static void ch_brown(Game *game, const char *args)
{
  game->LoadTextureSave(":Data:Textures:fur3.jpg",&player[0].skeleton.drawmodel.textureptr,1,
			&player[0].skeleton.skinText[0],&player[0].skeleton.skinsize);
}

static void ch_black(Game *game, const char *args)
{
  game->LoadTextureSave(":Data:Textures:fur2.jpg",&player[0].skeleton.drawmodel.textureptr,1,
			&player[0].skeleton.skinText[0],&player[0].skeleton.skinsize);
}

static void ch_sizemin(Game *game, const char *args)
{
  for (int i = 1; i < numplayers; i++)
    if (player[i].scale < 0.8 * 0.2)
      player[i].scale = 0.8 * 0.2;
}

static void ch_tutorial(Game *game, const char *args)
{
  tutoriallevel = atoi(args);
}

static void ch_hostile(Game *game, const char *args)
{
  hostile = atoi(args);
}

static void ch_indemo(Game *game, const char *args)
{
  game->indemo=1;
  hotspot[numhotspots]=player[0].coords;
  hotspotsize[numhotspots]=0;
  hotspottype[numhotspots]=-111;
  strcpy(hotspottext[numhotspots],"mapname");
  numhotspots++;
}

static void ch_notindemo(Game *game, const char *args)
{
  game->indemo=0;
  numhotspots--;
}

static void ch_type(Game *game, const char *args)
{
  int n = sizeof(editortypenames) / sizeof(editortypenames[0]);
	for (int i = 0; i < n; i++)
		if (stripfx(args, editortypenames[i])) {
			editoractive = i;
			break;
		}
}

static void ch_path(Game *game, const char *args)
{
  int n = sizeof(pathtypenames) / sizeof(pathtypenames[0]);
  for (int i = 0; i < n; i++)
    if (stripfx(args, pathtypenames[i])) {
		editorpathtype = i;
		break;
    }
}

static void ch_hs(Game *game, const char *args)
{
  hotspot[numhotspots]=player[0].coords;

  float size;
  int type, shift;
  sscanf(args, "%f%d %n", &size, &type, &shift);

  hotspotsize[numhotspots] = size;
  hotspottype[numhotspots] = type;

  strcpy(hotspottext[numhotspots], args + shift);
  strcat(hotspottext[numhotspots], "\n");

  numhotspots++;
}

static void ch_dialogue(Game *game, const char *args)
{
  int dlg;
  char buf1[32], buf2[64];

  sscanf(args, "%d %31s", &dlg, buf1);
  snprintf(buf2, 63, ":Data:Dialogues:%s.txt", buf1);

  dialoguetype[numdialogues] = dlg;

  memset(dialoguetext[numdialogues], 0, sizeof(dialoguetext[numdialogues]));
  memset(dialoguename[numdialogues], 0, sizeof(dialoguename[numdialogues]));

  ifstream ipstream(ConvertFileName(buf2));
  ipstream.ignore(256,':');
  ipstream >> numdialogueboxes[numdialogues];
  for(int i=0;i<numdialogueboxes[numdialogues];i++){
    ipstream.ignore(256,':');
    ipstream.ignore(256,':');
    ipstream.ignore(256,' ');
    ipstream >> dialogueboxlocation[numdialogues][i];
    ipstream.ignore(256,':');
    ipstream >> dialogueboxcolor[numdialogues][i][0];
    ipstream >> dialogueboxcolor[numdialogues][i][1];
    ipstream >> dialogueboxcolor[numdialogues][i][2];
    ipstream.ignore(256,':');
    ipstream.getline(dialoguename[numdialogues][i],64);
    ipstream.ignore(256,':');
    ipstream.ignore(256,' ');
    ipstream.getline(dialoguetext[numdialogues][i],128);
    for(int j=0;j<128;j++){
      if(dialoguetext[numdialogues][i][j]=='\\')dialoguetext[numdialogues][i][j]='\n';
    }
    ipstream.ignore(256,':');
    ipstream >> dialogueboxsound[numdialogues][i];
  }

  for(int i=0;i<numdialogueboxes[numdialogues];i++){
    for(int j=0;j<numplayers;j++){
      participantfacing[numdialogues][i][j]=player[j].facing;
    }
  }
  ipstream.close();

  directing=1;
  indialogue=0;
  whichdialogue=numdialogues;

  numdialogues++;
}

static void ch_fixdialogue(Game *game, const char *args)
{
  char buf1[32], buf2[64];
  int whichdi;

  sscanf(args, "%d %31s", &whichdi, buf1);
  snprintf(buf2, 63, ":Data:Dialogues:%s.txt", buf1);

  memset(dialoguetext[whichdi], 0, sizeof(dialoguetext[whichdi]));
  memset(dialoguename[whichdi], 0, sizeof(dialoguename[whichdi]));

  ifstream ipstream(ConvertFileName(buf2));
  ipstream.ignore(256,':');
  ipstream >> numdialogueboxes[whichdi];
  for(int i=0;i<numdialogueboxes[whichdi];i++){
    ipstream.ignore(256,':');
    ipstream.ignore(256,':');
    ipstream.ignore(256,' ');
    ipstream >> dialogueboxlocation[whichdi][i];
    ipstream.ignore(256,':');
    ipstream >> dialogueboxcolor[whichdi][i][0];
    ipstream >> dialogueboxcolor[whichdi][i][1];
    ipstream >> dialogueboxcolor[whichdi][i][2];
    ipstream.ignore(256,':');
    ipstream.getline(dialoguename[whichdi][i],64);
    ipstream.ignore(256,':');
    ipstream.ignore(256,' ');
    ipstream.getline(dialoguetext[whichdi][i],128);
    for(int j=0;j<128;j++){
      if(dialoguetext[whichdi][i][j]=='\\')dialoguetext[whichdi][i][j]='\n';
    }
    ipstream.ignore(256,':');
    ipstream >> dialogueboxsound[whichdi][i];
  }

  ipstream.close();
}

static void ch_fixtype(Game *game, const char *args)
{
  int dlg;
  sscanf(args, "%d", &dlg);
  dialoguetype[0] = dlg;
}

static void ch_fixrotation(Game *game, const char *args)
{
  participantrotation[whichdialogue][participantfocus[whichdialogue][indialogue]]=player[participantfocus[whichdialogue][indialogue]].rotation;
}

static void ch_ddialogue(Game *game, const char *args)
{
  if (numdialogues)
    numdialogues--;
}

static void ch_dhs(Game *game, const char *args)
{
  if (numhotspots)
    numhotspots--;
}

static void ch_immobile(Game *game, const char *args)
{
  player[0].immobile = 1;
}

static void ch_allimmobile(Game *game, const char *args)
{
  for (int i = 1; i < numplayers; i++)
    player[i].immobile = 1;
}

static void ch_mobile(Game *game, const char *args)
{
  player[0].immobile = 0;
}

static void ch_default(Game *game, const char *args)
{
  player[0].armorhead=1;
  player[0].armorhigh=1;
  player[0].armorlow=1;
  player[0].protectionhead=1;
  player[0].protectionhigh=1;
  player[0].protectionlow=1;
  player[0].metalhead=1;
  player[0].metalhigh=1;
  player[0].metallow=1;
  player[0].power=1;
  player[0].speedmult=1;
  player[0].scale=1;

  if(player[0].creature==wolftype){
    player[0].proportionhead=1.1;
    player[0].proportionbody=1.1;
    player[0].proportionarms=1.1;
    player[0].proportionlegs=1.1;
  } else if(player[0].creature==rabbittype){
    player[0].proportionhead=1.2;
    player[0].proportionbody=1.05;
    player[0].proportionarms=1.00;
    player[0].proportionlegs=1.1;
    player[0].proportionlegs.y=1.05;
  }

  player[0].numclothes=0;
  game->LoadTextureSave(creatureskin[player[0].creature][player[0].whichskin],
			&player[0].skeleton.drawmodel.textureptr,1,&player[0].skeleton.skinText[0],
			&player[0].skeleton.skinsize);

  editoractive=typeactive;
  player[0].immobile=0;
}

static void ch_play(Game *game, const char *args)
{
  int dlg;
  sscanf(args, "%d", &dlg);
  whichdialogue = dlg;

  if (whichdialogue >= numdialogues)
    return;

  for(int i=0;i<numdialogueboxes[whichdialogue];i++){
    player[participantfocus[whichdialogue][i]].coords=participantlocation[whichdialogue][participantfocus[whichdialogue][i]];
    player[participantfocus[whichdialogue][i]].rotation=participantrotation[whichdialogue][participantfocus[whichdialogue][i]];
    player[participantfocus[whichdialogue][i]].targetrotation=participantrotation[whichdialogue][participantfocus[whichdialogue][i]];
    player[participantfocus[whichdialogue][i]].velocity=0;
    player[participantfocus[whichdialogue][i]].targetanimation=player[participantfocus[whichdialogue][i]].getIdle();
    player[participantfocus[whichdialogue][i]].targetframe=0;
  }

  directing=0;
  indialogue=0;

  playdialogueboxsound();
}

static void ch_mapkilleveryone(Game *game, const char *args)
{
  maptype = mapkilleveryone;
}

static void ch_mapkillmost(Game *game, const char *args)
{
  maptype = mapkillmost;
}

static void ch_mapkillsomeone(Game *game, const char *args)
{
  maptype = mapkillsomeone;
}

static void ch_mapgosomewhere(Game *game, const char *args)
{
  maptype = mapgosomewhere;
}

static void ch_viewdistance(Game *game, const char *args)
{
  viewdistance = atof(args)*100;
}

static void ch_fadestart(Game *game, const char *args)
{
  fadestart = atof(args);
}

static void ch_slomo(Game *game, const char *args)
{
  slomospeed = atof(args);
  slomo = !slomo;
  slomodelay = 1000;
}

static void ch_slofreq(Game *game, const char *args)
{
  slomofreq = atof(args);
}

static void ch_skytint(Game *game, const char *args)
{
  sscanf(args, "%f%f%f", &skyboxr, &skyboxg, &skyboxb);

  skyboxlightr=skyboxr;
  skyboxlightg=skyboxg;
  skyboxlightb=skyboxb;

  game->SetUpLighting();

  terrain.DoShadows();
  objects.DoShadows();
}

static void ch_skylight(Game *game, const char *args)
{
  sscanf(args, "%f%f%f", &skyboxlightr, &skyboxlightg, &skyboxlightb);

  game->SetUpLighting();

  terrain.DoShadows();
  objects.DoShadows();
}

static void ch_skybox(Game *game, const char *args)
{
  skyboxtexture = !skyboxtexture;

  game->SetUpLighting();

  terrain.DoShadows();
  objects.DoShadows();
}

static void cmd_dispatch(Game *game, const char *cmd)
{
  int i, n_cmds = sizeof(cmd_names) / sizeof(cmd_names[0]);

  for (i = 0; i < n_cmds; i++)
    if (stripfx(cmd, cmd_names[i]))
      {
	cmd_handlers[i](game, cmd + strlen(cmd_names[i]));
	break;
      }
  emit_sound_np(i < n_cmds ? consolesuccesssound : consolefailsound);
}

/********************> Tick() <*****/
extern bool save_image(const char * fname);
void Screenshot	(void)
{
	char temp[1024];
	time_t	t = time(NULL);
	struct	tm *tme = localtime(&t);
	sprintf(temp, "Screenshots/Screenshot_%04d_%02d_%02d--%02d_%02d_%02d.png", tme->tm_year + 1900, tme->tm_mon + 1, tme->tm_mday, tme->tm_hour, tme->tm_min, tme->tm_sec);

	#if defined(_WIN32)
	mkdir("Screenshots");
	#else
	mkdir("Screenshots", S_IRWXU);
	#endif
	
	save_image(temp);
}



void Game::SetUpLighting(){
	if(environment==snowyenvironment)
        light.setColors(.65,.65,.7,.4,.4,.44);
	if(environment==desertenvironment)
        light.setColors(.95,.95,.95,.4,.35,.3);
	if(environment==grassyenvironment)
        light.setColors(.95,.95,1,.4,.4,.44);
	if(!skyboxtexture)
        light.setColors(1,1,1,.4,.4,.4);
	float average;
	average=(skyboxlightr+skyboxlightg+skyboxlightb)/3;
	light.color[0]*=(skyboxlightr+average)/2;
	light.color[1]*=(skyboxlightg+average)/2;
	light.color[2]*=(skyboxlightb+average)/2;
	light.ambient[0]*=(skyboxlightr+average)/2;
	light.ambient[1]*=(skyboxlightg+average)/2;
	light.ambient[2]*=(skyboxlightb+average)/2;
}

int Game::findPathDist(int start,int end){
	int smallestcount,count,connected;
	int last,last2,last3,last4;
	int closest;

	smallestcount=1000;
	for(int i=0;i<50;i++){
		count=0;
		last=start;
		last2=-1;
		last3=-1;
		last4=-1;
		while(last!=end&&count<30){
			closest=-1;
			for(int j=0;j<numpathpoints;j++){
				if(j!=last&&j!=last2&&j!=last3&&j!=last4)
				{
					connected=0;
					if(numpathpointconnect[j])
						for(int k=0;k<numpathpointconnect[j];k++){
							if(pathpointconnect[j][k]==last)connected=1;
						}
                    if(!connected)
                        if(numpathpointconnect[last])
                            for(int k=0;k<numpathpointconnect[last];k++){
                                if(pathpointconnect[last][k]==j)connected=1;
                            }
                    if(connected)
                        if(closest==-1||Random()%2==0){
                            closest=j;
                        }
				}
			}
			last4=last3;
			last3=last2;
			last2=last;
			last=closest;
			count++;
		}
		if(count<smallestcount)smallestcount=count;
	}
	return smallestcount;
}

int Game::checkcollide(XYZ startpoint,XYZ endpoint){
	static XYZ colpoint,colviewer,coltarget;
	static float minx,minz,maxx,maxz,miny,maxy;

    minx=min(startpoint.x,endpoint.x)-1;
    miny=min(startpoint.y,endpoint.y)-1;
    minz=min(startpoint.z,endpoint.z)-1;
    maxx=max(startpoint.x,endpoint.x)+1;
    maxy=max(startpoint.y,endpoint.y)+1;
    maxz=max(startpoint.z,endpoint.z)+1;

	for(int i=0;i<objects.numobjects;i++){
		if(     objects.position[i].x>minx-objects.model[i].boundingsphereradius&&
                objects.position[i].x<maxx+objects.model[i].boundingsphereradius&&
                objects.position[i].y>miny-objects.model[i].boundingsphereradius&&
                objects.position[i].y<maxy+objects.model[i].boundingsphereradius&&
                objects.position[i].z>minz-objects.model[i].boundingsphereradius&&
                objects.position[i].z<maxz+objects.model[i].boundingsphereradius){
			if(     objects.type[i]!=treeleavestype&&
                    objects.type[i]!=bushtype&&
                    objects.type[i]!=firetype){
				colviewer=startpoint;
				coltarget=endpoint;
				if(objects.model[i].LineCheck(&colviewer,&coltarget,&colpoint,&objects.position[i],&objects.rotation[i])!=-1)return i;
			}
		}
	}

	//if(terrain.lineTerrain(startpoint,endpoint,&colpoint)!=-1)return 1000;

	return -1;
}

int Game::checkcollide(XYZ startpoint,XYZ endpoint,int what){
	static XYZ colpoint,colviewer,coltarget;
	static float minx,minz,maxx,maxz,miny,maxy;
	static int i; //FIXME: see below

    minx=min(startpoint.x,endpoint.x)-1;
    miny=min(startpoint.y,endpoint.y)-1;
    minz=min(startpoint.z,endpoint.z)-1;
    maxx=max(startpoint.x,endpoint.x)+1;
    maxy=max(startpoint.y,endpoint.y)+1;
    maxz=max(startpoint.z,endpoint.z)+1;

	if(what!=1000){
		if(     objects.position[what].x>minx-objects.model[what].boundingsphereradius&&
                objects.position[what].x<maxx+objects.model[what].boundingsphereradius&&
                objects.position[what].y>miny-objects.model[what].boundingsphereradius&&
                objects.position[what].y<maxy+objects.model[what].boundingsphereradius&&
                objects.position[what].z>minz-objects.model[what].boundingsphereradius&&
                objects.position[what].z<maxz+objects.model[what].boundingsphereradius){
			if(     objects.type[what]!=treeleavestype&&
                    objects.type[what]!=bushtype&&
                    objects.type[what]!=firetype){
				colviewer=startpoint;
				coltarget=endpoint;
                //FIXME: i/what
				if(objects.model[what].LineCheck(&colviewer,&coltarget,&colpoint,&objects.position[what],&objects.rotation[what])!=-1)return i;
			}
		}
	}

	if(what==1000)if(terrain.lineTerrain(startpoint,endpoint,&colpoint)!=-1)return 1000;

	return -1;
}

void	Game::Setenvironment(int which)
{
	LOGFUNC;

	LOG(" Setting environment...");

	float temptexdetail;
	environment=which;

	pause_sound(stream_music1snow);
	pause_sound(stream_music1grass);
	pause_sound(stream_music1desert);
	pause_sound(stream_wind);
	pause_sound(stream_desertambient);


	if(environment==snowyenvironment){
		windvector=0;
		windvector.z=3;
		if(ambientsound)
		  emit_stream_np(stream_wind);

		LoadTexture(":Data:Textures:snowtree.png",&objects.treetextureptr,0,1);
		LoadTexture(":Data:Textures:bushsnow.png",&objects.bushtextureptr,0,1);
		LoadTexture(":Data:Textures:bouldersnow.jpg",&objects.rocktextureptr,1,0);
		LoadTexture(":Data:Textures:snowbox.jpg",&objects.boxtextureptr,1,0);

		footstepsound = footstepsn1;
		footstepsound2 = footstepsn2;
		footstepsound3 = footstepst1;
		footstepsound4 = footstepst2;

		LoadTexture(":Data:Textures:snow.jpg",&terraintexture,1,0);

		LoadTexture(":Data:Textures:rock.jpg",&terraintexture2,1,0);

		//LoadTexture(":Data:Textures:detailgrain.png",&terraintexture3,1);




		temptexdetail=texdetail;
		if(texdetail>1)texdetail=4;
		skybox.load(	":Data:Textures:Skybox(snow):Front.jpg",
			":Data:Textures:Skybox(snow):Left.jpg",
			":Data:Textures:Skybox(snow):Back.jpg",
			":Data:Textures:Skybox(snow):Right.jpg",
			":Data:Textures:Skybox(snow):Up.jpg",
			":Data:Textures:Skybox(snow):Down.jpg");




		texdetail=temptexdetail;
	}
	if(environment==desertenvironment){
		windvector=0;
		windvector.z=2;
		LoadTexture(":Data:Textures:deserttree.png",&objects.treetextureptr,0,1);
		LoadTexture(":Data:Textures:bushdesert.png",&objects.bushtextureptr,0,1);
		LoadTexture(":Data:Textures:boulderdesert.jpg",&objects.rocktextureptr,1,0);
		LoadTexture(":Data:Textures:desertbox.jpg",&objects.boxtextureptr,1,0);


		if(ambientsound)
		  emit_stream_np(stream_desertambient);

		footstepsound = footstepsn1;
		footstepsound2 = footstepsn2;
		footstepsound3 = footstepsn1;
		footstepsound4 = footstepsn2;

		LoadTexture(":Data:Textures:sand.jpg",&terraintexture,1,0);

		LoadTexture(":Data:Textures:sandslope.jpg",&terraintexture2,1,0);

		//LoadTexture(":Data:Textures:detailgrain.png",&terraintexture3,1);



		temptexdetail=texdetail;
		if(texdetail>1)texdetail=4;
		skybox.load(	":Data:Textures:Skybox(sand):Front.jpg",
			":Data:Textures:Skybox(sand):Left.jpg",
			":Data:Textures:Skybox(sand):Back.jpg",
			":Data:Textures:Skybox(sand):Right.jpg",
			":Data:Textures:Skybox(sand):Up.jpg",
			":Data:Textures:Skybox(sand):Down.jpg");




		texdetail=temptexdetail;
	}
	if(environment==grassyenvironment){
		windvector=0;
		windvector.z=2;
		LoadTexture(":Data:Textures:tree.png",&objects.treetextureptr,0,1);
		LoadTexture(":Data:Textures:bush.png",&objects.bushtextureptr,0,1);
		LoadTexture(":Data:Textures:boulder.jpg",&objects.rocktextureptr,1,0);
		LoadTexture(":Data:Textures:grassbox.jpg",&objects.boxtextureptr,1,0);

		if(ambientsound)
		  emit_stream_np(stream_wind, 100.);

		footstepsound = footstepgr1;
		footstepsound2 = footstepgr2;
		footstepsound3 = footstepst1;
		footstepsound4 = footstepst2;

		LoadTexture(":Data:Textures:grassdirt.jpg",&terraintexture,1,0);

		LoadTexture(":Data:Textures:mossrock.jpg",&terraintexture2,1,0);

		//LoadTexture(":Data:Textures:detail.png",&terraintexture3,1);



		temptexdetail=texdetail;
		if(texdetail>1)texdetail=4;
		skybox.load(	":Data:Textures:Skybox(grass):Front.jpg",
			":Data:Textures:Skybox(grass):Left.jpg",
			":Data:Textures:Skybox(grass):Back.jpg",
			":Data:Textures:Skybox(grass):Right.jpg",
			":Data:Textures:Skybox(grass):Up.jpg",
			":Data:Textures:Skybox(grass):Down.jpg");



		texdetail=temptexdetail;
	}
	temptexdetail=texdetail;
	texdetail=1;
	terrain.load(":Data:Textures:heightmap.png");

	texdetail=temptexdetail;
}


void	Game::Loadlevel(int which){
	stealthloading=0;
	whichlevel=which;

	if (which == -1) {
	    tutoriallevel = -1;
	    Loadlevel("tutorial");
	} else if (which >= 0 && which <= 15) {
	    char buf[32];
	    snprintf(buf, 32, "map%d", which + 1);
	    Loadlevel(buf);
	} else
	    Loadlevel("mapsave");
}

void	Game::Loadlevel(const char *name){
	static int oldlevel;
	int templength;
	float lamefloat;
	int lameint;
	static const char *pfx = ":Data:Maps:";
	char *buf;

	float headprop,legprop,armprop,bodyprop;

	LOGFUNC;

	LOG(std::string("Loading level...") + name);

	if(!gameon)visibleloading=1;

	if(stealthloading)visibleloading=0;

	if(!stillloading)loadtime=0;
	gamestarted=1;

	numenvsounds=0;
	//visibleloading=1;
	if(tutoriallevel!=-1)tutoriallevel=0;
	else tutoriallevel=1;

	if(tutoriallevel==1)tutorialstage=0;
	if(tutorialstage==0){
		tutorialstagetime=0;
		tutorialmaxtime=1;
	}
	loadingstuff=1;
	if(!firstload){
		oldlevel=50;
	}
	pause_sound(whooshsound);
	pause_sound(stream_firesound);

	// Change the map filename into something that is os specific
	buf = (char*) alloca(strlen(pfx) + strlen(name) + 1);
	sprintf(buf, "%s%s", pfx, name);
	const char *FixedFN = ConvertFileName(buf);

	int mapvers;
	FILE			*tfile;
	tfile=fopen( FixedFN, "rb" );
	if(tfile)
	{
		pause_sound(stream_firesound);


		scoreadded=0;
		windialogue=0;

		hostiletime=0;

		won=0;

		//campaign=0;
		animation[bounceidleanim].Load((char *)"Idle",middleheight,neutral);

		numdialogues=0;

		for(int i=0;i<20;i++)
		{
			dialoguegonethrough[i]=0;
		}

		indialogue=-1;
		cameramode=0;

		damagedealt=0;
		damagetaken=0;

		if(accountactive)difficulty=accountactive->getDifficulty();

		if(difficulty!=2)minimap=1;
		else minimap=0;

		numhotspots=0;
		currenthotspot=-1;
		bonustime=1;

		skyboxtexture=1;
		skyboxr=1;
		skyboxg=1;
		skyboxb=1;

		freeze=0;
		winfreeze=0;

		for(int i=0;i<100;i++)
		{
			bonusnum[i]=0;
		}

		numfalls=0;
		numflipfail=0;
		numseen=0;
		numstaffattack=0;
		numswordattack=0;
		numknifeattack=0;
		numunarmedattack=0;
		numescaped=0;
		numflipped=0;
		numwallflipped=0;
		numthrowkill=0;
		numafterkill=0;
		numreversals=0;
		numattacks=0;
		maxalarmed=0;
		numresponded=0;

		bonustotal=startbonustotal;
		bonus=0;
		gameon=1;
		changedelay=0;
		if(console)
		{
			emit_sound_np(consolesuccesssound);
			freeze=0;
			console=0;
		}

		if(!stealthloading)
		{
			terrain.numdecals=0;
			Sprite::deleteSprites();
			for(int i=0;i<objects.numobjects;i++)
			{
				objects.model[i].numdecals=0;
			}

			int j=objects.numobjects;
			for(int i=0;i<j;i++)
			{
				objects.DeleteObject(0);
				if(visibleloading){loadscreencolor=4; LoadingScreen();}
			}

			for(int i=0;i<subdivision;i++)
			{
				for(int j=0;j<subdivision;j++)
				{
					terrain.patchobjectnum[i][j]=0;
				}
			}
			if(visibleloading){loadscreencolor=4; LoadingScreen();}
		}

		weapons.numweapons=0;

		funpackf(tfile, "Bi", &mapvers);
		if(mapvers>=15)funpackf(tfile, "Bi", &indemo);
		else indemo=0;
		if(mapvers>=5)funpackf(tfile, "Bi", &maptype);
		else maptype=mapkilleveryone;
		if(mapvers>=6)funpackf(tfile, "Bi", &hostile);
		else hostile=1;
		if(mapvers>=4)funpackf(tfile, "Bf Bf", &viewdistance, &fadestart);
		else
		{
			viewdistance=100;
			fadestart=.6;
		}
		if(mapvers>=2)funpackf(tfile, "Bb Bf Bf Bf", &skyboxtexture, &skyboxr, &skyboxg, &skyboxb);
		else
		{
			skyboxtexture=1;
			skyboxr=1;
			skyboxg=1;
			skyboxb=1;
		}
		if(mapvers>=10)funpackf(tfile, "Bf Bf Bf", &skyboxlightr, &skyboxlightg, &skyboxlightb);
		else
		{
			skyboxlightr=skyboxr;
			skyboxlightg=skyboxg;
			skyboxlightb=skyboxb;
		}
		if(!stealthloading)funpackf(tfile, "Bf Bf Bf Bf Bf Bi", &player[0].coords.x,&player[0].coords.y,&player[0].coords.z,&player[0].rotation,&player[0].targetrotation, &player[0].num_weapons);
		if(stealthloading)funpackf(tfile, "Bf Bf Bf Bf Bf Bi", &lamefloat,&lamefloat,&lamefloat,&lamefloat,&lamefloat, &player[0].num_weapons);
		player[0].originalcoords=player[0].coords;
		if(player[0].num_weapons>0&&player[0].num_weapons<5)
		{
			for(int j=0;j<player[0].num_weapons;j++)
			{
				player[0].weaponids[j]=weapons.numweapons;
				funpackf(tfile, "Bi", &weapons.type[weapons.numweapons]);
				weapons.owner[weapons.numweapons]=0;
				weapons.numweapons++;
			}
		}

		if(visibleloading){loadscreencolor=4; LoadingScreen();}

		funpackf(tfile, "Bf Bf Bf", &player[0].armorhead, &player[0].armorhigh, &player[0].armorlow);
		funpackf(tfile, "Bf Bf Bf", &player[0].protectionhead, &player[0].protectionhigh, &player[0].protectionlow);
		funpackf(tfile, "Bf Bf Bf", &player[0].metalhead, &player[0].metalhigh, &player[0].metallow);
		funpackf(tfile, "Bf Bf", &player[0].power, &player[0].speedmult);

		funpackf(tfile, "Bi", &player[0].numclothes);

		if(mapvers>=9)
		{
			funpackf(tfile, "Bi Bi", &player[0].whichskin, &player[0].creature);
		}
		else
		{
			player[0].whichskin=0;
			player[0].creature=rabbittype;
		}

		player[0].lastattack=-1;
		player[0].lastattack2=-1;
		player[0].lastattack3=-1;

		if(mapvers>=8)
		{
			funpackf(tfile, "Bi", &numdialogues);
			if(numdialogues)
			{
				for(int k=0;k<numdialogues;k++)
				{
					funpackf(tfile, "Bi", &numdialogueboxes[k]);
					funpackf(tfile, "Bi", &dialoguetype[k]);
					for(int l=0;l<10;l++)
					{
						funpackf(tfile, "Bf Bf Bf", &participantlocation[k][l].x, &participantlocation[k][l].y, &participantlocation[k][l].z);
						funpackf(tfile, "Bf", &participantrotation[k][l]);
					}
					if(numdialogueboxes)
					{
						for(int l=0;l<numdialogueboxes[k];l++)
						{
							funpackf(tfile, "Bi", &dialogueboxlocation[k][l]);
							funpackf(tfile, "Bf", &dialogueboxcolor[k][l][0]);
							funpackf(tfile, "Bf", &dialogueboxcolor[k][l][1]);
							funpackf(tfile, "Bf", &dialogueboxcolor[k][l][2]);
							funpackf(tfile, "Bi", &dialogueboxsound[k][l]);

							bool doneread;

							funpackf(tfile, "Bi",&templength);
							if(templength>128||templength<=0)templength=128;
                            int m;
							for(m=0;m<templength;m++){
								funpackf(tfile, "Bb", &dialoguetext[k][l][m]);
								if(dialoguetext[k][l][m]=='\0')break;
							}
							dialoguetext[k][l][m] = 0;

							funpackf(tfile, "Bi",&templength);
							if(templength>64||templength<=0)templength=64;
							for(m=0;m<templength;m++){
								funpackf(tfile, "Bb", &dialoguename[k][l][m]);
								if(dialoguename[k][l][m]=='\0'){
									break;
								}
							}
							dialoguename[k][l][m] = 0;
							funpackf(tfile, "Bf Bf Bf", &dialoguecamera[k][l].x, &dialoguecamera[k][l].y, &dialoguecamera[k][l].z);
							funpackf(tfile, "Bi", &participantfocus[k][l]);
							funpackf(tfile, "Bi", &participantaction[k][l]);

							for(m=0;m<10;m++)
								funpackf(tfile, "Bf Bf Bf", &participantfacing[k][l][m].x, &participantfacing[k][l][m].y, &participantfacing[k][l][m].z);

							funpackf(tfile, "Bf Bf",&dialoguecamerarotation[k][l],&dialoguecamerarotation2[k][l]);
						}
					}
				}
			}
		}
		else numdialogues=0;

		if(player[0].numclothes)
		{
			for(int k=0;k<player[0].numclothes;k++)
			{
				funpackf(tfile, "Bi", &templength);
				for(int l=0;l<templength;l++)
					funpackf(tfile, "Bb", &player[0].clothes[k][l]);
				player[0].clothes[k][templength]='\0';
				funpackf(tfile, "Bf Bf Bf", &player[0].clothestintr[k], &player[0].clothestintg[k], &player[0].clothestintb[k]);
			}
		}

		funpackf(tfile, "Bi", &environment);

		funpackf(tfile, "Bi", &objects.numobjects);
		if(objects.numobjects)
		{
			for(int i=0;i<objects.numobjects;i++)
			{
				funpackf(tfile, "Bi Bf Bf Bf Bf Bf Bf", &objects.type[i],&objects.rotation[i],&objects.rotation2[i], &objects.position[i].x, &objects.position[i].y, &objects.position[i].z,&objects.scale[i]);
				if(objects.type[i]==treeleavestype)objects.scale[i]=objects.scale[i-1];
			}
		}

		if(mapvers>=7)
		{
			funpackf(tfile, "Bi", &numhotspots);
			if(numhotspots)
			{
				for(int i=0;i<numhotspots;i++)
				{
					funpackf(tfile, "Bi Bf Bf Bf Bf", &hotspottype[i],&hotspotsize[i],&hotspot[i].x,&hotspot[i].y,&hotspot[i].z);
					funpackf(tfile, "Bi", &templength);
					if(templength)
						for(int l=0;l<templength;l++)
							funpackf(tfile, "Bb", &hotspottext[i][l]);
					hotspottext[i][templength]='\0';
					if(hotspottype[i]==-111)indemo=1;
				}
			}
		}
		else numhotspots=0;

		if(visibleloading){loadscreencolor=4; LoadingScreen();}

		if(!stealthloading)
		{
			objects.center=0;
			for(int i=0;i<objects.numobjects;i++)
			{
				objects.center+=objects.position[i];
			}
			objects.center/=objects.numobjects;


			if(visibleloading){loadscreencolor=4; LoadingScreen();}

			float maxdistance=0;
			float tempdist;
			int whichclosest;
			for(int i=0;i<objects.numobjects;i++)
			{
				tempdist=findDistancefast(&objects.center,&objects.position[i]);
				if(tempdist>maxdistance)
				{
					whichclosest=i;
					maxdistance=tempdist;
				}
			}
			objects.radius=fast_sqrt(maxdistance);
		}

		if(visibleloading){loadscreencolor=4; LoadingScreen();}
		//mapcenter=objects.center;
		//mapradius=objects.radius;

		funpackf(tfile, "Bi", &numplayers);
		int howmanyremoved=0;
		bool removeanother=0;
		if(numplayers>1&&numplayers<maxplayers)
		{
			for(int i=1;i<numplayers;i++)
			{
				if(visibleloading){loadscreencolor=4; LoadingScreen();}
				removeanother=0;

				funpackf(tfile, "Bi Bi Bf Bf Bf Bi",&player[i-howmanyremoved].whichskin,&player[i-howmanyremoved].creature, &player[i-howmanyremoved].coords.x,&player[i-howmanyremoved].coords.y,&player[i-howmanyremoved].coords.z,&player[i-howmanyremoved].num_weapons);
				if(mapvers>=5)funpackf(tfile, "Bi", &player[i-howmanyremoved].howactive);
				else player[i-howmanyremoved].howactive=typeactive;
				if(mapvers>=3)funpackf(tfile, "Bf",&player[i-howmanyremoved].scale);
				else player[i-howmanyremoved].scale=-1;
				if(mapvers>=11)funpackf(tfile, "Bb",&player[i-howmanyremoved].immobile);
				else player[i-howmanyremoved].immobile=0;
				if(mapvers>=12)funpackf(tfile, "Bf",&player[i-howmanyremoved].rotation);
				else player[i-howmanyremoved].rotation=0;
				player[i-howmanyremoved].targetrotation=player[i-howmanyremoved].rotation;
				if(player[i-howmanyremoved].num_weapons<0||player[i-howmanyremoved].num_weapons>5){
					removeanother=1;
					howmanyremoved++;
				}
				if(!removeanother)
				{
					if(player[i-howmanyremoved].num_weapons>0&&player[i-howmanyremoved].num_weapons<5)
					{
						for(int j=0;j<player[i-howmanyremoved].num_weapons;j++)
						{
							player[i-howmanyremoved].weaponids[j]=weapons.numweapons;
							funpackf(tfile, "Bi", &weapons.type[player[i-howmanyremoved].weaponids[j]]);
							weapons.owner[player[i-howmanyremoved].weaponids[j]]=i;
							weapons.numweapons++;
						}
					}
					funpackf(tfile, "Bi", &player[i-howmanyremoved].numwaypoints);
					//player[i-howmanyremoved].numwaypoints=10;
					for(int j=0;j<player[i-howmanyremoved].numwaypoints;j++)
					{
						funpackf(tfile, "Bf", &player[i-howmanyremoved].waypoints[j].x);
						funpackf(tfile, "Bf", &player[i-howmanyremoved].waypoints[j].y);
						funpackf(tfile, "Bf", &player[i-howmanyremoved].waypoints[j].z);
						if(mapvers>=5)funpackf(tfile, "Bi", &player[i-howmanyremoved].waypointtype[j]);
						else player[i-howmanyremoved].waypointtype[j] = wpkeepwalking;
					}

					funpackf(tfile, "Bi", &player[i-howmanyremoved].waypoint);
					if(player[i-howmanyremoved].waypoint>player[i-howmanyremoved].numwaypoints-1)player[i-howmanyremoved].waypoint=0;

					funpackf(tfile, "Bf Bf Bf", &player[i-howmanyremoved].armorhead, &player[i-howmanyremoved].armorhigh, &player[i-howmanyremoved].armorlow);
					funpackf(tfile, "Bf Bf Bf", &player[i-howmanyremoved].protectionhead, &player[i-howmanyremoved].protectionhigh, &player[i-howmanyremoved].protectionlow);
					funpackf(tfile, "Bf Bf Bf", &player[i-howmanyremoved].metalhead, &player[i-howmanyremoved].metalhigh, &player[i-howmanyremoved].metallow);
					funpackf(tfile, "Bf Bf", &player[i-howmanyremoved].power, &player[i-howmanyremoved].speedmult);

					if(mapvers>=4)funpackf(tfile, "Bf Bf Bf Bf", &headprop, &bodyprop, &armprop, &legprop);
					else
					{
						headprop=1;
						bodyprop=1;
						armprop=1;
						legprop=1;
					}
					if(player[i-howmanyremoved].creature==wolftype)
					{
						player[i-howmanyremoved].proportionhead=1.1*headprop;
						player[i-howmanyremoved].proportionbody=1.1*bodyprop;
						player[i-howmanyremoved].proportionarms=1.1*armprop;
						player[i-howmanyremoved].proportionlegs=1.1*legprop;
					}

					if(player[i-howmanyremoved].creature==rabbittype)
					{
						player[i-howmanyremoved].proportionhead=1.2*headprop;
						player[i-howmanyremoved].proportionbody=1.05*bodyprop;
						player[i-howmanyremoved].proportionarms=1.00*armprop;
						player[i-howmanyremoved].proportionlegs=1.1*legprop;
						player[i-howmanyremoved].proportionlegs.y=1.05*legprop;
					}

					funpackf(tfile, "Bi", &player[i-howmanyremoved].numclothes);
					if(player[i-howmanyremoved].numclothes)
					{
						for(int k=0;k<player[i-howmanyremoved].numclothes;k++)
						{
							int templength;
							funpackf(tfile, "Bi", &templength);
							for(int l=0;l<templength;l++)
								funpackf(tfile, "Bb", &player[i-howmanyremoved].clothes[k][l]);
							player[i-howmanyremoved].clothes[k][templength]='\0';
							funpackf(tfile, "Bf Bf Bf", &player[i-howmanyremoved].clothestintr[k], &player[i-howmanyremoved].clothestintg[k], &player[i-howmanyremoved].clothestintb[k]);
						}
					}
				}
			}
		}
		if(visibleloading){loadscreencolor=4; LoadingScreen();}

		numplayers-=howmanyremoved;
		funpackf(tfile, "Bi", &numpathpoints);
		if(numpathpoints>30||numpathpoints<0)
			numpathpoints=0;
		if(numpathpoints)
		{
			for(int j=0;j<numpathpoints;j++)
			{
				funpackf(tfile, "Bf Bf Bf Bi", &pathpoint[j].x,&pathpoint[j].y,&pathpoint[j].z,&numpathpointconnect[j]);
				for(int k=0;k<numpathpointconnect[j];k++){
					funpackf(tfile, "Bi", &pathpointconnect[j][k]);
				}
			}
		}
		if(visibleloading){loadscreencolor=4; LoadingScreen();}

		funpackf(tfile, "Bf Bf Bf Bf", &mapcenter.x,&mapcenter.y,&mapcenter.z,&mapradius);

		SetUpLighting();
		if(environment!=oldenvironment)
            Setenvironment(environment);
		oldenvironment=environment;

		if(!stealthloading)
		{
			int j=objects.numobjects;
			objects.numobjects=0;
			for(int i=0;i<j;i++)
			{
				//if(objects.type[i]!=spiketype)
				objects.MakeObject(objects.type[i],objects.position[i],objects.rotation[i],objects.rotation2[i],objects.scale[i]);
				if(visibleloading){loadscreencolor=4; LoadingScreen();}
			}

			//if(skyboxtexture){
			terrain.DoShadows();
			if(visibleloading){loadscreencolor=4; LoadingScreen();}
			objects.DoShadows();
			if(visibleloading){loadscreencolor=4; LoadingScreen();}
			/*}
			else terrain.DoLighting();
			*/
		}

		fclose(tfile);

		oldlevel=whichlevel;


		if(numplayers>maxplayers-1)numplayers=maxplayers-1;
		for(int i=0;i<numplayers;i++)
		{
			if(visibleloading){loadscreencolor=4; LoadingScreen();}
			player[i].burnt=0;
			player[i].bled=0;
			player[i].onfire=0;
			if(i==0||player[i].scale<0)player[i].scale=.2;
			player[i].skeleton.free=0;
			player[i].skeleton.id=i;
			//if(Random()%2==0)player[i].creature=wolftype;
			//else player[i].creature=rabbittype;
			if(i==0&&mapvers<9)player[i].creature=rabbittype;
			if(player[i].creature!=wolftype)player[i].skeleton.Load(
                    (char *)":Data:Skeleton:Basic Figure",
                    (char *)":Data:Skeleton:Basic Figurelow",
                    (char *)":Data:Skeleton:Rabbitbelt",
                    (char *)":Data:Models:Body.solid",
                    (char *)":Data:Models:Body2.solid",
                    (char *)":Data:Models:Body3.solid",
                    (char *)":Data:Models:Body4.solid",
                    (char *)":Data:Models:Body5.solid",
                    (char *)":Data:Models:Body6.solid",
                    (char *)":Data:Models:Body7.solid",
                    (char *)":Data:Models:Bodylow.solid",
                    (char *)":Data:Models:Belt.solid",0);
			else
			{
				if(player[i].creature!=wolftype){
					player[i].skeleton.Load(
                            (char *)":Data:Skeleton:Basic Figure",
                            (char *)":Data:Skeleton:Basic Figurelow",
                            (char *)":Data:Skeleton:Rabbitbelt",
                            (char *)":Data:Models:Body.solid",
                            (char *)":Data:Models:Body2.solid",
                            (char *)":Data:Models:Body3.solid",
                            (char *)":Data:Models:Body4.solid",
                            (char *)":Data:Models:Body5.solid",
                            (char *)":Data:Models:Body6.solid",
                            (char *)":Data:Models:Body7.solid",
                            (char *)":Data:Models:Bodylow.solid",
                            (char *)":Data:Models:Belt.solid",1);
					LoadTexture(":Data:Textures:Belt.png",&player[i].skeleton.drawmodelclothes.textureptr,1,1);
				}
				if(player[i].creature==wolftype){
					player[i].skeleton.Load(
                            (char *)":Data:Skeleton:Basic Figure Wolf",
                            (char *)":Data:Skeleton:Basic Figure Wolf Low",
                            (char *)":Data:Skeleton:Rabbitbelt",
                            (char *)":Data:Models:Wolf.solid",
                            (char *)":Data:Models:Wolf2.solid",
                            (char *)":Data:Models:Wolf3.solid",
                            (char *)":Data:Models:Wolf4.solid",
                            (char *)":Data:Models:Wolf5.solid",
                            (char *)":Data:Models:Wolf6.solid",
                            (char *)":Data:Models:Wolf7.solid",
                            (char *)":Data:Models:Wolflow.solid",
                            (char *)":Data:Models:Belt.solid",0);
				}
			}


			int texsize;
			texsize=512*512*3/texdetail/texdetail;
			//if(!player[i].loaded)player[i].skeleton.skinText = new GLubyte[texsize];
			//player[i].skeleton.skinText.resize(texsize);

			LoadTextureSave(creatureskin[player[i].creature][player[i].whichskin],&player[i].skeleton.drawmodel.textureptr,1,&player[i].skeleton.skinText[0],&player[i].skeleton.skinsize);

			if(player[i].numclothes)
			{
				for(int j=0;j<player[i].numclothes;j++)
				{
					tintr=player[i].clothestintr[j];
					tintg=player[i].clothestintg[j];
					tintb=player[i].clothestintb[j];
					AddClothes((char *)player[i].clothes[j],0,1,&player[i].skeleton.skinText[0],&player[i].skeleton.skinsize);
				}
				player[i].DoMipmaps();
			}

			player[i].currentanimation=bounceidleanim;
			player[i].targetanimation=bounceidleanim;
			player[i].currentframe=0;
			player[i].targetframe=1;
			player[i].target=0;
			player[i].speed=1+(float)(Random()%100)/1000;
			if(difficulty==0)player[i].speed-=.2;
			if(difficulty==1)player[i].speed-=.1;

			player[i].velocity=0;
			player[i].oldcoords=player[i].coords;
			player[i].realoldcoords=player[i].coords;

			player[i].id=i;
			player[i].skeleton.id=i;
			player[i].updatedelay=0;
			player[i].normalsupdatedelay=0;

			player[i].aitype=passivetype;
			player[i].madskills=0;

			if(i==0)
			{
				player[i].proportionhead=1.2;
				player[i].proportionbody=1.05;
				player[i].proportionarms=1.00;
				player[i].proportionlegs=1.1;
				player[i].proportionlegs.y=1.05;
			}
			player[i].headless=0;
			player[i].currentoffset=0;
			player[i].targetoffset=0;
			/*player[i].armorhead=1;
			player[i].armorhigh=1;
			player[i].armorlow=1;
			player[i].protectionhead=1;
			player[i].protectionhigh=1;
			player[i].protectionlow=1;
			player[i].metalhead=1;
			player[i].metalhigh=1;
			player[i].metallow=1;
			player[i].power=1;
			player[i].speedmult=1;*/

			player[i].damagetolerance=200;

			if(player[i].creature==wolftype)
			{
				/*player[i].proportionhead=1.1;
				player[i].proportionbody=1.1;
				player[i].proportionarms=1.1;
				player[i].proportionlegs=1.1;
				player[i].proportionlegs.y=1.1;*/
				if(i==0||player[i].scale<0)player[i].scale=.23;

				player[i].damagetolerance=300;
			}

			if(visibleloading){loadscreencolor=4; LoadingScreen();}
			if(cellophane)
			{
				player[i].proportionhead.z=0;
				player[i].proportionbody.z=0;
				player[i].proportionarms.z=0;
				player[i].proportionlegs.z=0;
			}

			player[i].tempanimation.Load((char *)"Tempanim",0,0);

			player[i].headmorphness=0;
			player[i].targetheadmorphness=1;
			player[i].headmorphstart=0;
			player[i].headmorphend=0;

			player[i].pausetime=0;

			player[i].dead=0;
			player[i].jumppower=5;
			player[i].damage=0;
			player[i].permanentdamage=0;
			player[i].superpermanentdamage=0;

			player[i].forwardkeydown=0;
			player[i].leftkeydown=0;
			player[i].backkeydown=0;
			player[i].rightkeydown=0;
			player[i].jumpkeydown=0;
			player[i].crouchkeydown=0;
			player[i].throwkeydown=0;

			player[i].collided=-10;
			player[i].loaded=1;
			player[i].bloodloss=0;
			player[i].weaponactive=-1;
			player[i].weaponstuck=-1;
			player[i].bleeding=0;
			player[i].deathbleeding=0;
			player[i].stunned=0;
			player[i].hasvictim=0;
			player[i].wentforweapon=0;
		}

		player[0].aitype=playercontrolled;
		player[0].weaponactive=-1;

		if(difficulty==1)
		{
			//player[0].speedmult=1/.9;
			player[0].power=1/.9;
		}

		if(difficulty==0)
		{
			//player[0].speedmult=1/.8;
			player[0].power=1/.8;
		}

		//player[0].weaponstuck=1;

		if(difficulty==1)player[0].damagetolerance=250;
		if(difficulty==0)player[0].damagetolerance=300;
		if(difficulty==0)player[0].armorhead*=1.5;
		if(difficulty==0)player[0].armorhigh*=1.5;
		if(difficulty==0)player[0].armorlow*=1.5;
		cameraloc=player[0].coords;
		cameraloc.y+=5;
		rotation=player[0].rotation;

		hawkcoords=player[0].coords;
		hawkcoords.y+=30;

		if(visibleloading){loadscreencolor=4; LoadingScreen();}
		//weapons.numweapons=numplayers;
		for(int i=0;i<weapons.numweapons;i++)
		{
			weapons.bloody[i]=0;
			weapons.blooddrip[i]=0;
			weapons.blooddripdelay[i]=0;
			weapons.onfire[i]=0;
			weapons.flamedelay[i]=0;
			weapons.damage[i]=0;
			//weapons.type[i]=sword;
			if(weapons.type[i]==sword){
				weapons.mass[i]=1.5;
				weapons.tipmass[i]=1;
				weapons.length[i]=.8;
			}
			if(weapons.type[i]==staff){
				weapons.mass[i]=2;
				weapons.tipmass[i]=1;
				weapons.length[i]=1.5;
			}
			if(weapons.type[i]==knife){
				weapons.mass[i]=1;
				weapons.tipmass[i]=1.2;
				weapons.length[i]=.25;
			}
			weapons.position[i]=-1000;
			weapons.tippoint[i]=-1000;
		}
		
		LOG("Starting background music...");

		OPENAL_StopSound(OPENAL_ALL);
		if(environment==snowyenvironment)
		{
			if(ambientsound)
			  emit_stream_np(stream_wind);
		}
		else if(environment==desertenvironment)
		{
			if(ambientsound)
			  emit_stream_np(stream_desertambient);
		}
		else if(environment==grassyenvironment)
		{
			if(ambientsound)
			  emit_stream_np(stream_wind, 100.);
		}
		oldmusicvolume[0]=0;
		oldmusicvolume[1]=0;
		oldmusicvolume[2]=0;
		oldmusicvolume[3]=0;

		if(!firstload)
		{
			firstload=1;
		}
	}
	leveltime=0;
	loadingstuff=0;
	visibleloading=0;
}



void Game::MenuTick(){
    //menu buttons
    if(mainmenu==1||mainmenu==2){
        if(Input::MouseClicked()&&selected==1){
            if(!gameon){
                fireSound(firestartsound);
                flash();
                //new game
                if(accountactive) {
                    mainmenu=5;
                } else {
                    mainmenu=7;
                }
                selected=-1;
            }else{
                //resume
                mainmenu=0;
                pause_sound(stream_music3);
                resume_stream(music1);
            }
        }

        if(Input::MouseClicked()&&selected==2){
            fireSound();
            flash();
            //options
            mainmenu=3;
            if(newdetail>2)newdetail=detail;
            if(newdetail<0)newdetail=detail;
            if(newscreenwidth>3000)newscreenwidth=screenwidth;
            if(newscreenwidth<0)newscreenwidth=screenwidth;
            if(newscreenheight>3000)newscreenheight=screenheight;
            if(newscreenheight<0)newscreenheight=screenheight;
        }

        if(Input::MouseClicked()&&selected==3){
            fireSound();
            flash();
            if(!gameon){
                //quit
                tryquit=1;
                pause_sound(stream_music3);
            }else{
                //end game
                gameon=0;
                mainmenu=1;
            }
        }
    }
    if(mainmenu==3){
        if(Input::MouseClicked()){

            if(selected!=-1)
                fireSound();

            switch(selected){
                case 0: {
                    extern SDL_Rect **resolutions;
                    bool isCustomResolution = true;
                    bool found = false;
                    for(int i = 0; (!found) && (resolutions[i]); i++){
                        if((resolutions[i]->w == screenwidth) && (resolutions[i]->h == screenwidth))
                            isCustomResolution = false;

                        if((resolutions[i]->w == newscreenwidth) && (resolutions[i]->h == newscreenheight)){
                            i++;
                            if(resolutions[i] != NULL){
                                newscreenwidth = (int) resolutions[i]->w;
                                newscreenheight = (int) resolutions[i]->h;
                            }else if(isCustomResolution){
                                if((screenwidth == newscreenwidth) && (screenheight == newscreenheight)){
                                    newscreenwidth = (int) resolutions[0]->w;
                                    newscreenheight = (int) resolutions[0]->h;
                                }else{
                                    newscreenwidth = screenwidth;
                                    newscreenheight = screenheight;
                                }
                            }else{
                                newscreenwidth = (int) resolutions[0]->w;
                                newscreenheight = (int) resolutions[0]->h;
                            }
                            found = true;
                        }
                    }

                    if(!found){
                        newscreenwidth = (int) resolutions[0]->w;
                        newscreenheight = (int) resolutions[0]->h;
                    }
                    } break;
                case 1:
                    newdetail++;
                    if(newdetail>2)newdetail=0;
                    break;
                case 2:
                    bloodtoggle++;
                    if(bloodtoggle>2)bloodtoggle=0;
                    break;
                case 3:
                    difficulty++;
                    if(difficulty>2)difficulty=0;
                    break;
                case 4:
                    ismotionblur=1-ismotionblur;
                    break;
                case 5:
                    decals=1-decals;
                    break;
                case 6:
                    musictoggle=1-musictoggle;

                    if(!musictoggle){
                        pause_sound(music1);
                        pause_sound(stream_music2);
                        pause_sound(stream_music3);

                        for(int i=0;i<4;i++){
                            oldmusicvolume[i]=0;
                            musicvolume[i]=0;
                        }
                    }

                    if(musictoggle)
                      emit_stream_np(stream_music3);
                    break;
                case 7:
                    flash();
                    //options
                    mainmenu=4;
                    selected=-1;
                    keyselect=-1;
                    break;
                case 8:
                    flash();

                    if(newdetail>2)newdetail=detail;
                    if(newdetail<0)newdetail=detail;
                    if(newscreenwidth<0)newscreenwidth=screenwidth;
                    if(newscreenheight<0)newscreenheight=screenheight;

                    SaveSettings(*this);
                    if(mainmenu==3&&gameon)mainmenu=2;
                    if(mainmenu==3&&!gameon)mainmenu=1;
                    break;
                case 9:
                    invertmouse=1-invertmouse;
                    break;
                case 10:
                    usermousesensitivity+=.2;
                    if(usermousesensitivity>2)usermousesensitivity=.2;
                    break;
                case 11:
                    volume+=.1f;
                    if(volume>1.0001f)volume=0;
                    OPENAL_SetSFXMasterVolume((int)(volume*255));
                    break;
                case 12:
                    flash();
                    
                    newstereomode = stereomode;
                    mainmenu=18;
                    keyselect=-1;
                    break;
                case 13:
                    showdamagebar=!showdamagebar;
                    break;
            }
        }
    }
    if(mainmenu==4){
        if(Input::MouseClicked()&&selected!=-1&&!waiting){
            fireSound();
            if(selected<9&&keyselect==-1)
                keyselect=selected;
            if(keyselect!=-1)
                setKeySelected();
            if(selected==9){
                flash();

                mainmenu=3;

                if(newdetail>2)newdetail=detail;
                if(newdetail<0)newdetail=detail;
                if(newscreenwidth>3000)newscreenwidth=screenwidth;
                if(newscreenwidth<0)newscreenwidth=screenwidth;
                if(newscreenheight>3000)newscreenheight=screenheight;
                if(newscreenheight<0)newscreenheight=screenheight;
            }
        }
    }

    if(mainmenu==5){

        if(endgame==2){
            accountactive->endGame();
            endgame=0;
        }

        if(Input::MouseClicked()){
            if((selected-7>=accountactive->getCampaignChoicesMade())){
                fireSound();
                flash();
                startbonustotal=0;

                loading=2;
                loadtime=0;
                targetlevel=7;
                if(firstload) TickOnceAfter();
                else LoadStuff();
                whichchoice=selected-7-accountactive->getCampaignChoicesMade();
                visibleloading=1;
                stillloading=1;
                Loadlevel(campaignmapname[campaignchoicewhich[selected-7-accountactive->getCampaignChoicesMade()]]);
                //Loadlevel(campaignmapname[levelorder[selected-7]]);
                campaign=1;
                mainmenu=0;
                gameon=1;
                pause_sound(stream_music3);
            }
            if(selected>=1 && selected<=5){
                fireSound();
                flash();
            }
            switch(selected){
                case 1:
                    startbonustotal=0;

                    loading=2;
                    loadtime=0;
                    targetlevel=-1;
                    if(firstload)TickOnceAfter();
                    if(!firstload)LoadStuff();
                    else {
                        Loadlevel(-1);
                    }

                    mainmenu=0;
                    gameon=1;
                    pause_sound(stream_music3);
                    break;
                case 2:
                    mainmenu=9;
                    break;
                case 3:
                    mainmenu=6;
                    break;
                case 4:
                    if(mainmenu==5&&gameon)mainmenu=2;
                    if(mainmenu==5&&!gameon)mainmenu=1;
                    break;
                case 5:
                    mainmenu=7;
                    break;
            }
        }
    }
    else if(mainmenu==9){
        if(Input::MouseClicked()&&selected<numchallengelevels&&selected>=0&&selected<=accountactive->getProgress()){
            fireSound();
            flash();

            startbonustotal=0;

            loading=2;
            loadtime=0;
            targetlevel=selected;
            if(firstload)TickOnceAfter();
            if(!firstload)LoadStuff();
            else {
                Loadlevel(selected);
            }
            campaign=0;

            mainmenu=0;
            gameon=1;
            pause_sound(stream_music3);
        }
        if(Input::MouseClicked()&&selected==numchallengelevels){
            fireSound();
            flash();
            mainmenu=5;
        }
    }
    if(mainmenu==10){
        endgame=2;
        if(Input::MouseClicked()&&selected==3){
            fireSound();
            flash();
            mainmenu=5;
        }
    }

    if(mainmenu==6){
        if(Input::MouseClicked()) {
            if(selected>-1){
                fireSound();
                if(selected==1) {
                    flash();
                    accountactive = Account::destroy(accountactive);
                    mainmenu=7;
                } else if(selected==2) {
                    flash();
                    mainmenu=5;
                }
            }
        }
    }
    if(mainmenu==7){
        if(Input::MouseClicked()) {
            if(selected!=-1){
                fireSound();
                if(selected==0&&Account::getNbAccounts()<8){
                    entername=1;
                } else if (selected<Account::getNbAccounts()+1) {
                    accountactive=Account::get(selected-1);
                    mainmenu=5;
                    flash();
                } else if (selected==Account::getNbAccounts()+1) {
                    flash();

                    mainmenu=1;

                    for(int j=0;j<255;j++){
                        displaytext[0][j]=' ';
                    }
                    displaychars[0]=0;
                    displayselected=0;
                    entername=0;
                }
            }
        }
    }
    if(mainmenu==8){
        if(Input::MouseClicked()&&selected>-1){
            fireSound();
            flash();
            if(selected<=2)
                accountactive->setDifficulty(selected);
            mainmenu=5;
        }
    }
    if (mainmenu==18) {			
        if(Input::MouseClicked()&&selected==0) {
            newstereomode = (StereoMode)(newstereomode + 1);
            while(!CanInitStereo(newstereomode)) {
                printf("Failed to initialize mode %s (%i)\n", StereoModeName(newstereomode), newstereomode);
                newstereomode = (StereoMode)(newstereomode + 1);
                if ( newstereomode >= stereoCount ) {
                    newstereomode = stereoNone;
                }
            }
        }
        
        if(Input::isKeyPressed(MOUSEBUTTON1)&&selected==1)
            stereoseparation+=0.001;
        if(Input::isKeyPressed(MOUSEBUTTON2)&&selected==1)
            stereoseparation-=0.001;

        if(Input::MouseClicked()&&selected==2) {
            stereoreverse =! stereoreverse;
        }
        
        if(Input::MouseClicked()&&selected==3) {
            flash();

            stereomode = newstereomode;
            InitStereo(stereomode);
            
            mainmenu=3;
        }
    }

    if(Input::isKeyDown(SDLK_q)&&Input::isKeyDown(SDLK_LMETA)){
        tryquit=1;
        if(mainmenu==3){
            if(newdetail>2)newdetail=detail;
            if(newdetail<0)newdetail=detail;
            if(newscreenwidth<0)newscreenwidth=screenwidth;
            if(newscreenheight<0)newscreenheight=screenheight;

            SaveSettings(*this);
        }
    }

    if(mainmenu==1||mainmenu==2){
        if(loaddistrib>4)transition+=multiplier/8;
        if(transition>1){
            transition=0;
            anim++;
            if(anim>4)anim=0;
            loaddistrib=0;
        }
    }
    OPENAL_SetFrequency(channels[stream_music3], 22050);

    if(entername) {
        inputText(displaytext[0],&displayselected,&displaychars[0]);
        if(!waiting) { // the input as finished
            if(displaychars[0]){ // with enter
                accountactive = Account::add(string(displaytext[0]));

                mainmenu=8;

                flash();

                fireSound(firestartsound);

                for(int i=0;i<255;i++){
                    displaytext[0][i]=' ';
                }
                displaychars[0]=0;

                displayselected=0;
            }
            entername=0;
        }
        
        displayblinkdelay-=multiplier;
        if(displayblinkdelay<=0){
            displayblinkdelay=.3;
            displayblink=1-displayblink;
        }
    }
}

void Game::doTutorial(){
    if(tutorialstagetime>tutorialmaxtime){
        tutorialstage++;
        tutorialsuccess=0;
        if(tutorialstage<=1){
            canattack=0;
            cananger=0;
            reversaltrain=0;
        }
        switch(tutorialstage){
            case 1:
                tutorialmaxtime=5;
            break; case 2:
                tutorialmaxtime=2;
            break; case 3:
                tutorialmaxtime=600;
            break; case 4:
                tutorialmaxtime=1000;
            break; case 5:
                tutorialmaxtime=600;
            break; case 6:
                tutorialmaxtime=600;
            break; case 7:
                tutorialmaxtime=600;
            break; case 8:
                tutorialmaxtime=600;
            break; case 9:
                tutorialmaxtime=600;
            break; case 10:
                tutorialmaxtime=2;
            break; case 11:
                tutorialmaxtime=1000;
            break; case 12:
                tutorialmaxtime=1000;
            break; case 13:
                tutorialmaxtime=2;
            break; case 14: {
                tutorialmaxtime=3;

                XYZ temp,temp2;

                temp.x=1011;
                temp.y=84;
                temp.z=491;
                temp2.x=1025;
                temp2.y=75;
                temp2.z=447;

                player[1].coords=(temp+temp2)/2;

                emit_sound_at(fireendsound, player[1].coords);

                for(int i=0;i<player[1].skeleton.num_joints;i++){
                    if(Random()%2==0){
                        if(!player[1].skeleton.free)temp2=(player[1].coords-player[1].oldcoords)/multiplier/2;//velocity/2;
                        if(player[1].skeleton.free)temp2=player[1].skeleton.joints[i].velocity*player[1].scale/2;
                        if(!player[1].skeleton.free)temp=DoRotation(DoRotation(DoRotation(player[1].skeleton.joints[i].position,0,0,player[1].tilt),player[1].tilt2,0,0),0,player[1].rotation,0)*player[1].scale+player[1].coords;
                        if(player[1].skeleton.free)temp=player[1].skeleton.joints[i].position*player[1].scale+player[1].coords;
                        Sprite::MakeSprite(breathsprite, temp,temp2, 1,1,1, .6+(float)abs(Random()%100)/200-.25, 1);
                    }
                }
            }
            break; case 15:
                tutorialmaxtime=500;
            break; case 16:
                tutorialmaxtime=500;
            break; case 17:
                tutorialmaxtime=500;
            break; case 18:
                tutorialmaxtime=500;
            break; case 19:
                tutorialstage=20;
                //tutorialmaxtime=500;
            break; case 20:
                tutorialmaxtime=500;
            break; case 21:
                tutorialmaxtime=500;
                if(bonus==cannon){
                    bonus=Slicebonus;
                    againbonus=1;
                }
                else againbonus=0;
            break; case 22:
                tutorialmaxtime=500;
            break; case 23:
                tutorialmaxtime=500;
            break; case 24:
                tutorialmaxtime=500;
            break; case 25:
                tutorialmaxtime=500;
            break; case 26:
                tutorialmaxtime=2;
            break; case 27:
                tutorialmaxtime=4;
                reversaltrain=1;
                cananger=1;
                player[1].aitype=attacktypecutoff;
            break; case 28:
                tutorialmaxtime=400;
            break; case 29:
                tutorialmaxtime=400;
                player[0].escapednum=0;
            break; case 30:
                tutorialmaxtime=4;
                reversaltrain=0;
                cananger=0;
                player[1].aitype=passivetype;
            break; case 31:
                tutorialmaxtime=13;
            break; case 32:
                tutorialmaxtime=8;
            break; case 33:
                tutorialmaxtime=400;
                cananger=1;
                canattack=1;
                player[1].aitype=attacktypecutoff;
            break; case 34:
                tutorialmaxtime=400;
            break; case 35:
                tutorialmaxtime=400;
            break; case 36:
                tutorialmaxtime=2;
                reversaltrain=0;
                cananger=0;
                player[1].aitype=passivetype;
            break; case 37:
                damagedealt=0;
                damagetaken=0;
                tutorialmaxtime=50;
                cananger=1;
                canattack=1;
                player[1].aitype=attacktypecutoff;
            break; case 38:
                tutorialmaxtime=4;
                canattack=0;
                cananger=0;
                player[1].aitype=passivetype;
            break; case 39: {
                XYZ temp,temp2;

                temp.x=1011;
                temp.y=84;
                temp.z=491;
                temp2.x=1025;
                temp2.y=75;
                temp2.z=447;


                weapons.owner[weapons.numweapons]=-1;
                weapons.type[weapons.numweapons]=knife;
                weapons.damage[weapons.numweapons]=0;
                weapons.mass[weapons.numweapons]=1;
                weapons.tipmass[weapons.numweapons]=1.2;
                weapons.length[weapons.numweapons]=.25;
                weapons.position[weapons.numweapons]=(temp+temp2)/2;
                weapons.tippoint[weapons.numweapons]=(temp+temp2)/2;

                weapons.velocity[weapons.numweapons]=0.1;
                weapons.tipvelocity[weapons.numweapons]=0.1;
                weapons.missed[weapons.numweapons]=1;
                weapons.hitsomething[weapons.numweapons]=0;
                weapons.freetime[weapons.numweapons]=0;
                weapons.firstfree[weapons.numweapons]=1;
                weapons.physics[weapons.numweapons]=1;

                weapons.numweapons++;
            }
            break; case 40:
                tutorialmaxtime=300;
            break; case 41:
                tutorialmaxtime=300;
            break; case 42:
                tutorialmaxtime=8;
            break; case 43:
                tutorialmaxtime=300;
            break; case 44:
                weapons.owner[0]=1;
                player[0].weaponactive=-1;
                player[0].num_weapons=0;
                player[1].weaponactive=0;
                player[1].num_weapons=1;
                player[1].weaponids[0]=0;

                cananger=1;
                canattack=1;
                player[1].aitype=attacktypecutoff;

                tutorialmaxtime=300;
            break; case 45:
                weapons.owner[0]=1;
                player[0].weaponactive=-1;
                player[0].num_weapons=0;
                player[1].weaponactive=0;
                player[1].num_weapons=1;
                player[1].weaponids[0]=0;

                tutorialmaxtime=300;
            break; case 46:
                weapons.owner[0]=1;
                player[0].weaponactive=-1;
                player[0].num_weapons=0;
                player[1].weaponactive=0;
                player[1].num_weapons=1;
                player[1].weaponids[0]=0;

                weapons.type[0]=sword;

                tutorialmaxtime=300;
            break; case 47: {
                tutorialmaxtime=10;

                XYZ temp,temp2;

                temp.x=1011;
                temp.y=84;
                temp.z=491;
                temp2.x=1025;
                temp2.y=75;
                temp2.z=447;

                weapons.owner[weapons.numweapons]=-1;
                weapons.type[weapons.numweapons]=sword;
                weapons.damage[weapons.numweapons]=0;
                weapons.mass[weapons.numweapons]=1;
                weapons.tipmass[weapons.numweapons]=1.2;
                weapons.length[weapons.numweapons]=.25;
                weapons.position[weapons.numweapons]=(temp+temp2)/2;
                weapons.tippoint[weapons.numweapons]=(temp+temp2)/2;

                weapons.velocity[weapons.numweapons]=0.1;
                weapons.tipvelocity[weapons.numweapons]=0.1;
                weapons.missed[weapons.numweapons]=1;
                weapons.hitsomething[weapons.numweapons]=0;
                weapons.freetime[weapons.numweapons]=0;
                weapons.firstfree[weapons.numweapons]=1;
                weapons.physics[weapons.numweapons]=1;

                weapons.owner[0]=1;
                weapons.owner[1]=0;
                player[0].weaponactive=0;
                player[0].num_weapons=1;
                player[0].weaponids[0]=1;
                player[1].weaponactive=0;
                player[1].num_weapons=1;
                player[1].weaponids[0]=0;

                weapons.numweapons++;
            }
            break; case 48:
                canattack=0;
                cananger=0;
                player[1].aitype=passivetype;

                tutorialmaxtime=15;

                weapons.owner[0]=1;
                weapons.owner[1]=0;
                player[0].weaponactive=0;
                player[0].num_weapons=1;
                player[0].weaponids[0]=1;
                player[1].weaponactive=0;
                player[1].num_weapons=1;
                player[1].weaponids[0]=0;

                if(player[0].weaponactive!=-1)weapons.type[player[0].weaponids[player[0].weaponactive]]=staff;
                else weapons.type[0]=staff;

                weapons.numweapons++;
            break; case 49:
                canattack=0;
                cananger=0;
                player[1].aitype=passivetype;

                tutorialmaxtime=200;

                weapons.position[1]=1000;
                weapons.tippoint[1]=1000;

                weapons.numweapons=1;
                weapons.owner[0]=0;
                player[1].weaponactive=-1;
                player[1].num_weapons=0;
                player[0].weaponactive=0;
                player[0].num_weapons=1;
                player[0].weaponids[0]=0;

                weapons.type[0]=knife;

                weapons.numweapons++;
            break; case 50: {
                tutorialmaxtime=8;

                XYZ temp,temp2;
                emit_sound_at(fireendsound, player[1].coords);

                for(int i=0;i<player[1].skeleton.num_joints;i++){
                    if(Random()%2==0){
                        if(!player[1].skeleton.free)temp2=(player[1].coords-player[1].oldcoords)/multiplier/2;//velocity/2;
                        if(player[1].skeleton.free)temp2=player[1].skeleton.joints[i].velocity*player[1].scale/2;
                        if(!player[1].skeleton.free)temp=DoRotation(DoRotation(DoRotation(player[1].skeleton.joints[i].position,0,0,player[1].tilt),player[1].tilt2,0,0),0,player[1].rotation,0)*player[1].scale+player[1].coords;
                        if(player[1].skeleton.free)temp=player[1].skeleton.joints[i].position*player[1].scale+player[1].coords;
                        Sprite::MakeSprite(breathsprite, temp,temp2, 1,1,1, .6+(float)abs(Random()%100)/200-.25, 1);
                    }
                }

                player[1].num_weapons=0;
                player[1].weaponstuck=-1;
                player[1].weaponactive=-1;

                weapons.numweapons=0;

                weapons.owner[0]=-1;
                weapons.velocity[0]=0.1;
                weapons.tipvelocity[0]=-0.1;
                weapons.missed[0]=1;
                weapons.hitsomething[0]=0;
                weapons.freetime[0]=0;
                weapons.firstfree[0]=1;
                weapons.physics[0]=1;
            }
            break; case 51:
                tutorialmaxtime=80000;
            break; default: break;
        }
        if(tutorialstage<=51)tutorialstagetime=0;
    }

    //Tutorial success
    if(tutorialstagetime<tutorialmaxtime-3){
        switch(tutorialstage){
            case 3: if(deltah||deltav)tutorialsuccess+=multiplier;
            break; case 4: if(player[0].forwardkeydown||player[0].backkeydown||player[0].leftkeydown||player[0].rightkeydown)tutorialsuccess+=multiplier;
            break; case 5: if(player[0].jumpkeydown)tutorialsuccess=1;
            break; case 6: if(player[0].isCrouch())tutorialsuccess=1;
            break; case 7: if(player[0].targetanimation==rollanim)tutorialsuccess=1;
            break; case 8: if(player[0].targetanimation==sneakanim)tutorialsuccess+=multiplier;
            break; case 9: if(player[0].targetanimation==rabbitrunninganim||player[0].targetanimation==wolfrunninganim)tutorialsuccess+=multiplier;
            break; case 11: if(player[0].isWallJump())tutorialsuccess=1;
            break; case 12: if(player[0].targetanimation==flipanim)tutorialsuccess=1;
            break; case 15: if(player[0].targetanimation==upunchanim||player[0].targetanimation==winduppunchanim)tutorialsuccess=1;
            break; case 16: if(player[0].targetanimation==winduppunchanim)tutorialsuccess=1;
            break; case 17: if(player[0].targetanimation==spinkickanim)tutorialsuccess=1;
            break; case 18: if(player[0].targetanimation==sweepanim)tutorialsuccess=1;
            break; case 19: if(player[0].targetanimation==dropkickanim)tutorialsuccess=1;
            break; case 20: if(player[0].targetanimation==rabbitkickanim)tutorialsuccess=1;
            break; case 21: if(bonus==cannon)tutorialsuccess=1;
            break; case 22: if(bonus==spinecrusher)tutorialsuccess=1;
            break; case 23: if(player[0].targetanimation==walljumprightkickanim||player[0].targetanimation==walljumpleftkickanim)tutorialsuccess=1;
            break; case 24: if(player[0].targetanimation==rabbittacklinganim)tutorialsuccess=1;
            break; case 25: if(player[0].targetanimation==backhandspringanim)tutorialsuccess=1;
            break; case 28: if(animation[player[0].targetanimation].attack==reversed&&player[0].feint)tutorialsuccess=1;
            break; case 29:
                if(player[0].escapednum==2){
                    tutorialsuccess=1;
                    reversaltrain=0;
                    cananger=0;
                    player[1].aitype=passivetype;
                }
            break; case 33: if(animation[player[0].targetanimation].attack==reversal)tutorialsuccess=1;
            break; case 34: if(animation[player[0].targetanimation].attack==reversal)tutorialsuccess=1;
            break; case 35:
                if(animation[player[0].targetanimation].attack==reversal){
                    tutorialsuccess=1;
                    reversaltrain=0;
                    cananger=0;
                    player[1].aitype=passivetype;
                }
            break; case 40: if(player[0].num_weapons>0)tutorialsuccess=1;
            break; case 41: if(player[0].weaponactive==-1&&player[0].num_weapons>0)tutorialsuccess=1;
            break; case 43: if(player[0].targetanimation==knifeslashstartanim)tutorialsuccess=1;
            break; case 44: if(animation[player[0].targetanimation].attack==reversal)tutorialsuccess=1;
            break; case 45: if(animation[player[0].targetanimation].attack==reversal)tutorialsuccess=1;
            break; case 46: if(animation[player[0].targetanimation].attack==reversal)tutorialsuccess=1;
            break; case 49: if(player[1].weaponstuck!=-1)tutorialsuccess=1;
            break; default: break;
        }
        if(tutorialsuccess>=1)tutorialstagetime=tutorialmaxtime-3;


        if(tutorialstagetime==tutorialmaxtime-3){
            emit_sound_np(consolesuccesssound);
        }

        if(tutorialsuccess>=1){
            if(tutorialstage==34||tutorialstage==35)
                tutorialstagetime=tutorialmaxtime-1;
        }
    }

    if(tutorialstage<14||tutorialstage>=50){
        player[1].coords.y=300;
        player[1].velocity=0;
    }
}



void Game::doDebugKeys(){
	float headprop,bodyprop,armprop,legprop;
    if(debugmode){
		if(Input::isKeyPressed(SDLK_v)){
			freeze=1-freeze;
			if(freeze){
				OPENAL_SetFrequency(OPENAL_ALL, 0.001);
			}
		}

		if(Input::isKeyPressed(SDLK_BACKQUOTE)){
			console=1-console;
			if(console){
				OPENAL_SetFrequency(OPENAL_ALL, 0.001);
			} else {
				freeze=0;
				waiting=false;
			}
		}

		if(console)
            freeze=1;
		if(console&&!Input::isKeyDown(SDLK_LMETA)){
			inputText(consoletext[0],&consoleselected,&consolechars[0]);
			if(!waiting) {
				archiveselected=0;
				cmd_dispatch(this, consoletext[0]);
				if(consolechars[0]>0){

					for(int k=14;k>=1;k--){
						for(int j=0;j<255;j++){
							consoletext[k][j]=consoletext[k-1][j];
						}
						consolechars[k]=consolechars[k-1];
					}
					for(int j=0;j<255;j++){
						consoletext[0][j]=' ';
					}
					consolechars[0]=0;
					consoleselected=0;
				}
			}

			consoleblinkdelay-=multiplier;
			if(consoleblinkdelay<=0){
				consoleblinkdelay=.3;
				consoleblink=1-consoleblink;
			}
		}



        if(Input::isKeyPressed(SDLK_h)){
            player[0].damagetolerance=200000;
            player[0].damage=0;
            player[0].burnt=0;
            player[0].permanentdamage=0;
            player[0].superpermanentdamage=0;
        }

        if(Input::isKeyPressed(SDLK_j)){
            environment++;
            if(environment>2)
                environment=0;
            Setenvironment(environment);
        }

        if(Input::isKeyPressed(SDLK_c)){
            cameramode=1-cameramode;
        }

        if(Input::isKeyPressed(SDLK_x)&&!Input::isKeyDown(SDLK_LSHIFT)){
            if(player[0].num_weapons>0){
                if(weapons.type[player[0].weaponids[0]]==sword)weapons.type[player[0].weaponids[0]]=staff;
                else if(weapons.type[player[0].weaponids[0]]==staff)weapons.type[player[0].weaponids[0]]=knife;
                else weapons.type[player[0].weaponids[0]]=sword;
                if(weapons.type[player[0].weaponids[0]]==sword){
                    weapons.mass[player[0].weaponids[0]]=1.5;
                    weapons.tipmass[player[0].weaponids[0]]=1;
                    weapons.length[player[0].weaponids[0]]=.8;
                }
                if(weapons.type[player[0].weaponids[0]]==staff){
                    weapons.mass[player[0].weaponids[0]]=2;
                    weapons.tipmass[player[0].weaponids[0]]=1;
                    weapons.length[player[0].weaponids[0]]=1.5;
                }

                if(weapons.type[player[0].weaponids[0]]==knife){
                    weapons.mass[player[0].weaponids[0]]=1;
                    weapons.tipmass[player[0].weaponids[0]]=1.2;
                    weapons.length[player[0].weaponids[0]]=.25;
                }
            }
        }

        if(Input::isKeyPressed(SDLK_x)&&Input::isKeyDown(SDLK_LSHIFT)){
            int closest=-1;
            float closestdist=-1;
            float distance;
            if(numplayers>1)
                for(int i=1;i<numplayers;i++){
                    distance=findDistancefast(&player[i].coords,&player[0].coords);
                    if(closestdist==-1||distance<closestdist){
                        closestdist=distance;
                        closest=i;
                    }
                }
            if(closest!=-1){
                if(player[closest].num_weapons){
                    if(weapons.type[player[closest].weaponids[0]]==sword)
                        weapons.type[player[closest].weaponids[0]]=staff;
                    else if(weapons.type[player[closest].weaponids[0]]==staff)
                        weapons.type[player[closest].weaponids[0]]=knife;
                    else weapons.type[player[closest].weaponids[0]]=sword;
                    if(weapons.type[player[closest].weaponids[0]]==sword){
                        weapons.mass[player[closest].weaponids[0]]=1.5;
                        weapons.tipmass[player[closest].weaponids[0]]=1;
                        weapons.length[player[closest].weaponids[0]]=.8;
                    }
                    if(weapons.type[player[0].weaponids[0]]==staff){
                        weapons.mass[player[0].weaponids[0]]=2;
                        weapons.tipmass[player[0].weaponids[0]]=1;
                        weapons.length[player[0].weaponids[0]]=1.5;
                    }
                    if(weapons.type[player[closest].weaponids[0]]==knife){
                        weapons.mass[player[closest].weaponids[0]]=1;
                        weapons.tipmass[player[closest].weaponids[0]]=1.2;
                        weapons.length[player[closest].weaponids[0]]=.25;
                    }
                }
                if(!player[closest].num_weapons){
                    player[closest].weaponids[0]=weapons.numweapons;
                    weapons.owner[weapons.numweapons]=closest;
                    weapons.type[weapons.numweapons]=knife;
                    weapons.damage[weapons.numweapons]=0;
                    weapons.numweapons++;
                    player[closest].num_weapons=1;
                    if(weapons.type[player[closest].weaponids[0]]==sword){
                        weapons.mass[player[closest].weaponids[0]]=1.5;
                        weapons.tipmass[player[closest].weaponids[0]]=1;
                        weapons.length[player[closest].weaponids[0]]=.8;
                    }
                    if(weapons.type[player[closest].weaponids[0]]==knife){
                        weapons.mass[player[closest].weaponids[0]]=1;
                        weapons.tipmass[player[closest].weaponids[0]]=1.2;
                        weapons.length[player[closest].weaponids[0]]=.25;
                    }
                }
            }
        }

        if(Input::isKeyDown(SDLK_u)){
            int closest=-1;
            float closestdist=-1;
            float distance;
            if(numplayers>1)
                for(int i=1;i<numplayers;i++){
                    distance=findDistancefast(&player[i].coords,&player[0].coords);
                    if(closestdist==-1||distance<closestdist){
                        closestdist=distance;
                        closest=i;
                    }
                }
            player[closest].rotation+=multiplier*50;
            player[closest].targetrotation=player[closest].rotation;
        }


        if(Input::isKeyPressed(SDLK_o)&&!Input::isKeyDown(SDLK_LSHIFT)){
            int closest=-1;
            float closestdist=-1;
            float distance;
            if(numplayers>1)
                for(int i=1;i<numplayers;i++){
                    distance=findDistancefast(&player[i].coords,&player[0].coords);
                    if(closestdist==-1||distance<closestdist){
                        closestdist=distance;
                        closest=i;
                    }
                }
            if(Input::isKeyDown(SDLK_LCTRL))closest=0;

            if(closest!=-1){
                player[closest].whichskin++;
                if(player[closest].whichskin>9)
                    player[closest].whichskin=0;
                if(player[closest].whichskin>2&&player[closest].creature==wolftype)
                    player[closest].whichskin=0;

                LoadTextureSave(creatureskin[player[closest].creature][player[closest].whichskin],
                        &player[closest].skeleton.drawmodel.textureptr,1,&player[closest].skeleton.skinText[0],&player[closest].skeleton.skinsize);
            }

            if(player[closest].numclothes){
                for(int i=0;i<player[closest].numclothes;i++){
                    tintr=player[closest].clothestintr[i];
                    tintg=player[closest].clothestintg[i];
                    tintb=player[closest].clothestintb[i];
                    AddClothes((char *)player[closest].clothes[i],0,1,&player[closest].skeleton.skinText[0],&player[closest].skeleton.skinsize);
                }
                player[closest].DoMipmaps();
            }
        }

        if(Input::isKeyPressed(SDLK_o)&&Input::isKeyDown(SDLK_LSHIFT)){
            int closest=-1;
            float closestdist=-1;
            float distance;
            if(numplayers>1)
                for(int i=1;i<numplayers;i++){
                    distance=findDistancefast(&player[i].coords,&player[0].coords);
                    if(closestdist==-1||distance<closestdist){
                        closestdist=distance;
                        closest=i;
                    }
                }
            if(closest!=-1){
                if(player[closest].creature==wolftype){
                    headprop=player[closest].proportionhead.x/1.1;
                    bodyprop=player[closest].proportionbody.x/1.1;
                    armprop=player[closest].proportionarms.x/1.1;
                    legprop=player[closest].proportionlegs.x/1.1;
                }

                if(player[closest].creature==rabbittype){
                    headprop=player[closest].proportionhead.x/1.2;
                    bodyprop=player[closest].proportionbody.x/1.05;
                    armprop=player[closest].proportionarms.x/1.00;
                    legprop=player[closest].proportionlegs.x/1.1;
                }


                if(player[closest].creature==rabbittype){
                    player[closest].skeleton.id=closest;
                    player[closest].skeleton.Load((char *)":Data:Skeleton:Basic Figure Wolf",(char *)":Data:Skeleton:Basic Figure Wolf Low",(char *)":Data:Skeleton:Rabbitbelt",(char *)":Data:Models:Wolf.solid",(char *)":Data:Models:Wolf2.solid",(char *)":Data:Models:Wolf3.solid",(char *)":Data:Models:Wolf4.solid",(char *)":Data:Models:Wolf5.solid",(char *)":Data:Models:Wolf6.solid",(char *)":Data:Models:Wolf7.solid",(char *)":Data:Models:Wolflow.solid",(char *)":Data:Models:Belt.solid",0);
                    LoadTextureSave(":Data:Textures:Wolf.jpg",&player[closest].skeleton.drawmodel.textureptr,1,&player[closest].skeleton.skinText[closest],&player[closest].skeleton.skinsize);
                    player[closest].whichskin=0;
                    player[closest].creature=wolftype;

                    player[closest].proportionhead=1.1;
                    player[closest].proportionbody=1.1;
                    player[closest].proportionarms=1.1;
                    player[closest].proportionlegs=1.1;
                    player[closest].proportionlegs.y=1.1;
                    player[closest].scale=.23*5*player[0].scale;

                    player[closest].damagetolerance=300;
                }
                else
                {
                    player[closest].skeleton.id=closest;
                    player[closest].skeleton.Load((char *)":Data:Skeleton:Basic Figure",(char *)":Data:Skeleton:Basic Figurelow",(char *)":Data:Skeleton:Rabbitbelt",(char *)":Data:Models:Body.solid",(char *)":Data:Models:Body2.solid",(char *)":Data:Models:Body3.solid",(char *)":Data:Models:Body4.solid",(char *)":Data:Models:Body5.solid",(char *)":Data:Models:Body6.solid",(char *)":Data:Models:Body7.solid",(char *)":Data:Models:Bodylow.solid",(char *)":Data:Models:Belt.solid",1);
                    LoadTextureSave(":Data:Textures:Fur3.jpg",&player[closest].skeleton.drawmodel.textureptr,1,&player[closest].skeleton.skinText[0],&player[closest].skeleton.skinsize);
                    player[closest].whichskin=0;
                    player[closest].creature=rabbittype;

                    player[closest].proportionhead=1.2;
                    player[closest].proportionbody=1.05;
                    player[closest].proportionarms=1.00;
                    player[closest].proportionlegs=1.1;
                    player[closest].proportionlegs.y=1.05;
                    player[closest].scale=.2*5*player[0].scale;

                    player[closest].damagetolerance=200;
                }

                if(player[closest].creature==wolftype){
                    player[closest].proportionhead=1.1*headprop;
                    player[closest].proportionbody=1.1*bodyprop;
                    player[closest].proportionarms=1.1*armprop;
                    player[closest].proportionlegs=1.1*legprop;
                }

                if(player[closest].creature==rabbittype){
                    player[closest].proportionhead=1.2*headprop;
                    player[closest].proportionbody=1.05*bodyprop;
                    player[closest].proportionarms=1.00*armprop;
                    player[closest].proportionlegs=1.1*legprop;
                    player[closest].proportionlegs.y=1.05*legprop;
                }

            }
        }

        if(Input::isKeyPressed(SDLK_b)&&!Input::isKeyDown(SDLK_LSHIFT)){
            slomo=1-slomo;
            slomodelay=1000;
        }


        if(((Input::isKeyPressed(SDLK_i)&&!Input::isKeyDown(SDLK_LSHIFT)))){
            int closest=-1;
            float closestdist=-1;
            float distance;
            XYZ flatfacing2,flatvelocity2;
            XYZ blah;
            if(numplayers>1)
                for(int i=1;i<numplayers;i++){
                    distance=findDistancefast(&player[i].coords,&player[0].coords);
                    if(distance<144&&!player[i].headless)
                        if(closestdist==-1||distance<closestdist){
                            closestdist=distance;
                            closest=i;
                            blah = player[i].coords;
                        }
                }

            if(closest!=-1){
                XYZ headspurtdirection;
                //int i = player[closest].skeleton.jointlabels[head];
                Joint& headjoint=playerJoint(closest,head);
                for(int k=0;k<player[closest].skeleton.num_joints; k++){
                    if(!player[closest].skeleton.free)
                        flatvelocity2=player[closest].velocity;
                    if(player[closest].skeleton.free)
                        flatvelocity2=headjoint.velocity;
                    if(!player[closest].skeleton.free)
                        flatfacing2=DoRotation(DoRotation(DoRotation(headjoint.position,0,0,player[closest].tilt),player[closest].tilt2,0,0),0,player[closest].rotation,0)*player[closest].scale+player[closest].coords;
                    if(player[closest].skeleton.free)
                        flatfacing2=headjoint.position*player[closest].scale+player[closest].coords;
                    flatvelocity2.x+=(float)(abs(Random()%100)-50)/10;
                    flatvelocity2.y+=(float)(abs(Random()%100)-50)/10;
                    flatvelocity2.z+=(float)(abs(Random()%100)-50)/10;
                    headspurtdirection=headjoint.position-playerJoint(closest,neck).position;
                    Normalise(&headspurtdirection);
                    Sprite::MakeSprite(bloodflamesprite, flatfacing2,flatvelocity2, 1,1,1, .6, 1);
                    flatvelocity2+=headspurtdirection*8;
                    Sprite::MakeSprite(bloodsprite, flatfacing2,flatvelocity2/2, 1,1,1, .16, 1);
                }
                Sprite::MakeSprite(cloudsprite, flatfacing2,flatvelocity2*0, .6,0,0, 1, .5);

                emit_sound_at(splattersound, blah);
                emit_sound_at(breaksound2, blah, 100.);

                if(player[closest].skeleton.free==2)player[closest].skeleton.free=0;
                player[closest].RagDoll(0);
                player[closest].dead=2;
                player[closest].headless=1;
                player[closest].DoBloodBig(3,165);

                camerashake+=.3;
            }
        }

        if(((Input::isKeyPressed(SDLK_i)&&Input::isKeyDown(SDLK_LSHIFT)))){
            int closest=-1;
            float closestdist=-1;
            float distance;
            XYZ flatfacing2,flatvelocity2;
            XYZ blah;
            if(numplayers>1)
                for(int i=1;i<numplayers;i++){
                    distance=findDistancefast(&player[i].coords,&player[0].coords);
                    if(distance<144)
                        if(closestdist==-1||distance<closestdist){
                            closestdist=distance;
                            closest=i;
                            blah=player[i].coords;
                        }
                }

            if(closest!=-1){
                emit_sound_at(splattersound, blah);

                emit_sound_at(breaksound2, blah);

                for(int i=0;i<player[closest].skeleton.num_joints; i++){
                    if(!player[closest].skeleton.free)flatvelocity2=player[closest].velocity;
                    if(player[closest].skeleton.free)flatvelocity2=player[closest].skeleton.joints[i].velocity;
                    if(!player[closest].skeleton.free)flatfacing2=DoRotation(DoRotation(DoRotation(player[closest].skeleton.joints[i].position,0,0,player[closest].tilt),player[closest].tilt2,0,0),0,player[closest].rotation,0)*player[closest].scale+player[closest].coords;
                    if(player[closest].skeleton.free)flatfacing2=player[closest].skeleton.joints[i].position*player[closest].scale+player[closest].coords;
                    flatvelocity2.x+=(float)(abs(Random()%100)-50)/10;
                    flatvelocity2.y+=(float)(abs(Random()%100)-50)/10;
                    flatvelocity2.z+=(float)(abs(Random()%100)-50)/10;
                    Sprite::MakeSprite(bloodflamesprite, flatfacing2,flatvelocity2, 1,1,1, 3, 1);
                    Sprite::MakeSprite(bloodsprite, flatfacing2,flatvelocity2, 1,1,1, .3, 1);
                    Sprite::MakeSprite(cloudsprite, flatfacing2,flatvelocity2*0, .6,0,0, 1, .5);
                }

                for(int i=0;i<player[closest].skeleton.num_joints; i++){
                    if(!player[closest].skeleton.free)flatvelocity2=player[closest].velocity;
                    if(player[closest].skeleton.free)flatvelocity2=player[closest].skeleton.joints[i].velocity;
                    if(!player[closest].skeleton.free)flatfacing2=DoRotation(DoRotation(DoRotation(player[closest].skeleton.joints[i].position,0,0,player[closest].tilt),player[closest].tilt2,0,0),0,player[closest].rotation,0)*player[closest].scale+player[closest].coords;
                    if(player[closest].skeleton.free)flatfacing2=player[closest].skeleton.joints[i].position*player[closest].scale+player[closest].coords;
                    flatvelocity2.x+=(float)(abs(Random()%100)-50)/10;
                    flatvelocity2.y+=(float)(abs(Random()%100)-50)/10;
                    flatvelocity2.z+=(float)(abs(Random()%100)-50)/10;
                    Sprite::MakeSprite(bloodflamesprite, flatfacing2,flatvelocity2, 1,1,1, 3, 1);
                    Sprite::MakeSprite(bloodsprite, flatfacing2,flatvelocity2, 1,1,1, .4, 1);
                }

                for(int i=0;i<player[closest].skeleton.num_joints; i++){
                    if(!player[closest].skeleton.free)flatvelocity2=player[closest].velocity;
                    if(player[closest].skeleton.free)flatvelocity2=player[closest].skeleton.joints[i].velocity;
                    if(!player[closest].skeleton.free)flatfacing2=DoRotation(DoRotation(DoRotation(player[closest].skeleton.joints[i].position,0,0,player[closest].tilt),player[closest].tilt2,0,0),0,player[closest].rotation,0)*player[closest].scale+player[closest].coords;
                    if(player[closest].skeleton.free)flatfacing2=player[closest].skeleton.joints[i].position*player[closest].scale+player[closest].coords;
                    flatvelocity2.x+=(float)(abs(Random()%100)-50)/10;
                    flatvelocity2.y+=(float)(abs(Random()%100)-50)/10;
                    flatvelocity2.z+=(float)(abs(Random()%100)-50)/10;
                    Sprite::MakeSprite(bloodflamesprite, flatfacing2,flatvelocity2*2, 1,1,1, 3, 1);
                    Sprite::MakeSprite(bloodsprite, flatfacing2,flatvelocity2*2, 1,1,1, .4, 1);
                }

                for(int i=0;i<player[closest].skeleton.num_joints; i++){
                    if(!player[closest].skeleton.free)flatvelocity2=player[closest].velocity;
                    if(player[closest].skeleton.free)flatvelocity2=player[closest].skeleton.joints[i].velocity;
                    if(!player[closest].skeleton.free)flatfacing2=DoRotation(DoRotation(DoRotation(player[closest].skeleton.joints[i].position,0,0,player[closest].tilt),player[closest].tilt2,0,0),0,player[closest].rotation,0)*player[closest].scale+player[closest].coords;
                    if(player[closest].skeleton.free)flatfacing2=player[closest].skeleton.joints[i].position*player[closest].scale+player[closest].coords;
                    flatvelocity2.x+=(float)(abs(Random()%100)-50)/10;
                    flatvelocity2.y+=(float)(abs(Random()%100)-50)/10;
                    flatvelocity2.z+=(float)(abs(Random()%100)-50)/10;
                    Sprite::MakeSprite(bloodflamesprite, flatfacing2,flatvelocity2*2, 1,1,1, 3, 1);
                    Sprite::MakeSprite(bloodsprite, flatfacing2,flatvelocity2*2, 1,1,1, .4, 1);
                }

                XYZ temppos;
                for(int j=0;j<numplayers; j++){
                    if(j!=closest){
                        if(findDistancefast(&player[j].coords,&player[closest].coords)<25){
                            player[j].DoDamage((25-findDistancefast(&player[j].coords,&player[closest].coords))*60);
                            if(player[j].skeleton.free==2)
                                player[j].skeleton.free=1;
                            player[j].skeleton.longdead=0;
                            player[j].RagDoll(0);
                            for(int i=0;i<player[j].skeleton.num_joints; i++){
                                temppos=player[j].skeleton.joints[i].position+player[j].coords;
                                if(findDistancefast(&temppos,&player[closest].coords)<25){
                                    flatvelocity2=temppos-player[closest].coords;
                                    Normalise(&flatvelocity2);
                                    player[j].skeleton.joints[i].velocity+=flatvelocity2*((20-findDistancefast(&temppos,&player[closest].coords))*20);
                                }
                            }
                        }
                    }
                }

                player[closest].DoDamage(10000);
                player[closest].RagDoll(0);
                player[closest].dead=2;
                player[closest].coords=20;
                player[closest].skeleton.free=2;

                camerashake+=.6;

            }
        }

        if(Input::isKeyPressed(SDLK_f)){
            player[0].onfire=1-player[0].onfire;
            if(player[0].onfire){
                player[0].CatchFire();
            }
            if(!player[0].onfire){
                emit_sound_at(fireendsound, player[0].coords);
                pause_sound(stream_firesound);
            }
        }

        if(Input::isKeyPressed(SDLK_n)&&!Input::isKeyDown(SDLK_LCTRL)){
            //if(!player[0].skeleton.free)player[0].damage+=500;
            player[0].RagDoll(0);
            //player[0].spurt=1;
            //player[0].DoDamage(1000);

            emit_sound_at(whooshsound, player[0].coords, 128.);
        }

        if(Input::isKeyPressed(SDLK_n)&&Input::isKeyDown(SDLK_LCTRL)){
            int closest=-1;
            float closestdist=-1;
            float distance;
            for(int i=0;i<objects.numobjects;i++){
                if(objects.type[i]==treeleavestype){
                    objects.scale[i]*=.9;
                }
            }
        }

        if(Input::isKeyPressed(SDLK_m)&&Input::isKeyDown(SDLK_LSHIFT)){
            editorenabled=1-editorenabled;
            if(editorenabled){
                player[0].damagetolerance=100000;
            } else {
                player[0].damagetolerance=200;
            }
            player[0].damage=0; // these lines were in both if and else, but I think they would better fit in the if
            player[0].permanentdamage=0;
            player[0].superpermanentdamage=0;
            player[0].bloodloss=0;
            player[0].deathbleeding=0;
        }

        //skip level
        if(whichlevel!=-2&&Input::isKeyPressed(SDLK_k)&&Input::isKeyDown(SDLK_LSHIFT)&&!editorenabled){
            targetlevel++;
            if(targetlevel>numchallengelevels-1)targetlevel=0;
            loading=1;
            leveltime=5;
        }

        if(editorenabled){
            if(Input::isKeyPressed(SDLK_DELETE)&&Input::isKeyDown(SDLK_LSHIFT)){
                int closest=-1;
                float closestdist=-1;
                float distance;
                if(numplayers>1)
                    for(int i=1;i<numplayers;i++){
                        distance=findDistancefast(&player[i].coords,&player[0].coords);
                        if(closestdist==-1||distance<closestdist){
                            closestdist=distance;
                            closest=i;
                        }
                    }
                if(closestdist>0&&closest>=0){
                    //player[closest]=player[numplayers-1];
                    //player[closest].skeleton=player[numplayers-1].skeleton;
                    numplayers--;
                }
            }

            if(Input::isKeyPressed(SDLK_DELETE)&&Input::isKeyDown(SDLK_LCTRL)){
                int closest=-1;
                float closestdist=-1;
                float distance;
                if(max_objects>1)
                    for(int i=1;i<max_objects;i++){
                        distance=findDistancefast(&objects.position[i],&player[0].coords);
                        if(closestdist==-1||distance<closestdist){
                            closestdist=distance;
                            closest=i;
                        }
                    }
                if(closestdist>0&&closest>=0){
                    objects.position[closest].y-=500;
                }
            }

            if(Input::isKeyPressed(SDLK_m)&&Input::isKeyDown(SDLK_LSHIFT)){
                //drawmode++;
                //if(drawmode>2)drawmode=0;
                if(objects.numobjects<max_objects-1){
                    XYZ boxcoords;
                    boxcoords.x=player[0].coords.x;
                    boxcoords.z=player[0].coords.z;
                    boxcoords.y=player[0].coords.y-3;
                    if(editortype==bushtype)boxcoords.y=player[0].coords.y-.5;
                    if(editortype==firetype)boxcoords.y=player[0].coords.y-.5;
                    //objects.MakeObject(abs(Random()%3),boxcoords,Random()%360);
                    float temprotat,temprotat2;
                    temprotat=editorrotation;
                    temprotat2=editorrotation2;
                    if(temprotat<0||editortype==bushtype)temprotat=Random()%360;
                    if(temprotat2<0)temprotat2=Random()%360;

                    objects.MakeObject(editortype,boxcoords,(int)temprotat-((int)temprotat)%30,(int)temprotat2,editorsize);
                    if(editortype==treetrunktype)
                        objects.MakeObject(treeleavestype,boxcoords,Random()%360*(temprotat2<2)+(int)editorrotation-((int)editorrotation)%30,editorrotation2,editorsize);
                }
            }

            if(Input::isKeyPressed(SDLK_p)&&Input::isKeyDown(SDLK_LSHIFT)&&!Input::isKeyDown(SDLK_LCTRL)){
                if(numplayers<maxplayers-1){
                    player[numplayers].scale=.2*5*player[0].scale;
                    player[numplayers].creature=rabbittype;
                    player[numplayers].howactive=editoractive;
                    player[numplayers].skeleton.id=numplayers;
                    player[numplayers].skeleton.Load((char *)":Data:Skeleton:Basic Figure",(char *)":Data:Skeleton:Basic Figurelow",(char *)":Data:Skeleton:Rabbitbelt",(char *)":Data:Models:Body.solid",(char *)":Data:Models:Body2.solid",(char *)":Data:Models:Body3.solid",(char *)":Data:Models:Body4.solid",(char *)":Data:Models:Body5.solid",(char *)":Data:Models:Body6.solid",(char *)":Data:Models:Body7.solid",(char *)":Data:Models:Bodylow.solid",(char *)":Data:Models:Belt.solid",1);

                    //texsize=512*512*3/texdetail/texdetail;
                    //if(!player[numplayers].loaded)player[numplayers].skeleton.skinText = new GLubyte[texsize];
                    //player[numplayers].skeleton.skinText.resize(texsize);

                    int k=abs(Random()%2)+1;
                    if(k==0){
                        LoadTextureSave(":Data:Textures:Fur3.jpg",&player[numplayers].skeleton.drawmodel.textureptr,1,&player[numplayers].skeleton.skinText[0],&player[numplayers].skeleton.skinsize);
                        player[numplayers].whichskin=0;
                    }
                    else if(k==1){
                        LoadTextureSave(":Data:Textures:Fur.jpg",&player[numplayers].skeleton.drawmodel.textureptr,1,&player[numplayers].skeleton.skinText[0],&player[numplayers].skeleton.skinsize);
                        player[numplayers].whichskin=1;
                    }
                    else {
                        LoadTextureSave(":Data:Textures:Fur2.jpg",&player[numplayers].skeleton.drawmodel.textureptr,1,&player[numplayers].skeleton.skinText[0],&player[numplayers].skeleton.skinsize);
                        player[numplayers].whichskin=2;
                    }

                    LoadTexture(":Data:Textures:Belt.png",&player[numplayers].skeleton.drawmodelclothes.textureptr,1,1);
                    player[numplayers].power=1;
                    player[numplayers].speedmult=1;
                    player[numplayers].currentanimation=bounceidleanim;
                    player[numplayers].targetanimation=bounceidleanim;
                    player[numplayers].currentframe=0;
                    player[numplayers].targetframe=1;
                    player[numplayers].target=0;
                    player[numplayers].bled=0;
                    player[numplayers].speed=1+(float)(Random()%100)/1000;

                    player[numplayers].targetrotation=player[0].targetrotation;
                    player[numplayers].rotation=player[0].rotation;

                    player[numplayers].velocity=0;
                    player[numplayers].coords=player[0].coords;
                    player[numplayers].oldcoords=player[numplayers].coords;
                    player[numplayers].realoldcoords=player[numplayers].coords;

                    player[numplayers].id=numplayers;
                    player[numplayers].skeleton.id=numplayers;
                    player[numplayers].updatedelay=0;
                    player[numplayers].normalsupdatedelay=0;

                    player[numplayers].aitype=passivetype;

                    if(player[0].creature==wolftype){
                        headprop=player[0].proportionhead.x/1.1;
                        bodyprop=player[0].proportionbody.x/1.1;
                        armprop=player[0].proportionarms.x/1.1;
                        legprop=player[0].proportionlegs.x/1.1;
                    }

                    if(player[0].creature==rabbittype){
                        headprop=player[0].proportionhead.x/1.2;
                        bodyprop=player[0].proportionbody.x/1.05;
                        armprop=player[0].proportionarms.x/1.00;
                        legprop=player[0].proportionlegs.x/1.1;
                    }

                    if(player[numplayers].creature==wolftype){
                        player[numplayers].proportionhead=1.1*headprop;
                        player[numplayers].proportionbody=1.1*bodyprop;
                        player[numplayers].proportionarms=1.1*armprop;
                        player[numplayers].proportionlegs=1.1*legprop;
                    }

                    if(player[numplayers].creature==rabbittype){
                        player[numplayers].proportionhead=1.2*headprop;
                        player[numplayers].proportionbody=1.05*bodyprop;
                        player[numplayers].proportionarms=1.00*armprop;
                        player[numplayers].proportionlegs=1.1*legprop;
                        player[numplayers].proportionlegs.y=1.05*legprop;
                    }

                    player[numplayers].headless=0;
                    player[numplayers].onfire=0;

                    if(cellophane){
                        player[numplayers].proportionhead.z=0;
                        player[numplayers].proportionbody.z=0;
                        player[numplayers].proportionarms.z=0;
                        player[numplayers].proportionlegs.z=0;
                    }

                    player[numplayers].tempanimation.Load((char *)"Tempanim",0,0);

                    player[numplayers].damagetolerance=200;

                    player[numplayers].protectionhead=player[0].protectionhead;
                    player[numplayers].protectionhigh=player[0].protectionhigh;
                    player[numplayers].protectionlow=player[0].protectionlow;
                    player[numplayers].armorhead=player[0].armorhead;
                    player[numplayers].armorhigh=player[0].armorhigh;
                    player[numplayers].armorlow=player[0].armorlow;
                    player[numplayers].metalhead=player[0].metalhead;
                    player[numplayers].metalhigh=player[0].metalhigh;
                    player[numplayers].metallow=player[0].metallow;

                    player[numplayers].immobile=player[0].immobile;

                    player[numplayers].numclothes=player[0].numclothes;
                    if(player[numplayers].numclothes)
                        for(int i=0;i<player[numplayers].numclothes;i++){
                            strcpy(player[numplayers].clothes[i], player[0].clothes[i]);
                            player[numplayers].clothestintr[i]=player[0].clothestintr[i];
                            player[numplayers].clothestintg[i]=player[0].clothestintg[i];
                            player[numplayers].clothestintb[i]=player[0].clothestintb[i];
                            tintr=player[numplayers].clothestintr[i];
                            tintg=player[numplayers].clothestintg[i];
                            tintb=player[numplayers].clothestintb[i];
                            AddClothes((char *)player[numplayers].clothes[i],0,1,&player[numplayers].skeleton.skinText[0],&player[numplayers].skeleton.skinsize);
                        }
                    if(player[numplayers].numclothes){
                        player[numplayers].DoMipmaps();
                    }

                    player[numplayers].power=player[0].power;
                    player[numplayers].speedmult=player[0].speedmult;

                    player[numplayers].damage=0;
                    player[numplayers].permanentdamage=0;
                    player[numplayers].superpermanentdamage=0;
                    player[numplayers].deathbleeding=0;
                    player[numplayers].bleeding=0;
                    player[numplayers].numwaypoints=0;
                    player[numplayers].waypoint=0;
                    player[numplayers].jumppath=0;
                    player[numplayers].weaponstuck=-1;
                    player[numplayers].weaponactive=-1;
                    player[numplayers].num_weapons=0;
                    player[numplayers].bloodloss=0;
                    player[numplayers].dead=0;

                    player[numplayers].loaded=1;

                    numplayers++;
                }
            }

            if(Input::isKeyPressed(SDLK_p)&&Input::isKeyDown(SDLK_LSHIFT)){
                if(player[numplayers-1].numwaypoints<90){
                    player[numplayers-1].waypoints[player[numplayers-1].numwaypoints]=player[0].coords;
                    player[numplayers-1].waypointtype[player[numplayers-1].numwaypoints]=editorpathtype;
                    player[numplayers-1].numwaypoints++;
                }
            }

            if(Input::isKeyPressed(SDLK_p)&&Input::isKeyDown(SDLK_LCTRL)){
                if(numpathpoints<30){
                    bool connected,alreadyconnected;
                    connected=0;
                    if(numpathpoints>1)
                        for(int i=0;i<numpathpoints;i++){
                            if(findDistancefast(&pathpoint[i],&player[0].coords)<.5&&i!=pathpointselected&&!connected){
                                alreadyconnected=0;
                                for(int j=0;j<numpathpointconnect[pathpointselected];j++){
                                    if(pathpointconnect[pathpointselected][j]==i)alreadyconnected=1;
                                }
                                if(!alreadyconnected){
                                    numpathpointconnect[pathpointselected]++;
                                    connected=1;
                                    pathpointconnect[pathpointselected][numpathpointconnect[pathpointselected]-1]=i;
                                }
                            }
                        }
                    if(!connected){
                        numpathpoints++;
                        pathpoint[numpathpoints-1]=player[0].coords;
                        numpathpointconnect[numpathpoints-1]=0;
                        if(numpathpoints>1&&pathpointselected!=-1){
                            numpathpointconnect[pathpointselected]++;
                            pathpointconnect[pathpointselected][numpathpointconnect[pathpointselected]-1]=numpathpoints-1;
                        }
                        pathpointselected=numpathpoints-1;
                    }
                }
            }

            if(Input::isKeyPressed(SDLK_PERIOD)){
                pathpointselected++;
                if(pathpointselected>=numpathpoints)pathpointselected=-1;
            }
            if(Input::isKeyPressed(SDLK_COMMA)&&!Input::isKeyDown(SDLK_LSHIFT)){
                pathpointselected--;
                if(pathpointselected<=-2)
                    pathpointselected=numpathpoints-1;
            }
            if(Input::isKeyPressed(SDLK_COMMA)&&Input::isKeyDown(SDLK_LSHIFT)){
                if(pathpointselected!=-1){
                    numpathpoints--;
                    pathpoint[pathpointselected]=pathpoint[numpathpoints];
                    numpathpointconnect[pathpointselected]=numpathpointconnect[numpathpoints];
                    for(int i=0;i<numpathpointconnect[pathpointselected];i++){
                        pathpointconnect[pathpointselected][i]=pathpointconnect[numpathpoints][i];
                    }
                    for(int i=0;i<numpathpoints;i++){
                        for(int j=0;j<numpathpointconnect[i];j++){
                            if(pathpointconnect[i][j]==pathpointselected){
                                pathpointconnect[i][j]=pathpointconnect[i][numpathpointconnect[i]-1];
                                numpathpointconnect[i]--;
                            }
                            if(pathpointconnect[i][j]==numpathpoints){
                                pathpointconnect[i][j]=pathpointselected;
                            }
                        }
                    }
                    pathpointselected=numpathpoints-1;
                }
            }

            if(Input::isKeyPressed(SDLK_LEFT)&&Input::isKeyDown(SDLK_LSHIFT)&&!Input::isKeyDown(SDLK_LCTRL)){
                editortype--;
                if(editortype==treeleavestype||editortype==10)editortype--;
                if(editortype<0)editortype=firetype;
            }

            if(Input::isKeyPressed(SDLK_RIGHT)&&Input::isKeyDown(SDLK_LSHIFT)&&!Input::isKeyDown(SDLK_LCTRL)){
                editortype++;
                if(editortype==treeleavestype||editortype==10)editortype++;
                if(editortype>firetype)editortype=0;
            }

            if(Input::isKeyDown(SDLK_LEFT)&&!Input::isKeyDown(SDLK_LSHIFT)&&!Input::isKeyDown(SDLK_LCTRL)){
                editorrotation-=multiplier*100;
                if(editorrotation<-.01)editorrotation=-.01;
            }

            if(Input::isKeyDown(SDLK_RIGHT)&&!Input::isKeyDown(SDLK_LSHIFT)&&!Input::isKeyDown(SDLK_LCTRL)){
                editorrotation+=multiplier*100;
            }

            if(Input::isKeyDown(SDLK_UP)&&!Input::isKeyDown(SDLK_LCTRL)){
                editorsize+=multiplier;
            }

            if(Input::isKeyDown(SDLK_DOWN)&&!Input::isKeyDown(SDLK_LCTRL)){
                editorsize-=multiplier;
                if(editorsize<.1)editorsize=.1;
            }


            if(Input::isKeyPressed(SDLK_LEFT)&&Input::isKeyDown(SDLK_LSHIFT)&&Input::isKeyDown(SDLK_LCTRL)){
                mapradius-=multiplier*10;
            }

            if(Input::isKeyPressed(SDLK_RIGHT)&&Input::isKeyDown(SDLK_LSHIFT)&&Input::isKeyDown(SDLK_LCTRL)){
                mapradius+=multiplier*10;
            }
            if(Input::isKeyDown(SDLK_UP)&&Input::isKeyDown(SDLK_LCTRL)){
                editorrotation2+=multiplier*100;
            }

            if(Input::isKeyDown(SDLK_DOWN)&&Input::isKeyDown(SDLK_LCTRL)){
                editorrotation2-=multiplier*100;
                if(editorrotation2<-.01)editorrotation2=-.01;
            }
            if(Input::isKeyPressed(SDLK_DELETE)&&objects.numobjects&&Input::isKeyDown(SDLK_LSHIFT)){
                int closest=-1;
                float closestdist=-1;
                float distance;
                for(int i=0;i<objects.numobjects;i++){
                    distance=findDistancefast(&objects.position[i],&player[0].coords);
                    if(closestdist==-1||distance<closestdist){
                        closestdist=distance;
                        closest=i;
                    }
                }
                if(closestdist>0&&closest>=0)objects.DeleteObject(closest);
            }
        }
    }
}



void Game::doJumpReversals(){
    for(int k=0;k<numplayers;k++)
        for(int i=k;i<numplayers;i++){
            if(i==k)continue;
            if(     player[k].skeleton.free==0&&
                    player[i].skeleton.oldfree==0&&
                    (player[i].targetanimation==jumpupanim||
                     player[k].targetanimation==jumpupanim)&&
                    (player[i].aitype==playercontrolled||
                     player[k].aitype==playercontrolled)&&
                    (player[i].aitype==attacktypecutoff&&player[i].stunned<=0||
                     player[k].aitype==attacktypecutoff&&player[k].stunned<=0)){
                if(     findDistancefast(&player[i].coords,&player[k].coords)<10*sq((player[i].scale+player[k].scale)*2.5)&&
                        findDistancefastflat(&player[i].coords,&player[k].coords)<2*sq((player[i].scale+player[k].scale)*2.5)){
                    //TODO: refactor two huge similar ifs
                    if(player[i].targetanimation==jumpupanim&&
                            player[k].targetanimation!=getupfrombackanim&&
                            player[k].targetanimation!=getupfromfrontanim&&
                            animation[player[k].targetanimation].height==middleheight&&
                            normaldotproduct(player[i].velocity,player[k].coords-player[i].coords)<0&&
                            (player[k].aitype==playercontrolled&&player[k].attackkeydown||
                             player[k].aitype!=playercontrolled)){
                        player[i].victim=&player[k];
                        player[i].velocity=0;
                        player[i].currentanimation=jumpreversedanim;
                        player[i].targetanimation=jumpreversedanim;
                        player[i].currentframe=0;
                        player[i].targetframe=1;
                        player[i].targettilt2=0;
                        player[k].victim=&player[i];
                        player[k].velocity=0;
                        player[k].currentanimation=jumpreversalanim;
                        player[k].targetanimation=jumpreversalanim;
                        player[k].currentframe=0;
                        player[k].targetframe=1;
                        player[k].targettilt2=0;
                        if(player[i].coords.y<player[k].coords.y+1){
                            player[i].currentanimation=rabbitkickreversedanim;
                            player[i].targetanimation=rabbitkickreversedanim;
                            player[i].currentframe=1;
                            player[i].targetframe=2;
                            player[k].currentanimation=rabbitkickreversalanim;
                            player[k].targetanimation=rabbitkickreversalanim;
                            player[k].currentframe=1;
                            player[k].targetframe=2;
                        }
                        player[i].target=0;
                        player[k].oldcoords=player[k].coords;
                        player[i].coords=player[k].coords;
                        player[k].targetrotation=player[i].targetrotation;
                        player[k].rotation=player[i].targetrotation;
                        if(player[k].aitype==attacktypecutoff)
                            player[k].stunned=.5;
                    }
                    if(player[k].targetanimation==jumpupanim&&
                            player[i].targetanimation!=getupfrombackanim&&
                            player[i].targetanimation!=getupfromfrontanim&&
                            animation[player[i].targetanimation].height==middleheight&&
                            normaldotproduct(player[k].velocity,player[i].coords-player[k].coords)<0&&
                            ((player[i].aitype==playercontrolled&&player[i].attackkeydown)||
                             player[i].aitype!=playercontrolled)){
                        player[k].victim=&player[i];
                        player[k].velocity=0;
                        player[k].currentanimation=jumpreversedanim;
                        player[k].targetanimation=jumpreversedanim;
                        player[k].currentframe=0;
                        player[k].targetframe=1;
                        player[k].targettilt2=0;
                        player[i].victim=&player[k];
                        player[i].velocity=0;
                        player[i].currentanimation=jumpreversalanim;
                        player[i].targetanimation=jumpreversalanim;
                        player[i].currentframe=0;
                        player[i].targetframe=1;
                        player[i].targettilt2=0;
                        if(player[k].coords.y<player[i].coords.y+1){
                            player[k].targetanimation=rabbitkickreversedanim;
                            player[k].currentanimation=rabbitkickreversedanim;
                            player[i].currentanimation=rabbitkickreversalanim;
                            player[i].targetanimation=rabbitkickreversalanim;
                            player[k].currentframe=1;
                            player[k].targetframe=2;
                            player[i].currentframe=1;
                            player[i].targetframe=2;
                        }
                        player[k].target=0;
                        player[i].oldcoords=player[i].coords;
                        player[k].coords=player[i].coords;
                        player[i].targetrotation=player[k].targetrotation;
                        player[i].rotation=player[k].targetrotation;
                        if(player[i].aitype==attacktypecutoff)
                            player[i].stunned=.5;
                    }
                }
            }
        }
}

void Game::doAerialAcrobatics(){
	static XYZ facing,flatfacing;
    for(int k=0;k<numplayers;k++){
        player[k].turnspeed=500;

        if((player[k].isRun()&&
                    ((player[k].targetrotation!=rabbitrunninganim&&
                      player[k].targetrotation!=wolfrunninganim)||
                     player[k].targetframe==4))||
                player[k].targetanimation==removeknifeanim||
                player[k].targetanimation==crouchremoveknifeanim||
                player[k].targetanimation==flipanim||
                player[k].targetanimation==fightsidestep||
                player[k].targetanimation==walkanim){
            player[k].rotation=stepTowardf(player[k].rotation, player[k].targetrotation, multiplier*player[k].turnspeed);
        }


        if(player[k].isStop()||
                player[k].isLanding()||
                player[k].targetanimation==staggerbackhighanim||
                (player[k].targetanimation==sneakanim&&player[k].currentanimation==sneakanim)||
                player[k].targetanimation==staggerbackhardanim||
                player[k].targetanimation==backhandspringanim||
                player[k].targetanimation==dodgebackanim||
                player[k].targetanimation==rollanim||
                (animation[player[k].targetanimation].attack&&
                 player[k].targetanimation!=rabbitkickanim&&
                 (player[k].targetanimation!=crouchstabanim||player[k].hasvictim)&&
                 (player[k].targetanimation!=swordgroundstabanim||player[k].hasvictim))){
            player[k].rotation=stepTowardf(player[k].rotation, player[k].targetrotation, multiplier*player[k].turnspeed*2);
        }

        if(player[k].targetanimation==sneakanim&&player[k].currentanimation!=sneakanim){
            player[k].rotation=stepTowardf(player[k].rotation, player[k].targetrotation, multiplier*player[k].turnspeed*4);
        }

        /*if(player[k].aitype!=passivetype||(findDistancefast(&player[k].coords,&viewer)<viewdistance*viewdistance))*/
        player[k].DoStuff();
        if(player[k].immobile&&k!=0)
            player[k].coords=player[k].realoldcoords;

        //if player's position has changed (?)
        if(findDistancefast(&player[k].coords,&player[k].realoldcoords)>0&&
                !player[k].skeleton.free&&
                player[k].targetanimation!=climbanim&&
                player[k].targetanimation!=hanganim){
            XYZ lowpoint,lowpointtarget,lowpoint2,lowpointtarget2,lowpoint3,lowpointtarget3,lowpoint4,lowpointtarget4,lowpoint5,lowpointtarget5,lowpoint6,lowpointtarget6,lowpoint7,lowpointtarget7,colpoint,colpoint2;
            int whichhit;
            bool tempcollide=0;

            if(player[k].collide<-.3)
                player[k].collide=-.3;
            if(player[k].collide>1)
                player[k].collide=1;
            player[k].collide-=multiplier*30;

            //clip to terrain
            player[k].coords.y=max(player[k].coords.y, terrain.getHeight(player[k].coords.x,player[k].coords.z));

            for(int l=0;l<terrain.patchobjectnum[player[k].whichpatchx][player[k].whichpatchz];l++){
                int i=terrain.patchobjects[player[k].whichpatchx][player[k].whichpatchz][l];
                if(objects.type[i]!=rocktype||
                        objects.scale[i]>.5&&player[k].aitype==playercontrolled||
                        objects.position[i].y>player[k].coords.y){
                    lowpoint=player[k].coords;
                    if(player[k].targetanimation!=jumpupanim&&
                            player[k].targetanimation!=jumpdownanim&&
                            !player[k].isFlip())
                        lowpoint.y+=1.25;
                    else
                        lowpoint.y+=1.3;
                    if(     player[k].coords.y<terrain.getHeight(player[k].coords.x,player[k].coords.z)&&
                            player[k].coords.y>terrain.getHeight(player[k].coords.x,player[k].coords.z)-.1)
                        player[k].coords.y=terrain.getHeight(player[k].coords.x,player[k].coords.z);
                    if(player[k].SphereCheck(&lowpoint, 1.3, &colpoint, &objects.position[i], &objects.rotation[i], &objects.model[i])!=-1){
                        flatfacing=lowpoint-player[k].coords;
                        player[k].coords=lowpoint;
                        player[k].coords.y-=1.3;
                        player[k].collide=1;
                        tempcollide=1;
                        //wall jumps
                        //TODO: refactor four similar blocks
                        if(player[k].aitype==playercontrolled&&
                                (player[k].targetanimation==jumpupanim||
                                 player[k].targetanimation==jumpdownanim||
                                 player[k].isFlip())&&
                                !player[k].jumptogglekeydown&&
                                player[k].jumpkeydown){
                            lowpointtarget=lowpoint+DoRotation(player[k].facing,0,-90,0)*1.5;
                            XYZ tempcoords1=lowpoint;
                            whichhit=objects.model[i].LineCheck(&lowpoint,&lowpointtarget,&colpoint,&objects.position[i],&objects.rotation[i]);
                            if(whichhit!=-1&&fabs(objects.model[i].facenormals[whichhit].y)<.3){
                                setAnimation(k,walljumpleftanim);
                                emit_sound_at(movewhooshsound, player[k].coords);
                                if(k==0)
                                    pause_sound(whooshsound);

                                lowpointtarget=DoRotation(objects.model[i].facenormals[whichhit],0,objects.rotation[i],0);
                                player[k].rotation=-asin(0-lowpointtarget.x)*180/M_PI;
                                if(lowpointtarget.z<0)
                                    player[k].rotation=180-player[k].rotation;
                                player[k].targetrotation=player[k].rotation;
                                player[k].lowrotation=player[k].rotation;
                                if(k==0)
                                    numwallflipped++;
                            }
                            else
                            {
                                lowpoint=tempcoords1;
                                lowpointtarget=lowpoint+DoRotation(player[k].facing,0,90,0)*1.5;
                                whichhit=objects.model[i].LineCheck(&lowpoint,&lowpointtarget,&colpoint,&objects.position[i],&objects.rotation[i]);
                                if(whichhit!=-1&&fabs(objects.model[i].facenormals[whichhit].y)<.3){
                                    setAnimation(k,walljumprightanim);
                                    emit_sound_at(movewhooshsound, player[k].coords);
                                    if(k==0)pause_sound(whooshsound);

                                    lowpointtarget=DoRotation(objects.model[i].facenormals[whichhit],0,objects.rotation[i],0);
                                    player[k].rotation=-asin(0-lowpointtarget.x)*180/M_PI;
                                    if(lowpointtarget.z<0)player[k].rotation=180-player[k].rotation;
                                    player[k].targetrotation=player[k].rotation;
                                    player[k].lowrotation=player[k].rotation;
                                    if(k==0)numwallflipped++;
                                }
                                else
                                {
                                    lowpoint=tempcoords1;
                                    lowpointtarget=lowpoint+player[k].facing*2;
                                    whichhit=objects.model[i].LineCheck(&lowpoint,&lowpointtarget,&colpoint,&objects.position[i],&objects.rotation[i]);
                                    if(whichhit!=-1&&fabs(objects.model[i].facenormals[whichhit].y)<.3){
                                        setAnimation(k,walljumpbackanim);
                                        emit_sound_at(movewhooshsound, player[k].coords);
                                        if(k==0)pause_sound(whooshsound);

                                        lowpointtarget=DoRotation(objects.model[i].facenormals[whichhit],0,objects.rotation[i],0);
                                        player[k].rotation=-asin(0-lowpointtarget.x)*180/M_PI;
                                        if(lowpointtarget.z<0)player[k].rotation=180-player[k].rotation;
                                        player[k].targetrotation=player[k].rotation;
                                        player[k].lowrotation=player[k].rotation;
                                        if(k==0)numwallflipped++;
                                    }
                                    else
                                    {
                                        lowpoint=tempcoords1;
                                        lowpointtarget=lowpoint-player[k].facing*2;
                                        whichhit=objects.model[i].LineCheck(&lowpoint,&lowpointtarget,&colpoint,&objects.position[i],&objects.rotation[i]);
                                        if(whichhit!=-1&&fabs(objects.model[i].facenormals[whichhit].y)<.3){
                                            setAnimation(k,walljumpfrontanim);
                                            emit_sound_at(movewhooshsound, player[k].coords);
                                            if(k==0)pause_sound(whooshsound);

                                            lowpointtarget=DoRotation(objects.model[i].facenormals[whichhit],0,objects.rotation[i],0);
                                            player[k].rotation=-asin(0-lowpointtarget.x)*180/M_PI;
                                            if(lowpointtarget.z<0)player[k].rotation=180-player[k].rotation;
                                            player[k].rotation+=180;
                                            player[k].targetrotation=player[k].rotation;
                                            player[k].lowrotation=player[k].rotation;
                                            if(k==0)numwallflipped++;
                                        }
                                    }
                                }
                            }
                        }
                    }
                }
                else if(objects.type[i]==rocktype){
                    lowpoint2=player[k].coords;
                    lowpoint=player[k].coords;
                    lowpoint.y+=2;
                    if(objects.model[i].LineCheck(&lowpoint,&lowpoint2,&colpoint,&objects.position[i],&objects.rotation[i])!=-1){
                        player[k].coords=colpoint;
                        player[k].collide=1;
                        tempcollide=1;

                        if(player[k].targetanimation==jumpdownanim||player[k].isFlip()){
                            //flipped into a rock
                            if(player[k].isFlip()&&animation[player[k].targetanimation].label[player[k].targetframe]==7)
                                player[k].RagDoll(0);

                            if(player[k].targetanimation==jumpupanim){
                                player[k].jumppower=-4;
                                player[k].targetanimation=player[k].getIdle();
                            }
                            player[k].target=0;
                            player[k].targetframe=0;
                            player[k].onterrain=1;

                            if(player[k].id==0){
                                pause_sound(whooshsound);
                                OPENAL_SetVolume(channels[whooshsound], 0);
                            }

                            //landing
                            if((player[k].targetanimation==jumpdownanim||player[k].isFlip())&&!player[k].wasLanding()){
                                if(player[k].isFlip())
                                    player[k].jumppower=-4;
                                player[k].targetanimation=player[k].getLanding();
                                emit_sound_at(landsound, player[k].coords, 128.);
                                if(k==0){
                                    envsound[numenvsounds]=player[k].coords;
                                    envsoundvol[numenvsounds]=16;
                                    envsoundlife[numenvsounds]=.4;
                                    numenvsounds++;
                                }

                            }
                        }
                    }
                }
            }

            if(tempcollide&&(/*player[k].jumptogglekeydown*/1==1||player[k].aitype!=playercontrolled))
                for(int l=0;l<terrain.patchobjectnum[player[k].whichpatchx][player[k].whichpatchz];l++){
                    int i=terrain.patchobjects[player[k].whichpatchx][player[k].whichpatchz][l];
                    lowpoint=player[k].coords;
                    lowpoint.y+=1.35;
                    if(objects.type[i]!=rocktype)
                        if(player[k].SphereCheck(&lowpoint,1.33,&colpoint,&objects.position[i],&objects.rotation[i],&objects.model[i])!=-1){
                            if(player[k].targetanimation!=jumpupanim&&
                                    player[k].targetanimation!=jumpdownanim&&
                                    player[k].onterrain)
                                player[k].avoidcollided=1;
                            player[k].coords=lowpoint;
                            player[k].coords.y-=1.35;
                            player[k].collide=1;

                            if((player[k].grabdelay<=0||player[k].aitype!=playercontrolled)&&
                                    (player[k].currentanimation!=climbanim&&
                                     player[k].currentanimation!=hanganim&&
                                     !player[k].isWallJump()||
                                     player[k].targetanimation==jumpupanim||
                                     player[k].targetanimation==jumpdownanim)){
                                lowpoint=player[k].coords;
                                objects.model[i].SphereCheckPossible(&lowpoint, 1.5, &objects.position[i], &objects.rotation[i]);
                                lowpoint=player[k].coords;
                                lowpoint.y+=.05;
                                facing=0;
                                facing.z=-1;
                                facing=DoRotation(facing,0,player[k].targetrotation+180,0);
                                lowpointtarget=lowpoint+facing*1.4;
                                whichhit=objects.model[i].LineCheckPossible(&lowpoint,&lowpointtarget,&colpoint,&objects.position[i],&objects.rotation[i]);
                                if(whichhit!=-1){
                                    lowpoint=player[k].coords;
                                    lowpoint.y+=.1;
                                    lowpointtarget=lowpoint+facing*1.4;
                                    lowpoint2=lowpoint;
                                    lowpointtarget2=lowpointtarget;
                                    lowpoint3=lowpoint;
                                    lowpointtarget3=lowpointtarget;
                                    lowpoint4=lowpoint;
                                    lowpointtarget4=lowpointtarget;
                                    lowpoint5=lowpoint;
                                    lowpointtarget5=lowpointtarget;
                                    lowpoint6=lowpoint;
                                    lowpointtarget6=lowpointtarget;
                                    lowpoint7=lowpoint;
                                    lowpointtarget7=lowpoint;
                                    lowpoint2.x+=.1;
                                    lowpointtarget2.x+=.1;
                                    lowpoint3.z+=.1;
                                    lowpointtarget3.z+=.1;
                                    lowpoint4.x-=.1;
                                    lowpointtarget4.x-=.1;
                                    lowpoint5.z-=.1;
                                    lowpointtarget5.z-=.1;
                                    lowpoint6.y+=45/13;
                                    lowpointtarget6.y+=45/13;
                                    lowpointtarget6+=facing*.6;
                                    lowpointtarget7.y+=90/13;
                                    whichhit=objects.model[i].LineCheckPossible(&lowpoint,&lowpointtarget,&colpoint,&objects.position[i],&objects.rotation[i]);
                                    if(objects.friction[i]>.5)
                                        if(whichhit!=-1){
                                            if(whichhit!=-1&&player[k].targetanimation!=jumpupanim&&player[k].targetanimation!=jumpdownanim)
                                                player[k].collided=1;
                                            if(checkcollide(lowpoint7,lowpointtarget7)==-1)
                                                if(checkcollide(lowpoint6,lowpointtarget6)==-1)
                                                    if(     objects.model[i].LineCheckPossible(&lowpoint2,&lowpointtarget2,
                                                                &colpoint,&objects.position[i],&objects.rotation[i])!=-1&&
                                                            objects.model[i].LineCheckPossible(&lowpoint3,&lowpointtarget3,
                                                                &colpoint,&objects.position[i],&objects.rotation[i])!=-1&&
                                                            objects.model[i].LineCheckPossible(&lowpoint4,&lowpointtarget4,
                                                                &colpoint,&objects.position[i],&objects.rotation[i])!=-1&&
                                                            objects.model[i].LineCheckPossible(&lowpoint5,&lowpointtarget5,
                                                                &colpoint,&objects.position[i],&objects.rotation[i])!=-1)
                                                        for(int j=0;j<45;j++){
                                                            lowpoint=player[k].coords;
                                                            lowpoint.y+=(float)j/13;
                                                            lowpointtarget=lowpoint+facing*1.4;
                                                            if(objects.model[i].LineCheckPossible(&lowpoint,&lowpointtarget,
                                                                        &colpoint2,&objects.position[i],&objects.rotation[i])==-1){
                                                                if(j<=6||j<=25&&player[k].targetanimation==jumpdownanim)
                                                                    break;
                                                                if(player[k].targetanimation==jumpupanim||player[k].targetanimation==jumpdownanim){
                                                                    lowpoint=DoRotation(objects.model[i].facenormals[whichhit],0,objects.rotation[k],0);
                                                                    lowpoint=player[k].coords;
                                                                    lowpoint.y+=(float)j/13;
                                                                    lowpointtarget=lowpoint+facing*1.3;
                                                                    flatfacing=player[k].coords;
                                                                    player[k].coords=colpoint-DoRotation(objects.model[i].facenormals[whichhit],0,objects.rotation[k],0)*.01;
                                                                    player[k].coords.y=lowpointtarget.y-.07;
                                                                    player[k].currentoffset=(flatfacing-player[k].coords)/player[k].scale;

                                                                    if(j>10||!player[k].isRun()){
                                                                        if(player[k].targetanimation==jumpdownanim||player[k].targetanimation==jumpupanim){
                                                                            if(k==0)
                                                                                pause_sound(whooshsound);
                                                                        }
                                                                        emit_sound_at(jumpsound, player[k].coords, 128.);

                                                                        lowpointtarget=DoRotation(objects.model[i].facenormals[whichhit],0,objects.rotation[i],0);
                                                                        player[k].rotation=-asin(0-lowpointtarget.x)*180/M_PI;
                                                                        if(lowpointtarget.z<0)
                                                                            player[k].rotation=180-player[k].rotation;
                                                                        player[k].targetrotation=player[k].rotation;
                                                                        player[k].lowrotation=player[k].rotation;

                                                                        //player[k].velocity=lowpointtarget*.03;
                                                                        player[k].velocity=0;

                                                                        //climb ledge (?)
                                                                        if(player[k].targetanimation==jumpupanim){
                                                                            player[k].targetanimation=climbanim;
                                                                            player[k].jumppower=0;
                                                                            player[k].jumpclimb=1;
                                                                        }
                                                                        player[k].transspeed=6;
                                                                        player[k].target=0;
                                                                        player[k].targetframe=1;
                                                                        //hang ledge (?)
                                                                        if(j>25){
                                                                            setAnimation(k,hanganim);
                                                                            player[k].jumppower=0;
                                                                        }
                                                                    }
                                                                    break;
                                                                }
                                                            }
                                                        }
                                        }
                                }
                            }
                        }
                }
            if(player[k].collide<=0){
                //in the air
                if(!player[k].onterrain&&
                        player[k].targetanimation!=jumpupanim&&
                        player[k].targetanimation!=jumpdownanim&&
                        player[k].targetanimation!=climbanim&&
                        player[k].targetanimation!=hanganim&&
                        !player[k].isWallJump()&&
                        !player[k].isFlip()){
                    if(player[k].currentanimation!=climbanim&&
                            player[k].currentanimation!=tempanim&&
                            player[k].targetanimation!=backhandspringanim&&
                            (player[k].targetanimation!=rollanim||
                             player[k].targetframe<2||
                             player[k].targetframe>6)){
                        //stagger off ledge (?)
                        if(player[k].targetanimation==staggerbackhighanim||player[k].targetanimation==staggerbackhardanim)
                            player[k].RagDoll(0);
                        setAnimation(k,jumpdownanim);

                        if(!k)
                          emit_sound_at(whooshsound, player[k].coords, 128.);
                    }
                    //gravity
                    player[k].velocity.y+=gravity;
                }
            }
        }
        player[k].realoldcoords=player[k].coords;
    }
}

void Game::doAttacks(){
    static XYZ relative;
    static int randattack;
    static bool playerrealattackkeydown=0;

    if(!Input::isKeyDown(attackkey))
        oldattackkey=0;
    if(oldattackkey)
        player[0].attackkeydown=0;
    if(oldattackkey)
        playerrealattackkeydown=0;
    if(!oldattackkey)
        playerrealattackkeydown=Input::isKeyDown(attackkey);
    if((player[0].parriedrecently<=0||
                player[0].weaponactive==-1)&&
            (!oldattackkey||
             (realthreat&&
              player[0].lastattack!=swordslashanim&&
              player[0].lastattack!=knifeslashstartanim&&
              player[0].lastattack!=staffhitanim&&
              player[0].lastattack!=staffspinhitanim)))
        player[0].attackkeydown=Input::isKeyDown(attackkey);
    if(Input::isKeyDown(attackkey)&&
            !oldattackkey&&
            !player[0].backkeydown){
        for(int k=0;k<numplayers;k++){
            if((player[k].targetanimation==swordslashanim||
                        player[k].targetanimation==staffhitanim||
                        player[k].targetanimation==staffspinhitanim)&&
                    player[0].currentanimation!=dodgebackanim&&
                    !player[k].skeleton.free)
                player[k].Reverse();
        }
    }

    if(!hostile||indialogue!=-1)player[0].attackkeydown=0;

    for(int k=0;k<numplayers;k++){
        if(indialogue!=-1)player[k].attackkeydown=0;
        if(player[k].targetanimation!=rabbitrunninganim&&player[k].targetanimation!=wolfrunninganim){
            if(player[k].aitype!=playercontrolled)
                player[k].victim=&player[0];
            //attack key pressed
            if(player[k].attackkeydown){
                //dodge backward
                if(player[k].backkeydown&&
                        player[k].targetanimation!=backhandspringanim&&
                        (player[k].isIdle()||
                         player[k].isStop()||
                         player[k].isRun()||
                         player[k].targetanimation==walkanim)){
                    if(player[k].jumppower<=1){
                        player[k].jumppower-=2;
                    }else{
                        for(int i=0;i<numplayers;i++){
                            if(i==k)continue;
                            if(player[i].targetanimation==swordslashanim||
                                    player[i].targetanimation==knifeslashstartanim||
                                    player[i].targetanimation==staffhitanim||
                                    player[i].targetanimation==staffspinhitanim)
                                if(findDistancefast(&player[k].coords,&player[i].coords)<6.5&&!player[i].skeleton.free){
                                    setAnimation(k,dodgebackanim);
                                    player[k].targetrotation=roughDirectionTo(player[k].coords,player[i].coords);
                                    player[k].targettilt2=pitchTo(player[k].coords,player[i].coords);
                                }
                        }
                        if(player[k].targetanimation!=dodgebackanim){
                            if(k==0)numflipped++;
                            setAnimation(k,backhandspringanim);
                            player[k].targetrotation=-rotation+180;
                            if(player[k].leftkeydown)
                                player[k].targetrotation-=45;
                            if(player[k].rightkeydown)
                                player[k].targetrotation+=45;
                            player[k].rotation=player[k].targetrotation;
                            player[k].jumppower-=2;
                        }
                    }
                }
                //attack
                if(!animation[player[k].targetanimation].attack&&
                        !player[k].backkeydown&&
                        (player[k].isIdle()||
                         player[k].isRun()||
                         player[k].targetanimation==walkanim||
                         player[k].targetanimation==sneakanim||
                         player[k].isCrouch())){
                    const int attackweapon=player[k].weaponactive==-1?0:weapons.type[player[k].weaponids[player[k].weaponactive]];
                    //normal attacks (?)
                    player[k].hasvictim=0;
                    if(numplayers>1)
                        for(int i=0;i<numplayers;i++){
                            if(i==k||!(k==0||i==0))continue;
                            if(!player[k].hasvictim)
                                if(animation[player[k].targetanimation].attack!=reversal){
                                    //choose an attack
                                    const float distance=findDistancefast(&player[k].coords,&player[i].coords);
                                    if(distance<4.5&&
                                            !player[i].skeleton.free&&
                                            player[i].howactive<typedead1&&
                                            player[i].targetanimation!=jumpreversedanim&&
                                            player[i].targetanimation!=rabbitkickreversedanim&&
                                            player[i].targetanimation!=rabbitkickanim&&
                                            player[k].targetanimation!=rabbitkickanim&&
                                            player[i].targetanimation!=getupfrombackanim&&
                                            (player[i].targetanimation!=staggerbackhighanim&&
                                             (player[i].targetanimation!=staggerbackhardanim||
                                              animation[staggerbackhardanim].label[player[i].targetframe]==6))&&
                                            player[i].targetanimation!=jumpdownanim&&
                                            player[i].targetanimation!=jumpupanim&&
                                            player[i].targetanimation!=getupfromfrontanim){
                                        player[k].victim=&player[i];
                                        player[k].hasvictim=1;
                                        if(player[k].aitype==playercontrolled){ //human player
                                            //sweep
                                            if(distance<2.5*sq(player[k].scale*5)&&
                                                    player[k].crouchkeydown&&
                                                    animation[player[i].targetanimation].height!=lowheight)
                                                player[k].targetanimation=sweepanim;
                                            //winduppunch
                                            else if(distance<1.5*sq(player[k].scale*5)&&
                                                    animation[player[i].targetanimation].height!=lowheight&&
                                                    !player[k].forwardkeydown&&
                                                    !player[k].leftkeydown&&
                                                    !player[k].rightkeydown&&
                                                    !player[k].crouchkeydown&&
                                                    !attackweapon&&
                                                    !reversaltrain)
                                                player[k].targetanimation=winduppunchanim;
                                            //upunch
                                            else if(distance<2.5*sq(player[k].scale*5)&&
                                                    animation[player[i].targetanimation].height!=lowheight&&
                                                    !player[k].forwardkeydown&&
                                                    !player[k].leftkeydown&&
                                                    !player[k].rightkeydown&&
                                                    !player[k].crouchkeydown&&
                                                    !attackweapon)
                                                player[k].targetanimation=upunchanim;
                                            //knifefollow
                                            else if(distance<2.5*sq(player[k].scale*5)&&
                                                    player[i].staggerdelay>0&&
                                                    attackweapon==knife&&
                                                    player[i].bloodloss>player[i].damagetolerance/2)
                                                player[k].targetanimation=knifefollowanim;
                                            //knifeslashstart
                                            else if(distance<2.5*sq(player[k].scale*5)&&
                                                    animation[player[i].targetanimation].height!=lowheight&&
                                                    !player[k].forwardkeydown&&
                                                    !player[k].leftkeydown&&
                                                    !player[k].rightkeydown&&
                                                    !player[k].crouchkeydown&&
                                                    attackweapon==knife&&
                                                    player[k].weaponmissdelay<=0)
                                                player[k].targetanimation=knifeslashstartanim;
                                            //swordslash
                                            else if(distance<4.5*sq(player[k].scale*5)&&
                                                    animation[player[i].targetanimation].height!=lowheight&&
                                                    !player[k].crouchkeydown&&
                                                    attackweapon==sword&&
                                                    player[k].weaponmissdelay<=0)
                                                player[k].targetanimation=swordslashanim;
                                            //staffhit
                                            else if(distance<4.5*sq(player[k].scale*5)&&
                                                    animation[player[i].targetanimation].height!=lowheight&&
                                                    !player[k].crouchkeydown&&
                                                    attackweapon==staff&&
                                                    player[k].weaponmissdelay<=0&&
                                                    !player[k].leftkeydown&&
                                                    !player[k].rightkeydown&&
                                                    !player[k].forwardkeydown)
                                                player[k].targetanimation=staffhitanim;
                                            //staffspinhit
                                            else if(distance<4.5*sq(player[k].scale*5)&&
                                                    animation[player[i].targetanimation].height!=lowheight&&
                                                    !player[k].crouchkeydown&&
                                                    attackweapon==staff&&
                                                    player[k].weaponmissdelay<=0)
                                                player[k].targetanimation=staffspinhitanim;
                                            //spinkick
                                            else if(distance<2.5*sq(player[k].scale*5)&&
                                                    animation[player[i].targetanimation].height!=lowheight)
                                                player[k].targetanimation=spinkickanim;
                                            //lowkick
                                            else if(distance<2.5*sq(player[k].scale*5)&&
                                                    animation[player[i].targetanimation].height==lowheight&&
                                                    animation[player[k].targetanimation].attack!=normalattack)
                                                player[k].targetanimation=lowkickanim;
                                        } else { //AI player
                                            if(distance<4.5*sq(player[k].scale*5)){
                                                randattack=abs(Random()%5);
                                                if(!attackweapon&&distance<2.5*sq(player[k].scale*5)){
                                                    //sweep
                                                    if(randattack==0&&animation[player[i].targetanimation].height!=lowheight)
                                                        player[k].targetanimation=sweepanim;
                                                    //upunch
                                                    else if(randattack==1&&animation[player[i].targetanimation].height!=lowheight&&
                                                            !attackweapon)
                                                        player[k].targetanimation=upunchanim;
                                                    //spinkick
                                                    else if(randattack==2&&animation[player[i].targetanimation].height!=lowheight)
                                                        player[k].targetanimation=spinkickanim;
                                                    //lowkick
                                                    else if(animation[player[i].targetanimation].height==lowheight)
                                                        player[k].targetanimation=lowkickanim;
                                                }
                                                if(attackweapon){
                                                    //sweep
                                                    if((tutoriallevel!=1||!attackweapon)&&
                                                            distance<2.5*sq(player[k].scale*5)&&
                                                            randattack==0&&
                                                            animation[player[i].targetanimation].height!=lowheight)
                                                        player[k].targetanimation=sweepanim;
                                                    //knifeslashstart
                                                    else if(distance<2.5*sq(player[k].scale*5)&&
                                                            attackweapon==knife&&
                                                            player[k].weaponmissdelay<=0)
                                                        player[k].targetanimation=knifeslashstartanim;
                                                    //swordslash
                                                    else if(!(player[0].victim==&player[i]&&
                                                                player[0].hasvictim&&
                                                                player[0].targetanimation==swordslashanim)&&
                                                            attackweapon==sword&&
                                                            player[k].weaponmissdelay<=0)
                                                        player[k].targetanimation=swordslashanim;
                                                    //staffhit
                                                    else if(!(player[0].victim==&player[i]&&
                                                                player[0].hasvictim&&
                                                                player[0].targetanimation==swordslashanim)&&
                                                            attackweapon==staff&&
                                                            player[k].weaponmissdelay<=0&&
                                                            randattack<3)
                                                        player[k].targetanimation=staffhitanim;
                                                    //staffspinhit
                                                    else if(!(player[0].victim==&player[i]&&
                                                                player[0].hasvictim&&
                                                                player[0].targetanimation==swordslashanim)&&
                                                            attackweapon==staff&&
                                                            player[k].weaponmissdelay<=0&&
                                                            randattack>=3)
                                                        player[k].targetanimation=staffspinhitanim;
                                                    //spinkick
                                                    else if((tutoriallevel!=1||!attackweapon)&&
                                                            distance<2.5*sq(player[k].scale*5)&&
                                                            randattack==1&&
                                                            animation[player[i].targetanimation].height!=lowheight)
                                                        player[k].targetanimation=spinkickanim;
                                                    //lowkick
                                                    else if(distance<2.5*sq(player[k].scale*5)&&
                                                            animation[player[i].targetanimation].height==lowheight&&
                                                            animation[player[k].targetanimation].attack!=normalattack)
                                                        player[k].targetanimation=lowkickanim;
                                                }
                                            }
                                        }
                                        //upunch becomes wolfslap
                                        if(player[k].targetanimation==upunchanim&&player[k].creature==wolftype)
                                            player[k].targetanimation=wolfslapanim;
                                    }
                                    //sneak attacks
                                    if((k==0)&&(tutoriallevel!=1||tutorialstage==22)&&
                                            player[i].howactive<typedead1&&
                                            distance<1.5*sq(player[k].scale*5)&&
                                            !player[i].skeleton.free&&
                                            player[i].targetanimation!=getupfrombackanim&&
                                            player[i].targetanimation!=getupfromfrontanim&&
                                            (player[i].stunned>0&&player[k].madskills||
                                             player[i].surprised>0||
                                             player[i].aitype==passivetype||
                                             attackweapon&&player[i].stunned>0)&&
                                            normaldotproduct(player[i].facing,player[i].coords-player[k].coords)>0){
                                        //sneakattack
                                        if(!attackweapon){
                                            player[k].currentanimation=sneakattackanim;
                                            player[k].targetanimation=sneakattackanim;
                                            player[i].currentanimation=sneakattackedanim;
                                            player[i].targetanimation=sneakattackedanim;
                                            player[k].oldcoords=player[k].coords;
                                            player[k].coords=player[i].coords;
                                        }
                                        //knifesneakattack
                                        if(attackweapon==knife){
                                            player[k].currentanimation=knifesneakattackanim;
                                            player[k].targetanimation=knifesneakattackanim;
                                            player[i].currentanimation=knifesneakattackedanim;
                                            player[i].targetanimation=knifesneakattackedanim;
                                            player[i].oldcoords=player[i].coords;
                                            player[i].coords=player[k].coords;
                                        }
                                        //swordsneakattack
                                        if(attackweapon==sword){
                                            player[k].currentanimation=swordsneakattackanim;
                                            player[k].targetanimation=swordsneakattackanim;
                                            player[i].currentanimation=swordsneakattackedanim;
                                            player[i].targetanimation=swordsneakattackedanim;
                                            player[i].oldcoords=player[i].coords;
                                            player[i].coords=player[k].coords;
                                        }
                                        if(attackweapon!=staff){
                                            player[k].victim=&player[i];
                                            player[k].hasvictim=1;
                                            player[i].targettilt2=0;
                                            player[i].targetframe=1;
                                            player[i].currentframe=0;
                                            player[i].target=0;
                                            player[i].velocity=0;
                                            player[k].targettilt2=player[i].targettilt2;
                                            player[k].currentframe=player[i].currentframe;
                                            player[k].targetframe=player[i].targetframe;
                                            player[k].target=player[i].target;
                                            player[k].velocity=0;
                                            player[k].targetrotation=player[i].rotation;
                                            player[k].rotation=player[i].rotation;
                                            player[i].targetrotation=player[i].rotation;
                                        }
                                    }
                                    if(animation[player[k].targetanimation].attack==normalattack&&
                                            player[k].victim==&player[i]&&
                                            (!player[i].skeleton.free)){
                                        oldattackkey=1;
                                        player[k].targetframe=0;
                                        player[k].target=0;

                                        player[k].targetrotation=roughDirectionTo(player[k].coords,player[i].coords);
                                        player[k].targettilt2=pitchTo(player[k].coords,player[i].coords);
                                        player[k].lastattack3=player[k].lastattack2;
                                        player[k].lastattack2=player[k].lastattack;
                                        player[k].lastattack=player[k].targetanimation;
                                    }
                                    if(player[k].targetanimation==knifefollowanim&&
                                            player[k].victim==&player[i]){
                                        oldattackkey=1;
                                        player[k].targetrotation=roughDirectionTo(player[k].coords,player[i].coords);
                                        player[k].targettilt2=pitchTo(player[k].coords,player[i].coords);
                                        player[k].victim=&player[i];
                                        player[k].hasvictim=1;
                                        player[i].targetanimation=knifefollowedanim;
                                        player[i].currentanimation=knifefollowedanim;
                                        player[i].targettilt2=0;
                                        player[i].targettilt2=player[k].targettilt2;
                                        player[i].targetframe=1;
                                        player[i].currentframe=0;
                                        player[i].target=0;
                                        player[i].velocity=0;
                                        player[k].currentanimation=knifefollowanim;
                                        player[k].targetanimation=knifefollowanim;
                                        player[k].targettilt2=player[i].targettilt2;
                                        player[k].currentframe=player[i].currentframe;
                                        player[k].targetframe=player[i].targetframe;
                                        player[k].target=player[i].target;
                                        player[k].velocity=0;
                                        player[k].oldcoords=player[k].coords;
                                        player[i].coords=player[k].coords;
                                        player[i].targetrotation=player[k].targetrotation;
                                        player[i].rotation=player[k].targetrotation;
                                        player[k].rotation=player[k].targetrotation;
                                        player[i].rotation=player[k].targetrotation;
                                    }
                                }
                        }
                    const bool hasstaff=attackweapon==staff;
                    if(k==0&&numplayers>1)
                        for(int i=0;i<numplayers;i++){
                            if(i==k)continue;
                            if((playerrealattackkeydown||player[i].dead||!hasstaff)&&
                                    animation[player[k].targetanimation].attack==neutral){
                                const float distance=findDistancefast(&player[k].coords,&player[i].coords);
                                if(!player[i].dead||!realthreat||(!attackweapon&&player[k].crouchkeydown))
                                    if(player[i].skeleton.free)
                                        if(distance<3.5*sq(player[k].scale*5)&&
                                                (player[i].dead||
                                                 player[i].skeleton.longdead>1000||
                                                 player[k].isRun()||
                                                 hasstaff||
                                                 (attackweapon&&
                                                  (player[i].skeleton.longdead>2000||
                                                   player[i].damage>player[i].damagetolerance/8||
                                                   player[i].bloodloss>player[i].damagetolerance/2)&&
                                                  distance<1.5*sq(player[k].scale*5)))){
                                            player[k].victim=&player[i];
                                            player[k].hasvictim=1;
                                            if(attackweapon&&tutoriallevel!=1){
                                                //crouchstab
                                                if(player[k].crouchkeydown&&attackweapon==knife&&distance<1.5*sq(player[k].scale*5))
                                                    player[k].targetanimation=crouchstabanim;
                                                //swordgroundstab
                                                if(player[k].crouchkeydown&&distance<1.5*sq(player[k].scale*5)&&attackweapon==sword)
                                                    player[k].targetanimation=swordgroundstabanim;
                                                //staffgroundsmash
                                                if(distance<3.5*sq(player[k].scale*5)&&attackweapon==staff)
                                                    player[k].targetanimation=staffgroundsmashanim;
                                            }
                                            if(distance<2.5&&
                                                    player[k].crouchkeydown&&
                                                    player[k].targetanimation!=crouchstabanim&&
                                                    !attackweapon&&
                                                    player[i].dead&&
                                                    player[i].skeleton.free&&
                                                    player[i].skeleton.longdead>1000){
                                                player[k].targetanimation=killanim;
                                                //TODO: refactor this out, what does it do?
                                                for(int j=0;j<terrain.numdecals;j++){
                                                    if((terrain.decaltype[j]==blooddecal||terrain.decaltype[j]==blooddecalslow)&&
                                                            terrain.decalalivetime[j]<2)
                                                        terrain.DeleteDecal(j);
                                                }
                                                for(int l=0;l<objects.numobjects;l++){
                                                    if(objects.model[l].type==decalstype)
                                                        for(int j=0;j<objects.model[l].numdecals;j++){
                                                            if((objects.model[l].decaltype[j]==blooddecal||
                                                                    objects.model[l].decaltype[j]==blooddecalslow)&&
                                                                    objects.model[l].decalalivetime[j]<2)
                                                                objects.model[l].DeleteDecal(j);
                                                        }
                                                }
                                            }
                                            if(!player[i].dead||musictype!=2)
                                                if(distance<3.5&&
                                                        (player[k].isRun()||player[k].isIdle()&&player[k].attackkeydown)&&
                                                        player[k].staggerdelay<=0&&
                                                        (player[i].dead||
                                                         player[i].skeleton.longdead<300&&
                                                         player[k].lastattack!=spinkickanim&&
                                                         player[i].skeleton.free)&&
                                                        (!player[i].dead||musictype!=stream_music2)){
                                                    player[k].targetanimation=dropkickanim;
                                                    for(int j=0;j<terrain.numdecals;j++){
                                                        if((terrain.decaltype[j]==blooddecal||terrain.decaltype[j]==blooddecalslow)&&
                                                                terrain.decalalivetime[j]<2){
                                                            terrain.DeleteDecal(j);
                                                        }
                                                    }
                                                    for(int l=0;l<objects.numobjects;l++){
                                                        if(objects.model[l].type==decalstype)
                                                            for(int j=0;j<objects.model[l].numdecals;j++){
                                                                if((objects.model[l].decaltype[j]==blooddecal||
                                                                        objects.model[l].decaltype[j]==blooddecalslow)&&
                                                                        objects.model[l].decalalivetime[j]<2){
                                                                    objects.model[l].DeleteDecal(j);
                                                                }
                                                            }
                                                    }
                                                }
                                        }
                                if(animation[player[k].targetanimation].attack==normalattack&&
                                        player[k].victim==&player[i]&&
                                        (!player[i].skeleton.free||
                                         player[k].targetanimation==killanim||
                                         player[k].targetanimation==crouchstabanim||
                                         player[k].targetanimation==swordgroundstabanim||
                                         player[k].targetanimation==staffgroundsmashanim||
                                         player[k].targetanimation==dropkickanim)){
                                    oldattackkey=1;
                                    player[k].targetframe=0;
                                    player[k].target=0;

                                    XYZ targetpoint=player[i].coords;
                                    if(player[k].targetanimation==crouchstabanim||
                                            player[k].targetanimation==swordgroundstabanim||
                                            player[k].targetanimation==staffgroundsmashanim){
                                        targetpoint+=(playerJoint(i,abdomen).position+
                                                 playerJoint(i,neck).position)/2*
                                                player[i].scale;
                                    }
                                    player[k].targetrotation=roughDirectionTo(player[k].coords,targetpoint);
                                    player[k].targettilt2=pitchTo(player[k].coords,targetpoint);

                                    if(player[k].targetanimation==crouchstabanim||player[k].targetanimation==swordgroundstabanim){
                                        player[k].targetrotation+=(float)(abs(Random()%100)-50)/4;
                                    }

                                    if(player[k].targetanimation==staffgroundsmashanim)
                                        player[k].targettilt2+=10;

                                    player[k].lastattack3=player[k].lastattack2;
                                    player[k].lastattack2=player[k].lastattack;
                                    player[k].lastattack=player[k].targetanimation;

                                    if(player[k].targetanimation==swordgroundstabanim){
                                        player[k].targetrotation+=30;
                                    }
                                }
                            }
                        }
                    if(!player[k].hasvictim){
                        //find victim
                        for(int i=0;i<numplayers;i++){
                            if(i==k||!(i==0||k==0))continue;
                            if(!player[i].skeleton.free){
                                if(player[k].hasvictim){
                                    if(findDistancefast(&player[k].coords,&player[i].coords)<
                                       findDistancefast(&player[k].coords,&player[k].victim->coords))
                                        player[k].victim=&player[i];
                                }else{
                                    player[k].victim=&player[i];
                                    player[k].hasvictim=1;
                                }
                            }
                        }
                    }
                    if(player[k].aitype==playercontrolled)
                        //rabbit kick
                        if(player[k].attackkeydown&&
                                player[k].isRun()&&
                                player[k].wasRun()&&
                                ((player[k].hasvictim&&
                                  findDistancefast(&player[k].coords,&player[k].victim->coords)<12*sq(player[k].scale*5)&&
                                  findDistancefast(&player[k].coords,&player[k].victim->coords)>7*sq(player[k].scale*5)&&
                                  !player[k].victim->skeleton.free&&
                                  player[k].victim->targetanimation!=getupfrombackanim&&
                                  player[k].victim->targetanimation!=getupfromfrontanim&&
                                  animation[player[k].victim->targetanimation].height!=lowheight&&
                                  player[k].aitype!=playercontrolled&& //wat???
                                  normaldotproduct(player[k].facing,player[k].victim->coords-player[k].coords)>0&&
                                  player[k].rabbitkickenabled)||
                                 player[k].jumpkeydown)){
                            oldattackkey=1;
                            setAnimation(k,rabbitkickanim);
                        }
                    //update counts
                    if(animation[player[k].targetanimation].attack&&k==0){
                        numattacks++;
                        switch(attackweapon){
                            case 0: numunarmedattack++; break;
                            case knife: numknifeattack++; break;
                            case sword: numswordattack++; break;
                            case staff: numstaffattack++; break;
                        }
                    }
                }
            }
        }
    }
}

void Game::doPlayerCollisions(){
	static XYZ rotatetarget;
    static float collisionradius;
    if(numplayers>1)
        for(int k=0;k<numplayers;k++)
            for(int i=k+1;i<numplayers;i++){
                //neither player is part of a reversal
                if((animation[player[i].targetanimation].attack!=reversed&&
                            animation[player[i].targetanimation].attack!=reversal&&
                            animation[player[k].targetanimation].attack!=reversed&&
                            animation[player[k].targetanimation].attack!=reversal)||(i!=0&&k!=0))
                if((animation[player[i].currentanimation].attack!=reversed&&
                            animation[player[i].currentanimation].attack!=reversal&&
                            animation[player[k].currentanimation].attack!=reversed&&
                            animation[player[k].currentanimation].attack!=reversal)||(i!=0&&k!=0))
                //neither is sleeping
                if(player[i].howactive<=typesleeping&&player[k].howactive<=typesleeping)
                if(player[i].howactive!=typesittingwall&&player[k].howactive!=typesittingwall)
                //in same patch, neither is climbing
                if(player[i].whichpatchx==player[k].whichpatchx&&
                        player[i].whichpatchz==player[k].whichpatchz&&
                        player[k].skeleton.oldfree==player[k].skeleton.free&&
                        player[i].skeleton.oldfree==player[i].skeleton.free&&
                        player[i].targetanimation!=climbanim&&
                        player[i].targetanimation!=hanganim&&
                        player[k].targetanimation!=climbanim&&
                        player[k].targetanimation!=hanganim)
                //players are close (bounding box test)
                if(player[i].coords.y>player[k].coords.y-3)
                if(player[i].coords.y<player[k].coords.y+3)
                if(player[i].coords.x>player[k].coords.x-3)
                if(player[i].coords.x<player[k].coords.x+3)
                if(player[i].coords.z>player[k].coords.z-3)
                if(player[i].coords.z<player[k].coords.z+3){
                    //spread fire from player to player
                    if(findDistancefast(&player[i].coords,&player[k].coords)
                            <3*sq((player[i].scale+player[k].scale)*2.5)){
                        if(player[i].onfire||player[k].onfire){
                            if(!player[i].onfire)player[i].CatchFire();
                            if(!player[k].onfire)player[k].CatchFire();
                        }
                    }

                    XYZ tempcoords1=player[i].coords;
                    XYZ tempcoords2=player[k].coords;
                    if(!player[i].skeleton.oldfree)
                        tempcoords1.y+=playerJoint(i,abdomen).position.y*player[i].scale;
                    if(!player[k].skeleton.oldfree)
                        tempcoords2.y+=playerJoint(k,abdomen).position.y*player[k].scale;
                    collisionradius=1.2*sq((player[i].scale+player[k].scale)*2.5);
                    if(player[0].hasvictim)
                        if(player[0].targetanimation==rabbitkickanim&&(k==0||i==0)&&!player[0].victim->skeleton.free)
                            collisionradius=3;
                    if((!player[i].skeleton.oldfree||!player[k].skeleton.oldfree)&&
                            (findDistancefast(&tempcoords1,&tempcoords2)<collisionradius||
                             findDistancefast(&player[i].coords,&player[k].coords)<collisionradius)){
                        //jump down on a dead body
                        if(k==0||i==0){
                            int l=i?i:k;
                            if(player[0].targetanimation==jumpdownanim&&
                                    !player[0].skeleton.oldfree&&
                                    !player[0].skeleton.free&&
                                    player[l].skeleton.oldfree&&
                                    player[l].skeleton.free&&
                                    player[l].dead&&
                                    player[0].lastcollide<=0&&
                                    fabs(player[l].coords.y-player[0].coords.y)<.2&&
                                    findDistancefast(&player[0].coords,&player[l].coords)<.7*sq((player[l].scale+player[0].scale)*2.5)){
                                player[0].coords.y=player[l].coords.y;
                                player[l].velocity=player[0].velocity;
                                player[l].skeleton.free=0;
                                player[l].rotation=0;
                                player[l].RagDoll(0);
                                player[l].DoDamage(20);
                                camerashake+=.3;
                                player[l].skeleton.longdead=0;
                                player[0].lastcollide=1;
                            }
                        }

                        if(     (player[i].skeleton.oldfree==1&&findLengthfast(&player[i].velocity)>1)||
                                (player[k].skeleton.oldfree==1&&findLengthfast(&player[k].velocity)>1)||
                                (player[i].skeleton.oldfree==0&&player[k].skeleton.oldfree==0)){
                            rotatetarget=player[k].velocity-player[i].velocity;
                            if((player[i].targetanimation!=getupfrombackanim&&player[i].targetanimation!=getupfromfrontanim||
                                        player[i].skeleton.free)&&
                                    (player[k].targetanimation!=getupfrombackanim&&player[k].targetanimation!=getupfromfrontanim||
                                     player[k].skeleton.free))
                                if((((k!=0&&findLengthfast(&rotatetarget)>150||
                                                    k==0&&findLengthfast(&rotatetarget)>50&&player[0].rabbitkickragdoll)&&
                                                normaldotproduct(rotatetarget,player[k].coords-player[i].coords)>0)&&
                                            (k==0||
                                             k!=0&&player[i].skeleton.oldfree==1&&animation[player[k].currentanimation].attack==neutral||
                                         /*i!=0&&*/player[k].skeleton.oldfree==1&&animation[player[i].currentanimation].attack==neutral))||
                                        (player[i].targetanimation==jumpupanim||player[i].targetanimation==jumpdownanim||player[i].isFlip())&&
                                        (player[k].targetanimation==jumpupanim||player[k].targetanimation==jumpdownanim||player[k].isFlip())&&
                                        k==0&&!player[i].skeleton.oldfree&&!player[k].skeleton.oldfree){
                                    //If hit by body
                                    if(     (i!=0||player[i].skeleton.free)&&
                                            (k!=0||player[k].skeleton.free)||
                                            (animation[player[i].targetanimation].height==highheight&&
                                             animation[player[k].targetanimation].height==highheight)){
                                        if(tutoriallevel!=1){
                                            emit_sound_at(heavyimpactsound, player[i].coords);
                                        }

                                        player[i].RagDoll(0);
                                        if(player[i].damage>player[i].damagetolerance-findLengthfast(&rotatetarget)/4&&!player[i].dead){
                                          award_bonus(0, aimbonus);
                                        }
                                        player[i].DoDamage(findLengthfast(&rotatetarget)/4);
                                        player[k].RagDoll(0);
                                        if(player[k].damage>player[k].damagetolerance-findLengthfast(&rotatetarget)/4&&!player[k].dead){
                                          award_bonus(0, aimbonus); // Huh, again?
                                        }
                                        player[k].DoDamage(findLengthfast(&rotatetarget)/4);

                                        for(int j=0;j<player[i].skeleton.num_joints;j++){
                                            player[i].skeleton.joints[j].velocity=player[i].skeleton.joints[j].velocity/5+player[k].velocity;
                                        }
                                        for(int j=0;j<player[k].skeleton.num_joints;j++){
                                            player[k].skeleton.joints[j].velocity=player[k].skeleton.joints[j].velocity/5+player[i].velocity;
                                        }

                                    }
                                }
                            if(     (animation[player[i].targetanimation].attack==neutral||
                                     animation[player[i].targetanimation].attack==normalattack)&&
                                    (animation[player[k].targetanimation].attack==neutral||
                                     animation[player[k].targetanimation].attack==normalattack)){
                                //If bumped
                                if(player[i].skeleton.oldfree==0&&player[k].skeleton.oldfree==0){
                                    if(findDistancefast(&player[k].coords,&player[i].coords)<.5*sq((player[i].scale+player[k].scale)*2.5)){
                                        rotatetarget=player[k].coords-player[i].coords;
                                        Normalise(&rotatetarget);
                                        player[k].coords=(player[k].coords+player[i].coords)/2;
                                        player[i].coords=player[k].coords-rotatetarget*fast_sqrt(.6)/2
                                            *sq((player[i].scale+player[k].scale)*2.5);
                                        player[k].coords+=rotatetarget*fast_sqrt(.6)/2*sq((player[i].scale+player[k].scale)*2.5);
                                        if(player[k].howactive==typeactive||hostile)
                                            if(player[k].isIdle()){
                                                if(player[k].howactive<typesleeping)
                                                    setAnimation(k,player[k].getStop());
                                                else if(player[k].howactive==typesleeping)
                                                    setAnimation(k,getupfromfrontanim);
                                                if(!editorenabled)
                                                    player[k].howactive=typeactive;
                                            }
                                        if(player[i].howactive==typeactive||hostile)
                                            if(player[i].isIdle()){
                                                if(player[i].howactive<typesleeping)
                                                    setAnimation(i,player[k].getStop());
                                                else
                                                    setAnimation(i,getupfromfrontanim);
                                                if(!editorenabled)
                                                    player[i].howactive=typeactive;
                                            }
                                    }
                                    //jump down on player
                                    if(hostile){
                                        if(k==0&&i!=0&&player[k].targetanimation==jumpdownanim&&
                                                !player[i].isCrouch()&&
                                                player[i].targetanimation!=rollanim&&
                                                !player[k].skeleton.oldfree&&!
                                                player[k].skeleton.free&&
                                                player[k].lastcollide<=0&&
                                                player[k].velocity.y<-10){
                                            player[i].velocity=player[k].velocity;
                                            player[k].velocity=player[k].velocity*-.5;
                                            player[k].velocity.y=player[i].velocity.y;
                                            player[i].DoDamage(20);
                                            player[i].RagDoll(0);
                                            player[k].lastcollide=1;
                                            award_bonus(k, AboveBonus);
                                        }
                                        if(i==0&&k!=0&&player[i].targetanimation==jumpdownanim&&
                                                !player[k].isCrouch()&&
                                                player[k].targetanimation!=rollanim&&
                                                !player[i].skeleton.oldfree&&
                                                !player[i].skeleton.free&&
                                                player[i].lastcollide<=0&&
                                                player[i].velocity.y<-10){
                                            player[k].velocity=player[i].velocity;
                                            player[i].velocity=player[i].velocity*-.3;
                                            player[i].velocity.y=player[k].velocity.y;
                                            player[k].DoDamage(20);
                                            player[k].RagDoll(0);
                                            player[i].lastcollide=1;
                                            award_bonus(i, AboveBonus);
                                        }
                                    }
                                }
                            }
                        }
                        player[i].CheckKick();
                        player[k].CheckKick();
                    }
                }
            }
}



void Game::doAI(int i){
    static bool connected;
    if(player[i].aitype!=playercontrolled&&indialogue==-1){
        player[i].jumpclimb=0;
        //disable movement in editor
        if(editorenabled)
            player[i].stunned=1;

        player[i].pause=0;
        if(findDistancefastflat(&player[0].coords,&player[i].coords)<30&&
                player[0].coords.y>player[i].coords.y+2&&
                !player[0].onterrain)
            player[i].pause=1;

        //pathfinding
        if(player[i].aitype==pathfindtype){
            if(player[i].finalpathfindpoint==-1){
                float closestdistance;
                float tempdist;
                int closest;
                XYZ colpoint;
                closest=-1;
                closestdistance=-1;
                for(int j=0;j<numpathpoints;j++)
                    if(closest==-1||findDistancefast(&player[i].finalfinaltarget,&pathpoint[j])<closestdistance){
                        closestdistance=findDistancefast(&player[i].finalfinaltarget,&pathpoint[j]);
                        closest=j;
                        player[i].finaltarget=pathpoint[j];
                    }
                player[i].finalpathfindpoint=closest;
                for(int j=0;j<numpathpoints;j++)
                    for(int k=0;k<numpathpointconnect[j];k++){
                        DistancePointLine(&player[i].finalfinaltarget, &pathpoint[j], &pathpoint[pathpointconnect[j][k]], &tempdist,&colpoint );
                        if(sq(tempdist)<closestdistance)
                            if(findDistance(&colpoint,&pathpoint[j])+findDistance(&colpoint,&pathpoint[pathpointconnect[j][k]])<
                                    findDistance(&pathpoint[j],&pathpoint[pathpointconnect[j][k]])+.1){
                                closestdistance=sq(tempdist);
                                closest=j;
                                player[i].finaltarget=colpoint;
                            }
                    }
                player[i].finalpathfindpoint=closest;

            }
            if(player[i].targetpathfindpoint==-1){
                float closestdistance;
                float tempdist;
                int closest;
                XYZ colpoint;
                closest=-1;
                closestdistance=-1;
                if(player[i].lastpathfindpoint==-1){
                    for(int j=0;j<numpathpoints;j++){
                        if(j!=player[i].lastpathfindpoint)
                            if(closest==-1||(findDistancefast(&player[i].coords,&pathpoint[j])<closestdistance)){
                                closestdistance=findDistancefast(&player[i].coords,&pathpoint[j]);
                                closest=j;
                            }
                    }
                    player[i].targetpathfindpoint=closest;
                    for(int j=0;j<numpathpoints;j++)
                        if(j!=player[i].lastpathfindpoint)
                            for(int k=0;k<numpathpointconnect[j];k++){
                                DistancePointLine(&player[i].coords, &pathpoint[j], &pathpoint[pathpointconnect[j][k]], &tempdist,&colpoint );
                                if(sq(tempdist)<closestdistance){
                                    if(findDistance(&colpoint,&pathpoint[j])+findDistance(&colpoint,&pathpoint[pathpointconnect[j][k]])<
                                            findDistance(&pathpoint[j],&pathpoint[pathpointconnect[j][k]])+.1){
                                        closestdistance=sq(tempdist);
                                        closest=j;
                                    }
                                }
                            }
                    player[i].targetpathfindpoint=closest;
                }
                else
                {
                    for(int j=0;j<numpathpoints;j++)
                        if(j!=player[i].lastpathfindpoint&&
                                j!=player[i].lastpathfindpoint2&&
                                j!=player[i].lastpathfindpoint3&&
                                j!=player[i].lastpathfindpoint4){
                            connected=0;
                            if(numpathpointconnect[j])
                                for(int k=0;k<numpathpointconnect[j];k++)
                                    if(pathpointconnect[j][k]==player[i].lastpathfindpoint)
                                        connected=1;
                            if(!connected)
                                if(numpathpointconnect[player[i].lastpathfindpoint])
                                    for(int k=0;k<numpathpointconnect[player[i].lastpathfindpoint];k++)
                                        if(pathpointconnect[player[i].lastpathfindpoint][k]==j)
                                            connected=1;
                            if(connected){
                                tempdist=findPathDist(j,player[i].finalpathfindpoint);
                                if(closest==-1||tempdist<closestdistance){
                                    closestdistance=tempdist;
                                    closest=j;
                                }
                            }
                        }
                    player[i].targetpathfindpoint=closest;
                }
            }
            player[i].losupdatedelay-=multiplier;

            player[i].targetrotation=roughDirectionTo(player[i].coords,pathpoint[player[i].targetpathfindpoint]);
            player[i].lookrotation=player[i].targetrotation;

            //reached target point
            if(findDistancefastflat(&player[i].coords,&pathpoint[player[i].targetpathfindpoint])<.6){
                player[i].lastpathfindpoint4=player[i].lastpathfindpoint3;
                player[i].lastpathfindpoint3=player[i].lastpathfindpoint2;
                player[i].lastpathfindpoint2=player[i].lastpathfindpoint;
                player[i].lastpathfindpoint=player[i].targetpathfindpoint;
                if(player[i].lastpathfindpoint2==-1)
                    player[i].lastpathfindpoint2=player[i].lastpathfindpoint;
                if(player[i].lastpathfindpoint3==-1)
                    player[i].lastpathfindpoint3=player[i].lastpathfindpoint2;
                if(player[i].lastpathfindpoint4==-1)
                    player[i].lastpathfindpoint4=player[i].lastpathfindpoint3;
                player[i].targetpathfindpoint=-1;
            }
            if(     findDistancefastflat(&player[i].coords,&player[i].finalfinaltarget)<
                    findDistancefastflat(&player[i].coords,&player[i].finaltarget)||
                    findDistancefastflat(&player[i].coords,&player[i].finaltarget)<.6*sq(player[i].scale*5)||
                    player[i].lastpathfindpoint==player[i].finalpathfindpoint){
                player[i].aitype=passivetype;
            }

            player[i].forwardkeydown=1;
            player[i].leftkeydown=0;
            player[i].backkeydown=0;
            player[i].rightkeydown=0;
            player[i].crouchkeydown=0;
            player[i].attackkeydown=0;
            player[i].throwkeydown=0;

            if(player[i].avoidcollided>.8&&!player[i].jumpkeydown&&player[i].collided<.8)
                player[i].targetrotation+=90*(player[i].whichdirection*2-1);

            if(player[i].collided<1||player[i].targetanimation!=jumpupanim)
                player[i].jumpkeydown=0;
            if((player[i].collided>.8&&player[i].jumppower>=5))
                player[i].jumpkeydown=1;

            if((tutoriallevel!=1||cananger)&&
                    hostile&&
                    !player[0].dead&&
                    findDistancefast(&player[i].coords,&player[0].coords)<400&&
                    player[i].occluded<25){
                if(findDistancefast(&player[i].coords,&player[0].coords)<12&&
                        animation[player[0].targetanimation].height!=lowheight&&
                        !editorenabled&&
                        (player[0].coords.y<player[i].coords.y+5||player[0].onterrain))
                    player[i].aitype=attacktypecutoff;
                if(findDistancefast(&player[i].coords,&player[0].coords)<30&&
                        animation[player[0].targetanimation].height==highheight&&
                        !editorenabled)
                    player[i].aitype=attacktypecutoff;

                if(player[i].losupdatedelay<0&&!editorenabled&&player[i].occluded<2){
                    player[i].losupdatedelay=.2;
                    for(int j=0;j<numplayers;j++)
                        if(j==0||player[j].skeleton.free||player[j].aitype!=passivetype)
                            if(abs(Random()%2)||animation[player[j].targetanimation].height!=lowheight||j!=0)
                                if(findDistancefast(&player[i].coords,&player[j].coords)<400)
                                    if(normaldotproduct(player[i].facing,player[j].coords-player[i].coords)>0)
                                        if(player[j].coords.y<player[i].coords.y+5||player[j].onterrain)
                                            if(!player[j].isWallJump()&&-1==checkcollide(
                                                            DoRotation(playerJoint(i,head).position,0,player[i].rotation,0)
                                                                *player[i].scale+player[i].coords,
                                                            DoRotation(playerJoint(j,head).position,0,player[j].rotation,0)
                                                                *player[j].scale+player[j].coords)||
                                                    (player[j].targetanimation==hanganim&&
                                                     normaldotproduct(player[j].facing,player[i].coords-player[j].coords)<0)){
                                                player[i].aitype=searchtype;
                                                player[i].lastchecktime=12;
                                                player[i].lastseen=player[j].coords;
                                                player[i].lastseentime=12;
                                            }
                }
            }
            if(player[i].aitype==attacktypecutoff&&musictype!=2)
                if(player[i].creature!=wolftype){
                    player[i].stunned=.6;
                    player[i].surprised=.6;
                }
        }

        if(player[i].aitype!=passivetype&&leveltime>.5)
            player[i].howactive=typeactive;

        if(player[i].aitype==passivetype){
            player[i].aiupdatedelay-=multiplier;
            player[i].losupdatedelay-=multiplier;
            player[i].lastseentime+=multiplier;
            player[i].pausetime-=multiplier;
            if(player[i].lastseentime>1)
                player[i].lastseentime=1;

            if(player[i].aiupdatedelay<0){
                if(player[i].numwaypoints>1&&player[i].howactive==typeactive&&player[i].pausetime<=0){
                    player[i].targetrotation=roughDirectionTo(player[i].coords,player[i].waypoints[player[i].waypoint]);
                    player[i].lookrotation=player[i].targetrotation;
                    player[i].aiupdatedelay=.05;

                    if(findDistancefastflat(&player[i].coords,&player[i].waypoints[player[i].waypoint])<1){
                        if(player[i].waypointtype[player[i].waypoint]==wppause)
                            player[i].pausetime=4;
                        player[i].waypoint++;
                        if(player[i].waypoint>player[i].numwaypoints-1)
                            player[i].waypoint=0;

                    }
                }

                if(player[i].numwaypoints>1&&player[i].howactive==typeactive&&player[i].pausetime<=0)
                    player[i].forwardkeydown=1;
                else
                    player[i].forwardkeydown=0;
                player[i].leftkeydown=0;
                player[i].backkeydown=0;
                player[i].rightkeydown=0;
                player[i].crouchkeydown=0;
                player[i].attackkeydown=0;
                player[i].throwkeydown=0;

                if(player[i].avoidcollided>.8&&!player[i].jumpkeydown&&player[i].collided<.8){
                    if(!player[i].avoidsomething)
                        player[i].targetrotation+=90*(player[i].whichdirection*2-1);
                    else{
                        XYZ leftpos,rightpos;
                        float leftdist,rightdist;
                        leftpos = player[i].coords+DoRotation(player[i].facing,0,90,0);
                        rightpos = player[i].coords-DoRotation(player[i].facing,0,90,0);
                        leftdist = findDistancefast(&leftpos, &player[i].avoidwhere);
                        rightdist = findDistancefast(&rightpos, &player[i].avoidwhere);
                        if(leftdist<rightdist)
                            player[i].targetrotation+=90;
                        else
                            player[i].targetrotation-=90;
                    }
                }
            }
            if(player[i].collided<1||player[i].targetanimation!=jumpupanim)
                player[i].jumpkeydown=0;
            if((player[i].collided>.8&&player[i].jumppower>=5))
                player[i].jumpkeydown=1;


            //hearing sounds
            if(!editorenabled){
                if(player[i].howactive<=typesleeping)
                    if(numenvsounds>0&&(tutoriallevel!=1||cananger)&&hostile)
                        for(int j=0;j<numenvsounds;j++){
                            float vol=player[i].howactive==typesleeping?envsoundvol[j]-14:envsoundvol[j];
                            if(vol>0&&findDistancefast(&player[i].coords,&envsound[j])<
                                    2*(vol+vol*(player[i].creature==rabbittype)*3))
                                player[i].aitype=attacktypecutoff;
                        }

                if(player[i].aitype!=passivetype){
                    if(player[i].howactive==typesleeping)
                        setAnimation(i,getupfromfrontanim);
                    player[i].howactive=typeactive;
                }
            }

            if(player[i].howactive<typesleeping&&
                    ((tutoriallevel!=1||cananger)&&hostile)&&
                    !player[0].dead&&
                    findDistancefast(&player[i].coords,&player[0].coords)<400&&
                    player[i].occluded<25){
                if(findDistancefast(&player[i].coords,&player[0].coords)<12&&
                        animation[player[0].targetanimation].height!=lowheight&&!editorenabled)
                    player[i].aitype=attacktypecutoff;
                if(findDistancefast(&player[i].coords,&player[0].coords)<30&&
                        animation[player[0].targetanimation].height==highheight&&!editorenabled)
                    player[i].aitype=attacktypecutoff;

                //wolf smell
                if(player[i].creature==wolftype){
                    XYZ windsmell;
                    for(int j=0;j<numplayers;j++){
                        if(j==0||(player[j].dead&&player[j].bloodloss>0)){
                            float smelldistance=50;
                            if(j==0&&player[j].num_weapons>0){
                                if(weapons.bloody[player[j].weaponids[0]])
                                    smelldistance=100;
                                if(player[j].num_weapons==2)
                                    if(weapons.bloody[player[j].weaponids[1]])
                                        smelldistance=100;
                            }
                            if(j!=0)
                                smelldistance=100;
                            windsmell=windvector;
                            Normalise(&windsmell);
                            windsmell=windsmell*2+player[j].coords;
                            if(findDistancefast(&player[i].coords,&windsmell)<smelldistance&&!editorenabled)
                                player[i].aitype=attacktypecutoff;
                        }
                    }
                }

                if(player[i].howactive<typesleeping&&player[i].losupdatedelay<0&&!editorenabled&&player[i].occluded<2){
                    player[i].losupdatedelay=.2;
                    for(int j=0;j<numplayers;j++){
                        if(j==0||player[j].skeleton.free||player[j].aitype!=passivetype){
                            if(abs(Random()%2)||animation[player[j].targetanimation].height!=lowheight||j!=0)
                                if(findDistancefast(&player[i].coords,&player[j].coords)<400)
                                    if(normaldotproduct(player[i].facing,player[j].coords-player[i].coords)>0)
                                        if((-1==checkcollide(
                                                        DoRotation(playerJoint(i,head).position,0,player[i].rotation,0)*
                                                            player[i].scale+player[i].coords,
                                                        DoRotation(playerJoint(j,head).position,0,player[j].rotation,0)*
                                                            player[j].scale+player[j].coords)&&
                                                    !player[j].isWallJump())||
                                                (player[j].targetanimation==hanganim&&
                                                 normaldotproduct(player[j].facing,player[i].coords-player[j].coords)<0)){
                                            player[i].lastseentime-=.2;
                                            if(j==0&&animation[player[j].targetanimation].height==lowheight)
                                                player[i].lastseentime-=.4;
                                            else
                                                player[i].lastseentime-=.6;
                                        }
                            if(player[i].lastseentime<=0){
                                player[i].aitype=searchtype;
                                player[i].lastchecktime=12;
                                player[i].lastseen=player[j].coords;
                                player[i].lastseentime=12;
                            }
                        }
                    }
                }
            }
            //alerted surprise
            if(player[i].aitype==attacktypecutoff&&musictype!=2){
                if(player[i].creature!=wolftype){
                    player[i].stunned=.6;
                    player[i].surprised=.6;
                }
                if(player[i].creature==wolftype){
                    player[i].stunned=.47;
                    player[i].surprised=.47;
                }
                numseen++;
            }
        }

        //search for player
        int j;
        if(player[i].aitype==searchtype){
            player[i].aiupdatedelay-=multiplier;
            player[i].losupdatedelay-=multiplier;
            if(!player[i].pause)
                player[i].lastseentime-=multiplier;
            player[i].lastchecktime-=multiplier;

            if(player[i].isRun()&&!player[i].onground){
                if(player[i].coords.y>terrain.getHeight(player[i].coords.x,player[i].coords.z)+10){
                    XYZ test2=player[i].coords+player[i].facing;
                    test2.y+=5;
                    XYZ test=player[i].coords+player[i].facing;
                    test.y-=10;
                    j=checkcollide(test2,test,player[i].laststanding);
                    if(j==-1)
                        j=checkcollide(test2,test);
                    if(j==-1){
                        player[i].velocity=0;
                        setAnimation(i,player[i].getStop());
                        player[i].targetrotation+=180;
                        player[i].stunned=.5;
                        //player[i].aitype=passivetype;
                        player[i].aitype=pathfindtype;
                        player[i].finalfinaltarget=player[i].waypoints[player[i].waypoint];
                        player[i].finalpathfindpoint=-1;
                        player[i].targetpathfindpoint=-1;
                        player[i].lastpathfindpoint=-1;
                        player[i].lastpathfindpoint2=-1;
                        player[i].lastpathfindpoint3=-1;
                        player[i].lastpathfindpoint4=-1;
                    }
                    else player[i].laststanding=j;
                }
            }
            //check out last seen location
            if(player[i].aiupdatedelay<0){
                player[i].targetrotation=roughDirectionTo(player[i].coords,player[i].lastseen);
                player[i].lookrotation=player[i].targetrotation;
                player[i].aiupdatedelay=.05;
                player[i].forwardkeydown=1;

                if(findDistancefastflat(&player[i].coords,&player[i].lastseen)<1*sq(player[i].scale*5)||player[i].lastchecktime<0){
                    player[i].forwardkeydown=0;
                    player[i].aiupdatedelay=1;
                    player[i].lastseen.x+=(float(Random()%100)-50)/25;
                    player[i].lastseen.z+=(float(Random()%100)-50)/25;
                    player[i].lastchecktime=3;
                }

                player[i].leftkeydown=0;
                player[i].backkeydown=0;
                player[i].rightkeydown=0;
                player[i].crouchkeydown=0;
                player[i].attackkeydown=0;
                player[i].throwkeydown=0;

                if(player[i].avoidcollided>.8&&!player[i].jumpkeydown&&player[i].collided<.8){
                    if(!player[i].avoidsomething)player[i].targetrotation+=90*(player[i].whichdirection*2-1);
                    else{
                        XYZ leftpos,rightpos;
                        float leftdist,rightdist;
                        leftpos = player[i].coords+DoRotation(player[i].facing,0,90,0);
                        rightpos = player[i].coords-DoRotation(player[i].facing,0,90,0);
                        leftdist = findDistancefast(&leftpos, &player[i].avoidwhere);
                        rightdist = findDistancefast(&rightpos, &player[i].avoidwhere);
                        if(leftdist<rightdist)player[i].targetrotation+=90;
                        else player[i].targetrotation-=90;
                    }
                }
            }
            if(player[i].collided<1||player[i].targetanimation!=jumpupanim)
                player[i].jumpkeydown=0;
            if((player[i].collided>.8&&player[i].jumppower>=5))
                player[i].jumpkeydown=1;

            if(numenvsounds>0&&((tutoriallevel!=1||cananger)&&hostile))
                for(int k=0;k<numenvsounds;k++){
                    if(findDistancefast(&player[i].coords,&envsound[k])<2*(envsoundvol[k]+envsoundvol[k]*(player[i].creature==rabbittype)*3)){
                        player[i].aitype=attacktypecutoff;
                    }
                }

            if(!player[0].dead&&
                    player[i].losupdatedelay<0&&
                    !editorenabled&&
                    player[i].occluded<2&&
                    ((tutoriallevel!=1||cananger)&&hostile)){
                player[i].losupdatedelay=.2;
                if(findDistancefast(&player[i].coords,&player[0].coords)<4&&animation[player[i].targetanimation].height!=lowheight){
                    player[i].aitype=attacktypecutoff;
                    player[i].lastseentime=1;
                }
                if(abs(Random()%2)||animation[player[i].targetanimation].height!=lowheight)
                    //TODO: factor out canSeePlayer()
                    if(findDistancefast(&player[i].coords,&player[0].coords)<400)
                        if(normaldotproduct(player[i].facing,player[0].coords-player[i].coords)>0)
                            if((checkcollide(
                                        DoRotation(playerJoint(i,head).position,0,player[i].rotation,0)*
                                            player[i].scale+player[i].coords,
                                        DoRotation(playerJoint(0,head).position,0,player[0].rotation,0)*
                                            player[0].scale+player[0].coords)==-1)||
                                    (player[0].targetanimation==hanganim&&normaldotproduct(
                                        player[0].facing,player[i].coords-player[0].coords)<0)){
                                    /* //TODO: changed j to 0 on a whim, make sure this is correct
                                    (player[j].targetanimation==hanganim&&normaldotproduct(
                                        player[j].facing,player[i].coords-player[j].coords)<0)
                                    */
                                player[i].aitype=attacktypecutoff;
                                player[i].lastseentime=1;
                            }
            }
            //player escaped
            if(player[i].lastseentime<0){
                //player[i].aitype=passivetype;
                numescaped++;
                player[i].aitype=pathfindtype;
                player[i].finalfinaltarget=player[i].waypoints[player[i].waypoint];
                player[i].finalpathfindpoint=-1;
                player[i].targetpathfindpoint=-1;
                player[i].lastpathfindpoint=-1;
                player[i].lastpathfindpoint2=-1;
                player[i].lastpathfindpoint3=-1;
                player[i].lastpathfindpoint4=-1;
            }
        }

        if(player[i].aitype!=gethelptype)
            player[i].runninghowlong=0;

        //get help from buddies
        if(player[i].aitype==gethelptype){
            player[i].runninghowlong+=multiplier;
            player[i].aiupdatedelay-=multiplier;

            if(player[i].aiupdatedelay<0||player[i].ally==0){
                player[i].aiupdatedelay=.2;

                //find closest ally
                //TODO: factor out closest search somehow
                if(!player[i].ally){
                    int closest=-1;
                    float closestdist=-1;
                    for(int k=0;k<numplayers;k++){
                        if(k!=i&&k!=0&&!player[k].dead&&
                                player[k].howactive<typedead1&&
                                !player[k].skeleton.free&&
                                player[k].aitype==passivetype){
                            float distance=findDistancefast(&player[i].coords,&player[k].coords);
                            if(closestdist==-1||distance<closestdist){
                                closestdist=distance;
                                closest=k;
                            }
                            closest=k;
                        }
                    }
                    if(closest!=-1)
                        player[i].ally=closest;
                    else
                        player[i].ally=0;
                    player[i].lastseen=player[0].coords;
                    player[i].lastseentime=12;
                }


                player[i].lastchecktime=12;

                XYZ facing=player[i].coords;
                XYZ flatfacing=player[player[i].ally].coords;
                facing.y+=playerJoint(i,head).position.y*player[i].scale;
                flatfacing.y+=playerJoint(player[i].ally,head).position.y*player[player[i].ally].scale;
                if(-1!=checkcollide(facing,flatfacing))
                    player[i].lastseentime-=.1;

                //no available ally, run back to player
                if(player[i].ally<=0||
                        player[player[i].ally].skeleton.free||
                        player[player[i].ally].aitype!=passivetype||
                        player[i].lastseentime<=0){
                    player[i].aitype=searchtype;
                    player[i].lastseentime=12;
                }

                //seek out ally
                if(player[i].ally>0){
                    player[i].targetrotation=roughDirectionTo(player[i].coords,player[player[i].ally].coords);
                    player[i].lookrotation=player[i].targetrotation;
                    player[i].aiupdatedelay=.05;
                    player[i].forwardkeydown=1;

                    if(findDistancefastflat(&player[i].coords,&player[player[i].ally].coords)<3){
                        player[i].aitype=searchtype;
                        player[i].lastseentime=12;
                        player[player[i].ally].aitype=searchtype;
                        if(player[player[i].ally].lastseentime<player[i].lastseentime){
                            player[player[i].ally].lastseen=player[i].lastseen;
                            player[player[i].ally].lastseentime=player[i].lastseentime;
                            player[player[i].ally].lastchecktime=player[i].lastchecktime;
                        }
                    }

                    if(player[i].avoidcollided>.8&&!player[i].jumpkeydown&&player[i].collided<.8){
                        if(!player[i].avoidsomething)
                            player[i].targetrotation+=90*(player[i].whichdirection*2-1);
                        else{
                            XYZ leftpos,rightpos;
                            float leftdist,rightdist;
                            leftpos = player[i].coords+DoRotation(player[i].facing,0,90,0);
                            rightpos = player[i].coords-DoRotation(player[i].facing,0,90,0);
                            leftdist = findDistancefast(&leftpos, &player[i].avoidwhere);
                            rightdist = findDistancefast(&rightpos, &player[i].avoidwhere);
                            if(leftdist<rightdist)
                                player[i].targetrotation+=90;
                            else
                                player[i].targetrotation-=90;
                        }
                    }
                }

                player[i].leftkeydown=0;
                player[i].backkeydown=0;
                player[i].rightkeydown=0;
                player[i].crouchkeydown=0;
                player[i].attackkeydown=0;
            }
            if(player[i].collided<1||player[i].targetanimation!=jumpupanim)
                player[i].jumpkeydown=0;
            if(player[i].collided>.8&&player[i].jumppower>=5)
                player[i].jumpkeydown=1;
        }

        //retreiving a weapon on the ground
        if(player[i].aitype==getweapontype){
            player[i].aiupdatedelay-=multiplier;
            player[i].lastchecktime-=multiplier;

            if(player[i].aiupdatedelay<0){
                player[i].aiupdatedelay=.2;

                //ALLY IS WEPON
                if(player[i].ally<0){
                    int closest=-1;
                    float closestdist=-1;
                    for(int k=0;k<weapons.numweapons;k++)
                        if(weapons.owner[k]==-1){
                            float distance=findDistancefast(&player[i].coords,&weapons.position[k]);
                            if(closestdist==-1||distance<closestdist){
                                closestdist=distance;
                                closest=k;
                            }
                            closest=k;
                        }
                    if(closest!=-1)
                        player[i].ally=closest;
                    else
                        player[i].ally=-1;
                }

                player[i].lastseentime=12;

                if(!player[0].dead&&((tutoriallevel!=1||cananger)&&hostile))
                    if(player[i].ally<0||player[i].weaponactive!=-1||player[i].lastchecktime<=0){
                        player[i].aitype=attacktypecutoff;
                        player[i].lastseentime=1;
                    }
                if(!player[0].dead)
                    if(player[i].ally>=0){
                        if(weapons.owner[player[i].ally]!=-1||
                                findDistancefast(&player[i].coords,&weapons.position[player[i].ally])>16){
                            player[i].aitype=attacktypecutoff;
                            player[i].lastseentime=1;
                        }
                        //TODO: factor these out as moveToward()
                        player[i].targetrotation=roughDirectionTo(player[i].coords,weapons.position[player[i].ally]);
                        player[i].lookrotation=player[i].targetrotation;
                        player[i].aiupdatedelay=.05;
                        player[i].forwardkeydown=1;


                        if(player[i].avoidcollided>.8&&!player[i].jumpkeydown&&player[i].collided<.8){
                            if(!player[i].avoidsomething)
                                player[i].targetrotation+=90*(player[i].whichdirection*2-1);
                            else{
                                XYZ leftpos,rightpos;
                                float leftdist,rightdist;
                                leftpos = player[i].coords+DoRotation(player[i].facing,0,90,0);
                                rightpos = player[i].coords-DoRotation(player[i].facing,0,90,0);
                                leftdist = findDistancefast(&leftpos, &player[i].avoidwhere);
                                rightdist = findDistancefast(&rightpos, &player[i].avoidwhere);
                                if(leftdist<rightdist)
                                    player[i].targetrotation+=90;
                                else
                                    player[i].targetrotation-=90;
                            }
                        }
                    }

                player[i].leftkeydown=0;
                player[i].backkeydown=0;
                player[i].rightkeydown=0;
                player[i].attackkeydown=0;
                player[i].throwkeydown=1;
                player[i].crouchkeydown=0;
                if(player[i].targetanimation!=crouchremoveknifeanim&&
                        player[i].targetanimation!=removeknifeanim)
                    player[i].throwtogglekeydown=0;
                player[i].drawkeydown=0;
            }
            if(player[i].collided<1||player[i].targetanimation!=jumpupanim)
                player[i].jumpkeydown=0;
            if((player[i].collided>.8&&player[i].jumppower>=5))
                player[i].jumpkeydown=1;
        }

        if(player[i].aitype==attacktypecutoff){
            player[i].aiupdatedelay-=multiplier;
            //dodge or reverse rabbit kicks, knife throws, flips
            if(player[i].damage<player[i].damagetolerance*2/3)
                if((player[0].targetanimation==rabbitkickanim||
                            player[0].targetanimation==knifethrowanim||
                            (player[0].isFlip()&&
                             normaldotproduct(player[0].facing,player[0].coords-player[i].coords)<0))&&
                        !player[0].skeleton.free&&
                        (player[i].aiupdatedelay<.1)){
                    player[i].attackkeydown=0;
                    if(player[i].isIdle())
                        player[i].crouchkeydown=1;
                    if(player[0].targetanimation!=rabbitkickanim&&player[0].weaponactive!=-1){
                        if(weapons.type[player[0].weaponids[0]]==knife){
                            if(player[i].isIdle()||player[i].isCrouch()||player[i].isRun()||player[i].isFlip()){
                                if(abs(Random()%2==0))
                                    setAnimation(i,backhandspringanim);
                                else
                                    setAnimation(i,rollanim);
                                player[i].targetrotation+=90*(abs(Random()%2)*2-1);
                                player[i].wentforweapon=0;
                            }
                            if(player[i].targetanimation==jumpupanim||player[i].targetanimation==jumpdownanim)
                                setAnimation(i,flipanim);
                        }
                    }
                    player[i].forwardkeydown=0;
                    player[i].aiupdatedelay=.02;
                }
            //get confused by flips
            if(player[0].isFlip()&&
                    !player[0].skeleton.free&&
                    player[0].targetanimation!=walljumprightkickanim&&
                    player[0].targetanimation!=walljumpleftkickanim){
                if(findDistancefast(&player[0].coords,&player[i].coords)<25)
                    if((1-player[i].damage/player[i].damagetolerance)>.5)
                        player[i].stunned=1;
            }
            //go for weapon on the ground
            if(player[i].wentforweapon<3)
                for(int k=0;k<weapons.numweapons;k++)
                    if(player[i].creature!=wolftype)
                        if(player[i].num_weapons==0&&
                                weapons.owner[k]==-1&&
                                weapons.velocity[i].x==0&&
                                weapons.velocity[i].z==0&&
                                weapons.velocity[i].y==0){
                            if(findDistancefast(&player[i].coords,&weapons.position[k])<16){
                                player[i].wentforweapon++;
                                player[i].lastchecktime=6;
                                player[i].aitype=getweapontype;
                                player[i].ally=-1;
                            }
                        }
            //dodge/reverse walljump kicks
            if(player[i].damage<player[i].damagetolerance/2)
                if(animation[player[i].targetanimation].height!=highheight)
                    if(player[i].damage<player[i].damagetolerance*.5&&
                            ((player[0].targetanimation==walljumprightkickanim||
                              player[0].targetanimation==walljumpleftkickanim)&&
                             ((player[i].aiupdatedelay<.15&&
                               difficulty==2)||
                              (player[i].aiupdatedelay<.08&&
                               difficulty!=2)))){
                        player[i].crouchkeydown=1;
                    }
            //walked off a ledge (?)
            if(player[i].isRun()&&!player[i].onground)
                if(player[i].coords.y>terrain.getHeight(player[i].coords.x,player[i].coords.z)+10){
                    XYZ test2=player[i].coords+player[i].facing;
                    test2.y+=5;
                    XYZ test=player[i].coords+player[i].facing;
                    test.y-=10;
                    j=checkcollide(test2,test,player[i].laststanding);
                    if(j==-1)
                        j=checkcollide(test2,test);
                    if(j==-1){
                        player[i].velocity=0;
                        setAnimation(i,player[i].getStop());
                        player[i].targetrotation+=180;
                        player[i].stunned=.5;
                        player[i].aitype=pathfindtype;
                        player[i].finalfinaltarget=player[i].waypoints[player[i].waypoint];
                        player[i].finalpathfindpoint=-1;
                        player[i].targetpathfindpoint=-1;
                        player[i].lastpathfindpoint=-1;
                        player[i].lastpathfindpoint2=-1;
                        player[i].lastpathfindpoint3=-1;
                        player[i].lastpathfindpoint4=-1;
                    }else
                        player[i].laststanding=j;
                }
            //lose sight of player in the air (?)
            if(player[0].coords.y>player[i].coords.y+5&&
                    animation[player[0].targetanimation].height!=highheight&&
                    !player[0].onterrain){
                player[i].aitype=pathfindtype;
                player[i].finalfinaltarget=player[i].waypoints[player[i].waypoint];
                player[i].finalpathfindpoint=-1;
                player[i].targetpathfindpoint=-1;
                player[i].lastpathfindpoint=-1;
                player[i].lastpathfindpoint2=-1;
                player[i].lastpathfindpoint3=-1;
                player[i].lastpathfindpoint4=-1;
            }
            //it's time to think (?)
            if(player[i].aiupdatedelay<0&&
                    !animation[player[i].targetanimation].attack&&
                    player[i].targetanimation!=staggerbackhighanim&&
                    player[i].targetanimation!=staggerbackhardanim&&
                    player[i].targetanimation!=backhandspringanim&&
                    player[i].targetanimation!=dodgebackanim){
                //draw weapon
                if(player[i].weaponactive==-1&&player[i].num_weapons>0)
                    player[i].drawkeydown=Random()%2;
                else
                    player[i].drawkeydown=0;
                player[i].rabbitkickenabled=Random()%2;
                //chase player
                XYZ rotatetarget=player[0].coords+player[0].velocity;
                XYZ targetpoint=player[0].coords;
                if(findDistancefast(&player[0].coords,&player[i].coords)<
                        findDistancefast(&rotatetarget,&player[i].coords))
                    targetpoint+=player[0].velocity*
                        findDistance(&player[0].coords,&player[i].coords)/findLength(&player[i].velocity);
                player[i].targetrotation=roughDirectionTo(player[i].coords,targetpoint);
                player[i].lookrotation=player[i].targetrotation;
                player[i].aiupdatedelay=.2+fabs((float)(Random()%100)/1000);

                if(findDistancefast(&player[i].coords,&player[0].coords)>5&&(player[0].weaponactive==-1||player[i].weaponactive!=-1))
                    player[i].forwardkeydown=1;
                else if((findDistancefast(&player[i].coords,&player[0].coords)>16||
                            findDistancefast(&player[i].coords,&player[0].coords)<9)&&
                        player[0].weaponactive!=-1)
                    player[i].forwardkeydown=1;
                else if(Random()%6==0||(player[i].creature==wolftype&&Random()%3==0))
                    player[i].forwardkeydown=1;
                else
                    player[i].forwardkeydown=0;
                //chill out around the corpse
                if(player[0].dead){
                    player[i].forwardkeydown=0;
                    if(Random()%10==0)
                        player[i].forwardkeydown=1;
                    if(Random()%100==0){
                        player[i].aitype=pathfindtype;
                        player[i].finalfinaltarget=player[i].waypoints[player[i].waypoint];
                        player[i].finalpathfindpoint=-1;
                        player[i].targetpathfindpoint=-1;
                        player[i].lastpathfindpoint=-1;
                        player[i].lastpathfindpoint2=-1;
                        player[i].lastpathfindpoint3=-1;
                        player[i].lastpathfindpoint4=-1;
                    }
                }
                player[i].leftkeydown=0;
                player[i].backkeydown=0;
                player[i].rightkeydown=0;
                player[i].crouchkeydown=0;
                player[i].throwkeydown=0;

                if(player[i].avoidcollided>.8&&!player[i].jumpkeydown&&player[i].collided<.8)
                    player[i].targetrotation+=90*(player[i].whichdirection*2-1);
                //attack!!!
                if(Random()%2==0||player[i].weaponactive!=-1||player[i].creature==wolftype)
                    player[i].attackkeydown=1;
                else
                    player[i].attackkeydown=0;
                if(player[i].isRun()&&Random()%6&&findDistancefast(&player[i].coords,&player[0].coords)>7)
                    player[i].attackkeydown=0;

                //TODO: wat
                if(player[i].aitype!=playercontrolled&&
                        (player[i].isIdle()||
                         player[i].isCrouch()||
                         player[i].isRun())){
                    int target=-2;
                    for(int j=0;j<numplayers;j++)
                        if(j!=i&&!player[j].skeleton.free&&
                                player[j].hasvictim&&
                                (tutoriallevel==1&&reversaltrain||
                                 Random()%2==0&&difficulty==2||
                                 Random()%4==0&&difficulty==1||
                                 Random()%8==0&&difficulty==0||
                                 player[j].lastattack2==player[j].targetanimation&&
                                 player[j].lastattack3==player[j].targetanimation&&
                                 (Random()%2==0||difficulty==2)||
                                 (player[i].isIdle()||player[i].isRun())&&
                                 player[j].weaponactive!=-1||
                                 player[j].targetanimation==swordslashanim&&
                                 player[i].weaponactive!=-1||
                                 player[j].targetanimation==staffhitanim||
                                 player[j].targetanimation==staffspinhitanim))
                            if(findDistancefast(&player[j].coords,&player[j].victim->coords)<4&&
                                    player[j].victim==&player[i]&&
                                    (player[j].targetanimation==sweepanim||
                                     player[j].targetanimation==spinkickanim||
                                     player[j].targetanimation==staffhitanim||
                                     player[j].targetanimation==staffspinhitanim||
                                     player[j].targetanimation==winduppunchanim||
                                     player[j].targetanimation==upunchanim||
                                     player[j].targetanimation==wolfslapanim||
                                     player[j].targetanimation==knifeslashstartanim||
                                     player[j].targetanimation==swordslashanim&&
                                      (findDistancefast(&player[j].coords,&player[i].coords)<2||
                                       player[i].weaponactive!=-1))){
                                if(target>=0)
                                    target=-1;
                                else
                                    target=j;
                            }
                    if(target>=0)
                        player[target].Reverse();
                }

                if(player[i].collided<1)
                    player[i].jumpkeydown=0;
                if(player[i].collided>.8&&player[i].jumppower>=5||
                        findDistancefast(&player[i].coords,&player[0].coords)>400&&
                        player[i].onterrain&&
                        player[i].creature==rabbittype)
                    player[i].jumpkeydown=1;
                //TODO: why are we controlling the human?
                if(normaldotproduct(player[i].facing,player[0].coords-player[i].coords)>0)
                    player[0].jumpkeydown=0;
                if(player[0].targetanimation==jumpdownanim&&
                        findDistancefast(&player[0].coords,&player[i].coords)<40)
                    player[i].crouchkeydown=1;
                if(player[i].jumpkeydown)
                    player[i].attackkeydown=0;

                if(tutoriallevel==1)
                    if(!canattack)
                        player[i].attackkeydown=0;


                XYZ facing=player[i].coords;
                XYZ flatfacing=player[0].coords;
                facing.y+=playerJoint(i,head).position.y*player[i].scale;
                flatfacing.y+=playerJoint(0,head).position.y*player[0].scale;
                if(player[i].occluded>=2)
                    if(-1!=checkcollide(facing,flatfacing)){
                        if(!player[i].pause)
                            player[i].lastseentime-=.2;
                        if(player[i].lastseentime<=0&&
                                (player[i].creature!=wolftype||
                                 player[i].weaponstuck==-1)){
                            player[i].aitype=searchtype;
                            player[i].lastchecktime=12;
                            player[i].lastseen=player[0].coords;
                            player[i].lastseentime=12;
                        }
                    }else
                        player[i].lastseentime=1;
            }
        }
        if(animation[player[0].targetanimation].height==highheight&&
                (player[i].aitype==attacktypecutoff||
                 player[i].aitype==searchtype))
            if(player[0].coords.y>terrain.getHeight(player[0].coords.x,player[0].coords.z)+10){
                XYZ test=player[0].coords;
                test.y-=40;
                if(-1==checkcollide(player[0].coords,test))
                    player[i].stunned=1;
            }
        //stunned
        if(player[i].aitype==passivetype&&!(player[i].numwaypoints>1)||
                player[i].stunned>0||
                player[i].pause&&player[i].damage>player[i].superpermanentdamage){
            if(player[i].pause)
                player[i].lastseentime=1;
            player[i].targetrotation=player[i].rotation;
            player[i].forwardkeydown=0;
            player[i].leftkeydown=0;
            player[i].backkeydown=0;
            player[i].rightkeydown=0;
            player[i].jumpkeydown=0;
            player[i].attackkeydown=0;
            player[i].crouchkeydown=0;
            player[i].throwkeydown=0;
        }


        XYZ facing;
        facing=0;
        facing.z=-1;

        XYZ flatfacing=DoRotation(facing,0,player[i].rotation+180,0);
        facing=flatfacing;

        if(player[i].aitype==attacktypecutoff){
            player[i].targetheadrotation=180-roughDirectionTo(player[i].coords,player[0].coords);
            player[i].targetheadrotation2=pitchTo(player[i].coords,player[0].coords);
        }else if(player[i].howactive>=typesleeping){
            player[i].targetheadrotation=player[i].targetrotation;
            player[i].targetheadrotation2=0;
        }else{
            if(player[i].interestdelay<=0){
                player[i].interestdelay=.7+(float)(abs(Random()%100))/100;
                player[i].headtarget=player[i].coords;
                player[i].headtarget.x+=(float)(abs(Random()%200)-100)/100;
                player[i].headtarget.z+=(float)(abs(Random()%200)-100)/100;
                player[i].headtarget.y+=(float)(abs(Random()%200)-100)/300;
                player[i].headtarget+=player[i].facing*1.5;
            }
            player[i].targetheadrotation=180-roughDirectionTo(player[i].coords,player[i].headtarget);
            player[i].targetheadrotation2=pitchTo(player[i].coords,player[i].headtarget);
        }
    }
}



void Game::Tick(){
	static XYZ facing,flatfacing;
	static int target;

	int templength;

	for(int i=0;i<15;i++){
		displaytime[i]+=multiplier;
	}

	keyboardfrozen=0;
    Input::Tick();

	if(Input::isKeyPressed(SDLK_F6)){
		if(Input::isKeyDown(SDLK_LSHIFT))
			stereoreverse=true;
		else
			stereoreverse=false;

		if(stereoreverse)
			printf("Stereo reversed\n");
		else
			printf("Stereo unreversed\n");
	}

	if(Input::isKeyDown(SDLK_F7)){
		if(Input::isKeyDown(SDLK_LSHIFT))
			stereoseparation -= 0.001;
		else
			stereoseparation -= 0.010;
		printf("Stereo decreased increased to %f\n", stereoseparation);
	}

	if(Input::isKeyDown(SDLK_F8)){
		if(Input::isKeyDown(SDLK_LSHIFT))
			stereoseparation += 0.001;
		else
			stereoseparation += 0.010;
		printf("Stereo separation increased to %f\n", stereoseparation);
	}


	if(Input::isKeyPressed(SDLK_TAB)&&tutoriallevel){
		if(tutorialstage!=51)
			tutorialstagetime=tutorialmaxtime;
		emit_sound_np(consolefailsound, 128.);
	}

	if(!console){
		if(mainmenu&&endgame==1)mainmenu=10;
        // menu back
		if( (Input::isKeyPressed(SDLK_ESCAPE)
                    ||(mainmenu==0
                        &&((Input::isKeyPressed(jumpkey)
                                ||Input::isKeyPressed(SDLK_SPACE)
                                ||(campaign)))
                        &&campaign
                        &&winfreeze))
                && (!mainmenu||gameon||mainmenu==3||mainmenu==4||mainmenu==5||mainmenu==6||(mainmenu==7&&!entername)||mainmenu==9||mainmenu==10)){
			selected=-1;
			if(mainmenu==1||mainmenu==2||mainmenu==0){
				if(mainmenu==0&&!winfreeze)mainmenu=2;
				else if(mainmenu==0&&winfreeze&&(campaignchoosenext[campaignchoicewhich[whichchoice]])==1)mainmenu=100;
				else if(mainmenu==0&&winfreeze){ }
				else if(mainmenu==1||mainmenu==2)mainmenu=0;
				if(mainmenu&&musictoggle){
					if(mainmenu==1||mainmenu==2||mainmenu==100){
						OPENAL_SetFrequency(OPENAL_ALL, 0.001);
						emit_stream_np(stream_music3);
						pause_sound(music1);
					}
				}
				if(!mainmenu){
					pause_sound(stream_music3);
					resume_stream(music1);
				}
			}
			if(mainmenu==3){
				if(newdetail>2)newdetail=detail;
				if(newdetail<0)newdetail=detail;
				if(newscreenwidth<0)newscreenwidth=screenwidth;
				if(newscreenheight<0)newscreenheight=screenheight;
				SaveSettings(*this);
			}
			if((mainmenu>=3 && mainmenu<=7)||mainmenu==9||mainmenu==10||mainmenu==100){
				fireSound();
				flash();
			}
			if(mainmenu==3&&gameon)mainmenu=2;
			if(mainmenu==3&&!gameon)mainmenu=1;
			if(mainmenu==5&&gameon)mainmenu=2;
			if(mainmenu==5&&!gameon)mainmenu=1;
			if(mainmenu==4)mainmenu=3;
			if(mainmenu==6)mainmenu=5;
			if(mainmenu==7)mainmenu=1;
			if(mainmenu==9)mainmenu=5;
			if(mainmenu==10)mainmenu=5;
			if(mainmenu==100){
				mainmenu=5;
				gameon=0;
				winfreeze=0;
			}
		}
	}

	if(mainmenu){
        MenuTick();
	}

	if(!mainmenu){
		if(hostile==1)hostiletime+=multiplier;
		else hostiletime=0;
		if(!winfreeze)leveltime+=multiplier;

        //keys
		if(Input::isKeyDown(SDLK_ESCAPE)){
			chatting=0;
			console=0;
			freeze=0;
			displaychars[0]=0;
		}

		if(Input::isKeyPressed(chatkey)&&!console&&!chatting&&debugmode)
			chatting=1;

		if(chatting){
			inputText(displaytext[0],&displayselected,&displaychars[0]);
			if(!waiting) {
				if(displaychars[0]){
					for(int j=0;j<255;j++){
						displaytext[0][j]=' ';
					}
					displaychars[0]=0;
					displayselected=0;
				}	
				chatting=0;		
			}

			displayblinkdelay-=multiplier;
			if(displayblinkdelay<=0){
				displayblinkdelay=.3;
				displayblink=1-displayblink;
			}
		}
		if(chatting)
            keyboardfrozen=1;

		if(Input::isKeyDown(SDLK_q)&&Input::isKeyDown(SDLK_LMETA)){
			tryquit=1;
			if(mainmenu==3){
				if(newdetail>2)newdetail=detail;
				if(newdetail<0)newdetail=detail;
				if(newscreenwidth<0)newscreenwidth=screenwidth;
				if(newscreenheight<0)newscreenheight=screenheight;

				SaveSettings(*this);
			}
		}

		static int oldwinfreeze;
		if(winfreeze&&!oldwinfreeze){
			OPENAL_SetFrequency(OPENAL_ALL, 0.001);
			emit_sound_np(consolesuccesssound);
		}
		if(winfreeze==0)
            oldwinfreeze=winfreeze;
		else
            oldwinfreeze++;

		if((Input::isKeyPressed(jumpkey)||Input::isKeyPressed(SDLK_SPACE))&&!campaign)
			if(winfreeze)
                winfreeze=0;
		if((Input::isKeyDown(SDLK_ESCAPE))&&!campaign&&gameon)
			if(winfreeze){
				mainmenu=9;
				gameon=0;
			}


        //TODO: what is this test?
		if(!freeze&&!winfreeze&&!(mainmenu&&gameon)&&(gameon||!gamestarted)){

            //dialogues
			if(indialogue!=-1)
                talkdelay=1;
			talkdelay-=multiplier;

			if(talkdelay<=0&&indialogue==-1&&animation[player[0].targetanimation].height!=highheight)
                for(int i=0;i<numdialogues;i++){
                    int realdialoguetype;
                    bool special;
                    if(dialoguetype[i]>49){
                        realdialoguetype=dialoguetype[i]-50;
                        special=1;
                    }
                    else if(dialoguetype[i]>39){
                        realdialoguetype=dialoguetype[i]-40;
                        special=1;
                    }
                    else if(dialoguetype[i]>29){
                        realdialoguetype=dialoguetype[i]-30;
                        special=1;
                    }
                    else if(dialoguetype[i]>19){
                        realdialoguetype=dialoguetype[i]-20;
                        special=1;
                    }
                    else if(dialoguetype[i]>9){
                        realdialoguetype=dialoguetype[i]-10;
                        special=1;
                    }
                    else {
                        realdialoguetype=dialoguetype[i];
                        special=0;
                    }
                    if((!hostile||dialoguetype[i]>40&&dialoguetype[i]<50)&&
                            realdialoguetype<numplayers&&
                            realdialoguetype>0&&
                            (dialoguegonethrough[i]==0||!special)&&
                            (special||Input::isKeyPressed(attackkey))){
                        if(findDistancefast(&player[0].coords,&player[realdialoguetype].coords)<6||
                                player[realdialoguetype].howactive>=typedead1||
                                dialoguetype[i]>40&&dialoguetype[i]<50){
                            whichdialogue=i;
                            for(int j=0;j<numdialogueboxes[whichdialogue];j++){
                                player[participantfocus[whichdialogue][j]].coords=participantlocation[whichdialogue][participantfocus[whichdialogue][j]];
                                player[participantfocus[whichdialogue][j]].rotation=participantrotation[whichdialogue][participantfocus[whichdialogue][j]];
                                player[participantfocus[whichdialogue][j]].targetrotation=participantrotation[whichdialogue][participantfocus[whichdialogue][j]];
                                player[participantfocus[whichdialogue][j]].velocity=0;
                                player[participantfocus[whichdialogue][j]].targetanimation=player[participantfocus[whichdialogue][j]].getIdle();
                                player[participantfocus[whichdialogue][j]].targetframe=0;
                            }
                            directing=0;
                            indialogue=0;
                            dialoguetime=0;
                            dialoguegonethrough[i]++;
                            if(dialogueboxsound[whichdialogue][indialogue]!=0){
                                playdialogueboxsound();
                            }
                        }
                    }
                }

            windvar+=multiplier;
            smoketex+=multiplier;
            tutorialstagetime+=multiplier;

            //hotspots
            static float hotspotvisual[40];
            if(numhotspots){
                XYZ hotspotsprite;
                if(editorenabled)
                    for(int i=0;i<numhotspots;i++)
                        hotspotvisual[i]-=multiplier/320;

                for(int i=0;i<numhotspots;i++){
                    //if(hotspottype[i]<=10)
                    while(hotspotvisual[i]<0){
                        hotspotsprite=0;
                        hotspotsprite.x=float(abs(Random()%100000))/100000*hotspotsize[i];
                        hotspotsprite=DoRotation(hotspotsprite,0,0,Random()%360);
                        hotspotsprite=DoRotation(hotspotsprite,0,Random()%360,0);
                        hotspotsprite+=hotspot[i];
                        Sprite::MakeSprite(breathsprite, hotspotsprite, hotspotsprite*0, 1,0.5,0, 7, 0.4);
                        hotspotvisual[i]+=0.1/hotspotsize[i]/hotspotsize[i]/hotspotsize[i];
                    }
                }

                for(int i=0;i<numhotspots;i++){
                    if(hotspottype[i]<=10&&hotspottype[i]>0){
                        hotspot[i]=player[hotspottype[i]].coords;
                    }
                }
            }

            //Tutorial
            if(tutoriallevel){
                doTutorial();
            }

            //bonuses
            if(tutoriallevel!=1){
                if(bonustime==0&&
                        bonus!=solidhit&&
                        bonus!=spinecrusher&&
                        bonus!=tracheotomy&&
                        bonus!=backstab&&
                        bonusvalue>10){
                    emit_sound_np(consolesuccesssound);
                }
            } else if(bonustime==0){
                emit_sound_np(fireendsound);
            }
            if(bonustime==0){
                if(bonus!=solidhit&&
                        bonus!=twoxcombo&&
                        bonus!=threexcombo&&
                        bonus!=fourxcombo&&
                        bonus!=megacombo)
                    bonusnum[bonus]++;
                else
                    bonusnum[bonus]+=0.15;
                if(tutoriallevel)
                    bonusvalue=0;
                bonusvalue/=bonusnum[bonus];
                bonustotal+=bonusvalue;
            }
            bonustime+=multiplier;

            //snow effects
            if(environment==snowyenvironment){
                precipdelay-=multiplier;
                while(precipdelay<0){
                    precipdelay+=.04;
                    if(!detail)
                        precipdelay+=.04;
                    XYZ footvel,footpoint;

                    footvel=0;
                    footpoint=viewer+viewerfacing*6;
                    footpoint.y+=((float)abs(Random()%1200))/100-6;
                    footpoint.x+=((float)abs(Random()%1200))/100-6;
                    footpoint.z+=((float)abs(Random()%1200))/100-6;
                    Sprite::MakeSprite(snowsprite, footpoint,footvel, 1,1,1, .1, 1);
                }
            }


            doAerialAcrobatics();


            static XYZ oldviewer;

            //control keys
            if(indialogue==-1){
                player[0].forwardkeydown=Input::isKeyDown(forwardkey);
                player[0].leftkeydown=Input::isKeyDown(leftkey);
                player[0].backkeydown=Input::isKeyDown(backkey);
                player[0].rightkeydown=Input::isKeyDown(rightkey);
                player[0].jumpkeydown=Input::isKeyDown(jumpkey);
                player[0].crouchkeydown=Input::isKeyDown(crouchkey);
                player[0].drawkeydown=Input::isKeyDown(drawkey);
                player[0].throwkeydown=Input::isKeyDown(throwkey);
            }
            else
            {
                player[0].forwardkeydown=0;
                player[0].leftkeydown=0;
                player[0].backkeydown=0;
                player[0].rightkeydown=0;
                player[0].jumpkeydown=0;
                player[0].crouchkeydown=0;
                player[0].drawkeydown=0;
                player[0].throwkeydown=0;
            }

            if(!player[0].jumpkeydown)
                player[0].jumpclimb=0;


            if(indialogue!=-1){
                cameramode=1;
                if(directing){
                    facing=0;
                    facing.z=-1;

                    facing=DoRotation(facing,-rotation2,0,0);
                    facing=DoRotation(facing,0,0-rotation,0);

                    flatfacing=0;
                    flatfacing.z=-1;

                    flatfacing=DoRotation(flatfacing,0,-rotation,0);

                    if(Input::isKeyDown(forwardkey))
                        viewer+=facing*multiplier*4;
                    if(Input::isKeyDown(backkey))
                        viewer-=facing*multiplier*4;
                    if(Input::isKeyDown(leftkey))
                        viewer+=DoRotation(flatfacing*multiplier,0,90,0)*4;
                    if(Input::isKeyDown(rightkey))
                        viewer+=DoRotation(flatfacing*multiplier,0,-90,0)*4;
                    if(Input::isKeyDown(jumpkey))
                        viewer.y+=multiplier*4;
                    if(Input::isKeyDown(crouchkey))
                        viewer.y-=multiplier*4;
                    if(     Input::isKeyPressed(SDLK_1)||
                            Input::isKeyPressed(SDLK_2)||
                            Input::isKeyPressed(SDLK_3)||
                            Input::isKeyPressed(SDLK_4)||
                            Input::isKeyPressed(SDLK_5)||
                            Input::isKeyPressed(SDLK_6)||
                            Input::isKeyPressed(SDLK_7)||
                            Input::isKeyPressed(SDLK_8)||
                            Input::isKeyPressed(SDLK_9)||
                            Input::isKeyPressed(SDLK_0)||
                            Input::isKeyPressed(SDLK_MINUS)){
                        int whichend;
                        if(Input::isKeyPressed(SDLK_1))whichend=1;
                        if(Input::isKeyPressed(SDLK_2))whichend=2;
                        if(Input::isKeyPressed(SDLK_3))whichend=3;
                        if(Input::isKeyPressed(SDLK_4))whichend=4;
                        if(Input::isKeyPressed(SDLK_5))whichend=5;
                        if(Input::isKeyPressed(SDLK_6))whichend=6;
                        if(Input::isKeyPressed(SDLK_7))whichend=7;
                        if(Input::isKeyPressed(SDLK_8))whichend=8;
                        if(Input::isKeyPressed(SDLK_9))whichend=9;
                        if(Input::isKeyPressed(SDLK_0))whichend=0;
                        if(Input::isKeyPressed(SDLK_MINUS))
                            whichend=-1;
                        if(whichend!=-1){
                            participantfocus[whichdialogue][indialogue]=whichend;
                            participantlocation[whichdialogue][whichend]=player[whichend].coords;
                            participantrotation[whichdialogue][whichend]=player[whichend].rotation;
                        }
                        if(whichend==-1){
                            participantfocus[whichdialogue][indialogue]=-1;
                        }
                        if(player[participantfocus[whichdialogue][indialogue]].dead){
                            indialogue=-1;
                            directing=0;
                            cameramode=0;
                        }
                        dialoguecamera[whichdialogue][indialogue]=viewer;
                        dialoguecamerarotation[whichdialogue][indialogue]=rotation;
                        dialoguecamerarotation2[whichdialogue][indialogue]=rotation2;
                        indialogue++;
                        if(indialogue<numdialogueboxes[whichdialogue]){
                            if(dialogueboxsound[whichdialogue][indialogue]!=0){
                                playdialogueboxsound();
                            }
                        }

                        for(int j=0;j<numplayers;j++){
                            participantfacing[whichdialogue][indialogue][j]=participantfacing[whichdialogue][indialogue-1][j];
                        }
                    }
                    //TODO: should these be KeyDown or KeyPressed?
                    if(     Input::isKeyDown(SDLK_KP1)||
                            Input::isKeyDown(SDLK_KP2)||
                            Input::isKeyDown(SDLK_KP3)||
                            Input::isKeyDown(SDLK_KP4)||
                            Input::isKeyDown(SDLK_KP5)||
                            Input::isKeyDown(SDLK_KP6)||
                            Input::isKeyDown(SDLK_KP7)||
                            Input::isKeyDown(SDLK_KP8)||
                            Input::isKeyDown(SDLK_KP9)||
                            Input::isKeyDown(SDLK_KP0)){
                        int whichend;
                        if(Input::isKeyDown(SDLK_KP1))whichend=1;
                        if(Input::isKeyDown(SDLK_KP2))whichend=2;
                        if(Input::isKeyDown(SDLK_KP3))whichend=3;
                        if(Input::isKeyDown(SDLK_KP4))whichend=4;
                        if(Input::isKeyDown(SDLK_KP5))whichend=5;
                        if(Input::isKeyDown(SDLK_KP6))whichend=6;
                        if(Input::isKeyDown(SDLK_KP7))whichend=7;
                        if(Input::isKeyDown(SDLK_KP8))whichend=8;
                        if(Input::isKeyDown(SDLK_KP9))whichend=9;
                        if(Input::isKeyDown(SDLK_KP0))whichend=0;
                        participantfacing[whichdialogue][indialogue][whichend]=facing;
                    }
                    if(indialogue>=numdialogueboxes[whichdialogue]){
                        indialogue=-1;
                        directing=0;
                        cameramode=0;
                    }
                }
                if(!directing){
                    pause_sound(whooshsound);
                    viewer=dialoguecamera[whichdialogue][indialogue];
                    viewer.y=max((double)viewer.y,terrain.getHeight(viewer.x,viewer.z)+.1);
                    rotation=dialoguecamerarotation[whichdialogue][indialogue];
                    rotation2=dialoguecamerarotation2[whichdialogue][indialogue];
                    if(dialoguetime>0.5)
                        if(     Input::isKeyPressed(SDLK_1)||
                                Input::isKeyPressed(SDLK_2)||
                                Input::isKeyPressed(SDLK_3)||
                                Input::isKeyPressed(SDLK_4)||
                                Input::isKeyPressed(SDLK_5)||
                                Input::isKeyPressed(SDLK_6)||
                                Input::isKeyPressed(SDLK_7)||
                                Input::isKeyPressed(SDLK_8)||
                                Input::isKeyPressed(SDLK_9)||
                                Input::isKeyPressed(SDLK_0)||
                                Input::isKeyPressed(SDLK_MINUS)||
                                Input::isKeyPressed(attackkey)){
                            indialogue++;
                            if(indialogue<numdialogueboxes[whichdialogue]){
                                if(dialogueboxsound[whichdialogue][indialogue]!=0){
                                    playdialogueboxsound();
                                    if(dialogueboxsound[whichdialogue][indialogue]==-5){
                                        hotspot[numhotspots]=player[0].coords;
                                        hotspotsize[numhotspots]=10;
                                        hotspottype[numhotspots]=-1;

                                        numhotspots++;
                                    }
                                    if(dialogueboxsound[whichdialogue][indialogue]==-6){
                                        hostile=1;
                                    }

                                    if(player[participantfocus[whichdialogue][indialogue]].dead){
                                        indialogue=-1;
                                        directing=0;
                                        cameramode=0;
                                    }
                                }
                            }
                        }
                    if(indialogue>=numdialogueboxes[whichdialogue]){
                        indialogue=-1;
                        directing=0;
                        cameramode=0;
                        if(dialoguetype[whichdialogue]>19&&dialoguetype[whichdialogue]<30){
                            hostile=1;
                        }
                        if(dialoguetype[whichdialogue]>29&&dialoguetype[whichdialogue]<40){
                            windialogue=1;
                        }
                        if(dialoguetype[whichdialogue]>49&&dialoguetype[whichdialogue]<60){
                            hostile=1;
                            for(int i=1;i<numplayers;i++){
                                player[i].aitype = attacktypecutoff;
                            }
                        }
                    }
                }
            }

            static float keyrefreshdelay=0,bigrefreshdelay=0;

            if(!player[0].jumpkeydown){
                player[0].jumptogglekeydown=0;
            }
            if(player[0].jumpkeydown&&
                    player[0].targetanimation!=jumpupanim&&
                    player[0].targetanimation!=jumpdownanim&&
                    !player[0].isFlip())
                player[0].jumptogglekeydown=1;


            dialoguetime+=multiplier;
            hawkrotation+=multiplier*25;
            realhawkcoords=0;
            realhawkcoords.x=25;
            realhawkcoords=DoRotation(realhawkcoords,0,hawkrotation,0)+hawkcoords;
            hawkcalldelay-=multiplier/2;

            if(hawkcalldelay<=0){
                emit_sound_at(hawksound, realhawkcoords);

                hawkcalldelay=16+abs(Random()%8);
            }
            static float temptexdetail;


            doDebugKeys();

            doAttacks();

            doPlayerCollisions();

            doJumpReversals();

            for(int k=0;k<numplayers;k++)
                if(k!=0&&player[k].immobile)
                    player[k].coords=player[k].realoldcoords;

            for(int k=0;k<numplayers;k++){
                if(!isnormal(player[k].coords.x)||!isnormal(player[k].coords.y)||!isnormal(player[k].coords.z)){
                    if(!isnormal(player[k].coords.x)||!isnormal(player[k].coords.y)||!isnormal(player[k].coords.z)){
                        player[k].DoDamage(1000);
                    }
                }
            }

            //respawn
            static bool respawnkeydown;
            if(!editorenabled&&
                    (whichlevel!=-2&&
                     (Input::isKeyDown(SDLK_z)&&
                      Input::isKeyDown(SDLK_LMETA)&&
                      debugmode)||
                     (Input::isKeyDown(jumpkey)&&
                      !respawnkeydown&&
                      !oldattackkey&&
                      player[0].dead))){
                targetlevel=whichlevel;
                loading=1;
                leveltime=5;
            }
            if(!Input::isKeyDown(jumpkey))
                respawnkeydown=0;
            if(Input::isKeyDown(jumpkey))
                respawnkeydown=1;




            static bool movekey;

            //?
            for(int i=0;i<numplayers;i++){
                static float oldtargetrotation;
                if(!player[i].skeleton.free){
                    oldtargetrotation=player[i].targetrotation;
                    if(i==0&&indialogue==-1){
                        //TODO: refactor repetitive code
                        if(!animation[player[0].targetanimation].attack&&
                                player[0].targetanimation!=staggerbackhighanim&&
                                player[0].targetanimation!=staggerbackhardanim&&
                                player[0].targetanimation!=crouchremoveknifeanim&&
                                player[0].targetanimation!=removeknifeanim&&
                                player[0].targetanimation!=backhandspringanim&&
                                player[0].targetanimation!=dodgebackanim&&
                                player[0].targetanimation!=walljumprightkickanim&&
                                player[0].targetanimation!=walljumpleftkickanim){
                            if(cameramode)
                                player[0].targetrotation=0;
                            else
                                player[0].targetrotation=-rotation+180;
                        }

                        facing=0;
                        facing.z=-1;

                        flatfacing=DoRotation(facing,0,player[i].rotation+180,0);
                        if(cameramode){
                            facing=flatfacing;
                        }else{
                            facing=DoRotation(facing,-rotation2,0,0);
                            facing=DoRotation(facing,0,0-rotation,0);
                        }

                        player[0].lookrotation=-rotation;

                        player[i].targetheadrotation=rotation;
                        player[i].targetheadrotation2=rotation2;
                    }
                    if(i!=0&&player[i].aitype==playercontrolled&&indialogue==-1){
                        if(!animation[player[i].targetanimation].attack&&
                                player[i].targetanimation!=staggerbackhighanim&&
                                player[i].targetanimation!=staggerbackhardanim&&
                                player[i].targetanimation!=crouchremoveknifeanim&&
                                player[i].targetanimation!=removeknifeanim&&
                                player[i].targetanimation!=backhandspringanim&&
                                player[i].targetanimation!=dodgebackanim&&
                                player[i].targetanimation!=walljumprightkickanim&&
                                player[i].targetanimation!=walljumpleftkickanim){
                            player[i].targetrotation=-player[i].lookrotation+180;
                        }

                        facing=0;
                        facing.z=-1;

                        flatfacing=DoRotation(facing,0,player[i].rotation+180,0);

                        facing=DoRotation(facing,-player[i].lookrotation2,0,0);
                        facing=DoRotation(facing,0,0-player[i].lookrotation,0);

                        player[i].targetheadrotation=player[i].lookrotation;
                        player[i].targetheadrotation2=player[i].lookrotation2;
                    }
                    if(indialogue!=-1){
                        player[i].targetheadrotation=180-roughDirection(participantfacing[whichdialogue][indialogue][i]);
                        player[i].targetheadrotation2=pitch(participantfacing[whichdialogue][indialogue][i]);
                    }

                    bool pause;

                    if(leveltime<.5)
                        numenvsounds=0;

                    player[i].avoidsomething=0;

                    //avoid flaming things
                    for(int j=0;j<objects.numobjects;j++)
                        if(objects.onfire[j])
                            if(findDistancefast(&player[i].coords,&objects.position[j])<sq(objects.scale[j])*200)
                                if(     findDistancefast(&player[i].coords,&objects.position[j])<
                                        findDistancefast(&player[i].coords,&player[0].coords)){
                                    player[i].collided=0;
                                    player[i].avoidcollided=1;
                                    if(player[i].avoidsomething==0||
                                            findDistancefast(&player[i].coords,&objects.position[j])<
                                            findDistancefast(&player[i].coords,&player[i].avoidwhere)){
                                        player[i].avoidwhere=objects.position[j];
                                        player[i].avoidsomething=1;
                                    }
                                }

                    //avoid flaming players
                    for(int j=0;j<numplayers;j++)
                        if(player[j].onfire)
                            if(findDistancefast(&player[j].coords,&player[i].coords)<sq(0.3)*200)
                                if(     findDistancefast(&player[i].coords,&player[j].coords)<
                                        findDistancefast(&player[i].coords,&player[0].coords)){
                                    player[i].collided=0;
                                    player[i].avoidcollided=1;
                                    if(player[i].avoidsomething==0||
                                            findDistancefast(&player[i].coords,&player[j].coords)<
                                            findDistancefast(&player[i].coords,&player[i].avoidwhere)){
                                        player[i].avoidwhere=player[j].coords;
                                        player[i].avoidsomething=1;
                                    }
                                }

                    if(player[i].collided>.8)
                        player[i].avoidcollided=0;

                    doAI(i);

                    if(animation[player[i].targetanimation].attack==reversed){
                        //player[i].targetrotation=player[i].rotation;
                        player[i].forwardkeydown=0;
                        player[i].leftkeydown=0;
                        player[i].backkeydown=0;
                        player[i].rightkeydown=0;
                        player[i].jumpkeydown=0;
                        player[i].attackkeydown=0;
                        //player[i].crouchkeydown=0;
                        player[i].throwkeydown=0;
                    }

                    if(indialogue!=-1){
                        player[i].forwardkeydown=0;
                        player[i].leftkeydown=0;
                        player[i].backkeydown=0;
                        player[i].rightkeydown=0;
                        player[i].jumpkeydown=0;
                        player[i].crouchkeydown=0;
                        player[i].drawkeydown=0;
                        player[i].throwkeydown=0;
                    }

                    if(player[i].collided<-.3)
                        player[i].collided=-.3;
                    if(player[i].collided>1)
                        player[i].collided=1;
                    player[i].collided-=multiplier*4;
                    player[i].whichdirectiondelay-=multiplier;
                    if(player[i].avoidcollided<-.3||player[i].whichdirectiondelay<=0){
                        player[i].avoidcollided=-.3;
                        player[i].whichdirection=abs(Random()%2);
                        player[i].whichdirectiondelay=.4;
                    }
                    if(player[i].avoidcollided>1)
                        player[i].avoidcollided=1;
                    player[i].avoidcollided-=multiplier/4;
                    if(!player[i].skeleton.free){
                        player[i].stunned-=multiplier;
                        player[i].surprised-=multiplier;
                    }
                    if(i!=0&&player[i].surprised<=0&&
                            player[i].aitype==attacktypecutoff&&
                            !player[i].dead&&
                            !player[i].skeleton.free&&
                            animation[player[i].targetanimation].attack==neutral)
                        numresponded=1;

                    if(!player[i].throwkeydown)
                        player[i].throwtogglekeydown=0;

                    //pick up weapon
                    if(player[i].throwkeydown&&!player[i].throwtogglekeydown){
                        if(player[i].weaponactive==-1&&
                                player[i].num_weapons<2&&
                                (player[i].isIdle()||
                                 player[i].isCrouch()||
                                 player[i].targetanimation==sneakanim||
                                 player[i].targetanimation==rollanim||
                                 player[i].targetanimation==backhandspringanim||
                                 player[i].isFlip()||
                                 player[i].isFlip()||
                                 player[i].aitype!=playercontrolled)){
                            for(int j=0;j<weapons.numweapons;j++){
                                if((weapons.velocity[j].x==0&&weapons.velocity[j].y==0&&weapons.velocity[j].z==0||
                                            player[i].aitype==playercontrolled)&&
                                        weapons.owner[j]==-1&&
                                        player[i].weaponactive==-1)
                                    if(findDistancefastflat(&player[i].coords,&weapons.position[j])<2){
                                        if(findDistancefast(&player[i].coords,&weapons.position[j])<2){
                                            if(player[i].isCrouch()||
                                                    player[i].targetanimation==sneakanim||
                                                    player[i].isRun()||
                                                    player[i].isIdle()||
                                                    player[i].aitype!=playercontrolled){
                                                player[i].throwtogglekeydown=1;
                                                setAnimation(i,crouchremoveknifeanim);
                                                player[i].targetrotation=roughDirectionTo(player[i].coords,weapons.position[j]);
                                                player[i].hasvictim=0;
                                            }
                                            if(player[i].targetanimation==rollanim||player[i].targetanimation==backhandspringanim){
                                                player[i].throwtogglekeydown=1;
                                                player[i].hasvictim=0;

                                                if((weapons.velocity[j].x==0&&weapons.velocity[j].y==0&&weapons.velocity[j].z==0||
                                                                player[i].aitype==playercontrolled)&&
                                                            weapons.owner[j]==-1||
                                                        player[i].victim&&
                                                        weapons.owner[j]==player[i].victim->id)
                                                    if(findDistancefastflat(&player[i].coords,&weapons.position[j])<2&&player[i].weaponactive==-1)
                                                        if(findDistancefast(&player[i].coords,&weapons.position[j])<1||player[i].victim){
                                                            if(weapons.type[j]!=staff)
                                                                emit_sound_at(knifedrawsound, player[i].coords, 128.);

                                                            player[i].weaponactive=0;
                                                            weapons.owner[j]=player[i].id;
                                                            if(player[i].num_weapons>0)
                                                                player[i].weaponids[player[i].num_weapons]=player[i].weaponids[0];
                                                            player[i].num_weapons++;
                                                            player[i].weaponids[0]=j;
                                                        }
                                            }
                                        }else if((player[i].isIdle()||
                                                    player[i].isFlip()||
                                                    player[i].aitype!=playercontrolled)&&
                                                findDistancefast(&player[i].coords,&weapons.position[j])<5&&
                                                player[i].coords.y<weapons.position[j].y){
                                            if(!player[i].isFlip()){
                                                player[i].throwtogglekeydown=1;
                                                setAnimation(i,removeknifeanim);
                                                player[i].targetrotation=roughDirectionTo(player[i].coords,weapons.position[j]);
                                            }
                                            if(player[i].isFlip()){
                                                player[i].throwtogglekeydown=1;
                                                player[i].hasvictim=0;

                                                for(int k=0;k<weapons.numweapons;k++){
                                                    if(player[i].weaponactive==-1)
                                                        if((weapons.velocity[k].x==0&&weapons.velocity[k].y==0&&weapons.velocity[k].z==0||
                                                                        player[i].aitype==playercontrolled)&&
                                                                    weapons.owner[k]==-1||
                                                                player[i].victim&&
                                                                 weapons.owner[k]==player[i].victim->id)
                                                            if(findDistancefastflat(&player[i].coords,&weapons.position[k])<3&&
                                                                    player[i].weaponactive==-1){
                                                                if(weapons.type[k]!=staff)
                                                                    emit_sound_at(knifedrawsound, player[i].coords, 128.);

                                                                player[i].weaponactive=0;
                                                                weapons.owner[k]=player[i].id;
                                                                if(player[i].num_weapons>0)
                                                                    player[i].weaponids[player[i].num_weapons]=player[i].weaponids[0];
                                                                player[i].num_weapons++;
                                                                player[i].weaponids[0]=k;
                                                            }
                                                }
                                            }
                                        }
                                    }
                            }
                            if(player[i].isCrouch()||
                                    player[i].targetanimation==sneakanim||
                                    player[i].isRun()||
                                    player[i].isIdle()||player[i].targetanimation==rollanim||
                                    player[i].targetanimation==backhandspringanim){
                                if(numplayers>1)
                                    for(int j=0;j<numplayers;j++){
                                        if(player[i].weaponactive==-1)
                                            if(j!=i)
                                                if(player[j].num_weapons&&
                                                        player[j].skeleton.free&&
                                                        findDistancefast(&player[i].coords,&player[j].coords)<2/*&&player[j].dead*/&&
                                                        (((player[j].skeleton.forward.y<0&&
                                                           player[j].weaponstuckwhere==0)||
                                                          (player[j].skeleton.forward.y>0&&
                                                           player[j].weaponstuckwhere==1))||
                                                         player[j].weaponstuck==-1||
                                                         player[j].num_weapons>1)){
                                                    if(player[i].targetanimation!=rollanim&&player[i].targetanimation!=backhandspringanim){
                                                        player[i].throwtogglekeydown=1;
                                                        player[i].victim=&player[j];
                                                        player[i].hasvictim=1;
                                                        setAnimation(i,crouchremoveknifeanim);
                                                        player[i].targetrotation=roughDirectionTo(player[i].coords,player[j].coords);
                                                    }
                                                    if(player[i].targetanimation==rollanim||player[i].targetanimation==backhandspringanim){
                                                        player[i].throwtogglekeydown=1;
                                                        player[i].victim=&player[j];
                                                        player[i].hasvictim=1;
                                                        int k = player[j].weaponids[0];
                                                        if(player[i].hasvictim){
                                                            bool fleshstuck;
                                                            fleshstuck=0;
                                                            if(player[i].victim->weaponstuck!=-1){
                                                                if(player[i].victim->weaponids[player[i].victim->weaponstuck]==k){
                                                                    fleshstuck=1;
                                                                }
                                                            }
                                                            if(!fleshstuck){
                                                                if(weapons.type[k]!=staff)
                                                                  emit_sound_at(knifedrawsound, player[i].coords, 128.);
                                                            }
                                                            if(fleshstuck)
                                                              emit_sound_at(fleshstabremovesound, player[i].coords, 128.);

                                                            player[i].weaponactive=0;
                                                            if(weapons.owner[k]!=-1){
                                                                if(player[i].victim->num_weapons==1)player[i].victim->num_weapons=0;
                                                                else player[i].victim->num_weapons=1;

                                                                player[i].victim->skeleton.longdead=0;
                                                                player[i].victim->skeleton.free=1;
                                                                player[i].victim->skeleton.broken=0;

                                                                for(int l=0;l<player[i].victim->skeleton.num_joints;l++){
                                                                    player[i].victim->skeleton.joints[l].velchange=0;
                                                                    player[i].victim->skeleton.joints[l].locked=0;
                                                                }

                                                                XYZ relative;
                                                                relative=0;
                                                                relative.y=10;
                                                                Normalise(&relative);
                                                                XYZ footvel,footpoint;
                                                                footvel=0;
                                                                footpoint=weapons.position[k];
                                                                if(player[i].victim->weaponstuck!=-1){
                                                                    if(player[i].victim->weaponids[player[i].victim->weaponstuck]==k){
                                                                        if(bloodtoggle)Sprite::MakeSprite(cloudimpactsprite, footpoint,footvel, 1,0,0, .8, .3);
                                                                        weapons.bloody[k]=2;
                                                                        weapons.blooddrip[k]=5;
                                                                        player[i].victim->weaponstuck=-1;
                                                                        player[i].victim->bloodloss+=2000;
                                                                        player[i].victim->DoDamage(2000);
                                                                    }
                                                                }
                                                                if(player[i].victim->num_weapons>0){
                                                                    if(player[i].victim->weaponstuck!=0&&player[i].victim->weaponstuck!=-1)player[i].victim->weaponstuck=0;
                                                                    if(player[i].victim->weaponids[0]==k)
                                                                        player[i].victim->weaponids[0]=player[i].victim->weaponids[player[i].victim->num_weapons];
                                                                }

                                                                player[i].victim->weaponactive=-1;

                                                                playerJoint(player[i].victim,abdomen).velocity+=relative*6;
                                                                playerJoint(player[i].victim,neck).velocity+=relative*6;
                                                                playerJoint(player[i].victim,rightshoulder).velocity+=relative*6;
                                                                playerJoint(player[i].victim,leftshoulder).velocity+=relative*6;
                                                            }
                                                            weapons.owner[k]=i;
                                                            if(player[i].num_weapons>0){
                                                                player[i].weaponids[player[i].num_weapons]=player[i].weaponids[0];
                                                            }
                                                            player[i].num_weapons++;
                                                            player[i].weaponids[0]=k;
                                                        }
                                                    }
                                                }
                                    }
                            }
                        }
                        if(player[i].weaponactive!=-1&&player[i].aitype==playercontrolled){
                            if(weapons.type[player[i].weaponids[0]]==knife){
                                if(player[i].isIdle()||
                                        player[i].isRun()||
                                        player[i].isCrouch()||
                                        player[i].targetanimation==sneakanim||
                                        player[i].isFlip())
                                    if(numplayers>1)
                                        for(int j=0;j<numplayers;j++){
                                            if(i!=j)
                                                if(tutoriallevel!=1||tutorialstage==49)
                                                    if(hostile)
                                                        if(normaldotproduct(player[i].facing,player[i].coords-player[j].coords)<0&&
                                                                findDistancefast(&player[i].coords,&player[j].coords)<100&&
                                                                findDistancefast(&player[i].coords,&player[j].coords)>1.5&&
                                                                !player[j].skeleton.free&&
                                                                -1==checkcollide(DoRotation(playerJoint(j,head).position,0,player[j].rotation,0)*player[j].scale+player[j].coords,DoRotation(playerJoint(i,head).position,0,player[i].rotation,0)*player[i].scale+player[i].coords)){
                                                            if(!player[i].isFlip()){
                                                                player[i].throwtogglekeydown=1;
                                                                player[i].victim=&player[j];
                                                                setAnimation(i,knifethrowanim);
                                                                player[i].targetrotation=roughDirectionTo(player[i].coords,player[j].coords);
                                                                player[i].targettilt2=pitchTo(player[i].coords,player[j].coords);
                                                            }
                                                            if(player[i].isFlip()){
                                                                if(player[i].weaponactive!=-1){
                                                                    player[i].throwtogglekeydown=1;
                                                                    player[i].victim=&player[j];
                                                                    XYZ aim;
                                                                    weapons.owner[player[i].weaponids[0]]=-1;
                                                                    aim=player[i].victim->coords+DoRotation(playerJoint(player[i].victim,abdomen).position,0,player[i].victim->rotation,0)*player[i].victim->scale+player[i].victim->velocity*findDistance(&player[i].victim->coords,&player[i].coords)/50-(player[i].coords+DoRotation(playerJoint(i,righthand).position,0,player[i].rotation,0)*player[i].scale);
                                                                    Normalise(&aim);

                                                                    aim=DoRotation(aim,(float)abs(Random()%30)-15,(float)abs(Random()%30)-15,0);

                                                                    weapons.velocity[player[i].weaponids[0]]=aim*50;
                                                                    weapons.tipvelocity[player[i].weaponids[0]]=aim*50;
                                                                    weapons.missed[player[i].weaponids[0]]=0;
                                                                    weapons.freetime[player[i].weaponids[0]]=0;
                                                                    weapons.firstfree[player[i].weaponids[0]]=1;
                                                                    weapons.physics[player[i].weaponids[0]]=0;
                                                                    player[i].num_weapons--;
                                                                    if(player[i].num_weapons){
                                                                        player[i].weaponids[0]=player[i].weaponids[player[i].num_weapons];
                                                                    }
                                                                    player[i].weaponactive=-1;
                                                                }
                                                            }
                                                        }
                                        }
                            }
                        }
                        if(player[i].weaponactive!=-1&&player[i].aitype==playercontrolled){
                            if(player[i].isCrouch()||player[i].targetanimation==sneakanim){
                                player[i].throwtogglekeydown=1;
                                weapons.owner[player[i].weaponids[0]]=-1;
                                weapons.velocity[player[i].weaponids[0]]=player[i].velocity*.2;
                                if(weapons.velocity[player[i].weaponids[0]].x==0)weapons.velocity[player[i].weaponids[0]].x=.1;
                                weapons.tipvelocity[player[i].weaponids[0]]=weapons.velocity[player[i].weaponids[0]];
                                weapons.missed[player[i].weaponids[0]]=1;
                                weapons.freetime[player[i].weaponids[0]]=0;
                                weapons.firstfree[player[i].weaponids[0]]=1;
                                weapons.physics[player[i].weaponids[0]]=1;
                                player[i].num_weapons--;
                                if(player[i].num_weapons){
                                    player[i].weaponids[0]=player[i].weaponids[player[i].num_weapons];
                                    if(player[i].weaponstuck==player[i].num_weapons)player[i].weaponstuck=0;
                                }

                                player[i].weaponactive=-1;
                                for(int j=0;j<numplayers;j++){
                                    player[j].wentforweapon=0;
                                }
                            }
                        }

                    }

                    //draw weapon
                    if(i==0||!player[0].dead||player[i].weaponactive!=-1)
                        if(player[i].drawkeydown&&!player[i].drawtogglekeydown||
                                player[i].num_weapons==2&&
                                player[i].weaponactive==-1&&
                                player[i].isIdle()||
                                player[0].dead&&
                                player[i].weaponactive!=-1&&
                                i!=0){
                            bool isgood=1;
                            if(player[i].weaponactive!=-1)
                                if(weapons.type[player[i].weaponids[player[i].weaponactive]]==staff)
                                    isgood=0;
                            if(isgood&&player[i].creature!=wolftype){
                                if(player[i].isIdle()&&player[i].num_weapons&&weapons.type[player[i].weaponids[0]]==knife){
                                    setAnimation(i,drawrightanim);
                                    player[i].drawtogglekeydown=1;
                                }
                                if((player[i].isIdle()||
                                            (player[i].aitype!=playercontrolled&&
                                             player[0].weaponactive!=-1&&
                                             player[i].isRun()))&&
                                        player[i].num_weapons&&
                                        weapons.type[player[i].weaponids[0]]==sword){
                                    setAnimation(i,drawleftanim);
                                    player[i].drawtogglekeydown=1;
                                }
                                if(player[i].isCrouch()&&player[i].num_weapons&&weapons.type[player[i].weaponids[0]]==knife){
                                    setAnimation(i,crouchdrawrightanim);
                                    player[i].drawtogglekeydown=1;
                                }
                            }
                        }
                    //clean weapon
                    if(player[i].isCrouch()&&
                            weapons.bloody[player[i].weaponids[player[i].weaponactive]]&&
                            bloodtoggle&&
                            player[i].onterrain&&
                            player[i].num_weapons&&
                            player[i].weaponactive!=-1&&
                            player[i].attackkeydown){
                        if(weapons.bloody[player[i].weaponids[player[i].weaponactive]]&&
                                player[i].onterrain&&
                                bloodtoggle&&musictype!=stream_music2){
                            if(weapons.type[player[i].weaponids[player[i].weaponactive]]==knife)
                                setAnimation(i,crouchstabanim);
                            if(weapons.type[player[i].weaponids[player[i].weaponactive]]==sword)
                                setAnimation(i,swordgroundstabanim);
                            player[i].hasvictim=0;
                            //player[i].attacktogglekeydown=1;
                        }
                    }

                    if(!player[i].drawkeydown)
                        player[i].drawtogglekeydown=0;

                    XYZ absflatfacing;
                    if(i==0){
                        absflatfacing=0;
                        absflatfacing.z=-1;

                        absflatfacing=DoRotation(absflatfacing,0,-rotation,0);
                    }
                    else absflatfacing=flatfacing;

                    if(indialogue!=-1){
                        player[i].forwardkeydown=0;
                        player[i].leftkeydown=0;
                        player[i].backkeydown=0;
                        player[i].rightkeydown=0;
                        player[i].jumpkeydown=0;
                        player[i].crouchkeydown=0;
                        player[i].drawkeydown=0;
                        player[i].throwkeydown=0;
                    }
                    movekey=0;
                    //Do controls
                    if(!animation[player[i].targetanimation].attack&&
                            player[i].targetanimation!=staggerbackhighanim&&
                            player[i].targetanimation!=staggerbackhardanim&&
                            player[i].targetanimation!=backhandspringanim&&
                            player[i].targetanimation!=dodgebackanim){
                        if(!player[i].forwardkeydown)
                            player[i].forwardstogglekeydown=0;
                        if(player[i].crouchkeydown){
                            //Crouch
                            target=-2;
                            if(i==0){
                                player[i].superruntoggle=1;
                                if(numplayers>1)
                                    for(int j=0;j<numplayers;j++)
                                        if(j!=i&&!player[j].skeleton.free&&player[j].aitype==passivetype)
                                            if(findDistancefast(&player[j].coords,&player[i].coords)<16)
                                                player[i].superruntoggle=0;
                            }

                            if(numplayers>1)
                                for(int j=0;j<numplayers;j++){
                                    if(j!=i&&!player[j].skeleton.free&&player[j].victim&&player[i].lowreversaldelay<=0){
                                        if(findDistancefast(&player[j].coords,&player[j].victim->coords)<3&&
                                                player[j].victim==&player[i]&&
                                                (player[j].targetanimation==sweepanim||
                                                 player[j].targetanimation==upunchanim||
                                                 player[j].targetanimation==wolfslapanim||
                                                 ((player[j].targetanimation==swordslashanim||
                                                   player[j].targetanimation==knifeslashstartanim||
                                                   player[j].targetanimation==staffhitanim||
                                                   player[j].targetanimation==staffspinhitanim)&&
                                                  findDistancefast(&player[j].coords,&player[i].coords)<2))){
                                            if(target>=0)
                                                target=-1;
                                            else
                                                target=j;
                                        }
                                    }
                                }
                            if(target>=0)
                                player[target].Reverse();
                            player[i].lowreversaldelay=.5;

                            if(player[i].isIdle()){
                                setAnimation(i,player[i].getCrouch());
                                player[i].transspeed=10;
                            }
                            if(player[i].isRun()||
                                    (player[i].isStop()&&
                                     (player[i].leftkeydown||
                                      player[i].rightkeydown||
                                      player[i].forwardkeydown||
                                      player[i].backkeydown))){
                                setAnimation(i,rollanim);
                                player[i].transspeed=20;
                            }
                        }
                        if(!player[i].crouchkeydown){
                            //Uncrouch
                            if(!player[i].isRun()&&player[i].targetanimation!=sneakanim&&i==0)player[i].superruntoggle=0;
                            target=-2;
                            if(player[i].isCrouch()){
                                if(numplayers>1)
                                    for(int j=0;j<numplayers;j++){
                                        if(j!=i&&
                                                !player[j].skeleton.free&&
                                                player[j].victim&&
                                                player[i].highreversaldelay<=0){
                                            if(findDistancefast(&player[j].coords,&player[j].victim->coords)<3&&
                                                    player[j].victim==&player[i]&&
                                                    (player[j].targetanimation==spinkickanim)&&
                                                    player[i].isCrouch()){
                                                if(target>=0)
                                                    target=-1;
                                                else
                                                    target=j;
                                            }
                                        }
                                    }
                                if(target>=0)
                                    player[target].Reverse();
                                player[i].highreversaldelay=.5;

                                if(player[i].isCrouch()){
                                    if(!player[i].wasCrouch()){
                                        player[i].currentanimation=player[i].getCrouch();
                                        player[i].currentframe=0;
                                    }
                                    setAnimation(i,player[i].getIdle());
                                    player[i].transspeed=10;
                                }
                            }
                            if(player[i].targetanimation==sneakanim){
                                setAnimation(i,player[i].getIdle());
                                player[i].transspeed=10;
                            }
                        }
                        if(player[i].forwardkeydown){
                            if(player[i].isIdle()||
                                    (player[i].isStop()&&
                                     player[i].targetrotation==player[i].rotation)||
                                    (player[i].isLanding()&&
                                     player[i].targetframe>0&&
                                     !player[i].jumpkeydown)||
                                    (player[i].isLandhard()&&
                                     player[i].targetframe>0&&
                                     !player[i].jumpkeydown&&
                                     player[i].crouchkeydown)){
                                if(player[i].aitype==passivetype)
                                    setAnimation(i,walkanim);
                                else
                                    setAnimation(i,player[i].getRun());
                            }
                            if(player[i].isCrouch()){
                                player[i].targetanimation=sneakanim;
                                if(player[i].wasCrouch())
                                    player[i].target=0;
                                player[i].targetframe=0;
                            }
                            if(player[i].targetanimation==hanganim/*&&(!player[i].forwardstogglekeydown||player[i].aitype!=playercontrolled)*/){
                                setAnimation(i,climbanim);
                                player[i].targetframe=1;
                                player[i].jumpclimb=1;
                            }
                            if(player[i].targetanimation==jumpupanim||player[i].targetanimation==jumpdownanim||player[i].isFlip()){
                                player[i].velocity+=absflatfacing*5*multiplier;
                            }
                            player[i].forwardstogglekeydown=1;
                            movekey=1;
                        }
                        if (player[i].rightkeydown){
                            if(player[i].isIdle()||
                                    (player[i].isStop()&&
                                     player[i].targetrotation==player[i].rotation)||
                                    (player[i].isLanding()&&
                                     player[i].targetframe>0&&
                                     !player[i].jumpkeydown)||
                                    (player[i].isLandhard()&&
                                     player[i].targetframe>0&&
                                     !player[i].jumpkeydown&&
                                     player[i].crouchkeydown)){
                                setAnimation(i,player[i].getRun());
                            }
                            if(player[i].isCrouch()){
                                player[i].targetanimation=sneakanim;
                                if(player[i].wasCrouch())
                                    player[i].target=0;
                                player[i].targetframe=0;
                            }
                            if(player[i].targetanimation==jumpupanim||player[i].targetanimation==jumpdownanim||player[i].isFlip()){
                                player[i].velocity+=DoRotation(absflatfacing*5*multiplier,0,-90,0);
                            }
                            player[i].targetrotation-=90;
                            if(player[i].forwardkeydown)player[i].targetrotation+=45;
                            if(player[i].backkeydown)player[i].targetrotation-=45;
                            movekey=1;
                        }
                        if ( player[i].leftkeydown){
                            if(player[i].isIdle()||
                                    (player[i].isStop()&&
                                     player[i].targetrotation==player[i].rotation)||
                                    (player[i].isLanding()&&
                                     player[i].targetframe>0&&
                                     !player[i].jumpkeydown)||
                                    (player[i].isLandhard()&&
                                     player[i].targetframe>0&&
                                     !player[i].jumpkeydown&&
                                     player[i].crouchkeydown)){
                                setAnimation(i,player[i].getRun());
                            }
                            if(player[i].isCrouch()){
                                player[i].targetanimation=sneakanim;
                                if(player[i].wasCrouch())
                                    player[i].target=0;
                                player[i].targetframe=0;
                            }
                            if(player[i].targetanimation==jumpupanim||player[i].targetanimation==jumpdownanim||player[i].isFlip()){
                                player[i].velocity-=DoRotation(absflatfacing*5*multiplier,0,-90,0);
                            }
                            player[i].targetrotation+=90;
                            if(player[i].forwardkeydown)player[i].targetrotation-=45;
                            if(player[i].backkeydown)player[i].targetrotation+=45;
                            movekey=1;
                        }
                        if(player[i].backkeydown){
                            if(player[i].isIdle()||
                                    (player[i].isStop()&&
                                     player[i].targetrotation==player[i].rotation)||
                                    (player[i].isLanding()&&
                                     player[i].targetframe>0&&
                                     !player[i].jumpkeydown)||
                                    (player[i].isLandhard()&&
                                     player[i].targetframe>0&&
                                     !player[i].jumpkeydown&&
                                     player[i].crouchkeydown)){
                                setAnimation(i,player[i].getRun());
                            }
                            if(player[i].isCrouch()){
                                player[i].targetanimation=sneakanim;
                                if(player[i].wasCrouch())
                                    player[i].target=0;
                                player[i].targetframe=0;
                            }
                            if(player[i].targetanimation==jumpupanim||player[i].targetanimation==jumpdownanim||player[i].isFlip()){
                                player[i].velocity-=absflatfacing*5*multiplier;
                            }
                            if(player[i].targetanimation==hanganim){
                                player[i].currentanimation=jumpdownanim;
                                player[i].targetanimation=jumpdownanim;
                                player[i].target=0;
                                player[i].currentframe=0;
                                player[i].targetframe=1;
                                player[i].velocity=0;
                                player[i].velocity.y+=gravity;
                                player[i].coords.y-=1.4;
                                player[i].grabdelay=1;
                            }
                            if ( !player[i].leftkeydown&&!player[i].rightkeydown)
                                player[i].targetrotation+=180;
                            movekey=1;
                        }
                        if((player[i].jumpkeydown&&!player[i].jumpclimb)||player[i].jumpstart){
                            if((((player[i].isLanding()&&player[i].targetframe>=3)||
                                            player[i].isRun()||
                                            player[i].targetanimation==walkanim||
                                            player[i].isCrouch()||
                                            player[i].targetanimation==sneakanim)&&
                                        player[i].jumppower>1)&&
                                    ((player[i].targetanimation!=rabbitrunninganim&&
                                      player[i].targetanimation!=wolfrunninganim)||i!=0)){
                                player[i].jumpstart=0;
                                setAnimation(i,jumpupanim);
                                player[i].rotation=player[i].targetrotation;
                                player[i].transspeed=20;
                                player[i].FootLand(0,1);
                                player[i].FootLand(1,1);

                                facing=0;
                                facing.z=-1;
                                flatfacing=DoRotation(facing,0,player[i].targetrotation+180,0);

                                if(movekey)player[i].velocity=flatfacing*player[i].speed*45*player[i].scale;
                                if(!movekey)player[i].velocity=0;

                                //Dodge sweep?
                                target=-2;
                                if(numplayers>1)
                                    for(int j=0;j<numplayers;j++){
                                        if(j!=i&&!player[j].skeleton.free&&player[j].victim){
                                            if(findDistancefast(&player[j].coords,&player[j].victim->coords)<3&&
                                                    player[j].victim==&player[i]&&
                                                    (player[j].targetanimation==sweepanim)){
                                                if(target>=0)target=-1;
                                                else target=j;
                                            }
                                        }
                                    }
                                    if(target>=0)player[i].velocity.y=1;
                                    else if(player[i].crouchkeydown||player[i].aitype!=playercontrolled){
                                        player[i].velocity.y=7;
                                        player[i].crouchtogglekeydown=1;
                                    }
                                    else player[i].velocity.y=5;

                                    if(mousejump&&i==0&&debugmode){
                                        if(!player[i].isLanding())player[i].tempdeltav=deltav;
                                        if(player[i].tempdeltav<0)player[i].velocity.y-=(float)(player[i].tempdeltav)/multiplier/1000;
                                    }

                                    player[i].coords.y+=.2;
                                    player[i].jumppower-=1;

                                    if (!i)
                                      emit_sound_at(whooshsound, player[i].coords, 128.);

                                    emit_sound_at(jumpsound, player[i].coords, 128.);
                            }
                            if((player[i].isIdle())&&player[i].jumppower>1){
                                setAnimation(i,player[i].getLanding());
                                player[i].targetframe=2;
                                player[i].landhard=0;
                                player[i].jumpstart=1;
                                player[i].tempdeltav=deltav;
                            }
                            if(player[i].targetanimation==jumpupanim&&
                                    (((!floatjump&&
                                       !editorenabled)||
                                      !debugmode)||
                                     player[i].aitype!=playercontrolled)){
                                if(player[i].jumppower>multiplier*6){
                                    player[i].velocity.y+=multiplier*6;
                                    player[i].jumppower-=multiplier*6;
                                }
                                if(player[i].jumppower<=multiplier*6){
                                    player[i].velocity.y+=player[i].jumppower;
                                    player[i].jumppower=0;
                                }
                            }
                            if(((floatjump||editorenabled)&&debugmode)&&i==0)player[i].velocity.y+=multiplier*30;
                        }

                        if(!movekey){
                            if(player[i].isRun()||player[i].targetanimation==walkanim)
                                setAnimation(i,player[i].getStop());
                            if(player[i].targetanimation==sneakanim){
                                player[i].targetanimation=player[i].getCrouch();
                                if(player[i].currentanimation==sneakanim)
                                    player[i].target=0;
                                player[i].targetframe=0;
                            }
                        }
                        if(player[i].targetanimation==walkanim&&
                                (player[i].aitype==attacktypecutoff||
                                 player[i].aitype==searchtype||
                                 (player[i].aitype==passivetype&&
                                  player[i].numwaypoints<=1)))
                            setAnimation(i,player[i].getStop());
                        if(player[i].isRun()&&(player[i].aitype==passivetype))
                            setAnimation(i,player[i].getStop());
                    }
                }
                if(player[i].targetanimation==rollanim)
                    player[i].targetrotation=oldtargetrotation;
            }

            //Rotation
            for(int k=0;k<numplayers;k++){
                if(fabs(player[k].rotation-player[k].targetrotation)>180){
                    if(player[k].rotation>player[k].targetrotation)
                        player[k].rotation-=360;
                    else
                        player[k].rotation+=360;
                }

                //stop to turn in right direction
                if(fabs(player[k].rotation-player[k].targetrotation)>90&&(player[k].isRun()||player[k].targetanimation==walkanim))
                    setAnimation(k,player[k].getStop());

                if(player[k].targetanimation==backhandspringanim||player[k].targetanimation==dodgebackanim)
                    player[k].targettilt=0;

                if(player[k].targetanimation!=jumpupanim&&
                        player[k].targetanimation!=backhandspringanim&&
                        player[k].targetanimation!=jumpdownanim&&
                        !player[k].isFlip()){
                    player[k].targettilt=0;
                    if(player[k].jumppower<0&&!player[k].jumpkeydown)
                        player[k].jumppower=0;
                    player[k].jumppower+=multiplier*7;
                    if(player[k].isCrouch())
                        player[k].jumppower+=multiplier*7;
                    if(player[k].jumppower>5)
                        player[k].jumppower=5;
                }

                if(player[k].isRun())
                    player[k].targettilt=(player[k].rotation-player[k].targetrotation)/4;

                player[k].tilt=stepTowardf(player[k].tilt,player[k].targettilt,multiplier*150);
                player[k].grabdelay-=multiplier;
            }

            //do animations
            for(int k=0;k<numplayers;k++){
                player[k].DoAnimations();
                player[k].whichpatchx=player[k].coords.x/(terrain.size/subdivision*terrain.scale);
                player[k].whichpatchz=player[k].coords.z/(terrain.size/subdivision*terrain.scale);
            }

            //do stuff
            objects.DoStuff();
            
            for(int j=numenvsounds-1;j>=0;j--){
                envsoundlife[j]-=multiplier;
                if(envsoundlife[j]<0){
                    numenvsounds--;
                    envsoundlife[j]=envsoundlife[numenvsounds];
                    envsound[j]=envsound[numenvsounds];
                }
            }
            if(slomo)
                OPENAL_SetFrequency(OPENAL_ALL, slomofreq);
            else
                OPENAL_SetFrequency(OPENAL_ALL, 22050);

            if(tutoriallevel==1){
                XYZ temp;
                XYZ temp2;
                XYZ temp3;
                XYZ oldtemp;
                XYZ oldtemp2;
                temp.x=1011;
                temp.y=84;
                temp.z=491;
                temp2.x=1025;
                temp2.y=75;
                temp2.z=447;
                temp3.x=1038;
                temp3.y=76;
                temp3.z=453;
                oldtemp=temp;
                oldtemp2=temp2;
                if(tutorialstage>=51)
                    if(findDistancefast(&temp,&player[0].coords)>=findDistancefast(&temp,&temp2)-1||findDistancefast(&temp3,&player[0].coords)<4){
                        OPENAL_StopSound(OPENAL_ALL);  // hack...OpenAL renderer isn't stopping music after tutorial goes to level menu...
                        OPENAL_SetFrequency(OPENAL_ALL, 0.001);

                        emit_stream_np(stream_music3);

                        gameon=0;
                        mainmenu=5;

                        fireSound();

                        flash();
                    }
                if(tutorialstage<51)
                    if(findDistancefast(&temp,&player[0].coords)>=findDistancefast(&temp,&temp2)-1||findDistancefast(&temp3,&player[0].coords)<4){
                        emit_sound_at(fireendsound, player[0].coords);

                        player[0].coords=(oldtemp+oldtemp2)/2;

                        flash();
                    }
                if(tutorialstage>=14&&tutorialstage<50)
                    if(findDistancefast(&temp,&player[1].coords)>=findDistancefast(&temp,&temp2)-1||findDistancefast(&temp3,&player[1].coords)<4){
                        emit_sound_at(fireendsound, player[1].coords);

                        for(int i=0;i<player[1].skeleton.num_joints;i++){
                            if(Random()%2==0){
                                if(!player[1].skeleton.free)temp2=(player[1].coords-player[1].oldcoords)/multiplier/2;//velocity/2;
                                if(player[1].skeleton.free)temp2=player[1].skeleton.joints[i].velocity*player[1].scale/2;
                                if(!player[1].skeleton.free)temp=DoRotation(DoRotation(DoRotation(player[1].skeleton.joints[i].position,0,0,player[1].tilt),player[1].tilt2,0,0),0,player[1].rotation,0)*player[1].scale+player[1].coords;
                                if(player[1].skeleton.free)temp=player[1].skeleton.joints[i].position*player[1].scale+player[1].coords;
                                Sprite::MakeSprite(breathsprite, temp,temp2, 1,1,1, .6+(float)abs(Random()%100)/200-.25, 1);
                            }
                        }

                        player[1].coords=(oldtemp+oldtemp2)/2;
                        for(int i=0;i<player[1].skeleton.num_joints;i++){
                            player[1].skeleton.joints[i].velocity=0;
                            if(Random()%2==0){
                                if(!player[1].skeleton.free)temp2=(player[1].coords-player[1].oldcoords)/multiplier/2;//velocity/2;
                                if(player[1].skeleton.free)temp2=player[1].skeleton.joints[i].velocity*player[1].scale/2;
                                if(!player[1].skeleton.free)temp=DoRotation(DoRotation(DoRotation(player[1].skeleton.joints[i].position,0,0,player[1].tilt),player[1].tilt2,0,0),0,player[1].rotation,0)*player[1].scale+player[1].coords;
                                if(player[1].skeleton.free)temp=player[1].skeleton.joints[i].position*player[1].scale+player[1].coords;
                                Sprite::MakeSprite(breathsprite, temp,temp2, 1,1,1, .6+(float)abs(Random()%100)/200-.25, 1);
                            }
                        }
                    }
            }


            //3d sound
            static float gLoc[3];
            gLoc[0]=viewer.x;
            gLoc[1]=viewer.y;
            gLoc[2]=viewer.z;
            static float vel[3];
            vel[0]=(viewer.x-oldviewer.x)/multiplier;
            vel[1]=(viewer.y-oldviewer.y)/multiplier;
            vel[2]=(viewer.z-oldviewer.z)/multiplier;

            //Set orientation with forward and up vectors
            static XYZ upvector;
            upvector=0;
            upvector.z=-1;

            upvector=DoRotation(upvector,-rotation2+90,0,0);
            upvector=DoRotation(upvector,0,0-rotation,0);

            facing=0;
            facing.z=-1;

            facing=DoRotation(facing,-rotation2,0,0);
            facing=DoRotation(facing,0,0-rotation,0);


            static float ori[6];
            ori[0] = -facing.x;
            ori[1] = facing.y;
            ori[2] = -facing.z;
            ori[3] = -upvector.x;
            ori[4] = upvector.y;
            ori[5] = -upvector.z;

            OPENAL_3D_Listener_SetAttributes(&gLoc[0], &vel[0], ori[0], ori[1], ori[2], ori[3], ori[4], ori[5]);
            OPENAL_Update();

            oldviewer=viewer;
		}
	}

	if(Input::isKeyPressed(SDLK_F1))
		Screenshot();
}



void	Game::TickOnce(){
	if(mainmenu)
		rotation+=multiplier*5;
	else
		if(directing||indialogue==-1) {
			rotation+=deltah*.7;
			if(!invertmouse)
                rotation2+=deltav*.7;
			if(invertmouse)
                rotation2-=deltav*.7;
			if(rotation2>90)
                rotation2=90;
			if(rotation2<-70)
                rotation2=-70;
		}
}

void	Game::TickOnceAfter(){
	static XYZ colviewer;
	static XYZ coltarget;
	static XYZ target;
	static XYZ col;
	static float brotate;
	static XYZ facing;
	static float changedelay;
	static bool alldead;
	static float unseendelay;
	static float cameraspeed;

	if(!mainmenu){

		if(environment==snowyenvironment)
            music1=stream_music1snow;
		if(environment==grassyenvironment)
            music1=stream_music1grass;
		if(environment==desertenvironment)
            music1=stream_music1desert;

		realthreat=0;

		musictype=music1;
		for(int i=0;i<numplayers;i++){
			if((player[i].aitype==attacktypecutoff||
                        player[i].aitype==getweapontype||
                        player[i].aitype==gethelptype||
                        player[i].aitype==searchtype)&&
                    !player[i].dead/*&&player[i].surprised<=0*/&&
                    (player[i].targetanimation!=sneakattackedanim&&
                     player[i].targetanimation!=knifesneakattackedanim&&
                     player[i].targetanimation!=swordsneakattackedanim)){
				musictype=stream_music2;
				realthreat=1;
			}
		}
		if(player[0].dead)
            musictype=stream_music3;


		if(musictype==stream_music2)
			unseendelay=1;

		if(oldmusictype==stream_music2&&musictype!=stream_music2){
			unseendelay-=multiplier;
			if(unseendelay>0)
				musictype=stream_music2;
		}


		if(loading==2){
			musictype=stream_music3;
			musicvolume[2]=512;
			musicvolume[0]=0;
			musicvolume[1]=0;
			musicvolume[3]=0;
		}

		if(musictoggle)
			if(musictype!=oldmusictype&&musictype==stream_music2)
				emit_sound_np(alarmsound);
		musicselected=musictype;

		if(musicselected==music1)
            musicvolume[0]+=multiplier*450;
		else
            musicvolume[0]-=multiplier*450;
		if(musicselected==stream_music2)
            musicvolume[1]+=multiplier*450;
		else
            musicvolume[1]-=multiplier*450;
		if(musicselected==stream_music3)
            musicvolume[2]+=multiplier*450;
		else
            musicvolume[2]-=multiplier*450;

		for(int i=0;i<3;i++){
			if(musicvolume[i]<0)
                musicvolume[i]=0;
			if(musicvolume[i]>512)
                musicvolume[i]=512;
		}

		if(musicvolume[2]>128&&!loading&&!mainmenu)
            musicvolume[2]=128;

		if(musictoggle){
			if(musicvolume[0]>0&&oldmusicvolume[0]<=0)
			  emit_stream_np(music1, musicvolume[0]);
			if(musicvolume[1]>0&&oldmusicvolume[1]<=0)
			  emit_stream_np(stream_music2, musicvolume[1]);
			if(musicvolume[2]>0&&oldmusicvolume[2]<=0)
			  emit_stream_np(stream_music3, musicvolume[2]);
			if(musicvolume[0]<=0&&oldmusicvolume[0]>0)
				pause_sound(music1);
			if(musicvolume[1]<=0&&oldmusicvolume[1]>0)
				pause_sound(stream_music2);
			if(musicvolume[2]<=0&&oldmusicvolume[2]>0)
				pause_sound(stream_music3);

			if(musicvolume[0]!=oldmusicvolume[0])
				OPENAL_SetVolume(channels[music1], musicvolume[0]);
			if(musicvolume[1]!=oldmusicvolume[1])
				OPENAL_SetVolume(channels[stream_music2], musicvolume[1]);
			if(musicvolume[2]!=oldmusicvolume[2])
				OPENAL_SetVolume(channels[stream_music3], musicvolume[2]);

			for(int i=0;i<3;i++)
				oldmusicvolume[i]=musicvolume[i];
		} else {
			pause_sound(music1);
			pause_sound(stream_music2);
			pause_sound(stream_music3);

			for(int i=0;i<4;i++){
				oldmusicvolume[i]=0;
				musicvolume[i]=0;
			}
		}

		killhotspot=2;
		for(int i=0;i<numhotspots;i++){
			if(hotspottype[i]>10&&hotspottype[i]<20){
				if(player[hotspottype[i]-10].dead==0)
					killhotspot=0;
				else if(killhotspot==2)
					killhotspot=1;
			}
		}
		if(killhotspot==2)
            killhotspot=0;


		winhotspot=0;
		for(int i=0;i<numhotspots;i++)
			if(hotspottype[i]==-1)
				if(findDistancefast(&player[0].coords,&hotspot[i])<hotspotsize[i])
					winhotspot=1;

		int numalarmed=0;
		for(int i=1;i<numplayers;i++)
			if(!player[i].dead&&player[i].aitype==attacktypecutoff&&player[i].surprised<=0)
                numalarmed++;
		if(numalarmed>maxalarmed)
            maxalarmed=numalarmed;

		if(changedelay<=0&&!loading&&!editorenabled&&gameon&&!tutoriallevel&&changedelay!=-999&&!won){
			if(player[0].dead&&changedelay<=0){
				changedelay=1;
				targetlevel=whichlevel;
			}
			alldead=1;
			for(int i=1;i<numplayers;i++)
				if(!player[i].dead&&player[i].howactive<typedead1)
                    alldead=0;


			if(alldead&&!player[0].dead&&maptype==mapkilleveryone){
				changedelay=1;
				targetlevel=whichlevel+1;
				if(targetlevel>numchallengelevels-1)targetlevel=0;
			}
			if(winhotspot||windialogue){
				changedelay=0.1;
				targetlevel=whichlevel+1;
				if(targetlevel>numchallengelevels-1)targetlevel=0;
			}


			if(killhotspot){
				changedelay=1;
				targetlevel=whichlevel+1;
				if(targetlevel>numchallengelevels-1)targetlevel=0;
			}

			if(changedelay>0&&!player[0].dead&&!won){
				//high scores, awards, win
				if(campaign){
					accountactive->winCampaignLevel(whichchoice, bonustotal, leveltime);
					scoreadded=1;
				}else{
					accountactive->winLevel(whichlevel,bonustotal-startbonustotal,leveltime);
				}
				won=1;
			}
		}

		if(!winfreeze){

			if(leveltime<1){
				loading=0;
				changedelay=.1;
				alldead=0;
				winhotspot=0;
				killhotspot=0;
			}

			if(!editorenabled&&gameon&&!mainmenu){
				if(changedelay!=-999)
                    changedelay-=multiplier/7;
				if(player[0].dead)
                    targetlevel=whichlevel;
				if(loading==2&&!campaign){
					flash();

					fireSound(firestartsound);

					if(!player[0].dead&&targetlevel!=whichlevel)
						startbonustotal=bonustotal;
					if(player[0].dead)
                        Loadlevel(whichlevel);
					else
                        Loadlevel(targetlevel);

					fireSound();

					loading=3;
				}
				if(loading==2&&targetlevel==whichlevel){
					flash();
					loadtime=0;

					fireSound(firestartsound);

					Loadlevel(campaignmapname[levelorder[accountactive->getCampaignChoicesMade()]]);

					fireSound();

					loading=3;
				}
				if(changedelay<=-999&&
                        whichlevel!=-2&&
                        !loading&&
                        (player[0].dead||
                         (alldead&&maptype==mapkilleveryone)||
                         (winhotspot)||
                         (killhotspot))&&
                        !winfreeze)
                    loading=1;
				if((player[0].dead||
                            (alldead&&maptype==mapkilleveryone)||
                            (winhotspot)||
                            (windialogue)||
                            (killhotspot))&&
                        changedelay<=0){
                    if(whichlevel!=-2&&!loading&&!player[0].dead){
                        winfreeze=1;
                        changedelay=-999;
                    }
                    if(player[0].dead)
                        loading=1;
				}
			}

			if(campaign)
				if(mainmenu==0&&winfreeze&&(campaignchoosenext[campaignchoicewhich[whichchoice]])==1)
					if(campaignnumnext[campaignchoicewhich[whichchoice]]==0)
						endgame=1;
				else if(mainmenu==0&&winfreeze){
					if(campaignchoosenext[campaignchoicewhich[whichchoice]]==2)
						stealthloading=1;
					else
                        stealthloading=0;

					if(!stealthloading){
						float gLoc[3]={0,0,0};
						float vel[3]={0,0,0};
						fireSound(firestartsound);

						flash();
					}

					startbonustotal=0;

					ifstream ipstream(ConvertFileName(":Data:Campaigns:main.txt"));
					ipstream.ignore(256,':');
					ipstream >> campaignnumlevels;
					for(int i=0;i<campaignnumlevels;i++){
						ipstream.ignore(256,':');
						ipstream.ignore(256,':');
						ipstream.ignore(256,' ');
						ipstream >> campaignmapname[i];
						ipstream.ignore(256,':');
						ipstream >> campaigndescription[i];
						for(int j=0;j<256;j++){
							if(campaigndescription[i][j]=='_')campaigndescription[i][j]=' ';
						}
						ipstream.ignore(256,':');
						ipstream >> campaignchoosenext[i];
						ipstream.ignore(256,':');
						ipstream >> campaignnumnext[i];
						for(int j=0;j<campaignnumnext[i];j++){
							ipstream.ignore(256,':');
							ipstream >> campaignnextlevel[i][j];
							campaignnextlevel[i][j]-=1;
						}
						ipstream.ignore(256,':');
						ipstream >> campaignlocationx[i];
						ipstream.ignore(256,':');
						ipstream >> campaignlocationy[i];
					}
					ipstream.close();

					for(int i=0;i<campaignnumlevels;i++){
						levelvisible[i]=0;
						levelhighlight[i]=0;
					}


					for(int i=0;i<campaignnumlevels;i++){
						levelvisible[i]=0;
						levelhighlight[i]=0;
					}

					levelorder[0]=0;
					levelvisible[0]=1;
					for(int i=0;i<accountactive->getCampaignChoicesMade();i++){
						levelorder[i+1]=campaignnextlevel[levelorder[i]][accountactive->getCampaignChoice(i)];
						levelvisible[levelorder[i+1]]=1;
					}
					int whichlevelstart;
					whichlevelstart=accountactive->getCampaignChoicesMade()-1;
					if(whichlevelstart<0){
						campaignchoicenum=1;
						campaignchoicewhich[0]=0;
					}
					else
					{
						campaignchoicenum=campaignnumnext[levelorder[whichlevelstart]];
						for(int i=0;i<campaignchoicenum;i++){
							campaignchoicewhich[i]=campaignnextlevel[levelorder[whichlevelstart]][i];
							levelvisible[campaignnextlevel[levelorder[whichlevelstart]][i]]=1;
							levelhighlight[campaignnextlevel[levelorder[whichlevelstart]][i]]=1;
						}
					}

					loading=2;
					loadtime=0;
					targetlevel=7;
					if(!firstload)LoadStuff();
					whichchoice=0;
					visibleloading=1;
					stillloading=1;
					Loadlevel(campaignmapname[campaignchoicewhich[0]]);
					campaign=1;
					mainmenu=0;
					gameon=1;
					pause_sound(stream_music3);

					stealthloading=0;
				}

            if(loading==3)
                loading=0;

        }

        oldmusictype=musictype;
	}

	facing=0;
	facing.z=-1;

	facing=DoRotation(facing,-rotation2,0,0);
	facing=DoRotation(facing,0,0-rotation,0);
	viewerfacing=facing;

	brotate=0;
	if(!cameramode){
		if((animation[player[0].targetanimation].attack!=3&&animation[player[0].currentanimation].attack!=3)||player[0].skeleton.free)target=player[0].coords+player[0].currentoffset*(1-player[0].target)*player[0].scale+player[0].targetoffset*player[0].target*player[0].scale-player[0].facing*.05;
		else target=player[0].oldcoords+player[0].currentoffset*(1-player[0].target)*player[0].scale+player[0].targetoffset*player[0].target*player[0].scale-player[0].facing*.05;
		target.y+=.1;
		if(player[0].skeleton.free){
			for(int i=0;i<player[0].skeleton.num_joints;i++){
				if(player[0].skeleton.joints[i].position.y*player[0].scale+player[0].coords.y>target.y)
					target.y=player[0].skeleton.joints[i].position.y*player[0].scale+player[0].coords.y;
			}
			target.y+=.1;
		}
		if(player[0].skeleton.free!=2&&!autocam){
			cameraspeed=20;
			if(findLengthfast(&player[0].velocity)>400){
				cameraspeed=20+(findLength(&player[0].velocity)-20)*.96;
			}
			if(player[0].skeleton.free==0&&player[0].targetanimation!=hanganim&&player[0].targetanimation!=climbanim)target.y+=1.4;
			coltarget=target-cameraloc;
			if(findLengthfast(&coltarget)<multiplier*multiplier*400)cameraloc=target;
			else {
				Normalise(&coltarget);
				if(player[0].targetanimation!=hanganim&&player[0].targetanimation!=climbanim&&player[0].currentanimation!=climbanim&&player[0].currentoffset.x==0)cameraloc=cameraloc+coltarget*multiplier*cameraspeed;
				else cameraloc=cameraloc+coltarget*multiplier*8;
			}
			if(editorenabled)cameraloc=target;
			cameradist+=multiplier*5;
			if(cameradist>2.3)cameradist=2.3;
			viewer=cameraloc-facing*cameradist;
			colviewer=viewer;
			coltarget=cameraloc;
			objects.SphereCheckPossible(&colviewer, findDistance(&colviewer,&coltarget));
			if(terrain.patchobjectnum[player[0].whichpatchx][player[0].whichpatchz])
				for(int j=0;j<terrain.patchobjectnum[player[0].whichpatchx][player[0].whichpatchz];j++){
					int i=terrain.patchobjects[player[0].whichpatchx][player[0].whichpatchz][j];
					colviewer=viewer;
					coltarget=cameraloc;
					if(objects.model[i].LineCheckPossible(&colviewer,&coltarget,&col,&objects.position[i],&objects.rotation[i])!=-1)viewer=col;
				}
            if(terrain.patchobjectnum[player[0].whichpatchx][player[0].whichpatchz])
                for(int j=0;j<terrain.patchobjectnum[player[0].whichpatchx][player[0].whichpatchz];j++){
                    int i=terrain.patchobjects[player[0].whichpatchx][player[0].whichpatchz][j];
                    colviewer=viewer;
                    if(objects.model[i].SphereCheck(&colviewer,.15,&col,&objects.position[i],&objects.rotation[i])!=-1){
                        viewer=colviewer;
                    }
                }
            cameradist=findDistance(&viewer,&target);
            viewer.y=max((double)viewer.y,terrain.getHeight(viewer.x,viewer.z)+.6);
            if(cameraloc.y<terrain.getHeight(cameraloc.x,cameraloc.z)){
                cameraloc.y=terrain.getHeight(cameraloc.x,cameraloc.z);
            }
		}
		if(player[0].skeleton.free!=2&&autocam){
			cameraspeed=20;
			if(findLengthfast(&player[0].velocity)>400){
				cameraspeed=20+(findLength(&player[0].velocity)-20)*.96;
			}
			if(player[0].skeleton.free==0&&player[0].targetanimation!=hanganim&&player[0].targetanimation!=climbanim)target.y+=1.4;
			cameradist+=multiplier*5;
			if(cameradist>3.3)cameradist=3.3;
			coltarget=target-cameraloc;
			if(findLengthfast(&coltarget)<multiplier*multiplier*400)cameraloc=target;
			else if(findLengthfast(&coltarget)>1)
			{
				Normalise(&coltarget);
				if(player[0].targetanimation!=hanganim&&player[0].targetanimation!=climbanim&&player[0].currentanimation!=climbanim&&player[0].currentoffset.x==0)cameraloc=cameraloc+coltarget*multiplier*cameraspeed;
				else cameraloc=cameraloc+coltarget*multiplier*8;
			}
			if(editorenabled)cameraloc=target;
			viewer=cameraloc;
			colviewer=viewer;
			coltarget=cameraloc;
			objects.SphereCheckPossible(&colviewer, findDistance(&colviewer,&coltarget));
			if(terrain.patchobjectnum[player[0].whichpatchx][player[0].whichpatchz])
				for(int j=0;j<terrain.patchobjectnum[player[0].whichpatchx][player[0].whichpatchz];j++){
					int i=terrain.patchobjects[player[0].whichpatchx][player[0].whichpatchz][j];
					colviewer=viewer;
					coltarget=cameraloc;
					if(objects.model[i].LineCheckPossible(&colviewer,&coltarget,&col,&objects.position[i],&objects.rotation[i])!=-1)viewer=col;
				}
            if(terrain.patchobjectnum[player[0].whichpatchx][player[0].whichpatchz])
                for(int j=0;j<terrain.patchobjectnum[player[0].whichpatchx][player[0].whichpatchz];j++){
                    int i=terrain.patchobjects[player[0].whichpatchx][player[0].whichpatchz][j];
                    colviewer=viewer;
                    if(objects.model[i].SphereCheck(&colviewer,.15,&col,&objects.position[i],&objects.rotation[i])!=-1){
                        viewer=colviewer;
                    }
                }
            cameradist=findDistance(&viewer,&target);
            viewer.y=max((double)viewer.y,terrain.getHeight(viewer.x,viewer.z)+.6);
            if(cameraloc.y<terrain.getHeight(cameraloc.x,cameraloc.z)){
                cameraloc.y=terrain.getHeight(cameraloc.x,cameraloc.z);
            }
		}
		if(camerashake>.8)camerashake=.8;
		//if(woozy>10)woozy=10;
		//woozy+=multiplier;
		woozy+=multiplier;
		if(player[0].dead)camerashake=0;
		if(player[0].dead)woozy=0;
		camerashake-=multiplier*2;
		blackout-=multiplier*2;
		//if(player[0].isCrouch())woozy-=multiplier*8;
		if(camerashake<0)camerashake=0;
		if(blackout<0)blackout=0;
		//if(woozy<0)woozy=0;
		if(camerashake){
			viewer.x+=(float)(Random()%100)*.0005*camerashake;
			viewer.y+=(float)(Random()%100)*.0005*camerashake;
			viewer.z+=(float)(Random()%100)*.0005*camerashake;
		}
	}
}

