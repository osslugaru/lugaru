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
#include "Texture.h"

extern float screenwidth,screenheight;
extern float viewdistance;
extern XYZ viewer;
extern XYZ lightlocation;
extern float fadestart;
extern float texscale;
extern float gravity;
extern Light light;
extern Skeleton testskeleton;
extern int numsounds;
extern Terrain terrain;
//extern Sprites sprites;
extern int kTextureSize;
extern float texdetail;
extern float realtexdetail;
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
extern bool visibleloading;
extern float flashamount,flashr,flashg,flashb;
extern int flashdelay;
extern int whichjointstartarray[26];
extern int whichjointendarray[26];
extern int difficulty;
extern float tintr,tintg,tintb;
extern float slomospeed;
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
extern float dialoguecamerayaw[20][20];
extern float dialoguecamerapitch[20][20];
extern int indialogue;
extern int whichdialogue;
extern float dialoguetime;

extern float accountcampaignhighscore[10];
extern float accountcampaignfasttime[10];
extern float accountcampaignscore[10];
extern float accountcampaigntime[10];

extern int accountcampaignchoicesmade[10];
extern int accountcampaignchoices[10][5000];

void LOG(const std::string &fmt, ...)
{
    // !!! FIXME: write me.
}

void Dispose()
{
	LOGFUNC;

	if(Game::endgame==2){
        Game::accountactive->endGame();
        Game::endgame=0;
	}

	Account::saveFile(":Data:Users", Game::accountactive);

	//textures.clear();

	LOG("Shutting down sound system...");

	OPENAL_StopSound(OPENAL_ALL);

// this is causing problems on Linux, but we'll force an _exit() a little
//  later in the shutdown process.  --ryan.
#if !PLATFORM_LINUX

	for (int i=0; i < sounds_count; ++i)
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

void Game::newGame(){
    text = new Text();
    skybox = new SkyBox();
}

void Game::deleteGame(){
    if(skybox)
        delete skybox;
    if(text)
        delete text;
    glDeleteTextures( 10, &Mainmenuitems[0] );
    glDeleteTextures( 1, &cursortexture );
    glDeleteTextures( 1, &Maparrowtexture );
    glDeleteTextures( 1, &Mapboxtexture );
    glDeleteTextures( 1, &Mapcircletexture );
    glDeleteTextures( 1, &terraintexture );
    glDeleteTextures( 1, &terraintexture2 );
    glDeleteTextures( 1, &screentexture );
    glDeleteTextures( 1, &screentexture2 );
    glDeleteTextures( 1, &hawktexture );
    glDeleteTextures( 1, &logotexture );
    glDeleteTextures( 1, &loadscreentexture );

    Dispose();
}



void Game::LoadTexture(const string fileName, GLuint *textureid,int mipmap, bool hasalpha) {
	*textureid = Texture::Load(fileName,mipmap,hasalpha);
}

void Game::LoadTextureSave(const string fileName, GLuint *textureid,int mipmap,GLubyte *array, int *skinsize) {
	*textureid = Texture::Load(fileName,mipmap,false,array,skinsize);
}

void LoadSave(const char *fileName, GLuint *textureid,bool mipmap,GLubyte *array, int *skinsize)
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
	static float loadprogress;
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
	}
}

void FadeLoadingScreen(float howmuch)
{
	static float loadprogress;

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
	swap_gl_buffers();
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
	text->LoadFontTexture(":Data:Textures:Font.png");
	text->BuildFont();
	texdetail=temptexdetail;

	FadeLoadingScreen(10);

	if(detail==2){
		texdetail=1;
	}
	if(detail==1){
		texdetail=2;
	}
	if(detail==0){
		texdetail=4;
	}

	LOG("Initializing sound system...");

    #if PLATFORM_LINUX
    int output = -1;
    
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

	if(musictoggle)
	  emit_stream_np(stream_menutheme);

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
	LoadTexture(":Data:Textures:Eyelid.png",&Mainmenuitems[4],0,1);
	LoadTexture(":Data:Textures:Resume.png",&Mainmenuitems[5],0,0);
	LoadTexture(":Data:Textures:Endgame.png",&Mainmenuitems[6],0,0);
	
	//LoadTexture(":Data:Textures:Eye.jpg",&Mainmenuitems[5],0,1);
	//~ LoadTexture(":Data:Textures:World.png",&Mainmenuitems[7],0,0); // LoadCampaign will take care of that
	texdetail=temptexdetail;

	FadeLoadingScreen(95);


	gameon=0;
	mainmenu=1;

	stillloading=0;
	firstload=0;

	newdetail=detail;
	newscreenwidth=screenwidth;
	newscreenheight=screenheight;

    LoadMenu();
}


void LoadScreenTexture() {
    glPixelStorei( GL_UNPACK_ALIGNMENT, 1 );

    if(!Game::screentexture)
        glGenTextures( 1, &Game::screentexture );
    glTexEnvi( GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE );


    glEnable(GL_TEXTURE_2D);
    glBindTexture( GL_TEXTURE_2D, Game::screentexture);
    glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR );
    glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR );

    glCopyTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, 0, 0, kTextureSize, kTextureSize, 0);
}

//TODO: move LoadStuff() closer to GameTick.cpp to get rid of various vars shared in Game.h
void Game::LoadStuff()
{
	static float temptexdetail;
	static float viewdistdetail;
	static int i,j;
	float megascale =1;

	LOGFUNC;

	visibleloading=1;

	loadtime=0;

	stillloading=1;

	for(i=0;i<maxplayers;i++)
	{
        glDeleteTextures(1, &player[i].skeleton.drawmodel.textureptr);
		player[i].skeleton.drawmodel.textureptr=0;;
	}

	i=abs(Random()%4);
	LoadTexture(":Data:Textures:fire.jpg",&loadscreentexture,1,0);

	temptexdetail=texdetail;
	texdetail=1;
	text->LoadFontTexture(":Data:Textures:Font.png");
	text->BuildFont();
	texdetail=temptexdetail;

	numsounds=71;

	viewdistdetail=2;
	viewdistance=50*megascale*viewdistdetail;

	if(detail==2){
		texdetail=1;
	}
	if(detail==1){
		texdetail=2;
	}
	if(detail==0){
		texdetail=4;
	}

	realtexdetail=texdetail;

	numplayers=1;


	LOG("Loading weapon data...");

	LoadTexture(":Data:Textures:knife.png",&Weapon::knifetextureptr,0,1);
	LoadTexture(":Data:Textures:bloodknife.png",&Weapon::bloodknifetextureptr,0,1);
	LoadTexture(":Data:Textures:lightbloodknife.png",&Weapon::lightbloodknifetextureptr,0,1);
	LoadTexture(":Data:Textures:sword.jpg",&Weapon::swordtextureptr,1,0);
	LoadTexture(":Data:Textures:Swordblood.jpg",&Weapon::bloodswordtextureptr,1,0);
	LoadTexture(":Data:Textures:Swordbloodlight.jpg",&Weapon::lightbloodswordtextureptr,1,0);
	LoadTexture(":Data:Textures:Staff.jpg",&Weapon::stafftextureptr,1,0);

	Weapon::throwingknifemodel.load((char *)":Data:Models:throwingknife.solid",1);
	Weapon::throwingknifemodel.Scale(.001,.001,.001);
	//Weapon::throwingknifemodel.Rotate(0,0,-90);
	Weapon::throwingknifemodel.Rotate(90,0,0);
	Weapon::throwingknifemodel.Rotate(0,90,0);
	Weapon::throwingknifemodel.flat=0;
	Weapon::throwingknifemodel.CalculateNormals(1);
	//Weapon::throwingknifemodel.ScaleNormals(-1,-1,-1);

	Weapon::swordmodel.load((char *)":Data:Models:sword.solid",1);
	Weapon::swordmodel.Scale(.001,.001,.001);
	//Weapon::swordmodel.Rotate(0,0,-90);
	Weapon::swordmodel.Rotate(90,0,0);
	Weapon::swordmodel.Rotate(0,90,0);
	Weapon::swordmodel.Rotate(0,0,90);
	Weapon::swordmodel.flat=1;
	Weapon::swordmodel.CalculateNormals(1);
	//Weapon::swordmodel.ScaleNormals(-1,-1,-1);

	Weapon::staffmodel.load((char *)":Data:Models:staff.solid",1);
	Weapon::staffmodel.Scale(.005,.005,.005);
	//Weapon::staffmodel.Rotate(0,0,-90);
	Weapon::staffmodel.Rotate(90,0,0);
	Weapon::staffmodel.Rotate(0,90,0);
	Weapon::staffmodel.Rotate(0,0,90);
	Weapon::staffmodel.flat=1;
	Weapon::staffmodel.CalculateNormals(1);
	//Weapon::staffmodel.ScaleNormals(-1,-1,-1);

	LoadTexture(":Data:Textures:shadow.png",&terrain.shadowtexture,0,1);

	LoadTexture(":Data:Textures:blood.png",&terrain.bloodtexture,0,1);

	LoadTexture(":Data:Textures:break.png",&terrain.breaktexture,0,1);

	LoadTexture(":Data:Textures:blood.png",&terrain.bloodtexture2,0,1);


	LoadTexture(":Data:Textures:footprint.png",&terrain.footprinttexture,0,1);

	LoadTexture(":Data:Textures:bodyprint.png",&terrain.bodyprinttexture,0,1);

	LoadTexture(":Data:Textures:hawk.png",&hawktexture,0,1);

	LoadTexture(":Data:Textures:logo.png",&logotexture,0,1);


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

	yaw=0;
	pitch=0;
	ReSizeGLScene(90,.01);

	viewer=0;


	if(detail)kTextureSize=1024;
	if(detail==1)kTextureSize=512;
	if(detail==0)kTextureSize=256;
	
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
	terrain.scale=3*megascale*viewdistdetail;

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

	LoadingScreen();

	for(i=0;i<player[0].skeleton.num_joints;i++){
		for(j=0;j<animation[knifesneakattackedanim].numframes;j++){
			animation[knifesneakattackedanim].position[i][j]+=moveamount;
		}
	}

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
	LoadingScreen();
	for(j=0;j<animation[swordsneakattackanim].numframes;j++){
		animation[swordsneakattackanim].weapontarget[j]+=moveamount;
	}

	LoadingScreen();

	for(i=0;i<player[0].skeleton.num_joints;i++){
		for(j=0;j<animation[swordsneakattackedanim].numframes;j++){
			animation[swordsneakattackedanim].position[i][j]+=moveamount;
		}
	}
	
	LoadingScreen();
	temptexdetail=texdetail;
	texdetail=1;
	texdetail=temptexdetail;

	LoadingScreen();

	if(!screentexture){
        LoadScreenTexture();
	}

	if(targetlevel!=7){
		emit_sound_at(fireendsound);
	}

	stillloading=0;
	loading=0;
	changedelay=1;

	visibleloading=0;
}

