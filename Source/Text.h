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

#ifndef _TEXT_H_
#define _TEXT_H_


/**> HEADER FILES <**/
#include "Quaternions.h"
//#include "Files.h"
#include "Quaternions.h"
#include "gamegl.h"
#include "TGALoader.h"
#include "Texture.h"

class Text
{
public:
    Texture FontTexture;
    GLuint base;

    void LoadFontTexture(const char *fileName);
    void BuildFont();
    void glPrint(float x, float y, const char *string, int set, float size, float width, float height);
    void glPrintOutline(float x, float y, const char *string, int set, float size, float width, float height);
    void glPrint(float x, float y, const char *string, int set, float size, float width, float height, int start, int end);
    void glPrintOutline(float x, float y, const char *string, int set, float size, float width, float height, int start, int end);
    void glPrintOutlined(float x, float y, const char *string, int set, float size, float width, float height);
    void glPrintOutlined(float r, float g, float b, float x, float y, const char *string, int set, float size, float width, float height);

    Text();
    ~Text();

private:
    void _glPrint(float x, float y, const char *string, int set, float size, float width, float height, int start, int end, int offset);
};

#endif
