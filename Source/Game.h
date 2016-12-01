/*
Copyright (C) 2003, 2010 - Wolfire Games
Copyright (C) 2010-2016 - Lugaru contributors (see AUTHORS file)

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

#ifndef _GAME_H_
#define _GAME_H_

#include "SDL.h"

#include "ImageIO.h"

#include "Terrain.h"
#include "Skybox.h"
#include "Skeleton.h"
#include "Models.h"
#include "Lights.h"
#include "Person.h"
#include "Sprite.h"
#include "Text.h"
#include "Objects.h"
#include "Weapons.h"
#include "binio.h"
#include <fstream>
#include "gamegl.h"
#include "Stereo.h"
#include "Account.h"
#include "Sounds.h"
#include "Texture.h"
#include "optionparser.h"

#define NB_CAMPAIGN_MENU_ITEM 7

namespace Game
{
extern Texture terraintexture;
extern Texture terraintexture2;
extern Texture loadscreentexture;
extern Texture Maparrowtexture;
extern Texture Mapboxtexture;
extern Texture Mapcircletexture;
extern Texture cursortexture;
extern GLuint screentexture;
extern GLuint screentexture2;
extern Texture Mainmenuitems[10];

extern int selected;
extern int keyselect;

extern int newdetail;
extern int newscreenwidth;
extern int newscreenheight;

extern bool gameon;
extern float deltah, deltav;
extern int mousecoordh, mousecoordv;
extern int oldmousecoordh, oldmousecoordv;
extern float yaw, pitch;
extern SkyBox *skybox;
extern bool cameramode;
extern bool firstload;

extern float leveltime;
extern float loadtime;

extern Model hawk;
extern XYZ hawkcoords;
extern XYZ realhawkcoords;
extern Texture hawktexture;
extern float hawkyaw;
extern float hawkcalldelay;

extern Model eye;
extern Model iris;
extern Model cornea;

extern bool stealthloading;
extern int loading;

extern int musictype;

extern XYZ mapcenter;
extern float mapradius;

extern Text *text;
extern float fps;

extern bool editorenabled;
extern int editortype;
extern float editorsize;
extern float editoryaw;
extern float editorpitch;

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
extern std::string consoletext[15];
extern std::string displaytext[15];
extern float displaytime[15];
extern float displayblinkdelay;
extern bool displayblink;
extern unsigned displayselected;
extern float consoleblinkdelay;
extern bool consoleblink;
extern unsigned consoleselected;

extern int oldenvironment;
extern int targetlevel;
extern float changedelay;

extern bool waiting;
extern Account* accountactive;

extern unsigned short crouchkey, jumpkey, forwardkey, backkey, leftkey, rightkey, drawkey, throwkey, attackkey;
extern unsigned short consolekey;

void newGame();
void deleteGame();

void InitGame();
void LoadStuff();
void LoadScreenTexture();
void LoadingScreen();
int DrawGLScene(StereoSide side);
void LoadMenu();
void playdialoguescenesound();
int findClosestPlayer();
void Loadlevel(int which);
void Loadlevel(const std::string& name);
void Tick();
void TickOnce();
void TickOnceAfter();
void SetUpLighting();
GLvoid ReSizeGLScene(float fov, float near);
int checkcollide(XYZ startpoint, XYZ endpoint);
int checkcollide(XYZ startpoint, XYZ endpoint, int what);

void fireSound(int sound = fireendsound);
void setKeySelected();

void inputText(std::string& str, unsigned* charselected);
void flash(float amount = 1, int delay = 1);
}

#ifndef __forceinline
#  ifdef __GNUC__
#    define __forceinline inline __attribute__((always_inline))
#  endif
#endif

static __forceinline void swap_gl_buffers(void)
{
    extern SDL_Window *sdlwindow;
    SDL_GL_SwapWindow(sdlwindow);

    // try to limit this to 60fps, even if vsync fails.
    Uint32 now;
    static Uint32 frameticks = 0;
    const Uint32 endticks = (frameticks + 16);
    while ((now = SDL_GetTicks()) < endticks) { /* spin. */ }
    frameticks = now;
}

extern "C" {
    void UndefinedSymbolToExposeStubbedCode(void);
}
//#define STUBBED(x) UndefinedSymbolToExposeStubbedCode();
#define STUBBED(x) { static bool seen = false; if (!seen) { seen = true; fprintf(stderr, "STUBBED: %s at %s:%d\n", x, __FILE__, __LINE__); } }
//#define STUBBED(x)

enum maptypes {
    mapkilleveryone, mapgosomewhere,
    mapkillsomeone, mapkillmost // These two are unused
};

enum pathtypes {wpkeepwalking, wppause};

extern const char *pathtypenames[2];

enum editortypes {typeactive, typesitting, typesittingwall, typesleeping,
                  typedead1, typedead2, typedead3, typedead4
                 };

extern const char *editortypenames[8];

extern const char *rabbitskin[10];

extern const char *wolfskin[3];

extern const char **creatureskin[2];

SDL_bool sdlEventProc(const SDL_Event &e);



enum  optionIndex { UNKNOWN, HELP, FULLSCREEN, NOMOUSEGRAB, SOUND, OPENALINFO, SHOWRESOLUTIONS };
/* Number of options + 1 */
const int commandLineOptionsNumber = 8;

extern const option::Descriptor usage[12];

extern option::Option commandLineOptions[commandLineOptionsNumber];
extern option::Option* commandLineOptionsBuffer;

#endif
