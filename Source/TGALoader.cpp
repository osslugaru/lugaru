/*
Copyright (C) 2003, 2010 - Wolfire Games

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

/**> HEADER FILES <**/
#include "Game.h"
#include "TGALoader.h"

extern float texdetail;
extern TGAImageRec texture;
extern short vRefNum;
extern long dirID;
extern bool visibleloading;

extern bool LoadImage(const char * fname, TGAImageRec & tex);
/********************> LoadTGA() <*****/
bool upload_image(const unsigned char* filePath, bool hasalpha)
{
    if (visibleloading)
        Game::LoadingScreen();

    // for Windows, just use TGA loader for now
    char fileName[256];
    CopyPascalStringToC( filePath, fileName);
    return (LoadImage(fileName, texture));
}
