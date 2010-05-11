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

#ifndef _OBJECTS_H_
#define _OBJECTS_H_

#include "Quaternions.h"
#include "gamegl.h"
#include "TGALoader.h"
#include "Quaternions.h"
#include "Frustum.h"
#include "Lights.h"
#include "Models.h"
#include "Terrain.h"
#include "Sprites.h"
#include <vector>
//
// Model Structures
//

#define max_objects 300

#define boxtype 0
#define weirdtype 1
#define spiketype 2
#define treetrunktype 3
#define treeleavestype 4
#define bushtype 5
#define rocktype 6
#define walltype 7
#define chimneytype 8
#define platformtype 9
#define tunneltype 11
#define cooltype 12
#define firetype 13


class Objects{
public:
	XYZ center;
	float radius;
	XYZ position[max_objects];
	int type[max_objects];
	float rotation[max_objects];
	float rotation2[max_objects];
	float rotx[max_objects];
	float rotxvel[max_objects];
	float roty[max_objects];
	float rotyvel[max_objects];
	int numobjects;
	bool possible[max_objects];
	Model model[max_objects];
	Model displaymodel[max_objects];
	float friction[max_objects];
	float scale[max_objects];
	float messedwith[max_objects];
	float checked[max_objects];
	GLuint 	boxtextureptr;
	GLuint 	treetextureptr;
	GLuint 	bushtextureptr;
	GLuint 	rocktextureptr;
	float shadowed[max_objects];
	float occluded[max_objects];
	bool checkcollide(XYZ startpoint, XYZ endpoint,int which);
	bool onfire[max_objects];
	float flamedelay[max_objects];

	void SphereCheckPossible(XYZ *p1,float radius);
	void DeleteObject(int which);
	void MakeObject(int atype, XYZ where, float arotation, float ascale);
	void MakeObject(int atype, XYZ where, float arotation, float arotation2, float ascale);
	void Draw();
	void DoShadows();
	void DoStuff();

	Objects();
	~Objects();
};

#endif

