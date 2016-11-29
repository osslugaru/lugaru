/*
Copyright (C) 2003, 2010 - Wolfire Games
Copyright (C) 2010-2016 - Lugaru contributors (see AUTHORS file)

This file is part of Lugaru.

Lugaru is free software; you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation; either version 2 of the License, or
(at your option) any later version.

Lugaru is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with Lugaru.  If not, see <http://www.gnu.org/licenses/>.
*/

// Enable full math definitions
#define _USE_MATH_DEFINES

#if PLATFORM_UNIX
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>
#else
#include <direct.h>
#endif


#include <limits>
#include <ctime>
#include <cmath>
#include <dirent.h>
#include "Game.h"
#include "openal_wrapper.h"
#include "Settings.h"
#include "Input.h"
#include "Animation.h"
#include "Awards.h"
#include "Menu.h"
#include "ConsoleCmds.h"

#include <algorithm>
#include <set>

using namespace std;
using namespace Game;

// Added more evilness needed for MSVC
#ifdef _MSC_VER
#define strncasecmp(s1, s2, n) _strnicmp(s1, s2, n)
#define snprintf(buf, size, format, ...) _sprintf_p(buf, size, format)
#endif


extern float multiplier;
extern XYZ viewer;
extern int environment;
extern Terrain terrain;
extern float screenwidth, screenheight;
extern float gravity;
extern int detail;
extern float texdetail;
extern Objects objects;
extern int slomo;
extern float slomodelay;
extern bool floatjump;
extern float volume;
extern Light light;
extern float camerashake;
extern float woozy;
extern float blackout;
extern bool cellophane;
extern bool musictoggle;
extern int difficulty;
extern int bloodtoggle;
extern bool invertmouse;
extern float windvar;
extern float precipdelay;
extern XYZ viewerfacing;
extern bool ambientsound;
extern bool mousejump;
extern float viewdistance;
extern bool freeze;
extern XYZ windvector;
extern bool debugmode;
static int leveltheme;
extern int mainmenu;
extern int oldmainmenu;
extern bool visibleloading;
extern XYZ envsound[30];
extern float envsoundvol[30];
extern int numenvsounds;
extern float envsoundlife[30];
extern float usermousesensitivity;
extern bool ismotionblur;
extern bool showdamagebar; // (des)activate the damage bar
extern bool decals;
extern bool skyboxtexture;
extern float skyboxr;
extern float skyboxg;
extern float skyboxb;
extern float skyboxlightr;
extern float skyboxlightg;
extern float skyboxlightb;
extern float fadestart;
extern float slomospeed;
extern float slomofreq;
extern int tutoriallevel;
extern float smoketex;
extern float tutorialstagetime;
extern int tutorialstage;
extern float tutorialmaxtime;
extern float tutorialsuccess;
extern bool againbonus;
extern bool reversaltrain;
extern bool canattack;
extern bool cananger;
extern float damagedealt;
extern int maptype;
extern int editoractive;
extern int editorpathtype;

extern float hostiletime;

extern bool gamestarted;

extern int numhotspots;
extern int killhotspot;
extern XYZ hotspot[40];
extern int hotspottype[40];
extern float hotspotsize[40];
extern char hotspottext[40][256];
extern int currenthotspot;

extern int hostile;

extern bool stillloading;
extern bool winfreeze;

extern bool campaign;

extern void toggleFullscreen();

class CampaignLevel
{
private:
    int width;
    struct Position {
        int x, y;
    };
public:
    std::string mapname;
    std::string description;
    int choosenext;
    /*
    0 = Immediately load next level at the end of this one.
    1 = Go back to the world map.
    2 = Don't bring up the Fiery loading screen. Maybe other things, I've not investigated.
    */
    //int numnext; // 0 on final level. As David said: he meant to add story branching, but he eventually hadn't.
    std::vector<int> nextlevel;
    Position location;
    CampaignLevel() : width(10) {
        choosenext = 1;
        location.x = 0;
        location.y = 0;
    }
    int getStartX() {
        return 30 + 120 + location.x * 400 / 512;
    }
    int getStartY() {
        return 30 + 30 + (512 - location.y) * 400 / 512;
    }
    int getEndX() {
        return getStartX() + width;
    }
    int getEndY() {
        return getStartY() + width;
    }
    XYZ getCenter() {
        XYZ center;
        center.x = getStartX() + width / 2;
        center.y = getStartY() + width / 2;
        return center;
    }
    int getWidth() {
        return width;
    }
    istream& operator<< (istream& is) {
        is.ignore(256, ':');
        is.ignore(256, ':');
        is.ignore(256, ' ');
        is >> mapname;
        is.ignore(256, ':');
        is >> description;
        for (size_t pos = description.find('_'); pos != string::npos; pos = description.find('_', pos)) {
            description.replace(pos, 1, 1, ' ');
        }
        is.ignore(256, ':');
        is >> choosenext;
        is.ignore(256, ':');
        int numnext, next;
        is >> numnext;
        for (int j = 0; j < numnext; j++) {
            is.ignore(256, ':');
            is >> next;
            nextlevel.push_back(next - 1);
        }
        is.ignore(256, ':');
        is >> location.x;
        is.ignore(256, ':');
        is >> location.y;
        return is;
    }
    friend istream& operator>> (istream& is, CampaignLevel& cl) {
        return cl << is;
    }
};

bool won = false;
int entername = 0;
vector<CampaignLevel> campaignlevels;
int whichchoice = 0;
int actuallevel = 0;
bool winhotspot = false;
bool windialogue = false;
bool realthreat = 0;
XYZ cameraloc;
float cameradist = 0;
bool oldattackkey = 0;
int whichlevel = 0;
float musicvolume[4] = {};
float oldmusicvolume[4] = {};
int musicselected = 0;

const char *rabbitskin[] = {
    ":Data:Textures:Fur3.jpg",
    ":Data:Textures:Fur.jpg",
    ":Data:Textures:Fur2.jpg",
    ":Data:Textures:Lynx.jpg",
    ":Data:Textures:Otter.jpg",
    ":Data:Textures:Opal.jpg",
    ":Data:Textures:Sable.jpg",
    ":Data:Textures:Chocolate.jpg",
    ":Data:Textures:BW2.jpg",
    ":Data:Textures:WB2.jpg"
};

const char *wolfskin[] = {
    ":Data:Textures:Wolf.jpg",
    ":Data:Textures:Darkwolf.jpg",
    ":Data:Textures:Snowwolf.jpg"
};

const char **creatureskin[] = {rabbitskin, wolfskin};

#define STATIC_ASSERT(x) extern int s_a_dummy[2 * (!!(x)) - 1];
STATIC_ASSERT (rabbittype == 0 && wolftype == 1)

// utility functions

// TODO: this is slightly incorrect
inline float roughDirection(XYZ vec)
{
    Normalise(&vec);
    float angle = -asin(-vec.x) * 180 / M_PI;
    if (vec.z < 0)
        angle = 180 - angle;
    return angle;
}
inline float roughDirectionTo(XYZ start, XYZ end)
{
    return roughDirection(end - start);
}
inline float pitchOf(XYZ vec)
{
    Normalise(&vec);
    return -asin(vec.y) * 180 / M_PI;
}
inline float pitchTo(XYZ start, XYZ end)
{
    return pitchOf(end - start);
}
inline float sq(float n)
{
    return n * n;
}
inline float stepTowardf(float from, float to, float by)
{
    if (fabs(from - to) < by)
        return to;
    else if (from > to)
        return from - by;
    else
        return from + by;
}

void Game::playdialogueboxsound()
{
    XYZ temppos;
    temppos = Person::players[participantfocus[whichdialogue][indialogue]]->coords;
    temppos = temppos - viewer;
    Normalise(&temppos);
    temppos += viewer;

    int sound = -1;
    switch (dialogueboxsound[whichdialogue][indialogue]) {
    case -6:
        sound = alarmsound;
        break;
    case -4:
        sound = consolefailsound;
        break;
    case -3:
        sound = consolesuccesssound;
        break;
    case -2:
        sound = firestartsound;
        break;
    case -1:
        sound = fireendsound;
        break;
    case 1:
        sound = rabbitchitter;
        break;
    case 2:
        sound = rabbitchitter2;
        break;
    case 3:
        sound = rabbitpainsound;
        break;
    case 4:
        sound = rabbitpain1sound;
        break;
    case 5:
        sound = rabbitattacksound;
        break;
    case 6:
        sound = rabbitattack2sound;
        break;
    case 7:
        sound = rabbitattack3sound;
        break;
    case 8:
        sound = rabbitattack4sound;
        break;
    case 9:
        sound = growlsound;
        break;
    case 10:
        sound = growl2sound;
        break;
    case 11:
        sound = snarlsound;
        break;
    case 12:
        sound = snarl2sound;
        break;
    case 13:
        sound = barksound;
        break;
    case 14:
        sound = bark2sound;
        break;
    case 15:
        sound = bark3sound;
        break;
    case 16:
        sound = barkgrowlsound;
        break;
    default:
        break;
    }
    if (sound != -1)
        emit_sound_at(sound, temppos);
}

// ================================================================

int Game::findClosestPlayer()
{
    int closest = -1;
    float closestdist = std::numeric_limits<float>::max();

    for (unsigned i = 1; i < Person::players.size(); i++) {
        float distance = distsq(&Person::players[i]->coords, &Person::players[0]->coords);
        if (distance < closestdist) {
            closestdist = distance;
            closest = i;
        }
    }
    return closest;
}

static int findClosestObject()
{
    int closest = -1;
    float closestdist = std::numeric_limits<float>::max();

    for (int i = 0; i < objects.numobjects; i++) {
        float distance = distsq(&objects.position[i], &Person::players[0]->coords);
        if (distance < closestdist) {
            closestdist = distance;
            closest = i;
        }
    }
    return closest;
}

static void cmd_dispatch(const string cmd)
{
    int i, n_cmds = sizeof(cmd_names) / sizeof(cmd_names[0]);

    for (i = 0; i < n_cmds; i++)
        if (cmd.substr(0, cmd.find(' ')) == string(cmd_names[i])) {
            cout << "|" << cmd.substr(cmd.find(' ') + 1) << "|" << endl;
            cmd_handlers[i](cmd.substr(cmd.find(' ') + 1).c_str());
            break;
        }
    emit_sound_np(i < n_cmds ? consolesuccesssound : consolefailsound);
}

/********************> Tick() <*****/
extern bool save_screenshot(const char * fname);
void Screenshot (void)
{
    char filename[1024];
    time_t t = time(NULL);
    struct tm *tme = localtime(&t);
    sprintf(filename, "Screenshots/Screenshot-%04d%02d%02d-%02d%02d%02d.png",
            tme->tm_year + 1900, tme->tm_mon + 1, tme->tm_mday, tme->tm_hour, tme->tm_min, tme->tm_sec);

#if defined(_WIN32)
    mkdir("Screenshots");
#endif

    save_screenshot(filename);
}

void Game::SetUpLighting()
{
    if (environment == snowyenvironment)
        light.setColors(.65, .65, .7, .4, .4, .44);
    if (environment == desertenvironment)
        light.setColors(.95, .95, .95, .4, .35, .3);
    if (environment == grassyenvironment)
        light.setColors(.95, .95, 1, .4, .4, .44);
    if (!skyboxtexture)
        light.setColors(1, 1, 1, .4, .4, .4);
    float average;
    average = (skyboxlightr + skyboxlightg + skyboxlightb) / 3;
    light.color[0] *= (skyboxlightr + average) / 2;
    light.color[1] *= (skyboxlightg + average) / 2;
    light.color[2] *= (skyboxlightb + average) / 2;
    light.ambient[0] *= (skyboxlightr + average) / 2;
    light.ambient[1] *= (skyboxlightg + average) / 2;
    light.ambient[2] *= (skyboxlightb + average) / 2;
}

int findPathDist(int start, int end)
{
    int smallestcount, count, connected;
    int last, last2, last3, last4;
    int closest;

    smallestcount = 1000;
    for (int i = 0; i < 50; i++) {
        count = 0;
        last = start;
        last2 = -1;
        last3 = -1;
        last4 = -1;
        while (last != end && count < 30) {
            closest = -1;
            for (int j = 0; j < numpathpoints; j++) {
                if (j != last && j != last2 && j != last3 && j != last4) {
                    connected = 0;
                    if (numpathpointconnect[j])
                        for (int k = 0; k < numpathpointconnect[j]; k++) {
                            if (pathpointconnect[j][k] == last)connected = 1;
                        }
                    if (!connected)
                        if (numpathpointconnect[last])
                            for (int k = 0; k < numpathpointconnect[last]; k++) {
                                if (pathpointconnect[last][k] == j)connected = 1;
                            }
                    if (connected)
                        if (closest == -1 || Random() % 2 == 0) {
                            closest = j;
                        }
                }
            }
            last4 = last3;
            last3 = last2;
            last2 = last;
            last = closest;
            count++;
        }
        if (count < smallestcount)
            smallestcount = count;
    }
    return smallestcount;
}

int Game::checkcollide(XYZ startpoint, XYZ endpoint)
{
    static XYZ colpoint, colviewer, coltarget;
    static float minx, minz, maxx, maxz, miny, maxy;

    minx = min(startpoint.x, endpoint.x) - 1;
    miny = min(startpoint.y, endpoint.y) - 1;
    minz = min(startpoint.z, endpoint.z) - 1;
    maxx = max(startpoint.x, endpoint.x) + 1;
    maxy = max(startpoint.y, endpoint.y) + 1;
    maxz = max(startpoint.z, endpoint.z) + 1;

    for (int i = 0; i < objects.numobjects; i++) {
        if (     objects.position[i].x > minx - objects.model[i].boundingsphereradius &&
                 objects.position[i].x < maxx + objects.model[i].boundingsphereradius &&
                 objects.position[i].y > miny - objects.model[i].boundingsphereradius &&
                 objects.position[i].y < maxy + objects.model[i].boundingsphereradius &&
                 objects.position[i].z > minz - objects.model[i].boundingsphereradius &&
                 objects.position[i].z < maxz + objects.model[i].boundingsphereradius) {
            if (     objects.type[i] != treeleavestype &&
                     objects.type[i] != bushtype &&
                     objects.type[i] != firetype) {
                colviewer = startpoint;
                coltarget = endpoint;
                if (objects.model[i].LineCheck(&colviewer, &coltarget, &colpoint, &objects.position[i], &objects.yaw[i]) != -1)
                    return i;
            }
        }
    }

    return -1;
}

int Game::checkcollide(XYZ startpoint, XYZ endpoint, int what)
{
    static XYZ colpoint, colviewer, coltarget;
    static float minx, minz, maxx, maxz, miny, maxy;
    static int i; //FIXME: see below

    minx = min(startpoint.x, endpoint.x) - 1;
    miny = min(startpoint.y, endpoint.y) - 1;
    minz = min(startpoint.z, endpoint.z) - 1;
    maxx = max(startpoint.x, endpoint.x) + 1;
    maxy = max(startpoint.y, endpoint.y) + 1;
    maxz = max(startpoint.z, endpoint.z) + 1;

    if (what != 1000) {
        if (     objects.position[what].x > minx - objects.model[what].boundingsphereradius &&
                 objects.position[what].x < maxx + objects.model[what].boundingsphereradius &&
                 objects.position[what].y > miny - objects.model[what].boundingsphereradius &&
                 objects.position[what].y < maxy + objects.model[what].boundingsphereradius &&
                 objects.position[what].z > minz - objects.model[what].boundingsphereradius &&
                 objects.position[what].z < maxz + objects.model[what].boundingsphereradius) {
            if (     objects.type[what] != treeleavestype &&
                     objects.type[what] != bushtype &&
                     objects.type[what] != firetype) {
                colviewer = startpoint;
                coltarget = endpoint;
                //FIXME: i/what
                if (objects.model[what].LineCheck(&colviewer, &coltarget, &colpoint, &objects.position[what], &objects.yaw[what]) != -1)
                    return i;
            }
        }
    }

    if (what == 1000)
        if (terrain.lineTerrain(startpoint, endpoint, &colpoint) != -1)
            return 1000;

    return -1;
}

void Setenvironment(int which)
{
    LOGFUNC;

    LOG(" Setting environment...");

    float temptexdetail;
    environment = which;

    pause_sound(stream_snowtheme);
    pause_sound(stream_grasstheme);
    pause_sound(stream_deserttheme);
    pause_sound(stream_wind);
    pause_sound(stream_desertambient);


    if (environment == snowyenvironment) {
        windvector = 0;
        windvector.z = 3;
        if (ambientsound)
            emit_stream_np(stream_wind);

        objects.treetextureptr.load(":Data:Textures:snowtree.png", 0);
        objects.bushtextureptr.load(":Data:Textures:bushsnow.png", 0);
        objects.rocktextureptr.load(":Data:Textures:bouldersnow.jpg", 1);
        objects.boxtextureptr.load(":Data:Textures:snowbox.jpg", 1);

        footstepsound = footstepsn1;
        footstepsound2 = footstepsn2;
        footstepsound3 = footstepst1;
        footstepsound4 = footstepst2;

        terraintexture.load(":Data:Textures:snow.jpg", 1);
        terraintexture2.load(":Data:Textures:rock.jpg", 1);


        temptexdetail = texdetail;
        if (texdetail > 1)
            texdetail = 4;
        skybox->load(   ":Data:Textures:Skybox(snow):Front.jpg",
                        ":Data:Textures:Skybox(snow):Left.jpg",
                        ":Data:Textures:Skybox(snow):Back.jpg",
                        ":Data:Textures:Skybox(snow):Right.jpg",
                        ":Data:Textures:Skybox(snow):Up.jpg",
                        ":Data:Textures:Skybox(snow):Down.jpg");




        texdetail = temptexdetail;
    } else if (environment == desertenvironment) {
        windvector = 0;
        windvector.z = 2;
        objects.treetextureptr.load(":Data:Textures:deserttree.png", 0);
        objects.bushtextureptr.load(":Data:Textures:bushdesert.png", 0);
        objects.rocktextureptr.load(":Data:Textures:boulderdesert.jpg", 1);
        objects.boxtextureptr.load(":Data:Textures:desertbox.jpg", 1);


        if (ambientsound)
            emit_stream_np(stream_desertambient);

        footstepsound = footstepsn1;
        footstepsound2 = footstepsn2;
        footstepsound3 = footstepsn1;
        footstepsound4 = footstepsn2;

        terraintexture.load(":Data:Textures:sand.jpg", 1);
        terraintexture2.load(":Data:Textures:sandslope.jpg", 1);


        temptexdetail = texdetail;
        if (texdetail > 1)
            texdetail = 4;
        skybox->load(   ":Data:Textures:Skybox(sand):Front.jpg",
                        ":Data:Textures:Skybox(sand):Left.jpg",
                        ":Data:Textures:Skybox(sand):Back.jpg",
                        ":Data:Textures:Skybox(sand):Right.jpg",
                        ":Data:Textures:Skybox(sand):Up.jpg",
                        ":Data:Textures:Skybox(sand):Down.jpg");




        texdetail = temptexdetail;
    } else if (environment == grassyenvironment) {
        windvector = 0;
        windvector.z = 2;
        objects.treetextureptr.load(":Data:Textures:tree.png", 0);
        objects.bushtextureptr.load(":Data:Textures:bush.png", 0);
        objects.rocktextureptr.load(":Data:Textures:boulder.jpg", 1);
        objects.boxtextureptr.load(":Data:Textures:grassbox.jpg", 1);

        if (ambientsound)
            emit_stream_np(stream_wind, 100.);

        footstepsound = footstepgr1;
        footstepsound2 = footstepgr2;
        footstepsound3 = footstepst1;
        footstepsound4 = footstepst2;

        terraintexture.load(":Data:Textures:grassdirt.jpg", 1);
        terraintexture2.load(":Data:Textures:mossrock.jpg", 1);


        temptexdetail = texdetail;
        if (texdetail > 1)
            texdetail = 4;
        skybox->load(   ":Data:Textures:Skybox(grass):Front.jpg",
                        ":Data:Textures:Skybox(grass):Left.jpg",
                        ":Data:Textures:Skybox(grass):Back.jpg",
                        ":Data:Textures:Skybox(grass):Right.jpg",
                        ":Data:Textures:Skybox(grass):Up.jpg",
                        ":Data:Textures:Skybox(grass):Down.jpg");



        texdetail = temptexdetail;
    }
    temptexdetail = texdetail;
    texdetail = 1;
    terrain.load(":Data:Textures:heightmap.png");

    texdetail = temptexdetail;
}

void LoadCampaign()
{
    if (!accountactive)
        return;
    ifstream ipstream(ConvertFileName((":Data:Campaigns:" + accountactive->getCurrentCampaign() + ".txt").c_str()));
    if (!ipstream.good()) {
        if (accountactive->getCurrentCampaign() == "main") {
            cerr << "Could not found main campaign!" << endl;
            return;
        }
        cerr << "Could not found campaign \"" << accountactive->getCurrentCampaign() << "\", falling back to main." << endl;
        accountactive->setCurrentCampaign("main");
        return LoadCampaign();
    }
    ipstream.ignore(256, ':');
    int numlevels;
    ipstream >> numlevels;
    campaignlevels.clear();
    for (int i = 0; i < numlevels; i++) {
        CampaignLevel cl;
        ipstream >> cl;
        campaignlevels.push_back(cl);
    }
    ipstream.close();

    ifstream test(ConvertFileName((":Data:Textures:" + accountactive->getCurrentCampaign() + ":World.png").c_str()));
    if (test.good()) {
        Mainmenuitems[7].load((":Data:Textures:" + accountactive->getCurrentCampaign() + ":World.png").c_str(), 0);
    } else {
        Mainmenuitems[7].load(":Data:Textures:World.png", 0);
    }

    if (accountactive->getCampaignChoicesMade() == 0) {
        accountactive->setCampaignScore(0);
        accountactive->resetFasttime();
    }
}

vector<string> ListCampaigns()
{
    DIR *campaigns = opendir(ConvertFileName(":Data:Campaigns"));
    struct dirent *campaign = NULL;
    if (!campaigns) {
        perror("Problem while loading campaigns");
        cerr << "campaign folder was : " << ConvertFileName(":Data:Campaigns") << endl;
        exit(EXIT_FAILURE);
    }
    vector<string> campaignNames;
    while ((campaign = readdir(campaigns)) != NULL) {
        string name(campaign->d_name);
        if (name.length() < 5)
            continue;
        if (!name.compare(name.length() - 4, 4, ".txt")) {
            campaignNames.push_back(name.substr(0, name.length() - 4));
        }
    }
    closedir(campaigns);
    return campaignNames;
}

void Game::Loadlevel(int which)
{
    stealthloading = 0;
    whichlevel = which;

    if (which == -1) {
        tutoriallevel = -1;
        Loadlevel("tutorial");
    } else if (which >= 0 && which <= 15) {
        char buf[32];
        snprintf(buf, 32, "map%d", which + 1); // challenges
        Loadlevel(buf);
    } else
        Loadlevel("mapsave");
}

void Game::Loadlevel(const char *name)
{
    int indemo; // FIXME this should be removed
    int templength;
    float lamefloat;
    static const char *pfx = ":Data:Maps:";
    char *buf;

    LOGFUNC;

    LOG(std::string("Loading level...") + name);

    if (!gameon)
        visibleloading = 1;
    if (stealthloading)
        visibleloading = 0;
    if (!stillloading)
        loadtime = 0;
    gamestarted = 1;

    numenvsounds = 0;

    if (tutoriallevel != -1)
        tutoriallevel = 0;
    else
        tutoriallevel = 1;

    if (tutoriallevel == 1)
        tutorialstage = 0;
    if (tutorialstage == 0) {
        tutorialstagetime = 0;
        tutorialmaxtime = 1;
    }
    pause_sound(whooshsound);
    pause_sound(stream_firesound);

    // Change the map filename into something that is os specific
    buf = (char*) alloca(strlen(pfx) + strlen(name) + 1);
    sprintf(buf, "%s%s", pfx, name);
    const char *FixedFN = ConvertFileName(buf);

    int mapvers;
    FILE *tfile;
    tfile = fopen( FixedFN, "rb" );
    if (tfile) {
        pause_sound(stream_firesound);
        scoreadded = 0;
        windialogue = false;
        hostiletime = 0;
        won = 0;

        animation[bounceidleanim].Load((char *)"Idle", middleheight, neutral);

        numdialogues = 0;

        for (int i = 0; i < 20; i++)
            dialoguegonethrough[i] = 0;

        indialogue = -1;
        cameramode = 0;

        damagedealt = 0;
        damagetaken = 0;

        if (accountactive)
            difficulty = accountactive->getDifficulty();

        numhotspots = 0;
        currenthotspot = -1;
        bonustime = 1;

        skyboxtexture = 1;
        skyboxr = 1;
        skyboxg = 1;
        skyboxb = 1;

        freeze = 0;
        winfreeze = 0;

        for (int i = 0; i < 100; i++)
            bonusnum[i] = 0;

        numfalls = 0;
        numflipfail = 0;
        numseen = 0;
        numstaffattack = 0;
        numswordattack = 0;
        numknifeattack = 0;
        numunarmedattack = 0;
        numescaped = 0;
        numflipped = 0;
        numwallflipped = 0;
        numthrowkill = 0;
        numafterkill = 0;
        numreversals = 0;
        numattacks = 0;
        maxalarmed = 0;
        numresponded = 0;

        bonustotal = startbonustotal;
        bonus = 0;
        gameon = 1;
        changedelay = 0;
        if (console) {
            emit_sound_np(consolesuccesssound);
            freeze = 0;
            console = false;
        }

        if (!stealthloading) {
            terrain.numdecals = 0;
            Sprite::deleteSprites();
            for (int i = 0; i < objects.numobjects; i++)
                objects.model[i].numdecals = 0;

            int j = objects.numobjects;
            for (int i = 0; i < j; i++) {
                objects.DeleteObject(0);
                if (visibleloading)
                    LoadingScreen();
            }

            for (int i = 0; i < subdivision; i++)
                for (int j = 0; j < subdivision; j++)
                    terrain.patchobjectnum[i][j] = 0;
            if (visibleloading)
                LoadingScreen();
        }

        weapons.clear();
        Person::players.resize(1);

        funpackf(tfile, "Bi", &mapvers);
        if (mapvers >= 15)
            funpackf(tfile, "Bi", &indemo);
        else
            indemo = 0;
        if (mapvers >= 5)
            funpackf(tfile, "Bi", &maptype);
        else
            maptype = mapkilleveryone;
        if (mapvers >= 6)
            funpackf(tfile, "Bi", &hostile);
        else
            hostile = 1;
        if (mapvers >= 4)
            funpackf(tfile, "Bf Bf", &viewdistance, &fadestart);
        else {
            viewdistance = 100;
            fadestart = .6;
        }
        if (mapvers >= 2)
            funpackf(tfile, "Bb Bf Bf Bf", &skyboxtexture, &skyboxr, &skyboxg, &skyboxb);
        else {
            skyboxtexture = 1;
            skyboxr = 1;
            skyboxg = 1;
            skyboxb = 1;
        }
        if (mapvers >= 10)
            funpackf(tfile, "Bf Bf Bf", &skyboxlightr, &skyboxlightg, &skyboxlightb);
        else {
            skyboxlightr = skyboxr;
            skyboxlightg = skyboxg;
            skyboxlightb = skyboxb;
        }
        if (!stealthloading)
            funpackf(tfile, "Bf Bf Bf Bf Bf Bi", &Person::players[0]->coords.x, &Person::players[0]->coords.y, &Person::players[0]->coords.z, &Person::players[0]->yaw, &Person::players[0]->targetyaw, &Person::players[0]->num_weapons);
        if (stealthloading)
            funpackf(tfile, "Bf Bf Bf Bf Bf Bi", &lamefloat, &lamefloat, &lamefloat, &lamefloat, &lamefloat, &Person::players[0]->num_weapons);
        if (Person::players[0]->num_weapons > 0 && Person::players[0]->num_weapons < 5)
            for (int j = 0; j < Person::players[0]->num_weapons; j++) {
                Person::players[0]->weaponids[j] = weapons.size();
                int type;
                funpackf(tfile, "Bi", &type);
                weapons.push_back(Weapon(type, 0));
            }

        if (visibleloading)
            LoadingScreen();

        funpackf(tfile, "Bf Bf Bf", &Person::players[0]->armorhead, &Person::players[0]->armorhigh, &Person::players[0]->armorlow);
        funpackf(tfile, "Bf Bf Bf", &Person::players[0]->protectionhead, &Person::players[0]->protectionhigh, &Person::players[0]->protectionlow);
        funpackf(tfile, "Bf Bf Bf", &Person::players[0]->metalhead, &Person::players[0]->metalhigh, &Person::players[0]->metallow);
        funpackf(tfile, "Bf Bf", &Person::players[0]->power, &Person::players[0]->speedmult);

        funpackf(tfile, "Bi", &Person::players[0]->numclothes);

        if (mapvers >= 9)
            funpackf(tfile, "Bi Bi", &Person::players[0]->whichskin, &Person::players[0]->creature);
        else {
            Person::players[0]->whichskin = 0;
            Person::players[0]->creature = rabbittype;
        }

        Person::players[0]->lastattack = -1;
        Person::players[0]->lastattack2 = -1;
        Person::players[0]->lastattack3 = -1;

        //dialogues
        if (mapvers >= 8) {
            funpackf(tfile, "Bi", &numdialogues);
            for (int k = 0; k < numdialogues; k++) {
                funpackf(tfile, "Bi", &numdialogueboxes[k]);
                funpackf(tfile, "Bi", &dialoguetype[k]);
                for (int l = 0; l < 10; l++) {
                    funpackf(tfile, "Bf Bf Bf", &participantlocation[k][l].x, &participantlocation[k][l].y, &participantlocation[k][l].z);
                    funpackf(tfile, "Bf", &participantyaw[k][l]);
                }
                for (int l = 0; l < numdialogueboxes[k]; l++) {
                    funpackf(tfile, "Bi", &dialogueboxlocation[k][l]);
                    funpackf(tfile, "Bf", &dialogueboxcolor[k][l][0]);
                    funpackf(tfile, "Bf", &dialogueboxcolor[k][l][1]);
                    funpackf(tfile, "Bf", &dialogueboxcolor[k][l][2]);
                    funpackf(tfile, "Bi", &dialogueboxsound[k][l]);

                    funpackf(tfile, "Bi", &templength);
                    if (templength > 128 || templength <= 0)
                        templength = 128;
                    int m;
                    for (m = 0; m < templength; m++) {
                        funpackf(tfile, "Bb", &dialoguetext[k][l][m]);
                        if (dialoguetext[k][l][m] == '\0')
                            break;
                    }
                    dialoguetext[k][l][m] = 0;

                    funpackf(tfile, "Bi", &templength);
                    if (templength > 64 || templength <= 0)
                        templength = 64;
                    for (m = 0; m < templength; m++) {
                        funpackf(tfile, "Bb", &dialoguename[k][l][m]);
                        if (dialoguename[k][l][m] == '\0')
                            break;
                    }
                    dialoguename[k][l][m] = 0;
                    funpackf(tfile, "Bf Bf Bf", &dialoguecamera[k][l].x, &dialoguecamera[k][l].y, &dialoguecamera[k][l].z);
                    funpackf(tfile, "Bi", &participantfocus[k][l]);
                    funpackf(tfile, "Bi", &participantaction[k][l]);

                    for (m = 0; m < 10; m++)
                        funpackf(tfile, "Bf Bf Bf", &participantfacing[k][l][m].x, &participantfacing[k][l][m].y, &participantfacing[k][l][m].z);

                    funpackf(tfile, "Bf Bf", &dialoguecamerayaw[k][l], &dialoguecamerapitch[k][l]);
                }
            }
        } else {
            numdialogues = 0;
        }

        for (int k = 0; k < Person::players[0]->numclothes; k++) {
            funpackf(tfile, "Bi", &templength);
            for (int l = 0; l < templength; l++)
                funpackf(tfile, "Bb", &Person::players[0]->clothes[k][l]);
            Person::players[0]->clothes[k][templength] = '\0';
            funpackf(tfile, "Bf Bf Bf", &Person::players[0]->clothestintr[k], &Person::players[0]->clothestintg[k], &Person::players[0]->clothestintb[k]);
        }

        funpackf(tfile, "Bi", &environment);

        funpackf(tfile, "Bi", &objects.numobjects);
        for (int i = 0; i < objects.numobjects; i++) {
            funpackf(tfile, "Bi Bf Bf Bf Bf Bf Bf", &objects.type[i], &objects.yaw[i], &objects.pitch[i], &objects.position[i].x, &objects.position[i].y, &objects.position[i].z, &objects.scale[i]);
            if (objects.type[i] == treeleavestype)
                objects.scale[i] = objects.scale[i - 1];
        }

        if (mapvers >= 7) {
            funpackf(tfile, "Bi", &numhotspots);
            for (int i = 0; i < numhotspots; i++) {
                funpackf(tfile, "Bi Bf Bf Bf Bf", &hotspottype[i], &hotspotsize[i], &hotspot[i].x, &hotspot[i].y, &hotspot[i].z);
                funpackf(tfile, "Bi", &templength);
                if (templength)
                    for (int l = 0; l < templength; l++)
                        funpackf(tfile, "Bb", &hotspottext[i][l]);
                hotspottext[i][templength] = '\0';
                if (hotspottype[i] == -111)
                    indemo = 1;
            }
        } else
            numhotspots = 0;

        if (visibleloading)
            LoadingScreen();

        if (!stealthloading) {
            objects.center = 0;
            for (int i = 0; i < objects.numobjects; i++)
                objects.center += objects.position[i];
            objects.center /= objects.numobjects;


            if (visibleloading)
                LoadingScreen();

            float maxdistance = 0;
            float tempdist;
            for (int i = 0; i < objects.numobjects; i++) {
                tempdist = distsq(&objects.center, &objects.position[i]);
                if (tempdist > maxdistance) {
                    maxdistance = tempdist;
                }
            }
            objects.radius = fast_sqrt(maxdistance);
        }

        if (visibleloading)
            LoadingScreen();

        int numplayers;
        funpackf(tfile, "Bi", &numplayers);
        if (numplayers > maxplayers) {
            cout << "Warning: this level contains more players than allowed" << endl;
        }
        for (int i = 1; i < numplayers; i++) {
            unsigned j = 1;
            try {
                Person::players.push_back(shared_ptr<Person>(new Person(tfile, mapvers, j)));
                j++;
            } catch (InvalidPersonException e) {
            }
        }
        if (visibleloading)
            LoadingScreen();

        funpackf(tfile, "Bi", &numpathpoints);
        if (numpathpoints > 30 || numpathpoints < 0)
            numpathpoints = 0;
        for (int j = 0; j < numpathpoints; j++) {
            funpackf(tfile, "Bf Bf Bf Bi", &pathpoint[j].x, &pathpoint[j].y, &pathpoint[j].z, &numpathpointconnect[j]);
            for (int k = 0; k < numpathpointconnect[j]; k++) {
                funpackf(tfile, "Bi", &pathpointconnect[j][k]);
            }
        }
        if (visibleloading)
            LoadingScreen();

        funpackf(tfile, "Bf Bf Bf Bf", &mapcenter.x, &mapcenter.y, &mapcenter.z, &mapradius);

        SetUpLighting();
        if (environment != oldenvironment)
            Setenvironment(environment);
        oldenvironment = environment;

        if (!stealthloading) {
            int j = objects.numobjects;
            objects.numobjects = 0;
            for (int i = 0; i < j; i++) {
                objects.MakeObject(objects.type[i], objects.position[i], objects.yaw[i], objects.pitch[i], objects.scale[i]);
                if (visibleloading)
                    LoadingScreen();
            }

            terrain.DoShadows();
            if (visibleloading)
                LoadingScreen();
            objects.DoShadows();
            if (visibleloading)
                LoadingScreen();
        }

        fclose(tfile);

        for (unsigned i = 0; i < Person::players.size(); i++) {
            if (visibleloading)
                LoadingScreen();
            Person::players[i]->burnt = 0;
            Person::players[i]->bled = 0;
            Person::players[i]->onfire = 0;
            if (i == 0 || Person::players[i]->scale < 0)
                Person::players[i]->scale = .2;
            Person::players[i]->skeleton.free = 0;
            Person::players[i]->skeleton.id = i;
            if (i == 0 && mapvers < 9)
                Person::players[i]->creature = rabbittype;
            if (Person::players[i]->creature != wolftype) {
                Person::players[i]->skeleton.Load(
                    (char *)":Data:Skeleton:Basic Figure",
                    (char *)":Data:Skeleton:Basic Figurelow",
                    (char *)":Data:Skeleton:Rabbitbelt",
                    (char *)":Data:Models:Body.solid",
                    (char *)":Data:Models:Body2.solid",
                    (char *)":Data:Models:Body3.solid",
                    (char *)":Data:Models:Body4.solid",
                    (char *)":Data:Models:Body5.solid",
                    (char *)":Data:Models:Body6.solid",
                    (char *)":Data:Models:Body7.solid",
                    (char *)":Data:Models:Bodylow.solid",
                    (char *)":Data:Models:Belt.solid", 0);
            } else {
                if (Person::players[i]->creature != wolftype) {
                    Person::players[i]->skeleton.Load(
                        (char *)":Data:Skeleton:Basic Figure",
                        (char *)":Data:Skeleton:Basic Figurelow",
                        (char *)":Data:Skeleton:Rabbitbelt",
                        (char *)":Data:Models:Body.solid",
                        (char *)":Data:Models:Body2.solid",
                        (char *)":Data:Models:Body3.solid",
                        (char *)":Data:Models:Body4.solid",
                        (char *)":Data:Models:Body5.solid",
                        (char *)":Data:Models:Body6.solid",
                        (char *)":Data:Models:Body7.solid",
                        (char *)":Data:Models:Bodylow.solid",
                        (char *)":Data:Models:Belt.solid", 1);
                    Person::players[i]->skeleton.drawmodelclothes.textureptr.load(":Data:Textures:Belt.png", 1);
                }
                if (Person::players[i]->creature == wolftype) {
                    Person::players[i]->skeleton.Load(
                        (char *)":Data:Skeleton:Basic Figure Wolf",
                        (char *)":Data:Skeleton:Basic Figure Wolf Low",
                        (char *)":Data:Skeleton:Rabbitbelt",
                        (char *)":Data:Models:Wolf.solid",
                        (char *)":Data:Models:Wolf2.solid",
                        (char *)":Data:Models:Wolf3.solid",
                        (char *)":Data:Models:Wolf4.solid",
                        (char *)":Data:Models:Wolf5.solid",
                        (char *)":Data:Models:Wolf6.solid",
                        (char *)":Data:Models:Wolf7.solid",
                        (char *)":Data:Models:Wolflow.solid",
                        (char *)":Data:Models:Belt.solid", 0);
                }
            }

            Person::players[i]->skeleton.drawmodel.textureptr.load(creatureskin[Person::players[i]->creature][Person::players[i]->whichskin], 1, &Person::players[i]->skeleton.skinText[0], &Person::players[i]->skeleton.skinsize);

            Person::players[i]->addClothes();

            Person::players[i]->animCurrent = bounceidleanim;
            Person::players[i]->animTarget = bounceidleanim;
            Person::players[i]->frameCurrent = 0;
            Person::players[i]->frameTarget = 1;
            Person::players[i]->target = 0;
            Person::players[i]->speed = 1 + (float)(Random() % 100) / 1000;
            if (difficulty == 0)
                Person::players[i]->speed -= .2;
            if (difficulty == 1)
                Person::players[i]->speed -= .1;

            Person::players[i]->velocity = 0;
            Person::players[i]->oldcoords = Person::players[i]->coords;
            Person::players[i]->realoldcoords = Person::players[i]->coords;

            Person::players[i]->id = i;
            Person::players[i]->skeleton.id = i;
            Person::players[i]->updatedelay = 0;
            Person::players[i]->normalsupdatedelay = 0;

            Person::players[i]->aitype = passivetype;

            if (i == 0) {
                Person::players[i]->proportionhead = 1.2;
                Person::players[i]->proportionbody = 1.05;
                Person::players[i]->proportionarms = 1.00;
                Person::players[i]->proportionlegs = 1.1;
                Person::players[i]->proportionlegs.y = 1.05;
            }
            Person::players[i]->headless = 0;
            Person::players[i]->currentoffset = 0;
            Person::players[i]->targetoffset = 0;

            Person::players[i]->damagetolerance = 200;

            if (Person::players[i]->creature == wolftype) {
                if (i == 0 || Person::players[i]->scale < 0)
                    Person::players[i]->scale = .23;
                Person::players[i]->damagetolerance = 300;
            }

            if (visibleloading)
                LoadingScreen();
            if (cellophane) {
                Person::players[i]->proportionhead.z = 0;
                Person::players[i]->proportionbody.z = 0;
                Person::players[i]->proportionarms.z = 0;
                Person::players[i]->proportionlegs.z = 0;
            }

            Person::players[i]->tempanimation.Load((char *)"Tempanim", 0, 0);

            Person::players[i]->headmorphness = 0;
            Person::players[i]->targetheadmorphness = 1;
            Person::players[i]->headmorphstart = 0;
            Person::players[i]->headmorphend = 0;

            Person::players[i]->pausetime = 0;

            Person::players[i]->dead = 0;
            Person::players[i]->jumppower = 5;
            Person::players[i]->damage = 0;
            Person::players[i]->permanentdamage = 0;
            Person::players[i]->superpermanentdamage = 0;

            Person::players[i]->forwardkeydown = 0;
            Person::players[i]->leftkeydown = 0;
            Person::players[i]->backkeydown = 0;
            Person::players[i]->rightkeydown = 0;
            Person::players[i]->jumpkeydown = 0;
            Person::players[i]->crouchkeydown = 0;
            Person::players[i]->throwkeydown = 0;

            Person::players[i]->collided = -10;
            Person::players[i]->loaded = 1;
            Person::players[i]->bloodloss = 0;
            Person::players[i]->weaponactive = -1;
            Person::players[i]->weaponstuck = -1;
            Person::players[i]->bleeding = 0;
            Person::players[i]->deathbleeding = 0;
            Person::players[i]->stunned = 0;
            Person::players[i]->hasvictim = 0;
            Person::players[i]->wentforweapon = 0;
        }

        Person::players[0]->aitype = playercontrolled;
        Person::players[0]->weaponactive = -1;

        if (difficulty == 1) {
            Person::players[0]->power = 1 / .9;
            Person::players[0]->damagetolerance = 250;
        } else if (difficulty == 0) {
            Person::players[0]->power = 1 / .8;
            Person::players[0]->damagetolerance = 300;
            Person::players[0]->armorhead *= 1.5;
            Person::players[0]->armorhigh *= 1.5;
            Person::players[0]->armorlow *= 1.5;
        }

        cameraloc = Person::players[0]->coords;
        cameraloc.y += 5;
        yaw = Person::players[0]->yaw;

        hawkcoords = Person::players[0]->coords;
        hawkcoords.y += 30;

        if (visibleloading)
            LoadingScreen();

        LOG("Starting background music...");

        OPENAL_StopSound(OPENAL_ALL);
        if (ambientsound) {
            if (environment == snowyenvironment) {
                emit_stream_np(stream_wind);
            } else if (environment == desertenvironment) {
                emit_stream_np(stream_desertambient);
            } else if (environment == grassyenvironment) {
                emit_stream_np(stream_wind, 100.);
            }
        }
        oldmusicvolume[0] = 0;
        oldmusicvolume[1] = 0;
        oldmusicvolume[2] = 0;
        oldmusicvolume[3] = 0;

        if (!firstload)
            firstload = 1;
    } else {
        perror("Problem");
    }
    leveltime = 0;
    visibleloading = 0;
}

void doTutorial()
{
    if (tutorialstagetime > tutorialmaxtime) {
        tutorialstage++;
        tutorialsuccess = 0;
        if (tutorialstage <= 1) {
            canattack = 0;
            cananger = 0;
            reversaltrain = 0;
        }
        switch (tutorialstage) {
        case 1:
            tutorialmaxtime = 5;
            break;
        case 2:
            tutorialmaxtime = 2;
            break;
        case 3:
            tutorialmaxtime = 600;
            break;
        case 4:
            tutorialmaxtime = 1000;
            break;
        case 5:
            tutorialmaxtime = 600;
            break;
        case 6:
            tutorialmaxtime = 600;
            break;
        case 7:
            tutorialmaxtime = 600;
            break;
        case 8:
            tutorialmaxtime = 600;
            break;
        case 9:
            tutorialmaxtime = 600;
            break;
        case 10:
            tutorialmaxtime = 2;
            break;
        case 11:
            tutorialmaxtime = 1000;
            break;
        case 12:
            tutorialmaxtime = 1000;
            break;
        case 13:
            tutorialmaxtime = 2;
            break;
        case 14: {
            tutorialmaxtime = 3;

            XYZ temp, temp2;

            temp.x = 1011;
            temp.y = 84;
            temp.z = 491;
            temp2.x = 1025;
            temp2.y = 75;
            temp2.z = 447;

            Person::players[1]->coords = (temp + temp2) / 2;

            emit_sound_at(fireendsound, Person::players[1]->coords);

            for (int i = 0; i < Person::players[1]->skeleton.num_joints; i++) {
                if (Random() % 2 == 0) {
                    if (!Person::players[1]->skeleton.free)
                        temp2 = (Person::players[1]->coords - Person::players[1]->oldcoords) / multiplier / 2; //velocity/2;
                    if (Person::players[1]->skeleton.free)
                        temp2 = Person::players[1]->skeleton.joints[i].velocity * Person::players[1]->scale / 2;
                    if (!Person::players[1]->skeleton.free)
                        temp = DoRotation(DoRotation(DoRotation(Person::players[1]->skeleton.joints[i].position, 0, 0, Person::players[1]->tilt), Person::players[1]->tilt2, 0, 0), 0, Person::players[1]->yaw, 0) * Person::players[1]->scale + Person::players[1]->coords;
                    if (Person::players[1]->skeleton.free)
                        temp = Person::players[1]->skeleton.joints[i].position * Person::players[1]->scale + Person::players[1]->coords;
                    Sprite::MakeSprite(breathsprite, temp, temp2, 1, 1, 1, .6 + (float)abs(Random() % 100) / 200 - .25, 1);
                }
            }
        }
        break;
        case 15:
            tutorialmaxtime = 500;
            break;
        case 16:
            tutorialmaxtime = 500;
            break;
        case 17:
            tutorialmaxtime = 500;
            break;
        case 18:
            tutorialmaxtime = 500;
            break;
        case 19:
            tutorialstage = 20;
            break;
        case 20:
            tutorialmaxtime = 500;
            break;
        case 21:
            tutorialmaxtime = 500;
            if (bonus == cannon) {
                bonus = Slicebonus;
                againbonus = 1;
            } else
                againbonus = 0;
            break;
        case 22:
            tutorialmaxtime = 500;
            break;
        case 23:
            tutorialmaxtime = 500;
            break;
        case 24:
            tutorialmaxtime = 500;
            break;
        case 25:
            tutorialmaxtime = 500;
            break;
        case 26:
            tutorialmaxtime = 2;
            break;
        case 27:
            tutorialmaxtime = 4;
            reversaltrain = 1;
            cananger = 1;
            Person::players[1]->aitype = attacktypecutoff;
            break;
        case 28:
            tutorialmaxtime = 400;
            break;
        case 29:
            tutorialmaxtime = 400;
            Person::players[0]->escapednum = 0;
            break;
        case 30:
            tutorialmaxtime = 4;
            reversaltrain = 0;
            cananger = 0;
            Person::players[1]->aitype = passivetype;
            break;
        case 31:
            tutorialmaxtime = 13;
            break;
        case 32:
            tutorialmaxtime = 8;
            break;
        case 33:
            tutorialmaxtime = 400;
            cananger = 1;
            canattack = 1;
            Person::players[1]->aitype = attacktypecutoff;
            break;
        case 34:
            tutorialmaxtime = 400;
            break;
        case 35:
            tutorialmaxtime = 400;
            break;
        case 36:
            tutorialmaxtime = 2;
            reversaltrain = 0;
            cananger = 0;
            Person::players[1]->aitype = passivetype;
            break;
        case 37:
            damagedealt = 0;
            damagetaken = 0;
            tutorialmaxtime = 50;
            cananger = 1;
            canattack = 1;
            Person::players[1]->aitype = attacktypecutoff;
            break;
        case 38:
            tutorialmaxtime = 4;
            canattack = 0;
            cananger = 0;
            Person::players[1]->aitype = passivetype;
            break;
        case 39: {
            XYZ temp, temp2;

            temp.x = 1011;
            temp.y = 84;
            temp.z = 491;
            temp2.x = 1025;
            temp2.y = 75;
            temp2.z = 447;

            Weapon w(knife, -1);
            w.position = (temp + temp2) / 2;
            w.tippoint = (temp + temp2) / 2;

            w.velocity = 0.1;
            w.tipvelocity = 0.1;
            w.missed = 1;
            w.hitsomething = 0;
            w.freetime = 0;
            w.firstfree = 1;
            w.physics = 1;

            weapons.push_back(w);
        }
        break;
        case 40:
            tutorialmaxtime = 300;
            break;
        case 41:
            tutorialmaxtime = 300;
            break;
        case 42:
            tutorialmaxtime = 8;
            break;
        case 43:
            tutorialmaxtime = 300;
            break;
        case 44:
            weapons[0].owner = 1;
            Person::players[0]->weaponactive = -1;
            Person::players[0]->num_weapons = 0;
            Person::players[1]->weaponactive = 0;
            Person::players[1]->num_weapons = 1;
            Person::players[1]->weaponids[0] = 0;

            cananger = 1;
            canattack = 1;
            Person::players[1]->aitype = attacktypecutoff;

            tutorialmaxtime = 300;
            break;
        case 45:
            weapons[0].owner = 1;
            Person::players[0]->weaponactive = -1;
            Person::players[0]->num_weapons = 0;
            Person::players[1]->weaponactive = 0;
            Person::players[1]->num_weapons = 1;
            Person::players[1]->weaponids[0] = 0;

            tutorialmaxtime = 300;
            break;
        case 46:
            weapons[0].owner = 1;
            Person::players[0]->weaponactive = -1;
            Person::players[0]->num_weapons = 0;
            Person::players[1]->weaponactive = 0;
            Person::players[1]->num_weapons = 1;
            Person::players[1]->weaponids[0] = 0;

            weapons[0].setType(sword);

            tutorialmaxtime = 300;
            break;
        case 47: {
            tutorialmaxtime = 10;

            XYZ temp, temp2;

            temp.x = 1011;
            temp.y = 84;
            temp.z = 491;
            temp2.x = 1025;
            temp2.y = 75;
            temp2.z = 447;

            Weapon w(sword, -1);
            w.position = (temp + temp2) / 2;
            w.tippoint = (temp + temp2) / 2;

            w.velocity = 0.1;
            w.tipvelocity = 0.1;
            w.missed = 1;
            w.hitsomething = 0;
            w.freetime = 0;
            w.firstfree = 1;
            w.physics = 1;

            weapons.push_back(w);

            weapons[0].owner = 1;
            weapons[1].owner = 0;
            Person::players[0]->weaponactive = 0;
            Person::players[0]->num_weapons = 1;
            Person::players[0]->weaponids[0] = 1;
            Person::players[1]->weaponactive = 0;
            Person::players[1]->num_weapons = 1;
            Person::players[1]->weaponids[0] = 0;

        }
        break;
        case 48:
            canattack = 0;
            cananger = 0;
            Person::players[1]->aitype = passivetype;

            tutorialmaxtime = 15;

            weapons[0].owner = 1;
            weapons[1].owner = 0;
            Person::players[0]->weaponactive = 0;
            Person::players[0]->num_weapons = 1;
            Person::players[0]->weaponids[0] = 1;
            Person::players[1]->weaponactive = 0;
            Person::players[1]->num_weapons = 1;
            Person::players[1]->weaponids[0] = 0;

            if (Person::players[0]->weaponactive != -1)
                weapons[Person::players[0]->weaponids[Person::players[0]->weaponactive]].setType(staff);
            else
                weapons[0].setType(staff);
            break;
        case 49:
            canattack = 0;
            cananger = 0;
            Person::players[1]->aitype = passivetype;

            tutorialmaxtime = 200;

            weapons[1].position = 1000;
            weapons[1].tippoint = 1000;

            weapons[0].setType(knife);

            weapons[0].owner = 0;
            Person::players[1]->weaponactive = -1;
            Person::players[1]->num_weapons = 0;
            Person::players[0]->weaponactive = 0;
            Person::players[0]->num_weapons = 1;
            Person::players[0]->weaponids[0] = 0;

            break;
        case 50: {
            tutorialmaxtime = 8;

            XYZ temp, temp2;
            emit_sound_at(fireendsound, Person::players[1]->coords);

            for (int i = 0; i < Person::players[1]->skeleton.num_joints; i++) {
                if (Random() % 2 == 0) {
                    if (!Person::players[1]->skeleton.free)
                        temp2 = (Person::players[1]->coords - Person::players[1]->oldcoords) / multiplier / 2; //velocity/2;
                    if (Person::players[1]->skeleton.free)
                        temp2 = Person::players[1]->skeleton.joints[i].velocity * Person::players[1]->scale / 2;
                    if (!Person::players[1]->skeleton.free)
                        temp = DoRotation(DoRotation(DoRotation(Person::players[1]->skeleton.joints[i].position, 0, 0, Person::players[1]->tilt), Person::players[1]->tilt2, 0, 0), 0, Person::players[1]->yaw, 0) * Person::players[1]->scale + Person::players[1]->coords;
                    if (Person::players[1]->skeleton.free)
                        temp = Person::players[1]->skeleton.joints[i].position * Person::players[1]->scale + Person::players[1]->coords;
                    Sprite::MakeSprite(breathsprite, temp, temp2, 1, 1, 1, .6 + (float)abs(Random() % 100) / 200 - .25, 1);
                }
            }

            Person::players[1]->num_weapons = 0;
            Person::players[1]->weaponstuck = -1;
            Person::players[1]->weaponactive = -1;

            weapons.clear();
        }
        break;
        case 51:
            tutorialmaxtime = 80000;
            break;
        default:
            break;
        }
        if (tutorialstage <= 51)
            tutorialstagetime = 0;
    }

    //Tutorial success
    if (tutorialstagetime < tutorialmaxtime - 3) {
        switch (tutorialstage) {
        case 3:
            if (deltah || deltav)
                tutorialsuccess += multiplier;
            break;
        case 4:
            if (Person::players[0]->forwardkeydown || Person::players[0]->backkeydown || Person::players[0]->leftkeydown || Person::players[0]->rightkeydown)
                tutorialsuccess += multiplier;
            break;
        case 5:
            if (Person::players[0]->jumpkeydown)
                tutorialsuccess = 1;
            break;
        case 6:
            if (Person::players[0]->isCrouch())
                tutorialsuccess = 1;
            break;
        case 7:
            if (Person::players[0]->animTarget == rollanim)
                tutorialsuccess = 1;
            break;
        case 8:
            if (Person::players[0]->animTarget == sneakanim)
                tutorialsuccess += multiplier;
            break;
        case 9:
            if (Person::players[0]->animTarget == rabbitrunninganim || Person::players[0]->animTarget == wolfrunninganim)
                tutorialsuccess += multiplier;
            break;
        case 11:
            if (Person::players[0]->isWallJump())
                tutorialsuccess = 1;
            break;
        case 12:
            if (Person::players[0]->animTarget == flipanim)
                tutorialsuccess = 1;
            break;
        case 15:
            if (Person::players[0]->animTarget == upunchanim || Person::players[0]->animTarget == winduppunchanim)
                tutorialsuccess = 1;
            break;
        case 16:
            if (Person::players[0]->animTarget == winduppunchanim)
                tutorialsuccess = 1;
            break;
        case 17:
            if (Person::players[0]->animTarget == spinkickanim)
                tutorialsuccess = 1;
            break;
        case 18:
            if (Person::players[0]->animTarget == sweepanim)
                tutorialsuccess = 1;
            break;
        case 19:
            if (Person::players[0]->animTarget == dropkickanim)
                tutorialsuccess = 1;
            break;
        case 20:
            if (Person::players[0]->animTarget == rabbitkickanim)
                tutorialsuccess = 1;
            break;
        case 21:
            if (bonus == cannon)
                tutorialsuccess = 1;
            break;
        case 22:
            if (bonus == spinecrusher)
                tutorialsuccess = 1;
            break;
        case 23:
            if (Person::players[0]->animTarget == walljumprightkickanim || Person::players[0]->animTarget == walljumpleftkickanim)
                tutorialsuccess = 1;
            break;
        case 24:
            if (Person::players[0]->animTarget == rabbittacklinganim)
                tutorialsuccess = 1;
            break;
        case 25:
            if (Person::players[0]->animTarget == backhandspringanim)
                tutorialsuccess = 1;
            break;
        case 28:
            if (animation[Person::players[0]->animTarget].attack == reversed && Person::players[0]->feint)
                tutorialsuccess = 1;
            break;
        case 29:
            if (Person::players[0]->escapednum == 2) {
                tutorialsuccess = 1;
                reversaltrain = 0;
                cananger = 0;
                Person::players[1]->aitype = passivetype;
            }
            break;
        case 33:
            if (animation[Person::players[0]->animTarget].attack == reversal)
                tutorialsuccess = 1;
            break;
        case 34:
            if (animation[Person::players[0]->animTarget].attack == reversal)
                tutorialsuccess = 1;
            break;
        case 35:
            if (animation[Person::players[0]->animTarget].attack == reversal) {
                tutorialsuccess = 1;
                reversaltrain = 0;
                cananger = 0;
                Person::players[1]->aitype = passivetype;
            }
            break;
        case 40:
            if (Person::players[0]->num_weapons > 0)
                tutorialsuccess = 1;
            break;
        case 41:
            if (Person::players[0]->weaponactive == -1 && Person::players[0]->num_weapons > 0)
                tutorialsuccess = 1;
            break;
        case 43:
            if (Person::players[0]->animTarget == knifeslashstartanim)
                tutorialsuccess = 1;
            break;
        case 44:
            if (animation[Person::players[0]->animTarget].attack == reversal)
                tutorialsuccess = 1;
            break;
        case 45:
            if (animation[Person::players[0]->animTarget].attack == reversal)
                tutorialsuccess = 1;
            break;
        case 46:
            if (animation[Person::players[0]->animTarget].attack == reversal)
                tutorialsuccess = 1;
            break;
        case 49:
            if (Person::players[1]->weaponstuck != -1)
                tutorialsuccess = 1;
            break;
        default:
            break;
        }
        if (tutorialsuccess >= 1)
            tutorialstagetime = tutorialmaxtime - 3;


        if (tutorialstagetime == tutorialmaxtime - 3) {
            emit_sound_np(consolesuccesssound);
        }

        if (tutorialsuccess >= 1) {
            if (tutorialstage == 34 || tutorialstage == 35)
                tutorialstagetime = tutorialmaxtime - 1;
        }
    }

    if (tutorialstage < 14 || tutorialstage >= 50) {
        Person::players[1]->coords.y = 300;
        Person::players[1]->velocity = 0;
    }
}

void doDebugKeys()
{
    float headprop, bodyprop, armprop, legprop;
    if (debugmode) {
        if (Input::isKeyPressed(SDL_SCANCODE_H)) {
            Person::players[0]->damagetolerance = 200000;
            Person::players[0]->damage = 0;
            Person::players[0]->burnt = 0;
            Person::players[0]->permanentdamage = 0;
            Person::players[0]->superpermanentdamage = 0;
        }

        if (Input::isKeyPressed(SDL_SCANCODE_J)) {
            environment++;
            if (environment > 2)
                environment = 0;
            Setenvironment(environment);
        }

        if (Input::isKeyPressed(SDL_SCANCODE_C)) {
            cameramode = !cameramode;
        }

        if (Input::isKeyPressed(SDL_SCANCODE_X) && !Input::isKeyDown(SDL_SCANCODE_LSHIFT)) {
            if (Person::players[0]->num_weapons > 0) {
                if (weapons[Person::players[0]->weaponids[0]].getType() == sword)
                    weapons[Person::players[0]->weaponids[0]].setType(staff);
                else if (weapons[Person::players[0]->weaponids[0]].getType() == staff)
                    weapons[Person::players[0]->weaponids[0]].setType(knife);
                else
                    weapons[Person::players[0]->weaponids[0]].setType(sword);
            }
        }

        if (Input::isKeyPressed(SDL_SCANCODE_X) && Input::isKeyDown(SDL_SCANCODE_LSHIFT)) {
            int closest = findClosestPlayer();
            if (closest >= 0) {
                if (Person::players[closest]->num_weapons) {
                    if (weapons[Person::players[closest]->weaponids[0]].getType() == sword)
                        weapons[Person::players[closest]->weaponids[0]].setType(staff);
                    else if (weapons[Person::players[closest]->weaponids[0]].getType() == staff)
                        weapons[Person::players[closest]->weaponids[0]].setType(knife);
                    else
                        weapons[Person::players[closest]->weaponids[0]].setType(sword);
                }
                if (!Person::players[closest]->num_weapons) {
                    Person::players[closest]->weaponids[0] = weapons.size();

                    weapons.push_back(Weapon(knife, closest));

                    Person::players[closest]->num_weapons = 1;
                }
            }
        }

        if (Input::isKeyDown(SDL_SCANCODE_U)) {
            int closest = findClosestPlayer();
            if (closest >= 0) {
                Person::players[closest]->yaw += multiplier * 50;
                Person::players[closest]->targetyaw = Person::players[closest]->yaw;
            }
        }


        if (Input::isKeyPressed(SDL_SCANCODE_O) && !Input::isKeyDown(SDL_SCANCODE_LSHIFT)) {
            int closest = findClosestPlayer();
            if (Input::isKeyDown(SDL_SCANCODE_LCTRL))
                closest = 0;

            if (closest >= 0) {
                Person::players[closest]->whichskin++;
                if (Person::players[closest]->whichskin > 9)
                    Person::players[closest]->whichskin = 0;
                if (Person::players[closest]->whichskin > 2 && Person::players[closest]->creature == wolftype)
                    Person::players[closest]->whichskin = 0;

                Person::players[closest]->skeleton.drawmodel.textureptr.load(creatureskin[Person::players[closest]->creature][Person::players[closest]->whichskin], 1,
                        &Person::players[closest]->skeleton.skinText[0], &Person::players[closest]->skeleton.skinsize);
            }

            Person::players[closest]->addClothes();
        }

        if (Input::isKeyPressed(SDL_SCANCODE_O) && Input::isKeyDown(SDL_SCANCODE_LSHIFT)) {
            int closest = findClosestPlayer();
            if (closest >= 0) {
                if (Person::players[closest]->creature == wolftype) {
                    headprop = Person::players[closest]->proportionhead.x / 1.1;
                    bodyprop = Person::players[closest]->proportionbody.x / 1.1;
                    armprop = Person::players[closest]->proportionarms.x / 1.1;
                    legprop = Person::players[closest]->proportionlegs.x / 1.1;
                }

                if (Person::players[closest]->creature == rabbittype) {
                    headprop = Person::players[closest]->proportionhead.x / 1.2;
                    bodyprop = Person::players[closest]->proportionbody.x / 1.05;
                    armprop = Person::players[closest]->proportionarms.x / 1.00;
                    legprop = Person::players[closest]->proportionlegs.x / 1.1;
                }


                if (Person::players[closest]->creature == rabbittype) {
                    Person::players[closest]->skeleton.id = closest;
                    Person::players[closest]->skeleton.Load((char *)":Data:Skeleton:Basic Figure Wolf", (char *)":Data:Skeleton:Basic Figure Wolf Low", (char *)":Data:Skeleton:Rabbitbelt", (char *)":Data:Models:Wolf.solid", (char *)":Data:Models:Wolf2.solid", (char *)":Data:Models:Wolf3.solid", (char *)":Data:Models:Wolf4.solid", (char *)":Data:Models:Wolf5.solid", (char *)":Data:Models:Wolf6.solid", (char *)":Data:Models:Wolf7.solid", (char *)":Data:Models:Wolflow.solid", (char *)":Data:Models:Belt.solid", 0);
                    Person::players[closest]->skeleton.drawmodel.textureptr.load(":Data:Textures:Wolf.jpg", 1, &Person::players[closest]->skeleton.skinText[0], &Person::players[closest]->skeleton.skinsize);
                    Person::players[closest]->whichskin = 0;
                    Person::players[closest]->creature = wolftype;

                    Person::players[closest]->proportionhead = 1.1;
                    Person::players[closest]->proportionbody = 1.1;
                    Person::players[closest]->proportionarms = 1.1;
                    Person::players[closest]->proportionlegs = 1.1;
                    Person::players[closest]->proportionlegs.y = 1.1;
                    Person::players[closest]->scale = .23 * 5 * Person::players[0]->scale;

                    Person::players[closest]->damagetolerance = 300;
                } else {
                    Person::players[closest]->skeleton.id = closest;
                    Person::players[closest]->skeleton.Load((char *)":Data:Skeleton:Basic Figure", (char *)":Data:Skeleton:Basic Figurelow", (char *)":Data:Skeleton:Rabbitbelt", (char *)":Data:Models:Body.solid", (char *)":Data:Models:Body2.solid", (char *)":Data:Models:Body3.solid", (char *)":Data:Models:Body4.solid", (char *)":Data:Models:Body5.solid", (char *)":Data:Models:Body6.solid", (char *)":Data:Models:Body7.solid", (char *)":Data:Models:Bodylow.solid", (char *)":Data:Models:Belt.solid", 1);
                    Person::players[closest]->skeleton.drawmodel.textureptr.load(":Data:Textures:Fur3.jpg", 1, &Person::players[closest]->skeleton.skinText[0], &Person::players[closest]->skeleton.skinsize);
                    Person::players[closest]->whichskin = 0;
                    Person::players[closest]->creature = rabbittype;

                    Person::players[closest]->proportionhead = 1.2;
                    Person::players[closest]->proportionbody = 1.05;
                    Person::players[closest]->proportionarms = 1.00;
                    Person::players[closest]->proportionlegs = 1.1;
                    Person::players[closest]->proportionlegs.y = 1.05;
                    Person::players[closest]->scale = .2 * 5 * Person::players[0]->scale;

                    Person::players[closest]->damagetolerance = 200;
                }

                if (Person::players[closest]->creature == wolftype) {
                    Person::players[closest]->proportionhead = 1.1 * headprop;
                    Person::players[closest]->proportionbody = 1.1 * bodyprop;
                    Person::players[closest]->proportionarms = 1.1 * armprop;
                    Person::players[closest]->proportionlegs = 1.1 * legprop;
                }

                if (Person::players[closest]->creature == rabbittype) {
                    Person::players[closest]->proportionhead = 1.2 * headprop;
                    Person::players[closest]->proportionbody = 1.05 * bodyprop;
                    Person::players[closest]->proportionarms = 1.00 * armprop;
                    Person::players[closest]->proportionlegs = 1.1 * legprop;
                    Person::players[closest]->proportionlegs.y = 1.05 * legprop;
                }

            }
        }

        if (Input::isKeyPressed(SDL_SCANCODE_B) && !Input::isKeyDown(SDL_SCANCODE_LSHIFT)) {
            slomo = 1 - slomo;
            slomodelay = 1000;
        }


        if (((Input::isKeyPressed(SDL_SCANCODE_I) && !Input::isKeyDown(SDL_SCANCODE_LSHIFT)))) {
            int closest = -1;
            float closestdist = std::numeric_limits<float>::max();

            for (unsigned i = 1; i < Person::players.size(); i++) {
                float distance = distsq(&Person::players[i]->coords, &Person::players[0]->coords);
                if (!Person::players[i]->headless)
                    if (distance < closestdist) {
                        closestdist = distance;
                        closest = i;
                    }
            }

            XYZ flatfacing2, flatvelocity2;
            XYZ blah;
            if (closest != -1 && distsq(&Person::players[closest]->coords, &Person::players[0]->coords) < 144) {
                blah = Person::players[closest]->coords;
                XYZ headspurtdirection;
                //int i = Person::players[closest]->skeleton.jointlabels[head];
                Joint& headjoint = Person::players[closest]->joint(head);
                for (int k = 0; k < Person::players[closest]->skeleton.num_joints; k++) {
                    if (!Person::players[closest]->skeleton.free)
                        flatvelocity2 = Person::players[closest]->velocity;
                    if (Person::players[closest]->skeleton.free)
                        flatvelocity2 = headjoint.velocity;
                    if (!Person::players[closest]->skeleton.free)
                        flatfacing2 = DoRotation(DoRotation(DoRotation(headjoint.position, 0, 0, Person::players[closest]->tilt), Person::players[closest]->tilt2, 0, 0), 0, Person::players[closest]->yaw, 0) * Person::players[closest]->scale + Person::players[closest]->coords;
                    if (Person::players[closest]->skeleton.free)
                        flatfacing2 = headjoint.position * Person::players[closest]->scale + Person::players[closest]->coords;
                    flatvelocity2.x += (float)(abs(Random() % 100) - 50) / 10;
                    flatvelocity2.y += (float)(abs(Random() % 100) - 50) / 10;
                    flatvelocity2.z += (float)(abs(Random() % 100) - 50) / 10;
                    headspurtdirection = headjoint.position - Person::players[closest]->jointPos(neck);
                    Normalise(&headspurtdirection);
                    Sprite::MakeSprite(bloodflamesprite, flatfacing2, flatvelocity2, 1, 1, 1, .6, 1);
                    flatvelocity2 += headspurtdirection * 8;
                    Sprite::MakeSprite(bloodsprite, flatfacing2, flatvelocity2 / 2, 1, 1, 1, .16, 1);
                }
                Sprite::MakeSprite(cloudsprite, flatfacing2, flatvelocity2 * 0, .6, 0, 0, 1, .5);

                emit_sound_at(splattersound, blah);
                emit_sound_at(breaksound2, blah, 100.);

                if (Person::players[closest]->skeleton.free == 2)
                    Person::players[closest]->skeleton.free = 0;
                Person::players[closest]->RagDoll(0);
                Person::players[closest]->dead = 2;
                Person::players[closest]->headless = 1;
                Person::players[closest]->DoBloodBig(3, 165);

                camerashake += .3;
            }
        }

        if (((Input::isKeyPressed(SDL_SCANCODE_I) && Input::isKeyDown(SDL_SCANCODE_LSHIFT)))) {
            int closest = findClosestPlayer();
            XYZ flatfacing2, flatvelocity2;
            XYZ blah;
            if (closest >= 0 && distsq(&Person::players[closest]->coords, &Person::players[0]->coords) < 144) {
                blah = Person::players[closest]->coords;
                emit_sound_at(splattersound, blah);
                emit_sound_at(breaksound2, blah);

                for (int i = 0; i < Person::players[closest]->skeleton.num_joints; i++) {
                    if (!Person::players[closest]->skeleton.free)
                        flatvelocity2 = Person::players[closest]->velocity;
                    if (Person::players[closest]->skeleton.free)
                        flatvelocity2 = Person::players[closest]->skeleton.joints[i].velocity;
                    if (!Person::players[closest]->skeleton.free)
                        flatfacing2 = DoRotation(DoRotation(DoRotation(Person::players[closest]->skeleton.joints[i].position, 0, 0, Person::players[closest]->tilt), Person::players[closest]->tilt2, 0, 0), 0, Person::players[closest]->yaw, 0) * Person::players[closest]->scale + Person::players[closest]->coords;
                    if (Person::players[closest]->skeleton.free)
                        flatfacing2 = Person::players[closest]->skeleton.joints[i].position * Person::players[closest]->scale + Person::players[closest]->coords;
                    flatvelocity2.x += (float)(abs(Random() % 100) - 50) / 10;
                    flatvelocity2.y += (float)(abs(Random() % 100) - 50) / 10;
                    flatvelocity2.z += (float)(abs(Random() % 100) - 50) / 10;
                    Sprite::MakeSprite(bloodflamesprite, flatfacing2, flatvelocity2, 1, 1, 1, 3, 1);
                    Sprite::MakeSprite(bloodsprite, flatfacing2, flatvelocity2, 1, 1, 1, .3, 1);
                    Sprite::MakeSprite(cloudsprite, flatfacing2, flatvelocity2 * 0, .6, 0, 0, 1, .5);
                }

                for (int i = 0; i < Person::players[closest]->skeleton.num_joints; i++) {
                    if (!Person::players[closest]->skeleton.free)
                        flatvelocity2 = Person::players[closest]->velocity;
                    if (Person::players[closest]->skeleton.free)
                        flatvelocity2 = Person::players[closest]->skeleton.joints[i].velocity;
                    if (!Person::players[closest]->skeleton.free)
                        flatfacing2 = DoRotation(DoRotation(DoRotation(Person::players[closest]->skeleton.joints[i].position, 0, 0, Person::players[closest]->tilt), Person::players[closest]->tilt2, 0, 0), 0, Person::players[closest]->yaw, 0) * Person::players[closest]->scale + Person::players[closest]->coords;
                    if (Person::players[closest]->skeleton.free)
                        flatfacing2 = Person::players[closest]->skeleton.joints[i].position * Person::players[closest]->scale + Person::players[closest]->coords;
                    flatvelocity2.x += (float)(abs(Random() % 100) - 50) / 10;
                    flatvelocity2.y += (float)(abs(Random() % 100) - 50) / 10;
                    flatvelocity2.z += (float)(abs(Random() % 100) - 50) / 10;
                    Sprite::MakeSprite(bloodflamesprite, flatfacing2, flatvelocity2, 1, 1, 1, 3, 1);
                    Sprite::MakeSprite(bloodsprite, flatfacing2, flatvelocity2, 1, 1, 1, .4, 1);
                }

                for (int i = 0; i < Person::players[closest]->skeleton.num_joints; i++) {
                    if (!Person::players[closest]->skeleton.free)
                        flatvelocity2 = Person::players[closest]->velocity;
                    if (Person::players[closest]->skeleton.free)
                        flatvelocity2 = Person::players[closest]->skeleton.joints[i].velocity;
                    if (!Person::players[closest]->skeleton.free)
                        flatfacing2 = DoRotation(DoRotation(DoRotation(Person::players[closest]->skeleton.joints[i].position, 0, 0, Person::players[closest]->tilt), Person::players[closest]->tilt2, 0, 0), 0, Person::players[closest]->yaw, 0) * Person::players[closest]->scale + Person::players[closest]->coords;
                    if (Person::players[closest]->skeleton.free)
                        flatfacing2 = Person::players[closest]->skeleton.joints[i].position * Person::players[closest]->scale + Person::players[closest]->coords;
                    flatvelocity2.x += (float)(abs(Random() % 100) - 50) / 10;
                    flatvelocity2.y += (float)(abs(Random() % 100) - 50) / 10;
                    flatvelocity2.z += (float)(abs(Random() % 100) - 50) / 10;
                    Sprite::MakeSprite(bloodflamesprite, flatfacing2, flatvelocity2 * 2, 1, 1, 1, 3, 1);
                    Sprite::MakeSprite(bloodsprite, flatfacing2, flatvelocity2 * 2, 1, 1, 1, .4, 1);
                }

                for (int i = 0; i < Person::players[closest]->skeleton.num_joints; i++) {
                    if (!Person::players[closest]->skeleton.free)
                        flatvelocity2 = Person::players[closest]->velocity;
                    if (Person::players[closest]->skeleton.free)
                        flatvelocity2 = Person::players[closest]->skeleton.joints[i].velocity;
                    if (!Person::players[closest]->skeleton.free)
                        flatfacing2 = DoRotation(DoRotation(DoRotation(Person::players[closest]->skeleton.joints[i].position, 0, 0, Person::players[closest]->tilt), Person::players[closest]->tilt2, 0, 0), 0, Person::players[closest]->yaw, 0) * Person::players[closest]->scale + Person::players[closest]->coords;
                    if (Person::players[closest]->skeleton.free)
                        flatfacing2 = Person::players[closest]->skeleton.joints[i].position * Person::players[closest]->scale + Person::players[closest]->coords;
                    flatvelocity2.x += (float)(abs(Random() % 100) - 50) / 10;
                    flatvelocity2.y += (float)(abs(Random() % 100) - 50) / 10;
                    flatvelocity2.z += (float)(abs(Random() % 100) - 50) / 10;
                    Sprite::MakeSprite(bloodflamesprite, flatfacing2, flatvelocity2 * 2, 1, 1, 1, 3, 1);
                    Sprite::MakeSprite(bloodsprite, flatfacing2, flatvelocity2 * 2, 1, 1, 1, .4, 1);
                }

                XYZ temppos;
                for (unsigned j = 0; j < Person::players.size(); j++) {
                    if (int(j) != closest) {
                        if (distsq(&Person::players[j]->coords, &Person::players[closest]->coords) < 25) {
                            Person::players[j]->DoDamage((25 - distsq(&Person::players[j]->coords, &Person::players[closest]->coords)) * 60);
                            if (Person::players[j]->skeleton.free == 2)
                                Person::players[j]->skeleton.free = 1;
                            Person::players[j]->skeleton.longdead = 0;
                            Person::players[j]->RagDoll(0);
                            for (int i = 0; i < Person::players[j]->skeleton.num_joints; i++) {
                                temppos = Person::players[j]->skeleton.joints[i].position + Person::players[j]->coords;
                                if (distsq(&temppos, &Person::players[closest]->coords) < 25) {
                                    flatvelocity2 = temppos - Person::players[closest]->coords;
                                    Normalise(&flatvelocity2);
                                    Person::players[j]->skeleton.joints[i].velocity += flatvelocity2 * ((20 - distsq(&temppos, &Person::players[closest]->coords)) * 20);
                                }
                            }
                        }
                    }
                }

                Person::players[closest]->DoDamage(10000);
                Person::players[closest]->RagDoll(0);
                Person::players[closest]->dead = 2;
                Person::players[closest]->coords = 20;
                Person::players[closest]->skeleton.free = 2;

                camerashake += .6;

            }
        }

        if (Input::isKeyPressed(SDL_SCANCODE_F)) {
            Person::players[0]->onfire = 1 - Person::players[0]->onfire;
            if (Person::players[0]->onfire) {
                Person::players[0]->CatchFire();
            }
            if (!Person::players[0]->onfire) {
                emit_sound_at(fireendsound, Person::players[0]->coords);
                pause_sound(stream_firesound);
            }
        }

        if (Input::isKeyPressed(SDL_SCANCODE_N) && !Input::isKeyDown(SDL_SCANCODE_LCTRL)) {
            Person::players[0]->RagDoll(0);

            emit_sound_at(whooshsound, Person::players[0]->coords, 128.);
        }

        if (Input::isKeyPressed(SDL_SCANCODE_N) && Input::isKeyDown(SDL_SCANCODE_LCTRL)) {
            for (int i = 0; i < objects.numobjects; i++) {
                if (objects.type[i] == treeleavestype) {
                    objects.scale[i] *= .9;
                }
            }
        }

        if (Input::isKeyPressed(SDL_SCANCODE_M) && Input::isKeyDown(SDL_SCANCODE_LSHIFT)) {
            editorenabled = !editorenabled;
            if (editorenabled) {
                Person::players[0]->damagetolerance = 100000;
            } else {
                Person::players[0]->damagetolerance = 200;
            }
            Person::players[0]->damage = 0; // these lines were in both if and else, but I think they would better fit in the if
            Person::players[0]->permanentdamage = 0;
            Person::players[0]->superpermanentdamage = 0;
            Person::players[0]->bloodloss = 0;
            Person::players[0]->deathbleeding = 0;
        }

        //skip level
        if (whichlevel != -2 && Input::isKeyPressed(SDL_SCANCODE_K) && Input::isKeyDown(SDL_SCANCODE_LSHIFT) && !editorenabled) {
            targetlevel++;
            if (targetlevel > numchallengelevels - 1)
                targetlevel = 0;
            loading = 1;
            leveltime = 5;
        }

        if (editorenabled) {
            if (Input::isKeyPressed(SDL_SCANCODE_DELETE) && Input::isKeyDown(SDL_SCANCODE_LSHIFT)) {
                int closest = findClosestPlayer();
                if (closest >= 0) {
                    Person::players.erase(Person::players.begin()+closest);
                }
            }

            if (Input::isKeyPressed(SDL_SCANCODE_DELETE) && Input::isKeyDown(SDL_SCANCODE_LCTRL)) {
                int closest = findClosestObject();
                if (closest >= 0)
                    objects.position[closest].y -= 500;
            }

            if (Input::isKeyPressed(SDL_SCANCODE_M) && Input::isKeyDown(SDL_SCANCODE_LSHIFT)) {
                if (objects.numobjects < max_objects - 1) {
                    XYZ boxcoords;
                    boxcoords.x = Person::players[0]->coords.x;
                    boxcoords.z = Person::players[0]->coords.z;
                    boxcoords.y = Person::players[0]->coords.y - 3;
                    if (editortype == bushtype)
                        boxcoords.y = Person::players[0]->coords.y - .5;
                    if (editortype == firetype)
                        boxcoords.y = Person::players[0]->coords.y - .5;
                    //objects.MakeObject(abs(Random()%3),boxcoords,Random()%360);
                    float temprotat, temprotat2;
                    temprotat = editoryaw;
                    temprotat2 = editorpitch;
                    if (temprotat < 0 || editortype == bushtype)
                        temprotat = Random() % 360;
                    if (temprotat2 < 0)
                        temprotat2 = Random() % 360;

                    objects.MakeObject(editortype, boxcoords, (int)temprotat - ((int)temprotat) % 30, (int)temprotat2, editorsize);
                    if (editortype == treetrunktype)
                        objects.MakeObject(treeleavestype, boxcoords, Random() % 360 * (temprotat2 < 2) + (int)editoryaw - ((int)editoryaw) % 30, editorpitch, editorsize);
                }
            }

            if (Input::isKeyPressed(SDL_SCANCODE_P) && Input::isKeyDown(SDL_SCANCODE_LSHIFT) && !Input::isKeyDown(SDL_SCANCODE_LCTRL)) {
                Person::players.push_back(shared_ptr<Person>(new Person()));

                Person::players.back()->scale = .2 * 5 * Person::players[0]->scale;
                Person::players.back()->creature = rabbittype;
                Person::players.back()->howactive = editoractive;
                Person::players.back()->skeleton.id = Person::players.size()-1;
                Person::players.back()->skeleton.Load((char *)":Data:Skeleton:Basic Figure", (char *)":Data:Skeleton:Basic Figurelow", (char *)":Data:Skeleton:Rabbitbelt", (char *)":Data:Models:Body.solid", (char *)":Data:Models:Body2.solid", (char *)":Data:Models:Body3.solid", (char *)":Data:Models:Body4.solid", (char *)":Data:Models:Body5.solid", (char *)":Data:Models:Body6.solid", (char *)":Data:Models:Body7.solid", (char *)":Data:Models:Bodylow.solid", (char *)":Data:Models:Belt.solid", 1);

                int k = abs(Random() % 2) + 1;
                if (k == 0) {
                    Person::players.back()->skeleton.drawmodel.textureptr.load(":Data:Textures:Fur3.jpg", 1, &Person::players.back()->skeleton.skinText[0], &Person::players.back()->skeleton.skinsize);
                    Person::players.back()->whichskin = 0;
                } else if (k == 1) {
                    Person::players.back()->skeleton.drawmodel.textureptr.load(":Data:Textures:Fur.jpg", 1, &Person::players.back()->skeleton.skinText[0], &Person::players.back()->skeleton.skinsize);
                    Person::players.back()->whichskin = 1;
                } else {
                    Person::players.back()->skeleton.drawmodel.textureptr.load(":Data:Textures:Fur2.jpg", 1, &Person::players.back()->skeleton.skinText[0], &Person::players.back()->skeleton.skinsize);
                    Person::players.back()->whichskin = 2;
                }

                Person::players.back()->skeleton.drawmodelclothes.textureptr.load(":Data:Textures:Belt.png", 1);
                Person::players.back()->power = 1;
                Person::players.back()->speedmult = 1;
                Person::players.back()->animCurrent = bounceidleanim;
                Person::players.back()->animTarget = bounceidleanim;
                Person::players.back()->frameCurrent = 0;
                Person::players.back()->frameTarget = 1;
                Person::players.back()->target = 0;
                Person::players.back()->bled = 0;
                Person::players.back()->speed = 1 + (float)(Random() % 100) / 1000;

                Person::players.back()->targetyaw = Person::players[0]->targetyaw;
                Person::players.back()->yaw = Person::players[0]->yaw;

                Person::players.back()->velocity = 0;
                Person::players.back()->coords = Person::players[0]->coords;
                Person::players.back()->oldcoords = Person::players.back()->coords;
                Person::players.back()->realoldcoords = Person::players.back()->coords;

                Person::players.back()->id = Person::players.size()-1;
                Person::players.back()->updatedelay = 0;
                Person::players.back()->normalsupdatedelay = 0;

                Person::players.back()->aitype = passivetype;

                if (Person::players[0]->creature == wolftype) {
                    headprop = Person::players[0]->proportionhead.x / 1.1;
                    bodyprop = Person::players[0]->proportionbody.x / 1.1;
                    armprop = Person::players[0]->proportionarms.x / 1.1;
                    legprop = Person::players[0]->proportionlegs.x / 1.1;
                }

                if (Person::players[0]->creature == rabbittype) {
                    headprop = Person::players[0]->proportionhead.x / 1.2;
                    bodyprop = Person::players[0]->proportionbody.x / 1.05;
                    armprop = Person::players[0]->proportionarms.x / 1.00;
                    legprop = Person::players[0]->proportionlegs.x / 1.1;
                }

                if (Person::players.back()->creature == wolftype) {
                    Person::players.back()->proportionhead = 1.1 * headprop;
                    Person::players.back()->proportionbody = 1.1 * bodyprop;
                    Person::players.back()->proportionarms = 1.1 * armprop;
                    Person::players.back()->proportionlegs = 1.1 * legprop;
                }

                if (Person::players.back()->creature == rabbittype) {
                    Person::players.back()->proportionhead = 1.2 * headprop;
                    Person::players.back()->proportionbody = 1.05 * bodyprop;
                    Person::players.back()->proportionarms = 1.00 * armprop;
                    Person::players.back()->proportionlegs = 1.1 * legprop;
                    Person::players.back()->proportionlegs.y = 1.05 * legprop;
                }

                Person::players.back()->headless = 0;
                Person::players.back()->onfire = 0;

                if (cellophane) {
                    Person::players.back()->proportionhead.z = 0;
                    Person::players.back()->proportionbody.z = 0;
                    Person::players.back()->proportionarms.z = 0;
                    Person::players.back()->proportionlegs.z = 0;
                }

                Person::players.back()->tempanimation.Load((char *)"Tempanim", 0, 0);

                Person::players.back()->damagetolerance = 200;

                Person::players.back()->protectionhead = Person::players[0]->protectionhead;
                Person::players.back()->protectionhigh = Person::players[0]->protectionhigh;
                Person::players.back()->protectionlow = Person::players[0]->protectionlow;
                Person::players.back()->armorhead = Person::players[0]->armorhead;
                Person::players.back()->armorhigh = Person::players[0]->armorhigh;
                Person::players.back()->armorlow = Person::players[0]->armorlow;
                Person::players.back()->metalhead = Person::players[0]->metalhead;
                Person::players.back()->metalhigh = Person::players[0]->metalhigh;
                Person::players.back()->metallow = Person::players[0]->metallow;

                Person::players.back()->immobile = Person::players[0]->immobile;

                Person::players.back()->numclothes = Person::players[0]->numclothes;
                for (int i = 0; i < Person::players.back()->numclothes; i++) {
                    strcpy(Person::players.back()->clothes[i], Person::players[0]->clothes[i]);
                    Person::players.back()->clothestintr[i] = Person::players[0]->clothestintr[i];
                    Person::players.back()->clothestintg[i] = Person::players[0]->clothestintg[i];
                    Person::players.back()->clothestintb[i] = Person::players[0]->clothestintb[i];
                }
                Person::players.back()->addClothes();

                Person::players.back()->power = Person::players[0]->power;
                Person::players.back()->speedmult = Person::players[0]->speedmult;

                Person::players.back()->damage = 0;
                Person::players.back()->permanentdamage = 0;
                Person::players.back()->superpermanentdamage = 0;
                Person::players.back()->deathbleeding = 0;
                Person::players.back()->bleeding = 0;
                Person::players.back()->numwaypoints = 0;
                Person::players.back()->waypoint = 0;
                Person::players.back()->weaponstuck = -1;
                Person::players.back()->weaponactive = -1;
                Person::players.back()->num_weapons = 0;
                Person::players.back()->bloodloss = 0;
                Person::players.back()->dead = 0;

                Person::players.back()->loaded = 1;
            }

            if (Input::isKeyPressed(SDL_SCANCODE_P) && Input::isKeyDown(SDL_SCANCODE_LSHIFT)) {
                if (Person::players.back()->numwaypoints < 90) {
                    Person::players.back()->waypoints[Person::players.back()->numwaypoints] = Person::players[0]->coords;
                    Person::players.back()->waypointtype[Person::players.back()->numwaypoints] = editorpathtype;
                    Person::players.back()->numwaypoints++;
                }
            }

            if (Input::isKeyPressed(SDL_SCANCODE_P) && Input::isKeyDown(SDL_SCANCODE_LCTRL)) {
                if (numpathpoints < 30) {
                    bool connected, alreadyconnected;
                    connected = 0;
                    if (numpathpoints > 1)
                        for (int i = 0; i < numpathpoints; i++) {
                            if (distsq(&pathpoint[i], &Person::players[0]->coords) < .5 && i != pathpointselected && !connected) {
                                alreadyconnected = 0;
                                for (int j = 0; j < numpathpointconnect[pathpointselected]; j++) {
                                    if (pathpointconnect[pathpointselected][j] == i)
                                        alreadyconnected = 1;
                                }
                                if (!alreadyconnected) {
                                    numpathpointconnect[pathpointselected]++;
                                    connected = 1;
                                    pathpointconnect[pathpointselected][numpathpointconnect[pathpointselected] - 1] = i;
                                }
                            }
                        }
                    if (!connected) {
                        numpathpoints++;
                        pathpoint[numpathpoints - 1] = Person::players[0]->coords;
                        numpathpointconnect[numpathpoints - 1] = 0;
                        if (numpathpoints > 1 && pathpointselected != -1) {
                            numpathpointconnect[pathpointselected]++;
                            pathpointconnect[pathpointselected][numpathpointconnect[pathpointselected] - 1] = numpathpoints - 1;
                        }
                        pathpointselected = numpathpoints - 1;
                    }
                }
            }

            if (Input::isKeyPressed(SDL_SCANCODE_PERIOD)) {
                pathpointselected++;
                if (pathpointselected >= numpathpoints)
                    pathpointselected = -1;
            }
            if (Input::isKeyPressed(SDL_SCANCODE_COMMA) && !Input::isKeyDown(SDL_SCANCODE_LSHIFT)) {
                pathpointselected--;
                if (pathpointselected <= -2)
                    pathpointselected = numpathpoints - 1;
            }
            if (Input::isKeyPressed(SDL_SCANCODE_COMMA) && Input::isKeyDown(SDL_SCANCODE_LSHIFT)) {
                if (pathpointselected != -1) {
                    numpathpoints--;
                    pathpoint[pathpointselected] = pathpoint[numpathpoints];
                    numpathpointconnect[pathpointselected] = numpathpointconnect[numpathpoints];
                    for (int i = 0; i < numpathpointconnect[pathpointselected]; i++) {
                        pathpointconnect[pathpointselected][i] = pathpointconnect[numpathpoints][i];
                    }
                    for (int i = 0; i < numpathpoints; i++) {
                        for (int j = 0; j < numpathpointconnect[i]; j++) {
                            if (pathpointconnect[i][j] == pathpointselected) {
                                pathpointconnect[i][j] = pathpointconnect[i][numpathpointconnect[i] - 1];
                                numpathpointconnect[i]--;
                            }
                            if (pathpointconnect[i][j] == numpathpoints) {
                                pathpointconnect[i][j] = pathpointselected;
                            }
                        }
                    }
                    pathpointselected = numpathpoints - 1;
                }
            }

            if (Input::isKeyPressed(SDL_SCANCODE_LEFT) && Input::isKeyDown(SDL_SCANCODE_LSHIFT) && !Input::isKeyDown(SDL_SCANCODE_LCTRL)) {
                editortype--;
                if (editortype == treeleavestype || editortype == 10)
                    editortype--;
                if (editortype < 0)
                    editortype = firetype;
            }

            if (Input::isKeyPressed(SDL_SCANCODE_RIGHT) && Input::isKeyDown(SDL_SCANCODE_LSHIFT) && !Input::isKeyDown(SDL_SCANCODE_LCTRL)) {
                editortype++;
                if (editortype == treeleavestype || editortype == 10)
                    editortype++;
                if (editortype > firetype)
                    editortype = 0;
            }

            if (Input::isKeyDown(SDL_SCANCODE_LEFT) && !Input::isKeyDown(SDL_SCANCODE_LSHIFT) && !Input::isKeyDown(SDL_SCANCODE_LCTRL)) {
                editoryaw -= multiplier * 100;
                if (editoryaw < -.01)
                    editoryaw = -.01;
            }

            if (Input::isKeyDown(SDL_SCANCODE_RIGHT) && !Input::isKeyDown(SDL_SCANCODE_LSHIFT) && !Input::isKeyDown(SDL_SCANCODE_LCTRL)) {
                editoryaw += multiplier * 100;
            }

            if (Input::isKeyDown(SDL_SCANCODE_UP) && !Input::isKeyDown(SDL_SCANCODE_LCTRL)) {
                editorsize += multiplier;
            }

            if (Input::isKeyDown(SDL_SCANCODE_DOWN) && !Input::isKeyDown(SDL_SCANCODE_LCTRL)) {
                editorsize -= multiplier;
                if (editorsize < .1)
                    editorsize = .1;
            }


            if (Input::isKeyPressed(SDL_SCANCODE_LEFT) && Input::isKeyDown(SDL_SCANCODE_LSHIFT) && Input::isKeyDown(SDL_SCANCODE_LCTRL)) {
                mapradius -= multiplier * 10;
            }

            if (Input::isKeyPressed(SDL_SCANCODE_RIGHT) && Input::isKeyDown(SDL_SCANCODE_LSHIFT) && Input::isKeyDown(SDL_SCANCODE_LCTRL)) {
                mapradius += multiplier * 10;
            }
            if (Input::isKeyDown(SDL_SCANCODE_UP) && Input::isKeyDown(SDL_SCANCODE_LCTRL)) {
                editorpitch += multiplier * 100;
            }

            if (Input::isKeyDown(SDL_SCANCODE_DOWN) && Input::isKeyDown(SDL_SCANCODE_LCTRL)) {
                editorpitch -= multiplier * 100;
                if (editorpitch < -.01)
                    editorpitch = -.01;
            }
            if (Input::isKeyPressed(SDL_SCANCODE_DELETE) && objects.numobjects && Input::isKeyDown(SDL_SCANCODE_LSHIFT)) {
                int closest = findClosestObject();
                if (closest >= 0)
                    objects.DeleteObject(closest);
            }
        }
    }
}

void doJumpReversals()
{
    for (unsigned k = 0; k < Person::players.size(); k++)
        for (unsigned i = k; i < Person::players.size(); i++) {
            if (i == k)
                continue;
            if (     Person::players[k]->skeleton.free == 0 &&
                     Person::players[i]->skeleton.oldfree == 0 &&
                     (Person::players[i]->animTarget == jumpupanim ||
                      Person::players[k]->animTarget == jumpupanim) &&
                     (Person::players[i]->aitype == playercontrolled ||
                      Person::players[k]->aitype == playercontrolled) &&
                     ((Person::players[i]->aitype == attacktypecutoff && Person::players[i]->stunned <= 0) ||
                      (Person::players[k]->aitype == attacktypecutoff && Person::players[k]->stunned <= 0))) {
                if (     distsq(&Person::players[i]->coords, &Person::players[k]->coords) < 10 * sq((Person::players[i]->scale + Person::players[k]->scale) * 2.5) &&
                         distsqflat(&Person::players[i]->coords, &Person::players[k]->coords) < 2 * sq((Person::players[i]->scale + Person::players[k]->scale) * 2.5)) {
                    //TODO: refactor two huge similar ifs
                    if (Person::players[i]->animTarget == jumpupanim &&
                            Person::players[k]->animTarget != getupfrombackanim &&
                            Person::players[k]->animTarget != getupfromfrontanim &&
                            animation[Person::players[k]->animTarget].height == middleheight &&
                            normaldotproduct(Person::players[i]->velocity, Person::players[k]->coords - Person::players[i]->coords) < 0 &&
                            ((Person::players[k]->aitype == playercontrolled && Person::players[k]->attackkeydown) ||
                             Person::players[k]->aitype != playercontrolled)) {
                        Person::players[i]->victim = Person::players[k];
                        Person::players[i]->velocity = 0;
                        Person::players[i]->animCurrent = jumpreversedanim;
                        Person::players[i]->animTarget = jumpreversedanim;
                        Person::players[i]->frameCurrent = 0;
                        Person::players[i]->frameTarget = 1;
                        Person::players[i]->targettilt2 = 0;
                        Person::players[k]->victim = Person::players[i];
                        Person::players[k]->velocity = 0;
                        Person::players[k]->animCurrent = jumpreversalanim;
                        Person::players[k]->animTarget = jumpreversalanim;
                        Person::players[k]->frameCurrent = 0;
                        Person::players[k]->frameTarget = 1;
                        Person::players[k]->targettilt2 = 0;
                        if (Person::players[i]->coords.y < Person::players[k]->coords.y + 1) {
                            Person::players[i]->animCurrent = rabbitkickreversedanim;
                            Person::players[i]->animTarget = rabbitkickreversedanim;
                            Person::players[i]->frameCurrent = 1;
                            Person::players[i]->frameTarget = 2;
                            Person::players[k]->animCurrent = rabbitkickreversalanim;
                            Person::players[k]->animTarget = rabbitkickreversalanim;
                            Person::players[k]->frameCurrent = 1;
                            Person::players[k]->frameTarget = 2;
                        }
                        Person::players[i]->target = 0;
                        Person::players[k]->oldcoords = Person::players[k]->coords;
                        Person::players[i]->coords = Person::players[k]->coords;
                        Person::players[k]->targetyaw = Person::players[i]->targetyaw;
                        Person::players[k]->yaw = Person::players[i]->targetyaw;
                        if (Person::players[k]->aitype == attacktypecutoff)
                            Person::players[k]->stunned = .5;
                    }
                    if (Person::players[k]->animTarget == jumpupanim &&
                            Person::players[i]->animTarget != getupfrombackanim &&
                            Person::players[i]->animTarget != getupfromfrontanim &&
                            animation[Person::players[i]->animTarget].height == middleheight &&
                            normaldotproduct(Person::players[k]->velocity, Person::players[i]->coords - Person::players[k]->coords) < 0 &&
                            ((Person::players[i]->aitype == playercontrolled && Person::players[i]->attackkeydown) ||
                             Person::players[i]->aitype != playercontrolled)) {
                        Person::players[k]->victim = Person::players[i];
                        Person::players[k]->velocity = 0;
                        Person::players[k]->animCurrent = jumpreversedanim;
                        Person::players[k]->animTarget = jumpreversedanim;
                        Person::players[k]->frameCurrent = 0;
                        Person::players[k]->frameTarget = 1;
                        Person::players[k]->targettilt2 = 0;
                        Person::players[i]->victim = Person::players[k];
                        Person::players[i]->velocity = 0;
                        Person::players[i]->animCurrent = jumpreversalanim;
                        Person::players[i]->animTarget = jumpreversalanim;
                        Person::players[i]->frameCurrent = 0;
                        Person::players[i]->frameTarget = 1;
                        Person::players[i]->targettilt2 = 0;
                        if (Person::players[k]->coords.y < Person::players[i]->coords.y + 1) {
                            Person::players[k]->animTarget = rabbitkickreversedanim;
                            Person::players[k]->animCurrent = rabbitkickreversedanim;
                            Person::players[i]->animCurrent = rabbitkickreversalanim;
                            Person::players[i]->animTarget = rabbitkickreversalanim;
                            Person::players[k]->frameCurrent = 1;
                            Person::players[k]->frameTarget = 2;
                            Person::players[i]->frameCurrent = 1;
                            Person::players[i]->frameTarget = 2;
                        }
                        Person::players[k]->target = 0;
                        Person::players[i]->oldcoords = Person::players[i]->coords;
                        Person::players[k]->coords = Person::players[i]->coords;
                        Person::players[i]->targetyaw = Person::players[k]->targetyaw;
                        Person::players[i]->yaw = Person::players[k]->targetyaw;
                        if (Person::players[i]->aitype == attacktypecutoff)
                            Person::players[i]->stunned = .5;
                    }
                }
            }
        }
}

void doAerialAcrobatics()
{
    static XYZ facing, flatfacing;
    for (unsigned k = 0; k < Person::players.size(); k++) {
        Person::players[k]->turnspeed = 500;

        if ((Person::players[k]->isRun() &&
                ((Person::players[k]->targetyaw != rabbitrunninganim &&
                  Person::players[k]->targetyaw != wolfrunninganim) ||
                 Person::players[k]->frameTarget == 4)) ||
                Person::players[k]->animTarget == removeknifeanim ||
                Person::players[k]->animTarget == crouchremoveknifeanim ||
                Person::players[k]->animTarget == flipanim ||
                Person::players[k]->animTarget == fightsidestep ||
                Person::players[k]->animTarget == walkanim) {
            Person::players[k]->yaw = stepTowardf(Person::players[k]->yaw, Person::players[k]->targetyaw, multiplier * Person::players[k]->turnspeed);
        }


        if (Person::players[k]->isStop() ||
                Person::players[k]->isLanding() ||
                Person::players[k]->animTarget == staggerbackhighanim ||
                (Person::players[k]->animTarget == sneakanim && Person::players[k]->animCurrent == sneakanim) ||
                Person::players[k]->animTarget == staggerbackhardanim ||
                Person::players[k]->animTarget == backhandspringanim ||
                Person::players[k]->animTarget == dodgebackanim ||
                Person::players[k]->animTarget == rollanim ||
                (animation[Person::players[k]->animTarget].attack &&
                 Person::players[k]->animTarget != rabbitkickanim &&
                 (Person::players[k]->animTarget != crouchstabanim || Person::players[k]->hasvictim) &&
                 (Person::players[k]->animTarget != swordgroundstabanim || Person::players[k]->hasvictim))) {
            Person::players[k]->yaw = stepTowardf(Person::players[k]->yaw, Person::players[k]->targetyaw, multiplier * Person::players[k]->turnspeed * 2);
        }

        if (Person::players[k]->animTarget == sneakanim && Person::players[k]->animCurrent != sneakanim) {
            Person::players[k]->yaw = stepTowardf(Person::players[k]->yaw, Person::players[k]->targetyaw, multiplier * Person::players[k]->turnspeed * 4);
        }

        Person::players[k]->DoStuff();
        if (Person::players[k]->immobile && k != 0)
            Person::players[k]->coords = Person::players[k]->realoldcoords;

        //if player's position has changed (?)
        if (distsq(&Person::players[k]->coords, &Person::players[k]->realoldcoords) > 0 &&
                !Person::players[k]->skeleton.free &&
                Person::players[k]->animTarget != climbanim &&
                Person::players[k]->animTarget != hanganim) {
            XYZ lowpoint, lowpointtarget, lowpoint2, lowpointtarget2, lowpoint3, lowpointtarget3, lowpoint4, lowpointtarget4, lowpoint5, lowpointtarget5, lowpoint6, lowpointtarget6, lowpoint7, lowpointtarget7, colpoint, colpoint2;
            int whichhit;
            bool tempcollide = 0;

            if (Person::players[k]->collide < -.3)
                Person::players[k]->collide = -.3;
            if (Person::players[k]->collide > 1)
                Person::players[k]->collide = 1;
            Person::players[k]->collide -= multiplier * 30;

            //clip to terrain
            Person::players[k]->coords.y = max(Person::players[k]->coords.y, terrain.getHeight(Person::players[k]->coords.x, Person::players[k]->coords.z));

            for (int l = 0; l < terrain.patchobjectnum[Person::players[k]->whichpatchx][Person::players[k]->whichpatchz]; l++) {
                int i = terrain.patchobjects[Person::players[k]->whichpatchx][Person::players[k]->whichpatchz][l];
                if (objects.type[i] != rocktype ||
                        objects.scale[i] > .5 && Person::players[k]->aitype == playercontrolled ||
                        objects.position[i].y > Person::players[k]->coords.y) {
                    lowpoint = Person::players[k]->coords;
                    if (Person::players[k]->animTarget != jumpupanim &&
                            Person::players[k]->animTarget != jumpdownanim &&
                            !Person::players[k]->isFlip())
                        lowpoint.y += 1.25;
                    else
                        lowpoint.y += 1.3;
                    if (     Person::players[k]->coords.y < terrain.getHeight(Person::players[k]->coords.x, Person::players[k]->coords.z) &&
                             Person::players[k]->coords.y > terrain.getHeight(Person::players[k]->coords.x, Person::players[k]->coords.z) - .1)
                        Person::players[k]->coords.y = terrain.getHeight(Person::players[k]->coords.x, Person::players[k]->coords.z);
                    if (Person::players[k]->SphereCheck(&lowpoint, 1.3, &colpoint, &objects.position[i], &objects.yaw[i], &objects.model[i]) != -1) {
                        flatfacing = lowpoint - Person::players[k]->coords;
                        Person::players[k]->coords = lowpoint;
                        Person::players[k]->coords.y -= 1.3;
                        Person::players[k]->collide = 1;
                        tempcollide = 1;
                        //wall jumps
                        //TODO: refactor four similar blocks
                        if (Person::players[k]->aitype == playercontrolled &&
                                (Person::players[k]->animTarget == jumpupanim ||
                                 Person::players[k]->animTarget == jumpdownanim ||
                                 Person::players[k]->isFlip()) &&
                                !Person::players[k]->jumptogglekeydown &&
                                Person::players[k]->jumpkeydown) {
                            lowpointtarget = lowpoint + DoRotation(Person::players[k]->facing, 0, -90, 0) * 1.5;
                            XYZ tempcoords1 = lowpoint;
                            whichhit = objects.model[i].LineCheck(&lowpoint, &lowpointtarget, &colpoint, &objects.position[i], &objects.yaw[i]);
                            if (whichhit != -1 && fabs(objects.model[i].facenormals[whichhit].y) < .3) {
                                Person::players[k]->setAnimation(walljumpleftanim);
                                emit_sound_at(movewhooshsound, Person::players[k]->coords);
                                if (k == 0)
                                    pause_sound(whooshsound);

                                lowpointtarget = DoRotation(objects.model[i].facenormals[whichhit], 0, objects.yaw[i], 0);
                                Person::players[k]->yaw = -asin(0 - lowpointtarget.x) * 180 / M_PI;
                                if (lowpointtarget.z < 0)
                                    Person::players[k]->yaw = 180 - Person::players[k]->yaw;
                                Person::players[k]->targetyaw = Person::players[k]->yaw;
                                Person::players[k]->lowyaw = Person::players[k]->yaw;
                                if (k == 0)
                                    numwallflipped++;
                            } else {
                                lowpoint = tempcoords1;
                                lowpointtarget = lowpoint + DoRotation(Person::players[k]->facing, 0, 90, 0) * 1.5;
                                whichhit = objects.model[i].LineCheck(&lowpoint, &lowpointtarget, &colpoint, &objects.position[i], &objects.yaw[i]);
                                if (whichhit != -1 && fabs(objects.model[i].facenormals[whichhit].y) < .3) {
                                    Person::players[k]->setAnimation(walljumprightanim);
                                    emit_sound_at(movewhooshsound, Person::players[k]->coords);
                                    if (k == 0)
                                        pause_sound(whooshsound);

                                    lowpointtarget = DoRotation(objects.model[i].facenormals[whichhit], 0, objects.yaw[i], 0);
                                    Person::players[k]->yaw = -asin(0 - lowpointtarget.x) * 180 / M_PI;
                                    if (lowpointtarget.z < 0)
                                        Person::players[k]->yaw = 180 - Person::players[k]->yaw;
                                    Person::players[k]->targetyaw = Person::players[k]->yaw;
                                    Person::players[k]->lowyaw = Person::players[k]->yaw;
                                    if (k == 0)
                                        numwallflipped++;
                                } else {
                                    lowpoint = tempcoords1;
                                    lowpointtarget = lowpoint + Person::players[k]->facing * 2;
                                    whichhit = objects.model[i].LineCheck(&lowpoint, &lowpointtarget, &colpoint, &objects.position[i], &objects.yaw[i]);
                                    if (whichhit != -1 && fabs(objects.model[i].facenormals[whichhit].y) < .3) {
                                        Person::players[k]->setAnimation(walljumpbackanim);
                                        emit_sound_at(movewhooshsound, Person::players[k]->coords);
                                        if (k == 0)
                                            pause_sound(whooshsound);

                                        lowpointtarget = DoRotation(objects.model[i].facenormals[whichhit], 0, objects.yaw[i], 0);
                                        Person::players[k]->yaw = -asin(0 - lowpointtarget.x) * 180 / M_PI;
                                        if (lowpointtarget.z < 0)
                                            Person::players[k]->yaw = 180 - Person::players[k]->yaw;
                                        Person::players[k]->targetyaw = Person::players[k]->yaw;
                                        Person::players[k]->lowyaw = Person::players[k]->yaw;
                                        if (k == 0)
                                            numwallflipped++;
                                    } else {
                                        lowpoint = tempcoords1;
                                        lowpointtarget = lowpoint - Person::players[k]->facing * 2;
                                        whichhit = objects.model[i].LineCheck(&lowpoint, &lowpointtarget, &colpoint, &objects.position[i], &objects.yaw[i]);
                                        if (whichhit != -1 && fabs(objects.model[i].facenormals[whichhit].y) < .3) {
                                            Person::players[k]->setAnimation(walljumpfrontanim);
                                            emit_sound_at(movewhooshsound, Person::players[k]->coords);
                                            if (k == 0)
                                                pause_sound(whooshsound);

                                            lowpointtarget = DoRotation(objects.model[i].facenormals[whichhit], 0, objects.yaw[i], 0);
                                            Person::players[k]->yaw = -asin(0 - lowpointtarget.x) * 180 / M_PI;
                                            if (lowpointtarget.z < 0)
                                                Person::players[k]->yaw = 180 - Person::players[k]->yaw;
                                            Person::players[k]->yaw += 180;
                                            Person::players[k]->targetyaw = Person::players[k]->yaw;
                                            Person::players[k]->lowyaw = Person::players[k]->yaw;
                                            if (k == 0)
                                                numwallflipped++;
                                        }
                                    }
                                }
                            }
                        }
                    }
                } else if (objects.type[i] == rocktype) {
                    lowpoint2 = Person::players[k]->coords;
                    lowpoint = Person::players[k]->coords;
                    lowpoint.y += 2;
                    if (objects.model[i].LineCheck(&lowpoint, &lowpoint2, &colpoint, &objects.position[i], &objects.yaw[i]) != -1) {
                        Person::players[k]->coords = colpoint;
                        Person::players[k]->collide = 1;
                        tempcollide = 1;

                        if (Person::players[k]->animTarget == jumpdownanim || Person::players[k]->isFlip()) {
                            //flipped into a rock
                            if (Person::players[k]->isFlip() && animation[Person::players[k]->animTarget].label[Person::players[k]->frameTarget] == 7)
                                Person::players[k]->RagDoll(0);

                            if (Person::players[k]->animTarget == jumpupanim) {
                                Person::players[k]->jumppower = -4;
                                Person::players[k]->animTarget = Person::players[k]->getIdle();
                            }
                            Person::players[k]->target = 0;
                            Person::players[k]->frameTarget = 0;
                            Person::players[k]->onterrain = 1;

                            if (Person::players[k]->id == 0) {
                                pause_sound(whooshsound);
                                OPENAL_SetVolume(channels[whooshsound], 0);
                            }

                            //landing
                            if ((Person::players[k]->animTarget == jumpdownanim || Person::players[k]->isFlip()) && !Person::players[k]->wasLanding()) {
                                if (Person::players[k]->isFlip())
                                    Person::players[k]->jumppower = -4;
                                Person::players[k]->animTarget = Person::players[k]->getLanding();
                                emit_sound_at(landsound, Person::players[k]->coords, 128.);
                                if (k == 0) {
                                    addEnvSound(Person::players[k]->coords);
                                }
                            }
                        }
                    }
                }
            }

            if (tempcollide)
                for (int l = 0; l < terrain.patchobjectnum[Person::players[k]->whichpatchx][Person::players[k]->whichpatchz]; l++) {
                    int i = terrain.patchobjects[Person::players[k]->whichpatchx][Person::players[k]->whichpatchz][l];
                    lowpoint = Person::players[k]->coords;
                    lowpoint.y += 1.35;
                    if (objects.type[i] != rocktype)
                        if (Person::players[k]->SphereCheck(&lowpoint, 1.33, &colpoint, &objects.position[i], &objects.yaw[i], &objects.model[i]) != -1) {
                            if (Person::players[k]->animTarget != jumpupanim &&
                                    Person::players[k]->animTarget != jumpdownanim &&
                                    Person::players[k]->onterrain)
                                Person::players[k]->avoidcollided = 1;
                            Person::players[k]->coords = lowpoint;
                            Person::players[k]->coords.y -= 1.35;
                            Person::players[k]->collide = 1;

                            if ((Person::players[k]->grabdelay <= 0 || Person::players[k]->aitype != playercontrolled) &&
                                    (Person::players[k]->animCurrent != climbanim &&
                                     Person::players[k]->animCurrent != hanganim &&
                                     !Person::players[k]->isWallJump() ||
                                     Person::players[k]->animTarget == jumpupanim ||
                                     Person::players[k]->animTarget == jumpdownanim)) {
                                lowpoint = Person::players[k]->coords;
                                objects.model[i].SphereCheckPossible(&lowpoint, 1.5, &objects.position[i], &objects.yaw[i]);
                                lowpoint = Person::players[k]->coords;
                                lowpoint.y += .05;
                                facing = 0;
                                facing.z = -1;
                                facing = DoRotation(facing, 0, Person::players[k]->targetyaw + 180, 0);
                                lowpointtarget = lowpoint + facing * 1.4;
                                whichhit = objects.model[i].LineCheckPossible(&lowpoint, &lowpointtarget, &colpoint, &objects.position[i], &objects.yaw[i]);
                                if (whichhit != -1) {
                                    lowpoint = Person::players[k]->coords;
                                    lowpoint.y += .1;
                                    lowpointtarget = lowpoint + facing * 1.4;
                                    lowpoint2 = lowpoint;
                                    lowpointtarget2 = lowpointtarget;
                                    lowpoint3 = lowpoint;
                                    lowpointtarget3 = lowpointtarget;
                                    lowpoint4 = lowpoint;
                                    lowpointtarget4 = lowpointtarget;
                                    lowpoint5 = lowpoint;
                                    lowpointtarget5 = lowpointtarget;
                                    lowpoint6 = lowpoint;
                                    lowpointtarget6 = lowpointtarget;
                                    lowpoint7 = lowpoint;
                                    lowpointtarget7 = lowpoint;
                                    lowpoint2.x += .1;
                                    lowpointtarget2.x += .1;
                                    lowpoint3.z += .1;
                                    lowpointtarget3.z += .1;
                                    lowpoint4.x -= .1;
                                    lowpointtarget4.x -= .1;
                                    lowpoint5.z -= .1;
                                    lowpointtarget5.z -= .1;
                                    lowpoint6.y += 45 / 13;
                                    lowpointtarget6.y += 45 / 13;
                                    lowpointtarget6 += facing * .6;
                                    lowpointtarget7.y += 90 / 13;
                                    whichhit = objects.model[i].LineCheckPossible(&lowpoint, &lowpointtarget, &colpoint, &objects.position[i], &objects.yaw[i]);
                                    if (objects.friction[i] > .5)
                                        if (whichhit != -1) {
                                            if (whichhit != -1 && Person::players[k]->animTarget != jumpupanim && Person::players[k]->animTarget != jumpdownanim)
                                                Person::players[k]->collided = 1;
                                            if (checkcollide(lowpoint7, lowpointtarget7) == -1)
                                                if (checkcollide(lowpoint6, lowpointtarget6) == -1)
                                                    if (     objects.model[i].LineCheckPossible(&lowpoint2, &lowpointtarget2,
                                                             &colpoint, &objects.position[i], &objects.yaw[i]) != -1 &&
                                                             objects.model[i].LineCheckPossible(&lowpoint3, &lowpointtarget3,
                                                                     &colpoint, &objects.position[i], &objects.yaw[i]) != -1 &&
                                                             objects.model[i].LineCheckPossible(&lowpoint4, &lowpointtarget4,
                                                                     &colpoint, &objects.position[i], &objects.yaw[i]) != -1 &&
                                                             objects.model[i].LineCheckPossible(&lowpoint5, &lowpointtarget5,
                                                                     &colpoint, &objects.position[i], &objects.yaw[i]) != -1)
                                                        for (int j = 0; j < 45; j++) {
                                                            lowpoint = Person::players[k]->coords;
                                                            lowpoint.y += (float)j / 13;
                                                            lowpointtarget = lowpoint + facing * 1.4;
                                                            if (objects.model[i].LineCheckPossible(&lowpoint, &lowpointtarget,
                                                                                                   &colpoint2, &objects.position[i], &objects.yaw[i]) == -1) {
                                                                if (j <= 6 || j <= 25 && Person::players[k]->animTarget == jumpdownanim)
                                                                    break;
                                                                if (Person::players[k]->animTarget == jumpupanim || Person::players[k]->animTarget == jumpdownanim) {
                                                                    lowpoint = Person::players[k]->coords;
                                                                    lowpoint.y += (float)j / 13;
                                                                    lowpointtarget = lowpoint + facing * 1.3;
                                                                    flatfacing = Person::players[k]->coords;
                                                                    Person::players[k]->coords = colpoint - DoRotation(objects.model[i].facenormals[whichhit], 0, objects.yaw[k], 0) * .01;
                                                                    Person::players[k]->coords.y = lowpointtarget.y - .07;
                                                                    Person::players[k]->currentoffset = (flatfacing - Person::players[k]->coords) / Person::players[k]->scale;

                                                                    if (j > 10 || !Person::players[k]->isRun()) {
                                                                        if (Person::players[k]->animTarget == jumpdownanim || Person::players[k]->animTarget == jumpupanim) {
                                                                            if (k == 0)
                                                                                pause_sound(whooshsound);
                                                                        }
                                                                        emit_sound_at(jumpsound, Person::players[k]->coords, 128.);

                                                                        lowpointtarget = DoRotation(objects.model[i].facenormals[whichhit], 0, objects.yaw[i], 0);
                                                                        Person::players[k]->yaw = -asin(0 - lowpointtarget.x) * 180 / M_PI;
                                                                        if (lowpointtarget.z < 0)
                                                                            Person::players[k]->yaw = 180 - Person::players[k]->yaw;
                                                                        Person::players[k]->targetyaw = Person::players[k]->yaw;
                                                                        Person::players[k]->lowyaw = Person::players[k]->yaw;

                                                                        //Person::players[k]->velocity=lowpointtarget*.03;
                                                                        Person::players[k]->velocity = 0;

                                                                        //climb ledge (?)
                                                                        if (Person::players[k]->animTarget == jumpupanim) {
                                                                            Person::players[k]->animTarget = climbanim;
                                                                            Person::players[k]->jumppower = 0;
                                                                            Person::players[k]->jumpclimb = 1;
                                                                        }
                                                                        Person::players[k]->transspeed = 6;
                                                                        Person::players[k]->target = 0;
                                                                        Person::players[k]->frameTarget = 1;
                                                                        //hang ledge (?)
                                                                        if (j > 25) {
                                                                            Person::players[k]->setAnimation(hanganim);
                                                                            Person::players[k]->jumppower = 0;
                                                                        }
                                                                    }
                                                                    break;
                                                                }
                                                            }
                                                        }
                                        }
                                }
                            }
                        }
                }
            if (Person::players[k]->collide <= 0) {
                //in the air
                if (!Person::players[k]->onterrain &&
                        Person::players[k]->animTarget != jumpupanim &&
                        Person::players[k]->animTarget != jumpdownanim &&
                        Person::players[k]->animTarget != climbanim &&
                        Person::players[k]->animTarget != hanganim &&
                        !Person::players[k]->isWallJump() &&
                        !Person::players[k]->isFlip()) {
                    if (Person::players[k]->animCurrent != climbanim &&
                            Person::players[k]->animCurrent != tempanim &&
                            Person::players[k]->animTarget != backhandspringanim &&
                            (Person::players[k]->animTarget != rollanim ||
                             Person::players[k]->frameTarget < 2 ||
                             Person::players[k]->frameTarget > 6)) {
                        //stagger off ledge (?)
                        if (Person::players[k]->animTarget == staggerbackhighanim || Person::players[k]->animTarget == staggerbackhardanim)
                            Person::players[k]->RagDoll(0);
                        Person::players[k]->setAnimation(jumpdownanim);

                        if (!k)
                            emit_sound_at(whooshsound, Person::players[k]->coords, 128.);
                    }
                    //gravity
                    Person::players[k]->velocity.y += gravity;
                }
            }
        }
        Person::players[k]->realoldcoords = Person::players[k]->coords;
    }
}

void doAttacks()
{
    static int randattack;
    static bool playerrealattackkeydown = 0;

    if (!Input::isKeyDown(attackkey))
        oldattackkey = 0;
    if (oldattackkey)
        Person::players[0]->attackkeydown = 0;
    if (oldattackkey)
        playerrealattackkeydown = 0;
    if (!oldattackkey)
        playerrealattackkeydown = Input::isKeyDown(attackkey);
    if ((Person::players[0]->parriedrecently <= 0 ||
            Person::players[0]->weaponactive == -1) &&
            (!oldattackkey ||
             (realthreat &&
              Person::players[0]->lastattack != swordslashanim &&
              Person::players[0]->lastattack != knifeslashstartanim &&
              Person::players[0]->lastattack != staffhitanim &&
              Person::players[0]->lastattack != staffspinhitanim)))
        Person::players[0]->attackkeydown = Input::isKeyDown(attackkey);
    if (Input::isKeyDown(attackkey) &&
            !oldattackkey &&
            !Person::players[0]->backkeydown) {
        for (unsigned k = 0; k < Person::players.size(); k++) {
            if ((Person::players[k]->animTarget == swordslashanim ||
                    Person::players[k]->animTarget == staffhitanim ||
                    Person::players[k]->animTarget == staffspinhitanim) &&
                    Person::players[0]->animCurrent != dodgebackanim &&
                    !Person::players[k]->skeleton.free)
                Person::players[k]->Reverse();
        }
    }

    if (!hostile || indialogue != -1)
        Person::players[0]->attackkeydown = 0;

    for (unsigned k = 0; k < Person::players.size(); k++) {
        if (indialogue != -1)
            Person::players[k]->attackkeydown = 0;
        if (Person::players[k]->animTarget != rabbitrunninganim && Person::players[k]->animTarget != wolfrunninganim) {
            if (Person::players[k]->aitype != playercontrolled)
                Person::players[k]->victim = Person::players[0];
            //attack key pressed
            if (Person::players[k]->attackkeydown) {
                //dodge backward
                if (Person::players[k]->backkeydown &&
                        Person::players[k]->animTarget != backhandspringanim &&
                        (Person::players[k]->isIdle() ||
                         Person::players[k]->isStop() ||
                         Person::players[k]->isRun() ||
                         Person::players[k]->animTarget == walkanim)) {
                    if (Person::players[k]->jumppower <= 1) {
                        Person::players[k]->jumppower -= 2;
                    } else {
                        for (unsigned i = 0; i < Person::players.size(); i++) {
                            if (i == k)
                                continue;
                            if (Person::players[i]->animTarget == swordslashanim ||
                                    Person::players[i]->animTarget == knifeslashstartanim ||
                                    Person::players[i]->animTarget == staffhitanim ||
                                    Person::players[i]->animTarget == staffspinhitanim)
                                if (distsq(&Person::players[k]->coords, &Person::players[i]->coords) < 6.5 && !Person::players[i]->skeleton.free) {
                                    Person::players[k]->setAnimation(dodgebackanim);
                                    Person::players[k]->targetyaw = roughDirectionTo(Person::players[k]->coords, Person::players[i]->coords);
                                    Person::players[k]->targettilt2 = pitchTo(Person::players[k]->coords, Person::players[i]->coords);
                                }
                        }
                        if (Person::players[k]->animTarget != dodgebackanim) {
                            if (k == 0)
                                numflipped++;
                            Person::players[k]->setAnimation(backhandspringanim);
                            Person::players[k]->targetyaw = -yaw + 180;
                            if (Person::players[k]->leftkeydown)
                                Person::players[k]->targetyaw -= 45;
                            if (Person::players[k]->rightkeydown)
                                Person::players[k]->targetyaw += 45;
                            Person::players[k]->yaw = Person::players[k]->targetyaw;
                            Person::players[k]->jumppower -= 2;
                        }
                    }
                }
                //attack
                if (!animation[Person::players[k]->animTarget].attack &&
                        !Person::players[k]->backkeydown &&
                        (Person::players[k]->isIdle() ||
                         Person::players[k]->isRun() ||
                         Person::players[k]->animTarget == walkanim ||
                         Person::players[k]->animTarget == sneakanim ||
                         Person::players[k]->isCrouch())) {
                    const int attackweapon = Person::players[k]->weaponactive == -1 ? 0 : weapons[Person::players[k]->weaponids[Person::players[k]->weaponactive]].getType();
                    //normal attacks (?)
                    Person::players[k]->hasvictim = 0;
                    if (Person::players.size() > 1)
                        for (unsigned i = 0; i < Person::players.size(); i++) {
                            if (i == k || !(k == 0 || i == 0))
                                continue;
                            if (!Person::players[k]->hasvictim)
                                if (animation[Person::players[k]->animTarget].attack != reversal) {
                                    //choose an attack
                                    const float distance = distsq(&Person::players[k]->coords, &Person::players[i]->coords);
                                    if (distance < 4.5 &&
                                            !Person::players[i]->skeleton.free &&
                                            Person::players[i]->howactive < typedead1 &&
                                            Person::players[i]->animTarget != jumpreversedanim &&
                                            Person::players[i]->animTarget != rabbitkickreversedanim &&
                                            Person::players[i]->animTarget != rabbitkickanim &&
                                            Person::players[k]->animTarget != rabbitkickanim &&
                                            Person::players[i]->animTarget != getupfrombackanim &&
                                            (Person::players[i]->animTarget != staggerbackhighanim &&
                                             (Person::players[i]->animTarget != staggerbackhardanim ||
                                              animation[staggerbackhardanim].label[Person::players[i]->frameTarget] == 6)) &&
                                            Person::players[i]->animTarget != jumpdownanim &&
                                            Person::players[i]->animTarget != jumpupanim &&
                                            Person::players[i]->animTarget != getupfromfrontanim) {
                                        Person::players[k]->victim = Person::players[i];
                                        Person::players[k]->hasvictim = 1;
                                        if (Person::players[k]->aitype == playercontrolled) { //human player
                                            //sweep
                                            if (distance < 2.5 * sq(Person::players[k]->scale * 5) &&
                                                    Person::players[k]->crouchkeydown &&
                                                    animation[Person::players[i]->animTarget].height != lowheight)
                                                Person::players[k]->animTarget = sweepanim;
                                            //winduppunch
                                            else if (distance < 1.5 * sq(Person::players[k]->scale * 5) &&
                                                     animation[Person::players[i]->animTarget].height != lowheight &&
                                                     !Person::players[k]->forwardkeydown &&
                                                     !Person::players[k]->leftkeydown &&
                                                     !Person::players[k]->rightkeydown &&
                                                     !Person::players[k]->crouchkeydown &&
                                                     !attackweapon &&
                                                     !reversaltrain)
                                                Person::players[k]->animTarget = winduppunchanim;
                                            //upunch
                                            else if (distance < 2.5 * sq(Person::players[k]->scale * 5) &&
                                                     animation[Person::players[i]->animTarget].height != lowheight &&
                                                     !Person::players[k]->forwardkeydown &&
                                                     !Person::players[k]->leftkeydown &&
                                                     !Person::players[k]->rightkeydown &&
                                                     !Person::players[k]->crouchkeydown &&
                                                     !attackweapon)
                                                Person::players[k]->animTarget = upunchanim;
                                            //knifefollow
                                            else if (distance < 2.5 * sq(Person::players[k]->scale * 5) &&
                                                     Person::players[i]->staggerdelay > 0 &&
                                                     attackweapon == knife &&
                                                     Person::players[i]->bloodloss > Person::players[i]->damagetolerance / 2)
                                                Person::players[k]->animTarget = knifefollowanim;
                                            //knifeslashstart
                                            else if (distance < 2.5 * sq(Person::players[k]->scale * 5) &&
                                                     animation[Person::players[i]->animTarget].height != lowheight &&
                                                     !Person::players[k]->forwardkeydown &&
                                                     !Person::players[k]->leftkeydown &&
                                                     !Person::players[k]->rightkeydown &&
                                                     !Person::players[k]->crouchkeydown &&
                                                     attackweapon == knife &&
                                                     Person::players[k]->weaponmissdelay <= 0)
                                                Person::players[k]->animTarget = knifeslashstartanim;
                                            //swordslash
                                            else if (distance < 4.5 * sq(Person::players[k]->scale * 5) &&
                                                     animation[Person::players[i]->animTarget].height != lowheight &&
                                                     !Person::players[k]->crouchkeydown &&
                                                     attackweapon == sword &&
                                                     Person::players[k]->weaponmissdelay <= 0)
                                                Person::players[k]->animTarget = swordslashanim;
                                            //staffhit
                                            else if (distance < 4.5 * sq(Person::players[k]->scale * 5) &&
                                                     animation[Person::players[i]->animTarget].height != lowheight &&
                                                     !Person::players[k]->crouchkeydown &&
                                                     attackweapon == staff &&
                                                     Person::players[k]->weaponmissdelay <= 0 &&
                                                     !Person::players[k]->leftkeydown &&
                                                     !Person::players[k]->rightkeydown &&
                                                     !Person::players[k]->forwardkeydown)
                                                Person::players[k]->animTarget = staffhitanim;
                                            //staffspinhit
                                            else if (distance < 4.5 * sq(Person::players[k]->scale * 5) &&
                                                     animation[Person::players[i]->animTarget].height != lowheight &&
                                                     !Person::players[k]->crouchkeydown &&
                                                     attackweapon == staff &&
                                                     Person::players[k]->weaponmissdelay <= 0)
                                                Person::players[k]->animTarget = staffspinhitanim;
                                            //spinkick
                                            else if (distance < 2.5 * sq(Person::players[k]->scale * 5) &&
                                                     animation[Person::players[i]->animTarget].height != lowheight)
                                                Person::players[k]->animTarget = spinkickanim;
                                            //lowkick
                                            else if (distance < 2.5 * sq(Person::players[k]->scale * 5) &&
                                                     animation[Person::players[i]->animTarget].height == lowheight &&
                                                     animation[Person::players[k]->animTarget].attack != normalattack)
                                                Person::players[k]->animTarget = lowkickanim;
                                        } else { //AI player
                                            if (distance < 4.5 * sq(Person::players[k]->scale * 5)) {
                                                randattack = abs(Random() % 5);
                                                if (!attackweapon && distance < 2.5 * sq(Person::players[k]->scale * 5)) {
                                                    //sweep
                                                    if (randattack == 0 && animation[Person::players[i]->animTarget].height != lowheight)
                                                        Person::players[k]->animTarget = sweepanim;
                                                    //upunch
                                                    else if (randattack == 1 && animation[Person::players[i]->animTarget].height != lowheight &&
                                                             !attackweapon)
                                                        Person::players[k]->animTarget = upunchanim;
                                                    //spinkick
                                                    else if (randattack == 2 && animation[Person::players[i]->animTarget].height != lowheight)
                                                        Person::players[k]->animTarget = spinkickanim;
                                                    //lowkick
                                                    else if (animation[Person::players[i]->animTarget].height == lowheight)
                                                        Person::players[k]->animTarget = lowkickanim;
                                                }
                                                if (attackweapon) {
                                                    //sweep
                                                    if ((tutoriallevel != 1 || !attackweapon) &&
                                                            distance < 2.5 * sq(Person::players[k]->scale * 5) &&
                                                            randattack == 0 &&
                                                            animation[Person::players[i]->animTarget].height != lowheight)
                                                        Person::players[k]->animTarget = sweepanim;
                                                    //knifeslashstart
                                                    else if (distance < 2.5 * sq(Person::players[k]->scale * 5) &&
                                                             attackweapon == knife &&
                                                             Person::players[k]->weaponmissdelay <= 0)
                                                        Person::players[k]->animTarget = knifeslashstartanim;
                                                    //swordslash
                                                    else if (!(Person::players[0]->victim == Person::players[i] &&
                                                               Person::players[0]->hasvictim &&
                                                               Person::players[0]->animTarget == swordslashanim) &&
                                                             attackweapon == sword &&
                                                             Person::players[k]->weaponmissdelay <= 0)
                                                        Person::players[k]->animTarget = swordslashanim;
                                                    //staffhit
                                                    else if (!(Person::players[0]->victim == Person::players[i] &&
                                                               Person::players[0]->hasvictim &&
                                                               Person::players[0]->animTarget == swordslashanim) &&
                                                             attackweapon == staff &&
                                                             Person::players[k]->weaponmissdelay <= 0 &&
                                                             randattack < 3)
                                                        Person::players[k]->animTarget = staffhitanim;
                                                    //staffspinhit
                                                    else if (!(Person::players[0]->victim == Person::players[i] &&
                                                               Person::players[0]->hasvictim &&
                                                               Person::players[0]->animTarget == swordslashanim) &&
                                                             attackweapon == staff &&
                                                             Person::players[k]->weaponmissdelay <= 0 &&
                                                             randattack >= 3)
                                                        Person::players[k]->animTarget = staffspinhitanim;
                                                    //spinkick
                                                    else if ((tutoriallevel != 1 || !attackweapon) &&
                                                             distance < 2.5 * sq(Person::players[k]->scale * 5) &&
                                                             randattack == 1 &&
                                                             animation[Person::players[i]->animTarget].height != lowheight)
                                                        Person::players[k]->animTarget = spinkickanim;
                                                    //lowkick
                                                    else if (distance < 2.5 * sq(Person::players[k]->scale * 5) &&
                                                             animation[Person::players[i]->animTarget].height == lowheight &&
                                                             animation[Person::players[k]->animTarget].attack != normalattack)
                                                        Person::players[k]->animTarget = lowkickanim;
                                                }
                                            }
                                        }
                                        //upunch becomes wolfslap
                                        if (Person::players[k]->animTarget == upunchanim && Person::players[k]->creature == wolftype)
                                            Person::players[k]->animTarget = wolfslapanim;
                                    }
                                    //sneak attacks
                                    if ((k == 0) && (tutoriallevel != 1 || tutorialstage == 22) &&
                                            Person::players[i]->howactive < typedead1 &&
                                            distance < 1.5 * sq(Person::players[k]->scale * 5) &&
                                            !Person::players[i]->skeleton.free &&
                                            Person::players[i]->animTarget != getupfrombackanim &&
                                            Person::players[i]->animTarget != getupfromfrontanim &&
                                            (Person::players[i]->surprised > 0 ||
                                             Person::players[i]->aitype == passivetype ||
                                             attackweapon && Person::players[i]->stunned > 0) &&
                                            normaldotproduct(Person::players[i]->facing, Person::players[i]->coords - Person::players[k]->coords) > 0) {
                                        //sneakattack
                                        if (!attackweapon) {
                                            Person::players[k]->animCurrent = sneakattackanim;
                                            Person::players[k]->animTarget = sneakattackanim;
                                            Person::players[i]->animCurrent = sneakattackedanim;
                                            Person::players[i]->animTarget = sneakattackedanim;
                                            Person::players[k]->oldcoords = Person::players[k]->coords;
                                            Person::players[k]->coords = Person::players[i]->coords;
                                        }
                                        //knifesneakattack
                                        if (attackweapon == knife) {
                                            Person::players[k]->animCurrent = knifesneakattackanim;
                                            Person::players[k]->animTarget = knifesneakattackanim;
                                            Person::players[i]->animCurrent = knifesneakattackedanim;
                                            Person::players[i]->animTarget = knifesneakattackedanim;
                                            Person::players[i]->oldcoords = Person::players[i]->coords;
                                            Person::players[i]->coords = Person::players[k]->coords;
                                        }
                                        //swordsneakattack
                                        if (attackweapon == sword) {
                                            Person::players[k]->animCurrent = swordsneakattackanim;
                                            Person::players[k]->animTarget = swordsneakattackanim;
                                            Person::players[i]->animCurrent = swordsneakattackedanim;
                                            Person::players[i]->animTarget = swordsneakattackedanim;
                                            Person::players[i]->oldcoords = Person::players[i]->coords;
                                            Person::players[i]->coords = Person::players[k]->coords;
                                        }
                                        if (attackweapon != staff) {
                                            Person::players[k]->victim = Person::players[i];
                                            Person::players[k]->hasvictim = 1;
                                            Person::players[i]->targettilt2 = 0;
                                            Person::players[i]->frameTarget = 1;
                                            Person::players[i]->frameCurrent = 0;
                                            Person::players[i]->target = 0;
                                            Person::players[i]->velocity = 0;
                                            Person::players[k]->targettilt2 = Person::players[i]->targettilt2;
                                            Person::players[k]->frameCurrent = Person::players[i]->frameCurrent;
                                            Person::players[k]->frameTarget = Person::players[i]->frameTarget;
                                            Person::players[k]->target = Person::players[i]->target;
                                            Person::players[k]->velocity = 0;
                                            Person::players[k]->targetyaw = Person::players[i]->yaw;
                                            Person::players[k]->yaw = Person::players[i]->yaw;
                                            Person::players[i]->targetyaw = Person::players[i]->yaw;
                                        }
                                    }
                                    if (animation[Person::players[k]->animTarget].attack == normalattack &&
                                            Person::players[k]->victim == Person::players[i] &&
                                            (!Person::players[i]->skeleton.free)) {
                                        oldattackkey = 1;
                                        Person::players[k]->frameTarget = 0;
                                        Person::players[k]->target = 0;

                                        Person::players[k]->targetyaw = roughDirectionTo(Person::players[k]->coords, Person::players[i]->coords);
                                        Person::players[k]->targettilt2 = pitchTo(Person::players[k]->coords, Person::players[i]->coords);
                                        Person::players[k]->lastattack3 = Person::players[k]->lastattack2;
                                        Person::players[k]->lastattack2 = Person::players[k]->lastattack;
                                        Person::players[k]->lastattack = Person::players[k]->animTarget;
                                    }
                                    if (Person::players[k]->animTarget == knifefollowanim &&
                                            Person::players[k]->victim == Person::players[i]) {
                                        oldattackkey = 1;
                                        Person::players[k]->targetyaw = roughDirectionTo(Person::players[k]->coords, Person::players[i]->coords);
                                        Person::players[k]->targettilt2 = pitchTo(Person::players[k]->coords, Person::players[i]->coords);
                                        Person::players[k]->victim = Person::players[i];
                                        Person::players[k]->hasvictim = 1;
                                        Person::players[i]->animTarget = knifefollowedanim;
                                        Person::players[i]->animCurrent = knifefollowedanim;
                                        Person::players[i]->targettilt2 = 0;
                                        Person::players[i]->targettilt2 = Person::players[k]->targettilt2;
                                        Person::players[i]->frameTarget = 1;
                                        Person::players[i]->frameCurrent = 0;
                                        Person::players[i]->target = 0;
                                        Person::players[i]->velocity = 0;
                                        Person::players[k]->animCurrent = knifefollowanim;
                                        Person::players[k]->animTarget = knifefollowanim;
                                        Person::players[k]->targettilt2 = Person::players[i]->targettilt2;
                                        Person::players[k]->frameCurrent = Person::players[i]->frameCurrent;
                                        Person::players[k]->frameTarget = Person::players[i]->frameTarget;
                                        Person::players[k]->target = Person::players[i]->target;
                                        Person::players[k]->velocity = 0;
                                        Person::players[k]->oldcoords = Person::players[k]->coords;
                                        Person::players[i]->coords = Person::players[k]->coords;
                                        Person::players[i]->targetyaw = Person::players[k]->targetyaw;
                                        Person::players[i]->yaw = Person::players[k]->targetyaw;
                                        Person::players[k]->yaw = Person::players[k]->targetyaw;
                                        Person::players[i]->yaw = Person::players[k]->targetyaw;
                                    }
                                }
                        }
                    const bool hasstaff = attackweapon == staff;
                    if (k == 0 && Person::players.size() > 1)
                        for (unsigned i = 0; i < Person::players.size(); i++) {
                            if (i == k)
                                continue;
                            if ((playerrealattackkeydown || Person::players[i]->dead || !hasstaff) &&
                                    animation[Person::players[k]->animTarget].attack == neutral) {
                                const float distance = distsq(&Person::players[k]->coords, &Person::players[i]->coords);
                                if (!Person::players[i]->dead || !realthreat || (!attackweapon && Person::players[k]->crouchkeydown))
                                    if (Person::players[i]->skeleton.free)
                                        if (distance < 3.5 * sq(Person::players[k]->scale * 5) &&
                                                (Person::players[i]->dead ||
                                                 Person::players[i]->skeleton.longdead > 1000 ||
                                                 Person::players[k]->isRun() ||
                                                 hasstaff ||
                                                 (attackweapon &&
                                                  (Person::players[i]->skeleton.longdead > 2000 ||
                                                   Person::players[i]->damage > Person::players[i]->damagetolerance / 8 ||
                                                   Person::players[i]->bloodloss > Person::players[i]->damagetolerance / 2) &&
                                                  distance < 1.5 * sq(Person::players[k]->scale * 5)))) {
                                            Person::players[k]->victim = Person::players[i];
                                            Person::players[k]->hasvictim = 1;
                                            if (attackweapon && tutoriallevel != 1) {
                                                //crouchstab
                                                if (Person::players[k]->crouchkeydown && attackweapon == knife && distance < 1.5 * sq(Person::players[k]->scale * 5))
                                                    Person::players[k]->animTarget = crouchstabanim;
                                                //swordgroundstab
                                                if (Person::players[k]->crouchkeydown && distance < 1.5 * sq(Person::players[k]->scale * 5) && attackweapon == sword)
                                                    Person::players[k]->animTarget = swordgroundstabanim;
                                                //staffgroundsmash
                                                if (distance < 3.5 * sq(Person::players[k]->scale * 5) && attackweapon == staff)
                                                    Person::players[k]->animTarget = staffgroundsmashanim;
                                            }
                                            if (distance < 2.5 &&
                                                    Person::players[k]->crouchkeydown &&
                                                    Person::players[k]->animTarget != crouchstabanim &&
                                                    !attackweapon &&
                                                    Person::players[i]->dead &&
                                                    Person::players[i]->skeleton.free &&
                                                    Person::players[i]->skeleton.longdead > 1000) {
                                                Person::players[k]->animTarget = killanim;
                                                //TODO: refactor this out, what does it do?
                                                for (int j = 0; j < terrain.numdecals; j++) {
                                                    if ((terrain.decaltype[j] == blooddecal || terrain.decaltype[j] == blooddecalslow) &&
                                                            terrain.decalalivetime[j] < 2)
                                                        terrain.DeleteDecal(j);
                                                }
                                                for (int l = 0; l < objects.numobjects; l++) {
                                                    if (objects.model[l].type == decalstype)
                                                        for (int j = 0; j < objects.model[l].numdecals; j++) {
                                                            if ((objects.model[l].decaltype[j] == blooddecal ||
                                                                    objects.model[l].decaltype[j] == blooddecalslow) &&
                                                                    objects.model[l].decalalivetime[j] < 2)
                                                                objects.model[l].DeleteDecal(j);
                                                        }
                                                }
                                            }
                                            if (!Person::players[i]->dead || musictype != 2)
                                                if (distance < 3.5 &&
                                                        (Person::players[k]->isRun() || Person::players[k]->isIdle() && Person::players[k]->attackkeydown) &&
                                                        Person::players[k]->staggerdelay <= 0 &&
                                                        (Person::players[i]->dead ||
                                                         Person::players[i]->skeleton.longdead < 300 &&
                                                         Person::players[k]->lastattack != spinkickanim &&
                                                         Person::players[i]->skeleton.free) &&
                                                        (!Person::players[i]->dead || musictype != stream_fighttheme)) {
                                                    Person::players[k]->animTarget = dropkickanim;
                                                    for (int j = 0; j < terrain.numdecals; j++) {
                                                        if ((terrain.decaltype[j] == blooddecal || terrain.decaltype[j] == blooddecalslow) &&
                                                                terrain.decalalivetime[j] < 2) {
                                                            terrain.DeleteDecal(j);
                                                        }
                                                    }
                                                    for (int l = 0; l < objects.numobjects; l++) {
                                                        if (objects.model[l].type == decalstype)
                                                            for (int j = 0; j < objects.model[l].numdecals; j++) {
                                                                if ((objects.model[l].decaltype[j] == blooddecal ||
                                                                        objects.model[l].decaltype[j] == blooddecalslow) &&
                                                                        objects.model[l].decalalivetime[j] < 2) {
                                                                    objects.model[l].DeleteDecal(j);
                                                                }
                                                            }
                                                    }
                                                }
                                        }
                                if (animation[Person::players[k]->animTarget].attack == normalattack &&
                                        Person::players[k]->victim == Person::players[i] &&
                                        (!Person::players[i]->skeleton.free ||
                                         Person::players[k]->animTarget == killanim ||
                                         Person::players[k]->animTarget == crouchstabanim ||
                                         Person::players[k]->animTarget == swordgroundstabanim ||
                                         Person::players[k]->animTarget == staffgroundsmashanim ||
                                         Person::players[k]->animTarget == dropkickanim)) {
                                    oldattackkey = 1;
                                    Person::players[k]->frameTarget = 0;
                                    Person::players[k]->target = 0;

                                    XYZ targetpoint = Person::players[i]->coords;
                                    if (Person::players[k]->animTarget == crouchstabanim ||
                                            Person::players[k]->animTarget == swordgroundstabanim ||
                                            Person::players[k]->animTarget == staffgroundsmashanim) {
                                        targetpoint += (Person::players[i]->jointPos(abdomen) +
                                                        Person::players[i]->jointPos(neck)) / 2 *
                                                       Person::players[i]->scale;
                                    }
                                    Person::players[k]->targetyaw = roughDirectionTo(Person::players[k]->coords, targetpoint);
                                    Person::players[k]->targettilt2 = pitchTo(Person::players[k]->coords, targetpoint);

                                    if (Person::players[k]->animTarget == crouchstabanim || Person::players[k]->animTarget == swordgroundstabanim) {
                                        Person::players[k]->targetyaw += (float)(abs(Random() % 100) - 50) / 4;
                                    }

                                    if (Person::players[k]->animTarget == staffgroundsmashanim)
                                        Person::players[k]->targettilt2 += 10;

                                    Person::players[k]->lastattack3 = Person::players[k]->lastattack2;
                                    Person::players[k]->lastattack2 = Person::players[k]->lastattack;
                                    Person::players[k]->lastattack = Person::players[k]->animTarget;

                                    if (Person::players[k]->animTarget == swordgroundstabanim) {
                                        Person::players[k]->targetyaw += 30;
                                    }
                                }
                            }
                        }
                    if (!Person::players[k]->hasvictim) {
                        //find victim
                        for (unsigned i = 0; i < Person::players.size(); i++) {
                            if (i == k || !(i == 0 || k == 0))
                                continue;
                            if (!Person::players[i]->skeleton.free) {
                                if (Person::players[k]->hasvictim) {
                                    if (distsq(&Person::players[k]->coords, &Person::players[i]->coords) <
                                            distsq(&Person::players[k]->coords, &Person::players[k]->victim->coords))
                                        Person::players[k]->victim = Person::players[i];
                                } else {
                                    Person::players[k]->victim = Person::players[i];
                                    Person::players[k]->hasvictim = 1;
                                }
                            }
                        }
                    }
                    if (Person::players[k]->aitype == playercontrolled)
                        //rabbit kick
                        if (Person::players[k]->attackkeydown &&
                                Person::players[k]->isRun() &&
                                Person::players[k]->wasRun() &&
                                ((Person::players[k]->hasvictim &&
                                  distsq(&Person::players[k]->coords, &Person::players[k]->victim->coords) < 12 * sq(Person::players[k]->scale * 5) &&
                                  distsq(&Person::players[k]->coords, &Person::players[k]->victim->coords) > 7 * sq(Person::players[k]->scale * 5) &&
                                  !Person::players[k]->victim->skeleton.free &&
                                  Person::players[k]->victim->animTarget != getupfrombackanim &&
                                  Person::players[k]->victim->animTarget != getupfromfrontanim &&
                                  animation[Person::players[k]->victim->animTarget].height != lowheight &&
                                  Person::players[k]->aitype != playercontrolled && //wat???
                                  normaldotproduct(Person::players[k]->facing, Person::players[k]->victim->coords - Person::players[k]->coords) > 0 &&
                                  Person::players[k]->rabbitkickenabled) ||
                                 Person::players[k]->jumpkeydown)) {
                            oldattackkey = 1;
                            Person::players[k]->setAnimation(rabbitkickanim);
                        }
                    //update counts
                    if (animation[Person::players[k]->animTarget].attack && k == 0) {
                        numattacks++;
                        switch (attackweapon) {
                        case 0:
                            numunarmedattack++;
                            break;
                        case knife:
                            numknifeattack++;
                            break;
                        case sword:
                            numswordattack++;
                            break;
                        case staff:
                            numstaffattack++;
                            break;
                        }
                    }
                }
            }
        }
    }
}

void doPlayerCollisions()
{
    static XYZ rotatetarget;
    static float collisionradius;
    if (Person::players.size() > 1)
        for (unsigned k = 0; k < Person::players.size(); k++)
            for (unsigned i = k + 1; i < Person::players.size(); i++) {
                //neither player is part of a reversal
                if ((animation[Person::players[i]->animTarget].attack != reversed &&
                        animation[Person::players[i]->animTarget].attack != reversal &&
                        animation[Person::players[k]->animTarget].attack != reversed &&
                        animation[Person::players[k]->animTarget].attack != reversal) || (i != 0 && k != 0))
                    if ((animation[Person::players[i]->animCurrent].attack != reversed &&
                            animation[Person::players[i]->animCurrent].attack != reversal &&
                            animation[Person::players[k]->animCurrent].attack != reversed &&
                            animation[Person::players[k]->animCurrent].attack != reversal) || (i != 0 && k != 0))
                        //neither is sleeping
                        if (Person::players[i]->howactive <= typesleeping && Person::players[k]->howactive <= typesleeping)
                            if (Person::players[i]->howactive != typesittingwall && Person::players[k]->howactive != typesittingwall)
                                //in same patch, neither is climbing
                                if (Person::players[i]->whichpatchx == Person::players[k]->whichpatchx &&
                                        Person::players[i]->whichpatchz == Person::players[k]->whichpatchz &&
                                        Person::players[k]->skeleton.oldfree == Person::players[k]->skeleton.free &&
                                        Person::players[i]->skeleton.oldfree == Person::players[i]->skeleton.free &&
                                        Person::players[i]->animTarget != climbanim &&
                                        Person::players[i]->animTarget != hanganim &&
                                        Person::players[k]->animTarget != climbanim &&
                                        Person::players[k]->animTarget != hanganim)
                                    //players are close (bounding box test)
                                    if (Person::players[i]->coords.y > Person::players[k]->coords.y - 3)
                                        if (Person::players[i]->coords.y < Person::players[k]->coords.y + 3)
                                            if (Person::players[i]->coords.x > Person::players[k]->coords.x - 3)
                                                if (Person::players[i]->coords.x < Person::players[k]->coords.x + 3)
                                                    if (Person::players[i]->coords.z > Person::players[k]->coords.z - 3)
                                                        if (Person::players[i]->coords.z < Person::players[k]->coords.z + 3) {
                                                            //spread fire from player to player
                                                            if (distsq(&Person::players[i]->coords, &Person::players[k]->coords)
                                                                    < 3 * sq((Person::players[i]->scale + Person::players[k]->scale) * 2.5)) {
                                                                if (Person::players[i]->onfire || Person::players[k]->onfire) {
                                                                    if (!Person::players[i]->onfire)
                                                                        Person::players[i]->CatchFire();
                                                                    if (!Person::players[k]->onfire)
                                                                        Person::players[k]->CatchFire();
                                                                }
                                                            }

                                                            XYZ tempcoords1 = Person::players[i]->coords;
                                                            XYZ tempcoords2 = Person::players[k]->coords;
                                                            if (!Person::players[i]->skeleton.oldfree)
                                                                tempcoords1.y += Person::players[i]->jointPos(abdomen).y * Person::players[i]->scale;
                                                            if (!Person::players[k]->skeleton.oldfree)
                                                                tempcoords2.y += Person::players[k]->jointPos(abdomen).y * Person::players[k]->scale;
                                                            collisionradius = 1.2 * sq((Person::players[i]->scale + Person::players[k]->scale) * 2.5);
                                                            if (Person::players[0]->hasvictim)
                                                                if (Person::players[0]->animTarget == rabbitkickanim && (k == 0 || i == 0) && !Person::players[0]->victim->skeleton.free)
                                                                    collisionradius = 3;
                                                            if ((!Person::players[i]->skeleton.oldfree || !Person::players[k]->skeleton.oldfree) &&
                                                                    (distsq(&tempcoords1, &tempcoords2) < collisionradius ||
                                                                     distsq(&Person::players[i]->coords, &Person::players[k]->coords) < collisionradius)) {
                                                                //jump down on a dead body
                                                                if (k == 0 || i == 0) {
                                                                    int l = i ? i : k;
                                                                    if (Person::players[0]->animTarget == jumpdownanim &&
                                                                            !Person::players[0]->skeleton.oldfree &&
                                                                            !Person::players[0]->skeleton.free &&
                                                                            Person::players[l]->skeleton.oldfree &&
                                                                            Person::players[l]->skeleton.free &&
                                                                            Person::players[l]->dead &&
                                                                            Person::players[0]->lastcollide <= 0 &&
                                                                            fabs(Person::players[l]->coords.y - Person::players[0]->coords.y) < .2 &&
                                                                            distsq(&Person::players[0]->coords, &Person::players[l]->coords) < .7 * sq((Person::players[l]->scale + Person::players[0]->scale) * 2.5)) {
                                                                        Person::players[0]->coords.y = Person::players[l]->coords.y;
                                                                        Person::players[l]->velocity = Person::players[0]->velocity;
                                                                        Person::players[l]->skeleton.free = 0;
                                                                        Person::players[l]->yaw = 0;
                                                                        Person::players[l]->RagDoll(0);
                                                                        Person::players[l]->DoDamage(20);
                                                                        camerashake += .3;
                                                                        Person::players[l]->skeleton.longdead = 0;
                                                                        Person::players[0]->lastcollide = 1;
                                                                    }
                                                                }

                                                                if (     (Person::players[i]->skeleton.oldfree == 1 && findLengthfast(&Person::players[i]->velocity) > 1) ||
                                                                         (Person::players[k]->skeleton.oldfree == 1 && findLengthfast(&Person::players[k]->velocity) > 1) ||
                                                                         (Person::players[i]->skeleton.oldfree == 0 && Person::players[k]->skeleton.oldfree == 0)) {
                                                                    rotatetarget = Person::players[k]->velocity - Person::players[i]->velocity;
                                                                    if ((Person::players[i]->animTarget != getupfrombackanim && Person::players[i]->animTarget != getupfromfrontanim ||
                                                                            Person::players[i]->skeleton.free) &&
                                                                            (Person::players[k]->animTarget != getupfrombackanim && Person::players[k]->animTarget != getupfromfrontanim ||
                                                                             Person::players[k]->skeleton.free))
                                                                        if ((((k != 0 && findLengthfast(&rotatetarget) > 150 ||
                                                                                k == 0 && findLengthfast(&rotatetarget) > 50 && Person::players[0]->rabbitkickragdoll) &&
                                                                                normaldotproduct(rotatetarget, Person::players[k]->coords - Person::players[i]->coords) > 0) &&
                                                                                (k == 0 ||
                                                                                 k != 0 && Person::players[i]->skeleton.oldfree == 1 && animation[Person::players[k]->animCurrent].attack == neutral ||
                                                                                 /*i!=0&&*/Person::players[k]->skeleton.oldfree == 1 && animation[Person::players[i]->animCurrent].attack == neutral)) ||
                                                                                (Person::players[i]->animTarget == jumpupanim || Person::players[i]->animTarget == jumpdownanim || Person::players[i]->isFlip()) &&
                                                                                (Person::players[k]->animTarget == jumpupanim || Person::players[k]->animTarget == jumpdownanim || Person::players[k]->isFlip()) &&
                                                                                k == 0 && !Person::players[i]->skeleton.oldfree && !Person::players[k]->skeleton.oldfree) {
                                                                            //If hit by body
                                                                            if (     (i != 0 || Person::players[i]->skeleton.free) &&
                                                                                     (k != 0 || Person::players[k]->skeleton.free) ||
                                                                                     (animation[Person::players[i]->animTarget].height == highheight &&
                                                                                      animation[Person::players[k]->animTarget].height == highheight)) {
                                                                                if (tutoriallevel != 1) {
                                                                                    emit_sound_at(heavyimpactsound, Person::players[i]->coords);
                                                                                }

                                                                                Person::players[i]->RagDoll(0);
                                                                                if (Person::players[i]->damage > Person::players[i]->damagetolerance - findLengthfast(&rotatetarget) / 4 && !Person::players[i]->dead) {
                                                                                    award_bonus(0, aimbonus);
                                                                                }
                                                                                Person::players[i]->DoDamage(findLengthfast(&rotatetarget) / 4);
                                                                                Person::players[k]->RagDoll(0);
                                                                                if (Person::players[k]->damage > Person::players[k]->damagetolerance - findLengthfast(&rotatetarget) / 4 && !Person::players[k]->dead) {
                                                                                    award_bonus(0, aimbonus); // Huh, again?
                                                                                }
                                                                                Person::players[k]->DoDamage(findLengthfast(&rotatetarget) / 4);

                                                                                for (int j = 0; j < Person::players[i]->skeleton.num_joints; j++) {
                                                                                    Person::players[i]->skeleton.joints[j].velocity = Person::players[i]->skeleton.joints[j].velocity / 5 + Person::players[k]->velocity;
                                                                                }
                                                                                for (int j = 0; j < Person::players[k]->skeleton.num_joints; j++) {
                                                                                    Person::players[k]->skeleton.joints[j].velocity = Person::players[k]->skeleton.joints[j].velocity / 5 + Person::players[i]->velocity;
                                                                                }

                                                                            }
                                                                        }
                                                                    if (     (animation[Person::players[i]->animTarget].attack == neutral ||
                                                                              animation[Person::players[i]->animTarget].attack == normalattack) &&
                                                                             (animation[Person::players[k]->animTarget].attack == neutral ||
                                                                              animation[Person::players[k]->animTarget].attack == normalattack)) {
                                                                        //If bumped
                                                                        if (Person::players[i]->skeleton.oldfree == 0 && Person::players[k]->skeleton.oldfree == 0) {
                                                                            if (distsq(&Person::players[k]->coords, &Person::players[i]->coords) < .5 * sq((Person::players[i]->scale + Person::players[k]->scale) * 2.5)) {
                                                                                rotatetarget = Person::players[k]->coords - Person::players[i]->coords;
                                                                                Normalise(&rotatetarget);
                                                                                Person::players[k]->coords = (Person::players[k]->coords + Person::players[i]->coords) / 2;
                                                                                Person::players[i]->coords = Person::players[k]->coords - rotatetarget * fast_sqrt(.6) / 2
                                                                                                   * sq((Person::players[i]->scale + Person::players[k]->scale) * 2.5);
                                                                                Person::players[k]->coords += rotatetarget * fast_sqrt(.6) / 2 * sq((Person::players[i]->scale + Person::players[k]->scale) * 2.5);
                                                                                if (Person::players[k]->howactive == typeactive || hostile)
                                                                                    if (Person::players[k]->isIdle()) {
                                                                                        if (Person::players[k]->howactive < typesleeping)
                                                                                            Person::players[k]->setAnimation(Person::players[k]->getStop());
                                                                                        else if (Person::players[k]->howactive == typesleeping)
                                                                                            Person::players[k]->setAnimation(getupfromfrontanim);
                                                                                        if (!editorenabled)
                                                                                            Person::players[k]->howactive = typeactive;
                                                                                    }
                                                                                if (Person::players[i]->howactive == typeactive || hostile)
                                                                                    if (Person::players[i]->isIdle()) {
                                                                                        if (Person::players[i]->howactive < typesleeping)
                                                                                            Person::players[i]->setAnimation(Person::players[k]->getStop());
                                                                                        else
                                                                                            Person::players[i]->setAnimation(getupfromfrontanim);
                                                                                        if (!editorenabled)
                                                                                            Person::players[i]->howactive = typeactive;
                                                                                    }
                                                                            }
                                                                            //jump down on player
                                                                            if (hostile) {
                                                                                if (k == 0 && i != 0 && Person::players[k]->animTarget == jumpdownanim &&
                                                                                        !Person::players[i]->isCrouch() &&
                                                                                        Person::players[i]->animTarget != rollanim &&
                                                                                        !Person::players[k]->skeleton.oldfree && !
                                                                                        Person::players[k]->skeleton.free &&
                                                                                        Person::players[k]->lastcollide <= 0 &&
                                                                                        Person::players[k]->velocity.y < -10) {
                                                                                    Person::players[i]->velocity = Person::players[k]->velocity;
                                                                                    Person::players[k]->velocity = Person::players[k]->velocity * -.5;
                                                                                    Person::players[k]->velocity.y = Person::players[i]->velocity.y;
                                                                                    Person::players[i]->DoDamage(20);
                                                                                    Person::players[i]->RagDoll(0);
                                                                                    Person::players[k]->lastcollide = 1;
                                                                                    award_bonus(k, AboveBonus);
                                                                                }
                                                                                if (i == 0 && k != 0 && Person::players[i]->animTarget == jumpdownanim &&
                                                                                        !Person::players[k]->isCrouch() &&
                                                                                        Person::players[k]->animTarget != rollanim &&
                                                                                        !Person::players[i]->skeleton.oldfree &&
                                                                                        !Person::players[i]->skeleton.free &&
                                                                                        Person::players[i]->lastcollide <= 0 &&
                                                                                        Person::players[i]->velocity.y < -10) {
                                                                                    Person::players[k]->velocity = Person::players[i]->velocity;
                                                                                    Person::players[i]->velocity = Person::players[i]->velocity * -.3;
                                                                                    Person::players[i]->velocity.y = Person::players[k]->velocity.y;
                                                                                    Person::players[k]->DoDamage(20);
                                                                                    Person::players[k]->RagDoll(0);
                                                                                    Person::players[i]->lastcollide = 1;
                                                                                    award_bonus(i, AboveBonus);
                                                                                }
                                                                            }
                                                                        }
                                                                    }
                                                                }
                                                                Person::players[i]->CheckKick();
                                                                Person::players[k]->CheckKick();
                                                            }
                                                        }
            }
}

void doAI(unsigned i)
{
    static bool connected;
    if (Person::players[i]->aitype != playercontrolled && indialogue == -1) {
        Person::players[i]->jumpclimb = 0;
        //disable movement in editor
        if (editorenabled)
            Person::players[i]->stunned = 1;

        Person::players[i]->pause = 0;
        if (distsqflat(&Person::players[0]->coords, &Person::players[i]->coords) < 30 &&
                Person::players[0]->coords.y > Person::players[i]->coords.y + 2 &&
                !Person::players[0]->onterrain)
            Person::players[i]->pause = 1;

        //pathfinding
        if (Person::players[i]->aitype == pathfindtype) {
            if (Person::players[i]->finalpathfindpoint == -1) {
                float closestdistance;
                float tempdist;
                int closest;
                XYZ colpoint;
                closest = -1;
                closestdistance = -1;
                for (int j = 0; j < numpathpoints; j++)
                    if (closest == -1 || distsq(&Person::players[i]->finalfinaltarget, &pathpoint[j]) < closestdistance) {
                        closestdistance = distsq(&Person::players[i]->finalfinaltarget, &pathpoint[j]);
                        closest = j;
                        Person::players[i]->finaltarget = pathpoint[j];
                    }
                Person::players[i]->finalpathfindpoint = closest;
                for (int j = 0; j < numpathpoints; j++)
                    for (int k = 0; k < numpathpointconnect[j]; k++) {
                        DistancePointLine(&Person::players[i]->finalfinaltarget, &pathpoint[j], &pathpoint[pathpointconnect[j][k]], &tempdist, &colpoint );
                        if (sq(tempdist) < closestdistance)
                            if (findDistance(&colpoint, &pathpoint[j]) + findDistance(&colpoint, &pathpoint[pathpointconnect[j][k]]) <
                                    findDistance(&pathpoint[j], &pathpoint[pathpointconnect[j][k]]) + .1) {
                                closestdistance = sq(tempdist);
                                closest = j;
                                Person::players[i]->finaltarget = colpoint;
                            }
                    }
                Person::players[i]->finalpathfindpoint = closest;

            }
            if (Person::players[i]->targetpathfindpoint == -1) {
                float closestdistance;
                float tempdist;
                int closest;
                XYZ colpoint;
                closest = -1;
                closestdistance = -1;
                if (Person::players[i]->lastpathfindpoint == -1) {
                    for (int j = 0; j < numpathpoints; j++) {
                        if (j != Person::players[i]->lastpathfindpoint)
                            if (closest == -1 || (distsq(&Person::players[i]->coords, &pathpoint[j]) < closestdistance)) {
                                closestdistance = distsq(&Person::players[i]->coords, &pathpoint[j]);
                                closest = j;
                            }
                    }
                    Person::players[i]->targetpathfindpoint = closest;
                    for (int j = 0; j < numpathpoints; j++)
                        if (j != Person::players[i]->lastpathfindpoint)
                            for (int k = 0; k < numpathpointconnect[j]; k++) {
                                DistancePointLine(&Person::players[i]->coords, &pathpoint[j], &pathpoint[pathpointconnect[j][k]], &tempdist, &colpoint );
                                if (sq(tempdist) < closestdistance) {
                                    if (findDistance(&colpoint, &pathpoint[j]) + findDistance(&colpoint, &pathpoint[pathpointconnect[j][k]]) <
                                            findDistance(&pathpoint[j], &pathpoint[pathpointconnect[j][k]]) + .1) {
                                        closestdistance = sq(tempdist);
                                        closest = j;
                                    }
                                }
                            }
                    Person::players[i]->targetpathfindpoint = closest;
                } else {
                    for (int j = 0; j < numpathpoints; j++)
                        if (j != Person::players[i]->lastpathfindpoint &&
                                j != Person::players[i]->lastpathfindpoint2 &&
                                j != Person::players[i]->lastpathfindpoint3 &&
                                j != Person::players[i]->lastpathfindpoint4) {
                            connected = 0;
                            if (numpathpointconnect[j])
                                for (int k = 0; k < numpathpointconnect[j]; k++)
                                    if (pathpointconnect[j][k] == Person::players[i]->lastpathfindpoint)
                                        connected = 1;
                            if (!connected)
                                if (numpathpointconnect[Person::players[i]->lastpathfindpoint])
                                    for (int k = 0; k < numpathpointconnect[Person::players[i]->lastpathfindpoint]; k++)
                                        if (pathpointconnect[Person::players[i]->lastpathfindpoint][k] == j)
                                            connected = 1;
                            if (connected) {
                                tempdist = findPathDist(j, Person::players[i]->finalpathfindpoint);
                                if (closest == -1 || tempdist < closestdistance) {
                                    closestdistance = tempdist;
                                    closest = j;
                                }
                            }
                        }
                    Person::players[i]->targetpathfindpoint = closest;
                }
            }
            Person::players[i]->losupdatedelay -= multiplier;

            Person::players[i]->targetyaw = roughDirectionTo(Person::players[i]->coords, pathpoint[Person::players[i]->targetpathfindpoint]);
            Person::players[i]->lookyaw = Person::players[i]->targetyaw;

            //reached target point
            if (distsqflat(&Person::players[i]->coords, &pathpoint[Person::players[i]->targetpathfindpoint]) < .6) {
                Person::players[i]->lastpathfindpoint4 = Person::players[i]->lastpathfindpoint3;
                Person::players[i]->lastpathfindpoint3 = Person::players[i]->lastpathfindpoint2;
                Person::players[i]->lastpathfindpoint2 = Person::players[i]->lastpathfindpoint;
                Person::players[i]->lastpathfindpoint = Person::players[i]->targetpathfindpoint;
                if (Person::players[i]->lastpathfindpoint2 == -1)
                    Person::players[i]->lastpathfindpoint2 = Person::players[i]->lastpathfindpoint;
                if (Person::players[i]->lastpathfindpoint3 == -1)
                    Person::players[i]->lastpathfindpoint3 = Person::players[i]->lastpathfindpoint2;
                if (Person::players[i]->lastpathfindpoint4 == -1)
                    Person::players[i]->lastpathfindpoint4 = Person::players[i]->lastpathfindpoint3;
                Person::players[i]->targetpathfindpoint = -1;
            }
            if (     distsqflat(&Person::players[i]->coords, &Person::players[i]->finalfinaltarget) <
                     distsqflat(&Person::players[i]->coords, &Person::players[i]->finaltarget) ||
                     distsqflat(&Person::players[i]->coords, &Person::players[i]->finaltarget) < .6 * sq(Person::players[i]->scale * 5) ||
                     Person::players[i]->lastpathfindpoint == Person::players[i]->finalpathfindpoint) {
                Person::players[i]->aitype = passivetype;
            }

            Person::players[i]->forwardkeydown = 1;
            Person::players[i]->leftkeydown = 0;
            Person::players[i]->backkeydown = 0;
            Person::players[i]->rightkeydown = 0;
            Person::players[i]->crouchkeydown = 0;
            Person::players[i]->attackkeydown = 0;
            Person::players[i]->throwkeydown = 0;

            if (Person::players[i]->avoidcollided > .8 && !Person::players[i]->jumpkeydown && Person::players[i]->collided < .8)
                Person::players[i]->targetyaw += 90 * (Person::players[i]->whichdirection * 2 - 1);

            if (Person::players[i]->collided < 1 || Person::players[i]->animTarget != jumpupanim)
                Person::players[i]->jumpkeydown = 0;
            if ((Person::players[i]->collided > .8 && Person::players[i]->jumppower >= 5))
                Person::players[i]->jumpkeydown = 1;

            if ((tutoriallevel != 1 || cananger) &&
                    hostile &&
                    !Person::players[0]->dead &&
                    distsq(&Person::players[i]->coords, &Person::players[0]->coords) < 400 &&
                    Person::players[i]->occluded < 25) {
                if (distsq(&Person::players[i]->coords, &Person::players[0]->coords) < 12 &&
                        animation[Person::players[0]->animTarget].height != lowheight &&
                        !editorenabled &&
                        (Person::players[0]->coords.y < Person::players[i]->coords.y + 5 || Person::players[0]->onterrain))
                    Person::players[i]->aitype = attacktypecutoff;
                if (distsq(&Person::players[i]->coords, &Person::players[0]->coords) < 30 &&
                        animation[Person::players[0]->animTarget].height == highheight &&
                        !editorenabled)
                    Person::players[i]->aitype = attacktypecutoff;

                if (Person::players[i]->losupdatedelay < 0 && !editorenabled && Person::players[i]->occluded < 2) {
                    Person::players[i]->losupdatedelay = .2;
                    for (unsigned j = 0; j < Person::players.size(); j++)
                        if (j == 0 || Person::players[j]->skeleton.free || Person::players[j]->aitype != passivetype)
                            if (abs(Random() % 2) || animation[Person::players[j]->animTarget].height != lowheight || j != 0)
                                if (distsq(&Person::players[i]->coords, &Person::players[j]->coords) < 400)
                                    if (normaldotproduct(Person::players[i]->facing, Person::players[j]->coords - Person::players[i]->coords) > 0)
                                        if (Person::players[j]->coords.y < Person::players[i]->coords.y + 5 || Person::players[j]->onterrain)
                                            if (!Person::players[j]->isWallJump() && -1 == checkcollide(
                                                        DoRotation(Person::players[i]->jointPos(head), 0, Person::players[i]->yaw, 0)
                                                        *Person::players[i]->scale + Person::players[i]->coords,
                                                        DoRotation(Person::players[j]->jointPos(head), 0, Person::players[j]->yaw, 0)
                                                        *Person::players[j]->scale + Person::players[j]->coords) ||
                                                    (Person::players[j]->animTarget == hanganim &&
                                                     normaldotproduct(Person::players[j]->facing, Person::players[i]->coords - Person::players[j]->coords) < 0)) {
                                                Person::players[i]->aitype = searchtype;
                                                Person::players[i]->lastchecktime = 12;
                                                Person::players[i]->lastseen = Person::players[j]->coords;
                                                Person::players[i]->lastseentime = 12;
                                            }
                }
            }
            if (Person::players[i]->aitype == attacktypecutoff && musictype != 2)
                if (Person::players[i]->creature != wolftype) {
                    Person::players[i]->stunned = .6;
                    Person::players[i]->surprised = .6;
                }
        }

        if (Person::players[i]->aitype != passivetype && leveltime > .5)
            Person::players[i]->howactive = typeactive;

        if (Person::players[i]->aitype == passivetype) {
            Person::players[i]->aiupdatedelay -= multiplier;
            Person::players[i]->losupdatedelay -= multiplier;
            Person::players[i]->lastseentime += multiplier;
            Person::players[i]->pausetime -= multiplier;
            if (Person::players[i]->lastseentime > 1)
                Person::players[i]->lastseentime = 1;

            if (Person::players[i]->aiupdatedelay < 0) {
                if (Person::players[i]->numwaypoints > 1 && Person::players[i]->howactive == typeactive && Person::players[i]->pausetime <= 0) {
                    Person::players[i]->targetyaw = roughDirectionTo(Person::players[i]->coords, Person::players[i]->waypoints[Person::players[i]->waypoint]);
                    Person::players[i]->lookyaw = Person::players[i]->targetyaw;
                    Person::players[i]->aiupdatedelay = .05;

                    if (distsqflat(&Person::players[i]->coords, &Person::players[i]->waypoints[Person::players[i]->waypoint]) < 1) {
                        if (Person::players[i]->waypointtype[Person::players[i]->waypoint] == wppause)
                            Person::players[i]->pausetime = 4;
                        Person::players[i]->waypoint++;
                        if (Person::players[i]->waypoint > Person::players[i]->numwaypoints - 1)
                            Person::players[i]->waypoint = 0;

                    }
                }

                if (Person::players[i]->numwaypoints > 1 && Person::players[i]->howactive == typeactive && Person::players[i]->pausetime <= 0)
                    Person::players[i]->forwardkeydown = 1;
                else
                    Person::players[i]->forwardkeydown = 0;
                Person::players[i]->leftkeydown = 0;
                Person::players[i]->backkeydown = 0;
                Person::players[i]->rightkeydown = 0;
                Person::players[i]->crouchkeydown = 0;
                Person::players[i]->attackkeydown = 0;
                Person::players[i]->throwkeydown = 0;

                if (Person::players[i]->avoidcollided > .8 && !Person::players[i]->jumpkeydown && Person::players[i]->collided < .8) {
                    if (!Person::players[i]->avoidsomething)
                        Person::players[i]->targetyaw += 90 * (Person::players[i]->whichdirection * 2 - 1);
                    else {
                        XYZ leftpos, rightpos;
                        float leftdist, rightdist;
                        leftpos = Person::players[i]->coords + DoRotation(Person::players[i]->facing, 0, 90, 0);
                        rightpos = Person::players[i]->coords - DoRotation(Person::players[i]->facing, 0, 90, 0);
                        leftdist = distsq(&leftpos, &Person::players[i]->avoidwhere);
                        rightdist = distsq(&rightpos, &Person::players[i]->avoidwhere);
                        if (leftdist < rightdist)
                            Person::players[i]->targetyaw += 90;
                        else
                            Person::players[i]->targetyaw -= 90;
                    }
                }
            }
            if (Person::players[i]->collided < 1 || Person::players[i]->animTarget != jumpupanim)
                Person::players[i]->jumpkeydown = 0;
            if ((Person::players[i]->collided > .8 && Person::players[i]->jumppower >= 5))
                Person::players[i]->jumpkeydown = 1;


            //hearing sounds
            if (!editorenabled) {
                if (Person::players[i]->howactive <= typesleeping)
                    if (numenvsounds > 0 && (tutoriallevel != 1 || cananger) && hostile)
                        for (int j = 0; j < numenvsounds; j++) {
                            float vol = Person::players[i]->howactive == typesleeping ? envsoundvol[j] - 14 : envsoundvol[j];
                            if (vol > 0 && distsq(&Person::players[i]->coords, &envsound[j]) <
                                    2 * (vol + vol * (Person::players[i]->creature == rabbittype) * 3))
                                Person::players[i]->aitype = attacktypecutoff;
                        }

                if (Person::players[i]->aitype != passivetype) {
                    if (Person::players[i]->howactive == typesleeping)
                        Person::players[i]->setAnimation(getupfromfrontanim);
                    Person::players[i]->howactive = typeactive;
                }
            }

            if (Person::players[i]->howactive < typesleeping &&
                    ((tutoriallevel != 1 || cananger) && hostile) &&
                    !Person::players[0]->dead &&
                    distsq(&Person::players[i]->coords, &Person::players[0]->coords) < 400 &&
                    Person::players[i]->occluded < 25) {
                if (distsq(&Person::players[i]->coords, &Person::players[0]->coords) < 12 &&
                        animation[Person::players[0]->animTarget].height != lowheight && !editorenabled)
                    Person::players[i]->aitype = attacktypecutoff;
                if (distsq(&Person::players[i]->coords, &Person::players[0]->coords) < 30 &&
                        animation[Person::players[0]->animTarget].height == highheight && !editorenabled)
                    Person::players[i]->aitype = attacktypecutoff;

                //wolf smell
                if (Person::players[i]->creature == wolftype) {
                    XYZ windsmell;
                    for (unsigned j = 0; j < Person::players.size(); j++) {
                        if (j == 0 || (Person::players[j]->dead && Person::players[j]->bloodloss > 0)) {
                            float smelldistance = 50;
                            if (j == 0 && Person::players[j]->num_weapons > 0) {
                                if (weapons[Person::players[j]->weaponids[0]].bloody)
                                    smelldistance = 100;
                                if (Person::players[j]->num_weapons == 2)
                                    if (weapons[Person::players[j]->weaponids[1]].bloody)
                                        smelldistance = 100;
                            }
                            if (j != 0)
                                smelldistance = 100;
                            windsmell = windvector;
                            Normalise(&windsmell);
                            windsmell = windsmell * 2 + Person::players[j]->coords;
                            if (distsq(&Person::players[i]->coords, &windsmell) < smelldistance && !editorenabled)
                                Person::players[i]->aitype = attacktypecutoff;
                        }
                    }
                }

                if (Person::players[i]->howactive < typesleeping && Person::players[i]->losupdatedelay < 0 && !editorenabled && Person::players[i]->occluded < 2) {
                    Person::players[i]->losupdatedelay = .2;
                    for (unsigned j = 0; j < Person::players.size(); j++) {
                        if (j == 0 || Person::players[j]->skeleton.free || Person::players[j]->aitype != passivetype) {
                            if (abs(Random() % 2) || animation[Person::players[j]->animTarget].height != lowheight || j != 0)
                                if (distsq(&Person::players[i]->coords, &Person::players[j]->coords) < 400)
                                    if (normaldotproduct(Person::players[i]->facing, Person::players[j]->coords - Person::players[i]->coords) > 0)
                                        if ((-1 == checkcollide(
                                                    DoRotation(Person::players[i]->jointPos(head), 0, Person::players[i]->yaw, 0)*
                                                    Person::players[i]->scale + Person::players[i]->coords,
                                                    DoRotation(Person::players[j]->jointPos(head), 0, Person::players[j]->yaw, 0)*
                                                    Person::players[j]->scale + Person::players[j]->coords) &&
                                                !Person::players[j]->isWallJump()) ||
                                                (Person::players[j]->animTarget == hanganim &&
                                                 normaldotproduct(Person::players[j]->facing, Person::players[i]->coords - Person::players[j]->coords) < 0)) {
                                            Person::players[i]->lastseentime -= .2;
                                            if (j == 0 && animation[Person::players[j]->animTarget].height == lowheight)
                                                Person::players[i]->lastseentime -= .4;
                                            else
                                                Person::players[i]->lastseentime -= .6;
                                        }
                            if (Person::players[i]->lastseentime <= 0) {
                                Person::players[i]->aitype = searchtype;
                                Person::players[i]->lastchecktime = 12;
                                Person::players[i]->lastseen = Person::players[j]->coords;
                                Person::players[i]->lastseentime = 12;
                            }
                        }
                    }
                }
            }
            //alerted surprise
            if (Person::players[i]->aitype == attacktypecutoff && musictype != 2) {
                if (Person::players[i]->creature != wolftype) {
                    Person::players[i]->stunned = .6;
                    Person::players[i]->surprised = .6;
                }
                if (Person::players[i]->creature == wolftype) {
                    Person::players[i]->stunned = .47;
                    Person::players[i]->surprised = .47;
                }
                numseen++;
            }
        }

        //search for player
        int j;
        if (Person::players[i]->aitype == searchtype) {
            Person::players[i]->aiupdatedelay -= multiplier;
            Person::players[i]->losupdatedelay -= multiplier;
            if (!Person::players[i]->pause)
                Person::players[i]->lastseentime -= multiplier;
            Person::players[i]->lastchecktime -= multiplier;

            if (Person::players[i]->isRun() && !Person::players[i]->onground) {
                if (Person::players[i]->coords.y > terrain.getHeight(Person::players[i]->coords.x, Person::players[i]->coords.z) + 10) {
                    XYZ test2 = Person::players[i]->coords + Person::players[i]->facing;
                    test2.y += 5;
                    XYZ test = Person::players[i]->coords + Person::players[i]->facing;
                    test.y -= 10;
                    j = checkcollide(test2, test, Person::players[i]->laststanding);
                    if (j == -1)
                        j = checkcollide(test2, test);
                    if (j == -1) {
                        Person::players[i]->velocity = 0;
                        Person::players[i]->setAnimation(Person::players[i]->getStop());
                        Person::players[i]->targetyaw += 180;
                        Person::players[i]->stunned = .5;
                        //Person::players[i]->aitype=passivetype;
                        Person::players[i]->aitype = pathfindtype;
                        Person::players[i]->finalfinaltarget = Person::players[i]->waypoints[Person::players[i]->waypoint];
                        Person::players[i]->finalpathfindpoint = -1;
                        Person::players[i]->targetpathfindpoint = -1;
                        Person::players[i]->lastpathfindpoint = -1;
                        Person::players[i]->lastpathfindpoint2 = -1;
                        Person::players[i]->lastpathfindpoint3 = -1;
                        Person::players[i]->lastpathfindpoint4 = -1;
                    } else
                        Person::players[i]->laststanding = j;
                }
            }
            //check out last seen location
            if (Person::players[i]->aiupdatedelay < 0) {
                Person::players[i]->targetyaw = roughDirectionTo(Person::players[i]->coords, Person::players[i]->lastseen);
                Person::players[i]->lookyaw = Person::players[i]->targetyaw;
                Person::players[i]->aiupdatedelay = .05;
                Person::players[i]->forwardkeydown = 1;

                if (distsqflat(&Person::players[i]->coords, &Person::players[i]->lastseen) < 1 * sq(Person::players[i]->scale * 5) || Person::players[i]->lastchecktime < 0) {
                    Person::players[i]->forwardkeydown = 0;
                    Person::players[i]->aiupdatedelay = 1;
                    Person::players[i]->lastseen.x += (float(Random() % 100) - 50) / 25;
                    Person::players[i]->lastseen.z += (float(Random() % 100) - 50) / 25;
                    Person::players[i]->lastchecktime = 3;
                }

                Person::players[i]->leftkeydown = 0;
                Person::players[i]->backkeydown = 0;
                Person::players[i]->rightkeydown = 0;
                Person::players[i]->crouchkeydown = 0;
                Person::players[i]->attackkeydown = 0;
                Person::players[i]->throwkeydown = 0;

                if (Person::players[i]->avoidcollided > .8 && !Person::players[i]->jumpkeydown && Person::players[i]->collided < .8) {
                    if (!Person::players[i]->avoidsomething)
                        Person::players[i]->targetyaw += 90 * (Person::players[i]->whichdirection * 2 - 1);
                    else {
                        XYZ leftpos, rightpos;
                        float leftdist, rightdist;
                        leftpos = Person::players[i]->coords + DoRotation(Person::players[i]->facing, 0, 90, 0);
                        rightpos = Person::players[i]->coords - DoRotation(Person::players[i]->facing, 0, 90, 0);
                        leftdist = distsq(&leftpos, &Person::players[i]->avoidwhere);
                        rightdist = distsq(&rightpos, &Person::players[i]->avoidwhere);
                        if (leftdist < rightdist)
                            Person::players[i]->targetyaw += 90;
                        else
                            Person::players[i]->targetyaw -= 90;
                    }
                }
            }
            if (Person::players[i]->collided < 1 || Person::players[i]->animTarget != jumpupanim)
                Person::players[i]->jumpkeydown = 0;
            if ((Person::players[i]->collided > .8 && Person::players[i]->jumppower >= 5))
                Person::players[i]->jumpkeydown = 1;

            if (numenvsounds > 0 && ((tutoriallevel != 1 || cananger) && hostile))
                for (int k = 0; k < numenvsounds; k++) {
                    if (distsq(&Person::players[i]->coords, &envsound[k]) < 2 * (envsoundvol[k] + envsoundvol[k] * (Person::players[i]->creature == rabbittype) * 3)) {
                        Person::players[i]->aitype = attacktypecutoff;
                    }
                }

            if (!Person::players[0]->dead &&
                    Person::players[i]->losupdatedelay < 0 &&
                    !editorenabled &&
                    Person::players[i]->occluded < 2 &&
                    ((tutoriallevel != 1 || cananger) && hostile)) {
                Person::players[i]->losupdatedelay = .2;
                if (distsq(&Person::players[i]->coords, &Person::players[0]->coords) < 4 && animation[Person::players[i]->animTarget].height != lowheight) {
                    Person::players[i]->aitype = attacktypecutoff;
                    Person::players[i]->lastseentime = 1;
                }
                if (abs(Random() % 2) || animation[Person::players[i]->animTarget].height != lowheight)
                    //TODO: factor out canSeePlayer()
                    if (distsq(&Person::players[i]->coords, &Person::players[0]->coords) < 400)
                        if (normaldotproduct(Person::players[i]->facing, Person::players[0]->coords - Person::players[i]->coords) > 0)
                            if ((checkcollide(
                                        DoRotation(Person::players[i]->jointPos(head), 0, Person::players[i]->yaw, 0)*
                                        Person::players[i]->scale + Person::players[i]->coords,
                                        DoRotation(Person::players[0]->jointPos(head), 0, Person::players[0]->yaw, 0)*
                                        Person::players[0]->scale + Person::players[0]->coords) == -1) ||
                                    (Person::players[0]->animTarget == hanganim && normaldotproduct(
                                         Person::players[0]->facing, Person::players[i]->coords - Person::players[0]->coords) < 0)) {
                                /* //TODO: changed j to 0 on a whim, make sure this is correct
                                (Person::players[j]->animTarget==hanganim&&normaldotproduct(
                                    Person::players[j]->facing,Person::players[i]->coords-Person::players[j]->coords)<0)
                                */
                                Person::players[i]->aitype = attacktypecutoff;
                                Person::players[i]->lastseentime = 1;
                            }
            }
            //player escaped
            if (Person::players[i]->lastseentime < 0) {
                //Person::players[i]->aitype=passivetype;
                numescaped++;
                Person::players[i]->aitype = pathfindtype;
                Person::players[i]->finalfinaltarget = Person::players[i]->waypoints[Person::players[i]->waypoint];
                Person::players[i]->finalpathfindpoint = -1;
                Person::players[i]->targetpathfindpoint = -1;
                Person::players[i]->lastpathfindpoint = -1;
                Person::players[i]->lastpathfindpoint2 = -1;
                Person::players[i]->lastpathfindpoint3 = -1;
                Person::players[i]->lastpathfindpoint4 = -1;
            }
        }

        if (Person::players[i]->aitype != gethelptype)
            Person::players[i]->runninghowlong = 0;

        //get help from buddies
        if (Person::players[i]->aitype == gethelptype) {
            Person::players[i]->runninghowlong += multiplier;
            Person::players[i]->aiupdatedelay -= multiplier;

            if (Person::players[i]->aiupdatedelay < 0 || Person::players[i]->ally == 0) {
                Person::players[i]->aiupdatedelay = .2;

                //find closest ally
                //TODO: factor out closest search somehow
                if (!Person::players[i]->ally) {
                    int closest = -1;
                    float closestdist = -1;
                    for (unsigned k = 0; k < Person::players.size(); k++) {
                        if (k != i && k != 0 && !Person::players[k]->dead &&
                                Person::players[k]->howactive < typedead1 &&
                                !Person::players[k]->skeleton.free &&
                                Person::players[k]->aitype == passivetype) {
                            float distance = distsq(&Person::players[i]->coords, &Person::players[k]->coords);
                            if (closestdist == -1 || distance < closestdist) {
                                closestdist = distance;
                                closest = k;
                            }
                            closest = k;
                        }
                    }
                    if (closest != -1)
                        Person::players[i]->ally = closest;
                    else
                        Person::players[i]->ally = 0;
                    Person::players[i]->lastseen = Person::players[0]->coords;
                    Person::players[i]->lastseentime = 12;
                }


                Person::players[i]->lastchecktime = 12;

                XYZ facing = Person::players[i]->coords;
                XYZ flatfacing = Person::players[Person::players[i]->ally]->coords;
                facing.y += Person::players[i]->jointPos(head).y * Person::players[i]->scale;
                flatfacing.y += Person::players[Person::players[i]->ally]->jointPos(head).y * Person::players[Person::players[i]->ally]->scale;
                if (-1 != checkcollide(facing, flatfacing))
                    Person::players[i]->lastseentime -= .1;

                //no available ally, run back to player
                if (Person::players[i]->ally <= 0 ||
                        Person::players[Person::players[i]->ally]->skeleton.free ||
                        Person::players[Person::players[i]->ally]->aitype != passivetype ||
                        Person::players[i]->lastseentime <= 0) {
                    Person::players[i]->aitype = searchtype;
                    Person::players[i]->lastseentime = 12;
                }

                //seek out ally
                if (Person::players[i]->ally > 0) {
                    Person::players[i]->targetyaw = roughDirectionTo(Person::players[i]->coords, Person::players[Person::players[i]->ally]->coords);
                    Person::players[i]->lookyaw = Person::players[i]->targetyaw;
                    Person::players[i]->aiupdatedelay = .05;
                    Person::players[i]->forwardkeydown = 1;

                    if (distsqflat(&Person::players[i]->coords, &Person::players[Person::players[i]->ally]->coords) < 3) {
                        Person::players[i]->aitype = searchtype;
                        Person::players[i]->lastseentime = 12;
                        Person::players[Person::players[i]->ally]->aitype = searchtype;
                        if (Person::players[Person::players[i]->ally]->lastseentime < Person::players[i]->lastseentime) {
                            Person::players[Person::players[i]->ally]->lastseen = Person::players[i]->lastseen;
                            Person::players[Person::players[i]->ally]->lastseentime = Person::players[i]->lastseentime;
                            Person::players[Person::players[i]->ally]->lastchecktime = Person::players[i]->lastchecktime;
                        }
                    }

                    if (Person::players[i]->avoidcollided > .8 && !Person::players[i]->jumpkeydown && Person::players[i]->collided < .8) {
                        if (!Person::players[i]->avoidsomething)
                            Person::players[i]->targetyaw += 90 * (Person::players[i]->whichdirection * 2 - 1);
                        else {
                            XYZ leftpos, rightpos;
                            float leftdist, rightdist;
                            leftpos = Person::players[i]->coords + DoRotation(Person::players[i]->facing, 0, 90, 0);
                            rightpos = Person::players[i]->coords - DoRotation(Person::players[i]->facing, 0, 90, 0);
                            leftdist = distsq(&leftpos, &Person::players[i]->avoidwhere);
                            rightdist = distsq(&rightpos, &Person::players[i]->avoidwhere);
                            if (leftdist < rightdist)
                                Person::players[i]->targetyaw += 90;
                            else
                                Person::players[i]->targetyaw -= 90;
                        }
                    }
                }

                Person::players[i]->leftkeydown = 0;
                Person::players[i]->backkeydown = 0;
                Person::players[i]->rightkeydown = 0;
                Person::players[i]->crouchkeydown = 0;
                Person::players[i]->attackkeydown = 0;
            }
            if (Person::players[i]->collided < 1 || Person::players[i]->animTarget != jumpupanim)
                Person::players[i]->jumpkeydown = 0;
            if (Person::players[i]->collided > .8 && Person::players[i]->jumppower >= 5)
                Person::players[i]->jumpkeydown = 1;
        }

        //retreiving a weapon on the ground
        if (Person::players[i]->aitype == getweapontype) {
            Person::players[i]->aiupdatedelay -= multiplier;
            Person::players[i]->lastchecktime -= multiplier;

            if (Person::players[i]->aiupdatedelay < 0) {
                Person::players[i]->aiupdatedelay = .2;

                //ALLY IS WEPON
                if (Person::players[i]->ally < 0) {
                    int closest = -1;
                    float closestdist = -1;
                    for (unsigned k = 0; k < weapons.size(); k++)
                        if (weapons[k].owner == -1) {
                            float distance = distsq(&Person::players[i]->coords, &weapons[k].position);
                            if (closestdist == -1 || distance < closestdist) {
                                closestdist = distance;
                                closest = k;
                            }
                            closest = k;
                        }
                    if (closest != -1)
                        Person::players[i]->ally = closest;
                    else
                        Person::players[i]->ally = -1;
                }

                Person::players[i]->lastseentime = 12;

                if (!Person::players[0]->dead && ((tutoriallevel != 1 || cananger) && hostile))
                    if (Person::players[i]->ally < 0 || Person::players[i]->weaponactive != -1 || Person::players[i]->lastchecktime <= 0) {
                        Person::players[i]->aitype = attacktypecutoff;
                        Person::players[i]->lastseentime = 1;
                    }
                if (!Person::players[0]->dead)
                    if (Person::players[i]->ally >= 0) {
                        if (weapons[Person::players[i]->ally].owner != -1 ||
                                distsq(&Person::players[i]->coords, &weapons[Person::players[i]->ally].position) > 16) {
                            Person::players[i]->aitype = attacktypecutoff;
                            Person::players[i]->lastseentime = 1;
                        }
                        //TODO: factor these out as moveToward()
                        Person::players[i]->targetyaw = roughDirectionTo(Person::players[i]->coords, weapons[Person::players[i]->ally].position);
                        Person::players[i]->lookyaw = Person::players[i]->targetyaw;
                        Person::players[i]->aiupdatedelay = .05;
                        Person::players[i]->forwardkeydown = 1;


                        if (Person::players[i]->avoidcollided > .8 && !Person::players[i]->jumpkeydown && Person::players[i]->collided < .8) {
                            if (!Person::players[i]->avoidsomething)
                                Person::players[i]->targetyaw += 90 * (Person::players[i]->whichdirection * 2 - 1);
                            else {
                                XYZ leftpos, rightpos;
                                float leftdist, rightdist;
                                leftpos = Person::players[i]->coords + DoRotation(Person::players[i]->facing, 0, 90, 0);
                                rightpos = Person::players[i]->coords - DoRotation(Person::players[i]->facing, 0, 90, 0);
                                leftdist = distsq(&leftpos, &Person::players[i]->avoidwhere);
                                rightdist = distsq(&rightpos, &Person::players[i]->avoidwhere);
                                if (leftdist < rightdist)
                                    Person::players[i]->targetyaw += 90;
                                else
                                    Person::players[i]->targetyaw -= 90;
                            }
                        }
                    }

                Person::players[i]->leftkeydown = 0;
                Person::players[i]->backkeydown = 0;
                Person::players[i]->rightkeydown = 0;
                Person::players[i]->attackkeydown = 0;
                Person::players[i]->throwkeydown = 1;
                Person::players[i]->crouchkeydown = 0;
                if (Person::players[i]->animTarget != crouchremoveknifeanim &&
                        Person::players[i]->animTarget != removeknifeanim)
                    Person::players[i]->throwtogglekeydown = 0;
                Person::players[i]->drawkeydown = 0;
            }
            if (Person::players[i]->collided < 1 || Person::players[i]->animTarget != jumpupanim)
                Person::players[i]->jumpkeydown = 0;
            if ((Person::players[i]->collided > .8 && Person::players[i]->jumppower >= 5))
                Person::players[i]->jumpkeydown = 1;
        }

        if (Person::players[i]->aitype == attacktypecutoff) {
            Person::players[i]->aiupdatedelay -= multiplier;
            //dodge or reverse rabbit kicks, knife throws, flips
            if (Person::players[i]->damage < Person::players[i]->damagetolerance * 2 / 3)
                if ((Person::players[0]->animTarget == rabbitkickanim ||
                        Person::players[0]->animTarget == knifethrowanim ||
                        (Person::players[0]->isFlip() &&
                         normaldotproduct(Person::players[0]->facing, Person::players[0]->coords - Person::players[i]->coords) < 0)) &&
                        !Person::players[0]->skeleton.free &&
                        (Person::players[i]->aiupdatedelay < .1)) {
                    Person::players[i]->attackkeydown = 0;
                    if (Person::players[i]->isIdle())
                        Person::players[i]->crouchkeydown = 1;
                    if (Person::players[0]->animTarget != rabbitkickanim && Person::players[0]->weaponactive != -1) {
                        if (weapons[Person::players[0]->weaponids[0]].getType() == knife) {
                            if (Person::players[i]->isIdle() || Person::players[i]->isCrouch() || Person::players[i]->isRun() || Person::players[i]->isFlip()) {
                                if (abs(Random() % 2) == 0)
                                    Person::players[i]->setAnimation(backhandspringanim);
                                else
                                    Person::players[i]->setAnimation(rollanim);
                                Person::players[i]->targetyaw += 90 * (abs(Random() % 2) * 2 - 1);
                                Person::players[i]->wentforweapon = 0;
                            }
                            if (Person::players[i]->animTarget == jumpupanim || Person::players[i]->animTarget == jumpdownanim)
                                Person::players[i]->setAnimation(flipanim);
                        }
                    }
                    Person::players[i]->forwardkeydown = 0;
                    Person::players[i]->aiupdatedelay = .02;
                }
            //get confused by flips
            if (Person::players[0]->isFlip() &&
                    !Person::players[0]->skeleton.free &&
                    Person::players[0]->animTarget != walljumprightkickanim &&
                    Person::players[0]->animTarget != walljumpleftkickanim) {
                if (distsq(&Person::players[0]->coords, &Person::players[i]->coords) < 25)
                    if ((1 - Person::players[i]->damage / Person::players[i]->damagetolerance) > .5)
                        Person::players[i]->stunned = 1;
            }
            //go for weapon on the ground
            if (Person::players[i]->wentforweapon < 3)
                for (unsigned k = 0; k < weapons.size(); k++)
                    if (Person::players[i]->creature != wolftype)
                        if (Person::players[i]->num_weapons == 0 &&
                                weapons[k].owner == -1 &&
                                weapons[i].velocity.x == 0 &&
                                weapons[i].velocity.z == 0 &&
                                weapons[i].velocity.y == 0) {
                            if (distsq(&Person::players[i]->coords, &weapons[k].position) < 16) {
                                Person::players[i]->wentforweapon++;
                                Person::players[i]->lastchecktime = 6;
                                Person::players[i]->aitype = getweapontype;
                                Person::players[i]->ally = -1;
                            }
                        }
            //dodge/reverse walljump kicks
            if (Person::players[i]->damage < Person::players[i]->damagetolerance / 2)
                if (animation[Person::players[i]->animTarget].height != highheight)
                    if (Person::players[i]->damage < Person::players[i]->damagetolerance * .5 &&
                            ((Person::players[0]->animTarget == walljumprightkickanim ||
                              Person::players[0]->animTarget == walljumpleftkickanim) &&
                             ((Person::players[i]->aiupdatedelay < .15 &&
                               difficulty == 2) ||
                              (Person::players[i]->aiupdatedelay < .08 &&
                               difficulty != 2)))) {
                        Person::players[i]->crouchkeydown = 1;
                    }
            //walked off a ledge (?)
            if (Person::players[i]->isRun() && !Person::players[i]->onground)
                if (Person::players[i]->coords.y > terrain.getHeight(Person::players[i]->coords.x, Person::players[i]->coords.z) + 10) {
                    XYZ test2 = Person::players[i]->coords + Person::players[i]->facing;
                    test2.y += 5;
                    XYZ test = Person::players[i]->coords + Person::players[i]->facing;
                    test.y -= 10;
                    j = checkcollide(test2, test, Person::players[i]->laststanding);
                    if (j == -1)
                        j = checkcollide(test2, test);
                    if (j == -1) {
                        Person::players[i]->velocity = 0;
                        Person::players[i]->setAnimation(Person::players[i]->getStop());
                        Person::players[i]->targetyaw += 180;
                        Person::players[i]->stunned = .5;
                        Person::players[i]->aitype = pathfindtype;
                        Person::players[i]->finalfinaltarget = Person::players[i]->waypoints[Person::players[i]->waypoint];
                        Person::players[i]->finalpathfindpoint = -1;
                        Person::players[i]->targetpathfindpoint = -1;
                        Person::players[i]->lastpathfindpoint = -1;
                        Person::players[i]->lastpathfindpoint2 = -1;
                        Person::players[i]->lastpathfindpoint3 = -1;
                        Person::players[i]->lastpathfindpoint4 = -1;
                    } else
                        Person::players[i]->laststanding = j;
                }
            //lose sight of player in the air (?)
            if (Person::players[0]->coords.y > Person::players[i]->coords.y + 5 &&
                    animation[Person::players[0]->animTarget].height != highheight &&
                    !Person::players[0]->onterrain) {
                Person::players[i]->aitype = pathfindtype;
                Person::players[i]->finalfinaltarget = Person::players[i]->waypoints[Person::players[i]->waypoint];
                Person::players[i]->finalpathfindpoint = -1;
                Person::players[i]->targetpathfindpoint = -1;
                Person::players[i]->lastpathfindpoint = -1;
                Person::players[i]->lastpathfindpoint2 = -1;
                Person::players[i]->lastpathfindpoint3 = -1;
                Person::players[i]->lastpathfindpoint4 = -1;
            }
            //it's time to think (?)
            if (Person::players[i]->aiupdatedelay < 0 &&
                    !animation[Person::players[i]->animTarget].attack &&
                    Person::players[i]->animTarget != staggerbackhighanim &&
                    Person::players[i]->animTarget != staggerbackhardanim &&
                    Person::players[i]->animTarget != backhandspringanim &&
                    Person::players[i]->animTarget != dodgebackanim) {
                //draw weapon
                if (Person::players[i]->weaponactive == -1 && Person::players[i]->num_weapons > 0)
                    Person::players[i]->drawkeydown = Random() % 2;
                else
                    Person::players[i]->drawkeydown = 0;
                Person::players[i]->rabbitkickenabled = Random() % 2;
                //chase player
                XYZ rotatetarget = Person::players[0]->coords + Person::players[0]->velocity;
                XYZ targetpoint = Person::players[0]->coords;
                if (distsq(&Person::players[0]->coords, &Person::players[i]->coords) <
                        distsq(&rotatetarget, &Person::players[i]->coords))
                    targetpoint += Person::players[0]->velocity *
                                   findDistance(&Person::players[0]->coords, &Person::players[i]->coords) / findLength(&Person::players[i]->velocity);
                Person::players[i]->targetyaw = roughDirectionTo(Person::players[i]->coords, targetpoint);
                Person::players[i]->lookyaw = Person::players[i]->targetyaw;
                Person::players[i]->aiupdatedelay = .2 + fabs((float)(Random() % 100) / 1000);

                if (distsq(&Person::players[i]->coords, &Person::players[0]->coords) > 5 && (Person::players[0]->weaponactive == -1 || Person::players[i]->weaponactive != -1))
                    Person::players[i]->forwardkeydown = 1;
                else if ((distsq(&Person::players[i]->coords, &Person::players[0]->coords) > 16 ||
                          distsq(&Person::players[i]->coords, &Person::players[0]->coords) < 9) &&
                         Person::players[0]->weaponactive != -1)
                    Person::players[i]->forwardkeydown = 1;
                else if (Random() % 6 == 0 || (Person::players[i]->creature == wolftype && Random() % 3 == 0))
                    Person::players[i]->forwardkeydown = 1;
                else
                    Person::players[i]->forwardkeydown = 0;
                //chill out around the corpse
                if (Person::players[0]->dead) {
                    Person::players[i]->forwardkeydown = 0;
                    if (Random() % 10 == 0)
                        Person::players[i]->forwardkeydown = 1;
                    if (Random() % 100 == 0) {
                        Person::players[i]->aitype = pathfindtype;
                        Person::players[i]->finalfinaltarget = Person::players[i]->waypoints[Person::players[i]->waypoint];
                        Person::players[i]->finalpathfindpoint = -1;
                        Person::players[i]->targetpathfindpoint = -1;
                        Person::players[i]->lastpathfindpoint = -1;
                        Person::players[i]->lastpathfindpoint2 = -1;
                        Person::players[i]->lastpathfindpoint3 = -1;
                        Person::players[i]->lastpathfindpoint4 = -1;
                    }
                }
                Person::players[i]->leftkeydown = 0;
                Person::players[i]->backkeydown = 0;
                Person::players[i]->rightkeydown = 0;
                Person::players[i]->crouchkeydown = 0;
                Person::players[i]->throwkeydown = 0;

                if (Person::players[i]->avoidcollided > .8 && !Person::players[i]->jumpkeydown && Person::players[i]->collided < .8)
                    Person::players[i]->targetyaw += 90 * (Person::players[i]->whichdirection * 2 - 1);
                //attack!!!
                if (Random() % 2 == 0 || Person::players[i]->weaponactive != -1 || Person::players[i]->creature == wolftype)
                    Person::players[i]->attackkeydown = 1;
                else
                    Person::players[i]->attackkeydown = 0;
                if (Person::players[i]->isRun() && Random() % 6 && distsq(&Person::players[i]->coords, &Person::players[0]->coords) > 7)
                    Person::players[i]->attackkeydown = 0;

                //TODO: wat
                if (Person::players[i]->aitype != playercontrolled &&
                        (Person::players[i]->isIdle() ||
                         Person::players[i]->isCrouch() ||
                         Person::players[i]->isRun())) {
                    int target = -2;
                    for (unsigned j = 0; j < Person::players.size(); j++)
                        if (j != i && !Person::players[j]->skeleton.free &&
                                Person::players[j]->hasvictim &&
                                (tutoriallevel == 1 && reversaltrain ||
                                 Random() % 2 == 0 && difficulty == 2 ||
                                 Random() % 4 == 0 && difficulty == 1 ||
                                 Random() % 8 == 0 && difficulty == 0 ||
                                 Person::players[j]->lastattack2 == Person::players[j]->animTarget &&
                                 Person::players[j]->lastattack3 == Person::players[j]->animTarget &&
                                 (Random() % 2 == 0 || difficulty == 2) ||
                                 (Person::players[i]->isIdle() || Person::players[i]->isRun()) &&
                                 Person::players[j]->weaponactive != -1 ||
                                 Person::players[j]->animTarget == swordslashanim &&
                                 Person::players[i]->weaponactive != -1 ||
                                 Person::players[j]->animTarget == staffhitanim ||
                                 Person::players[j]->animTarget == staffspinhitanim))
                            if (distsq(&Person::players[j]->coords, &Person::players[j]->victim->coords) < 4 &&
                                    Person::players[j]->victim == Person::players[i] &&
                                    (Person::players[j]->animTarget == sweepanim ||
                                     Person::players[j]->animTarget == spinkickanim ||
                                     Person::players[j]->animTarget == staffhitanim ||
                                     Person::players[j]->animTarget == staffspinhitanim ||
                                     Person::players[j]->animTarget == winduppunchanim ||
                                     Person::players[j]->animTarget == upunchanim ||
                                     Person::players[j]->animTarget == wolfslapanim ||
                                     Person::players[j]->animTarget == knifeslashstartanim ||
                                     Person::players[j]->animTarget == swordslashanim &&
                                     (distsq(&Person::players[j]->coords, &Person::players[i]->coords) < 2 ||
                                      Person::players[i]->weaponactive != -1))) {
                                if (target >= 0)
                                    target = -1;
                                else
                                    target = j;
                            }
                    if (target >= 0)
                        Person::players[target]->Reverse();
                }

                if (Person::players[i]->collided < 1)
                    Person::players[i]->jumpkeydown = 0;
                if (Person::players[i]->collided > .8 && Person::players[i]->jumppower >= 5 ||
                        distsq(&Person::players[i]->coords, &Person::players[0]->coords) > 400 &&
                        Person::players[i]->onterrain &&
                        Person::players[i]->creature == rabbittype)
                    Person::players[i]->jumpkeydown = 1;
                //TODO: why are we controlling the human?
                if (normaldotproduct(Person::players[i]->facing, Person::players[0]->coords - Person::players[i]->coords) > 0)
                    Person::players[0]->jumpkeydown = 0;
                if (Person::players[0]->animTarget == jumpdownanim &&
                        distsq(&Person::players[0]->coords, &Person::players[i]->coords) < 40)
                    Person::players[i]->crouchkeydown = 1;
                if (Person::players[i]->jumpkeydown)
                    Person::players[i]->attackkeydown = 0;

                if (tutoriallevel == 1)
                    if (!canattack)
                        Person::players[i]->attackkeydown = 0;


                XYZ facing = Person::players[i]->coords;
                XYZ flatfacing = Person::players[0]->coords;
                facing.y += Person::players[i]->jointPos(head).y * Person::players[i]->scale;
                flatfacing.y += Person::players[0]->jointPos(head).y * Person::players[0]->scale;
                if (Person::players[i]->occluded >= 2)
                    if (-1 != checkcollide(facing, flatfacing)) {
                        if (!Person::players[i]->pause)
                            Person::players[i]->lastseentime -= .2;
                        if (Person::players[i]->lastseentime <= 0 &&
                                (Person::players[i]->creature != wolftype ||
                                 Person::players[i]->weaponstuck == -1)) {
                            Person::players[i]->aitype = searchtype;
                            Person::players[i]->lastchecktime = 12;
                            Person::players[i]->lastseen = Person::players[0]->coords;
                            Person::players[i]->lastseentime = 12;
                        }
                    } else
                        Person::players[i]->lastseentime = 1;
            }
        }
        if (animation[Person::players[0]->animTarget].height == highheight &&
                (Person::players[i]->aitype == attacktypecutoff ||
                 Person::players[i]->aitype == searchtype))
            if (Person::players[0]->coords.y > terrain.getHeight(Person::players[0]->coords.x, Person::players[0]->coords.z) + 10) {
                XYZ test = Person::players[0]->coords;
                test.y -= 40;
                if (-1 == checkcollide(Person::players[0]->coords, test))
                    Person::players[i]->stunned = 1;
            }
        //stunned
        if (Person::players[i]->aitype == passivetype && !(Person::players[i]->numwaypoints > 1) ||
                Person::players[i]->stunned > 0 ||
                Person::players[i]->pause && Person::players[i]->damage > Person::players[i]->superpermanentdamage) {
            if (Person::players[i]->pause)
                Person::players[i]->lastseentime = 1;
            Person::players[i]->targetyaw = Person::players[i]->yaw;
            Person::players[i]->forwardkeydown = 0;
            Person::players[i]->leftkeydown = 0;
            Person::players[i]->backkeydown = 0;
            Person::players[i]->rightkeydown = 0;
            Person::players[i]->jumpkeydown = 0;
            Person::players[i]->attackkeydown = 0;
            Person::players[i]->crouchkeydown = 0;
            Person::players[i]->throwkeydown = 0;
        }


        XYZ facing;
        facing = 0;
        facing.z = -1;

        XYZ flatfacing = DoRotation(facing, 0, Person::players[i]->yaw + 180, 0);
        facing = flatfacing;

        if (Person::players[i]->aitype == attacktypecutoff) {
            Person::players[i]->targetheadyaw = 180 - roughDirectionTo(Person::players[i]->coords, Person::players[0]->coords);
            Person::players[i]->targetheadpitch = pitchTo(Person::players[i]->coords, Person::players[0]->coords);
        } else if (Person::players[i]->howactive >= typesleeping) {
            Person::players[i]->targetheadyaw = Person::players[i]->targetyaw;
            Person::players[i]->targetheadpitch = 0;
        } else {
            if (Person::players[i]->interestdelay <= 0) {
                Person::players[i]->interestdelay = .7 + (float)(abs(Random() % 100)) / 100;
                Person::players[i]->headtarget = Person::players[i]->coords;
                Person::players[i]->headtarget.x += (float)(abs(Random() % 200) - 100) / 100;
                Person::players[i]->headtarget.z += (float)(abs(Random() % 200) - 100) / 100;
                Person::players[i]->headtarget.y += (float)(abs(Random() % 200) - 100) / 300;
                Person::players[i]->headtarget += Person::players[i]->facing * 1.5;
            }
            Person::players[i]->targetheadyaw = 180 - roughDirectionTo(Person::players[i]->coords, Person::players[i]->headtarget);
            Person::players[i]->targetheadpitch = pitchTo(Person::players[i]->coords, Person::players[i]->headtarget);
        }
    }
}



void updateSettingsMenu()
{
    char sbuf[256];
    if ((float)newscreenwidth > (float)newscreenheight * 1.61 || (float)newscreenwidth < (float)newscreenheight * 1.59)
        sprintf (sbuf, "Resolution: %d*%d", (int)newscreenwidth, (int)newscreenheight);
    else
        sprintf (sbuf, "Resolution: %d*%d (widescreen)", (int)newscreenwidth, (int)newscreenheight);
    Menu::setText(0, sbuf);
    Menu::setText(14, fullscreen ? "Fullscreen: On" : "Fullscreen: Off");
    if (newdetail == 0) Menu::setText(1, "Detail: Low");
    if (newdetail == 1) Menu::setText(1, "Detail: Medium");
    if (newdetail == 2) Menu::setText(1, "Detail: High");
    if (bloodtoggle == 0) Menu::setText(2, "Blood: Off");
    if (bloodtoggle == 1) Menu::setText(2, "Blood: On, low detail");
    if (bloodtoggle == 2) Menu::setText(2, "Blood: On, high detail (slower)");
    if (difficulty == 0) Menu::setText(3, "Difficulty: Easier");
    if (difficulty == 1) Menu::setText(3, "Difficulty: Difficult");
    if (difficulty == 2) Menu::setText(3, "Difficulty: Insane");
    Menu::setText(4, ismotionblur ? "Blur Effects: Enabled (less compatible)" : "Blur Effects: Disabled (more compatible)");
    Menu::setText(5, decals ? "Decals: Enabled (slower)" : "Decals: Disabled");
    Menu::setText(6, musictoggle ? "Music: Enabled" : "Music: Disabled");
    Menu::setText(9, invertmouse ? "Invert mouse: Yes" : "Invert mouse: No");
    sprintf (sbuf, "Mouse Speed: %d", (int)(usermousesensitivity * 5));
    Menu::setText(10, sbuf);
    sprintf (sbuf, "Volume: %d%%", (int)(volume * 100));
    Menu::setText(11, sbuf);
    Menu::setText(13, showdamagebar ? "Damage Bar: On" : "Damage Bar: Off");
    if (newdetail == detail && newscreenheight == (int)screenheight && newscreenwidth == (int)screenwidth)
        sprintf (sbuf, "Back");
    else
        sprintf (sbuf, "Back (some changes take effect next time Lugaru is opened)");
    Menu::setText(8, sbuf);
}

void updateStereoConfigMenu()
{
    char sbuf[256];
    sprintf(sbuf, "Stereo mode: %s", StereoModeName(newstereomode));
    Menu::setText(0, sbuf);
    sprintf(sbuf, "Stereo separation: %.3f", stereoseparation);
    Menu::setText(1, sbuf);
    sprintf(sbuf, "Reverse stereo: %s", stereoreverse ? "Yes" : "No");
    Menu::setText(2, sbuf);
}

void updateControlsMenu()
{
    Menu::setText(0, (string)"Forwards: " + (keyselect == 0 ? "_" : Input::keyToChar(forwardkey)));
    Menu::setText(1, (string)"Back: "    + (keyselect == 1 ? "_" : Input::keyToChar(backkey)));
    Menu::setText(2, (string)"Left: "    + (keyselect == 2 ? "_" : Input::keyToChar(leftkey)));
    Menu::setText(3, (string)"Right: "   + (keyselect == 3 ? "_" : Input::keyToChar(rightkey)));
    Menu::setText(4, (string)"Crouch: "  + (keyselect == 4 ? "_" : Input::keyToChar(crouchkey)));
    Menu::setText(5, (string)"Jump: "    + (keyselect == 5 ? "_" : Input::keyToChar(jumpkey)));
    Menu::setText(6, (string)"Draw: "    + (keyselect == 6 ? "_" : Input::keyToChar(drawkey)));
    Menu::setText(7, (string)"Throw: "   + (keyselect == 7 ? "_" : Input::keyToChar(throwkey)));
    Menu::setText(8, (string)"Attack: "  + (keyselect == 8 ? "_" : Input::keyToChar(attackkey)));
    if (debugmode)
        Menu::setText(9, (string)"Console: " + (keyselect == 9 ? "_" : Input::keyToChar(consolekey)));
}

/*
Values of mainmenu :
1 Main menu
2 Menu pause (resume/end game)
3 Option menu
4 Controls configuration menu
5 Main game menu (choose level or challenge)
6 Deleting user menu
7 User managment menu (select/add)
8 Choose difficulty menu
9 Challenge level selection menu
10 End of the campaign congratulation (is that really a menu?)
11 Same that 9 ??? => unused
18 stereo configuration
*/

void Game::LoadMenu()
{
    Menu::clearMenu();
    switch (mainmenu) {
    case 1:
    case 2:
        Menu::addImage(0, Mainmenuitems[0], 150, 480 - 128, 256, 128);
        Menu::addButtonImage(1, Mainmenuitems[mainmenu == 1 ? 1 : 5], 18, 480 - 152 - 32, 128, 32);
        Menu::addButtonImage(2, Mainmenuitems[2], 18, 480 - 228 - 32, 112, 32);
        Menu::addButtonImage(3, Mainmenuitems[mainmenu == 1 ? 3 : 6], 18, 480 - 306 - 32, mainmenu == 1 ? 68 : 132, 32);
        break;
    case 3:
        Menu::addButton( 0, "", 10 + 20, 440);
        Menu::addButton(14, "", 10 + 400, 440);
        Menu::addButton( 1, "", 10 + 60, 405);
        Menu::addButton( 2, "", 10 + 70, 370);
        Menu::addButton( 3, "", 10 + 20 - 1000, 335 - 1000);
        Menu::addButton( 4, "", 10   , 335);
        Menu::addButton( 5, "", 10 + 60, 300);
        Menu::addButton( 6, "", 10 + 70, 265);
        Menu::addButton( 9, "", 10   , 230);
        Menu::addButton(10, "", 20   , 195);
        Menu::addButton(11, "", 10 + 60, 160);
        Menu::addButton(13, "", 30   , 125);
        Menu::addButton( 7, "-Configure Controls-", 10 + 15, 90);
        Menu::addButton(12, "-Configure Stereo -", 10 + 15, 55);
        Menu::addButton(8, "Back", 10, 10);
        updateSettingsMenu();
        break;
    case 4:
        Menu::addButton(0, "", 10   , 400);
        Menu::addButton(1, "", 10 + 40, 360);
        Menu::addButton(2, "", 10 + 40, 320);
        Menu::addButton(3, "", 10 + 30, 280);
        Menu::addButton(4, "", 10 + 20, 240);
        Menu::addButton(5, "", 10 + 40, 200);
        Menu::addButton(6, "", 10 + 40, 160);
        Menu::addButton(7, "", 10 + 30, 120);
        Menu::addButton(8, "", 10 + 20, 80);
        if (debugmode)
            Menu::addButton(9, "", 10 + 10, 40);
        Menu::addButton(debugmode ? 10 : 9, "Back", 10, 10);
        updateControlsMenu();
        break;
    case 5: {
        LoadCampaign();
        Menu::addLabel(-1, accountactive->getName(), 5, 400);
        Menu::addButton(1, "Tutorial", 5, 300);
        Menu::addButton(2, "Challenge", 5, 240);
        Menu::addButton(3, "Delete User", 400, 10);
        Menu::addButton(4, "Main Menu", 5, 10);
        Menu::addButton(5, "Change User", 5, 180);
        Menu::addButton(6, "Campaign : " + accountactive->getCurrentCampaign(), 200, 420);

        //show campaign map
        //with (2,-5) offset from old code
        Menu::addImage(-1, Mainmenuitems[7], 150 + 2, 60 - 5, 400, 400);
        //show levels
        int numlevels = accountactive->getCampaignChoicesMade();
        numlevels += numlevels > 0 ? campaignlevels[numlevels - 1].nextlevel.size() : 1;
        for (int i = 0; i < numlevels; i++) {
            XYZ midpoint = campaignlevels[i].getCenter();
            float itemsize = campaignlevels[i].getWidth();
            const bool active = i >= accountactive->getCampaignChoicesMade();
            if (!active)
                itemsize /= 2;

            if (i >= 1) {
                XYZ start = campaignlevels[i - 1].getCenter();
                Menu::addMapLine(start.x, start.y, midpoint.x - start.x, midpoint.y - start.y, 0.5, active ? 1 : 0.5, active ? 1 : 0.5, 0, 0);
            }
            Menu::addMapMarker(NB_CAMPAIGN_MENU_ITEM + i, Mapcircletexture,
                               midpoint.x - itemsize / 2, midpoint.y - itemsize / 2, itemsize, itemsize, active ? 1 : 0.5, 0, 0);

            if (active) {
                Menu::addMapLabel(-2, campaignlevels[i].description,
                                  campaignlevels[i].getStartX() + 10,
                                  campaignlevels[i].getStartY() - 4);
            }
        }
    }
    break;
    case 6:
        Menu::addLabel(-1, "Are you sure you want to delete this user?", 10, 400);
        Menu::addButton(1, "Yes", 10, 360);
        Menu::addButton(2, "No", 10, 320);
        break;
    case 7:
        if (Account::getNbAccounts() < 8)
            Menu::addButton(0, "New User", 10, 400);
        else
            Menu::addLabel(0, "No More Users", 10, 400);
        Menu::addLabel(-2, "", 20, 400);
        Menu::addButton(Account::getNbAccounts() + 1, "Back", 10, 10);
        for (int i = 0; i < Account::getNbAccounts(); i++)
            Menu::addButton(i + 1, Account::get(i)->getName(), 10, 340 - 20 * (i + 1));
        break;
    case 8:
        Menu::addButton(0, "Easier", 10, 400);
        Menu::addButton(1, "Difficult", 10, 360);
        Menu::addButton(2, "Insane", 10, 320);
        break;
    case 9:
        for (int i = 0; i < numchallengelevels; i++) {
            char temp[255];
            string name = "";
            sprintf (temp, "Level %d", i + 1);
            for (int j = strlen(temp); j < 17; j++)
                strcat(temp, " ");
            name += temp;
            sprintf (temp, "%d", (int)accountactive->getHighScore(i));
            for (int j = strlen(temp); j < (32 - 17); j++)
                strcat(temp, " ");
            name += temp;
            sprintf (temp, "%d:", (int)(((int)accountactive->getFastTime(i) - (int)(accountactive->getFastTime(i)) % 60) / 60));
            if ((int)(accountactive->getFastTime(i)) % 60 < 10)
                strcat(temp, "0");
            name += temp;
            sprintf (temp, "%d", (int)(accountactive->getFastTime(i)) % 60);
            name += temp;

            Menu::addButton(i, name, 10, 400 - i * 25, i > accountactive->getProgress() ? 0.5 : 1, 0, 0);
        }

        Menu::addButton(-1, "             High Score      Best Time", 10, 440);
        Menu::addButton(numchallengelevels, "Back", 10, 10);
        break;
    case 10: {
        Menu::addLabel(0, "Congratulations!", 220, 330);
        Menu::addLabel(1, "You have avenged your family and", 140, 300);
        Menu::addLabel(2, "restored peace to the island of Lugaru.", 110, 270);
        Menu::addButton(3, "Back", 10, 10);
        char sbuf[256];
        sprintf(sbuf, "Your score:         %d", (int)accountactive->getCampaignScore());
        Menu::addLabel(4, sbuf, 190, 200);
        sprintf(sbuf, "Highest score:      %d", (int)accountactive->getCampaignHighScore());
        Menu::addLabel(5, sbuf, 190, 180);
    }
    break;
    case 18:
        Menu::addButton(0, "", 70, 400);
        Menu::addButton(1, "", 10, 360);
        Menu::addButton(2, "", 40, 320);
        Menu::addButton(3, "Back", 10, 10);
        updateStereoConfigMenu();
        break;
    }
}

extern set<pair<int,int>> resolutions;

void MenuTick()
{
    //menu buttons
    selected = Menu::getSelected(mousecoordh * 640 / screenwidth, 480 - mousecoordv * 480 / screenheight);

    // some specific case where we do something even if the left mouse button is not pressed.
    if ((mainmenu == 5) && (endgame == 2)) {
        accountactive->endGame();
        endgame = 0;
    }
    if (mainmenu == 10)
        endgame = 2;
    if (mainmenu == 18 && Input::isKeyPressed(MOUSEBUTTON2) && selected == 1) {
        stereoseparation -= 0.001;
        updateStereoConfigMenu();
    }

    static int oldmainmenu = mainmenu;

    if (Input::MouseClicked() && (selected >= 0)) { // handling of the left mouse clic in menus
        set<pair<int,int>>::iterator newscreenresolution;
        switch (mainmenu) {
        case 1:
        case 2:
            switch (selected) {
            case 1:
                if (gameon) { //resume
                    mainmenu = 0;
                    pause_sound(stream_menutheme);
                    resume_stream(leveltheme);
                } else { //new game
                    fireSound(firestartsound);
                    flash();
                    mainmenu = (accountactive ? 5 : 7);
                    selected = -1;
                }
                break;
            case 2: //options
                fireSound();
                flash();
                mainmenu = 3;
                if (newdetail > 2)
                    newdetail = detail;
                if (newdetail < 0)
                    newdetail = detail;
                if (newscreenwidth > 3000)
                    newscreenwidth = screenwidth;
                if (newscreenwidth < 0)
                    newscreenwidth = screenwidth;
                if (newscreenheight > 3000)
                    newscreenheight = screenheight;
                if (newscreenheight < 0)
                    newscreenheight = screenheight;
                break;
            case 3:
                fireSound();
                flash();
                if (gameon) { //end game
                    gameon = 0;
                    mainmenu = 1;
                } else { //quit
                    tryquit = 1;
                    pause_sound(stream_menutheme);
                }
                break;
            }
            break;
        case 3:
            fireSound();
            switch (selected) {
            case 0:
                newscreenresolution = resolutions.find(make_pair(newscreenwidth, newscreenheight));
                /* Next one (end() + 1 is also end() so the ++ is safe even if it was not found) */
                newscreenresolution++;
                if (newscreenresolution == resolutions.end()) {
                    /* It was the last one (or not found), go back to the beginning */
                    newscreenresolution = resolutions.begin();
                }
                newscreenwidth  = newscreenresolution->first;
                newscreenheight = newscreenresolution->second;
                break;
            case 1:
                newdetail++;
                if (newdetail > 2)
                    newdetail = 0;
                break;
            case 2:
                bloodtoggle++;
                if (bloodtoggle > 2)
                    bloodtoggle = 0;
                break;
            case 3:
                difficulty++;
                if (difficulty > 2)
                    difficulty = 0;
                break;
            case 4:
                ismotionblur = !ismotionblur;
                break;
            case 5:
                decals = !decals;
                break;
            case 6:
                musictoggle = !musictoggle;
                if (musictoggle) {
                    emit_stream_np(stream_menutheme);
                } else {
                    pause_sound(leveltheme);
                    pause_sound(stream_fighttheme);
                    pause_sound(stream_menutheme);

                    for (int i = 0; i < 4; i++) {
                        oldmusicvolume[i] = 0;
                        musicvolume[i] = 0;
                    }
                }
                break;
            case 7: // controls
                flash();
                mainmenu = 4;
                selected = -1;
                keyselect = -1;
                break;
            case 8:
                flash();
                SaveSettings();
                mainmenu = gameon ? 2 : 1;
                break;
            case 9:
                invertmouse = !invertmouse;
                break;
            case 10:
                usermousesensitivity += .2;
                if (usermousesensitivity > 2)
                    usermousesensitivity = .2;
                break;
            case 11:
                volume += .1f;
                if (volume > 1.0001f)
                    volume = 0;
                OPENAL_SetSFXMasterVolume((int)(volume * 255));
                break;
            case 12:
                flash();
                newstereomode = stereomode;
                mainmenu = 18;
                keyselect = -1;
                break;
            case 13:
                showdamagebar = !showdamagebar;
                break;
            case 14:
                toggleFullscreen();
                break;
            }
            updateSettingsMenu();
            break;
        case 4:
            if (!waiting) {
                fireSound();
                if (selected < (debugmode ? 10 : 9) && keyselect == -1)
                    keyselect = selected;
                if (keyselect != -1)
                    setKeySelected();
                if (selected == (debugmode ? 10 : 9)) {
                    flash();
                    mainmenu = 3;
                }
            }
            updateControlsMenu();
            break;
        case 5:
            fireSound();
            flash();
            if ((selected - NB_CAMPAIGN_MENU_ITEM >= accountactive->getCampaignChoicesMade())) {
                startbonustotal = 0;

                loading = 2;
                loadtime = 0;
                targetlevel = 7;
                if (firstload)
                    TickOnceAfter();
                else
                    LoadStuff();
                whichchoice = selected - NB_CAMPAIGN_MENU_ITEM - accountactive->getCampaignChoicesMade();
                actuallevel = (accountactive->getCampaignChoicesMade() > 0 ? campaignlevels[accountactive->getCampaignChoicesMade() - 1].nextlevel[whichchoice] : 0);
                visibleloading = 1;
                stillloading = 1;
                Loadlevel(campaignlevels[actuallevel].mapname.c_str());
                campaign = 1;
                mainmenu = 0;
                gameon = 1;
                pause_sound(stream_menutheme);
            }
            switch (selected) {
            case 1:
                startbonustotal = 0;

                loading = 2;
                loadtime = 0;
                targetlevel = -1;
                if (firstload) {
                    TickOnceAfter();
                } else
                    LoadStuff();
                Loadlevel(-1);

                mainmenu = 0;
                gameon = 1;
                pause_sound(stream_menutheme);
                break;
            case 2:
                mainmenu = 9;
                break;
            case 3:
                mainmenu = 6;
                break;
            case 4:
                mainmenu = (gameon ? 2 : 1);
                break;
            case 5:
                mainmenu = 7;
                break;
            case 6:
                vector<string> campaigns = ListCampaigns();
                vector<string>::iterator c;
                if ((c = find(campaigns.begin(), campaigns.end(), accountactive->getCurrentCampaign())) == campaigns.end()) {
                    if (!campaigns.empty())
                        accountactive->setCurrentCampaign(campaigns.front());
                } else {
                    c++;
                    if (c == campaigns.end())
                        c = campaigns.begin();
                    accountactive->setCurrentCampaign(*c);
                }
                LoadMenu();
                break;
            }
            break;
        case 6:
            fireSound();
            if (selected == 1) {
                flash();
                accountactive = Account::destroy(accountactive);
                mainmenu = 7;
            } else if (selected == 2) {
                flash();
                mainmenu = 5;
            }
            break;
        case 7:
            fireSound();
            if (selected == 0 && Account::getNbAccounts() < 8) {
                entername = 1;
            } else if (selected < Account::getNbAccounts() + 1) {
                flash();
                mainmenu = 5;
                accountactive = Account::get(selected - 1);
            } else if (selected == Account::getNbAccounts() + 1) {
                flash();
                if (accountactive)
                    mainmenu = 5;
                else
                    mainmenu = 1;
                displaytext[0].clear();
                displayselected = 0;
                entername = 0;
            }
            break;
        case 8:
            fireSound();
            flash();
            if (selected <= 2)
                accountactive->setDifficulty(selected);
            mainmenu = 5;
            break;
        case 9:
            if (selected < numchallengelevels && selected <= accountactive->getProgress()) {
                fireSound();
                flash();

                startbonustotal = 0;

                loading = 2;
                loadtime = 0;
                targetlevel = selected;
                if (firstload)
                    TickOnceAfter();
                else
                    LoadStuff();
                Loadlevel(selected);
                campaign = 0;

                mainmenu = 0;
                gameon = 1;
                pause_sound(stream_menutheme);
            }
            if (selected == numchallengelevels) {
                fireSound();
                flash();
                mainmenu = 5;
            }
            break;
        case 10:
            if (selected == 3) {
                fireSound();
                flash();
                mainmenu = 5;
            }
            break;
        case 18:
            if (selected == 1)
                stereoseparation += 0.001;
            else {
                fireSound();
                if (selected == 0) {
                    newstereomode = (StereoMode)(newstereomode + 1);
                    while (!CanInitStereo(newstereomode)) {
                        printf("Failed to initialize mode %s (%i)\n", StereoModeName(newstereomode), newstereomode);
                        newstereomode = (StereoMode)(newstereomode + 1);
                        if (newstereomode >= stereoCount)
                            newstereomode = stereoNone;
                    }
                } else if (selected == 2) {
                    stereoreverse = !stereoreverse;
                } else if (selected == 3) {
                    flash();
                    mainmenu = 3;

                    stereomode = newstereomode;
                    InitStereo(stereomode);
                }
            }
            updateStereoConfigMenu();
            break;
        }
    }

    OPENAL_SetFrequency(channels[stream_menutheme], 22050);

    if (entername) {
        inputText(displaytext[0], &displayselected);
        if (!waiting) { // the input as finished
            if (!displaytext[0].empty()) { // with enter
                accountactive = Account::add(string(displaytext[0]));

                mainmenu = 8;

                flash();

                fireSound(firestartsound);

                displaytext[0].clear();

                displayselected = 0;
            }
            entername = 0;
            LoadMenu();
        }

        displayblinkdelay -= multiplier;
        if (displayblinkdelay <= 0) {
            displayblinkdelay = .3;
            displayblink = !displayblink;
        }
    }

    if (entername) {
        Menu::setText(0, displaytext[0], 20, 400, -1, -1);
        Menu::setText(-2, displayblink ? "_" : "", 20 + displayselected * 10, 400, -1, -1);
    }

    if (oldmainmenu != mainmenu)
        LoadMenu();
    oldmainmenu = mainmenu;

}

void Game::Tick()
{
    static XYZ facing, flatfacing;
    static int target;

    for (int i = 0; i < 15; i++) {
        displaytime[i] += multiplier;
    }

    Input::Tick();

    if (Input::isKeyPressed(SDL_SCANCODE_F6)) {
        if (Input::isKeyDown(SDL_SCANCODE_LSHIFT))
            stereoreverse = true;
        else
            stereoreverse = false;

        if (stereoreverse)
            printf("Stereo reversed\n");
        else
            printf("Stereo unreversed\n");
    }

    if (Input::isKeyDown(SDL_SCANCODE_F7)) {
        if (Input::isKeyDown(SDL_SCANCODE_LSHIFT))
            stereoseparation -= 0.001;
        else
            stereoseparation -= 0.010;
        printf("Stereo decreased increased to %f\n", stereoseparation);
    }

    if (Input::isKeyDown(SDL_SCANCODE_F8)) {
        if (Input::isKeyDown(SDL_SCANCODE_LSHIFT))
            stereoseparation += 0.001;
        else
            stereoseparation += 0.010;
        printf("Stereo separation increased to %f\n", stereoseparation);
    }


    if (Input::isKeyPressed(SDL_SCANCODE_TAB) && tutoriallevel) {
        if (tutorialstage != 51)
            tutorialstagetime = tutorialmaxtime;
        emit_sound_np(consolefailsound, 128.);
    }

    /*
    Values of mainmenu :
    1 Main menu
    2 Menu pause (resume/end game)
    3 Option menu
    4 Controls configuration menu
    5 Main game menu (choose level or challenge)
    6 Deleting user menu
    7 User managment menu (select/add)
    8 Choose difficulty menu
    9 Challenge level selection menu
    10 End of the campaign congratulation (is that really a menu?)
    11 Same that 9 ??? => unused
    18 stereo configuration
    */

    if (!console) {
        //campaign over?
        if (mainmenu && endgame == 1)
            mainmenu = 10;
        //go to level select after completing a campaign level
        if (campaign && winfreeze && mainmenu == 0 && campaignlevels[actuallevel].choosenext == 1) {
            mainmenu = 5;
            gameon = 0;
            winfreeze = 0;
            fireSound();
            flash();
            if (musictoggle) {
                OPENAL_SetFrequency(OPENAL_ALL, 0.001);
                emit_stream_np(stream_menutheme);
                pause_sound(leveltheme);
            }
            LoadMenu();
        }
        //escape key pressed
        if (Input::isKeyPressed(SDL_SCANCODE_ESCAPE) &&
                (gameon || mainmenu == 0 || (mainmenu >= 3 && mainmenu != 8 && !(mainmenu == 7 && entername)))) {
            selected = -1;
            if (mainmenu == 0 && !winfreeze)
                mainmenu = 2; //pause
            else if (mainmenu == 1 || mainmenu == 2) {
                mainmenu = 0; //unpause
            }
            //play menu theme
            if (musictoggle && (mainmenu == 1 || mainmenu == 2)) {
                OPENAL_SetFrequency(OPENAL_ALL, 0.001);
                emit_stream_np(stream_menutheme);
                pause_sound(leveltheme);
            }
            //on resume, play level music
            if (!mainmenu) {
                pause_sound(stream_menutheme);
                resume_stream(leveltheme);
            }
            //finished with settings menu
            if (mainmenu == 3) {
                SaveSettings();
            }
            //effects
            if (mainmenu >= 3 && mainmenu != 8) {
                fireSound();
                flash();
            }
            //go back
            switch (mainmenu) {
            case 3:
            case 5:
                mainmenu = gameon ? 2 : 1;
                break;
            case 4:
            case 18:
                mainmenu = 3;
                break;
            case 6:
            case 7:
            case 9:
            case 10:
                mainmenu = 5;
                break;
            }
        }
    }

    if (mainmenu) {
        MenuTick();
    }

    if (!mainmenu) {
        if (hostile == 1)
            hostiletime += multiplier;
        else
            hostiletime = 0;
        if (!winfreeze)
            leveltime += multiplier;

        //keys
        if (Input::isKeyPressed(SDL_SCANCODE_V) && debugmode) {
            freeze = !freeze;
            if (freeze) {
                OPENAL_SetFrequency(OPENAL_ALL, 0.001);
            }
        }

        if (Input::isKeyPressed(consolekey) && debugmode) {
            console = !console;
            if (console) {
                OPENAL_SetFrequency(OPENAL_ALL, 0.001);
            } else {
                freeze = 0;
                waiting = false;
            }
        }

        if (console)
            freeze = 1;
        if (console && !Input::isKeyDown(SDL_SCANCODE_LGUI)) {
            inputText(consoletext[0], &consoleselected);
            if (!waiting) {
                if (!consoletext[0].empty()) {
                    cmd_dispatch(consoletext[0]);
                    for (int k = 14; k >= 1; k--) {
                        consoletext[k] = consoletext[k - 1];
                    }
                    consoletext[0].clear();
                    consoleselected = 0;
                }
            }

            consoleblinkdelay -= multiplier;
            if (consoleblinkdelay <= 0) {
                consoleblinkdelay = .3;
                consoleblink = !consoleblink;
            }
        }

        static int oldwinfreeze;
        if (winfreeze && !oldwinfreeze) {
            OPENAL_SetFrequency(OPENAL_ALL, 0.001);
            emit_sound_np(consolesuccesssound);
        }
        if (winfreeze == 0)
            oldwinfreeze = winfreeze;
        else
            oldwinfreeze++;

        if ((Input::isKeyPressed(jumpkey) || Input::isKeyPressed(SDL_SCANCODE_SPACE)) && !campaign)
            if (winfreeze)
                winfreeze = 0;
        if ((Input::isKeyDown(SDL_SCANCODE_ESCAPE)) && !campaign && gameon) {
            if (console) {
                console = false;
                freeze = 0;
            } else if (winfreeze) {
                mainmenu = 9;
                gameon = 0;
            }
        }



        if (!freeze && !winfreeze && !(mainmenu && gameon) && (gameon || !gamestarted)) {

            //dialogues
            static float talkdelay = 0;

            if (indialogue != -1)
                talkdelay = 1;
            talkdelay -= multiplier;

            if (talkdelay <= 0 && indialogue == -1 && animation[Person::players[0]->animTarget].height != highheight)
                for (int i = 0; i < numdialogues; i++) {
                    unsigned realdialoguetype;
                    bool special;
                    /* FIXME - Seems like modulo done with ifs */
                    if (dialoguetype[i] > 49) {
                        realdialoguetype = dialoguetype[i] - 50;
                        special = 1;
                    } else if (dialoguetype[i] > 39) {
                        realdialoguetype = dialoguetype[i] - 40;
                        special = 1;
                    } else if (dialoguetype[i] > 29) {
                        realdialoguetype = dialoguetype[i] - 30;
                        special = 1;
                    } else if (dialoguetype[i] > 19) {
                        realdialoguetype = dialoguetype[i] - 20;
                        special = 1;
                    } else if (dialoguetype[i] > 9) {
                        realdialoguetype = dialoguetype[i] - 10;
                        special = 1;
                    } else {
                        realdialoguetype = dialoguetype[i];
                        special = 0;
                    }
                    if ((!hostile || dialoguetype[i] > 40 && dialoguetype[i] < 50) &&
                            realdialoguetype < Person::players.size() &&
                            realdialoguetype > 0 &&
                            (dialoguegonethrough[i] == 0 || !special) &&
                            (special || Input::isKeyPressed(attackkey))) {
                        if (distsq(&Person::players[0]->coords, &Person::players[realdialoguetype]->coords) < 6 ||
                                Person::players[realdialoguetype]->howactive >= typedead1 ||
                                dialoguetype[i] > 40 && dialoguetype[i] < 50) {
                            whichdialogue = i;
                            for (int j = 0; j < numdialogueboxes[whichdialogue]; j++) {
                                Person::players[participantfocus[whichdialogue][j]]->coords = participantlocation[whichdialogue][participantfocus[whichdialogue][j]];
                                Person::players[participantfocus[whichdialogue][j]]->yaw = participantyaw[whichdialogue][participantfocus[whichdialogue][j]];
                                Person::players[participantfocus[whichdialogue][j]]->targetyaw = participantyaw[whichdialogue][participantfocus[whichdialogue][j]];
                                Person::players[participantfocus[whichdialogue][j]]->velocity = 0;
                                Person::players[participantfocus[whichdialogue][j]]->animTarget = Person::players[participantfocus[whichdialogue][j]]->getIdle();
                                Person::players[participantfocus[whichdialogue][j]]->frameTarget = 0;
                            }
                            directing = 0;
                            indialogue = 0;
                            dialoguetime = 0;
                            dialoguegonethrough[i]++;
                            if (dialogueboxsound[whichdialogue][indialogue] != 0) {
                                playdialogueboxsound();
                            }
                        }
                    }
                }

            windvar += multiplier;
            smoketex += multiplier;
            tutorialstagetime += multiplier;

            //hotspots
            static float hotspotvisual[40];
            if (numhotspots) {
                XYZ hotspotsprite;
                if (editorenabled)
                    for (int i = 0; i < numhotspots; i++)
                        hotspotvisual[i] -= multiplier / 320;

                for (int i = 0; i < numhotspots; i++) {
                    while (hotspotvisual[i] < 0) {
                        hotspotsprite = 0;
                        hotspotsprite.x = float(abs(Random() % 100000)) / 100000 * hotspotsize[i];
                        hotspotsprite = DoRotation(hotspotsprite, 0, 0, Random() % 360);
                        hotspotsprite = DoRotation(hotspotsprite, 0, Random() % 360, 0);
                        hotspotsprite += hotspot[i];
                        Sprite::MakeSprite(breathsprite, hotspotsprite, hotspotsprite * 0, 1, 0.5, 0, 7, 0.4);
                        hotspotvisual[i] += 0.1 / hotspotsize[i] / hotspotsize[i] / hotspotsize[i];
                    }
                }

                for (int i = 0; i < numhotspots; i++) {
                    if (hotspottype[i] <= 10 && hotspottype[i] > 0) {
                        hotspot[i] = Person::players[hotspottype[i]]->coords;
                    }
                }
            }

            //Tutorial
            if (tutoriallevel) {
                doTutorial();
            }

            //bonuses
            if (tutoriallevel != 1) {
                if (bonustime == 0 &&
                        bonus != solidhit &&
                        bonus != spinecrusher &&
                        bonus != tracheotomy &&
                        bonus != backstab &&
                        bonusvalue > 10) {
                    emit_sound_np(consolesuccesssound);
                }
            } else if (bonustime == 0) {
                emit_sound_np(fireendsound);
            }
            if (bonustime == 0) {
                if (bonus != solidhit &&
                        bonus != twoxcombo &&
                        bonus != threexcombo &&
                        bonus != fourxcombo &&
                        bonus != megacombo)
                    bonusnum[bonus]++;
                else
                    bonusnum[bonus] += 0.15;
                if (tutoriallevel)
                    bonusvalue = 0;
                bonusvalue /= bonusnum[bonus];
                bonustotal += bonusvalue;
            }
            bonustime += multiplier;

            //snow effects
            if (environment == snowyenvironment) {
                precipdelay -= multiplier;
                while (precipdelay < 0) {
                    precipdelay += .04;
                    if (!detail)
                        precipdelay += .04;
                    XYZ footvel, footpoint;

                    footvel = 0;
                    footpoint = viewer + viewerfacing * 6;
                    footpoint.y += ((float)abs(Random() % 1200)) / 100 - 6;
                    footpoint.x += ((float)abs(Random() % 1200)) / 100 - 6;
                    footpoint.z += ((float)abs(Random() % 1200)) / 100 - 6;
                    Sprite::MakeSprite(snowsprite, footpoint, footvel, 1, 1, 1, .1, 1);
                }
            }


            doAerialAcrobatics();


            static XYZ oldviewer;

            //control keys
            if (indialogue == -1) {
                Person::players[0]->forwardkeydown = Input::isKeyDown(forwardkey);
                Person::players[0]->leftkeydown = Input::isKeyDown(leftkey);
                Person::players[0]->backkeydown = Input::isKeyDown(backkey);
                Person::players[0]->rightkeydown = Input::isKeyDown(rightkey);
                Person::players[0]->jumpkeydown = Input::isKeyDown(jumpkey);
                Person::players[0]->crouchkeydown = Input::isKeyDown(crouchkey);
                Person::players[0]->drawkeydown = Input::isKeyDown(drawkey);
                Person::players[0]->throwkeydown = Input::isKeyDown(throwkey);
            } else {
                Person::players[0]->forwardkeydown = 0;
                Person::players[0]->leftkeydown = 0;
                Person::players[0]->backkeydown = 0;
                Person::players[0]->rightkeydown = 0;
                Person::players[0]->jumpkeydown = 0;
                Person::players[0]->crouchkeydown = 0;
                Person::players[0]->drawkeydown = 0;
                Person::players[0]->throwkeydown = 0;
            }

            if (!Person::players[0]->jumpkeydown)
                Person::players[0]->jumpclimb = 0;


            if (indialogue != -1) {
                cameramode = 1;
                if (directing) {
                    facing = 0;
                    facing.z = -1;

                    facing = DoRotation(facing, -pitch, 0, 0);
                    facing = DoRotation(facing, 0, 0 - yaw, 0);

                    flatfacing = 0;
                    flatfacing.z = -1;

                    flatfacing = DoRotation(flatfacing, 0, -yaw, 0);

                    if (Input::isKeyDown(forwardkey))
                        viewer += facing * multiplier * 4;
                    if (Input::isKeyDown(backkey))
                        viewer -= facing * multiplier * 4;
                    if (Input::isKeyDown(leftkey))
                        viewer += DoRotation(flatfacing * multiplier, 0, 90, 0) * 4;
                    if (Input::isKeyDown(rightkey))
                        viewer += DoRotation(flatfacing * multiplier, 0, -90, 0) * 4;
                    if (Input::isKeyDown(jumpkey))
                        viewer.y += multiplier * 4;
                    if (Input::isKeyDown(crouchkey))
                        viewer.y -= multiplier * 4;
                    if (     Input::isKeyPressed(SDL_SCANCODE_1) ||
                             Input::isKeyPressed(SDL_SCANCODE_2) ||
                             Input::isKeyPressed(SDL_SCANCODE_3) ||
                             Input::isKeyPressed(SDL_SCANCODE_4) ||
                             Input::isKeyPressed(SDL_SCANCODE_5) ||
                             Input::isKeyPressed(SDL_SCANCODE_6) ||
                             Input::isKeyPressed(SDL_SCANCODE_7) ||
                             Input::isKeyPressed(SDL_SCANCODE_8) ||
                             Input::isKeyPressed(SDL_SCANCODE_9) ||
                             Input::isKeyPressed(SDL_SCANCODE_0) ||
                             Input::isKeyPressed(SDL_SCANCODE_MINUS)) {
                        int whichend;
                        if (Input::isKeyPressed(SDL_SCANCODE_1)) whichend = 1;
                        if (Input::isKeyPressed(SDL_SCANCODE_2)) whichend = 2;
                        if (Input::isKeyPressed(SDL_SCANCODE_3)) whichend = 3;
                        if (Input::isKeyPressed(SDL_SCANCODE_4)) whichend = 4;
                        if (Input::isKeyPressed(SDL_SCANCODE_5)) whichend = 5;
                        if (Input::isKeyPressed(SDL_SCANCODE_6)) whichend = 6;
                        if (Input::isKeyPressed(SDL_SCANCODE_7)) whichend = 7;
                        if (Input::isKeyPressed(SDL_SCANCODE_8)) whichend = 8;
                        if (Input::isKeyPressed(SDL_SCANCODE_9)) whichend = 9;
                        if (Input::isKeyPressed(SDL_SCANCODE_0)) whichend = 0;
                        if (Input::isKeyPressed(SDL_SCANCODE_MINUS))
                            whichend = -1;
                        if (whichend != -1) {
                            participantfocus[whichdialogue][indialogue] = whichend;
                            participantlocation[whichdialogue][whichend] = Person::players[whichend]->coords;
                            participantyaw[whichdialogue][whichend] = Person::players[whichend]->yaw;
                        }
                        if (whichend == -1) {
                            participantfocus[whichdialogue][indialogue] = -1;
                        }
                        if (Person::players[participantfocus[whichdialogue][indialogue]]->dead) {
                            indialogue = -1;
                            directing = 0;
                            cameramode = 0;
                        }
                        dialoguecamera[whichdialogue][indialogue] = viewer;
                        dialoguecamerayaw[whichdialogue][indialogue] = yaw;
                        dialoguecamerapitch[whichdialogue][indialogue] = pitch;
                        indialogue++;
                        if (indialogue < numdialogueboxes[whichdialogue]) {
                            if (dialogueboxsound[whichdialogue][indialogue] != 0) {
                                playdialogueboxsound();
                            }
                        }

                        for (unsigned j = 0; j < Person::players.size(); j++) {
                            participantfacing[whichdialogue][indialogue][j] = participantfacing[whichdialogue][indialogue - 1][j];
                        }
                    }
                    //TODO: should these be KeyDown or KeyPressed?
                    if (     Input::isKeyDown(SDL_SCANCODE_KP_1) ||
                             Input::isKeyDown(SDL_SCANCODE_KP_2) ||
                             Input::isKeyDown(SDL_SCANCODE_KP_3) ||
                             Input::isKeyDown(SDL_SCANCODE_KP_4) ||
                             Input::isKeyDown(SDL_SCANCODE_KP_5) ||
                             Input::isKeyDown(SDL_SCANCODE_KP_6) ||
                             Input::isKeyDown(SDL_SCANCODE_KP_7) ||
                             Input::isKeyDown(SDL_SCANCODE_KP_8) ||
                             Input::isKeyDown(SDL_SCANCODE_KP_9) ||
                             Input::isKeyDown(SDL_SCANCODE_KP_0)) {
                        int whichend;
                        if (Input::isKeyDown(SDL_SCANCODE_KP_1)) whichend = 1;
                        if (Input::isKeyDown(SDL_SCANCODE_KP_2)) whichend = 2;
                        if (Input::isKeyDown(SDL_SCANCODE_KP_3)) whichend = 3;
                        if (Input::isKeyDown(SDL_SCANCODE_KP_4)) whichend = 4;
                        if (Input::isKeyDown(SDL_SCANCODE_KP_5)) whichend = 5;
                        if (Input::isKeyDown(SDL_SCANCODE_KP_6)) whichend = 6;
                        if (Input::isKeyDown(SDL_SCANCODE_KP_7)) whichend = 7;
                        if (Input::isKeyDown(SDL_SCANCODE_KP_8)) whichend = 8;
                        if (Input::isKeyDown(SDL_SCANCODE_KP_9)) whichend = 9;
                        if (Input::isKeyDown(SDL_SCANCODE_KP_0)) whichend = 0;
                        participantfacing[whichdialogue][indialogue][whichend] = facing;
                    }
                    if (indialogue >= numdialogueboxes[whichdialogue]) {
                        indialogue = -1;
                        directing = 0;
                        cameramode = 0;
                    }
                }
                if (!directing) {
                    pause_sound(whooshsound);
                    viewer = dialoguecamera[whichdialogue][indialogue];
                    viewer.y = max((double)viewer.y, terrain.getHeight(viewer.x, viewer.z) + .1);
                    yaw = dialoguecamerayaw[whichdialogue][indialogue];
                    pitch = dialoguecamerapitch[whichdialogue][indialogue];
                    if (dialoguetime > 0.5)
                        if (     Input::isKeyPressed(SDL_SCANCODE_1) ||
                                 Input::isKeyPressed(SDL_SCANCODE_2) ||
                                 Input::isKeyPressed(SDL_SCANCODE_3) ||
                                 Input::isKeyPressed(SDL_SCANCODE_4) ||
                                 Input::isKeyPressed(SDL_SCANCODE_5) ||
                                 Input::isKeyPressed(SDL_SCANCODE_6) ||
                                 Input::isKeyPressed(SDL_SCANCODE_7) ||
                                 Input::isKeyPressed(SDL_SCANCODE_8) ||
                                 Input::isKeyPressed(SDL_SCANCODE_9) ||
                                 Input::isKeyPressed(SDL_SCANCODE_0) ||
                                 Input::isKeyPressed(SDL_SCANCODE_MINUS) ||
                                 Input::isKeyPressed(attackkey)) {
                            indialogue++;
                            if (indialogue < numdialogueboxes[whichdialogue]) {
                                if (dialogueboxsound[whichdialogue][indialogue] != 0) {
                                    playdialogueboxsound();
                                    if (dialogueboxsound[whichdialogue][indialogue] == -5) {
                                        hotspot[numhotspots] = Person::players[0]->coords;
                                        hotspotsize[numhotspots] = 10;
                                        hotspottype[numhotspots] = -1;

                                        numhotspots++;
                                    }
                                    if (dialogueboxsound[whichdialogue][indialogue] == -6) {
                                        hostile = 1;
                                    }

                                    if (Person::players[participantfocus[whichdialogue][indialogue]]->dead) {
                                        indialogue = -1;
                                        directing = 0;
                                        cameramode = 0;
                                    }
                                }
                            }
                        }
                    if (indialogue >= numdialogueboxes[whichdialogue]) {
                        indialogue = -1;
                        directing = 0;
                        cameramode = 0;
                        if (dialoguetype[whichdialogue] > 19 && dialoguetype[whichdialogue] < 30) {
                            hostile = 1;
                        }
                        if (dialoguetype[whichdialogue] > 29 && dialoguetype[whichdialogue] < 40) {
                            windialogue = true;
                        }
                        if (dialoguetype[whichdialogue] > 49 && dialoguetype[whichdialogue] < 60) {
                            hostile = 1;
                            for (unsigned i = 1; i < Person::players.size(); i++) {
                                Person::players[i]->aitype = attacktypecutoff;
                            }
                        }
                    }
                }
            }

            if (!Person::players[0]->jumpkeydown) {
                Person::players[0]->jumptogglekeydown = 0;
            }
            if (Person::players[0]->jumpkeydown &&
                    Person::players[0]->animTarget != jumpupanim &&
                    Person::players[0]->animTarget != jumpdownanim &&
                    !Person::players[0]->isFlip())
                Person::players[0]->jumptogglekeydown = 1;


            dialoguetime += multiplier;
            hawkyaw += multiplier * 25;
            realhawkcoords = 0;
            realhawkcoords.x = 25;
            realhawkcoords = DoRotation(realhawkcoords, 0, hawkyaw, 0) + hawkcoords;
            hawkcalldelay -= multiplier / 2;

            if (hawkcalldelay <= 0) {
                emit_sound_at(hawksound, realhawkcoords);

                hawkcalldelay = 16 + abs(Random() % 8);
            }

            doDebugKeys();

            doAttacks();

            doPlayerCollisions();

            doJumpReversals();

            for (unsigned k = 0; k < Person::players.size(); k++)
                if (k != 0 && Person::players[k]->immobile)
                    Person::players[k]->coords = Person::players[k]->realoldcoords;

            for (unsigned k = 0; k < Person::players.size(); k++) {
                if (!isnormal(Person::players[k]->coords.x) || !isnormal(Person::players[k]->coords.y) || !isnormal(Person::players[k]->coords.z)) {
                    if (!isnormal(Person::players[k]->coords.x) || !isnormal(Person::players[k]->coords.y) || !isnormal(Person::players[k]->coords.z)) {
                        Person::players[k]->DoDamage(1000);
                    }
                }
            }

            //respawn
            static bool respawnkeydown;
            if (!editorenabled &&
                    (whichlevel != -2 &&
                     (Input::isKeyDown(SDL_SCANCODE_Z) &&
                      Input::isKeyDown(SDL_SCANCODE_LGUI) &&
                      debugmode) ||
                     (Input::isKeyDown(jumpkey) &&
                      !respawnkeydown &&
                      !oldattackkey &&
                      Person::players[0]->dead))) {
                targetlevel = whichlevel;
                loading = 1;
                leveltime = 5;
            }
            if (!Input::isKeyDown(jumpkey))
                respawnkeydown = 0;
            if (Input::isKeyDown(jumpkey))
                respawnkeydown = 1;




            static bool movekey;

            //?
            for (unsigned i = 0; i < Person::players.size(); i++) {
                static float oldtargetyaw;
                if (!Person::players[i]->skeleton.free) {
                    oldtargetyaw = Person::players[i]->targetyaw;
                    if (i == 0 && indialogue == -1) {
                        //TODO: refactor repetitive code
                        if (!animation[Person::players[0]->animTarget].attack &&
                                Person::players[0]->animTarget != staggerbackhighanim &&
                                Person::players[0]->animTarget != staggerbackhardanim &&
                                Person::players[0]->animTarget != crouchremoveknifeanim &&
                                Person::players[0]->animTarget != removeknifeanim &&
                                Person::players[0]->animTarget != backhandspringanim &&
                                Person::players[0]->animTarget != dodgebackanim &&
                                Person::players[0]->animTarget != walljumprightkickanim &&
                                Person::players[0]->animTarget != walljumpleftkickanim) {
                            if (cameramode)
                                Person::players[0]->targetyaw = 0;
                            else
                                Person::players[0]->targetyaw = -yaw + 180;
                        }

                        facing = 0;
                        facing.z = -1;

                        flatfacing = DoRotation(facing, 0, Person::players[i]->yaw + 180, 0);
                        if (cameramode) {
                            facing = flatfacing;
                        } else {
                            facing = DoRotation(facing, -pitch, 0, 0);
                            facing = DoRotation(facing, 0, 0 - yaw, 0);
                        }

                        Person::players[0]->lookyaw = -yaw;

                        Person::players[i]->targetheadyaw = yaw;
                        Person::players[i]->targetheadpitch = pitch;
                    }
                    if (i != 0 && Person::players[i]->aitype == playercontrolled && indialogue == -1) {
                        if (!animation[Person::players[i]->animTarget].attack &&
                                Person::players[i]->animTarget != staggerbackhighanim &&
                                Person::players[i]->animTarget != staggerbackhardanim &&
                                Person::players[i]->animTarget != crouchremoveknifeanim &&
                                Person::players[i]->animTarget != removeknifeanim &&
                                Person::players[i]->animTarget != backhandspringanim &&
                                Person::players[i]->animTarget != dodgebackanim &&
                                Person::players[i]->animTarget != walljumprightkickanim &&
                                Person::players[i]->animTarget != walljumpleftkickanim) {
                            Person::players[i]->targetyaw = -Person::players[i]->lookyaw + 180;
                        }

                        facing = 0;
                        facing.z = -1;

                        flatfacing = DoRotation(facing, 0, Person::players[i]->yaw + 180, 0);

                        facing = DoRotation(facing, -Person::players[i]->lookpitch, 0, 0);
                        facing = DoRotation(facing, 0, 0 - Person::players[i]->lookyaw, 0);

                        Person::players[i]->targetheadyaw = Person::players[i]->lookyaw;
                        Person::players[i]->targetheadpitch = Person::players[i]->lookpitch;
                    }
                    if (indialogue != -1) {
                        Person::players[i]->targetheadyaw = 180 - roughDirection(participantfacing[whichdialogue][indialogue][i]);
                        Person::players[i]->targetheadpitch = pitchOf(participantfacing[whichdialogue][indialogue][i]);
                    }

                    if (leveltime < .5)
                        numenvsounds = 0;

                    Person::players[i]->avoidsomething = 0;

                    //avoid flaming things
                    for (int j = 0; j < objects.numobjects; j++)
                        if (objects.onfire[j])
                            if (distsq(&Person::players[i]->coords, &objects.position[j]) < sq(objects.scale[j]) * 200)
                                if (     distsq(&Person::players[i]->coords, &objects.position[j]) <
                                         distsq(&Person::players[i]->coords, &Person::players[0]->coords)) {
                                    Person::players[i]->collided = 0;
                                    Person::players[i]->avoidcollided = 1;
                                    if (Person::players[i]->avoidsomething == 0 ||
                                            distsq(&Person::players[i]->coords, &objects.position[j]) <
                                            distsq(&Person::players[i]->coords, &Person::players[i]->avoidwhere)) {
                                        Person::players[i]->avoidwhere = objects.position[j];
                                        Person::players[i]->avoidsomething = 1;
                                    }
                                }

                    //avoid flaming players
                    for (unsigned j = 0; j < Person::players.size(); j++)
                        if (Person::players[j]->onfire)
                            if (distsq(&Person::players[j]->coords, &Person::players[i]->coords) < sq(0.3) * 200)
                                if (     distsq(&Person::players[i]->coords, &Person::players[j]->coords) <
                                         distsq(&Person::players[i]->coords, &Person::players[0]->coords)) {
                                    Person::players[i]->collided = 0;
                                    Person::players[i]->avoidcollided = 1;
                                    if (Person::players[i]->avoidsomething == 0 ||
                                            distsq(&Person::players[i]->coords, &Person::players[j]->coords) <
                                            distsq(&Person::players[i]->coords, &Person::players[i]->avoidwhere)) {
                                        Person::players[i]->avoidwhere = Person::players[j]->coords;
                                        Person::players[i]->avoidsomething = 1;
                                    }
                                }

                    if (Person::players[i]->collided > .8)
                        Person::players[i]->avoidcollided = 0;

                    doAI(i);

                    if (animation[Person::players[i]->animTarget].attack == reversed) {
                        //Person::players[i]->targetyaw=Person::players[i]->yaw;
                        Person::players[i]->forwardkeydown = 0;
                        Person::players[i]->leftkeydown = 0;
                        Person::players[i]->backkeydown = 0;
                        Person::players[i]->rightkeydown = 0;
                        Person::players[i]->jumpkeydown = 0;
                        Person::players[i]->attackkeydown = 0;
                        //Person::players[i]->crouchkeydown=0;
                        Person::players[i]->throwkeydown = 0;
                    }

                    if (indialogue != -1) {
                        Person::players[i]->forwardkeydown = 0;
                        Person::players[i]->leftkeydown = 0;
                        Person::players[i]->backkeydown = 0;
                        Person::players[i]->rightkeydown = 0;
                        Person::players[i]->jumpkeydown = 0;
                        Person::players[i]->crouchkeydown = 0;
                        Person::players[i]->drawkeydown = 0;
                        Person::players[i]->throwkeydown = 0;
                    }

                    if (Person::players[i]->collided < -.3)
                        Person::players[i]->collided = -.3;
                    if (Person::players[i]->collided > 1)
                        Person::players[i]->collided = 1;
                    Person::players[i]->collided -= multiplier * 4;
                    Person::players[i]->whichdirectiondelay -= multiplier;
                    if (Person::players[i]->avoidcollided < -.3 || Person::players[i]->whichdirectiondelay <= 0) {
                        Person::players[i]->avoidcollided = -.3;
                        Person::players[i]->whichdirection = abs(Random() % 2);
                        Person::players[i]->whichdirectiondelay = .4;
                    }
                    if (Person::players[i]->avoidcollided > 1)
                        Person::players[i]->avoidcollided = 1;
                    Person::players[i]->avoidcollided -= multiplier / 4;
                    if (!Person::players[i]->skeleton.free) {
                        Person::players[i]->stunned -= multiplier;
                        Person::players[i]->surprised -= multiplier;
                    }
                    if (i != 0 && Person::players[i]->surprised <= 0 &&
                            Person::players[i]->aitype == attacktypecutoff &&
                            !Person::players[i]->dead &&
                            !Person::players[i]->skeleton.free &&
                            animation[Person::players[i]->animTarget].attack == neutral)
                        numresponded = 1;

                    if (!Person::players[i]->throwkeydown)
                        Person::players[i]->throwtogglekeydown = 0;

                    //pick up weapon
                    if (Person::players[i]->throwkeydown && !Person::players[i]->throwtogglekeydown) {
                        if (Person::players[i]->weaponactive == -1 &&
                                Person::players[i]->num_weapons < 2 &&
                                (Person::players[i]->isIdle() ||
                                 Person::players[i]->isCrouch() ||
                                 Person::players[i]->animTarget == sneakanim ||
                                 Person::players[i]->animTarget == rollanim ||
                                 Person::players[i]->animTarget == backhandspringanim ||
                                 Person::players[i]->isFlip() ||
                                 Person::players[i]->aitype != playercontrolled)) {
                            for (unsigned j = 0; j < weapons.size(); j++) {
                                if ((weapons[j].velocity.x == 0 && weapons[j].velocity.y == 0 && weapons[j].velocity.z == 0 ||
                                        Person::players[i]->aitype == playercontrolled) &&
                                        weapons[j].owner == -1 &&
                                        Person::players[i]->weaponactive == -1)
                                    if (distsqflat(&Person::players[i]->coords, &weapons[j].position) < 2) {
                                        if (distsq(&Person::players[i]->coords, &weapons[j].position) < 2) {
                                            if (Person::players[i]->isCrouch() ||
                                                    Person::players[i]->animTarget == sneakanim ||
                                                    Person::players[i]->isRun() ||
                                                    Person::players[i]->isIdle() ||
                                                    Person::players[i]->aitype != playercontrolled) {
                                                Person::players[i]->throwtogglekeydown = 1;
                                                Person::players[i]->setAnimation(crouchremoveknifeanim);
                                                Person::players[i]->targetyaw = roughDirectionTo(Person::players[i]->coords, weapons[j].position);
                                                Person::players[i]->hasvictim = 0;
                                            }
                                            if (Person::players[i]->animTarget == rollanim || Person::players[i]->animTarget == backhandspringanim) {
                                                Person::players[i]->throwtogglekeydown = 1;
                                                Person::players[i]->hasvictim = 0;

                                                if ((weapons[j].velocity.x == 0 && weapons[j].velocity.y == 0 && weapons[j].velocity.z == 0 ||
                                                        Person::players[i]->aitype == playercontrolled) &&
                                                        weapons[j].owner == -1 ||
                                                        Person::players[i]->victim &&
                                                        weapons[j].owner == int(Person::players[i]->victim->id))
                                                    if (distsqflat(&Person::players[i]->coords, &weapons[j].position) < 2 && Person::players[i]->weaponactive == -1)
                                                        if (distsq(&Person::players[i]->coords, &weapons[j].position) < 1 || Person::players[i]->victim) {
                                                            if (weapons[j].getType() != staff)
                                                                emit_sound_at(knifedrawsound, Person::players[i]->coords, 128.);

                                                            Person::players[i]->takeWeapon(j);
                                                        }
                                            }
                                        } else if ((Person::players[i]->isIdle() ||
                                                    Person::players[i]->isFlip() ||
                                                    Person::players[i]->aitype != playercontrolled) &&
                                                   distsq(&Person::players[i]->coords, &weapons[j].position) < 5 &&
                                                   Person::players[i]->coords.y < weapons[j].position.y) {
                                            if (!Person::players[i]->isFlip()) {
                                                Person::players[i]->throwtogglekeydown = 1;
                                                Person::players[i]->setAnimation(removeknifeanim);
                                                Person::players[i]->targetyaw = roughDirectionTo(Person::players[i]->coords, weapons[j].position);
                                            }
                                            if (Person::players[i]->isFlip()) {
                                                Person::players[i]->throwtogglekeydown = 1;
                                                Person::players[i]->hasvictim = 0;

                                                for (unsigned k = 0; k < weapons.size(); k++) {
                                                    if (Person::players[i]->weaponactive == -1)
                                                        if ((weapons[k].velocity.x == 0 && weapons[k].velocity.y == 0 && weapons[k].velocity.z == 0 ||
                                                                Person::players[i]->aitype == playercontrolled) &&
                                                                weapons[k].owner == -1 ||
                                                                Person::players[i]->victim &&
                                                                weapons[k].owner == int(Person::players[i]->victim->id))
                                                            if (distsqflat(&Person::players[i]->coords, &weapons[k].position) < 3 &&
                                                                    Person::players[i]->weaponactive == -1) {
                                                                if (weapons[k].getType() != staff)
                                                                    emit_sound_at(knifedrawsound, Person::players[i]->coords, 128.);

                                                                Person::players[i]->takeWeapon(k);
                                                            }
                                                }
                                            }
                                        }
                                    }
                            }
                            if (Person::players[i]->isCrouch() ||
                                    Person::players[i]->animTarget == sneakanim ||
                                    Person::players[i]->isRun() ||
                                    Person::players[i]->isIdle() || Person::players[i]->animTarget == rollanim ||
                                    Person::players[i]->animTarget == backhandspringanim) {
                                if (Person::players.size() > 1)
                                    for (unsigned j = 0; j < Person::players.size(); j++) {
                                        if (Person::players[i]->weaponactive == -1)
                                            if (j != i)
                                                if (Person::players[j]->num_weapons &&
                                                        Person::players[j]->skeleton.free &&
                                                        distsq(&Person::players[i]->coords, &Person::players[j]->coords) < 2/*&&Person::players[j]->dead*/ &&
                                                        (((Person::players[j]->skeleton.forward.y < 0 &&
                                                           Person::players[j]->weaponstuckwhere == 0) ||
                                                          (Person::players[j]->skeleton.forward.y > 0 &&
                                                           Person::players[j]->weaponstuckwhere == 1)) ||
                                                         Person::players[j]->weaponstuck == -1 ||
                                                         Person::players[j]->num_weapons > 1)) {
                                                    if (Person::players[i]->animTarget != rollanim && Person::players[i]->animTarget != backhandspringanim) {
                                                        Person::players[i]->throwtogglekeydown = 1;
                                                        Person::players[i]->victim = Person::players[j];
                                                        Person::players[i]->hasvictim = 1;
                                                        Person::players[i]->setAnimation(crouchremoveknifeanim);
                                                        Person::players[i]->targetyaw = roughDirectionTo(Person::players[i]->coords, Person::players[j]->coords);
                                                    }
                                                    if (Person::players[i]->animTarget == rollanim || Person::players[i]->animTarget == backhandspringanim) {
                                                        Person::players[i]->throwtogglekeydown = 1;
                                                        Person::players[i]->victim = Person::players[j];
                                                        Person::players[i]->hasvictim = 1;
                                                        int k = Person::players[j]->weaponids[0];
                                                        if (Person::players[i]->hasvictim) {
                                                            bool fleshstuck;
                                                            fleshstuck = 0;
                                                            if (Person::players[i]->victim->weaponstuck != -1) {
                                                                if (Person::players[i]->victim->weaponids[Person::players[i]->victim->weaponstuck] == k) {
                                                                    fleshstuck = 1;
                                                                }
                                                            }
                                                            if (!fleshstuck) {
                                                                if (weapons[k].getType() != staff)
                                                                    emit_sound_at(knifedrawsound, Person::players[i]->coords, 128.);
                                                            }
                                                            if (fleshstuck)
                                                                emit_sound_at(fleshstabremovesound, Person::players[i]->coords, 128.);

                                                            if (weapons[k].owner != -1) {
                                                                if (Person::players[i]->victim->num_weapons == 1)
                                                                    Person::players[i]->victim->num_weapons = 0;
                                                                else
                                                                    Person::players[i]->victim->num_weapons = 1;

                                                                Person::players[i]->victim->skeleton.longdead = 0;
                                                                Person::players[i]->victim->skeleton.free = 1;
                                                                Person::players[i]->victim->skeleton.broken = 0;

                                                                for (int l = 0; l < Person::players[i]->victim->skeleton.num_joints; l++) {
                                                                    Person::players[i]->victim->skeleton.joints[l].velchange = 0;
                                                                    Person::players[i]->victim->skeleton.joints[l].locked = 0;
                                                                }

                                                                XYZ relative;
                                                                relative = 0;
                                                                relative.y = 10;
                                                                Normalise(&relative);
                                                                XYZ footvel, footpoint;
                                                                footvel = 0;
                                                                footpoint = weapons[k].position;
                                                                if (Person::players[i]->victim->weaponstuck != -1) {
                                                                    if (Person::players[i]->victim->weaponids[Person::players[i]->victim->weaponstuck] == k) {
                                                                        if (bloodtoggle)
                                                                            Sprite::MakeSprite(cloudimpactsprite, footpoint, footvel, 1, 0, 0, .8, .3);
                                                                        weapons[k].bloody = 2;
                                                                        weapons[k].blooddrip = 5;
                                                                        Person::players[i]->victim->weaponstuck = -1;
                                                                        Person::players[i]->victim->bloodloss += 2000;
                                                                        Person::players[i]->victim->DoDamage(2000);
                                                                    }
                                                                }
                                                                if (Person::players[i]->victim->num_weapons > 0) {
                                                                    if (Person::players[i]->victim->weaponstuck != 0 && Person::players[i]->victim->weaponstuck != -1)
                                                                        Person::players[i]->victim->weaponstuck = 0;
                                                                    if (Person::players[i]->victim->weaponids[0] == k)
                                                                        Person::players[i]->victim->weaponids[0] = Person::players[i]->victim->weaponids[Person::players[i]->victim->num_weapons];
                                                                }

                                                                Person::players[i]->victim->weaponactive = -1;

                                                                Person::players[i]->victim->jointVel(abdomen) += relative * 6;
                                                                Person::players[i]->victim->jointVel(neck) += relative * 6;
                                                                Person::players[i]->victim->jointVel(rightshoulder) += relative * 6;
                                                                Person::players[i]->victim->jointVel(leftshoulder) += relative * 6;
                                                            }
                                                            Person::players[i]->takeWeapon(k);
                                                        }
                                                    }
                                                }
                                    }
                            }
                        }
                        if (Person::players[i]->weaponactive != -1 && Person::players[i]->aitype == playercontrolled) {
                            if (weapons[Person::players[i]->weaponids[0]].getType() == knife) {
                                if (Person::players[i]->isIdle() ||
                                        Person::players[i]->isRun() ||
                                        Person::players[i]->isCrouch() ||
                                        Person::players[i]->animTarget == sneakanim ||
                                        Person::players[i]->isFlip())
                                    if (Person::players.size() > 1)
                                        for (unsigned j = 0; j < Person::players.size(); j++) {
                                            if (i != j)
                                                if (tutoriallevel != 1 || tutorialstage == 49)
                                                    if (hostile)
                                                        if (normaldotproduct(Person::players[i]->facing, Person::players[i]->coords - Person::players[j]->coords) < 0 &&
                                                                distsq(&Person::players[i]->coords, &Person::players[j]->coords) < 100 &&
                                                                distsq(&Person::players[i]->coords, &Person::players[j]->coords) > 1.5 &&
                                                                !Person::players[j]->skeleton.free &&
                                                                -1 == checkcollide(DoRotation(Person::players[j]->jointPos(head), 0, Person::players[j]->yaw, 0)*Person::players[j]->scale + Person::players[j]->coords, DoRotation(Person::players[i]->jointPos(head), 0, Person::players[i]->yaw, 0)*Person::players[i]->scale + Person::players[i]->coords)) {
                                                            if (!Person::players[i]->isFlip()) {
                                                                Person::players[i]->throwtogglekeydown = 1;
                                                                Person::players[i]->victim = Person::players[j];
                                                                Person::players[i]->setAnimation(knifethrowanim);
                                                                Person::players[i]->targetyaw = roughDirectionTo(Person::players[i]->coords, Person::players[j]->coords);
                                                                Person::players[i]->targettilt2 = pitchTo(Person::players[i]->coords, Person::players[j]->coords);
                                                            }
                                                            if (Person::players[i]->isFlip()) {
                                                                if (Person::players[i]->weaponactive != -1) {
                                                                    Person::players[i]->throwtogglekeydown = 1;
                                                                    Person::players[i]->victim = Person::players[j];
                                                                    XYZ aim;
                                                                    aim = Person::players[i]->victim->coords + DoRotation(Person::players[i]->victim->jointPos(abdomen), 0, Person::players[i]->victim->yaw, 0) * Person::players[i]->victim->scale + Person::players[i]->victim->velocity * findDistance(&Person::players[i]->victim->coords, &Person::players[i]->coords) / 50 - (Person::players[i]->coords + DoRotation(Person::players[i]->jointPos(righthand), 0, Person::players[i]->yaw, 0) * Person::players[i]->scale);
                                                                    Normalise(&aim);

                                                                    aim = DoRotation(aim, (float)abs(Random() % 30) - 15, (float)abs(Random() % 30) - 15, 0);

                                                                    weapons[Person::players[i]->weaponids[0]].thrown(aim * 50, false);
                                                                    Person::players[i]->num_weapons--;
                                                                    if (Person::players[i]->num_weapons) {
                                                                        Person::players[i]->weaponids[0] = Person::players[i]->weaponids[Person::players[i]->num_weapons];
                                                                    }
                                                                    Person::players[i]->weaponactive = -1;
                                                                }
                                                            }
                                                        }
                                        }
                            }
                        }
                        if (Person::players[i]->weaponactive != -1 && Person::players[i]->aitype == playercontrolled) {
                            if (Person::players[i]->isCrouch() || Person::players[i]->animTarget == sneakanim) {
                                Person::players[i]->throwtogglekeydown = 1;
                                XYZ tempVelocity = Person::players[i]->velocity * .2;
                                if (tempVelocity.x == 0)
                                    tempVelocity.x = .1;
                                weapons[Person::players[i]->weaponids[0]].drop(tempVelocity, tempVelocity, false);
                                Person::players[i]->num_weapons--;
                                if (Person::players[i]->num_weapons) {
                                    Person::players[i]->weaponids[0] = Person::players[i]->weaponids[Person::players[i]->num_weapons];
                                    if (Person::players[i]->weaponstuck == Person::players[i]->num_weapons)
                                        Person::players[i]->weaponstuck = 0;
                                }

                                Person::players[i]->weaponactive = -1;
                                for (unsigned j = 0; j < Person::players.size(); j++) {
                                    Person::players[j]->wentforweapon = 0;
                                }
                            }
                        }

                    }

                    //draw weapon
                    if (i == 0 || !Person::players[0]->dead || (Person::players[i]->weaponactive != -1)) {
                        if (Person::players[i]->drawkeydown && !Person::players[i]->drawtogglekeydown ||
                                (Person::players[i]->num_weapons == 2) &&
                                (Person::players[i]->weaponactive == -1) &&
                                Person::players[i]->isIdle() ||
                                Person::players[0]->dead &&
                                (Person::players[i]->weaponactive != -1) &&
                                i != 0) {
                            bool isgood = true;
                            if (Person::players[i]->weaponactive != -1)
                                if (weapons[Person::players[i]->weaponids[Person::players[i]->weaponactive]].getType() == staff)
                                    isgood = false;
                            if (isgood && Person::players[i]->creature != wolftype) {
                                if (Person::players[i]->isIdle() && Person::players[i]->num_weapons && weapons[Person::players[i]->weaponids[0]].getType() == knife) {
                                    Person::players[i]->setAnimation(drawrightanim);
                                    Person::players[i]->drawtogglekeydown = 1;
                                }
                                if ((Person::players[i]->isIdle() ||
                                        (Person::players[i]->aitype != playercontrolled &&
                                         Person::players[0]->weaponactive != -1 &&
                                         Person::players[i]->isRun())) &&
                                        Person::players[i]->num_weapons &&
                                        weapons[Person::players[i]->weaponids[0]].getType() == sword) {
                                    Person::players[i]->setAnimation(drawleftanim);
                                    Person::players[i]->drawtogglekeydown = 1;
                                }
                                if (Person::players[i]->isCrouch() && Person::players[i]->num_weapons && weapons[Person::players[i]->weaponids[0]].getType() == knife) {
                                    Person::players[i]->setAnimation(crouchdrawrightanim);
                                    Person::players[i]->drawtogglekeydown = 1;
                                }
                            }
                        }
                    }

                    //clean weapon
                    if (Person::players[i]->weaponactive != -1) {
                        if (Person::players[i]->isCrouch() &&
                                weapons[Person::players[i]->weaponids[Person::players[i]->weaponactive]].bloody &&
                                bloodtoggle &&
                                Person::players[i]->onterrain &&
                                Person::players[i]->num_weapons &&
                                Person::players[i]->attackkeydown &&
                                musictype != stream_fighttheme) {
                            if (weapons[Person::players[i]->weaponids[Person::players[i]->weaponactive]].getType() == knife)
                                Person::players[i]->setAnimation(crouchstabanim);
                            if (weapons[Person::players[i]->weaponids[Person::players[i]->weaponactive]].getType() == sword)
                                Person::players[i]->setAnimation(swordgroundstabanim);
                            Person::players[i]->hasvictim = 0;
                        }
                    }

                    if (!Person::players[i]->drawkeydown)
                        Person::players[i]->drawtogglekeydown = 0;

                    XYZ absflatfacing;
                    if (i == 0) {
                        absflatfacing = 0;
                        absflatfacing.z = -1;

                        absflatfacing = DoRotation(absflatfacing, 0, -yaw, 0);
                    } else
                        absflatfacing = flatfacing;

                    if (indialogue != -1) {
                        Person::players[i]->forwardkeydown = 0;
                        Person::players[i]->leftkeydown = 0;
                        Person::players[i]->backkeydown = 0;
                        Person::players[i]->rightkeydown = 0;
                        Person::players[i]->jumpkeydown = 0;
                        Person::players[i]->crouchkeydown = 0;
                        Person::players[i]->drawkeydown = 0;
                        Person::players[i]->throwkeydown = 0;
                    }
                    movekey = 0;
                    //Do controls
                    if (!animation[Person::players[i]->animTarget].attack &&
                            Person::players[i]->animTarget != staggerbackhighanim &&
                            Person::players[i]->animTarget != staggerbackhardanim &&
                            Person::players[i]->animTarget != backhandspringanim &&
                            Person::players[i]->animTarget != dodgebackanim) {
                        if (!Person::players[i]->forwardkeydown)
                            Person::players[i]->forwardstogglekeydown = 0;
                        if (Person::players[i]->crouchkeydown) {
                            //Crouch
                            target = -2;
                            if (i == 0) {
                                Person::players[i]->superruntoggle = 1;
                                if (Person::players.size() > 1)
                                    for (unsigned j = 0; j < Person::players.size(); j++)
                                        if (j != i && !Person::players[j]->skeleton.free && Person::players[j]->aitype == passivetype)
                                            if (distsq(&Person::players[j]->coords, &Person::players[i]->coords) < 16)
                                                Person::players[i]->superruntoggle = 0;
                            }

                            if (Person::players.size() > 1)
                                for (unsigned j = 0; j < Person::players.size(); j++) {
                                    if (j != i && !Person::players[j]->skeleton.free && Person::players[j]->victim && Person::players[i]->lowreversaldelay <= 0) {
                                        if (distsq(&Person::players[j]->coords, &Person::players[j]->victim->coords) < 3 &&
                                                Person::players[j]->victim == Person::players[i] &&
                                                (Person::players[j]->animTarget == sweepanim ||
                                                 Person::players[j]->animTarget == upunchanim ||
                                                 Person::players[j]->animTarget == wolfslapanim ||
                                                 ((Person::players[j]->animTarget == swordslashanim ||
                                                   Person::players[j]->animTarget == knifeslashstartanim ||
                                                   Person::players[j]->animTarget == staffhitanim ||
                                                   Person::players[j]->animTarget == staffspinhitanim) &&
                                                  distsq(&Person::players[j]->coords, &Person::players[i]->coords) < 2))) {
                                            if (target >= 0)
                                                target = -1;
                                            else
                                                target = j;
                                        }
                                    }
                                }
                            if (target >= 0)
                                Person::players[target]->Reverse();
                            Person::players[i]->lowreversaldelay = .5;

                            if (Person::players[i]->isIdle()) {
                                Person::players[i]->setAnimation(Person::players[i]->getCrouch());
                                Person::players[i]->transspeed = 10;
                            }
                            if (Person::players[i]->isRun() ||
                                    (Person::players[i]->isStop() &&
                                     (Person::players[i]->leftkeydown ||
                                      Person::players[i]->rightkeydown ||
                                      Person::players[i]->forwardkeydown ||
                                      Person::players[i]->backkeydown))) {
                                Person::players[i]->setAnimation(rollanim);
                                Person::players[i]->transspeed = 20;
                            }
                        }
                        if (!Person::players[i]->crouchkeydown) {
                            //Uncrouch
                            if (!Person::players[i]->isRun() && Person::players[i]->animTarget != sneakanim && i == 0)
                                Person::players[i]->superruntoggle = 0;
                            target = -2;
                            if (Person::players[i]->isCrouch()) {
                                if (Person::players.size() > 1)
                                    for (unsigned j = 0; j < Person::players.size(); j++) {
                                        if (j != i &&
                                                !Person::players[j]->skeleton.free &&
                                                Person::players[j]->victim &&
                                                Person::players[i]->highreversaldelay <= 0) {
                                            if (distsq(&Person::players[j]->coords, &Person::players[j]->victim->coords) < 3 &&
                                                    Person::players[j]->victim == Person::players[i] &&
                                                    (Person::players[j]->animTarget == spinkickanim) &&
                                                    Person::players[i]->isCrouch()) {
                                                if (target >= 0)
                                                    target = -1;
                                                else
                                                    target = j;
                                            }
                                        }
                                    }
                                if (target >= 0)
                                    Person::players[target]->Reverse();
                                Person::players[i]->highreversaldelay = .5;

                                if (Person::players[i]->isCrouch()) {
                                    if (!Person::players[i]->wasCrouch()) {
                                        Person::players[i]->animCurrent = Person::players[i]->getCrouch();
                                        Person::players[i]->frameCurrent = 0;
                                    }
                                    Person::players[i]->setAnimation(Person::players[i]->getIdle());
                                    Person::players[i]->transspeed = 10;
                                }
                            }
                            if (Person::players[i]->animTarget == sneakanim) {
                                Person::players[i]->setAnimation(Person::players[i]->getIdle());
                                Person::players[i]->transspeed = 10;
                            }
                        }
                        if (Person::players[i]->forwardkeydown) {
                            if (Person::players[i]->isIdle() ||
                                    (Person::players[i]->isStop() &&
                                     Person::players[i]->targetyaw == Person::players[i]->yaw) ||
                                    (Person::players[i]->isLanding() &&
                                     Person::players[i]->frameTarget > 0 &&
                                     !Person::players[i]->jumpkeydown) ||
                                    (Person::players[i]->isLandhard() &&
                                     Person::players[i]->frameTarget > 0 &&
                                     !Person::players[i]->jumpkeydown &&
                                     Person::players[i]->crouchkeydown)) {
                                if (Person::players[i]->aitype == passivetype)
                                    Person::players[i]->setAnimation(walkanim);
                                else
                                    Person::players[i]->setAnimation(Person::players[i]->getRun());
                            }
                            if (Person::players[i]->isCrouch()) {
                                Person::players[i]->animTarget = sneakanim;
                                if (Person::players[i]->wasCrouch())
                                    Person::players[i]->target = 0;
                                Person::players[i]->frameTarget = 0;
                            }
                            if (Person::players[i]->animTarget == hanganim/*&&(!Person::players[i]->forwardstogglekeydown||Person::players[i]->aitype!=playercontrolled)*/) {
                                Person::players[i]->setAnimation(climbanim);
                                Person::players[i]->frameTarget = 1;
                                Person::players[i]->jumpclimb = 1;
                            }
                            if (Person::players[i]->animTarget == jumpupanim || Person::players[i]->animTarget == jumpdownanim || Person::players[i]->isFlip()) {
                                Person::players[i]->velocity += absflatfacing * 5 * multiplier;
                            }
                            Person::players[i]->forwardstogglekeydown = 1;
                            movekey = 1;
                        }
                        if (Person::players[i]->rightkeydown) {
                            if (Person::players[i]->isIdle() ||
                                    (Person::players[i]->isStop() &&
                                     Person::players[i]->targetyaw == Person::players[i]->yaw) ||
                                    (Person::players[i]->isLanding() &&
                                     Person::players[i]->frameTarget > 0 &&
                                     !Person::players[i]->jumpkeydown) ||
                                    (Person::players[i]->isLandhard() &&
                                     Person::players[i]->frameTarget > 0 &&
                                     !Person::players[i]->jumpkeydown &&
                                     Person::players[i]->crouchkeydown)) {
                                Person::players[i]->setAnimation(Person::players[i]->getRun());
                            }
                            if (Person::players[i]->isCrouch()) {
                                Person::players[i]->animTarget = sneakanim;
                                if (Person::players[i]->wasCrouch())
                                    Person::players[i]->target = 0;
                                Person::players[i]->frameTarget = 0;
                            }
                            if (Person::players[i]->animTarget == jumpupanim || Person::players[i]->animTarget == jumpdownanim || Person::players[i]->isFlip()) {
                                Person::players[i]->velocity += DoRotation(absflatfacing * 5 * multiplier, 0, -90, 0);
                            }
                            Person::players[i]->targetyaw -= 90;
                            if (Person::players[i]->forwardkeydown)
                                Person::players[i]->targetyaw += 45;
                            if (Person::players[i]->backkeydown)
                                Person::players[i]->targetyaw -= 45;
                            movekey = 1;
                        }
                        if ( Person::players[i]->leftkeydown) {
                            if (Person::players[i]->isIdle() ||
                                    (Person::players[i]->isStop() &&
                                     Person::players[i]->targetyaw == Person::players[i]->yaw) ||
                                    (Person::players[i]->isLanding() &&
                                     Person::players[i]->frameTarget > 0 &&
                                     !Person::players[i]->jumpkeydown) ||
                                    (Person::players[i]->isLandhard() &&
                                     Person::players[i]->frameTarget > 0 &&
                                     !Person::players[i]->jumpkeydown &&
                                     Person::players[i]->crouchkeydown)) {
                                Person::players[i]->setAnimation(Person::players[i]->getRun());
                            }
                            if (Person::players[i]->isCrouch()) {
                                Person::players[i]->animTarget = sneakanim;
                                if (Person::players[i]->wasCrouch())
                                    Person::players[i]->target = 0;
                                Person::players[i]->frameTarget = 0;
                            }
                            if (Person::players[i]->animTarget == jumpupanim || Person::players[i]->animTarget == jumpdownanim || Person::players[i]->isFlip()) {
                                Person::players[i]->velocity -= DoRotation(absflatfacing * 5 * multiplier, 0, -90, 0);
                            }
                            Person::players[i]->targetyaw += 90;
                            if (Person::players[i]->forwardkeydown)
                                Person::players[i]->targetyaw -= 45;
                            if (Person::players[i]->backkeydown)
                                Person::players[i]->targetyaw += 45;
                            movekey = 1;
                        }
                        if (Person::players[i]->backkeydown) {
                            if (Person::players[i]->isIdle() ||
                                    (Person::players[i]->isStop() &&
                                     Person::players[i]->targetyaw == Person::players[i]->yaw) ||
                                    (Person::players[i]->isLanding() &&
                                     Person::players[i]->frameTarget > 0 &&
                                     !Person::players[i]->jumpkeydown) ||
                                    (Person::players[i]->isLandhard() &&
                                     Person::players[i]->frameTarget > 0 &&
                                     !Person::players[i]->jumpkeydown &&
                                     Person::players[i]->crouchkeydown)) {
                                Person::players[i]->setAnimation(Person::players[i]->getRun());
                            }
                            if (Person::players[i]->isCrouch()) {
                                Person::players[i]->animTarget = sneakanim;
                                if (Person::players[i]->wasCrouch())
                                    Person::players[i]->target = 0;
                                Person::players[i]->frameTarget = 0;
                            }
                            if (Person::players[i]->animTarget == jumpupanim || Person::players[i]->animTarget == jumpdownanim || Person::players[i]->isFlip()) {
                                Person::players[i]->velocity -= absflatfacing * 5 * multiplier;
                            }
                            if (Person::players[i]->animTarget == hanganim) {
                                Person::players[i]->animCurrent = jumpdownanim;
                                Person::players[i]->animTarget = jumpdownanim;
                                Person::players[i]->target = 0;
                                Person::players[i]->frameCurrent = 0;
                                Person::players[i]->frameTarget = 1;
                                Person::players[i]->velocity = 0;
                                Person::players[i]->velocity.y += gravity;
                                Person::players[i]->coords.y -= 1.4;
                                Person::players[i]->grabdelay = 1;
                            }
                            if ( !Person::players[i]->leftkeydown && !Person::players[i]->rightkeydown)
                                Person::players[i]->targetyaw += 180;
                            movekey = 1;
                        }
                        if ((Person::players[i]->jumpkeydown && !Person::players[i]->jumpclimb) || Person::players[i]->jumpstart) {
                            if ((((Person::players[i]->isLanding() && Person::players[i]->frameTarget >= 3) ||
                                    Person::players[i]->isRun() ||
                                    Person::players[i]->animTarget == walkanim ||
                                    Person::players[i]->isCrouch() ||
                                    Person::players[i]->animTarget == sneakanim) &&
                                    Person::players[i]->jumppower > 1) &&
                                    ((Person::players[i]->animTarget != rabbitrunninganim &&
                                      Person::players[i]->animTarget != wolfrunninganim) || i != 0)) {
                                Person::players[i]->jumpstart = 0;
                                Person::players[i]->setAnimation(jumpupanim);
                                Person::players[i]->yaw = Person::players[i]->targetyaw;
                                Person::players[i]->transspeed = 20;
                                Person::players[i]->FootLand(0, 1);
                                Person::players[i]->FootLand(1, 1);

                                facing = 0;
                                facing.z = -1;
                                flatfacing = DoRotation(facing, 0, Person::players[i]->targetyaw + 180, 0);

                                if (movekey)
                                    Person::players[i]->velocity = flatfacing * Person::players[i]->speed * 45 * Person::players[i]->scale;
                                if (!movekey)
                                    Person::players[i]->velocity = 0;

                                //Dodge sweep?
                                target = -2;
                                if (Person::players.size() > 1)
                                    for (unsigned j = 0; j < Person::players.size(); j++) {
                                        if (j != i && !Person::players[j]->skeleton.free && Person::players[j]->victim) {
                                            if (distsq(&Person::players[j]->coords, &Person::players[j]->victim->coords) < 3 &&
                                                    (Person::players[j]->victim == Person::players[i]) &&
                                                    (Person::players[j]->animTarget == sweepanim)) {
                                                if (target >= 0)
                                                    target = -1;
                                                else
                                                    target = j;
                                            }
                                        }
                                    }
                                if (target >= 0)
                                    Person::players[i]->velocity.y = 1;
                                else
                                    if (Person::players[i]->crouchkeydown || Person::players[i]->aitype != playercontrolled) {
                                    Person::players[i]->velocity.y = 7;
                                    Person::players[i]->crouchtogglekeydown = 1;
                                } else Person::players[i]->velocity.y = 5;

                                if (mousejump && i == 0 && debugmode) {
                                    if (!Person::players[i]->isLanding())
                                        Person::players[i]->tempdeltav = deltav;
                                    if (Person::players[i]->tempdeltav < 0)
                                        Person::players[i]->velocity.y -= (float)(Person::players[i]->tempdeltav) / multiplier / 1000;
                                }

                                Person::players[i]->coords.y += .2;
                                Person::players[i]->jumppower -= 1;

                                if (!i)
                                    emit_sound_at(whooshsound, Person::players[i]->coords, 128.);

                                emit_sound_at(jumpsound, Person::players[i]->coords, 128.);
                            }
                            if ((Person::players[i]->isIdle()) && Person::players[i]->jumppower > 1) {
                                Person::players[i]->setAnimation(Person::players[i]->getLanding());
                                Person::players[i]->frameTarget = 2;
                                Person::players[i]->landhard = 0;
                                Person::players[i]->jumpstart = 1;
                                Person::players[i]->tempdeltav = deltav;
                            }
                            if (Person::players[i]->animTarget == jumpupanim &&
                                    (((!floatjump &&
                                       !editorenabled) ||
                                      !debugmode) ||
                                     Person::players[i]->aitype != playercontrolled)) {
                                if (Person::players[i]->jumppower > multiplier * 6) {
                                    Person::players[i]->velocity.y += multiplier * 6;
                                    Person::players[i]->jumppower -= multiplier * 6;
                                }
                                if (Person::players[i]->jumppower <= multiplier * 6) {
                                    Person::players[i]->velocity.y += Person::players[i]->jumppower;
                                    Person::players[i]->jumppower = 0;
                                }
                            }
                            if (((floatjump || editorenabled) && debugmode) && i == 0)
                                Person::players[i]->velocity.y += multiplier * 30;
                        }

                        if (!movekey) {
                            if (Person::players[i]->isRun() || Person::players[i]->animTarget == walkanim)
                                Person::players[i]->setAnimation(Person::players[i]->getStop());
                            if (Person::players[i]->animTarget == sneakanim) {
                                Person::players[i]->animTarget = Person::players[i]->getCrouch();
                                if (Person::players[i]->animCurrent == sneakanim)
                                    Person::players[i]->target = 0;
                                Person::players[i]->frameTarget = 0;
                            }
                        }
                        if (Person::players[i]->animTarget == walkanim &&
                                (Person::players[i]->aitype == attacktypecutoff ||
                                 Person::players[i]->aitype == searchtype ||
                                 (Person::players[i]->aitype == passivetype &&
                                  Person::players[i]->numwaypoints <= 1)))
                            Person::players[i]->setAnimation(Person::players[i]->getStop());
                        if (Person::players[i]->isRun() && (Person::players[i]->aitype == passivetype))
                            Person::players[i]->setAnimation(Person::players[i]->getStop());
                    }
                }
                if (Person::players[i]->animTarget == rollanim)
                    Person::players[i]->targetyaw = oldtargetyaw;
            }

            //Rotation
            for (unsigned k = 0; k < Person::players.size(); k++) {
                if (fabs(Person::players[k]->yaw - Person::players[k]->targetyaw) > 180) {
                    if (Person::players[k]->yaw > Person::players[k]->targetyaw)
                        Person::players[k]->yaw -= 360;
                    else
                        Person::players[k]->yaw += 360;
                }

                //stop to turn in right direction
                if (fabs(Person::players[k]->yaw - Person::players[k]->targetyaw) > 90 && (Person::players[k]->isRun() || Person::players[k]->animTarget == walkanim))
                    Person::players[k]->setAnimation(Person::players[k]->getStop());

                if (Person::players[k]->animTarget == backhandspringanim || Person::players[k]->animTarget == dodgebackanim)
                    Person::players[k]->targettilt = 0;

                if (Person::players[k]->animTarget != jumpupanim &&
                        Person::players[k]->animTarget != backhandspringanim &&
                        Person::players[k]->animTarget != jumpdownanim &&
                        !Person::players[k]->isFlip()) {
                    Person::players[k]->targettilt = 0;
                    if (Person::players[k]->jumppower < 0 && !Person::players[k]->jumpkeydown)
                        Person::players[k]->jumppower = 0;
                    Person::players[k]->jumppower += multiplier * 7;
                    if (Person::players[k]->isCrouch())
                        Person::players[k]->jumppower += multiplier * 7;
                    if (Person::players[k]->jumppower > 5)
                        Person::players[k]->jumppower = 5;
                }

                if (Person::players[k]->isRun())
                    Person::players[k]->targettilt = (Person::players[k]->yaw - Person::players[k]->targetyaw) / 4;

                Person::players[k]->tilt = stepTowardf(Person::players[k]->tilt, Person::players[k]->targettilt, multiplier * 150);
                Person::players[k]->grabdelay -= multiplier;
            }

            //do animations
            for (unsigned k = 0; k < Person::players.size(); k++) {
                Person::players[k]->DoAnimations();
                Person::players[k]->whichpatchx = Person::players[k]->coords.x / (terrain.size / subdivision * terrain.scale);
                Person::players[k]->whichpatchz = Person::players[k]->coords.z / (terrain.size / subdivision * terrain.scale);
            }

            //do stuff
            objects.DoStuff();

            for (int j = numenvsounds - 1; j >= 0; j--) {
                envsoundlife[j] -= multiplier;
                if (envsoundlife[j] < 0) {
                    numenvsounds--;
                    envsoundlife[j] = envsoundlife[numenvsounds];
                    envsound[j] = envsound[numenvsounds];
                }
            }
            if (slomo)
                OPENAL_SetFrequency(OPENAL_ALL, slomofreq);
            else
                OPENAL_SetFrequency(OPENAL_ALL, 22050);

            if (tutoriallevel == 1) {
                XYZ temp;
                XYZ temp2;
                XYZ temp3;
                XYZ oldtemp;
                XYZ oldtemp2;
                temp.x = 1011;
                temp.y = 84;
                temp.z = 491;
                temp2.x = 1025;
                temp2.y = 75;
                temp2.z = 447;
                temp3.x = 1038;
                temp3.y = 76;
                temp3.z = 453;
                oldtemp = temp;
                oldtemp2 = temp2;
                if (tutorialstage >= 51)
                    if (distsq(&temp, &Person::players[0]->coords) >= distsq(&temp, &temp2) - 1 || distsq(&temp3, &Person::players[0]->coords) < 4) {
                        OPENAL_StopSound(OPENAL_ALL);  // hack...OpenAL renderer isn't stopping music after tutorial goes to level menu...
                        OPENAL_SetFrequency(OPENAL_ALL, 0.001);

                        emit_stream_np(stream_menutheme);

                        gameon = 0;
                        mainmenu = 5;

                        fireSound();

                        flash();
                    }
                if (tutorialstage < 51)
                    if (distsq(&temp, &Person::players[0]->coords) >= distsq(&temp, &temp2) - 1 || distsq(&temp3, &Person::players[0]->coords) < 4) {
                        emit_sound_at(fireendsound, Person::players[0]->coords);

                        Person::players[0]->coords = (oldtemp + oldtemp2) / 2;

                        flash();
                    }
                if (tutorialstage >= 14 && tutorialstage < 50)
                    if (distsq(&temp, &Person::players[1]->coords) >= distsq(&temp, &temp2) - 1 || distsq(&temp3, &Person::players[1]->coords) < 4) {
                        emit_sound_at(fireendsound, Person::players[1]->coords);

                        for (int i = 0; i < Person::players[1]->skeleton.num_joints; i++) {
                            if (Random() % 2 == 0) {
                                if (!Person::players[1]->skeleton.free)
                                    temp2 = (Person::players[1]->coords - Person::players[1]->oldcoords) / multiplier / 2; //velocity/2;
                                if (Person::players[1]->skeleton.free)
                                    temp2 = Person::players[1]->skeleton.joints[i].velocity * Person::players[1]->scale / 2;
                                if (!Person::players[1]->skeleton.free)
                                    temp = DoRotation(DoRotation(DoRotation(Person::players[1]->skeleton.joints[i].position, 0, 0, Person::players[1]->tilt), Person::players[1]->tilt2, 0, 0), 0, Person::players[1]->yaw, 0) * Person::players[1]->scale + Person::players[1]->coords;
                                if (Person::players[1]->skeleton.free)
                                    temp = Person::players[1]->skeleton.joints[i].position * Person::players[1]->scale + Person::players[1]->coords;
                                Sprite::MakeSprite(breathsprite, temp, temp2, 1, 1, 1, .6 + (float)abs(Random() % 100) / 200 - .25, 1);
                            }
                        }

                        Person::players[1]->coords = (oldtemp + oldtemp2) / 2;
                        for (int i = 0; i < Person::players[1]->skeleton.num_joints; i++) {
                            Person::players[1]->skeleton.joints[i].velocity = 0;
                            if (Random() % 2 == 0) {
                                if (!Person::players[1]->skeleton.free)
                                    temp2 = (Person::players[1]->coords - Person::players[1]->oldcoords) / multiplier / 2; //velocity/2;
                                if (Person::players[1]->skeleton.free)
                                    temp2 = Person::players[1]->skeleton.joints[i].velocity * Person::players[1]->scale / 2;
                                if (!Person::players[1]->skeleton.free)
                                    temp = DoRotation(DoRotation(DoRotation(Person::players[1]->skeleton.joints[i].position, 0, 0, Person::players[1]->tilt), Person::players[1]->tilt2, 0, 0), 0, Person::players[1]->yaw, 0) * Person::players[1]->scale + Person::players[1]->coords;
                                if (Person::players[1]->skeleton.free)
                                    temp = Person::players[1]->skeleton.joints[i].position * Person::players[1]->scale + Person::players[1]->coords;
                                Sprite::MakeSprite(breathsprite, temp, temp2, 1, 1, 1, .6 + (float)abs(Random() % 100) / 200 - .25, 1);
                            }
                        }
                    }
            }


            //3d sound
            static float gLoc[3];
            gLoc[0] = viewer.x;
            gLoc[1] = viewer.y;
            gLoc[2] = viewer.z;
            static float vel[3];
            vel[0] = (viewer.x - oldviewer.x) / multiplier;
            vel[1] = (viewer.y - oldviewer.y) / multiplier;
            vel[2] = (viewer.z - oldviewer.z) / multiplier;

            //Set orientation with forward and up vectors
            static XYZ upvector;
            upvector = 0;
            upvector.z = -1;

            upvector = DoRotation(upvector, -pitch + 90, 0, 0);
            upvector = DoRotation(upvector, 0, 0 - yaw, 0);

            facing = 0;
            facing.z = -1;

            facing = DoRotation(facing, -pitch, 0, 0);
            facing = DoRotation(facing, 0, 0 - yaw, 0);


            static float ori[6];
            ori[0] = -facing.x;
            ori[1] = facing.y;
            ori[2] = -facing.z;
            ori[3] = -upvector.x;
            ori[4] = upvector.y;
            ori[5] = -upvector.z;

            OPENAL_3D_Listener_SetAttributes(&gLoc[0], &vel[0], ori[0], ori[1], ori[2], ori[3], ori[4], ori[5]);
            OPENAL_Update();

            oldviewer = viewer;
        }
    }

    if (Input::isKeyPressed(SDL_SCANCODE_F1))
        Screenshot();
}

void Game::TickOnce()
{
    if (mainmenu)
        yaw += multiplier * 5;
    else if (directing || indialogue == -1) {
        yaw += deltah * .7;
        if (!invertmouse)
            pitch += deltav * .7;
        if (invertmouse)
            pitch -= deltav * .7;
        if (pitch > 90)
            pitch = 90;
        if (pitch < -70)
            pitch = -70;
    }
}

void Game::TickOnceAfter()
{
    static XYZ colviewer;
    static XYZ coltarget;
    static XYZ target;
    static XYZ col;
    static XYZ facing;
    static float changedelay;
    static bool alldead;
    static float unseendelay;
    static float cameraspeed;

    if (!mainmenu) {
        static int oldmusictype = musictype;

        if (environment == snowyenvironment)
            leveltheme = stream_snowtheme;
        if (environment == grassyenvironment)
            leveltheme = stream_grasstheme;
        if (environment == desertenvironment)
            leveltheme = stream_deserttheme;

        realthreat = 0;

        musictype = leveltheme;
        for (unsigned i = 0; i < Person::players.size(); i++) {
            if ((Person::players[i]->aitype == attacktypecutoff ||
                    Person::players[i]->aitype == getweapontype ||
                    Person::players[i]->aitype == gethelptype ||
                    Person::players[i]->aitype == searchtype) &&
                    !Person::players[i]->dead/*&&Person::players[i]->surprised<=0*/ &&
                    (Person::players[i]->animTarget != sneakattackedanim &&
                     Person::players[i]->animTarget != knifesneakattackedanim &&
                     Person::players[i]->animTarget != swordsneakattackedanim)) {
                musictype = stream_fighttheme;
                realthreat = 1;
            }
        }
        if (Person::players[0]->dead)
            musictype = stream_menutheme;


        if (musictype == stream_fighttheme)
            unseendelay = 1;

        if (oldmusictype == stream_fighttheme && musictype != stream_fighttheme) {
            unseendelay -= multiplier;
            if (unseendelay > 0)
                musictype = stream_fighttheme;
        }


        if (loading == 2) {
            musictype = stream_menutheme;
            musicvolume[2] = 512;
            musicvolume[0] = 0;
            musicvolume[1] = 0;
            musicvolume[3] = 0;
        }

        if (musictoggle)
            if (musictype != oldmusictype && musictype == stream_fighttheme)
                emit_sound_np(alarmsound);
        musicselected = musictype;

        if (musicselected == leveltheme)
            musicvolume[0] += multiplier * 450;
        else
            musicvolume[0] -= multiplier * 450;
        if (musicselected == stream_fighttheme)
            musicvolume[1] += multiplier * 450;
        else
            musicvolume[1] -= multiplier * 450;
        if (musicselected == stream_menutheme)
            musicvolume[2] += multiplier * 450;
        else
            musicvolume[2] -= multiplier * 450;

        for (int i = 0; i < 3; i++) {
            if (musicvolume[i] < 0)
                musicvolume[i] = 0;
            if (musicvolume[i] > 512)
                musicvolume[i] = 512;
        }

        if (musicvolume[2] > 128 && !loading && !mainmenu)
            musicvolume[2] = 128;

        if (musictoggle) {
            if (musicvolume[0] > 0 && oldmusicvolume[0] <= 0)
                emit_stream_np(leveltheme, musicvolume[0]);
            if (musicvolume[1] > 0 && oldmusicvolume[1] <= 0)
                emit_stream_np(stream_fighttheme, musicvolume[1]);
            if (musicvolume[2] > 0 && oldmusicvolume[2] <= 0)
                emit_stream_np(stream_menutheme, musicvolume[2]);
            if (musicvolume[0] <= 0 && oldmusicvolume[0] > 0)
                pause_sound(leveltheme);
            if (musicvolume[1] <= 0 && oldmusicvolume[1] > 0)
                pause_sound(stream_fighttheme);
            if (musicvolume[2] <= 0 && oldmusicvolume[2] > 0)
                pause_sound(stream_menutheme);

            if (musicvolume[0] != oldmusicvolume[0])
                OPENAL_SetVolume(channels[leveltheme], musicvolume[0]);
            if (musicvolume[1] != oldmusicvolume[1])
                OPENAL_SetVolume(channels[stream_fighttheme], musicvolume[1]);
            if (musicvolume[2] != oldmusicvolume[2])
                OPENAL_SetVolume(channels[stream_menutheme], musicvolume[2]);

            for (int i = 0; i < 3; i++)
                oldmusicvolume[i] = musicvolume[i];
        } else {
            pause_sound(leveltheme);
            pause_sound(stream_fighttheme);
            pause_sound(stream_menutheme);

            for (int i = 0; i < 4; i++) {
                oldmusicvolume[i] = 0;
                musicvolume[i] = 0;
            }
        }

        killhotspot = 2;
        for (int i = 0; i < numhotspots; i++) {
            if (hotspottype[i] > 10 && hotspottype[i] < 20) {
                if (Person::players[hotspottype[i] - 10]->dead == 0)
                    killhotspot = 0;
                else if (killhotspot == 2)
                    killhotspot = 1;
            }
        }
        if (killhotspot == 2)
            killhotspot = 0;


        winhotspot = false;
        for (int i = 0; i < numhotspots; i++)
            if (hotspottype[i] == -1)
                if (distsq(&Person::players[0]->coords, &hotspot[i]) < hotspotsize[i])
                    winhotspot = true;

        int numalarmed = 0;
        for (unsigned i = 1; i < Person::players.size(); i++)
            if (!Person::players[i]->dead && Person::players[i]->aitype == attacktypecutoff && Person::players[i]->surprised <= 0)
                numalarmed++;
        if (numalarmed > maxalarmed)
            maxalarmed = numalarmed;

        if (changedelay <= 0 && !loading && !editorenabled && gameon && !tutoriallevel && changedelay != -999 && !won) {
            if (Person::players[0]->dead && changedelay <= 0) {
                changedelay = 1;
                targetlevel = whichlevel;
            }
            alldead = true;
            for (unsigned i = 1; i < Person::players.size(); i++) {
                if (!Person::players[i]->dead && Person::players[i]->howactive < typedead1) {
                    alldead = false;
                    break;
                }
            }


            if (alldead && !Person::players[0]->dead && maptype == mapkilleveryone) {
                changedelay = 1;
                targetlevel = whichlevel + 1;
                if (targetlevel > numchallengelevels - 1)
                    targetlevel = 0;
            }
            if (winhotspot || windialogue) {
                changedelay = 0.1;
                targetlevel = whichlevel + 1;
                if (targetlevel > numchallengelevels - 1)
                    targetlevel = 0;
            }


            if (killhotspot) {
                changedelay = 1;
                targetlevel = whichlevel + 1;
                if (targetlevel > numchallengelevels - 1)
                    targetlevel = 0;
            }

            if (changedelay > 0 && !Person::players[0]->dead && !won) {
                //high scores, awards, win
                if (campaign) {
                    accountactive->winCampaignLevel(whichchoice, bonustotal, leveltime);
                    scoreadded = 1;
                } else {
                    accountactive->winLevel(whichlevel, bonustotal - startbonustotal, leveltime);
                }
                won = 1;
            }
        }

        if (!winfreeze) {

            if (leveltime < 1) {
                loading = 0;
                changedelay = .1;
                alldead = false;
                winhotspot = false;
                killhotspot = 0;
            }

            if (!editorenabled && gameon && !mainmenu) {
                if (changedelay != -999)
                    changedelay -= multiplier / 7;
                if (Person::players[0]->dead)
                    targetlevel = whichlevel;
                if (loading == 2 && !campaign) {
                    flash();

                    fireSound(firestartsound);

                    if (!Person::players[0]->dead && targetlevel != whichlevel)
                        startbonustotal = bonustotal;
                    if (Person::players[0]->dead)
                        Loadlevel(whichlevel);
                    else
                        Loadlevel(targetlevel);

                    fireSound();

                    loading = 3;
                }
                if (loading == 2 && targetlevel == whichlevel) {
                    flash();
                    loadtime = 0;

                    fireSound(firestartsound);

                    Loadlevel(campaignlevels[accountactive->getCampaignChoicesMade()].mapname.c_str());

                    fireSound();

                    loading = 3;
                }
                if (changedelay <= -999 &&
                        whichlevel != -2 &&
                        !loading &&
                        (Person::players[0]->dead ||
                         (alldead && maptype == mapkilleveryone) ||
                         (winhotspot) ||
                         (killhotspot)))
                    loading = 1;
                if ((Person::players[0]->dead ||
                        (alldead && maptype == mapkilleveryone) ||
                        (winhotspot) ||
                        (windialogue) ||
                        (killhotspot)) &&
                        changedelay <= 0) {
                    if (whichlevel != -2 && !loading && !Person::players[0]->dead) {
                        winfreeze = true;
                        changedelay = -999;
                    }
                    if (Person::players[0]->dead)
                        loading = 1;
                }
            }

            if (campaign) {
                // campaignchoosenext determines what to do when the level is complete:
                // 0 = load next level
                // 1 = go back to level select screen
                // 2 = stealthload next level
                if (mainmenu == 0 && winfreeze && (campaignlevels[actuallevel].choosenext) == 1) {
                    if (campaignlevels[actuallevel].nextlevel.empty())
                        endgame = 1;
                } else if (mainmenu == 0 && winfreeze) {
                    stealthloading = (campaignlevels[actuallevel].choosenext == 2);

                    if (!stealthloading) {
                        fireSound(firestartsound);

                        flash();
                    }

                    startbonustotal = 0;

                    LoadCampaign();

                    loading = 2;
                    loadtime = 0;
                    targetlevel = 7;
                    if (!firstload)
                        LoadStuff();
                    whichchoice = 0;
                    actuallevel = campaignlevels[actuallevel].nextlevel.front();
                    visibleloading = 1;
                    stillloading = 1;
                    Loadlevel(campaignlevels[actuallevel].mapname.c_str());
                    campaign = 1;
                    mainmenu = 0;
                    gameon = 1;
                    pause_sound(stream_menutheme);

                    stealthloading = 0;
                }
            }

            if (loading == 3)
                loading = 0;

        }

        oldmusictype = musictype;
    }

    facing = 0;
    facing.z = -1;

    facing = DoRotation(facing, -pitch, 0, 0);
    facing = DoRotation(facing, 0, 0 - yaw, 0);
    viewerfacing = facing;

    if (!cameramode) {
        if ((animation[Person::players[0]->animTarget].attack != 3 && animation[Person::players[0]->animCurrent].attack != 3) || Person::players[0]->skeleton.free)
            target = Person::players[0]->coords + Person::players[0]->currentoffset * (1 - Person::players[0]->target) * Person::players[0]->scale + Person::players[0]->targetoffset * Person::players[0]->target * Person::players[0]->scale - Person::players[0]->facing * .05;
        else
            target = Person::players[0]->oldcoords + Person::players[0]->currentoffset * (1 - Person::players[0]->target) * Person::players[0]->scale + Person::players[0]->targetoffset * Person::players[0]->target * Person::players[0]->scale - Person::players[0]->facing * .05;
        target.y += .1;
        if (Person::players[0]->skeleton.free) {
            for (int i = 0; i < Person::players[0]->skeleton.num_joints; i++) {
                if (Person::players[0]->skeleton.joints[i].position.y * Person::players[0]->scale + Person::players[0]->coords.y > target.y)
                    target.y = Person::players[0]->skeleton.joints[i].position.y * Person::players[0]->scale + Person::players[0]->coords.y;
            }
            target.y += .1;
        }
        if (Person::players[0]->skeleton.free != 2) {
            cameraspeed = 20;
            if (findLengthfast(&Person::players[0]->velocity) > 400) {
                cameraspeed = 20 + (findLength(&Person::players[0]->velocity) - 20) * .96;
            }
            if (Person::players[0]->skeleton.free == 0 && Person::players[0]->animTarget != hanganim && Person::players[0]->animTarget != climbanim)
                target.y += 1.4;
            coltarget = target - cameraloc;
            if (findLengthfast(&coltarget) < multiplier * multiplier * 400)
                cameraloc = target;
            else {
                Normalise(&coltarget);
                if (Person::players[0]->animTarget != hanganim && Person::players[0]->animTarget != climbanim && Person::players[0]->animCurrent != climbanim && Person::players[0]->currentoffset.x == 0)
                    cameraloc = cameraloc + coltarget * multiplier * cameraspeed;
                else
                    cameraloc = cameraloc + coltarget * multiplier * 8;
            }
            if (editorenabled)
                cameraloc = target;
            cameradist += multiplier * 5;
            if (cameradist > 2.3)
                cameradist = 2.3;
            viewer = cameraloc - facing * cameradist;
            colviewer = viewer;
            coltarget = cameraloc;
            objects.SphereCheckPossible(&colviewer, findDistance(&colviewer, &coltarget));
            if (terrain.patchobjectnum[Person::players[0]->whichpatchx][Person::players[0]->whichpatchz])
                for (int j = 0; j < terrain.patchobjectnum[Person::players[0]->whichpatchx][Person::players[0]->whichpatchz]; j++) {
                    int i = terrain.patchobjects[Person::players[0]->whichpatchx][Person::players[0]->whichpatchz][j];
                    colviewer = viewer;
                    coltarget = cameraloc;
                    if (objects.model[i].LineCheckPossible(&colviewer, &coltarget, &col, &objects.position[i], &objects.yaw[i]) != -1)
                        viewer = col;
                }
            if (terrain.patchobjectnum[Person::players[0]->whichpatchx][Person::players[0]->whichpatchz])
                for (int j = 0; j < terrain.patchobjectnum[Person::players[0]->whichpatchx][Person::players[0]->whichpatchz]; j++) {
                    int i = terrain.patchobjects[Person::players[0]->whichpatchx][Person::players[0]->whichpatchz][j];
                    colviewer = viewer;
                    if (objects.model[i].SphereCheck(&colviewer, .15, &col, &objects.position[i], &objects.yaw[i]) != -1) {
                        viewer = colviewer;
                    }
                }
            cameradist = findDistance(&viewer, &target);
            viewer.y = max((double)viewer.y, terrain.getHeight(viewer.x, viewer.z) + .6);
            if (cameraloc.y < terrain.getHeight(cameraloc.x, cameraloc.z)) {
                cameraloc.y = terrain.getHeight(cameraloc.x, cameraloc.z);
            }
        }
        if (camerashake > .8)
            camerashake = .8;
        woozy += multiplier;
        if (Person::players[0]->dead)
            camerashake = 0;
        if (Person::players[0]->dead)
            woozy = 0;
        camerashake -= multiplier * 2;
        blackout -= multiplier * 2;
        if (camerashake < 0)
            camerashake = 0;
        if (blackout < 0)
            blackout = 0;
        if (camerashake) {
            viewer.x += (float)(Random() % 100) * .0005 * camerashake;
            viewer.y += (float)(Random() % 100) * .0005 * camerashake;
            viewer.z += (float)(Random() % 100) * .0005 * camerashake;
        }
    }
}

