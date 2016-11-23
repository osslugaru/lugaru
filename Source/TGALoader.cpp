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

#if !PLATFORM_MACOSX

    // for Windows, just use TGA loader for now
    char fileName[256];
    CopyPascalStringToC( filePath, fileName);
    return (LoadImage(fileName, texture));

#else

    OSStatus err;
    ComponentResult cr;

    //Boolean isdir;
    FSSpec fsspec;
    err = FSMakeFSSpec (0, 0, filePath, &fsspec);
    if (err)
        return;

    GraphicsImportComponent gi;
    err = GetGraphicsImporterForFile(&fsspec, &gi);
    if (err)
        return;

    Rect natbounds;
    cr = GraphicsImportGetNaturalBounds(gi, &natbounds);

    texture.sizeX = natbounds.right;
    texture.sizeY = natbounds.bottom;
    texture.bpp = 32;

    GWorldPtr gw;
    err = QTNewGWorldFromPtr(&gw, k32ARGBPixelFormat, &natbounds, NULL, NULL,
                             0, texture.data, 4 * natbounds.right);
    if (err)
        return;

    cr = GraphicsImportSetGWorld(gi, gw, NULL);

    natbounds.top = natbounds.bottom;
    natbounds.bottom = 0;

    cr = GraphicsImportSetBoundsRect(gi, &natbounds);

    cr = GraphicsImportDraw(gi);

    err = CloseComponent(gi);
    if (err)
        return;

    DisposeGWorld(gw);

    // Loop Through The Image Data
    GLuint imageSize; // Used To Store The Image Size When Setting Aside Ram
    GLuint temp; // Temporary Variable
    GLuint bytesPerPixel; // Temporary Variable
    bytesPerPixel = texture.bpp / 8;
    imageSize = texture.sizeX * texture.sizeY * bytesPerPixel;

    for ( GLuint i = 0; i < int( imageSize ); i += 4 ) {
        // Swaps The 1st And 3rd Bytes ('R'ed and 'B'lue)
        temp = texture.data[i]; // Temporarily Store The Value At Image Data 'i'
        texture.data[i] = texture.data[i + 1]; // Set The 1st Byte To The Value Of The 3rd Byte
        texture.data[i + 1] = texture.data[i + 2]; // Set The 3rd Byte To The Value In 'temp' (1st Byte Value)
        texture.data[i + 2] = texture.data[i + 3];
        texture.data[i + 3] = temp;
    }

    if (!hasalpha) {
        for ( GLuint i = 0; i < int( imageSize ); i += 4 ) {
            texture.data[i + 3] = 255;
        }
    }

    if (texdetail > 1) {
        int which = 0;
        float temp;
        float howmany;
        for ( GLuint k = 0; k < int( imageSize); k += bytesPerPixel * texture.sizeX * texdetail ) {
            for ( GLuint i = 0; i < int( imageSize / texture.sizeY ); i += bytesPerPixel * texdetail ) {
                for ( GLuint b = 0; b < bytesPerPixel ; b ++ ) {
                    temp = 0;
                    howmany = 0;
                    for ( GLuint l = 0; l < texdetail * texture.sizeX ; l += texture.sizeX ) {
                        for ( GLuint j = 0; j < texdetail ; j ++ ) {
                            temp += (int)texture.data[k + i + j * bytesPerPixel + l * bytesPerPixel + b]; // Set The 1st Byte To The Value Of The 3rd Byte
                            howmany++;
                        }
                    }
                    texture.data[which + b] = GLubyte(temp / howmany);
                }
                which += bytesPerPixel;
            }
        }
        texture.sizeX /= texdetail;
        texture.sizeY /= texdetail;
    }

    return true;

#endif
}
