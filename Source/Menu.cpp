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

#include <vector>
#include <string>
#include "gamegl.h"

#include "Menu.h"

extern float multiplier;

std::vector<MenuItem> Menu::items;

MenuItem::MenuItem(MenuItemType _type, int _id, const string& _text, Texture _texture,
          int _x, int _y, int _w, int _h, float _r, float _g, float _b,
          float _linestartsize, float _lineendsize):
    type(_type),
    id(_id),
    text(_text),
    texture(_texture),
    x(_x),
    y(_y),
    w(_w),
    h(_h),
    r(_r),
    g(_g),
    b(_b),
    effectfade(0),
    linestartsize(_linestartsize),
    lineendsize(_lineendsize)
{
    if (type == MenuItem::BUTTON) {
        if (w == -1) {
            w = text.length() * 10;
        }
        if (h == -1) {
            h = 20;
        }
    }
}

void Menu::clearMenu()
{
    items.clear();
}

void Menu::addLabel(int id, const string& text, int x, int y, float r, float g, float b)
{
    items.emplace_back(MenuItem::LABEL, id, text, Texture(), x, y, -1, -1, r, g, b);
}
void Menu::addButton(int id, const string& text, int x, int y, float r, float g, float b)
{
    items.emplace_back(MenuItem::BUTTON, id, text, Texture(), x, y, -1, -1, r, g, b);
}
void Menu::addImage(int id, Texture texture, int x, int y, int w, int h, float r, float g, float b)
{
    items.emplace_back(MenuItem::IMAGE, id, "", texture, x, y, w, h, r, g, b);
}
void Menu::addButtonImage(int id, Texture texture, int x, int y, int w, int h, float r, float g, float b)
{
    items.emplace_back(MenuItem::IMAGEBUTTON, id, "", texture, x, y, w, h, r, g, b);
}
void Menu::addMapLine(int x, int y, int w, int h, float startsize, float endsize, float r, float g, float b)
{
    items.emplace_back(MenuItem::MAPLINE, -1, "", Texture(), x, y, w, h, r, g, b, startsize, endsize);
}
void Menu::addMapMarker(int id, Texture texture, int x, int y, int w, int h, float r, float g, float b)
{
    items.emplace_back(MenuItem::MAPMARKER, id, "", texture, x, y, w, h, r, g, b);
}
void Menu::addMapLabel(int id, const string& text, int x, int y, float r, float g, float b)
{
    items.emplace_back(MenuItem::MAPLABEL, id, text, Texture(), x, y, -1, -1, r, g, b);
}

void Menu::setText(int id, const string& text)
{
    for (vector<MenuItem>::iterator it = items.begin(); it != items.end(); it++)
        if (it->id == id) {
            it->text = text;
            it->w = it->text.length() * 10;
            break;
        }
}

void Menu::setText(int id, const string& text, int x, int y, int w, int h)
{
    for (vector<MenuItem>::iterator it = items.begin(); it != items.end(); it++)
        if (it->id == id) {
            it->text = text;
            it->x = x;
            it->y = y;
            if (w == -1)
                it->w = it->text.length() * 10;
            if (h == -1)
                it->h = 20;
            break;
        }
}

int Menu::getSelected(int mousex, int mousey)
{
    for (vector<MenuItem>::iterator it = items.begin(); it != items.end(); it++)
        if (it->type == MenuItem::BUTTON || it->type == MenuItem::IMAGEBUTTON || it->type == MenuItem::MAPMARKER) {
            int mx = mousex;
            int my = mousey;
            if (it->type == MenuItem::MAPMARKER) {
                mx -= 1;
                my += 2;
            }
            if (mx >= it->x && mx < it->x + it->w && my >= it->y && my < it->y + it->h)
                return it->id;
        }
    return -1;
}

void Menu::handleFadeEffect()
{
    for (vector<MenuItem>::iterator it = items.begin(); it != items.end(); it++) {
        if (it->id == Game::selected) {
            it->effectfade += multiplier * 5;
            if (it->effectfade > 1)
                it->effectfade = 1;
        } else {
            it->effectfade -= multiplier * 5;
            if (it->effectfade < 0)
                it->effectfade = 0;
        }
    }
}

void Menu::drawItems()
{
    handleFadeEffect();
    glEnable(GL_TEXTURE_2D);
    glEnable(GL_ALPHA_TEST);
    glEnable(GL_BLEND);
    for (vector<MenuItem>::iterator it = items.begin(); it != items.end(); it++) {
        switch (it->type) {
        case MenuItem::IMAGE:
        case MenuItem::IMAGEBUTTON:
        case MenuItem::MAPMARKER:
            glColor4f(it->r, it->g, it->b, 1);
            glPushMatrix();
            if (it->type == MenuItem::MAPMARKER) {
                glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
                glTranslatef(2.5, -4.5, 0); //from old code
            } else {
                glBlendFunc(GL_SRC_ALPHA, GL_ONE);
            }
            it->texture.bind();
            glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
            glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
            glBegin(GL_QUADS);
            glTexCoord2f(0, 0);
            glVertex3f(it->x, it->y, 0);
            glTexCoord2f(1, 0);
            glVertex3f(it->x + it->w, it->y, 0);
            glTexCoord2f(1, 1);
            glVertex3f(it->x + it->w, it->y + it->h, 0);
            glTexCoord2f(0, 1);
            glVertex3f(it->x, it->y + it->h, 0);
            glEnd();
            if (it->type != MenuItem::IMAGE) {
                //mouseover highlight
                for (int i = 0; i < 10; i++) {
                    if (1 - ((float)i) / 10 - (1 - it->effectfade) > 0) {
                        glColor4f(it->r, it->g, it->b, (1 - ((float)i) / 10 - (1 - it->effectfade))*.25);
                        glBegin(GL_QUADS);
                        glTexCoord2f(0, 0);
                        glVertex3f(it->x - ((float)i) * 1 / 2, it->y - ((float)i) * 1 / 2, 0);
                        glTexCoord2f(1, 0);
                        glVertex3f(it->x + it->w + ((float)i) * 1 / 2, it->y - ((float)i) * 1 / 2, 0);
                        glTexCoord2f(1, 1);
                        glVertex3f(it->x + it->w + ((float)i) * 1 / 2, it->y + it->h + ((float)i) * 1 / 2, 0);
                        glTexCoord2f(0, 1);
                        glVertex3f(it->x - ((float)i) * 1 / 2, it->y + it->h + ((float)i) * 1 / 2, 0);
                        glEnd();
                    }
                }
            }
            glPopMatrix();
            break;
        case MenuItem::LABEL:
        case MenuItem::BUTTON:
            glColor4f(it->r, it->g, it->b, 1);
            glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
            Game::text->glPrint(it->x, it->y, it->text.c_str(), 0, 1, 640, 480);
            if (it->type != MenuItem::LABEL) {
                //mouseover highlight
                glBlendFunc(GL_SRC_ALPHA, GL_ONE);
                for (int i = 0; i < 15; i++) {
                    if (1 - ((float)i) / 15 - (1 - it->effectfade) > 0) {
                        glColor4f(it->r, it->g, it->b, (1 - ((float)i) / 10 - (1 - it->effectfade))*.25);
                        Game::text->glPrint(it->x - ((float)i), it->y, it->text.c_str(), 0, 1 + ((float)i) / 70, 640, 480);
                    }
                }
            }
            break;
        case MenuItem::MAPLABEL:
            Game::text->glPrintOutlined(0.9, 0, 0, it->x, it->y, it->text.c_str(), 0, 0.6, 640, 480);
            break;
        case MenuItem::MAPLINE: {
            XYZ linestart;
            linestart.x = it->x;
            linestart.y = it->y;
            linestart.z = 0;
            XYZ lineend;
            lineend.x = it->x + it->w;
            lineend.y = it->y + it->h;
            lineend.z = 0;
            XYZ offset = lineend - linestart;
            XYZ fac = offset;
            Normalise(&fac);
            offset = DoRotation(offset, 0, 0, 90);
            Normalise(&offset);

            linestart += fac * 4 * it->linestartsize;
            lineend -= fac * 4 * it->lineendsize;

            glDisable(GL_TEXTURE_2D);
            glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
            glColor4f(it->r, it->g, it->b, 1);
            glPushMatrix();
            glTranslatef(2, -5, 0); //from old code
            glBegin(GL_QUADS);
            glVertex3f(linestart.x - offset.x * it->linestartsize, linestart.y - offset.y * it->linestartsize, 0.0f);
            glVertex3f(linestart.x + offset.x * it->linestartsize, linestart.y + offset.y * it->linestartsize, 0.0f);
            glVertex3f(lineend.x + offset.x * it->lineendsize, lineend.y + offset.y * it->lineendsize, 0.0f);
            glVertex3f(lineend.x - offset.x * it->lineendsize, lineend.y - offset.y * it->lineendsize, 0.0f);
            glEnd();
            glPopMatrix();
            glEnable(GL_TEXTURE_2D);
        }
        break;
        default:
        case MenuItem::NONE:
        break;
        }
    }
}

