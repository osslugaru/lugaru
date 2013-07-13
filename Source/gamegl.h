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

#ifndef _LUGARU_GL_H_
#define _LUGARU_GL_H_


#include <cstring>
#include <iostream>
#include <fstream>
#include <algorithm>
#include <map>
#include <string>

#ifdef WIN32
  #define WIN32_LEAN_AND_MEAN
  #define Polygon WinPolygon
  #include <windows.h>
  #undef Polygon
#endif

#define GL_GLEXT_PROTOTYPES 1
#include "GL/gl.h"
#include "GL/glu.h"
#include "GL/glext.h"
#include "MacCompatibility.h"

using namespace std;

/* !!! FIXME: until we replace logger better. --ryan. */
#define LOGFUNC
void LOG(const std::string &fmt, ...);

#endif


