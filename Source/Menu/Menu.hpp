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

#ifndef _MENU_HPP_
#define _MENU_HPP_

#include "Game.hpp"

struct MenuItem
{
    enum MenuItemType
    {
        NONE,
        LABEL,
        BUTTON,
        IMAGE,
        IMAGEBUTTON,
        MAPMARKER,
        MAPLINE,
        MAPLABEL
    } type;
    int id;
    string text;
    Texture texture;
    int x, y, w, h;
    float r, g, b;
    float effectfade;

    float linestartsize;
    float lineendsize;

    MenuItem(MenuItemType _type, int _id, const string& _text, Texture _texture,
             int _x, int _y, int _w, int _h, float _r, float _g, float _b,
             float _linestartsize = 1, float _lineendsize = 1);
};

class Menu
{
public:
    static void clearMenu();
    static void addLabel(int id, const string& text, int x, int y, float r = 1, float g = 0, float b = 0);
    static void addButton(int id, const string& text, int x, int y, float r = 1, float g = 0, float b = 0);
    static void addImage(int id, Texture texture, int x, int y, int w, int h, float r = 1, float g = 1, float b = 1);
    static void addButtonImage(int id, Texture texture, int x, int y, int w, int h, float r = 1, float g = 1, float b = 1);
    static void addMapLine(int x, int y, int w, int h, float startsize, float endsize, float r, float g, float b);
    static void addMapMarker(int id, Texture texture, int x, int y, int w, int h, float r, float g, float b);
    static void addMapLabel(int id, const string& text, int x, int y, float r = 1, float g = 0, float b = 0);
    static void setText(int id, const string& text);
    static void setText(int id, const string& text, int x, int y, int w, int h);
    static int getSelected(int mousex, int mousey);
    static void drawItems();

    static void Load();
    static void Tick();
    static void updateSettingsMenu();
    static void updateStereoConfigMenu();
    static void updateControlsMenu();
    static void setKeySelected();
    static void startChallengeLevel(int selected);

private:
    static void handleFadeEffect();

    static std::vector<MenuItem> items;
};

#endif
