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

#include "Game.h"
#include "openal_wrapper.h"
#include "Animation.h"

extern float screenwidth,screenheight;
extern float viewdistance;
extern XYZ viewer;
extern XYZ lightlocation;
extern float lightambient[3],lightbrightness[3];
extern float fadestart;
extern float texscale;
extern float gravity;
extern Light light;
extern Skeleton testskeleton;
extern int numsounds;
extern int channels[100];
extern Terrain terrain;
//extern Sprites sprites;
extern int kTextureSize;
extern float texdetail;
extern float realtexdetail;
extern float terraindetail;
extern float volume;
extern Objects objects;
extern int detail;
extern bool cellophane;
extern GLubyte bloodText[512*512*3];
extern GLubyte wolfbloodText[512*512*3];
extern bool ismotionblur;
extern bool trilinear;
extern bool osx;
extern bool musictoggle;
extern int environment;
extern bool ambientsound;
extern float multiplier;
extern int netdatanew;
extern float mapinfo;
extern bool stillloading;
extern TGAImageRec texture;
extern short vRefNum;
extern long dirID;
extern int mainmenu;
extern int oldmainmenu;
extern bool visibleloading;
extern int loadscreencolor;
extern float flashamount,flashr,flashg,flashb;
extern int flashdelay;
extern int whichjointstartarray[26];
extern int whichjointendarray[26];
extern int difficulty;
extern float tintr,tintg,tintb;
extern float slomospeed;
extern char mapname[256];
extern bool gamestarted;

extern int numdialogues;
extern int numdialogueboxes[20];
extern int dialoguetype[20];
extern int dialogueboxlocation[20][20];
extern float dialogueboxcolor[20][20][3];
extern int dialogueboxsound[20][20];
extern char dialoguetext[20][20][128];
extern char dialoguename[20][20][64];
extern XYZ dialoguecamera[20][20];
extern float dialoguecamerarotation[20][20];
extern float dialoguecamerarotation2[20][20];
extern int indialogue;
extern int whichdialogue;
extern float dialoguetime;

extern float accountcampaignhighscore[10];
extern float accountcampaignfasttime[10];
extern float accountcampaignscore[10];
extern float accountcampaigntime[10];

extern int accountcampaignchoicesmade[10];
extern int accountcampaignchoices[10][5000];

extern "C"	void PlaySoundEx(int channel, OPENAL_SAMPLE *sptr, OPENAL_DSPUNIT *dsp, signed char startpaused);
extern "C" void PlayStreamEx(int chan, OPENAL_STREAM *sptr, OPENAL_DSPUNIT *dsp, signed char startpaused);

void LOG(const std::string &fmt, ...)
{
    // !!! FIXME: write me.
}


Game::TextureList Game::textures;

void Game::Dispose()
{
	int i,j;

	LOGFUNC;

	if(endgame==2){
		accountactive->endGame();
		endgame=0;
	}

	Account::saveFile(":Data:Users", accountactive);

	TexIter it = textures.begin();
	for (; it != textures.end(); ++it)
	{
		if (glIsTexture(it->second))
			glDeleteTextures(1, &it->second);
	}
	textures.clear();

	LOG("Shutting down sound system...");

	OPENAL_StopSound(OPENAL_ALL);

// this is causing problems on Linux, but we'll force an _exit() a little
//  later in the shutdown process.  --ryan.
#if !PLATFORM_LINUX

	for (i=0; i < sounds_count; ++i)
	{
		OPENAL_Sample_Free(samp[i]);
	}

	OPENAL_Close();
	if (texture.data)
	{
		free(texture.data);
	}
	texture.data = 0;
#endif
}


void Game::LoadTexture(const char *fileName, GLuint *textureid,int mipmap, bool hasalpha)
{
	GLuint		type;

	LOGFUNC;

	LOG(std::string("Loading texture...") + fileName);

	// Fix filename so that is os appropreate
	char * fixedFN = ConvertFileName(fileName);

	unsigned char fileNamep[256];
	CopyCStringToPascal(fixedFN, fileNamep);
	//Load Image
	upload_image( fileNamep ,hasalpha);

//	std::string fname(fileName);
//	std::transform(fname.begin(), fname.end(), tolower);
//	TexIter it = textures.find(fname);

	//Is it valid?
	if(1==1)
	//if(textures.end() == it)
	{
		//Alpha channel?
		if ( texture.bpp == 24 )
			type = GL_RGB;
		else
			type = GL_RGBA;

		glPixelStorei( GL_UNPACK_ALIGNMENT, 1 );

		if(!*textureid)
			glGenTextures( 1, textureid );
		glTexEnvi( GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE );

		glBindTexture( GL_TEXTURE_2D, *textureid);
		glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR );
		if(trilinear)if(mipmap)glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR );
		if(!trilinear)if(mipmap)glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_NEAREST );
		if(!mipmap)glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR );
		
		gluBuild2DMipmaps( GL_TEXTURE_2D, type, texture.sizeX, texture.sizeY, type, GL_UNSIGNED_BYTE, texture.data );
	}
}

void Game::LoadTextureSave(const char *fileName, GLuint *textureid,int mipmap,GLubyte *array, int *skinsize)
{
	GLuint		type;
	int i;
	int bytesPerPixel;

	LOGFUNC;

	LOG(std::string("Loading texture (S)...") + fileName);

	//Load Image
	unsigned char fileNamep[256];
	CopyCStringToPascal(ConvertFileName(fileName), fileNamep);
	//Load Image
	upload_image( fileNamep ,0);
	//LoadTGA( fileName );

//	std::string fname(fileName);
//	std::transform(fname.begin(), fname.end(), tolower);
//	TexIter it = textures.find(fname);

	//Is it valid?
	if(1==1)
	//if(textures.end() == it)
	{
		bytesPerPixel=texture.bpp/8;

		//Alpha channel?
		if ( texture.bpp == 24 )
			type = GL_RGB;
		else
			type = GL_RGBA;

		glPixelStorei( GL_UNPACK_ALIGNMENT, 1 );

		if(!*textureid)glGenTextures( 1, textureid );
		glTexEnvi( GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE );

		glBindTexture( GL_TEXTURE_2D, *textureid);
		glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR );
		if(trilinear)if(mipmap)glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR );
		if(!trilinear)if(mipmap)glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_NEAREST );
		if(!mipmap)glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR );

		int tempnum=0;
		for(i=0;i<(int)(texture.sizeY*texture.sizeX*bytesPerPixel);i++){
			if((i+1)%4||type==GL_RGB){
				array[tempnum]=texture.data[i];
				tempnum++;
			}
		}

		*skinsize=texture.sizeX;

		gluBuild2DMipmaps( GL_TEXTURE_2D, type, texture.sizeX, texture.sizeY, GL_RGB, GL_UNSIGNED_BYTE, array );

//		textures.insert(std::make_pair(fname, *textureid));
	}
//	else
//	{
//		*textureid = it->second;
//	}
}

void Game::LoadSave(const char *fileName, GLuint *textureid,bool mipmap,GLubyte *array, int *skinsize)
{
	int i;
	int bytesPerPixel;

	LOGFUNC;

	LOG(std::string("Loading (S)...") + fileName);

	//Load Image
	float temptexdetail=texdetail;
	texdetail=1;
	//upload_image( fileName );
	//LoadTGA( fileName );

	// Converting file to something os specific
	char * fixedFN = ConvertFileName(fileName);

	//Load Image
	unsigned char fileNamep[256];
	CopyCStringToPascal(fixedFN, fileNamep);
	//Load Image
	upload_image( fileNamep ,0);
	texdetail=temptexdetail;

	//Is it valid?
	if(1==1){
		bytesPerPixel=texture.bpp/8;

		int tempnum=0;
		for(i=0;i<(int)(texture.sizeY*texture.sizeX*bytesPerPixel);i++){
			if((i+1)%4||bytesPerPixel==3){
				array[tempnum]=texture.data[i];
				tempnum++;
			}
		}
	}
}

bool Game::AddClothes(const char *fileName, GLuint *textureid,bool mipmap,GLubyte *array, int *skinsize)
{
	int i;
	int bytesPerPixel;

	LOGFUNC;

	//upload_image( fileName );
	//LoadTGA( fileName );
	//Load Image
	unsigned char fileNamep[256];
	CopyCStringToPascal(fileName,fileNamep);
	//Load Image
	bool opened;
	opened=upload_image( fileNamep ,1);

	float alphanum;
	//Is it valid?
	if(opened){
		if(tintr>1)tintr=1;
		if(tintg>1)tintg=1;
		if(tintb>1)tintb=1;

		if(tintr<0)tintr=0;
		if(tintg<0)tintg=0;
		if(tintb<0)tintb=0;

		bytesPerPixel=texture.bpp/8;

		int tempnum=0;
		alphanum=255;
		for(i=0;i<(int)(texture.sizeY*texture.sizeX*bytesPerPixel);i++){
			if(bytesPerPixel==3)alphanum=255;
			else if((i+1)%4==0)alphanum=texture.data[i];
			//alphanum/=2;
			if((i+1)%4||bytesPerPixel==3){
				if((i%4)==0)texture.data[i]*=tintr;
				if((i%4)==1)texture.data[i]*=tintg;
				if((i%4)==2)texture.data[i]*=tintb;
				array[tempnum]=(float)array[tempnum]*(1-alphanum/255)+(float)texture.data[i]*(alphanum/255);
				tempnum++;
			}
		}
	}
	else return 0;
	return 1;
}


//***************> ResizeGLScene() <******/
GLvoid Game::ReSizeGLScene(float fov, float pnear)
{
	if (screenheight==0)
	{
		screenheight=1;
	}

	glViewport(0,0,screenwidth,screenheight);

	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();

	gluPerspective(fov,(GLfloat)screenwidth/(GLfloat)screenheight,pnear,viewdistance);

	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();
}

void Game::LoadingScreen()
{
	static float loadprogress,minprogress,maxprogress;
	static AbsoluteTime time = {0,0};
	static AbsoluteTime frametime = {0,0};
	AbsoluteTime currTime = UpTime ();
	double deltaTime = (float) AbsoluteDeltaToDuration (currTime, frametime);

	if (0 > deltaTime)	// if negative microseconds
		deltaTime /= -1000000.0;
	else				// else milliseconds
		deltaTime /= 1000.0;

	multiplier=deltaTime;
	if(multiplier<.001)multiplier=.001;
	if(multiplier>10)multiplier=10;
	if(multiplier>.05){
		frametime = currTime;	// reset for next time interval

		float size=1;
		glLoadIdentity();
		//Clear to black
		glClearColor(0,0,0,1);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);


		loadtime+=multiplier*4;

		loadprogress=loadtime;
		if(loadprogress>100)loadprogress=100;

		//loadprogress=abs(Random()%100);

		//Background

		glEnable(GL_TEXTURE_2D);
		glBindTexture( GL_TEXTURE_2D, loadscreentexture);
		glTexParameterf( GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT );
		glTexParameterf( GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT );
		glDisable(GL_DEPTH_TEST);							// Disables Depth Testing
		glDisable(GL_CULL_FACE);
		glDisable(GL_LIGHTING);
		glDepthMask(0);
		glMatrixMode(GL_PROJECTION);						// Select The Projection Matrix
		glPushMatrix();										// Store The Projection Matrix
		glLoadIdentity();									// Reset The Projection Matrix
		glOrtho(0,screenwidth,0,screenheight,-100,100);						// Set Up An Ortho Screen
		glMatrixMode(GL_MODELVIEW);							// Select The Modelview Matrix
		glPushMatrix();										// Store The Modelview Matrix
		glLoadIdentity();								// Reset The Modelview Matrix
		glTranslatef(screenwidth/2,screenheight/2,0);
		glScalef((float)screenwidth/2,(float)screenheight/2,1);
		glBlendFunc(GL_SRC_ALPHA,GL_ONE_MINUS_SRC_ALPHA);
		glDisable(GL_BLEND);
		glColor4f(loadprogress/100,loadprogress/100,loadprogress/100,1);
		//glColor4f(1,1,1,1);
		/*if(loadscreencolor==0)glColor4f(1,1,1,1);
		if(loadscreencolor==1)glColor4f(1,0,0,1);
		if(loadscreencolor==2)glColor4f(0,1,0,1);
		if(loadscreencolor==3)glColor4f(0,0,1,1);
		if(loadscreencolor==4)glColor4f(1,1,0,1);
		if(loadscreencolor==5)glColor4f(1,0,1,1);
		*/
		glPushMatrix();
		//glScalef(.25,.25,.25);
		glBegin(GL_QUADS);
		glTexCoord2f(.1-loadprogress/100,0+loadprogress/100+.3);
		glVertex3f(-1,		-1,	 0.0f);
		glTexCoord2f(.1-loadprogress/100,0+loadprogress/100+.3);
		glVertex3f(1,	-1,	 0.0f);
		glTexCoord2f(.1-loadprogress/100,1+loadprogress/100+.3);
		glVertex3f(1,	1, 0.0f);
		glTexCoord2f(.1-loadprogress/100,1+loadprogress/100+.3);
		glVertex3f(-1,	1, 0.0f);
		glEnd();
		glPopMatrix();
		glEnable(GL_BLEND);
		glPushMatrix();
		//glScalef(.25,.25,.25);
		glBegin(GL_QUADS);
		glTexCoord2f(.4+loadprogress/100,0+loadprogress/100);
		glVertex3f(-1,		-1,	 0.0f);
		glTexCoord2f(.4+loadprogress/100,0+loadprogress/100);
		glVertex3f(1,	-1,	 0.0f);
		glTexCoord2f(.4+loadprogress/100,1+loadprogress/100);
		glVertex3f(1,	1, 0.0f);
		glTexCoord2f(.4+loadprogress/100,1+loadprogress/100);
		glVertex3f(-1,	1, 0.0f);
		glEnd();
		glPopMatrix();
		glDisable(GL_TEXTURE_2D);
		glMatrixMode(GL_PROJECTION);						// Select The Projection Matrix
		glPopMatrix();										// Restore The Old Projection Matrix
		glMatrixMode(GL_MODELVIEW);							// Select The Modelview Matrix
		glPopMatrix();										// Restore The Old Projection Matrix
		glDisable(GL_BLEND);
		glDepthMask(1);

		glEnable(GL_TEXTURE_2D);
		glBindTexture( GL_TEXTURE_2D, loadscreentexture);
		glTexParameterf( GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT );
		glTexParameterf( GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT );
		glDisable(GL_DEPTH_TEST);							// Disables Depth Testing
		glDisable(GL_CULL_FACE);
		glDisable(GL_LIGHTING);
		glDepthMask(0);
		glMatrixMode(GL_PROJECTION);						// Select The Projection Matrix
		glPushMatrix();										// Store The Projection Matrix
		glLoadIdentity();									// Reset The Projection Matrix
		glOrtho(0,screenwidth,0,screenheight,-100,100);						// Set Up An Ortho Screen
		glMatrixMode(GL_MODELVIEW);							// Select The Modelview Matrix
		glPushMatrix();										// Store The Modelview Matrix
		glLoadIdentity();								// Reset The Modelview Matrix
		glTranslatef(screenwidth/2,screenheight/2,0);
		glScalef((float)screenwidth/2*(1.5-(loadprogress)/200),(float)screenheight/2*(1.5-(loadprogress)/200),1);
		glBlendFunc(GL_SRC_ALPHA,GL_ONE);
		glEnable(GL_BLEND);
		//glColor4f(loadprogress/100,loadprogress/100,loadprogress/100,1);
		glColor4f(loadprogress/100,loadprogress/100,loadprogress/100,1);
		/*if(loadscreencolor==0)glColor4f(1,1,1,1);
		if(loadscreencolor==1)glColor4f(1,0,0,1);
		if(loadscreencolor==2)glColor4f(0,1,0,1);
		if(loadscreencolor==3)glColor4f(0,0,1,1);
		if(loadscreencolor==4)glColor4f(1,1,0,1);
		if(loadscreencolor==5)glColor4f(1,0,1,1);
		*/
		glPushMatrix();
		//glScalef(.25,.25,.25);
		glBegin(GL_QUADS);
		glTexCoord2f(0+.5,0+.5);
		glVertex3f(-1,		-1,	 0.0f);
		glTexCoord2f(1+.5,0+.5);
		glVertex3f(1,	-1,	 0.0f);
		glTexCoord2f(1+.5,1+.5);
		glVertex3f(1,	1, 0.0f);
		glTexCoord2f(0+.5,1+.5);
		glVertex3f(-1,	1, 0.0f);
		glEnd();
		glPopMatrix();
		glDisable(GL_TEXTURE_2D);
		glMatrixMode(GL_PROJECTION);						// Select The Projection Matrix
		glPopMatrix();										// Restore The Old Projection Matrix
		glMatrixMode(GL_MODELVIEW);							// Select The Modelview Matrix
		glPopMatrix();										// Restore The Old Projection Matrix
		glDisable(GL_BLEND);
		glDepthMask(1);

		glEnable(GL_TEXTURE_2D);
		glBindTexture( GL_TEXTURE_2D, loadscreentexture);
		glTexParameterf( GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT );
		glTexParameterf( GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT );
		glDisable(GL_DEPTH_TEST);							// Disables Depth Testing
		glDisable(GL_CULL_FACE);
		glDisable(GL_LIGHTING);
		glDepthMask(0);
		glMatrixMode(GL_PROJECTION);						// Select The Projection Matrix
		glPushMatrix();										// Store The Projection Matrix
		glLoadIdentity();									// Reset The Projection Matrix
		glOrtho(0,screenwidth,0,screenheight,-100,100);						// Set Up An Ortho Screen
		glMatrixMode(GL_MODELVIEW);							// Select The Modelview Matrix
		glPushMatrix();										// Store The Modelview Matrix
		glLoadIdentity();								// Reset The Modelview Matrix
		glTranslatef(screenwidth/2,screenheight/2,0);
		glScalef((float)screenwidth/2*(100+loadprogress)/100,(float)screenheight/2*(100+loadprogress)/100,1);
		glBlendFunc(GL_SRC_ALPHA,GL_ONE);
		glEnable(GL_BLEND);
		glColor4f(loadprogress/100,loadprogress/100,loadprogress/100,.4);
		glPushMatrix();
		//glScalef(.25,.25,.25);
		glBegin(GL_QUADS);
		glTexCoord2f(0+.2,0+.8);
		glVertex3f(-1,		-1,	 0.0f);
		glTexCoord2f(1+.2,0+.8);
		glVertex3f(1,	-1,	 0.0f);
		glTexCoord2f(1+.2,1+.8);
		glVertex3f(1,	1, 0.0f);
		glTexCoord2f(0+.2,1+.8);
		glVertex3f(-1,	1, 0.0f);
		glEnd();
		glPopMatrix();
		glDisable(GL_TEXTURE_2D);
		glMatrixMode(GL_PROJECTION);						// Select The Projection Matrix
		glPopMatrix();										// Restore The Old Projection Matrix
		glMatrixMode(GL_MODELVIEW);							// Select The Modelview Matrix
		glPopMatrix();										// Restore The Old Projection Matrix
		glDisable(GL_BLEND);
		glDepthMask(1);

		//Text
		/*
		glEnable(GL_TEXTURE_2D);
		static char string[256]="";
		sprintf (string, "LOADING... %d%",(int)loadprogress);
		glColor4f(1,1,1,.2);
		text.glPrint(280-280*loadprogress/100/2/4,125-125*loadprogress/100/2/4,string,1,1+loadprogress/100,640,480);
		glColor4f(1.2-loadprogress/100,1.2-loadprogress/100,1.2-loadprogress/100,1);
		text.glPrint(280,125,string,1,1,640,480);
		*/

		if(flashamount>0){
			if(flashamount>1)flashamount=1;
			if(flashdelay<=0)flashamount-=multiplier;
			flashdelay--;
			if(flashamount<0)flashamount=0;
			glDisable(GL_DEPTH_TEST);							// Disables Depth Testing
			glDisable(GL_CULL_FACE);
			glDisable(GL_LIGHTING);
			glDisable(GL_TEXTURE_2D);
			glDepthMask(0);
			glMatrixMode(GL_PROJECTION);						// Select The Projection Matrix
			glPushMatrix();										// Store The Projection Matrix
			glLoadIdentity();									// Reset The Projection Matrix
			glOrtho(0,screenwidth,0,screenheight,-100,100);						// Set Up An Ortho Screen
			glMatrixMode(GL_MODELVIEW);							// Select The Modelview Matrix
			glPushMatrix();										// Store The Modelview Matrix
			glLoadIdentity();								// Reset The Modelview Matrix
			glScalef(screenwidth,screenheight,1);
			glBlendFunc(GL_SRC_ALPHA,GL_ONE_MINUS_SRC_ALPHA);
			glEnable(GL_BLEND);
			glColor4f(flashr,flashg,flashb,flashamount);
			glBegin(GL_QUADS);
			glVertex3f(0,		0,	 0.0f);
			glVertex3f(256,	0,	 0.0f);
			glVertex3f(256,	256, 0.0f);
			glVertex3f(0,	256, 0.0f);
			glEnd();
			glMatrixMode(GL_PROJECTION);						// Select The Projection Matrix
			glPopMatrix();										// Restore The Old Projection Matrix
			glMatrixMode(GL_MODELVIEW);							// Select The Modelview Matrix
			glPopMatrix();										// Restore The Old Projection Matrix
			glEnable(GL_DEPTH_TEST);							// Enables Depth Testing
			glEnable(GL_CULL_FACE);
			glDisable(GL_BLEND);
			glDepthMask(1);
		}

		swap_gl_buffers();
		loadscreencolor=0;
	}
}

void Game::FadeLoadingScreen(float howmuch)
{
	static float loadprogress,minprogress,maxprogress;

	float size=1;
	glLoadIdentity();
	//Clear to black
	glClearColor(0,0,0,1);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	loadprogress=howmuch;

	//loadprogress=abs(Random()%100);

	//Background

	//glEnable(GL_TEXTURE_2D);
	glDisable(GL_TEXTURE_2D);
	//glBindTexture( GL_TEXTURE_2D, loadscreentexture);
	glTexParameterf( GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE );
	glTexParameterf( GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE );
	glDisable(GL_DEPTH_TEST);							// Disables Depth Testing
	glDisable(GL_CULL_FACE);
	glDisable(GL_LIGHTING);
	glDepthMask(0);
	glMatrixMode(GL_PROJECTION);						// Select The Projection Matrix
	glPushMatrix();										// Store The Projection Matrix
	glLoadIdentity();									// Reset The Projection Matrix
	glOrtho(0,screenwidth,0,screenheight,-100,100);						// Set Up An Ortho Screen
	glMatrixMode(GL_MODELVIEW);							// Select The Modelview Matrix
	glPushMatrix();										// Store The Modelview Matrix
	glLoadIdentity();								// Reset The Modelview Matrix
	glTranslatef(screenwidth/2,screenheight/2,0);
	glScalef((float)screenwidth/2,(float)screenheight/2,1);
	glBlendFunc(GL_SRC_ALPHA,GL_ONE_MINUS_SRC_ALPHA);
	glDisable(GL_BLEND);
	glColor4f(loadprogress/100,0,0,1);
	/*if(loadscreencolor==0)glColor4f(1,1,1,1);
	if(loadscreencolor==1)glColor4f(1,0,0,1);
	if(loadscreencolor==2)glColor4f(0,1,0,1);
	if(loadscreencolor==3)glColor4f(0,0,1,1);
	if(loadscreencolor==4)glColor4f(1,1,0,1);
	if(loadscreencolor==5)glColor4f(1,0,1,1);
	*/
	glPushMatrix();
	//glScalef(.25,.25,.25);
	glBegin(GL_QUADS);
	glTexCoord2f(0,0);
	glVertex3f(-1,		-1,	 0.0f);
	glTexCoord2f(1,0);
	glVertex3f(1,	-1,	 0.0f);
	glTexCoord2f(1,1);
	glVertex3f(1,	1, 0.0f);
	glTexCoord2f(0,1);
	glVertex3f(-1,	1, 0.0f);
	glEnd();
	glPopMatrix();
	glDisable(GL_TEXTURE_2D);
	glMatrixMode(GL_PROJECTION);						// Select The Projection Matrix
	glPopMatrix();										// Restore The Old Projection Matrix
	glMatrixMode(GL_MODELVIEW);							// Select The Modelview Matrix
	glPopMatrix();										// Restore The Old Projection Matrix
	glDisable(GL_BLEND);
	glDepthMask(1);
	//Text
	/*
	glEnable(GL_TEXTURE_2D);
	static char string[256]="";
	sprintf (string, "LOADING... %d%",(int)loadprogress);
	glColor4f(1,1,1,.2);
	text.glPrint(280-280*loadprogress/100/2/4,125-125*loadprogress/100/2/4,string,1,1+loadprogress/100,640,480);
	glColor4f(1.2-loadprogress/100,1.2-loadprogress/100,1.2-loadprogress/100,1);
	text.glPrint(280,125,string,1,1,640,480);
	*/
	swap_gl_buffers();
	loadscreencolor=0;
}


void Game::InitGame()
{
#if PLATFORM_MACOSX
	ProcessSerialNumber PSN;
	ProcessInfoRec pinfo;
	FSSpec pspec;
	OSStatus err;
	/* set up process serial number */
	PSN.highLongOfPSN = 0;
	PSN.lowLongOfPSN = kCurrentProcess;
	/* set up info block */
	pinfo.processInfoLength = sizeof(pinfo);
	pinfo.processName = NULL;
	pinfo.processAppSpec = &pspec;
	/* grab the vrefnum and directory */
	err = GetProcessInformation(&PSN, &pinfo);
	if (err == noErr) {
		vRefNum = pspec.vRefNum;
		dirID = pspec.parID;
	}
#endif

	LOGFUNC;

	autocam=0;

	int i,j;

	numchallengelevels=14;

	accountactive=Account::loadFile(":Data:Users");

	tintr=1;
	tintg=1;
	tintb=1;

	whichjointstartarray[0]=righthip;
	whichjointendarray[0]=rightfoot;

	whichjointstartarray[1]=righthip;
	whichjointendarray[1]=rightankle;

	whichjointstartarray[2]=righthip;
	whichjointendarray[2]=rightknee;

	whichjointstartarray[3]=rightknee;
	whichjointendarray[3]=rightankle;

	whichjointstartarray[4]=rightankle;
	whichjointendarray[4]=rightfoot;

	whichjointstartarray[5]=lefthip;
	whichjointendarray[5]=leftfoot;

	whichjointstartarray[6]=lefthip;
	whichjointendarray[6]=leftankle;

	whichjointstartarray[7]=lefthip;
	whichjointendarray[7]=leftknee;

	whichjointstartarray[8]=leftknee;
	whichjointendarray[8]=leftankle;

	whichjointstartarray[9]=leftankle;
	whichjointendarray[9]=leftfoot;

	whichjointstartarray[10]=abdomen;
	whichjointendarray[10]=rightshoulder;

	whichjointstartarray[11]=abdomen;
	whichjointendarray[11]=rightelbow;

	whichjointstartarray[12]=abdomen;
	whichjointendarray[12]=rightwrist;

	whichjointstartarray[13]=abdomen;
	whichjointendarray[13]=righthand;

	whichjointstartarray[14]=rightshoulder;
	whichjointendarray[14]=rightelbow;

	whichjointstartarray[15]=rightelbow;
	whichjointendarray[15]=rightwrist;

	whichjointstartarray[16]=rightwrist;
	whichjointendarray[16]=righthand;

	whichjointstartarray[17]=abdomen;
	whichjointendarray[17]=leftshoulder;

	whichjointstartarray[18]=abdomen;
	whichjointendarray[18]=leftelbow;

	whichjointstartarray[19]=abdomen;
	whichjointendarray[19]=leftwrist;

	whichjointstartarray[20]=abdomen;
	whichjointendarray[20]=lefthand;

	whichjointstartarray[21]=leftshoulder;
	whichjointendarray[21]=leftelbow;

	whichjointstartarray[22]=leftelbow;
	whichjointendarray[22]=leftwrist;

	whichjointstartarray[23]=leftwrist;
	whichjointendarray[23]=lefthand;

	whichjointstartarray[24]=abdomen;
	whichjointendarray[24]=neck;

	whichjointstartarray[25]=neck;
	whichjointendarray[25]=head;

	FadeLoadingScreen(0);

	stillloading=1;

	texture.data = ( GLubyte* )malloc( 1024*1024*4 );

	int temptexdetail=texdetail;
	texdetail=1;
	text.LoadFontTexture(":Data:Textures:Font.png");
	text.BuildFont();
	texdetail=temptexdetail;

	FadeLoadingScreen(10);

	if(detail==2){
		texdetail=1;
		terraindetail=1;
	}
	if(detail==1){
		texdetail=2;
		terraindetail=1;
	}
	if(detail==0){
		texdetail=4;
		terraindetail=1;
		//terraindetail=2;
	}

	memset(channels, 0xff, sizeof(channels));

	LOG("Initializing sound system...");

    int output = -1;

    #if PLATFORM_LINUX
    extern bool cmdline(const char *cmd);
    unsigned char rc = 0;
    output = OPENAL_OUTPUT_ALSA;  // Try alsa first...
    if (cmdline("forceoss"))      //  ...but let user override that.
        output = OPENAL_OUTPUT_OSS;
    else if (cmdline("nosound"))
        output = OPENAL_OUTPUT_NOSOUND;

    OPENAL_SetOutput(output);
	if ((rc = OPENAL_Init(44100, 32, 0)) == false)
    {
        // if we tried ALSA and failed, fall back to OSS.
        if ( (output == OPENAL_OUTPUT_ALSA) && (!cmdline("forcealsa")) )
        {
            OPENAL_Close();
            output = OPENAL_OUTPUT_OSS;
            OPENAL_SetOutput(output);
	        rc = OPENAL_Init(44100, 32, 0);
        }
    }

    if (rc == false)
    {
        OPENAL_Close();
        output = OPENAL_OUTPUT_NOSOUND;  // we tried! just do silence.
        OPENAL_SetOutput(output);
	    rc = OPENAL_Init(44100, 32, 0);
    }
    #else
	OPENAL_Init(44100, 32, 0);
    #endif

	OPENAL_SetSFXMasterVolume((int)(volume*255));
	loadAllSounds();

	if(musictoggle){
		PlayStreamEx(stream_music3, samp[stream_music3], 0, true);
		OPENAL_SetPaused(channels[stream_music3], false);
		OPENAL_SetVolume(channels[stream_music3], 256);
	}

	LoadTexture(":Data:Textures:Cursor.png",&cursortexture,0,1);

	LoadTexture(":Data:Textures:MapCircle.png",&Mapcircletexture,0,1);
	LoadTexture(":Data:Textures:MapBox.png",&Mapboxtexture,0,1);
	LoadTexture(":Data:Textures:MapArrow.png",&Maparrowtexture,0,1);

	temptexdetail=texdetail;
	if(texdetail>2)texdetail=2;
	LoadTexture(":Data:Textures:Lugaru.png",&Mainmenuitems[0],0,0);
	LoadTexture(":Data:Textures:Newgame.png",&Mainmenuitems[1],0,0);
	LoadTexture(":Data:Textures:Options.png",&Mainmenuitems[2],0,0);
	LoadTexture(":Data:Textures:Quit.png",&Mainmenuitems[3],0,0);
	LoadTexture(":Data:Textures:World.png",&Mainmenuitems[7],0,0);
	LoadTexture(":Data:Textures:Eyelid.png",&Mainmenuitems[4],0,1);
	//LoadTexture(":Data:Textures:Eye.jpg",&Mainmenuitems[5],0,1);
	texdetail=temptexdetail;

	loaddistrib=0;
	anim=0;

	FadeLoadingScreen(95);


	gameon=0;
	mainmenu=1;

	stillloading=0;
	firstload=0;
	oldmainmenu=0;

	newdetail=detail;
	newscreenwidth=screenwidth;
	newscreenheight=screenheight;
}


void Game::LoadStuff()
{
	static float temptexdetail;
	static float viewdistdetail;
	static int i,j,texsize;
	float megascale =1;

	LOGFUNC;

	visibleloading=1;

	/*musicvolume[3]=512;
	PlaySoundEx( music4, samp[music4], NULL, true);
	OPENAL_SetPaused(channels[music4], false);
	OPENAL_SetVolume(channels[music4], 512);
	*/
	loadtime=0;

	stillloading=1;

	//texture.data = ( GLubyte* )malloc( 1024*1024*4 );

	for(i=0;i<maxplayers;i++)
	{
		if (glIsTexture(player[i].skeleton.drawmodel.textureptr))
		{
			glDeleteTextures(1, &player[i].skeleton.drawmodel.textureptr);
		}
		player[i].skeleton.drawmodel.textureptr=0;;
	}

	//temptexdetail=texdetail;
	//texdetail=1;
	i=abs(Random()%4);
	LoadTexture(":Data:Textures:fire.jpg",&loadscreentexture,1,0);
	//texdetail=temptexdetail;

	temptexdetail=texdetail;
	texdetail=1;
	text.LoadFontTexture(":Data:Textures:Font.png");
	text.BuildFont();
	texdetail=temptexdetail;

	numsounds=71;

	viewdistdetail=2;
	viewdistance=50*megascale*viewdistdetail;

	brightness=100;



	if(detail==2){
		texdetail=1;
		terraindetail=1;
	}
	if(detail==1){
		texdetail=2;
		terraindetail=1;
	}
	if(detail==0){
		texdetail=4;
		terraindetail=1;
		//terraindetail=2;
	}

	realtexdetail=texdetail;

	/*texdetail/=4;
	if(texdetail<1)texdetail=1;
	realtexdetail=texdetail*4;
	*/
	numplayers=1;



	/*LoadTexture(":Data:Textures:snow.png",&terraintexture,1);

	LoadTexture(":Data:Textures:rock.png",&terraintexture2,1);

	LoadTexture(":Data:Textures:detail.png",&terraintexture3,1);
	*/


	LOG("Loading weapon data...");

	LoadTexture(":Data:Textures:knife.png",&weapons.knifetextureptr,0,1);
	LoadTexture(":Data:Textures:bloodknife.png",&weapons.bloodknifetextureptr,0,1);
	LoadTexture(":Data:Textures:lightbloodknife.png",&weapons.lightbloodknifetextureptr,0,1);
	LoadTexture(":Data:Textures:sword.jpg",&weapons.swordtextureptr,1,0);
	LoadTexture(":Data:Textures:Swordblood.jpg",&weapons.bloodswordtextureptr,1,0);
	LoadTexture(":Data:Textures:Swordbloodlight.jpg",&weapons.lightbloodswordtextureptr,1,0);
	LoadTexture(":Data:Textures:Staff.jpg",&weapons.stafftextureptr,1,0);

	weapons.throwingknifemodel.load((char *)":Data:Models:throwingknife.solid",1);
	weapons.throwingknifemodel.Scale(.001,.001,.001);
	//weapons.throwingknifemodel.Rotate(0,0,-90);
	weapons.throwingknifemodel.Rotate(90,0,0);
	weapons.throwingknifemodel.Rotate(0,90,0);
	weapons.throwingknifemodel.flat=0;
	weapons.throwingknifemodel.CalculateNormals(1);
	//weapons.throwingknifemodel.ScaleNormals(-1,-1,-1);

	weapons.swordmodel.load((char *)":Data:Models:sword.solid",1);
	weapons.swordmodel.Scale(.001,.001,.001);
	//weapons.swordmodel.Rotate(0,0,-90);
	weapons.swordmodel.Rotate(90,0,0);
	weapons.swordmodel.Rotate(0,90,0);
	weapons.swordmodel.Rotate(0,0,90);
	weapons.swordmodel.flat=1;
	weapons.swordmodel.CalculateNormals(1);
	//weapons.swordmodel.ScaleNormals(-1,-1,-1);

	weapons.staffmodel.load((char *)":Data:Models:staff.solid",1);
	weapons.staffmodel.Scale(.005,.005,.005);
	//weapons.staffmodel.Rotate(0,0,-90);
	weapons.staffmodel.Rotate(90,0,0);
	weapons.staffmodel.Rotate(0,90,0);
	weapons.staffmodel.Rotate(0,0,90);
	weapons.staffmodel.flat=1;
	weapons.staffmodel.CalculateNormals(1);
	//weapons.staffmodel.ScaleNormals(-1,-1,-1);

	//temptexdetail=texdetail;
	//if(texdetail>4)texdetail=4;
	LoadTexture(":Data:Textures:shadow.png",&terrain.shadowtexture,0,1);

	LoadTexture(":Data:Textures:blood.png",&terrain.bloodtexture,0,1);

	LoadTexture(":Data:Textures:break.png",&terrain.breaktexture,0,1);

	LoadTexture(":Data:Textures:blood.png",&terrain.bloodtexture2,0,1);


	LoadTexture(":Data:Textures:footprint.png",&terrain.footprinttexture,0,1);

	LoadTexture(":Data:Textures:bodyprint.png",&terrain.bodyprinttexture,0,1);

	/*LoadTexture(":Data:Textures:cloud.png",&Sprite::cloudtexture,1);

	LoadTexture(":Data:Textures:cloudimpact.png",&Sprite::cloudimpacttexture,1);

	LoadTexture(":Data:Textures:bloodparticle.png",&Sprite::bloodtexture,1);

	LoadTexture(":Data:Textures:snowflake.png",&Sprite::snowflaketexture,1);

	LoadTexture(":Data:Textures:flame.png",&Sprite::flametexture,1);

	LoadTexture(":Data:Textures:smoke.png",&Sprite::smoketexture,1);
	//texdetail=temptexdetail;
	LoadTexture(":Data:Textures:shine.png",&Sprite::shinetexture,1);*/



	LoadTexture(":Data:Textures:hawk.png",&hawktexture,0,1);

	LoadTexture(":Data:Textures:logo.png",&logotexture,0,1);


	//LoadTexture(":Data:Textures:box.jpg",&objects.boxtextureptr,1,0);


	LoadTexture(":Data:Textures:cloud.png",&Sprite::cloudtexture,1,1);
	LoadTexture(":Data:Textures:cloudimpact.png",&Sprite::cloudimpacttexture,1,1);
	LoadTexture(":Data:Textures:bloodparticle.png",&Sprite::bloodtexture,1,1);
	LoadTexture(":Data:Textures:snowflake.png",&Sprite::snowflaketexture,1,1);
	LoadTexture(":Data:Textures:flame.png",&Sprite::flametexture,1,1);
	LoadTexture(":Data:Textures:bloodflame.png",&Sprite::bloodflametexture,1,1);
	LoadTexture(":Data:Textures:smoke.png",&Sprite::smoketexture,1,1);
	LoadTexture(":Data:Textures:shine.png",&Sprite::shinetexture,1,0);
	LoadTexture(":Data:Textures:splinter.png",&Sprite::splintertexture,1,1);
	LoadTexture(":Data:Textures:leaf.png",&Sprite::leaftexture,1,1);
	LoadTexture(":Data:Textures:tooth.png",&Sprite::toothtexture,1,1);

	rotation=0;
	rotation2=0;
	ReSizeGLScene(90,.01);

	viewer=0;




	if(detail)kTextureSize=1024;
	if(detail==1)kTextureSize=512;
	if(detail==0)kTextureSize=256;


	//drawmode=motionblurmode;

	//Set up distant light
	light.color[0]=.95;
	light.color[1]=.95;
	light.color[2]=1;
	light.ambient[0]=.2;
	light.ambient[1]=.2;
	light.ambient[2]=.24;
	light.location.x=1;
	light.location.y=1;
	light.location.z=-.2;
	Normalise(&light.location);

	LoadingScreen();

	SetUpLighting();


	fadestart=.6;
	gravity=-10;

	texscale=.2/megascale/viewdistdetail;
	terrain.scale=3*megascale*terraindetail*viewdistdetail;

	viewer.x=terrain.size/2*terrain.scale;
	viewer.z=terrain.size/2*terrain.scale;

	hawk.load((char *)":Data:Models:hawk.solid",1);
	hawk.Scale(.03,.03,.03);
	hawk.Rotate(90,1,1);
	hawk.CalculateNormals(0);
	hawk.ScaleNormals(-1,-1,-1);
	hawkcoords.x=terrain.size/2*terrain.scale-5-7;
	hawkcoords.z=terrain.size/2*terrain.scale-5-7;
	hawkcoords.y=terrain.getHeight(hawkcoords.x,hawkcoords.z)+25;


	eye.load((char *)":Data:Models:eye.solid",1);
	eye.Scale(.03,.03,.03);
	eye.CalculateNormals(0);

	cornea.load((char *)":Data:Models:cornea.solid",1);
	cornea.Scale(.03,.03,.03);
	cornea.CalculateNormals(0);

	iris.load((char *)":Data:Models:iris.solid",1);
	iris.Scale(.03,.03,.03);
	iris.CalculateNormals(0);

	LoadSave(":Data:Textures:Bloodfur.png",0,1,&bloodText[0],0);
	LoadSave(":Data:Textures:Wolfbloodfur.png",0,1,&wolfbloodText[0],0);

	oldenvironment=-4;

	gameon=1;
	mainmenu=0;

	firstload=0;
	//if(targetlevel!=7)
		Loadlevel(targetlevel);


	rabbitcoords=player[0].coords;
	rabbitcoords.y=terrain.getHeight(rabbitcoords.x,rabbitcoords.z);

	loadAllAnimations();
	//Fix knife stab, too lazy to do it manually
	XYZ moveamount;
	moveamount=0;
	moveamount.z=2;
	for(i=0;i<player[0].skeleton.num_joints;i++){
		for(j=0;j<animation[knifesneakattackanim].numframes;j++){
			animation[knifesneakattackanim].position[i][j]+=moveamount;
		}
	}

	loadscreencolor=4;
	LoadingScreen();

	for(i=0;i<player[0].skeleton.num_joints;i++){
		for(j=0;j<animation[knifesneakattackedanim].numframes;j++){
			animation[knifesneakattackedanim].position[i][j]+=moveamount;
		}
	}

	loadscreencolor=4;
	LoadingScreen();

	for(i=0;i<player[0].skeleton.num_joints;i++){
		animation[dead1anim].position[i][1]=animation[dead1anim].position[i][0];
		animation[dead2anim].position[i][1]=animation[dead2anim].position[i][0];
		animation[dead3anim].position[i][1]=animation[dead3anim].position[i][0];
		animation[dead4anim].position[i][1]=animation[dead4anim].position[i][0];
	}
	animation[dead1anim].speed[0]=0.001;
	animation[dead2anim].speed[0]=0.001;
	animation[dead3anim].speed[0]=0.001;
	animation[dead4anim].speed[0]=0.001;

	animation[dead1anim].speed[1]=0.001;
	animation[dead2anim].speed[1]=0.001;
	animation[dead3anim].speed[1]=0.001;
	animation[dead4anim].speed[1]=0.001;

	for(i=0;i<player[0].skeleton.num_joints;i++){
		for(j=0;j<animation[swordsneakattackanim].numframes;j++){
			animation[swordsneakattackanim].position[i][j]+=moveamount;
		}
	}
	loadscreencolor=4;
	LoadingScreen();
	for(j=0;j<animation[swordsneakattackanim].numframes;j++){
		animation[swordsneakattackanim].weapontarget[j]+=moveamount;
	}

	loadscreencolor=4;
	LoadingScreen();

	for(i=0;i<player[0].skeleton.num_joints;i++){
		for(j=0;j<animation[swordsneakattackedanim].numframes;j++){
			animation[swordsneakattackedanim].position[i][j]+=moveamount;
		}
	}
	/*
	for(i=0;i<player[0].skeleton.num_joints;i++){
	for(j=0;j<animation[sleepanim].numframes;j++){
	animation[sleepanim].position[i][j]=DoRotation(animation[sleepanim].position[i][j],0,180,0);
	}
	}
	*/
	loadscreencolor=4;
	LoadingScreen();
	temptexdetail=texdetail;
	texdetail=1;
	texdetail=temptexdetail;

	loadscreencolor=4;
	LoadingScreen();

	//if(ismotionblur){
	if(!screentexture){
		glPixelStorei( GL_UNPACK_ALIGNMENT, 1 );

		glGenTextures( 1, &screentexture );
		glTexEnvi( GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE );


		glEnable(GL_TEXTURE_2D);
		glBindTexture( GL_TEXTURE_2D, screentexture);
		glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR );
		glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR );

		glCopyTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, 0, 0, kTextureSize, kTextureSize, 0);
	}

	if(targetlevel!=7){
		emit_sound_at(fireendsound);
	}

	stillloading=0;
	loading=0;
	changedelay=1;

	visibleloading=0;
}

