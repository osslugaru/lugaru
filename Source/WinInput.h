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

#ifndef _WININPUT_H_
#define _WININPUT_H_

/**> HEADER FILES <**/
#include <stdlib.h>
#include <stdio.h>
#include "MacCompatibility.h"
#include "SDL.h"

/**> CONSTANT DECLARATIONS <**/
#define MOUSEBUTTON1 		SDLK_LAST+SDL_BUTTON_LEFT
#define MOUSEBUTTON2 		SDLK_LAST+SDL_BUTTON_RIGHT

/**> FUNCTION PROTOTYPES <**/
void 	InitMouse();
void 	MoveMouse(int xcoord, int ycoord, Point *mouseloc);
void 	RefreshMouse(Point *mouseloc);
void 	DisposeMouse();
unsigned short 	CharToKey(const char* which);
char 	KeyToSingleChar(unsigned short which);
char 	Shift(char which);

Boolean Button();

#endif
