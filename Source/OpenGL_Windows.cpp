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

#ifdef WIN32
#define UINT8 WIN32API_UINT8
#define UINT16 WIN32API_UINT16
#define boolean WIN32API_boolean
#include <windows.h>
#undef UINT8
#undef UINT16
#undef boolean
#endif

#define USE_DEVIL 0

#ifndef USE_DEVIL
#  ifdef WIN32
#    define USE_DEVIL
#  endif
#endif

#if USE_DEVIL
    #include "IL/il.h"
    #include "IL/ilu.h"
    #include "IL/ilut.h"
	#include "Game.h"
#else

	#include "Game.h"
	extern "C" {
		#include "zlib.h"
		#include "png.h"
		#include "jpeglib.h"
	}

    static bool load_image(const char * fname, TGAImageRec & tex);
    static bool load_png(const char * fname, TGAImageRec & tex);
    static bool load_jpg(const char * fname, TGAImageRec & tex);
    static bool save_image(const char * fname);
    static bool save_png(const char * fname);
#endif

// ADDED GWC
#ifdef _MSC_VER
#pragma comment(lib, "opengl32.lib")
#pragma comment(lib, "glu32.lib")
#pragma comment(lib, "glaux.lib")
#endif

extern bool buttons[3];
extern float multiplier;
extern float screenwidth,screenheight;
extern float sps;
extern float realmultiplier;
extern int slomo;
extern bool ismotionblur;
extern float usermousesensitivity;
extern int detail;
extern bool floatjump;
extern bool cellophane;
// MODIFIED GWC
//extern int terraindetail;
//extern int texdetail;
extern float terraindetail;
extern float texdetail;
extern int bloodtoggle;
extern bool osx;
extern bool autoslomo;
extern bool foliage;
extern bool musictoggle;
extern bool trilinear;
extern float gamespeed;
extern int difficulty;
extern bool damageeffects;
extern int numplayers;
extern bool decals;
extern bool invertmouse;
extern bool texttoggle;
extern bool ambientsound;
extern bool mousejump;
extern bool freeze;
extern Person player[maxplayers];
extern bool vblsync;
extern bool stillloading;
extern bool showpoints;
extern bool alwaysblur;
extern bool immediate;
extern bool velocityblur;
extern bool debugmode;
extern int mainmenu;
/*extern*/ bool gameFocused;
extern int kBitsPerPixel;
extern float slomospeed;
extern float slomofreq;
extern float oldgamespeed;
extern float volume;

#include <math.h>
#include <stdio.h>
#include <string.h>
#include <fstream>
#include <iostream>
#include "gamegl.h"
#include "MacCompatibility.h"

#ifdef WIN32
#include <shellapi.h>
#endif

#include "fmod.h"

#include "res/resource.h"

using namespace std;


#if USE_SDL
SDL_Rect **resolutions = NULL;
static SDL_Rect rect_1024_768 = { 0, 0, 1024, 768 };
static SDL_Rect rect_800_600  = { 0, 0, 800,  600 };
static SDL_Rect rect_640_480  = { 0, 0, 640,  480 };
static SDL_Rect *hardcoded_resolutions[] = {
    &rect_1024_768,
    &rect_800_600,
    &rect_640_480,
    NULL
};
#endif


unsigned int resolutionDepths[8][2] = {0};

bool selectDetail(int & width, int & height, int & bpp, int & detail);
int closestResolution(int width, int height);
int resolutionID(int width, int height);

void ReportError (char * strError);

void SetupDSpFullScreen();
void ShutdownDSp();

void DrawGL(Game & game);

void CreateGLWindow (void);
Boolean SetUp (Game & game);
void DoKey (SInt8 theKey, SInt8 theCode);
void DoUpdate (Game & game);

void DoEvent (void);
void CleanUp (void);


// statics/globals (internal only) ------------------------------------------
#ifndef WIN32
typedef struct tagPOINT { 
  int x;
  int y;
} POINT, *PPOINT; 
#endif

#if USE_SDL

#ifdef _MSC_VER
#pragma warning(push)
#pragma warning(disable: 4273)
#endif

#define GL_FUNC(ret,fn,params,call,rt) \
    extern "C" { \
        static ret (GLAPIENTRY *p##fn) params = NULL; \
        ret GLAPIENTRY fn params { rt p##fn call; } \
    }
#include "glstubs.h"
#undef GL_FUNC

#ifdef _MSC_VER
#pragma warning(pop)
#endif

static bool lookup_glsym(const char *funcname, void **func)
{
    *func = SDL_GL_GetProcAddress(funcname);
    if (*func == NULL)
    {
        fprintf(stderr, "Failed to find OpenGL symbol \"%s\"\n", funcname);
        return false;
    }
    return true;
}

static bool lookup_all_glsyms(void)
{
    bool retval = true;
    #define GL_FUNC(ret,fn,params,call,rt) \
        if (!lookup_glsym(#fn, (void **) &p##fn)) retval = false;
    #include "glstubs.h"
    #undef GL_FUNC
    return retval;
}

static void GLAPIENTRY glDeleteTextures_doNothing(GLsizei n, const GLuint *textures)
{
    // no-op.
}



void sdlGetCursorPos(POINT *pt)
{
    int x, y;
    SDL_GetMouseState(&x, &y);
    pt->x = x;
    pt->y = y;
}
#define GetCursorPos(x) sdlGetCursorPos(x)
#define SetCursorPos(x, y) SDL_WarpMouse(x, y)
#define ScreenToClient(x, pt)
#define ClientToScreen(x, pt)
#ifdef MessageBox
#undef MessageBox
#endif
#define MessageBox(hwnd,text,title,flags) STUBBED("msgbox")
#endif

Point delta;

static bool g_button, fullscreen = true;


// Menu defs
enum 
{
	kFileQuit = 1
};

enum 
{
	kForegroundSleep = 10,
	kBackgroundSleep = 10000
};


int kContextWidth;
int kContextHeight;

const RGBColor rgbBlack = { 0x0000, 0x0000, 0x0000 };

GLuint gFontList;
char gcstrMode [256] = "";

UInt32 gSleepTime = kForegroundSleep;
Boolean gDone = false, gfFrontProcess = true;

Game * pgame = 0;

#ifndef __MINGW32__
static int _argc = 0;
static char **_argv = NULL;
#endif

bool cmdline(const char *cmd)
{
    for (int i = 1; i < _argc; i++)
    {
        char *arg = _argv[i];
        while (*arg == '-')
            arg++;
        if (strcasecmp(arg, cmd) == 0)
            return true;
    }

    return false;
}


// --------------------------------------------------------------------------

void ReportError (char * strError)
{
#ifdef _MSC_VER  // !!! FIXME.  --ryan.
	throw std::exception( strError);
#endif

	/*	char errMsgCStr [256];
	Str255 strErr;

	sprintf (errMsgCStr, "%s", strError); 

	// out as debug string
	CToPStr (strErr, errMsgCStr);
	DebugStr (strErr);
	*/
}

void SetupDSpFullScreen ()
{
}


void ShutdownDSp ()
{
}


//-----------------------------------------------------------------------------------------------------------------------

// OpenGL Drawing

void DrawGL (Game & game)
{
	game.DrawGLScene();
}


static KeyMap g_theKeys;

void SetKey( int key)
{
    g_theKeys[ key >> 3] |= (1 << (key & 7));
}

void ClearKey( int key)
{
    g_theKeys[ key >> 3] &= (0xff ^ (1 << (key & 7)));
}

void GetKeys(  unsigned char theKeys[16])
{
    memcpy( theKeys, &g_theKeys, 16);
}

Boolean Button()
{
    return g_button;
}

#if !USE_SDL
static void initSDLKeyTable(void) {}
#else
#define MAX_SDLKEYS SDLK_LAST
static unsigned short KeyTable[MAX_SDLKEYS];

static void initSDLKeyTable(void)
{
    memset(KeyTable, 0xFF, sizeof (KeyTable));
    KeyTable[SDLK_BACKSPACE] = MAC_DELETE_KEY;
    KeyTable[SDLK_TAB] = MAC_TAB_KEY;
    KeyTable[SDLK_RETURN] = MAC_RETURN_KEY;
    KeyTable[SDLK_ESCAPE] = MAC_ESCAPE_KEY;
    KeyTable[SDLK_SPACE] = MAC_SPACE_KEY;
    KeyTable[SDLK_PAGEUP] = MAC_PAGE_UP_KEY;
    KeyTable[SDLK_PAGEDOWN] = MAC_PAGE_DOWN_KEY;
    KeyTable[SDLK_END] = MAC_END_KEY;
    KeyTable[SDLK_HOME] = MAC_HOME_KEY;
    KeyTable[SDLK_LEFT] = MAC_ARROW_LEFT_KEY;
    KeyTable[SDLK_UP] = MAC_ARROW_UP_KEY;
    KeyTable[SDLK_RIGHT] = MAC_ARROW_RIGHT_KEY;
    KeyTable[SDLK_DOWN] = MAC_ARROW_DOWN_KEY;
    KeyTable[SDLK_INSERT] = MAC_INSERT_KEY;
    KeyTable[SDLK_DELETE] = MAC_DEL_KEY;
    KeyTable[SDLK_0] = MAC_0_KEY;
    KeyTable[SDLK_1] = MAC_1_KEY;
    KeyTable[SDLK_2] = MAC_2_KEY;
    KeyTable[SDLK_3] = MAC_3_KEY;
    KeyTable[SDLK_4] = MAC_4_KEY;
    KeyTable[SDLK_5] = MAC_5_KEY;
    KeyTable[SDLK_6] = MAC_6_KEY;
    KeyTable[SDLK_7] = MAC_7_KEY;
    KeyTable[SDLK_8] = MAC_8_KEY;
    KeyTable[SDLK_9] = MAC_9_KEY;
    KeyTable[SDLK_a] = MAC_A_KEY;
    KeyTable[SDLK_b] = MAC_B_KEY;
    KeyTable[SDLK_c] = MAC_C_KEY;
    KeyTable[SDLK_d] = MAC_D_KEY;
    KeyTable[SDLK_e] = MAC_E_KEY;
    KeyTable[SDLK_f] = MAC_F_KEY;
    KeyTable[SDLK_g] = MAC_G_KEY;
    KeyTable[SDLK_h] = MAC_H_KEY;
    KeyTable[SDLK_i] = MAC_I_KEY;
    KeyTable[SDLK_j] = MAC_J_KEY;
    KeyTable[SDLK_k] = MAC_K_KEY;
    KeyTable[SDLK_l] = MAC_L_KEY;
    KeyTable[SDLK_m] = MAC_M_KEY;
    KeyTable[SDLK_n] = MAC_N_KEY;
    KeyTable[SDLK_o] = MAC_O_KEY;
    KeyTable[SDLK_p] = MAC_P_KEY;
    KeyTable[SDLK_q] = MAC_Q_KEY;
    KeyTable[SDLK_r] = MAC_R_KEY;
    KeyTable[SDLK_s] = MAC_S_KEY;
    KeyTable[SDLK_t] = MAC_T_KEY;
    KeyTable[SDLK_u] = MAC_U_KEY;
    KeyTable[SDLK_v] = MAC_V_KEY;
    KeyTable[SDLK_w] = MAC_W_KEY;
    KeyTable[SDLK_x] = MAC_X_KEY;
    KeyTable[SDLK_y] = MAC_Y_KEY;
    KeyTable[SDLK_z] = MAC_Z_KEY;
    KeyTable[SDLK_KP0] = MAC_NUMPAD_0_KEY;
    KeyTable[SDLK_KP1] = MAC_NUMPAD_1_KEY;
    KeyTable[SDLK_KP2] = MAC_NUMPAD_2_KEY;
    KeyTable[SDLK_KP3] = MAC_NUMPAD_3_KEY;
    KeyTable[SDLK_KP4] = MAC_NUMPAD_4_KEY;
    KeyTable[SDLK_KP5] = MAC_NUMPAD_5_KEY;
    KeyTable[SDLK_KP6] = MAC_NUMPAD_6_KEY;
    KeyTable[SDLK_KP7] = MAC_NUMPAD_7_KEY;
    KeyTable[SDLK_KP8] = MAC_NUMPAD_8_KEY;
    KeyTable[SDLK_KP9] = MAC_NUMPAD_9_KEY;
    KeyTable[SDLK_KP_MULTIPLY] = MAC_NUMPAD_ASTERISK_KEY;
    KeyTable[SDLK_KP_PLUS] = MAC_NUMPAD_PLUS_KEY;
    KeyTable[SDLK_KP_ENTER] = MAC_NUMPAD_ENTER_KEY;
    KeyTable[SDLK_KP_MINUS] = MAC_NUMPAD_MINUS_KEY;
    KeyTable[SDLK_KP_PERIOD] = MAC_NUMPAD_PERIOD_KEY;
    KeyTable[SDLK_KP_DIVIDE] = MAC_NUMPAD_SLASH_KEY;
    KeyTable[SDLK_F1] = MAC_F1_KEY;
    KeyTable[SDLK_F2] = MAC_F2_KEY;
    KeyTable[SDLK_F3] = MAC_F3_KEY;
    KeyTable[SDLK_F4] = MAC_F4_KEY;
    KeyTable[SDLK_F5] = MAC_F5_KEY;
    KeyTable[SDLK_F6] = MAC_F6_KEY;
    KeyTable[SDLK_F7] = MAC_F7_KEY;
    KeyTable[SDLK_F8] = MAC_F8_KEY;
    KeyTable[SDLK_F9] = MAC_F9_KEY;
    KeyTable[SDLK_F10] = MAC_F10_KEY;
    KeyTable[SDLK_F11] = MAC_F11_KEY;
    KeyTable[SDLK_F12] = MAC_F12_KEY;
    KeyTable[SDLK_SEMICOLON] = MAC_SEMICOLON_KEY;
    KeyTable[SDLK_PLUS] = MAC_PLUS_KEY;
    KeyTable[SDLK_COMMA] = MAC_COMMA_KEY;
    KeyTable[SDLK_MINUS] = MAC_MINUS_KEY;
    KeyTable[SDLK_PERIOD] = MAC_PERIOD_KEY;
    KeyTable[SDLK_SLASH] = MAC_SLASH_KEY;
    KeyTable[SDLK_BACKQUOTE] = MAC_TILDE_KEY;
    KeyTable[SDLK_LEFTBRACKET] = MAC_LEFTBRACKET_KEY;
    KeyTable[SDLK_BACKSLASH] = MAC_BACKSLASH_KEY;
    KeyTable[SDLK_RIGHTBRACKET] = MAC_RIGHTBRACKET_KEY;
    KeyTable[SDLK_QUOTE] = MAC_APOSTROPHE_KEY;
}

static inline int clamp_sdl_mouse_button(Uint8 button)
{
    if (button == 2)   // right mouse button is button 3 in SDL.
        button = 3;
    else if (button == 3)
        button = 2;

    if ((button >= 1) && (button <= 3))
        return button - 1;
    return -1;
}

static void sdlEventProc(const SDL_Event &e, Game &game)
{
    int val;
    bool skipkey = false;
    SDLMod mod;

    switch(e.type)
	{
        case SDL_MOUSEMOTION:
            game.deltah += e.motion.xrel;
            game.deltav += e.motion.yrel;
            return;

		case SDL_MOUSEBUTTONDOWN:
			{
                val = clamp_sdl_mouse_button(e.button.button);
                if ((val >= 0) && (val <= 2))
                {
                    if (val == 0)
    				    g_button = true;
    				buttons[val] = true;
                }
			}
			return;

		case SDL_MOUSEBUTTONUP:
			{
                val = clamp_sdl_mouse_button(e.button.button);
                if ((val >= 0) && (val <= 2))
                {
                    if (val == 0)
    				    g_button = false;
    				buttons[val] = false;
                }
			}
            return;

        case SDL_KEYDOWN:
            if (e.key.keysym.sym == SDLK_g)
            {
                if (e.key.keysym.mod & KMOD_CTRL)
                {
                    skipkey = true;
                    SDL_GrabMode mode = SDL_GRAB_ON;
                    if ((SDL_GetVideoSurface()->flags & SDL_FULLSCREEN) == 0)
                    {
                        mode = SDL_WM_GrabInput(SDL_GRAB_QUERY);
                        mode = (mode==SDL_GRAB_ON) ? SDL_GRAB_OFF:SDL_GRAB_ON;
                    }
                    SDL_WM_GrabInput(mode);
                }
            }

            else if (e.key.keysym.sym == SDLK_RETURN)
            {
                if (e.key.keysym.mod & KMOD_ALT)
                {
                    skipkey = true;
                    SDL_WM_ToggleFullScreen(SDL_GetVideoSurface());
                }
            }

            if ((!skipkey) && (e.key.keysym.sym < SDLK_LAST))
            {
                if (KeyTable[e.key.keysym.sym] != 0xffff)
                    SetKey(KeyTable[e.key.keysym.sym]);
            }

            mod = SDL_GetModState();
            if (mod & KMOD_CTRL)
                SetKey(MAC_CONTROL_KEY);
            if (mod & KMOD_ALT)
                SetKey(MAC_OPTION_KEY);
            if (mod & KMOD_META)
                SetKey(MAC_COMMAND_KEY);
            if (mod & KMOD_SHIFT)
                SetKey(MAC_SHIFT_KEY);
            if (mod & KMOD_CAPS)
                SetKey(MAC_CAPS_LOCK_KEY);

            return;

        case SDL_KEYUP:
            if (e.key.keysym.sym < SDLK_LAST)
            {
                if (KeyTable[e.key.keysym.sym] != 0xffff)
                    ClearKey(KeyTable[e.key.keysym.sym]);
            }

            mod = SDL_GetModState();
            if ((mod & KMOD_CTRL) == 0)
                ClearKey(MAC_CONTROL_KEY);
            if ((mod & KMOD_ALT) == 0)
                ClearKey(MAC_OPTION_KEY);
            if ((mod & KMOD_META) == 0)
                ClearKey(MAC_COMMAND_KEY);
            if ((mod & KMOD_SHIFT) == 0)
                ClearKey(MAC_SHIFT_KEY);
            if ((mod & KMOD_CAPS) == 0)
                ClearKey(MAC_CAPS_LOCK_KEY);
            return;
    }
}
#endif

// --------------------------------------------------------------------------

static Point gMidPoint;

Boolean SetUp (Game & game)
{
	char string[10];

	LOGFUNC;

	randSeed = UpTime().lo;

	osx = 0;
	ifstream ipstream(ConvertFileName(":Data:config.txt"), std::ios::in /*| std::ios::nocreate*/);
	detail=1;
	ismotionblur=0;
	usermousesensitivity=1;
	kContextWidth=640;
	kContextHeight=480;
	kBitsPerPixel = 32;
	floatjump=0;
	cellophane=0;
	texdetail=4;
	autoslomo=1;
	decals=1;
	invertmouse=0;
	bloodtoggle=0;
	terraindetail=2;
	foliage=1;
	musictoggle=1;
	trilinear=1;
	gamespeed=1;
	difficulty=1;
	damageeffects=0;
	texttoggle=1;
	alwaysblur=0;
	showpoints=0;
	immediate=0;
	velocityblur=0;

	slomospeed=0.25;
	slomofreq=8012;

	volume = 0.8f;

	game.crouchkey=MAC_SHIFT_KEY;
	game.jumpkey=MAC_SPACE_KEY;
	game.leftkey=MAC_A_KEY;
	game.forwardkey=MAC_W_KEY;
	game.backkey=MAC_S_KEY;
	game.rightkey=MAC_D_KEY;
	game.drawkey=MAC_E_KEY;
	game.throwkey=MAC_Q_KEY;
	game.attackkey=MAC_MOUSEBUTTON1;
	game.chatkey=MAC_T_KEY;
	numplayers=1;
	ambientsound=1;
	vblsync=0;
	debugmode=0;

	selectDetail(kContextWidth, kContextHeight, kBitsPerPixel, detail);

	if(!ipstream) {
		ofstream opstream(ConvertFileName(":Data:config.txt", "w"));
		opstream << "Screenwidth:\n";
		opstream << kContextWidth;
		opstream << "\nScreenheight:\n";
		opstream << kContextHeight;
		opstream << "\nMouse sensitivity:\n";
		opstream << usermousesensitivity;
		opstream << "\nBlur(0,1):\n";
		opstream << ismotionblur;
		opstream << "\nOverall Detail(0,1,2) higher=better:\n";
		opstream << detail;
		opstream << "\nFloating jump:\n";
		opstream << floatjump;
		opstream << "\nMouse jump:\n";
		opstream << mousejump;
		opstream << "\nAmbient sound:\n";
		opstream << ambientsound;
		opstream << "\nBlood (0,1,2):\n";
		opstream << bloodtoggle;
		opstream << "\nAuto slomo:\n";
		opstream << autoslomo;
		opstream << "\nFoliage:\n";
		opstream << foliage;
		opstream << "\nMusic:\n";
		opstream << musictoggle;
		opstream << "\nTrilinear:\n";
		opstream << trilinear;
		opstream << "\nDecals(shadows,blood puddles,etc):\n";
		opstream << decals;
		opstream << "\nInvert mouse:\n";
		opstream << invertmouse;
		opstream << "\nGamespeed:\n";
		opstream << gamespeed;
		opstream << "\nDifficulty(0,1,2) higher=harder:\n";
		opstream << difficulty;
		opstream << "\nDamage effects(blackout, doublevision):\n";
		opstream << damageeffects;
		opstream << "\nText:\n";
		opstream << texttoggle;
		opstream << "\nDebug:\n";
		opstream << debugmode;
		opstream << "\nVBL Sync:\n";
		opstream << vblsync;
		opstream << "\nShow Points:\n";
		opstream << showpoints;
		opstream << "\nAlways Blur:\n";
		opstream << alwaysblur;
		opstream << "\nImmediate mode (turn on on G5):\n";
		opstream << immediate;
		opstream << "\nVelocity blur:\n";
		opstream << velocityblur;
		opstream << "\nVolume:\n";
		opstream << volume;
		opstream << "\nForward key:\n";
		opstream << KeyToChar(game.forwardkey);
		opstream << "\nBack key:\n";
		opstream << KeyToChar(game.backkey);
		opstream << "\nLeft key:\n";
		opstream << KeyToChar(game.leftkey);
		opstream << "\nRight key:\n";
		opstream << KeyToChar(game.rightkey);
		opstream << "\nJump key:\n";
		opstream << KeyToChar(game.jumpkey);
		opstream << "\nCrouch key:\n";
		opstream << KeyToChar(game.crouchkey);
		opstream << "\nDraw key:\n";
		opstream << KeyToChar(game.drawkey);
		opstream << "\nThrow key:\n";
		opstream << KeyToChar(game.throwkey);
		opstream << "\nAttack key:\n";
		opstream << KeyToChar(game.attackkey);
		opstream << "\nChat key:\n";
		opstream << KeyToChar(game.chatkey);
		opstream.close();
	}
	if(ipstream){
		int i;
		ipstream.ignore(256,'\n');
		ipstream >> kContextWidth;
		ipstream.ignore(256,'\n');
		ipstream.ignore(256,'\n');
		ipstream >> kContextHeight;
		ipstream.ignore(256,'\n');
		ipstream.ignore(256,'\n');
		ipstream >> usermousesensitivity;
		ipstream.ignore(256,'\n');
		ipstream.ignore(256,'\n');
		ipstream >> i;
		ismotionblur = (i != 0);
		ipstream.ignore(256,'\n');
		ipstream.ignore(256,'\n');
		ipstream >> detail;
		if(detail!=0)kBitsPerPixel=32;
		else kBitsPerPixel=16;
		ipstream.ignore(256,'\n');
		ipstream.ignore(256,'\n');
		ipstream >> i;
		floatjump = (i != 0);
		ipstream.ignore(256,'\n');
		ipstream.ignore(256,'\n');
		ipstream >> i;
		mousejump = (i != 0);
		ipstream.ignore(256,'\n');
		ipstream.ignore(256,'\n');
		ipstream >> i;
		ambientsound = (i != 0);
		ipstream.ignore(256,'\n');
		ipstream.ignore(256,'\n');
		ipstream >> bloodtoggle;
		ipstream.ignore(256,'\n');
		ipstream.ignore(256,'\n');
		ipstream >> i;
		autoslomo = (i != 0);
		ipstream.ignore(256,'\n');
		ipstream.ignore(256,'\n');
		ipstream >> i;
		foliage = (i != 0);
		ipstream.ignore(256,'\n');
		ipstream.ignore(256,'\n');
		ipstream >> i;
		musictoggle = (i != 0);
		ipstream.ignore(256,'\n');
		ipstream.ignore(256,'\n');
		ipstream >> i;
		trilinear = (i != 0);
		ipstream.ignore(256,'\n');
		ipstream.ignore(256,'\n');
		ipstream >> i;
		decals = (i != 0);
		ipstream.ignore(256,'\n');
		ipstream.ignore(256,'\n');
		ipstream >> i;
		invertmouse = (i != 0);
		ipstream.ignore(256,'\n');
		ipstream.ignore(256,'\n');
		ipstream >> gamespeed;
		oldgamespeed=gamespeed;
		if(oldgamespeed==0){
			gamespeed=1;
			oldgamespeed=1;
		}
		ipstream.ignore(256,'\n');
		ipstream.ignore(256,'\n');
		ipstream >> difficulty;
		ipstream.ignore(256,'\n');
		ipstream.ignore(256,'\n');
		ipstream >> i;
		damageeffects = (i != 0);
		ipstream.ignore(256,'\n');
		ipstream.ignore(256,'\n');
		ipstream >> i;
		texttoggle = (i != 0);
		ipstream.ignore(256,'\n');
		ipstream.ignore(256,'\n');
		ipstream >> i;
		debugmode = (i != 0);
		ipstream.ignore(256,'\n');
		ipstream.ignore(256,'\n');
		ipstream >> i;
		vblsync = (i != 0);
		ipstream.ignore(256,'\n');
		ipstream.ignore(256,'\n');
		ipstream >> i;
		showpoints = (i != 0);
		ipstream.ignore(256,'\n');
		ipstream.ignore(256,'\n');
		ipstream >> i;
		alwaysblur = (i != 0);
		ipstream.ignore(256,'\n');
		ipstream.ignore(256,'\n');
		ipstream >> i;
		immediate = (i != 0);
		ipstream.ignore(256,'\n');
		ipstream.ignore(256,'\n');
		ipstream >> i;
		velocityblur = (i != 0);
		ipstream.ignore(256,'\n');
		ipstream.ignore(256,'\n'); 
		ipstream >> volume;
		ipstream.ignore(256,'\n');
		ipstream.ignore(256,'\n'); 
		ipstream >> string;
		game.forwardkey=CharToKey(string);
		ipstream.ignore(256,'\n');
		ipstream.ignore(256,'\n');
		ipstream >> string;
		game.backkey=CharToKey(string);
		ipstream.ignore(256,'\n');
		ipstream.ignore(256,'\n');
		ipstream >> string;
		game.leftkey=CharToKey(string);
		ipstream.ignore(256,'\n');
		ipstream.ignore(256,'\n');
		ipstream >> string;
		game.rightkey=CharToKey(string);
		ipstream.ignore(256,'\n');
		ipstream.ignore(256,'\n');
		ipstream >> string;
		game.jumpkey=CharToKey(string);
		ipstream.ignore(256,'\n');
		ipstream.ignore(256,'\n');
		ipstream >> string;
		game.crouchkey=CharToKey(string);
		ipstream.ignore(256,'\n');
		ipstream.ignore(256,'\n');
		ipstream >> string;
		game.drawkey=CharToKey(string);
		ipstream.ignore(256,'\n');
		ipstream.ignore(256,'\n');
		ipstream >> string;
		game.throwkey=CharToKey(string);
		ipstream.ignore(256,'\n');
		ipstream.ignore(256,'\n');
		ipstream >> string;
		game.attackkey=CharToKey(string);
		ipstream.ignore(256,'\n');
		ipstream.ignore(256,'\n');
		ipstream >> string;
		game.chatkey=CharToKey(string);
		ipstream.close();

		if(detail>2)detail=2;
		if(detail<0)detail=0;
		if(screenwidth<0)screenwidth=640;
		if(screenheight<0)screenheight=480;
#if !USE_SDL  // we'll take anything that works.
		if(screenwidth>3000)screenwidth=640;
		if(screenheight>3000)screenheight=480;
#endif
	}
	if(kBitsPerPixel!=32&&kBitsPerPixel!=16){
		kBitsPerPixel=16;
	}


	selectDetail(kContextWidth, kContextHeight, kBitsPerPixel, detail);

	SetupDSpFullScreen();

#if USE_SDL
    if (!SDL_WasInit(SDL_INIT_VIDEO))
    {
        if (SDL_Init(SDL_INIT_VIDEO) == -1)
        {
            fprintf(stderr, "SDL_Init() failed: %s\n", SDL_GetError());
            return false;
        }

        if (SDL_GL_LoadLibrary(NULL) == -1)
        {
            fprintf(stderr, "SDL_GL_LoadLibrary() failed: %s\n", SDL_GetError());
            SDL_Quit();
            return false;
        }

        SDL_Rect **res = SDL_ListModes(NULL, SDL_FULLSCREEN|SDL_OPENGL);
        if ( (res == NULL) || (res == ((SDL_Rect **)-1)) || (res[0] == NULL) || (res[0]->w < 640) || (res[0]->h < 480) )
            res = hardcoded_resolutions;

        // reverse list (it was sorted biggest to smallest by SDL)...
        int count;
        for (count = 0; res[count]; count++)
        {
            if ((res[count]->w < 640) || (res[count]->h < 480))
                break;   // sane lower limit.
        }

        static SDL_Rect *resolutions_block = NULL;
        resolutions_block = (SDL_Rect*) realloc(resolutions_block, sizeof (SDL_Rect) * count);
        resolutions = (SDL_Rect**) realloc(resolutions, sizeof (SDL_Rect *) * (count + 1));
        if ((resolutions_block == NULL) || (resolutions == NULL))
        {
            SDL_Quit();
            fprintf(stderr, "Out of memory!\n");
            return false;
        }

        resolutions[count--] = NULL;
        for (int i = 0; count >= 0; i++, count--)
        {
            memcpy(&resolutions_block[count], res[i], sizeof (SDL_Rect));
            resolutions[count] = &resolutions_block[count];
        }

        if (cmdline("showresolutions"))
        {
            printf("Resolutions we think are okay:\n");
            for (int i = 0; resolutions[i]; i++)
                printf("  %d x %d\n", (int) resolutions[i]->w, (int) resolutions[i]->h);
        }
    }

    Uint32 sdlflags = SDL_OPENGL;
    if (!cmdline("windowed"))
        sdlflags |= SDL_FULLSCREEN;

    SDL_WM_SetCaption("Lugaru", "Lugaru");

    SDL_ShowCursor(0);

    SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER, 1);

    if (SDL_SetVideoMode(kContextWidth, kContextHeight, 0, sdlflags) == NULL)
    {
        fprintf(stderr, "SDL_SetVideoMode() failed: %s\n", SDL_GetError());
        fprintf(stderr, "forcing 640x480...\n");
        kContextWidth = 640;
        kContextHeight = 480;
        if (SDL_SetVideoMode(kContextWidth, kContextHeight, 0, sdlflags) == NULL)
        {
            fprintf(stderr, "SDL_SetVideoMode() failed: %s\n", SDL_GetError());
            fprintf(stderr, "forcing 640x480 windowed mode...\n");
            sdlflags &= ~SDL_FULLSCREEN;
            if (SDL_SetVideoMode(kContextWidth, kContextHeight, 0, sdlflags) == NULL)
            {
                fprintf(stderr, "SDL_SetVideoMode() failed: %s\n", SDL_GetError());
                return false;
            }
        }
    }

    int dblbuf = 0;
    if ((SDL_GL_GetAttribute(SDL_GL_DOUBLEBUFFER, &dblbuf) == -1) || (!dblbuf))
    {
        fprintf(stderr, "Failed to get double buffered GL context!\n");
        SDL_Quit();
        return false;
    }

    if (!lookup_all_glsyms())
    {
        SDL_Quit();
        return false;
    }

    if (!cmdline("nomousegrab"))
        SDL_WM_GrabInput(SDL_GRAB_ON);
#endif

	glClear( GL_COLOR_BUFFER_BIT );
	swap_gl_buffers();

	// clear all states
	glDisable( GL_ALPHA_TEST);
	glDisable( GL_BLEND);
	glDisable( GL_DEPTH_TEST);
	//	glDisable( GL_DITHER);
	glDisable( GL_FOG);
	glDisable( GL_LIGHTING);
	glDisable( GL_LOGIC_OP);
	glDisable( GL_STENCIL_TEST);
	glDisable( GL_TEXTURE_1D);
	glDisable( GL_TEXTURE_2D);
	glPixelTransferi( GL_MAP_COLOR, GL_FALSE);
	glPixelTransferi( GL_RED_SCALE, 1);
	glPixelTransferi( GL_RED_BIAS, 0);
	glPixelTransferi( GL_GREEN_SCALE, 1);
	glPixelTransferi( GL_GREEN_BIAS, 0);
	glPixelTransferi( GL_BLUE_SCALE, 1);
	glPixelTransferi( GL_BLUE_BIAS, 0);
	glPixelTransferi( GL_ALPHA_SCALE, 1);
	glPixelTransferi( GL_ALPHA_BIAS, 0);

	// set initial rendering states
	glShadeModel( GL_SMOOTH);
	glClearDepth( 1.0f);
	glDepthFunc( GL_LEQUAL);
	glDepthMask( GL_TRUE);
	//	glDepthRange( FRONT_CLIP, BACK_CLIP);
	glEnable( GL_DEPTH_TEST);
	glHint( GL_PERSPECTIVE_CORRECTION_HINT, GL_NICEST);
	glCullFace( GL_FRONT);
	glEnable( GL_CULL_FACE);
	glEnable( GL_LIGHTING);
//	glEnable( GL_LIGHT_MODEL_AMBIENT);
	glEnable( GL_DITHER);
	glEnable( GL_COLOR_MATERIAL);
	glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glAlphaFunc( GL_GREATER, 0.5f);

#if USE_DEVIL
	if (ilGetInteger(IL_VERSION_NUM) < IL_VERSION ||
		iluGetInteger(ILU_VERSION_NUM) < ILU_VERSION ||
		ilutGetInteger(ILUT_VERSION_NUM) < ILUT_VERSION)
	{
		ReportError("DevIL version is different...exiting!\n");
		return false;
	}

	ilInit();
	iluInit();
	ilutInit();

	ilutRenderer(ILUT_OPENGL);

	ilEnable(IL_ORIGIN_SET);
	ilOriginFunc(IL_ORIGIN_LOWER_LEFT);
#endif

	GLint width = kContextWidth;
	GLint height = kContextHeight;
	gMidPoint.h = width / 2;
	gMidPoint.v = height / 2;
	screenwidth=width;
	screenheight=height;

	game.newdetail=detail;
	game.newscreenwidth=screenwidth;
	game.newscreenheight=screenheight;

	game.InitGame();

	return true;
}


static void DoMouse(Game & game)
{
#if USE_SDL
	if(mainmenu||(abs(game.deltah)<10*realmultiplier*1000&&abs(game.deltav)<10*realmultiplier*1000))
	{
		game.deltah *= usermousesensitivity;
		game.deltav *= usermousesensitivity;
		game.mousecoordh += game.deltah;
		game.mousecoordv += game.deltav;
        if (game.mousecoordh < 0)
            game.mousecoordh = 0;
        else if (game.mousecoordh >= kContextWidth)
            game.mousecoordh = kContextWidth - 1;
        if (game.mousecoordv < 0)
            game.mousecoordv = 0;
        else if (game.mousecoordv >= kContextHeight)
            game.mousecoordv = kContextHeight - 1;
	}
#endif
}



// --------------------------------------------------------------------------

void DoKey (SInt8 theKey, SInt8 theCode)
{
	// do nothing
}

// --------------------------------------------------------------------------



void DoFrameRate (int update)
{	
	static long frames = 0;

	static AbsoluteTime time = {0,0};
	static AbsoluteTime frametime = {0,0};
	AbsoluteTime currTime = UpTime ();
	double deltaTime = (float) AbsoluteDeltaToDuration (currTime, frametime);

	if (0 > deltaTime)	// if negative microseconds
		deltaTime /= -1000000.0;
	else				// else milliseconds
		deltaTime /= 1000.0;

	multiplier=deltaTime;
	if(multiplier<.001)multiplier=.001;
	if(multiplier>10)multiplier=10;
	if(update)frametime = currTime;	// reset for next time interval

	deltaTime = (float) AbsoluteDeltaToDuration (currTime, time);

	if (0 > deltaTime)	// if negative microseconds
		deltaTime /= -1000000.0;
	else				// else milliseconds
		deltaTime /= 1000.0;
	frames++;
	if (0.001 <= deltaTime)	// has update interval passed
	{
		if(update){
			time = currTime;	// reset for next time interval
			frames = 0;
		}
	}
}


void DoUpdate (Game & game)
{
	static float sps=200;
	static int count;
	static float oldmult;

	DoFrameRate(1);
	if(multiplier>.6)multiplier=.6;

	game.fps=1/multiplier;

	count = multiplier*sps;
	if(count<2)count=2;
	//if(count>10)count=10;

	realmultiplier=multiplier;
	multiplier*=gamespeed;
	if(difficulty==1)multiplier*=.9;
	if(difficulty==0)multiplier*=.8;

	if(game.loading==4)multiplier*=.00001;
	//multiplier*.9;
	if(slomo&&!mainmenu)multiplier*=slomospeed;
	//if(freeze)multiplier*=0.00001;
	oldmult=multiplier;
	multiplier/=(float)count;

	DoMouse(game);

	game.TickOnce();

	for(int i=0;i<count;i++)
	{
		game.Tick();
	}
	multiplier=oldmult;

	game.TickOnceAfter();
/* - Debug code to test how many channels were active on average per frame
	static long frames = 0;

	static AbsoluteTime start = {0,0};
	AbsoluteTime currTime = UpTime ();
	static int num_channels = 0;
	
	num_channels += FSOUND_GetChannelsPlaying();
	double deltaTime = (float) AbsoluteDeltaToDuration (currTime, start);

	if (0 > deltaTime)	// if negative microseconds
		deltaTime /= -1000000.0;
	else				// else milliseconds
		deltaTime /= 1000.0;

	++frames;

	if (deltaTime >= 1)
	{
		start = currTime;
		float avg_channels = (float)num_channels / (float)frames;

		ofstream opstream("log.txt",ios::app); 
		opstream << "Average frame count: ";
		opstream << frames;
		opstream << " frames - ";
		opstream << avg_channels;
		opstream << " per frame.\n";
		opstream.close();

		frames = 0;
		num_channels = 0;
	}
*/
	DrawGL (game);
}

// --------------------------------------------------------------------------


void CleanUp (void)
{
	LOGFUNC;

//	game.Dispose();

#if USE_DEVIL
	ilShutDown();
#endif

#if USE_SDL
    SDL_Quit();
    #define GL_FUNC(ret,fn,params,call,rt) p##fn = NULL;
    #include "glstubs.h"
    #undef GL_FUNC
    // cheat here...static destructors are calling glDeleteTexture() after
    //  the context is destroyed and libGL unloaded by SDL_Quit().
    pglDeleteTextures = glDeleteTextures_doNothing;
#endif
}

// --------------------------------------------------------------------------

static bool g_focused = true;


static bool IsFocused()
{
    STUBBED("write me");
	return true;
}


static void launch_web_browser(const char *url)
{
#ifdef WIN32
    ShellExecute(NULL, "open", url, NULL, NULL, SW_SHOWNORMAL);

#elif (defined(__APPLE__) && defined(__MACH__))
    const char *fmt = "open '%s'";
    const size_t len = strlen(fmt) + strlen(url) + 16;
    char *buf = new char[len];
    snprintf(buf, len, fmt, url);
    system(buf);
    delete[] buf;

#elif PLATFORM_LINUX
    const char *fmt = "PATH=$PATH:. xdg-open '%s'";
    const size_t len = strlen(fmt) + strlen(url) + 16;
    char *buf = new char[len];
    snprintf(buf, len, fmt, url);
    system(buf);
    delete[] buf;
#endif
}


#ifndef WIN32
// (code lifted from physfs: http://icculus.org/physfs/ ... zlib license.)
static char *findBinaryInPath(const char *bin, char *envr)
{
    size_t alloc_size = 0;
    char *exe = NULL;
    char *start = envr;
    char *ptr;

    do
    {
        size_t size;
        ptr = strchr(start, ':');  /* find next $PATH separator. */
        if (ptr)
            *ptr = '\0';

        size = strlen(start) + strlen(bin) + 2;
        if (size > alloc_size)
        {
            char *x = (char *) realloc(exe, size);
            if (x == NULL)
            {
                if (exe != NULL)
                    free(exe);
                return(NULL);
            } /* if */

            alloc_size = size;
            exe = x;
        } /* if */

        /* build full binary path... */
        strcpy(exe, start);
        if ((exe[0] == '\0') || (exe[strlen(exe) - 1] != '/'))
            strcat(exe, "/");
        strcat(exe, bin);

        if (access(exe, X_OK) == 0)  /* Exists as executable? We're done. */
        {
            strcpy(exe, start);  /* i'm lazy. piss off. */
            return(exe);
        } /* if */

        start = ptr + 1;  /* start points to beginning of next element. */
    } while (ptr != NULL);

    if (exe != NULL)
        free(exe);

    return(NULL);  /* doesn't exist in path. */
} /* findBinaryInPath */


char *calcBaseDir(const char *argv0)
{
    /* If there isn't a path on argv0, then look through the $PATH for it. */
    char *retval;
    char *envr;

    char *ptr = strrchr((char *)argv0, '/');
    if (strchr(argv0, '/'))
    {
        retval = strdup(argv0);
        if (retval)
            *(strrchr(retval, '/')) = '\0';
        return(retval);
    }

    envr = getenv("PATH");
    if (!envr) return NULL;
    envr = strdup(envr);
    if (!envr) return NULL;
    retval = findBinaryInPath(argv0, envr);
    free(envr);
    return(retval);
}

static inline void chdirToAppPath(const char *argv0)
{
    char *dir = calcBaseDir(argv0);
    if (dir)
    {
        #if (defined(__APPLE__) && defined(__MACH__))
        // Chop off /Contents/MacOS if it's at the end of the string, so we
        //  land in the base of the app bundle.
        const size_t len = strlen(dir);
        const char *bundledirs = "/Contents/MacOS";
        const size_t bundledirslen = strlen(bundledirs);
        if (len > bundledirslen)
        {
            char *ptr = (dir + len) - bundledirslen;
            if (strcasecmp(ptr, bundledirs) == 0)
                *ptr = '\0';
        }
        #endif
        chdir(dir);
        free(dir);
    }
}
#endif


int main(int argc, char **argv)
{
#ifndef __MINGW32__
    _argc = argc;
    _argv = argv;
#endif

    // !!! FIXME: we could use a Win32 API for this.  --ryan.
#ifndef WIN32
    chdirToAppPath(argv[0]);
#endif

	LOGFUNC;

	memset( &g_theKeys, 0, sizeof( KeyMap));

    initSDLKeyTable();

	try
	{
		bool regnow = false;
		{
			Game game;
			pgame = &game;

			//ofstream os("error.txt");
			//os.close();
			//ofstream os("log.txt");
			//os.close();

			if (!SetUp (game))
                return 42;

			while (!gDone&&!game.quit&&(!game.tryquit||!game.registered))
			{
				if (IsFocused())
				{
					gameFocused = true;

					// check windows messages
					#if USE_SDL
					game.deltah = 0;
					game.deltav = 0;
					SDL_Event e;
					// message pump
					while( SDL_PollEvent( &e ) )
					{
						if( e.type == SDL_QUIT )
						{
							gDone=true;
							break;
						}
						sdlEventProc(e, game);
					}
					#endif

					// game
					DoUpdate(game);
				}
				else
				{
					if (gameFocused)
					{
						// allow game chance to pause
						gameFocused = false;
						DoUpdate(game);
					}

					// game is not in focus, give CPU time to other apps by waiting for messages instead of 'peeking'
                    STUBBED("give up CPU but sniff the event queue");
				}
			}

			regnow = game.registernow;
		}
		pgame = 0;

		CleanUp ();
//		if(game.registernow){
		if(regnow)
		{
            #if (defined(__APPLE__) && defined(__MACH__))
            launch_web_browser("http://www.wolfire.com/purchase/lugaru/mac");
            #elif PLATFORM_LINUX
            launch_web_browser("http://www.wolfire.com/purchase/lugaru/linux");
            #else
            launch_web_browser("http://www.wolfire.com/purchase/lugaru/pc");
            #endif
		}

        #if PLATFORM_LINUX  // (this may not be necessary any more.)
        _exit(0);  // !!! FIXME: hack...crashes on exit!
        #endif
		return 0;
	}
	catch (const std::exception& error)
	{
		CleanUp();

		std::string e = "Caught exception: ";
		e += error.what();

		LOG(e);

		MessageBox(g_windowHandle, error.what(), "ERROR", MB_OK | MB_ICONEXCLAMATION);
	}

	CleanUp();

	return -1;
}



	// --------------------------------------------------------------------------

#if !USE_SDL
	int resolutionID(int width, int height)
	{
		int whichres;
		whichres=-1;
		if(width==640 && height==480)whichres=0;
		if(width==800 && height==600)whichres=1;
		if(width==1024 && height==768)whichres=2;
		if(width==1280 && height==1024)whichres=3;
		if(width==1600 && height==1200)whichres=4;
		if(width==840 && height==524)whichres=5;
		if(width==1024 && height==640)whichres=6;
		if(width==1344 && height==840)whichres=7;
		if(width==1920 && height==1200)whichres=8;

		return whichres;
	}

	int closestResolution(int width, int height)
	{
		int whichres;
		whichres=-1;
		if(width>=640 && height>=480)whichres=0;
		if(width>=800 && height>=600)whichres=1;
		if(width>=1024 && height>=768)whichres=2;
		if(width>=1280 && height>=1024)whichres=3;
		if(width>=1600 && height>=1200)whichres=4;
		if(width==840 && height==524)whichres=5;
		if(width==1024 && height==640)whichres=6;
		if(width==1344 && height==840)whichres=7;
		if(width>=1920 && height>=1200)whichres=8;

		return whichres;
	}
#endif

	bool selectDetail(int & width, int & height, int & bpp, int & detail)
	{
		bool res = true;

		// currently with SDL, we just use whatever is requested
		//  and don't care.  --ryan.
		#if !USE_SDL
		int whichres = closestResolution(width, height);

		while (true)
		{
			if(whichres<=0 || whichres>8){
				whichres = 0;
				width=640;
				height=480;
			}
			if(whichres==1){
				width=800;
				height=600;
			}
			if(whichres==2){
				width=1024;
				height=768;
			}
			if(whichres==3){
				width=1280;
				height=1024;
			}
			if(whichres==4){
				width=1600;
				height=1200;
			}
			if(whichres==5){
				width=840;
				height=524;
			}
			if(whichres==6){
				width=1024;
				height=640;
			}
			if(whichres==7){
				width=1344;
				height=840;
			}
			if(whichres==8){
				width=1920;
				height=1200;
			}

			if ((detail != 0) && (resolutionDepths[whichres][1] != 0))
			{
				break;
			}
			else if ((detail == 0) && (resolutionDepths[whichres][0] != 0))
			{
				break;
			}
			else if ((detail != 0) && (resolutionDepths[whichres][0] != 0))
			{
				res = false;
				detail = 0;
				break;
			}
			else

            if (0 == whichres)
			{
				break;
			}

			--whichres;
		}

		bpp = resolutionDepths[whichres][(detail != 0)];
		#endif

		return res;
	}

	extern int channels[100];
	extern FSOUND_SAMPLE * samp[100];
	extern FSOUND_STREAM * strm[20];

	extern "C" void PlaySoundEx(int chan, FSOUND_SAMPLE *sptr, FSOUND_DSPUNIT *dsp, signed char startpaused)
	{
		const FSOUND_SAMPLE * currSample = FSOUND_GetCurrentSample(channels[chan]);
		if (currSample && currSample == samp[chan])
		{
			if (FSOUND_GetPaused(channels[chan]))
			{
				FSOUND_StopSound(channels[chan]);
				channels[chan] = FSOUND_FREE;
			}
			else if (FSOUND_IsPlaying(channels[chan]))
			{
				int loop_mode = FSOUND_GetLoopMode(channels[chan]);
				if (loop_mode & FSOUND_LOOP_OFF)
				{
					channels[chan] = FSOUND_FREE;
				}
			}
		}
		else
		{
			channels[chan] = FSOUND_FREE;
		}

		channels[chan] = FSOUND_PlaySoundEx(channels[chan], sptr, dsp, startpaused);
		if (channels[chan] < 0)
		{
			channels[chan] = FSOUND_PlaySoundEx(FSOUND_FREE, sptr, dsp, startpaused);
		}
	}

	extern "C" void PlayStreamEx(int chan, FSOUND_STREAM *sptr, FSOUND_DSPUNIT *dsp, signed char startpaused)
	{
		const FSOUND_SAMPLE * currSample = FSOUND_GetCurrentSample(channels[chan]);
		if (currSample && currSample == FSOUND_Stream_GetSample(sptr))
		{
				FSOUND_StopSound(channels[chan]);
				FSOUND_Stream_Stop(sptr);
		}
		else
		{
			FSOUND_Stream_Stop(sptr);
			channels[chan] = FSOUND_FREE;
		}

		channels[chan] = FSOUND_Stream_PlayEx(channels[chan], sptr, dsp, startpaused);
		if (channels[chan] < 0)
		{
			channels[chan] = FSOUND_Stream_PlayEx(FSOUND_FREE, sptr, dsp, startpaused);
		}
	}

	bool LoadImage(const char * fname, TGAImageRec & tex)
	{
		bool res = true;

		if ( tex.data == NULL )
		{
			return false;
		}

        #if USE_DEVIL
		ILstring f = strdup(ConvertFileName(fname));
		if (!f)
		{
			return false;
		}

		ILuint iid=0;
		ilGenImages(1, &iid);
		ilBindImage(iid);
		if (ilLoadImage(f))
		{
			//iluFlipImage();
			tex.sizeX = ilGetInteger(IL_IMAGE_WIDTH);
			tex.sizeY = ilGetInteger(IL_IMAGE_HEIGHT);
			tex.bpp = ilGetInteger(IL_IMAGE_BITS_PER_PIXEL);
			ILuint Bpp = ilGetInteger(IL_IMAGE_BYTES_PER_PIXEL),
				imageSize = tex.sizeX * tex.sizeY * Bpp;
			ILubyte *Data = ilGetData();
			memcpy(tex.data, Data, imageSize);

			// Truvision Targa files are stored as BGR colors
			// We want RGB so Blue and Red bytes are switched
			if (IL_TGA == ilGetInteger(IL_IMAGE_FORMAT))
			{
				// Loop Through The Image Data
				for (GLuint i = 0; i < int(imageSize); i += Bpp)
				{
					// Swaps The 1st And 3rd Bytes ('R'ed and 'B'lue)
					GLbyte temp;						// Temporary Variable
					temp = tex.data[i];					// Temporarily Store The Value At Image Data 'i'
					tex.data[i] = tex.data[i + 2];		// Set The 1st Byte To The Value Of The 3rd Byte
					tex.data[i + 2] = temp;				// Set The 3rd Byte To The Value In 'temp' (1st Byte Value)
				}
			}
		}
		else
		{
			res = false;
		}
		ilDeleteImages(1, &iid);
/*
		if (tid)
		{
			GLuint texid = ilutGLLoadImage(f);
			*tid = texid;
		}
		else if (mip)
		{
			ilutGLBuildMipmaps()
		}
		else
		{
			ilutGLTexImage(0);
		}
*/
		free(f);
        #else
        res = load_image(fname, tex);
        //if (!res) printf("failed to load %s\n", fname);
        #endif

		return res;
	}

	void ScreenShot(const char * fname)
	{
        #if USE_DEVIL
		ILstring f = strdup(fname);
		if (!f)
		{
			return;
		}

		ILuint iid;
		ilGenImages(1, &iid);
		ilBindImage(iid);
		if (ilutGLScreen())
		{
			ilSaveImage(f);
		}
		ilDeleteImages(1, &iid);

		free(f);
        #else
        save_image(fname);
        #endif
	}


#if !USE_DEVIL
static bool load_image(const char *file_name, TGAImageRec &tex)
{
    char *ptr = strrchr((char *)file_name, '.');
    if (ptr)
    {
        if (strcasecmp(ptr+1, "png") == 0)
            return load_png(file_name, tex);
        else if (strcasecmp(ptr+1, "jpg") == 0)
            return load_jpg(file_name, tex);
    }

    STUBBED("Unsupported image type");
    return false;
}


struct my_error_mgr {
  struct jpeg_error_mgr pub;	/* "public" fields */
  jmp_buf setjmp_buffer;	/* for return to caller */
};
typedef struct my_error_mgr * my_error_ptr;


static void my_error_exit(j_common_ptr cinfo)
{
	struct my_error_mgr *err = (struct my_error_mgr *)cinfo->err;
	longjmp(err->setjmp_buffer, 1);
}

/* stolen from public domain example.c code in libjpg distribution. */
static bool load_jpg(const char *file_name, TGAImageRec &tex)
{
    struct jpeg_decompress_struct cinfo;
    struct my_error_mgr jerr;
    JSAMPROW buffer[1];		/* Output row buffer */
    int row_stride;		/* physical row width in output buffer */
    FILE *infile = fopen(file_name, "rb");

    if (infile == NULL)
        return false;

    cinfo.err = jpeg_std_error(&jerr.pub);
    jerr.pub.error_exit = my_error_exit;
    if (setjmp(jerr.setjmp_buffer)) {
        jpeg_destroy_decompress(&cinfo);
        fclose(infile);
        return false;
    }

    jpeg_create_decompress(&cinfo);
    jpeg_stdio_src(&cinfo, infile);
    (void) jpeg_read_header(&cinfo, TRUE);

    cinfo.out_color_space = JCS_RGB;
    cinfo.quantize_colors = 0;
    (void) jpeg_calc_output_dimensions(&cinfo);
    (void) jpeg_start_decompress(&cinfo);

    row_stride = cinfo.output_width * cinfo.output_components;
    tex.sizeX = cinfo.output_width;
    tex.sizeY = cinfo.output_height;
    tex.bpp = 24;

    while (cinfo.output_scanline < cinfo.output_height) {
        buffer[0] = (JSAMPROW)(char *)tex.data +
                        ((cinfo.output_height-1) - cinfo.output_scanline) * row_stride;
        (void) jpeg_read_scanlines(&cinfo, buffer, 1);
    }

    (void) jpeg_finish_decompress(&cinfo);
    jpeg_destroy_decompress(&cinfo);
    fclose(infile);

    return true;
}


/* stolen from public domain example.c code in libpng distribution. */
static bool load_png(const char *file_name, TGAImageRec &tex)
{
    bool hasalpha = false;
    png_structp png_ptr = NULL;
    png_infop info_ptr = NULL;
    png_uint_32 width, height;
    int bit_depth, color_type, interlace_type;
    png_byte **rows = NULL;
    bool retval = false;
    png_byte **row_pointers = NULL;
    FILE *fp = fopen(file_name, "rb");

    if (fp == NULL)
        return(NULL);

    png_ptr = png_create_read_struct(PNG_LIBPNG_VER_STRING, NULL, NULL, NULL);
    if (png_ptr == NULL)
        goto png_done;

    info_ptr = png_create_info_struct(png_ptr);
    if (info_ptr == NULL)
        goto png_done;

    if (setjmp(png_jmpbuf(png_ptr)))
        goto png_done;

    png_init_io(png_ptr, fp);
    png_read_png(png_ptr, info_ptr,
                 PNG_TRANSFORM_STRIP_16 | PNG_TRANSFORM_PACKING,
                 NULL);
    png_get_IHDR(png_ptr, info_ptr, &width, &height,
                 &bit_depth, &color_type, &interlace_type, NULL, NULL);

    if (bit_depth != 8)  // transform SHOULD handle this...
        goto png_done;

    if (color_type & PNG_COLOR_MASK_PALETTE)  // !!! FIXME?
        goto png_done;

    if ((color_type & PNG_COLOR_MASK_COLOR) == 0)  // !!! FIXME?
        goto png_done;

    hasalpha = ((color_type & PNG_COLOR_MASK_ALPHA) != 0);
    row_pointers = png_get_rows(png_ptr, info_ptr);
    if (!row_pointers)
        goto png_done;

    if (!hasalpha)
    {
        png_byte *dst = tex.data;
        for (int i = height-1; i >= 0; i--)
        {
            png_byte *src = row_pointers[i];
            for (int j = 0; j < width; j++)
            {
                dst[0] = src[0];
                dst[1] = src[1];
                dst[2] = src[2];
                dst[3] = 0xFF;
                src += 3;
                dst += 4;
            }
        }
    }

    else
    {
        png_byte *dst = tex.data;
        int pitch = width * 4;
        for (int i = height-1; i >= 0; i--, dst += pitch)
            memcpy(dst, row_pointers[i], pitch);
    }

    tex.sizeX = width;
    tex.sizeY = height;
    tex.bpp = 32;
    retval = true;

png_done:
    png_destroy_read_struct(&png_ptr, &info_ptr, NULL);
    if (fp)
        fclose(fp);
    return (retval);
}


static bool save_image(const char *file_name)
{
    char *ptr = strrchr((char *)file_name, '.');
    if (ptr)
    {
        if (strcasecmp(ptr+1, "png") == 0)
            return save_png(file_name);
    }

    STUBBED("Unsupported image type");
    return false;
}


static bool save_png(const char *file_name)
{
    FILE *fp = NULL;
    png_structp png_ptr = NULL;
    png_infop info_ptr = NULL;
    bool retval = false;

    fp = fopen(file_name, "wb");
    if (fp == NULL)
        return false;

    png_bytep *row_pointers = new png_bytep[kContextHeight];
    png_bytep screenshot = new png_byte[kContextWidth * kContextHeight * 3];
    if ((!screenshot) || (!row_pointers))
        goto save_png_done;

    glGetError();
    glReadPixels(0, 0, kContextWidth, kContextHeight,
                 GL_RGB, GL_UNSIGNED_BYTE, screenshot);
    if (glGetError() != GL_NO_ERROR)
        goto save_png_done;

    for (int i = 0; i < kContextHeight; i++)
        row_pointers[i] = screenshot + ((kContextWidth * ((kContextHeight-1) - i)) * 3);

    png_ptr = png_create_write_struct(PNG_LIBPNG_VER_STRING, NULL, NULL, NULL);
    if (png_ptr == NULL)
        goto save_png_done;

    info_ptr = png_create_info_struct(png_ptr);
    if (info_ptr == NULL)
        goto save_png_done;

    if (setjmp(png_jmpbuf(png_ptr)))
        goto save_png_done;

    png_init_io(png_ptr, fp);

    if (setjmp(png_jmpbuf(png_ptr)))
        goto save_png_done;

    png_set_IHDR(png_ptr, info_ptr, kContextWidth, kContextHeight,
                 8, PNG_COLOR_TYPE_RGB, PNG_INTERLACE_NONE,
                 PNG_COMPRESSION_TYPE_BASE, PNG_FILTER_TYPE_BASE);

    png_write_info(png_ptr, info_ptr);

    if (setjmp(png_jmpbuf(png_ptr)))
        goto save_png_done;

	png_write_image(png_ptr, row_pointers);

	if (setjmp(png_jmpbuf(png_ptr)))
        goto save_png_done;

    png_write_end(png_ptr, NULL);
    retval = true;

save_png_done:
    png_destroy_write_struct(&png_ptr, &info_ptr);
    delete[] screenshot;
    delete[] row_pointers;
    if (fp)
        fclose(fp);
    if (!retval)
        unlink(ConvertFileName(file_name));
    return retval;
}

#endif

