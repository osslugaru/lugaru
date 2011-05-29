#include "Game.h"
#include "openal_wrapper.h"
#include "SDL_thread.h"

extern int mainmenu;

int numdialogues;
int numdialogueboxes[max_dialogues];
int dialoguetype[max_dialogues];
int dialogueboxlocation[max_dialogues][max_dialoguelength];
float dialogueboxcolor[max_dialogues][max_dialoguelength][3];
int dialogueboxsound[max_dialogues][max_dialoguelength];
char dialoguetext[max_dialogues][max_dialoguelength][128];
char dialoguename[max_dialogues][max_dialoguelength][64];
XYZ dialoguecamera[max_dialogues][max_dialoguelength];
XYZ participantlocation[max_dialogues][10];
int participantfocus[max_dialogues][max_dialoguelength];
int participantaction[max_dialogues][max_dialoguelength];
float participantyaw[max_dialogues][10];
XYZ participantfacing[max_dialogues][max_dialoguelength][10];
float dialoguecamerayaw[max_dialogues][max_dialoguelength];
float dialoguecamerapitch[max_dialogues][max_dialoguelength];
int indialogue;
int whichdialogue;
int directing;
float dialoguetime;
int dialoguegonethrough[20];

namespace Game{
    GLuint terraintexture = 0;
    GLuint terraintexture2 = 0;
    GLuint terraintexture3 = 0;
    GLuint screentexture = 0;
    GLuint screentexture2 = 0;
    GLuint logotexture = 0;
    GLuint loadscreentexture = 0;
    GLuint Maparrowtexture = 0;
    GLuint Mapboxtexture = 0;
    GLuint Mapcircletexture = 0;
    GLuint cursortexture = 0;
    GLuint Mainmenuitems[10] = {};

    int selected = 0;
    int keyselect = 0;

    int newdetail = 0;
    int newscreenwidth = 0;
    int newscreenheight = 0;

    bool gameon = 0;
    float deltah = 0;
    float deltav = 0;
    int mousecoordh = 0;
    int mousecoordv = 0;
    int oldmousecoordh = 0;
    int oldmousecoordv = 0;
    float yaw = 0;
    float pitch = 0;
    SkyBox *skybox = NULL;
    bool cameramode = 0;
    bool firstload = 0;

	GLuint hawktexture = 0;
	float hawkyaw = 0;
	float hawkcalldelay = 0;
    float leveltime = 0;
    float loadtime = 0;

    Model hawk;
    XYZ hawkcoords;
    XYZ realhawkcoords;

    Model eye;
    Model iris;
    Model cornea;

    bool stealthloading = 0;

    int musictype = 0;

    XYZ mapcenter;
    float mapradius = 0;

    Text *text = NULL;
    float fps = 0;

    bool editorenabled = 0;
    int editortype = 0;
    float editorsize = 0;
    float editoryaw = 0;
    float editorpitch = 0;

    int tryquit = 0;

    XYZ pathpoint[30];
    int numpathpoints = 0;
    int numpathpointconnect[30] = {};
    int pathpointconnect[30][30] = {};
    int pathpointselected = 0;

    int endgame = 0;
    bool scoreadded = 0;
    int numchallengelevels = 0;

    bool console = false;
    char consoletext[15][256] = {};
    int consolechars[15] = {};
    bool chatting = 0;
    char displaytext[15][256] = {};
    int displaychars[15] = {};
    float displaytime[15] = {};
    float displayblinkdelay = 0;
    bool displayblink = 0;
    int displayselected = 0;
    float consoleblinkdelay = 0;
    bool consoleblink = 0;
    int consoleselected = 0;

    unsigned short crouchkey=0,jumpkey=0,forwardkey=0,chatkey=0,backkey=0,leftkey=0,rightkey=0,drawkey=0,throwkey=0,attackkey=0;
    unsigned short consolekey=0;

    int loading = 0;
    
    int oldenvironment = 0;
    int targetlevel = 0;
    float changedelay = 0;

    bool waiting = false;
    Account* accountactive = NULL;
}

void Game::fireSound(int sound) {
	emit_sound_at(sound);
}

void Game::inputText(char* str, int* charselected, int* nb_chars) {
	SDL_Event evenement;
	
	if(!waiting) {
		waiting=true;
		SDL_EnableKeyRepeat(SDL_DEFAULT_REPEAT_DELAY,SDL_DEFAULT_REPEAT_INTERVAL);
		SDL_EnableUNICODE(true);
	}

	while(SDL_PollEvent(&evenement)) {
	
		switch(evenement.type) {
			case SDL_KEYDOWN:
				if(evenement.key.keysym.sym == SDLK_ESCAPE) {
					for(int i=0;i<255;i++)
						str[i]=0;
					*nb_chars=0;
					*charselected=0;
					waiting=false;
				} else if(evenement.key.keysym.sym==SDLK_BACKSPACE){
					if((*charselected)!=0) {
						for(int i=(*charselected)-1;i<255;i++)
							str[i]=str[i+1];
						str[255]=0;
						(*charselected)--;
						(*nb_chars)--;
					}
				} else if(evenement.key.keysym.sym==SDLK_DELETE){
					if((*charselected)<(*nb_chars)){
						for(int i=(*charselected);i<255;i++)
							str[i]=str[i+1];
						str[255]=0;
						(*nb_chars)--;
					}
				} else if(evenement.key.keysym.sym==SDLK_HOME){
					(*charselected)=0;
				} else if(evenement.key.keysym.sym==SDLK_END){
					(*charselected)=(*nb_chars);
				} else if(evenement.key.keysym.sym==SDLK_LEFT){
					if((*charselected)!=0)
						(*charselected)--;
				} else if(evenement.key.keysym.sym==SDLK_RIGHT){
					if((*charselected)<(*nb_chars))
						(*charselected)++;
				} else if(evenement.key.keysym.sym==SDLK_RETURN) {
					waiting=false;
				} else if(evenement.key.keysym.unicode>=32&&evenement.key.keysym.unicode<127&&(*nb_chars)<60){
					for(int i=255;i>=(*charselected)+1;i--)
						str[i]=str[i-1];
					str[*charselected]=evenement.key.keysym.unicode;
					(*charselected)++;
					(*nb_chars)++;
				}
			break;
		}
	}
	
	if(!waiting) {
		SDL_EnableKeyRepeat(0,0); // disable key repeat
		SDL_EnableUNICODE(false);
	}
}

int setKeySelected_thread(void* data) {
    using namespace Game;
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
			case 9: consolekey=keycode;
			break;
			default:
			break;
		}
	}
	keyselect=-1;
	waiting=false;
    LoadMenu();
    return 0;
}

void Game::setKeySelected() {
	waiting=true;
    printf("launch thread\n");
	SDL_Thread* thread = SDL_CreateThread(setKeySelected_thread,NULL);
    if ( thread == NULL ) {
        fprintf(stderr, "Unable to create thread: %s\n", SDL_GetError());
		waiting=false;
        return;
    }
}
