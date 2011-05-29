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

#ifndef _GAME_H_
#define _GAME_H_

#include "SDL.h"

#if (defined(__APPLE__) && defined(__MACH__))
#  ifdef PLATFORM_MACOSX
#    error Do not define PLATFORM_MACOSX for new builds. It is for the old Carbonized build.
#  endif
#endif

#ifdef PLATFORM_MACOSX
#include <Carbon.h>
#include "Quicktime.h"
#endif

//Jordan included glut.h
//#include <glut.h>

#include "TGALoader.h"

#include "Terrain.h"
#include "Skybox.h"
#include "Skeleton.h"
#include "Models.h"
#include "Lights.h"
#include "Person.h"
#include "Sprite.h"
//#include <agl.h>
#include "Text.h"
#include "Objects.h"
//#include <DrawSprocket.h>
#include "Weapons.h"
#include "binio.h"
#include <fstream>
#include "gamegl.h"
#include "Stereo.h"
#include "Account.h"
#include "Sounds.h"

#define NB_CAMPAIGN_MENU_ITEM 7

extern GLuint rabbittexture;

struct TextureInfo;

class CampaignLevel
{
private:
	int width;
	struct Position
	{
		int x;
		int y;
	};
public: 
	std::string mapname;
	std::string description;
	int choosenext;
		/*	
	  	0 = Immediately load next level at the end of this one.
		1 = Go back to the world map.
		2 = Don't bring up the Fiery loading screen. Maybe other things, I've not investigated.
		*/
	//int numnext; // 0 on final level. As David said: he meant to add story branching, but he eventually hadn't. 
	std::vector<int> nextlevel;
	Position location;
	
	CampaignLevel() : width(10) {
		choosenext = 1;
		location.x = 0;
		location.y = 0;
	}
	
	int getStartX() {
		return 30+120+location.x*400/512;
	}
	
	int getStartY() {
		return 30+30+(512-location.y)*400/512;
	}
	
	int getEndX() {
		return getStartX()+width;
	}
	
	int getEndY() {
		return getStartY()+width;
	}
	
	XYZ getCenter() {
		XYZ center;
		center.x=getStartX()+width/2;
		center.y=getStartY()+width/2;
		return center;
	}
	
	int getWidth() {
		return width;
	}
	
	istream& operator<< (istream& is) {
		is.ignore(256,':');
		is.ignore(256,':');
		is.ignore(256,' ');
		is >> mapname;
		is.ignore(256,':');
		is >> description;
		for(int pos = description.find('_');pos!=string::npos;pos = description.find('_',pos)) {
			description.replace(pos,1,1,' ');
		}
		is.ignore(256,':');
		is >> choosenext;
		is.ignore(256,':');
		int numnext,next;
		is >> numnext;
		for(int j=0;j<numnext;j++) {
			is.ignore(256,':');
			is >> next;
			nextlevel.push_back(next-1);
		}
		is.ignore(256,':');
		is >> location.x;
		is.ignore(256,':');
		is >> location.y;
		return is;
	}
	
	friend istream& operator>> (istream& is, CampaignLevel& cl) {
		return cl << is;
	}
};

namespace Game
{
    //public:
    extern GLuint terraintexture;
    extern GLuint terraintexture2;
    extern GLuint terraintexture3;
    extern GLuint screentexture;
    extern GLuint screentexture2;
    extern GLuint logotexture;
    extern GLuint loadscreentexture;
    extern GLuint Maparrowtexture;
    extern GLuint Mapboxtexture;
    extern GLuint Mapcircletexture;
    extern GLuint cursortexture;
    extern GLuint Mainmenuitems[10];

    extern int selected;
    extern int keyselect;
    extern int indemo;

    extern bool won;

    extern bool entername;

    extern char registrationname[256];
    extern float registrationnumber;

    extern int newdetail;
    extern int newscreenwidth;
    extern int newscreenheight;

    extern bool gameon;
    extern float deltah,deltav;
    extern int mousecoordh,mousecoordv;
    extern int oldmousecoordh,oldmousecoordv;
    extern float rotation,rotation2;
    extern SkyBox skybox;
    extern bool cameramode;
    extern int olddrawmode;
    extern int drawmode;
    extern bool firstload;
    extern bool oldbutton;

    extern float leveltime;
    extern float loadtime;

    extern Model hawk;
    extern XYZ hawkcoords;
    extern XYZ realhawkcoords;
    extern GLuint hawktexture;
    extern float hawkrotation;
    extern float hawkcalldelay;

    extern Model eye;
    extern Model iris;
    extern Model cornea;

    extern bool stealthloading;

    extern std::vector<CampaignLevel> campaignlevels;
    extern int whichchoice;
    extern int actuallevel;
    extern bool winhotspot;
    extern bool windialogue;

    extern bool minimap;

    extern int musictype,oldmusictype,oldoldmusictype;
    extern bool realthreat;

    extern Model rabbit;
    extern XYZ rabbitcoords;

    extern XYZ mapcenter;
    extern float mapradius;

    extern Text* text;
    extern float fps;

    extern XYZ cameraloc;
    extern float cameradist;

    extern int drawtoggle;

    extern bool editorenabled;
    extern int editortype;
    extern float editorsize;
    extern float editorrotation;
    extern float editorrotation2;

    extern float brightness;

    extern int quit;
    extern int tryquit;

    extern XYZ pathpoint[30];
    extern int numpathpoints;
    extern int numpathpointconnect[30];
    extern int pathpointconnect[30][30];
    extern int pathpointselected;

    extern int endgame;
    extern bool scoreadded;
    extern int numchallengelevels;

    extern bool console;
    extern int archiveselected;
    extern char consoletext[15][256];
    extern int consolechars[15];
    extern bool chatting;
    extern char displaytext[15][256];
    extern int displaychars[15];
    extern float displaytime[15];
    extern float displayblinkdelay;
    extern bool displayblink;
    extern int displayselected;
    extern bool consolekeydown;
    extern float consoleblinkdelay;
    extern bool consoleblink;
    extern int consoleselected;
    extern bool autocam;

    extern unsigned short crouchkey,jumpkey,forwardkey,chatkey,backkey,leftkey,rightkey,drawkey,throwkey,attackkey;
    extern unsigned short consolekey;
    extern bool oldattackkey;

    void LoadTexture(const string fileName, GLuint *textureid,int mipmap, bool hasalpha);
    void LoadTextureSave(const string fileName, GLuint *textureid,int mipmap,GLubyte *array, int *skinsize);
    void LoadSave(const char *fileName, GLuint *textureid,bool mipmap,GLubyte *array, int *skinsize);
    bool AddClothes(const char *fileName, GLubyte *array);
    void InitGame();
    void LoadScreenTexture();
    void LoadStuff();
    void LoadingScreen();
    void LoadCampaign();
    std::vector<std::string> ListCampaigns();
    void FadeLoadingScreen(float howmuch);
    void Dispose();
    int DrawGLScene(StereoSide side);
    void DrawMenu();
    void DrawGL();
    void LoadMenu();
    //factored from Tick() -sf17k
    void MenuTick();
    void doTutorial();
    void doDebugKeys();
    void doJumpReversals();
    void doAerialAcrobatics();
    void doAttacks();
    void doPlayerCollisions();
    void doAI(int i);
    //end factored
    void Tick();
    void TickOnce();
    void TickOnceAfter();
    void SetUpLighting();
    void Loadlevel(int which);
    void Loadlevel(const char *name);
    void Setenvironment(int which);
    GLvoid ReSizeGLScene(float fov, float near);
    int findPathDist(int start,int end);
    int checkcollide(XYZ startpoint, XYZ endpoint);
    int checkcollide(XYZ startpoint, XYZ endpoint, int what);
    extern int loading;
    extern float talkdelay;
    
    void fireSound(int sound=fireendsound);
    void setKeySelected();

    extern int numboundaries;
    extern XYZ boundary[360];

    extern int whichlevel;
    extern int oldenvironment;
    extern int targetlevel;
    extern float changedelay;

    extern float musicvolume[4];
    extern float oldmusicvolume[4];
    extern int musicselected;
    extern int change;
    void newGame();
    void deleteGame();
    extern bool waiting;
    //private:
    int setKeySelected_thread(void*);
    int thread(void *data);
    void inputText(char* str, int* charselected, int* nb_chars);
    void flash();
    //bool waiting;
    extern Account* accountactive;
}

#ifndef __forceinline
#  ifdef __GNUC__
#    define __forceinline inline __attribute__((always_inline))
#  endif
#endif

static __forceinline void swap_gl_buffers(void)
{
    SDL_GL_SwapBuffers();
}

extern "C" { void UndefinedSymbolToExposeStubbedCode(void); }
//#define STUBBED(x) UndefinedSymbolToExposeStubbedCode();
#define STUBBED(x) { static bool seen = false; if (!seen) { seen = true; fprintf(stderr, "STUBBED: %s at %s:%d\n", x, __FILE__, __LINE__); } }
//#define STUBBED(x)

extern int numplayers;

extern int numdialogues;
const int max_dialogues = 20;
const int max_dialoguelength = 20;
extern int numdialogueboxes[max_dialogues];
extern int dialoguetype[max_dialogues];
extern int dialogueboxlocation[max_dialogues][max_dialoguelength];
extern float dialogueboxcolor[max_dialogues][max_dialoguelength][3];
extern int dialogueboxsound[max_dialogues][max_dialoguelength];
extern char dialoguetext[max_dialogues][max_dialoguelength][128];
extern char dialoguename[max_dialogues][max_dialoguelength][64];
extern XYZ dialoguecamera[max_dialogues][max_dialoguelength];
extern XYZ participantlocation[max_dialogues][10];
extern int participantfocus[max_dialogues][max_dialoguelength];
extern int participantaction[max_dialogues][max_dialoguelength];
extern float participantrotation[max_dialogues][10];
extern XYZ participantfacing[max_dialogues][max_dialoguelength][10];
extern float dialoguecamerarotation[max_dialogues][max_dialoguelength];
extern float dialoguecamerarotation2[max_dialogues][max_dialoguelength];
extern int indialogue;
extern int whichdialogue;
extern int directing;
extern float dialoguetime;
extern int dialoguegonethrough[20];

enum maptypes {
  mapkilleveryone, mapgosomewhere,
  mapkillsomeone, mapkillmost // These two are unused
};

enum pathtypes {wpkeepwalking, wppause};

static const char *pathtypenames[] = {"keepwalking", "pause"};

enum editortypes {typeactive, typesitting, typesittingwall, typesleeping,
		  typedead1, typedead2, typedead3, typedead4};

static const char *editortypenames[] = {
  "active", "sitting", "sitting wall", "sleeping",
  "dead1", "dead2", "dead3", "dead4"
};

#endif
