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

#ifndef _SPRITES_H_
#define _SPRITES_H_

#include "Quaternions.h"
#include "gamegl.h"
#include "TGALoader.h"
#include "Quaternions.h"
#include "Frustum.h"
#include "Lights.h"
#include "Terrain.h"
#include "Objects.h"
//
// Model Structures
//

#define max_sprites 20000

#define cloudsprite 0
#define bloodsprite 1
#define flamesprite 2
#define smoketype 3
#define weaponflamesprite 4
#define cloudimpactsprite 5
#define snowsprite 6
#define weaponshinesprite 7
#define bloodflamesprite 8
#define breathsprite 9
#define splintersprite 10

class Sprites{
public:
	GLuint cloudtexture;
	GLuint cloudimpacttexture;
	GLuint bloodtexture;
	GLuint flametexture;
	GLuint bloodflametexture;
	GLuint smoketexture;
	GLuint snowflaketexture;
	GLuint shinetexture;
	GLuint splintertexture;
	GLuint leaftexture;
	GLuint toothtexture;

	XYZ oldposition[max_sprites];
	XYZ position[max_sprites];
	XYZ velocity[max_sprites];
	float size[max_sprites];
	float initialsize[max_sprites];
	int type[max_sprites];
	int special[max_sprites];
	float color[max_sprites][3];
	float opacity[max_sprites];
	float rotation[max_sprites];
	float alivetime[max_sprites];
	float speed[max_sprites];
	float rotatespeed[max_sprites];
	float checkdelay;
	int numsprites;

	void DeleteSprite(int which);
	void MakeSprite(int atype, XYZ where, XYZ avelocity, float red, float green, float blue, float asize, float aopacity);
	void Draw();

	Sprites();
	~Sprites();
};

#endif
