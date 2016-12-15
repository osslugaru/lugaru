/*
Copyright (C) 2003, 2010 - Wolfire Games
Copyright (C) 2010-2016 - Lugaru contributors (see AUTHORS file)

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

#ifndef _TEXTURE_HPP_
#define _TEXTURE_HPP_

#include "Graphic/gamegl.hpp"

#include <map>
#include <string>
#include <vector>

using namespace std;

class TextureRes
{
private:
    GLuint id;
    string filename;
    bool hasMipmap;
    bool isSkin;
    int skinsize;
    GLubyte* data;
    int datalen;

    void load();

public:
    TextureRes(const string& filename, bool hasMipmap);
    TextureRes(const string& filename, bool hasMipmap, GLubyte* array, int* skinsize);
    ~TextureRes();
    void bind();
};

class Texture
{
private:
    TextureRes* tex;
public:
    inline Texture(): tex(NULL) {}
    void load(const string& filename, bool hasMipmap);
    void load(const string& filename, bool hasMipmap, GLubyte* array, int* skinsizep);
    void destroy();
    void bind();
};

#endif
