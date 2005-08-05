#include "gamegl.h"
#include "Quaternions.h"
#include "Lights.h"
#include "Skeleton.h"
#include "fmod.h"
#include "Terrain.h"
#include "Sprites.h"
//#include <agl.h>
#include "Frustum.h"
#include "Objects.h"
#include "Weapons.h"
#include "Person.h"
#include "TGALoader.h"

#include "Constants.h"

bool visibleloading = 0;
FSOUND_SAMPLE	*samp[100] = {0};
FSOUND_STREAM * strm[20] = {0};
int channels[100] = {0};

float volume = 0;bool buttons[3] = {0};
bool oldbuttons[3] = {0};
bool ismotionblur = 0;
float usermousesensitivity = 0;
bool floatjump = 0;
bool cellophane = 0;
bool autoslomo = 0;
bool decals = 0;
bool invertmouse = 0;
bool texttoggle = 0;
float blurness = 0;
float targetblurness = 0;
float windvar = 0;
float precipdelay = 0;
float gamespeed = 0;
float oldgamespeed = 0;
float tintr = 0,tintg = 0,tintb = 0;
int difficulty = 0;
float multiplier = 0;
float realmultiplier = 0;
float screenwidth = 0,screenheight = 0;
float viewdistance = 0;
XYZ viewer;
XYZ viewerfacing;
XYZ lightlocation;
float fadestart = 0;
int environment = 0;
float texscale = 0;
float gravity = 0;
Light light;
Animation animation[animation_count];
Skeleton testskeleton;
int numsounds = 0;
Terrain terrain;
Sprites sprites;
float sps = 0;
#ifdef WIN32
HDC hDC;
#else
AGLContext gaglContext;
#endif
int kTextureSize = 0;
int detail = 0;
FRUSTUM frustum;
float texdetail = 0;
float realtexdetail = 0;
float terraindetail = 0;
float playerdist = 0;
Objects objects;
int slomo = 0;
float slomodelay = 0;
GLubyte bloodText[512*512*3] = {0};
GLubyte wolfbloodText[512*512*3] = {0};
float colors[3] = {0};
int bloodtoggle = 0;
bool osx = 0;
float camerashake = 0;
float woozy = 0;
float blackout = 0;
bool foliage = 0;
bool musictoggle = 0;
bool trilinear;
Weapons weapons;
bool damageeffects = 0;
//apvector<Person> player(maxplayers);
Person player[maxplayers];
int numplayers = 0;
bool ambientsound = 0;
bool mousejump = 0;
bool freeze = 0;
bool winfreeze = 0;
float flashamount = 0,flashr = 0,flashg = 0,flashb = 0;
int flashdelay = 0;
bool vblsync = 0;
float motionbluramount = 0;
bool keyboardfrozen = 0;
int newnetmessages = 0;
char netmessages[256] = {0};
char mapname[256] = {0};
bool loadingstuff = 0;
bool stillloading = 0;
bool showpoints = 0;
bool alwaysblur = 0;
bool immediate = 0;
bool velocityblur = 0;
int test = 0;
XYZ windvector;
short vRefNum = 0;
long dirID = 0;
int mainmenu = 0;
int oldmainmenu = 0;
GLubyte texturearray[512*512*3] = {0};
int loadscreencolor = 0;
int whichjointstartarray[26] = {0};
int whichjointendarray[26] = {0};
int kBitsPerPixel = 0;

int numhotspots = 0;
XYZ hotspot[40];
int hotspottype[40] = {0};
float hotspotsize[40] = {0};
char hotspottext[40][256] = {0};
int currenthotspot = 0;	
int winhotspot = 0;
int windialogue = 0;
int killhotspot = 0;

float menupulse = 0;

int numdialogues = 0;
int numdialogueboxes[max_dialogues] = {0};
int dialoguetype[max_dialogues] = {0};
int dialogueboxlocation[max_dialogues][max_dialoguelength] = {0};
float dialogueboxcolor[max_dialogues][max_dialoguelength][3] = {0};
int dialogueboxsound[max_dialogues][max_dialoguelength] = {0};
char dialoguetext[max_dialogues][max_dialoguelength][128] = {0};
char dialoguename[max_dialogues][max_dialoguelength][64] = {0};
XYZ dialoguecamera[max_dialogues][max_dialoguelength] = {0};
XYZ participantlocation[max_dialogues][10] = {0};
int participantfocus[max_dialogues][max_dialoguelength] = {0};
int participantaction[max_dialogues][max_dialoguelength] = {0};
float participantrotation[max_dialogues][10] = {0};
XYZ participantfacing[max_dialogues][max_dialoguelength][10] = {0};
float dialoguecamerarotation[max_dialogues][max_dialoguelength] = {0};
float dialoguecamerarotation2[max_dialogues][max_dialoguelength] = {0};
int indialogue = 0;
int whichdialogue = 0;
int directing = 0;
float dialoguetime = 0;
int dialoguegonethrough[20] = {0};

float smoketex = 0;

float slomospeed = 0;
float slomofreq = 0;

int tutoriallevel = 0;
int tutorialstage = 0;
float tutorialstagetime = 0;
float tutorialmaxtime = 0;
float tutorialsuccess = 0;

bool againbonus = 0;

float damagedealt = 0;
float damagetaken = 0;

int maptype = 0;

int editoractive = 0;
int editorpathtype = 0;

bool reversaltrain = 0;
bool cananger = 0;
bool canattack = 0;

bool skyboxtexture = 0;
float skyboxr = 0;
float skyboxg = 0;
float skyboxb = 0;
float skyboxlightr = 0;
float skyboxlightg = 0;
float skyboxlightb = 0;

float bonusnum[100] = {0};

int hostile = 0;
float hostiletime = 0;

XYZ envsound[30] = {0};
float envsoundvol[30] = {0};
float envsoundlife[30] = {0};
int numenvsounds;


bool tilt2weird = 0;
bool tiltweird = 0;
bool midweird = 0;
bool proportionweird = 0;
bool vertexweird[6] = {0};
TGAImageRec texture;
bool debugmode = 0;

int oldbonus = 0;
int bonus = 0;
float bonusvalue = 0;
float bonustotal = 0;
float startbonustotal = 0;
float bonustime = 0;

int numaccounts = 0;
int accountactive = 0;
int accountdifficulty[10] = {0};
int accountprogress[10] = {0};
float accountpoints[10] = {0};
float accounthighscore[10][50] = {0};
float accountfasttime[10][50] = {0};
bool accountunlocked[10][60] = {0};
char accountname[10][256] = {0};
float accountcampaignhighscore[10] = {0};
float accountcampaignfasttime[10] = {0};
float accountcampaignscore[10] = {0};
float accountcampaigntime[10] = {0};
int accountcampaignchoicesmade[10] = {0};int accountcampaignchoices[10][5000] = {0};
bool won = 0;


bool campaign = 0;

int numfalls = 0;
int numflipfail = 0;
int numseen = 0;
int numresponded = 0;
int numstaffattack = 0;
int numswordattack = 0;
int numknifeattack = 0;
int numunarmedattack = 0;
int numescaped = 0;
int numflipped = 0;
int numwallflipped = 0;
int numthrowkill = 0;
int numafterkill = 0;
int numreversals = 0;
int numattacks = 0;
int maxalarmed = 0;

bool gamestarted = 0;

//TextureList textures;
