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

#include "User/Settings.hpp"

#include "Game.hpp"
#include "Utils/Folders.hpp"
#include "Utils/Input.hpp"

using namespace Game;

void DefaultSettings()
{
    detail = 2;
    ismotionblur = 1;
    usermousesensitivity = 1;
    newscreenwidth = kContextWidth = 1024;
    newscreenheight = kContextHeight = 768;
    fullscreen = 0;
    floatjump = 0;
    autoslomo = 1;
    decalstoggle = true;
    invertmouse = 0;
    bloodtoggle = 0;
    foliage = 1;
    musictoggle = 1;
    trilinear = 1;
    gamespeed = 1;
    damageeffects = 0;
    texttoggle = 1;
    alwaysblur = 0;
    showpoints = 0;
    showdamagebar = 0;
    immediate = 0;
    velocityblur = 0;
    volume = 0.8f;
    ambientsound = 1;
    devtools = 0;

    Keys::crouch = SDL_SCANCODE_LSHIFT;
    Keys::jump = SDL_SCANCODE_SPACE;
    Keys::left = SDL_SCANCODE_A;
    Keys::forward = SDL_SCANCODE_W;
    Keys::back = SDL_SCANCODE_S;
    Keys::right = SDL_SCANCODE_D;
    Keys::draw = SDL_SCANCODE_E;
    Keys::throwk = SDL_SCANCODE_Q;
    Keys::attack = MOUSEBUTTON_LEFT;
    Keys::console = SDL_SCANCODE_GRAVE;
    Keys::camright  = SDL_NUM_SCANCODES + 6 + SDL_CONTROLLER_BUTTON_MAX + 2 * SDL_CONTROLLER_AXIS_RIGHTX;
    Keys::camleft   = SDL_NUM_SCANCODES + 6 + SDL_CONTROLLER_BUTTON_MAX + 2 * SDL_CONTROLLER_AXIS_RIGHTX + 1;
    Keys::camup     = SDL_NUM_SCANCODES + 6 + SDL_CONTROLLER_BUTTON_MAX + 2 * SDL_CONTROLLER_AXIS_RIGHTY;
    Keys::camdown   = SDL_NUM_SCANCODES + 6 + SDL_CONTROLLER_BUTTON_MAX + 2 * SDL_CONTROLLER_AXIS_RIGHTY + 1;

    newdetail = detail;
}

void SaveSettings()
{
    if (newdetail < 0) {
        newdetail = 0;
    }
    if (newdetail > 2) {
        newdetail = 2;
    }
    if (newscreenwidth < minscreenwidth || newscreenwidth > maxscreenwidth) {
        newscreenwidth = screenwidth;
    }
    if (newscreenheight < minscreenheight || newscreenheight > maxscreenheight) {
        newscreenheight = screenheight;
    }
    errno = 0;
    ofstream opstream(Folders::getConfigFilePath());
    if (opstream.fail()) {
        perror(("Couldn't save config file " + Folders::getConfigFilePath()).c_str());
        return;
    }
    opstream << "Screenwidth:\n";
    opstream << newscreenwidth;
    opstream << "\nScreenheight:\n";
    opstream << newscreenheight;
    opstream << "\nFullscreen:\n";
    opstream << fullscreen;
    opstream << "\nMouse sensitivity:\n";
    opstream << usermousesensitivity;
    opstream << "\nBlur(0,1):\n";
    opstream << ismotionblur;
    opstream << "\nOverall Detail(0,1,2) higher=better:\n";
    opstream << newdetail;
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
    opstream << decalstoggle;
    opstream << "\nInvert mouse:\n";
    opstream << invertmouse;
    opstream << "\nGamespeed:\n";
    if (oldgamespeed == 0) {
        oldgamespeed = 1;
    }
    opstream << oldgamespeed;
    opstream << "\nDamage effects(blackout, doublevision):\n";
    opstream << damageeffects;
    opstream << "\nText:\n";
    opstream << texttoggle;
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
    opstream << Keys::forward;
    opstream << "\nBack key:\n";
    opstream << Keys::back;
    opstream << "\nLeft key:\n";
    opstream << Keys::left;
    opstream << "\nRight key:\n";
    opstream << Keys::right;
    opstream << "\nJump key:\n";
    opstream << Keys::jump;
    opstream << "\nCrouch key:\n";
    opstream << Keys::crouch;
    opstream << "\nDraw key:\n";
    opstream << Keys::draw;
    opstream << "\nThrow key:\n";
    opstream << Keys::throwk;
    opstream << "\nAttack key:\n";
    opstream << Keys::attack;
    opstream << "\nConsole key:\n";
    opstream << Keys::console;
    opstream << "\nDamage bar:\n";
    opstream << showdamagebar;
    opstream << "\nStereoMode:\n";
    opstream << stereomode;
    opstream << "\nStereoSeparation:\n";
    opstream << stereoseparation;
    opstream << "\nStereoReverse:\n";
    opstream << stereoreverse;
    opstream << "\n";
    opstream.close();
}

bool LoadSettings()
{
    errno = 0;
    ifstream ipstream(Folders::getConfigFilePath(), std::ios::in);
    if (ipstream.fail()) {
        perror(("Couldn't read config file " + Folders::getConfigFilePath()).c_str());
        return false;
    }
    char setting[256];
    char string[256];

    printf("Loading config\n");
    while (!ipstream.eof()) {
        ipstream.getline(setting, sizeof(setting));

        // skip blank lines
        // assume lines starting with spaces are all blank
        if (strlen(setting) == 0 || setting[0] == ' ' || setting[0] == '\t') {
            continue;
        }
        //~ printf("setting : %s\n",setting);

        if (ipstream.eof() || ipstream.fail()) {
            fprintf(stderr, "Error reading config file: Got setting name '%s', but value can't be read\n", setting);
            ipstream.close();
            return false;
        }

        if (!strncmp(setting, "Screenwidth", 11)) {
            ipstream >> kContextWidth;
            if (kContextWidth < (int)minscreenwidth || kContextWidth > (int)maxscreenwidth) {
               kContextWidth = (int)minscreenwidth;
            }
        } else if (!strncmp(setting, "Screenheight", 12)) {
            ipstream >> kContextHeight;
            if (kContextHeight < (int)minscreenheight || kContextHeight > (int)maxscreenheight) {
               kContextHeight = (int)minscreenheight;
            }
        } else if (!strncmp(setting, "Fullscreen", 10)) {
            ipstream >> fullscreen;
        } else if (!strncmp(setting, "Mouse sensitivity", 17)) {
            ipstream >> usermousesensitivity;
        } else if (!strncmp(setting, "Blur", 4)) {
            ipstream >> ismotionblur;
        } else if (!strncmp(setting, "Overall Detail", 14)) {
            ipstream >> detail;
        } else if (!strncmp(setting, "Floating jump", 13)) {
            ipstream >> floatjump;
        } else if (!strncmp(setting, "Mouse jump", 10)) {
            ipstream >> mousejump;
        } else if (!strncmp(setting, "Ambient sound", 13)) {
            ipstream >> ambientsound;
        } else if (!strncmp(setting, "Blood", 5)) {
            ipstream >> bloodtoggle;
        } else if (!strncmp(setting, "Auto slomo", 10)) {
            ipstream >> autoslomo;
        } else if (!strncmp(setting, "Foliage", 7)) {
            ipstream >> foliage;
        } else if (!strncmp(setting, "Music", 5)) {
            ipstream >> musictoggle;
        } else if (!strncmp(setting, "Trilinear", 9)) {
            ipstream >> trilinear;
        } else if (!strncmp(setting, "Decals", 6)) {
            ipstream >> decalstoggle;
        } else if (!strncmp(setting, "Invert mouse", 12)) {
            ipstream >> invertmouse;
        } else if (!strncmp(setting, "Gamespeed", 9)) {
            ipstream >> gamespeed;
            oldgamespeed = gamespeed;
            if (oldgamespeed == 0) {
                gamespeed = 1;
                oldgamespeed = 1;
            }
        } else if (!strncmp(setting, "Damage effects", 14)) {
            ipstream >> damageeffects;
        } else if (!strncmp(setting, "Text", 4)) {
            ipstream >> texttoggle;
        } else if (!strncmp(setting, "Devtools", 8)) {
            ipstream >> devtools;
        } else if (!strncmp(setting, "Show Points", 11)) {
            ipstream >> showpoints;
        } else if (!strncmp(setting, "Always Blur", 11)) {
            ipstream >> alwaysblur;
        } else if (!strncmp(setting, "Immediate mode ", 15)) {
            ipstream >> immediate;
        } else if (!strncmp(setting, "Velocity blur", 13)) {
            ipstream >> velocityblur;
        } else if (!strncmp(setting, "Volume", 6)) {
            ipstream >> volume;
        } else if (!strncmp(setting, "Forward key", 11)) {
            ipstream >> Keys::forward;
        } else if (!strncmp(setting, "Back key", 8)) {
            ipstream >> Keys::back;
        } else if (!strncmp(setting, "Left key", 8)) {
            ipstream >> Keys::left;
        } else if (!strncmp(setting, "Right key", 9)) {
            ipstream >> Keys::right;
        } else if (!strncmp(setting, "Jump key", 8)) {
            ipstream >> Keys::jump;
        } else if (!strncmp(setting, "Crouch key", 10)) {
            ipstream >> Keys::crouch;
        } else if (!strncmp(setting, "Draw key", 8)) {
            ipstream >> Keys::draw;
        } else if (!strncmp(setting, "Throw key", 9)) {
            ipstream >> Keys::throwk;
        } else if (!strncmp(setting, "Attack key", 10)) {
            ipstream >> Keys::attack;
        } else if (!strncmp(setting, "Console key", 11)) {
            ipstream >> Keys::console;
        } else if (!strncmp(setting, "Damage bar", 10)) {
            ipstream >> showdamagebar;
        } else if (!strncmp(setting, "StereoMode", 10)) {
            int i;
            ipstream >> i;
            stereomode = (StereoMode)i;
        } else if (!strncmp(setting, "StereoSeparation", 16)) {
            ipstream >> stereoseparation;
        } else if (!strncmp(setting, "StereoReverse", 13)) {
            ipstream >> stereoreverse;
        } else {
            ipstream >> string;
            fprintf(stderr, "Unknown config option '%s' with value '%s'. Ignoring.\n", setting, string);
        }

        if (ipstream.fail()) {
            fprintf(stderr, "Error reading config file: EOF reached when trying to read value for setting '%s'.\n", setting);
            ipstream.close();
            return false;
        }

        if (ipstream.bad()) {
            fprintf(stderr, "Error reading config file: Failed to read value for setting '%s'.\n", setting);
            ipstream.close();
            return false;
        }
    }

    ipstream.close();

    if (detail > 2) {
        detail = 2;
    }
    if (detail < 0) {
        detail = 0;
    }
    if (screenwidth < minscreenwidth || screenwidth > maxscreenwidth) {
        screenwidth = 1024;
    }
    if (screenheight < minscreenheight || screenheight > maxscreenheight) {
        screenheight = 768;
    }

    newdetail = detail;
    return true;
}
