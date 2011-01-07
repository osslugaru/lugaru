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

extern GLuint rabbittexture;

class Game
{
	public:
		typedef std::map<std::string, GLuint> TextureList;
		typedef std::map<GLuint, std::string> GLTextureList;
		typedef TextureList::iterator TexIter;
		static TextureList textures;

		GLuint terraintexture;
		GLuint terraintexture2;
		GLuint terraintexture3;
		GLuint screentexture;
		GLuint screentexture2;
		GLuint logotexture;
		GLuint loadscreentexture;
		GLuint Maparrowtexture;
		GLuint Mapboxtexture;
		GLuint Mapcircletexture;
		GLuint cursortexture;
		GLuint Mainmenuitems[10];

		int nummenuitems;
		int startx[100];
		int starty[100];
		int endx[100];
		int endy[100];
		float selectedlong[100];
		float offsetx[100];
		float offsety[100];
		float movex[100];
		float movey[100];
		float transition;
		int anim;
		int selected;
		int keyselect;
		int loaddistrib;
		int indemo;

		bool won;

		bool entername;

		char menustring[100][256];
		char registrationname[256];
		float registrationnumber;

		int newdetail;
		int newscreenwidth;
		int newscreenheight;

		bool gameon;
		float deltah,deltav;
		int mousecoordh,mousecoordv;
		int oldmousecoordh,oldmousecoordv;
		float rotation,rotation2;
		SkyBox skybox;
		bool cameramode;
		bool cameratogglekeydown;
		bool chattogglekeydown;
		int olddrawmode;
		int drawmode;
		bool drawmodetogglekeydown;
		bool explodetogglekeydown;
		bool detailtogglekeydown;
		bool firstload;
		bool oldbutton;

		float leveltime;
		float loadtime;

		Model hawk;
		XYZ hawkcoords;
		XYZ realhawkcoords;
		GLuint hawktexture;
		float hawkrotation;
		float hawkcalldelay;

		Model eye;
		Model iris;
		Model cornea;

		bool stealthloading;

		int campaignnumlevels;
		char campaignmapname[50][256];
		char campaigndescription[50][256];
		int campaignchoosenext[50];
		int campaignnumnext[50];
		int campaignnextlevel[50][10];
		int campaignchoicesmade;
		int campaignchoices[5000];
		int campaignlocationx[50];
		int campaignlocationy[50];
		int campaignchoicenum;
		int campaignchoicewhich[10];
		int whichchoice;

		int numlevelspassed;
		int levelorder[5000];
		int levelvisible[50];
		int levelhighlight[50];

		bool minimap;

		int musictype,oldmusictype,oldoldmusictype;
		bool realthreat;

		Model rabbit;
		XYZ rabbitcoords;

		XYZ mapcenter;
		float mapradius;

		Text text;
		float fps;

		XYZ cameraloc;
		float cameradist;

		bool envtogglekeydown;
		bool slomotogglekeydown;
		bool texturesizetogglekeydown;
		bool freezetogglekeydown;
		int drawtoggle;

		bool editorenabled;
		int editortype;
		float editorsize;
		float editorrotation;
		float editorrotation2;

		float brightness;

		int quit;
		int tryquit;

		XYZ pathpoint[30];
		int numpathpoints;
		int numpathpointconnect[30];
		int pathpointconnect[30][30];
		int pathpointselected;

		int endgame;
		bool scoreadded;
		int numchallengelevels;

		bool console;
		int archiveselected;
		char consoletext[15][256];
		int consolechars[15];
		bool chatting;
		char displaytext[15][256];
		int displaychars[15];
		float displaytime[15];
		float displayblinkdelay;
		bool displayblink;
		int displayselected;
		bool consolekeydown;
		bool consoletogglekeydown;
		float consoleblinkdelay;
		bool consoleblink;
		int consoleselected;
		//int togglekey[140];
		//float togglekeydelay[140];
		bool autocam;

		unsigned short crouchkey,jumpkey,forwardkey,chatkey,backkey,leftkey,rightkey,drawkey,throwkey,attackkey;
		bool oldattackkey;

		static void LoadTexture(const char *fileName, GLuint *textureid,int mipmap, bool hasalpha);
		static void LoadTextureSave(const char *fileName, GLuint *textureid,int mipmap,GLubyte *array, int *skinsize);
		void LoadSave(const char *fileName, GLuint *textureid,bool mipmap,GLubyte *array, int *skinsize);
		bool AddClothes(const char *fileName, GLuint *textureid,bool mipmap,GLubyte *array, int *skinsize);
		void InitGame();
		void LoadStuff();
		void LoadingScreen();
		void FadeLoadingScreen(float howmuch);
		void Dispose();
		int DrawGLScene(StereoSide side);
		void DrawGL();
		void Tick();
		void TickOnce();
		void TickOnceAfter();
		void SetUpLighting();
		void Loadlevel(int which);
		void Loadlevel(char *name);
		void Setenvironment(int which);
		GLvoid ReSizeGLScene(float fov, float near);
		int findPathDist(int start,int end);
		int checkcollide(XYZ startpoint, XYZ endpoint);
		int checkcollide(XYZ startpoint, XYZ endpoint, int what);
		int loading;
		float talkdelay;
		
		void fireSound(int sound=fireendsound);
		void setKeySelected();

		int numboundaries;
		XYZ boundary[360];

		int whichlevel;
		int oldenvironment;
		int targetlevel;
		float changedelay;

		float musicvolume[4];
		float oldmusicvolume[4];
		int musicselected;
		int change;
		Game();
		~Game() {
			for(int i=0;i<10;i++){
				if(Mainmenuitems[i])glDeleteTextures( 1, &Mainmenuitems[i] );
			}
			glDeleteTextures( 1, &cursortexture );
			glDeleteTextures( 1, &Maparrowtexture );
			glDeleteTextures( 1, &Mapboxtexture );
			glDeleteTextures( 1, &Mapcircletexture );
			glDeleteTextures( 1, &terraintexture );
			glDeleteTextures( 1, &terraintexture2 );
			if(screentexture>0)glDeleteTextures( 1, &screentexture );
			if(screentexture2>0)glDeleteTextures( 1, &screentexture2 );
			glDeleteTextures( 1, &hawktexture );
			glDeleteTextures( 1, &logotexture );
			glDeleteTextures( 1, &loadscreentexture );

			Dispose();
		}
		bool isWaiting() { return waiting; };
	private:
		void setKeySelected_thread();
		static int thread(void *data);
		void inputText(char* str, int* charselected, int* nb_chars);
		void flash();
		bool waiting;
		bool mainmenutogglekeydown;
		//int mainmenu;
		Account* accountactive;
};

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
