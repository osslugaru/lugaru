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

/**> HEADER FILES <**/
#include "Input.h"

extern bool keyboardfrozen;

bool keyDown[SDLK_LAST + 6];
bool keyPressed[SDLK_LAST + 6];

void Input::Tick()
{
    SDL_PumpEvents();
    Uint8 *keyState = SDL_GetKeyState(NULL);
    for (int i = 0; i < SDLK_LAST; i++) {
        keyPressed[i] = !keyDown[i] && keyState[i];
        keyDown[i] = keyState[i];
    }
    Uint8 mb = SDL_GetMouseState(NULL, NULL);
    for (int i = 1; i < 6; i++) {
        keyPressed[SDLK_LAST + i] = !keyDown[SDLK_LAST + i] && (mb & SDL_BUTTON(i));
        keyDown[SDLK_LAST + i] = (mb & SDL_BUTTON(i));
    }
}

bool Input::isKeyDown(int k)
{
    if (keyboardfrozen || k >= SDLK_LAST + 6) // really useful? check that.
        return false;
    return keyDown[k];
}

bool Input::isKeyPressed(int k)
{
    if (keyboardfrozen || k >= SDLK_LAST + 6)
        return false;
    return keyPressed[k];
}

const char* Input::keyToChar(unsigned short i)
{
    if (i < SDLK_LAST)
        return SDL_GetKeyName(SDLKey(i));
    else if (i == SDLK_LAST + SDL_BUTTON_LEFT)
        return "mouse1";
    else if (i == SDLK_LAST + SDL_BUTTON_RIGHT)
        return "mouse2";
    else if (i == SDLK_LAST + SDL_BUTTON_MIDDLE)
        return "mouse3";
    else
        return "unknown";
}

unsigned short Input::CharToKey(const char* which)
{
    for (unsigned short i = 0; i < SDLK_LAST; i++) {
        if (!strcasecmp(which, SDL_GetKeyName(SDLKey(i))))
            return i;
    }
    if (!strcasecmp(which, "mouse1")) {
        return MOUSEBUTTON1;
    }
    if (!strcasecmp(which, "mouse2")) {
        return MOUSEBUTTON2;
    }
    return SDLK_LAST;
}

Boolean Input::MouseClicked()
{
    return isKeyPressed(SDLK_LAST + SDL_BUTTON_LEFT);
}
