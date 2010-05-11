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

#include "Skybox.h"
#include "Game.h"

extern float viewdistance;
extern int detail;
extern bool trilinear;
extern float blurness;
extern int environment;
extern TGAImageRec texture;
extern bool skyboxtexture;
extern float skyboxr;
extern float skyboxg;
extern float skyboxb;
extern int tutoriallevel;

bool	SkyBox::load( char *ffront,char *fleft,char *fback,char *fright,char *fup,char *fdown,char *fcloud,char *freflect)
{
/*	static GLuint		type;
	unsigned char fileNamep[256];
*/
	LOGFUNC;

	//front
	Game::LoadTexture(ffront, &front, true, false);
	//left
	Game::LoadTexture(fleft, &left, true, false);
	//back
	Game::LoadTexture(fback, &back, true, false);
	//right
	Game::LoadTexture(fright, &right, true, false);
	//up
	Game::LoadTexture(fup, &up, true, false);
	//down
	Game::LoadTexture(fdown, &down, true, false);
	//cloud
	Game::LoadTexture(fcloud, &cloud, true, false);
	//reflect
	Game::LoadTexture(freflect, &reflect, true, false);
/*
	//front
	CopyCStringToPascal(ffront,fileNamep);
	upload_image( fileNamep ,0); 
	if(1==1){
		if ( texture.bpp == 24 )
			type = GL_RGB;
		else
			type = GL_RGBA;

		glPixelStorei( GL_UNPACK_ALIGNMENT, 1 );

		if(!front)glGenTextures( 1, &front );
		glTexEnvi( GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE );

		glBindTexture( GL_TEXTURE_2D, front);
		glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR );
		//glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR );
		if(trilinear)glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR );
		if(!trilinear)glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_NEAREST );

		gluBuild2DMipmaps( GL_TEXTURE_2D, type, texture.sizeX, texture.sizeY, type, GL_UNSIGNED_BYTE, texture.data );

	}

	//left
	CopyCStringToPascal(fleft,fileNamep);
	upload_image( fileNamep ,0); 
	if(1==1){
		if ( texture.bpp == 24 )
			type = GL_RGB;
		else
			type = GL_RGBA;

		glPixelStorei( GL_UNPACK_ALIGNMENT, 1 );

		if(!left)glGenTextures( 1, &left );
		glTexEnvi( GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE );

		glBindTexture( GL_TEXTURE_2D, left);
		glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR );
		//glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR );
		if(trilinear)glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR );
		if(!trilinear)glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_NEAREST );

		gluBuild2DMipmaps( GL_TEXTURE_2D, type, texture.sizeX, texture.sizeY, type, GL_UNSIGNED_BYTE, texture.data );

	}

	//back
	CopyCStringToPascal(fback,fileNamep);
	upload_image( fileNamep ,0); 
	if(1==1){
		if ( texture.bpp == 24 )
			type = GL_RGB;
		else
			type = GL_RGBA;

		glPixelStorei( GL_UNPACK_ALIGNMENT, 1 );

		if(!back)glGenTextures( 1, &back );
		glTexEnvi( GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE );

		glBindTexture( GL_TEXTURE_2D, back);
		glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR );
		//glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR );
		if(trilinear)glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR );
		if(!trilinear)glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_NEAREST );

		gluBuild2DMipmaps( GL_TEXTURE_2D, type, texture.sizeX, texture.sizeY, type, GL_UNSIGNED_BYTE, texture.data );

	}

	//right
	CopyCStringToPascal(fright,fileNamep);
	upload_image( fileNamep ,0); 
	if(1==1){
		if ( texture.bpp == 24 )
			type = GL_RGB;
		else
			type = GL_RGBA;

		glPixelStorei( GL_UNPACK_ALIGNMENT, 1 );

		if(!right)glGenTextures( 1, &right );
		glTexEnvi( GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE );

		glBindTexture( GL_TEXTURE_2D, right);
		glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR );
		//glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR );
		if(trilinear)glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR );
		if(!trilinear)glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_NEAREST );

		gluBuild2DMipmaps( GL_TEXTURE_2D, type, texture.sizeX, texture.sizeY, type, GL_UNSIGNED_BYTE, texture.data );

	}

	//up
	CopyCStringToPascal(fup,fileNamep);
	upload_image( fileNamep ,0); 
	if(1==1){
		if ( texture.bpp == 24 )
			type = GL_RGB;
		else
			type = GL_RGBA;

		glPixelStorei( GL_UNPACK_ALIGNMENT, 1 );

		if(!up)glGenTextures( 1, &up );
		glTexEnvi( GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE );

		glBindTexture( GL_TEXTURE_2D, up);
		glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR );
		//glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR );
		if(trilinear)glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR );
		if(!trilinear)glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_NEAREST );

		gluBuild2DMipmaps( GL_TEXTURE_2D, type, texture.sizeX, texture.sizeY, type, GL_UNSIGNED_BYTE, texture.data );

	}

	//down
	CopyCStringToPascal(fdown,fileNamep);
	upload_image( fileNamep ,0); 
	if(1==1){
		if ( texture.bpp == 24 )
			type = GL_RGB;
		else
			type = GL_RGBA;

		glPixelStorei( GL_UNPACK_ALIGNMENT, 1 );

		if(!down)glGenTextures( 1, &down );
		glTexEnvi( GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE );

		glBindTexture( GL_TEXTURE_2D, down);
		glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR );
		//glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR );
		if(trilinear)glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR );
		if(!trilinear)glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_NEAREST );

		gluBuild2DMipmaps( GL_TEXTURE_2D, type, texture.sizeX, texture.sizeY, type, GL_UNSIGNED_BYTE, texture.data );

	}

	//cloud
	CopyCStringToPascal(fcloud,fileNamep);
	upload_image( fileNamep ,0); 
	if(1==1){
		if ( texture.bpp == 24 )
			type = GL_RGB;
		else
			type = GL_RGBA;

		glPixelStorei( GL_UNPACK_ALIGNMENT, 1 );

		if(!cloud)glGenTextures( 1, &cloud );
		glTexEnvi( GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE );

		glBindTexture( GL_TEXTURE_2D, cloud);
		glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR );
		//glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR );
		if(trilinear)glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR );
		if(!trilinear)glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_NEAREST );

		gluBuild2DMipmaps( GL_TEXTURE_2D, type, texture.sizeX, texture.sizeY, type, GL_UNSIGNED_BYTE, texture.data );

	}

	//up
	CopyCStringToPascal(freflect,fileNamep);
	upload_image( fileNamep ,0); 
	if(1==1){
		if ( texture.bpp == 24 )
			type = GL_RGB;
		else
			type = GL_RGBA;

		glPixelStorei( GL_UNPACK_ALIGNMENT, 1 );

		if(!reflect)glGenTextures( 1, &reflect );
		glTexEnvi( GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE );

		glBindTexture( GL_TEXTURE_2D, reflect);
		glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR );
		//glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR );
		if(trilinear)glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR );
		if(!trilinear)glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_NEAREST );

		gluBuild2DMipmaps( GL_TEXTURE_2D, type, texture.sizeX, texture.sizeY, type, GL_UNSIGNED_BYTE, texture.data );

	}
*/
	return true;
}

void	SkyBox::draw()
{
	static float size=viewdistance/4;
	glPushMatrix();
	static GLfloat M[16];
	glGetFloatv(GL_MODELVIEW_MATRIX,M);
	M[12]=0;
	M[13]=0;
	M[14]=0;
	glLoadMatrixf(M);
	//if(environment==2)glTranslatef(0,blurness*viewdistance/1000,0);
	if(environment==2)glScalef(1+blurness/1000,1,1+blurness/1000);
	if(environment!=2)glColor3f(.85*skyboxr,.85*skyboxg,.95*skyboxb);
	else glColor3f(1*skyboxr,.95*skyboxg,.95*skyboxb);

	if(!skyboxtexture){
		glDisable(GL_TEXTURE_2D); 
		glColor3f(skyboxr*.8,skyboxg*.8,skyboxb*.8);
	}
	glDepthMask(0);
	glDisable(GL_CULL_FACE);
	glEnable(GL_BLEND);
	glDisable(GL_LIGHTING);
	//		glActiveTextureARB(GL_TEXTURE0_ARB);
	if(skyboxtexture)glEnable(GL_TEXTURE_2D);
	glTexEnvi( GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE );
	glBindTexture(GL_TEXTURE_2D, front); 
	glTexParameterf( GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE );
	glTexParameterf( GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE );
	glBegin(GL_QUADS);
	glNormal3f( 0.0f, 0.0f, -1);
	glTexCoord2f(0, 0); glVertex3f(-size, -size,  size);
	glTexCoord2f(1, 0); glVertex3f( size, -size,  size);
	glTexCoord2f(1, 1); glVertex3f( size,  size,  size);
	glTexCoord2f(0, 1); glVertex3f(-size,  size,  size);
	glEnd();
	glBindTexture(GL_TEXTURE_2D, back);
	glTexParameterf( GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE );
	glTexParameterf( GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE );
	glBegin(GL_QUADS);
	glNormal3f( 0.0f, 0.0f, 1);
	glTexCoord2f(1, 0); glVertex3f(-size, -size, -size);
	glTexCoord2f(1, 1); glVertex3f(-size,  size, -size);
	glTexCoord2f(0, 1); glVertex3f( size,  size, -size);
	glTexCoord2f(0, 0); glVertex3f( size, -size, -size);
	glEnd();
	glBindTexture(GL_TEXTURE_2D, up);
	glTexParameterf( GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE );
	glTexParameterf( GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE );
	glBegin(GL_QUADS);
	glNormal3f( 0.0f, -1.0f, 0);
	glTexCoord2f(0, 1); glVertex3f(-size,  size, -size);
	glTexCoord2f(0, 0); glVertex3f(-size,  size,  size);
	glTexCoord2f(1, 0); glVertex3f( size,  size,  size);
	glTexCoord2f(1, 1); glVertex3f( size,  size, -size);
	glEnd();
	//if(detail!=0){
	glBindTexture(GL_TEXTURE_2D, down);
	glTexParameterf( GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE );
	glTexParameterf( GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE );
	glBegin(GL_QUADS);
	glNormal3f( 0.0f, 1.0f, 0);

	glTexCoord2f(0, 0); glVertex3f(-size, -size, -size);
	glTexCoord2f(1, 0); glVertex3f( size, -size, -size);
	glTexCoord2f(1, 1); glVertex3f( size, -size,  size);
	glTexCoord2f(0, 1); glVertex3f(-size, -size,  size);
	glEnd();
	//}
	glBindTexture(GL_TEXTURE_2D, right);
	glTexParameterf( GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE );
	glTexParameterf( GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE );
	glBegin(GL_QUADS);
	glNormal3f( -1.0f, 0.0f, 0);
	glTexCoord2f(1, 0); glVertex3f( size, -size, -size);
	glTexCoord2f(1, 1); glVertex3f( size,  size, -size);
	glTexCoord2f(0, 1); glVertex3f( size,  size,  size);
	glTexCoord2f(0, 0); glVertex3f( size, -size,  size);
	glEnd();
	glBindTexture(GL_TEXTURE_2D, left);
	glTexParameterf( GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE );
	glTexParameterf( GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE );
	glBegin(GL_QUADS);
	glNormal3f( 1.0f, 0.0f, 0);
	glTexCoord2f(0, 0); glVertex3f(-size, -size, -size);
	glTexCoord2f(1, 0); glVertex3f(-size, -size,  size);
	glTexCoord2f(1, 1); glVertex3f(-size,  size,  size);
	glTexCoord2f(0, 1); glVertex3f(-size,  size, -size);
	glEnd();
	/*
	glEnable(GL_BLEND);
	glColor4f(1,1,1,1);
	glBindTexture(GL_TEXTURE_2D, cloud);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT); 
	glBegin(GL_QUADS);
	glNormal3f( 0.0f, -1.0f, 0);
	glTexCoord2f(0, 				(1+cloudmove)*30);	glVertex3f(-size,  	size/200, -size);
	glTexCoord2f(0, 				0); 				glVertex3f(-size,  	size/200,  size);
	glTexCoord2f((1+cloudmove)*30, 	0); 				glVertex3f( size, 	size/200,  size);
	glTexCoord2f((1+cloudmove)*30, 	(1+cloudmove)*30); 	glVertex3f( size,  	size/200, -size);
	glEnd();
	glDisable(GL_BLEND);*/
	glEnable(GL_CULL_FACE);
	glDepthMask(1);
	glPopMatrix();
}

SkyBox::SkyBox()
{
	front = 0,left = 0,back = 0,right = 0,up = 0,down = 0,cloud = 0,reflect = 0;
	cloudmove = 0;
}
SkyBox::~SkyBox()
{
	if (front) glDeleteTextures( 1, &front );
	if (left) glDeleteTextures( 1, &left );
	if (back) glDeleteTextures( 1, &back );
	if (right) glDeleteTextures( 1, &right );
	if (up) glDeleteTextures( 1, &up );
	if (down) glDeleteTextures( 1, &down );
	if (cloud) glDeleteTextures( 1, &cloud );
	if (reflect) glDeleteTextures( 1, &reflect );
};

