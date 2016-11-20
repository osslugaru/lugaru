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

#ifndef _MENU_H_
#define _MENU_H_

#include "Game.h"

namespace Menu
{
void clearMenu();
void addLabel(int id, const string& text, int x, int y, float r = 1, float g = 0, float b = 0);
void addButton(int id, const string& text, int x, int y, float r = 1, float g = 0, float b = 0);
void addImage(int id, Texture texture, int x, int y, int w, int h, float r = 1, float g = 1, float b = 1);
void addButtonImage(int id, Texture texture, int x, int y, int w, int h, float r = 1, float g = 1, float b = 1);
void addMapLine(int x, int y, int w, int h, float startsize, float endsize, float r, float g, float b);
void addMapMarker(int id, Texture texture, int x, int y, int w, int h, float r, float g, float b);
void addMapLabel(int id, const string& text, int x, int y, float r = 1, float g = 0, float b = 0);
void setText(int id, const string& text);
void setText(int id, const string& text, int x, int y, int w, int h);
int getSelected(int mousex, int mousey);
void drawItems();
}

#endif
