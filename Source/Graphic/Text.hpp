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

#ifndef _TEXT_HPP_
#define _TEXT_HPP_

#include "Graphic/Texture.hpp"
#include "Graphic/gamegl.hpp"
#include "Math/XYZ.hpp"
#include "Utils/ImageIO.hpp"

#include <string>

class Text
{
public:
    Texture FontTexture;
    GLuint base;

    void LoadFontTexture(const std::string& fileName);
    void BuildFont();
    void glPrint(float x, float y, const std::string& string, int set, float size, float width, float height, int start = 0, int end = -1);
    void glPrintOutline(float x, float y, const std::string& string, int set, float size, float width, float height, int start = 0, int end = -1);
    void glPrintOutlined(float x, float y, const std::string& string, int set, float size, float width, float height, int start = 0, int end = -1);
    void glPrintOutlined(float r, float g, float b, float a, float x, float y, const std::string& string, int set, float size, float width, float height, int start = 0, int end = -1);

    Text();
    ~Text();

private:
    void _glPrint(float x, float y, const std::string& string, int set, float size, float width, float height, int start, int end, int offset);
};

#endif
