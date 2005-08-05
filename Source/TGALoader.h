#pragma once

#ifndef	_TGA_LOADER_H_
#define	_TGA_LOADER_H_


/**> HEADER FILES <**/
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#ifdef WIN32
#define WIN32_LEAN_AND_MEAN
#define Polygon WinPolygon
#include <windows.h>
#undef Polygon
#include <gl/gl.h>
#else
#include "gamegl.h"
//	#include "MoreFilesX.h"
#endif

//#include <stdbool.h>
//#include <QuickTime.h>

/**> DATA STRUCTURES <**/
typedef struct TGAImageRec
{
	GLubyte	*data;		// Image Data (Up To 32 Bits)
	GLuint	bpp;		// Image Color Depth In Bits Per Pixel.
	GLuint	sizeX;
	GLuint	sizeY;
}	TGAImageRec;

bool upload_image(const unsigned char* filePath, bool hasalpha);

/**> FUNCTION PROTOTYPES <**/
TGAImageRec*	LoadTGA( char *filename );


#endif
