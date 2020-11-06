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

#ifndef _INPUT_HPP_
#define _INPUT_HPP_

#include "Game.hpp"

#include <SDL.h>

/**> CONSTANT DECLARATIONS <**/
#define MOUSEBUTTON_LEFT (SDL_NUM_SCANCODES + SDL_BUTTON_LEFT)
#define MOUSEBUTTON_RIGHT (SDL_NUM_SCANCODES + SDL_BUTTON_RIGHT)
#define MOUSEBUTTON_MIDDLE (SDL_NUM_SCANCODES + SDL_BUTTON_MIDDLE)
#define MOUSEBUTTON_X1 (SDL_NUM_SCANCODES + SDL_BUTTON_X1)
#define MOUSEBUTTON_X2 (SDL_NUM_SCANCODES + SDL_BUTTON_X2)

/**> FUNCTION PROTOTYPES <**/
class Input
{
protected:
    static const int KEYMAX = SDL_NUM_SCANCODES + 6 + SDL_CONTROLLER_BUTTON_MAX + 2 * SDL_CONTROLLER_AXIS_MAX;
    static const int AXIS_THRESHOLD = 10;
    static SDL_GameController* controller;
    static int joyid;
    static bool keyDown[Input::KEYMAX];
    static bool keyPressed[Input::KEYMAX];
public:
    static void Tick();
    static bool isKeyDown(int k);
    static bool isKeyPressed(int k);
    static const char* keyToChar(unsigned short which);
    static bool MouseClicked();
    static int getScancode(SDL_Event evenement);
    static void loadGamepads();
    static bool loadGamepad(int i);
    static void removeGamepad(int i);
    static void closeGamepads();
    static void handleEvent(SDL_Event event);
};

#endif
