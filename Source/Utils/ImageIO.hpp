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

#ifndef _IMAGE_IO_HPP_
#define _IMAGE_IO_HPP_

#ifdef _MSC_VER
#pragma once
#endif

#include <cstdio>
#include <cstdlib>
#include <cstring>

#ifdef WIN32
#define WIN32_LEAN_AND_MEAN
#define Polygon WinPolygon
#include <windows.h>
#undef Polygon
#include <GL/gl.h>
#else
#include "Graphic/gamegl.hpp"
#endif

/**> DATA STRUCTURES <**/
class ImageRec
{
public:
    GLubyte* data; // Image Data (Up To 32 Bits)
    GLuint bpp;    // Image Color Depth In Bits Per Pixel.
    GLuint sizeX;
    GLuint sizeY;
    ImageRec();
    ~ImageRec();

private:
    /* Make sure this class cannot be copied to avoid memory problems */
    ImageRec(ImageRec const&);
    ImageRec& operator=(ImageRec const&);
};

bool load_image(const char* fname, ImageRec& tex);
bool save_screenshot(const char* fname);

#endif
