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

#include "Game.hpp"

#include "Audio/openal_wrapper.hpp"
#include "Level/Dialog.hpp"

#include <SDL_thread.h>

extern int mainmenu;

const char* pathtypenames[] = { "keepwalking", "pause" };
const char* editortypenames[] = {
    "active", "sitting", "sitting wall", "sleeping",
    "dead1", "dead2", "dead3", "dead4"
};

namespace Game
{
Texture terraintexture;
Texture terraintexture2;
Texture loadscreentexture;
Texture Maparrowtexture;
Texture Mapboxtexture;
Texture Mapcircletexture;
Texture cursortexture;
GLuint screentexture = 0;
GLuint screentexture2 = 0;
Texture Mainmenuitems[10];

int selected = 0;
int keyselect = 0;

int newdetail = 0;
int newscreenwidth = 0;
int newscreenheight = 0;

bool gameon = 0;
float deltah = 0;
float deltav = 0;
int mousecoordh = 0;
int mousecoordv = 0;
int oldmousecoordh = 0;
int oldmousecoordv = 0;
float yaw = 0;
float pitch = 0;
SkyBox* skybox = NULL;
bool cameramode = 0;
bool firstLoadDone = false;

Texture hawktexture;
float hawkyaw = 0;
float hawkcalldelay = 0;

float leveltime = 0;
float wonleveltime = 0;
float loadtime = 0;

Model hawk;
XYZ hawkcoords;
XYZ realhawkcoords;

Model eye;
Model iris;
Model cornea;

bool stealthloading = 0;

int musictype = 0;

XYZ mapcenter;
float mapradius = 0;

Text* text = NULL;
Text* textmono = NULL;
float fps = 0;

bool editorenabled = 0;
int editortype = 0;
float editorsize = 0;
float editoryaw = 0;
float editorpitch = 0;

int tryquit = 0;

XYZ pathpoint[30];
int numpathpoints = 0;
int numpathpointconnect[30] = {};
int pathpointconnect[30][30] = {};
int pathpointselected = 0;

int endgame = 0;
bool scoreadded = 0;
int numchallengelevels = 0;

bool console = false;
std::string consoletext[15] = {};
float consoleblinkdelay = 0;
bool consoleblink = 0;
unsigned consoleselected = 0;

unsigned short crouchkey = 0, jumpkey = 0, forwardkey = 0, backkey = 0, leftkey = 0, rightkey = 0, drawkey = 0, throwkey = 0, attackkey = 0;
unsigned short consolekey = 0;

int loading = 0;

int oldenvironment = 0;
int targetlevel = 0;
float changedelay = 0;

bool waiting = false;
}

void Game::fireSound(int sound)
{
    emit_sound_at(sound);
}

void Game::inputText(string& str, unsigned* charselected)
{
    SDL_Event evenement;

    if (!waiting) {
        SDL_StartTextInput();
        waiting = true;
    }

    while (SDL_PollEvent(&evenement)) {
        if (!sdlEventProc(evenement)) {
            tryquit = 1;
            break;
        }
        switch (evenement.type) {
            case SDL_TEXTEDITING:
                /* FIXME - We should handle this for complete input method support */
                break;
            case SDL_TEXTINPUT:
                str.insert(*charselected, evenement.text.text);
                (*charselected) += strlen(evenement.text.text);
                break;
            case SDL_KEYDOWN:
                if (evenement.key.keysym.sym == SDLK_ESCAPE) {
                    str.clear();
                    *charselected = 0;
                    waiting = false;
                } else if (evenement.key.keysym.sym == SDLK_BACKSPACE) {
                    if ((*charselected) > 0) {
                        (*charselected)--;
                        str.erase(*charselected, 1);
                    }
                } else if (evenement.key.keysym.sym == SDLK_DELETE) {
                    if ((*charselected) < str.size()) {
                        str.erase(*charselected, 1);
                    }
                } else if (evenement.key.keysym.sym == SDLK_HOME) {
                    (*charselected) = 0;
                } else if (evenement.key.keysym.sym == SDLK_END) {
                    (*charselected) = str.size();
                } else if (evenement.key.keysym.sym == SDLK_LEFT) {
                    if ((*charselected) != 0) {
                        (*charselected)--;
                    }
                } else if (evenement.key.keysym.sym == SDLK_RIGHT) {
                    if ((*charselected) < str.size()) {
                        (*charselected)++;
                    }
                } else if (evenement.key.keysym.sym == SDLK_RETURN) {
                    waiting = false;
                }
                break;
        }
    }

    if (!waiting) {
        SDL_StopTextInput();
    }
}
