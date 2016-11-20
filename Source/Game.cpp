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

#include "Game.h"
#include "openal_wrapper.h"
#include "SDL_thread.h"

extern int mainmenu;

int numdialogues;
int numdialogueboxes[max_dialogues];
int dialoguetype[max_dialogues];
int dialogueboxlocation[max_dialogues][max_dialoguelength];
float dialogueboxcolor[max_dialogues][max_dialoguelength][3];
int dialogueboxsound[max_dialogues][max_dialoguelength];
char dialoguetext[max_dialogues][max_dialoguelength][128];
char dialoguename[max_dialogues][max_dialoguelength][64];
XYZ dialoguecamera[max_dialogues][max_dialoguelength];
XYZ participantlocation[max_dialogues][10];
int participantfocus[max_dialogues][max_dialoguelength];
int participantaction[max_dialogues][max_dialoguelength];
float participantyaw[max_dialogues][10];
XYZ participantfacing[max_dialogues][max_dialoguelength][10];
float dialoguecamerayaw[max_dialogues][max_dialoguelength];
float dialoguecamerapitch[max_dialogues][max_dialoguelength];
int indialogue;
int whichdialogue;
int directing;
float dialoguetime;
int dialoguegonethrough[20];

const char *pathtypenames[] = {"keepwalking", "pause"};
const char *editortypenames[] = {
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
SkyBox *skybox = NULL;
bool cameramode = 0;
bool firstload = 0;

Texture hawktexture;
float hawkyaw = 0;
float hawkcalldelay = 0;
float leveltime = 0;
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

Text *text = NULL;
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
std::string displaytext[15] = {};
float displaytime[15] = {};
float displayblinkdelay = 0;
bool displayblink = 0;
int displayselected = 0;
float consoleblinkdelay = 0;
bool consoleblink = 0;
int consoleselected = 0;

unsigned short crouchkey = 0, jumpkey = 0, forwardkey = 0, backkey = 0, leftkey = 0, rightkey = 0, drawkey = 0, throwkey = 0, attackkey = 0;
unsigned short consolekey = 0;

int loading = 0;

int oldenvironment = 0;
int targetlevel = 0;
float changedelay = 0;

bool waiting = false;
Account* accountactive = NULL;
}

void Game::fireSound(int sound)
{
    emit_sound_at(sound);
}

void Game::inputText(string& str, int* charselected)
{
    SDL_Event evenement;

    if (!waiting) {
        SDL_StartTextInput();
        waiting = true;
    }

    while (SDL_PollEvent(&evenement)) {
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
                if ((*charselected) != 0)
                    (*charselected)--;
            } else if (evenement.key.keysym.sym == SDLK_RIGHT) {
                if ((*charselected) < str.size())
                    (*charselected)++;
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

int setKeySelected_thread(void* data)
{
    using namespace Game;
    int scancode = -1;
    SDL_Event evenement;
    while (scancode == -1) {
        SDL_WaitEvent(&evenement);
        switch (evenement.type) {
        case SDL_KEYDOWN:
            scancode = evenement.key.keysym.scancode;
            break;
        case SDL_MOUSEBUTTONDOWN:
            scancode = SDL_NUM_SCANCODES + evenement.button.button;
            break;
        default:
            break;
        }
    }
    if (scancode != SDL_SCANCODE_ESCAPE) {
        fireSound();
        switch (keyselect) {
        case 0:
            forwardkey = scancode;
            break;
        case 1:
            backkey = scancode;
            break;
        case 2:
            leftkey = scancode;
            break;
        case 3:
            rightkey = scancode;
            break;
        case 4:
            crouchkey = scancode;
            break;
        case 5:
            jumpkey = scancode;
            break;
        case 6:
            drawkey = scancode;
            break;
        case 7:
            throwkey = scancode;
            break;
        case 8:
            attackkey = scancode;
            break;
        case 9:
            consolekey = scancode;
            break;
        default:
            break;
        }
    }
    keyselect = -1;
    waiting = false;
    LoadMenu();
    return 0;
}

void Game::setKeySelected()
{
    waiting = true;
    printf("launch thread\n");
    SDL_Thread* thread = SDL_CreateThread(setKeySelected_thread, NULL, NULL);
    if ( thread == NULL ) {
        fprintf(stderr, "Unable to create thread: %s\n", SDL_GetError());
        waiting = false;
        return;
    }
}
