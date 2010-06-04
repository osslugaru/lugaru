/*
Copyright (C) 2003, 2010 - Wolfire Games
Copyright (C) 2010 - MCMic

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

bool Input::isKeyDown(int k) {
	if(keyboardfrozen)return 0; // vraiment utile? à vérifier
	Uint8 *keystate = SDL_GetKeyState(NULL);
	if(k<SDLK_LAST)
		return keystate[k];
	else
		return SDL_GetMouseState(NULL,NULL)&SDL_BUTTON(k-SDLK_LAST);
}

const char* Input::keyToChar(unsigned short i) {
	if(i<SDLK_LAST)
		return SDL_GetKeyName(SDLKey(i));
	else if(i==SDLK_LAST+SDL_BUTTON_LEFT)
		return "mouse1";
	else if(i==SDLK_LAST+SDL_BUTTON_RIGHT)
		return "mouse2";
	else if(i==SDLK_LAST+SDL_BUTTON_MIDDLE)
		return "mouse3";
	else
		return "unknown";
}

