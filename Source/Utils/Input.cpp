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

#include "Utils/Input.hpp"

bool keyDown[SDL_NUM_SCANCODES + 6];
bool keyPressed[SDL_NUM_SCANCODES + 6];

void Input::Tick()
{
    SDL_PumpEvents();
    int numkeys;
    const Uint8* keyState = SDL_GetKeyboardState(&numkeys);
    for (int i = 0; i < numkeys; i++) {
        keyPressed[i] = !keyDown[i] && keyState[i];
        keyDown[i] = keyState[i];
    }
    Uint8 mb = SDL_GetMouseState(NULL, NULL);
    for (int i = 1; i < 6; i++) {
        keyPressed[SDL_NUM_SCANCODES + i] = !keyDown[SDL_NUM_SCANCODES + i] && (mb & SDL_BUTTON(i));
        keyDown[SDL_NUM_SCANCODES + i] = (mb & SDL_BUTTON(i));
    }
}

bool Input::isKeyDown(int k)
{
    if (k >= SDL_NUM_SCANCODES + 6) {
        return false;
    }
    return keyDown[k];
}

bool Input::isKeyPressed(int k)
{
    if (k >= SDL_NUM_SCANCODES + 6) {
        return false;
    }
    return keyPressed[k];
}

const char* Input::keyToChar(unsigned short i)
{
    if (i < SDL_NUM_SCANCODES) {
        return SDL_GetKeyName(SDL_GetKeyFromScancode(SDL_Scancode(i)));
    } else if (i == MOUSEBUTTON_LEFT) {
        return "mouse left button";
    } else if (i == MOUSEBUTTON_RIGHT) {
        return "mouse right button";
    } else if (i == MOUSEBUTTON_MIDDLE) {
        return "mouse middle button";
    } else if (i == MOUSEBUTTON_X1) {
        return "mouse button 4";
    } else if (i == MOUSEBUTTON_X2) {
        return "mouse button 5";
    } else {
        return "unknown";
    }
}

bool Input::MouseClicked()
{
    return isKeyPressed(MOUSEBUTTON_LEFT);
}
