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

#include "gamegl.h"
#include "Texture.h"
#include "ImageIO.h"

using namespace std;

extern bool trilinear;

vector<TextureRes*> TextureRes::list;

void TextureRes::load()
{
    ImageRec texture;

    //load image into 'texture'
    load_image(ConvertFileName(filename.c_str()), texture);

    skinsize = texture.sizeX;
    GLuint type = GL_RGBA;
    if (texture.bpp == 24)
        type = GL_RGB;

    glPixelStorei(GL_UNPACK_ALIGNMENT, 1);

    glDeleteTextures(1, &id);
    glGenTextures(1, &id);
    glTexEnvi(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE);

    glBindTexture(GL_TEXTURE_2D, id);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    if (hasMipmap) {
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, (trilinear ? GL_LINEAR_MIPMAP_LINEAR : GL_LINEAR_MIPMAP_NEAREST));
        glTexParameteri(GL_TEXTURE_2D, GL_GENERATE_MIPMAP, GL_TRUE);
    } else {
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    }

    if (isSkin) {
        free(data);
        const int nb = texture.sizeY * texture.sizeX * (texture.bpp / 8);
        data = (GLubyte*)malloc(nb * sizeof(GLubyte));
        datalen = 0;
        for (int i = 0; i < nb; i++)
            if ((i + 1) % 4 || type == GL_RGB)
                data[datalen++] = texture.data[i];
        glTexImage2D(GL_TEXTURE_2D, 0, type, texture.sizeX, texture.sizeY, 0, GL_RGB, GL_UNSIGNED_BYTE, data);
    } else {
        glTexImage2D(GL_TEXTURE_2D, 0, type, texture.sizeX, texture.sizeY, 0, type, GL_UNSIGNED_BYTE, texture.data);
    }
}

void TextureRes::bind()
{
    glBindTexture(GL_TEXTURE_2D, id);
}

TextureRes::TextureRes(const string& _filename, bool _hasMipmap, bool _hasAlpha):
    id(0), filename(_filename), hasMipmap(_hasMipmap), hasAlpha(_hasAlpha), isSkin(false),
    skinsize(0), data(NULL), datalen(0)
{
    load();
    list.push_back(this);
}

TextureRes::TextureRes(const string& _filename, bool _hasMipmap, GLubyte* array, int* skinsizep):
    id(0), filename(_filename), hasMipmap(_hasMipmap), hasAlpha(false), isSkin(true),
    skinsize(0), data(NULL), datalen(0)
{
    load();
    *skinsizep = skinsize;
    for (int i = 0; i < datalen; i++)
        array[i] = data[i];
    list.push_back(this);
}

TextureRes::~TextureRes()
{
    free(data);
    glDeleteTextures(1, &id);
    for (vector<TextureRes*>::iterator it = list.begin(); it != list.end(); it++)
        if (*it == this) {
            list.erase(it);
            break;
        }
}

void Texture::load(const string& filename, bool hasMipmap, bool hasAlpha)
{
    destroy();
    tex = new TextureRes(filename, hasMipmap, hasAlpha);
}

void Texture::load(const string& filename, bool hasMipmap, GLubyte* array, int* skinsizep)
{
    destroy();
    tex = new TextureRes(filename, hasMipmap, array, skinsizep);
}

void Texture::destroy()
{
    if (tex) {
        delete tex;
        tex = NULL;
    }
}

void Texture::bind()
{
    if (tex)
        tex->bind();
    else
        glBindTexture(GL_TEXTURE_2D, 0);
}
