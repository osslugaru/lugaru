#include "Game.h"
#include "openal_wrapper.h"
#include "SDL_thread.h"

extern int mainmenu;
extern int channels[100];
extern "C" void PlaySoundEx(int channel, OPENAL_SAMPLE *sptr, OPENAL_DSPUNIT *dsp, signed char startpaused);



Game::Game()
{
	terraintexture = 0;
	terraintexture2 = 0;
	terraintexture3 = 0;
	screentexture = 0;
	screentexture2 = 0;
	logotexture = 0;
	loadscreentexture = 0;
	Maparrowtexture = 0;
	Mapboxtexture = 0;
	Mapcircletexture = 0;
	cursortexture = 0;

	memset(Mainmenuitems, 0, sizeof(Mainmenuitems));

	nummenuitems = 0;

	memset(startx, 0, sizeof(startx));
	memset(starty, 0, sizeof(starty));
	memset(endx, 0, sizeof(endx));
	memset(endy, 0, sizeof(endy));

	memset(selectedlong, 0, sizeof(selectedlong));
	memset(offsetx, 0, sizeof(offsetx));
	memset(offsety, 0, sizeof(offsety));
	memset(movex, 0, sizeof(movex));
	memset(movey, 0, sizeof(movey));
	memset(endy, 0, sizeof(endy));

	transition = 0;
	anim = 0;
	selected = 0;
	loaddistrib = 0;
	keyselect = 0;
	indemo = 0;

	won = 0;

	entername = 0;

	memset(menustring, 0, sizeof(menustring));
	memset(registrationname, 0, sizeof(registrationname));
	registrationnumber = 0;

	newdetail = 0;
	newscreenwidth = 0;
	newscreenheight = 0;

	gameon = 0;
	deltah = 0,deltav = 0;
	mousecoordh = 0,mousecoordv = 0;
	oldmousecoordh = 0,oldmousecoordv = 0;
	rotation = 0,rotation2 = 0;

//	SkyBox skybox;

	cameramode = 0;
	cameratogglekeydown = 0;
	chattogglekeydown = 0;
	olddrawmode = 0;
	drawmode = 0;
	drawmodetogglekeydown = 0;
	explodetogglekeydown = 0;
	detailtogglekeydown = 0;
	firstload = 0;
	oldbutton = 0;

	leveltime = 0;
	loadtime = 0;

//	Model hawk;

//	XYZ hawkcoords;
//	XYZ realhawkcoords;

	hawktexture = 0;
	hawkrotation = 0;
	hawkcalldelay = 0;
/*
	Model eye;
	Model iris;
	Model cornea;
*/
	stealthloading = 0;

	campaignnumlevels = 0;

	memset(campaignmapname, 0, sizeof(campaignmapname));
	memset(campaigndescription, 0, sizeof(campaigndescription));
	memset(campaignchoosenext, 0, sizeof(campaignchoosenext));
	memset(campaignnumnext, 0, sizeof(campaignnumnext));
	memset(campaignnextlevel, 0, sizeof(campaignnextlevel));
	int campaignchoicesmade;
	memset(campaignchoices, 0, sizeof(campaignchoices));
	memset(campaignlocationx, 0, sizeof(campaignlocationx));
	memset(campaignlocationy, 0, sizeof(campaignlocationy));
	memset(campaignlocationy, 0, sizeof(campaignlocationy));

	campaignchoicenum = 0;

	memset(campaignchoicewhich, 0, sizeof(campaignchoicewhich));

	whichchoice = 0;

	numlevelspassed = 0;

	memset(levelorder, 0, sizeof(levelorder));
	memset(levelvisible, 0, sizeof(levelvisible));
	memset(levelhighlight, 0, sizeof(levelhighlight));

	minimap = 0;

	musictype = 0,oldmusictype = 0,oldoldmusictype = 0;
	realthreat = 0;

//	Model rabbit;
//	XYZ rabbitcoords;

//	XYZ mapcenter;
	mapradius = 0;

//	Text text;
	fps = 0;

//	XYZ cameraloc;
	cameradist = 0;

	envtogglekeydown = 0;
	slomotogglekeydown = 0;
	texturesizetogglekeydown = 0;
	freezetogglekeydown = 0;
	drawtoggle = 0;

	editorenabled = 0;
	editortype = 0;
	editorsize = 0;
	editorrotation = 0;
	editorrotation2 = 0;

	brightness = 0;

	quit = 0;
	tryquit = 0;

//	XYZ pathpoint[30];
	numpathpoints = 0;
	memset(numpathpointconnect, 0, sizeof(numpathpointconnect));
	memset(pathpointconnect, 0, sizeof(pathpointconnect));
	pathpointselected = 0;

	endgame = 0;
	scoreadded = 0;
	numchallengelevels = 0;

	console = 0;
	archiveselected = 0;

	memset(consoletext, 0, sizeof(consoletext));
	memset(consolechars, 0, sizeof(consolechars));
	chatting = 0;
	memset(displaytext, 0, sizeof(displaytext));
	memset(displaychars, 0, sizeof(displaychars));
	memset(displaytime, 0, sizeof(displaytime));
	displayblinkdelay = 0;
	displayblink = 0;
	displayselected = 0;
	consolekeydown = 0;
	consoletogglekeydown = 0;
	consoleblinkdelay = 0;
	consoleblink = 0;
	consoleselected = 0;
//	memset(togglekey, 0, sizeof(togglekey));
//	memset(togglekeydelay, 0, sizeof(togglekeydelay));
	registernow = 0;
	autocam = 0;

	crouchkey = 0,jumpkey = 0,forwardkey = 0,chatkey = 0,backkey = 0,leftkey = 0,rightkey = 0,drawkey = 0,throwkey = 0,attackkey = 0;
	oldattackkey = 0;

	loading = 0;
	talkdelay = 0;

	numboundaries = 0;
//	XYZ boundary[360];

	whichlevel = 0;
	oldenvironment = 0;
	targetlevel = 0;
	changedelay = 0;

	memset(musicvolume, 0, sizeof(musicvolume));
	memset(oldmusicvolume, 0, sizeof(oldmusicvolume));
	musicselected = 0;
	change = 0;
	
//------------

	waiting = false;
	mainmenu = 0;
	
	accountactive = NULL;
}

typedef struct {
	Game* game;
	void (Game::*method)();
} params_thread;

void Game::fireSound(int sound) {
	float gLoc[3]={0,0,0};
	float vel[3]={0,0,0};
	PlaySoundEx( sound, samp[sound], NULL, true);
	OPENAL_3D_SetAttributes(channels[sound], gLoc, vel);
	OPENAL_SetVolume(channels[sound], 256);
	OPENAL_SetPaused(channels[sound], false);
}

void Game::inputText(char* str, int* charselected, int* nb_chars) {
	SDL_Event evenement;
	int i;
	
	if(!waiting) {
		waiting=true;
		SDL_EnableKeyRepeat(SDL_DEFAULT_REPEAT_DELAY,SDL_DEFAULT_REPEAT_INTERVAL);
		SDL_EnableUNICODE(true);
	}

	SDL_PollEvent(&evenement);
	
	switch(evenement.type) {
		case SDL_KEYDOWN:
			if(evenement.key.keysym.sym == SDLK_ESCAPE) {
				for(i=0;i<255;i++){
					str[i]=' ';
				}
				*nb_chars=0;
				*charselected=0;
				waiting=false;
			} else if(evenement.key.keysym.sym==SDLK_BACKSPACE){
				if((*charselected)!=0) {
					for(i=(*charselected)-1;i<255;i++){
						str[i]=str[i+1];
					}
					str[255]=' ';
					(*charselected)--;
					(*nb_chars)--;
				}
			} else if(evenement.key.keysym.sym==SDLK_DELETE){
				for(i=(*charselected);i<255;i++){
					str[i]=str[i+1];
				}
				str[255]=' ';
				(*nb_chars)--;
			} else if(evenement.key.keysym.sym==SDLK_LEFT){
				if((*charselected)!=0)
					(*charselected)--;
			} else if(evenement.key.keysym.sym==SDLK_RIGHT){
				if((*charselected)<(*nb_chars))
					(*charselected)++;
			} else if(evenement.key.keysym.sym==SDLK_RETURN) {
				waiting=false;
			} else if((evenement.key.keysym.unicode<127)&&((*nb_chars)<60)&&(evenement.key.keysym.sym!=SDLK_LSHIFT)&&(evenement.key.keysym.sym!=SDLK_RSHIFT)) {
				for(i=255;i>=(*charselected)+1;i--){
					str[i]=str[i-1];
				}
				str[*charselected]=evenement.key.keysym.unicode;
				(*charselected)++;
				(*nb_chars)++;
			}
		break;
	}
	
	if(!waiting) {
		SDL_EnableKeyRepeat(0,0); // disable key repeat
		SDL_EnableUNICODE(false);
	}
}

void Game::setKeySelected() {
	waiting=true;
	params_thread* data = new params_thread;
    data->game = this;
    data->method = &Game::setKeySelected_thread;
    printf("launch thread\n");
	SDL_Thread* thread = SDL_CreateThread(Game::thread, data);
    if ( thread == NULL ) {
        fprintf(stderr, "Unable to create thread: %s\n", SDL_GetError());
		waiting=false;
        return;
    }
}

void Game::setKeySelected_thread() {
	int keycode=-1;
	SDL_Event evenement;
	while(keycode==-1) {
		SDL_WaitEvent(&evenement);
		switch(evenement.type) {
			case SDL_KEYDOWN:
				keycode = evenement.key.keysym.sym;
			break;
			case SDL_MOUSEBUTTONDOWN:
				keycode = SDLK_LAST+evenement.button.button;
			break;
			default:
			break;
		}
	}
	if(keycode != SDLK_ESCAPE) {
		fireSound();
		switch(keyselect) {
			case 0: forwardkey=keycode;
			break;
			case 1: backkey=keycode;
			break;
			case 2: leftkey=keycode;
			break;
			case 3: rightkey=keycode;
			break;
			case 4: crouchkey=keycode;
			break;
			case 5: jumpkey=keycode;
			break;
			case 6: drawkey=keycode;
			break;
			case 7: throwkey=keycode;
			break;
			case 8: attackkey=keycode;
			break;
			default:
			break;
		}
	}
	keyselect=-1;
	waiting=false;
}

int Game::thread(void *data) {
	params_thread* pt = (params_thread*)data;
    if(pt) {
        (pt->game->*(pt->method))();
    }
}

void Game::DrawGL() {
	if ( stereomode == stereoNone ) {
		DrawGLScene(stereoCenter);
	} else {
		DrawGLScene(stereoLeft);
		DrawGLScene(stereoRight);
	}
}
