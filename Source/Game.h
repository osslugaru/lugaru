#ifndef _GAME_H_
#define _GAME_H_

#ifndef WIN32
#include <Carbon.h>
#include "Quicktime.h"
#endif
//Jordan included glut.h
//#include <glut.h>

#include "TGALoader.h"
#ifdef WIN32
#include "WinInput.h"
#else
#include "Macinput.h"
#endif
#include "Terrain.h"
#include "Skybox.h"
#include "Skeleton.h"
#include "Models.h"   
#include "Lights.h"
#include "Person.h"
#include "Constants.h"
#include "fmod.h"
#include "Sprites.h"
//#include <agl.h>
#include "Text.h"
#include "Objects.h"
//#include <DrawSprocket.h>
#include "Weapons.h"
#include "binio.h"
#include <fstream>
#include "gl.h"

extern GLuint rabbittexture;


class Game		
{
public:

	typedef std::map<std::string, GLuint> TextureList;
	typedef std::map<GLuint, std::string> GLTextureList;
	typedef TextureList::iterator TexIter;
	static TextureList textures;

	GLuint terraintexture;
	GLuint terraintexture2;
	GLuint terraintexture3;
	GLuint screentexture;
	GLuint screentexture2;
	GLuint logotexture;
	GLuint loadscreentexture;
	GLuint Maparrowtexture;
	GLuint Mapboxtexture;
	GLuint Mapcircletexture;
	GLuint cursortexture;
	GLuint Mainmenuitems[10];

	int nummenuitems;
	int startx[100];
	int starty[100];
	int endx[100];
	int endy[100];
	float selectedlong[100];
	float offsetx[100];
	float offsety[100];
	float movex[100];
	float movey[100];
	float transition;
	int anim;
	int selected;
	int loaddistrib;
	int keyselect;
	int indemo;
	int registered;

	bool won;

	bool entername;

	char menustring[100][256];
	char registrationname[256];
	float registrationnumber;

	int newdetail;
	int newscreenwidth;
	int newscreenheight;

	bool gameon;
	float deltah,deltav;
	int mousecoordh,mousecoordv;
	int oldmousecoordh,oldmousecoordv;
	float rotation,rotation2;
	SkyBox skybox;
	bool cameramode;
	bool cameratogglekeydown;
	bool chattogglekeydown;
	int olddrawmode;
	int drawmode;
	bool drawmodetogglekeydown;
	bool explodetogglekeydown;
	bool detailtogglekeydown;
	bool firstload;
	bool oldbutton;

	float leveltime;
	float loadtime;

	Model hawk;
	XYZ hawkcoords;
	XYZ realhawkcoords;
	GLuint hawktexture;
	float hawkrotation;
	float hawkcalldelay;

	Model eye;
	Model iris;
	Model cornea;

	bool stealthloading;

	int campaignnumlevels;
	char campaignmapname[50][256];
	char campaigndescription[50][256];
	int campaignchoosenext[50];
	int campaignnumnext[50];
	int campaignnextlevel[50][10];
	int campaignchoicesmade;
	int campaignchoices[5000];
	int campaignlocationx[50];
	int campaignlocationy[50];
	int campaignchoicenum;
	int campaignchoicewhich[10];
	int whichchoice;

	int numlevelspassed;
	int levelorder[5000];
	int levelvisible[50];
	int levelhighlight[50];

	bool minimap;

	int musictype,oldmusictype,oldoldmusictype;
	bool realthreat;

	Model rabbit;
	XYZ rabbitcoords;

	XYZ mapcenter;
	float mapradius;

	Text text;
	float fps;

	XYZ cameraloc;
	float cameradist;

	bool envtogglekeydown;
	bool slomotogglekeydown;
	bool texturesizetogglekeydown;
	bool freezetogglekeydown;
	int drawtoggle;

	bool editorenabled;
	int editortype;
	float editorsize;
	float editorrotation;
	float editorrotation2;

	float brightness;

	int quit;
	int tryquit;

	XYZ pathpoint[30];
	int numpathpoints;
	int numpathpointconnect[30];
	int pathpointconnect[30][30];
	int pathpointselected;

	int endgame;
	bool scoreadded;
	int numchallengelevels;

	bool console;
	int archiveselected;
	char consoletext[15][256];
	int consolechars[15];
	bool chatting;
	char displaytext[15][256];
	int displaychars[15];
	float displaytime[15];
	float displayblinkdelay;
	bool displayblink;
	int displayselected;
	bool consolekeydown;
	bool consoletogglekeydown;
	float consoleblinkdelay;
	bool consoleblink;
	int consoleselected;
	int togglekey[140];
	float togglekeydelay[140];
	bool registernow;
	bool autocam;

	unsigned short crouchkey,jumpkey,forwardkey,chatkey,backkey,leftkey,rightkey,drawkey,throwkey,attackkey;
	bool oldattackkey;

	long long MD5_string (char *string);
	static void LoadTexture(char *fileName, GLuint *textureid,int mipmap, bool hasalpha);
	static void LoadTextureSave(char *fileName, GLuint *textureid,int mipmap,GLubyte *array, int *skinsize);
	void LoadSave(char *fileName, GLuint *textureid,bool mipmap,GLubyte *array, int *skinsize);
	bool AddClothes(char *fileName, GLuint *textureid,bool mipmap,GLubyte *array, int *skinsize);
	void InitGame();
	void LoadStuff();
	void LoadingScreen();
	void FadeLoadingScreen(float howmuch);
	void Dispose();
	int DrawGLScene(GLvoid);
	void Tick();
	void TickOnce();
	void TickOnceAfter();
	void SetUpLighting();
	void Loadlevel(int which);
	void Loadlevel(char *name);
	void LoadSounds();
	void Setenvironment(int which);
	GLvoid ReSizeGLScene(float fov, float near);
	int findPathDist(int start,int end);
	int checkcollide(XYZ startpoint, XYZ endpoint);
	int checkcollide(XYZ startpoint, XYZ endpoint, int what);
	int loading;
	float talkdelay;

	int numboundaries;
	XYZ boundary[360];

	int whichlevel;
	int oldenvironment;
	int targetlevel;
	float changedelay;

	float musicvolume[4];
	float oldmusicvolume[4];
	int musicselected;
	int change;
	Game();
	~Game() {		
		for(int i=0;i<10;i++){
			if(Mainmenuitems[i])glDeleteTextures( 1, (const unsigned long *)&Mainmenuitems[i] );
		}
		glDeleteTextures( 1, (const unsigned long *)&cursortexture );
		glDeleteTextures( 1, (const unsigned long *)&Maparrowtexture );
		glDeleteTextures( 1, (const unsigned long *)&Mapboxtexture );
		glDeleteTextures( 1, (const unsigned long *)&Mapcircletexture );
		glDeleteTextures( 1, (const unsigned long *)&terraintexture );
		glDeleteTextures( 1, (const unsigned long *)&terraintexture2 );
		if(screentexture>0)glDeleteTextures( 1, (const unsigned long *)&screentexture );
		if(screentexture2>0)glDeleteTextures( 1, (const unsigned long *)&screentexture2 );
		glDeleteTextures( 1, (const unsigned long *)&hawktexture );
		glDeleteTextures( 1, (const unsigned long *)&logotexture );
		glDeleteTextures( 1, (const unsigned long *)&loadscreentexture );

		Dispose();
	}

};

#endif