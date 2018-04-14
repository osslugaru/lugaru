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

#ifndef _GAME_HPP_
#define _GAME_HPP_

#include "Animation/Skeleton.hpp"
#include "Audio/Sounds.hpp"
#include "Environment/Lights.hpp"
#include "Environment/Skybox.hpp"
#include "Environment/Terrain.hpp"
#include "Graphic/Models.hpp"
#include "Graphic/Sprite.hpp"
#include "Graphic/Stereo.hpp"
#include "Graphic/Text.hpp"
#include "Graphic/Texture.hpp"
#include "Graphic/gamegl.hpp"
#include "Objects/Object.hpp"
#include "Objects/Person.hpp"
#include "Objects/Weapons.hpp"
#include "Thirdparty/optionparser.h"
#include "User/Account.hpp"
#include "Utils/ImageIO.hpp"
#include "Utils/binio.h"

#include <SDL.h>
#include <fstream>

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
extern SkyBox* skybox;
extern bool cameramode;
extern bool firstLoadDone;

extern float leveltime;
extern float wonleveltime;
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

extern Text* text;
extern Text* textmono;
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
extern float consoleblinkdelay;
extern bool consoleblink;
extern unsigned consoleselected;

extern int oldenvironment;
extern int targetlevel;
extern float changedelay;

extern bool waiting;

extern unsigned short crouchkey, jumpkey, forwardkey, backkey, leftkey, rightkey, drawkey, throwkey, attackkey;
extern unsigned short consolekey;

void newGame();
void deleteGame();

void InitGame();
void LoadStuff();
void LoadScreenTexture();
void LoadingScreen();
int DrawGLScene(StereoSide side);
void playdialoguescenesound();
int findClosestPlayer();
void ResetBeforeLevelLoad(bool tutorial);
bool LoadLevel(int which);
bool LoadLevel(const std::string& name, bool tutorial = false);
bool LoadJsonLevel(const std::string& name, bool tutorial = false);

void cmd_dispatch(const string cmd);

void ProcessInput();
void ProcessDevInput();

void Tick();
void TickOnce();
void TickOnceAfter();

void SetUpLighting();
GLvoid ReSizeGLScene(float fov, float near);

void fireSound(int sound = fireendsound);

void inputText(std::string& str, unsigned* charselected);
void flash(float amount = 1, int delay = 1);
}
float roughDirection(XYZ vec);
float roughDirectionTo(XYZ start, XYZ end);
float pitchTo(XYZ start, XYZ end);
float sq(float n);

#ifndef __forceinline
#ifdef __GNUC__
#define __forceinline inline __attribute__((always_inline))
#endif
#endif

static __forceinline void swap_gl_buffers(void)
{
    extern SDL_Window* sdlwindow;
    SDL_GL_SwapWindow(sdlwindow);

    // try to limit this to 60fps, even if vsync fails.
    Uint32 now;
    static Uint32 frameticks = 0;
    const Uint32 endticks = (frameticks + 16);
    while ((now = SDL_GetTicks()) < endticks) { /* spin. */
    }
    frameticks = now;
}

enum maptypes
{
    mapkilleveryone,
    mapgosomewhere,
    mapkillsomeone,
    mapkillmost // These two are unused
};

enum pathtypes
{
    wpkeepwalking,
    wppause
};

extern const char* pathtypenames[2];

enum editortypes
{
    typeactive,
    typesitting,
    typesittingwall,
    typesleeping,
    typedead1,
    typedead2,
    typedead3,
    typedead4
};

extern const char* editortypenames[8];

SDL_bool sdlEventProc(const SDL_Event& e);

enum optionIndex
{
    UNKNOWN,
    VERSION,
    HELP,
    FULLSCREEN,
    NOMOUSEGRAB,
    SOUND,
    OPENALINFO,
    SHOWRESOLUTIONS,
    DEVTOOLS,
    CMD
};
/* Number of options + 1 */
const int commandLineOptionsNumber = 11;

extern const option::Descriptor usage[];

extern option::Option commandLineOptions[commandLineOptionsNumber];
extern option::Option* commandLineOptionsBuffer;

#endif
