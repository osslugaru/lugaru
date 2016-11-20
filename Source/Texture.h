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

#ifndef _TEXTURE_H_
#define _TEXTURE_H_

#include <map>
#include <vector>
#include <string>
using namespace std;

class TextureRes;

class Texture
{
private:
    TextureRes* tex;
public:
    inline Texture(): tex(NULL) {}
    void load(const string& filename, bool hasMipmap, bool hasAlpha);
    void load(const string& filename, bool hasMipmap, GLubyte* array, int* skinsizep);
    void destroy();
    void bind();

    static void reloadAll();
};

#endif
