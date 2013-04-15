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

#ifndef	_TGA_LOADER_H_
#define	_TGA_LOADER_H_

#ifdef _MSC_VER
#pragma once
#endif


/**> HEADER FILES <**/
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#ifdef WIN32
#define WIN32_LEAN_AND_MEAN
#define Polygon WinPolygon
#include <windows.h>
#undef Polygon
#include "GL/gl.h"
#else
#include "gamegl.h"
//	#include "MoreFilesX.h"
#endif

//#include <stdbool.h>
//#include <QuickTime.h>

/**> DATA STRUCTURES <**/
typedef struct TGAImageRec {
    GLubyte	*data;		// Image Data (Up To 32 Bits)
    GLuint	bpp;		// Image Color Depth In Bits Per Pixel.
    GLuint	sizeX;
    GLuint	sizeY;
}	TGAImageRec;

bool upload_image(const unsigned char* filePath, bool hasalpha);

#endif

