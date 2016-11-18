/*
Copyright (C) 2003, 2010 - Wolfire Games
Copyright (C) 2010 - Côme <MCMic> BERNIGAUD

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

#ifndef _Input_H_
#define _Input_H_

/**> HEADER FILES <**/
#include "SDL.h"
#include "Game.h"

/**> CONSTANT DECLARATIONS <**/
#define MOUSEBUTTON1 (SDL_NUM_SCANCODES + SDL_BUTTON_LEFT)
#define MOUSEBUTTON2 (SDL_NUM_SCANCODES + SDL_BUTTON_RIGHT)
#define MOUSEBUTTON3 (SDL_NUM_SCANCODES + SDL_BUTTON_MIDDLE)

/**> FUNCTION PROTOTYPES <**/
class Input
{
public:
    static void Tick();
    static bool isKeyDown(int k);
    static bool isKeyPressed(int k);
    static const char* keyToChar(unsigned short which);
    static unsigned short CharToKey(const char* which);
    static Boolean MouseClicked();
};

#endif
