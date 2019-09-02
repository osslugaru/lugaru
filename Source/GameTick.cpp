/*
Copyright (C) 2003, 2010 - Wolfire Games
Copyright (C) 2010-2017 - Lugaru contributors (see AUTHORS file)

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

#include "Game.hpp"

#include "Animation/Animation.hpp"
#include "Audio/openal_wrapper.hpp"
#include "Devtools/ConsoleCmds.hpp"
#include "Level/Awards.hpp"
#include "Level/Campaign.hpp"
#include "Level/Dialog.hpp"
#include "Level/Hotspot.hpp"
#include "Menu/Menu.hpp"
#include "Tutorial.hpp"
#include "User/Settings.hpp"
#include "Utils/Folders.hpp"
#include "Utils/Input.hpp"

#if PLATFORM_UNIX
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>
#else
#include <direct.h>
#endif

#include <algorithm>
#include <cmath>
#include <ctime>
#include <dirent.h>
#include <limits>
#include <set>
#include <json/reader.h>

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
extern bool devtools;
int leveltheme;
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
extern bool decalstoggle;
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
extern float smoketex;
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

extern int hostile;

extern bool stillloading;
extern bool winfreeze;

extern bool campaign;

extern void toggleFullscreen();

bool won = false;
int whichchoice = 0;
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

#define STATIC_ASSERT(x) extern int s_a_dummy[2 * (!!(x)) - 1];
STATIC_ASSERT(rabbittype == 0 && wolftype == 1)

// utility functions

// TODO: this is slightly incorrect
float roughDirection(XYZ vec)
{
    Normalise(&vec);
    float angle = -asin(-vec.x) * 180 / M_PI;
    if (vec.z < 0) {
        angle = 180 - angle;
    }
    return angle;
}
float roughDirectionTo(XYZ start, XYZ end)
{
    return roughDirection(end - start);
}
inline float pitchOf(XYZ vec)
{
    Normalise(&vec);
    return -asin(vec.y) * 180 / M_PI;
}
float pitchTo(XYZ start, XYZ end)
{
    return pitchOf(end - start);
}
float sq(float n)
{
    return n * n;
}
inline float stepTowardf(float from, float to, float by)
{
    if (fabs(from - to) < by) {
        return to;
    } else if (from > to) {
        return from - by;
    } else {
        return from + by;
    }
}

void Game::playdialoguescenesound()
{
    XYZ temppos;
    temppos = Person::players.at(Dialog::currentScene().participantfocus)->coords;
    temppos = temppos - viewer;
    Normalise(&temppos);
    temppos += viewer;

    int sound = -1;
    switch (Dialog::currentScene().sound) {
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
    if (sound != -1) {
        emit_sound_at(sound, temppos);
    }
}

// ================================================================

int Game::findClosestPlayer()
{
    int closest = -1;
    float closestdist = std::numeric_limits<float>::max();

    for (unsigned int i = 1; i < Person::players.size(); i++) {
        float distance = distsq(&Person::players[i]->coords, &Person::players[0]->coords);
        if (distance < closestdist) {
            closestdist = distance;
            closest = (int)i;
        }
    }
    return closest;
}

static int findClosestObject()
{
    int closest = -1;
    float closestdist = std::numeric_limits<float>::max();

    for (unsigned int i = 0; i < Object::objects.size(); i++) {
        float distance = distsq(&Object::objects[i]->position,
                                &Person::players[0]->coords);
        if (distance < closestdist) {
            closestdist = distance;
            closest = (int)i;
        }
    }
    return closest;
}

void Game::cmd_dispatch(const string cmd)
{
    int i, n_cmds = sizeof(cmd_names) / sizeof(cmd_names[0]);

    for (i = 0; i < n_cmds; i++) {
        if (cmd.substr(0, cmd.find(' ')) == string(cmd_names[i])) {
            cout << "|" << cmd.substr(cmd.find(' ') + 1) << "|" << endl;
            cmd_handlers[i](cmd.substr(cmd.find(' ') + 1).c_str());
            break;
        }
    }
    emit_sound_np(i < n_cmds ? consolesuccesssound : consolefailsound);
}

/********************> Tick() <*****/
extern bool save_screenshot(const char* fname);
void Screenshot(void)
{
    char filename[1024];
    time_t t = time(NULL);
    struct tm* tme = localtime(&t);
    sprintf(filename, "Screenshot-%04d%02d%02d-%02d%02d%02d.png",
            tme->tm_year + 1900, tme->tm_mon + 1, tme->tm_mday, tme->tm_hour, tme->tm_min, tme->tm_sec);

    save_screenshot(filename);
}

void Game::SetUpLighting()
{
    if (environment == snowyenvironment) {
        light.setColors(.65, .65, .7, .4, .4, .44);
    }
    if (environment == desertenvironment) {
        light.setColors(.95, .95, .95, .4, .35, .3);
    }
    if (environment == grassyenvironment) {
        light.setColors(.95, .95, 1, .4, .4, .44);
    }
    if (!skyboxtexture) {
        light.setColors(1, 1, 1, .4, .4, .4);
    }
    float average;
    average = (skyboxlightr + skyboxlightg + skyboxlightb) / 3;
    light.color[0] *= (skyboxlightr + average) / 2;
    light.color[1] *= (skyboxlightg + average) / 2;
    light.color[2] *= (skyboxlightb + average) / 2;
    light.ambient[0] *= (skyboxlightr + average) / 2;
    light.ambient[1] *= (skyboxlightg + average) / 2;
    light.ambient[2] *= (skyboxlightb + average) / 2;
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
        if (ambientsound) {
            emit_stream_np(stream_wind);
        }

        Object::treetextureptr.load("Textures/SnowTree.png", 0);
        Object::bushtextureptr.load("Textures/BushSnow.png", 0);
        Object::rocktextureptr.load("Textures/BoulderSnow.jpg", 1);
        Object::boxtextureptr.load("Textures/SnowBox.jpg", 1);

        footstepsound = footstepsn1;
        footstepsound2 = footstepsn2;
        footstepsound3 = footstepst1;
        footstepsound4 = footstepst2;

        terraintexture.load("Textures/Snow.jpg", 1);
        terraintexture2.load("Textures/Rock.jpg", 1);

        temptexdetail = texdetail;
        if (texdetail > 1) {
            texdetail = 4;
        }
        skybox->load("Textures/Skybox(snow)/Front.jpg",
                     "Textures/Skybox(snow)/Left.jpg",
                     "Textures/Skybox(snow)/Back.jpg",
                     "Textures/Skybox(snow)/Right.jpg",
                     "Textures/Skybox(snow)/Up.jpg",
                     "Textures/Skybox(snow)/Down.jpg");

        texdetail = temptexdetail;
    } else if (environment == desertenvironment) {
        windvector = 0;
        windvector.z = 2;
        Object::treetextureptr.load("Textures/DesertTree.png", 0);
        Object::bushtextureptr.load("Textures/BushDesert.png", 0);
        Object::rocktextureptr.load("Textures/BoulderDesert.jpg", 1);
        Object::boxtextureptr.load("Textures/DesertBox.jpg", 1);

        if (ambientsound) {
            emit_stream_np(stream_desertambient);
        }

        footstepsound = footstepsn1;
        footstepsound2 = footstepsn2;
        footstepsound3 = footstepsn1;
        footstepsound4 = footstepsn2;

        terraintexture.load("Textures/Sand.jpg", 1);
        terraintexture2.load("Textures/SandSlope.jpg", 1);

        temptexdetail = texdetail;
        if (texdetail > 1) {
            texdetail = 4;
        }
        skybox->load("Textures/Skybox(sand)/Front.jpg",
                     "Textures/Skybox(sand)/Left.jpg",
                     "Textures/Skybox(sand)/Back.jpg",
                     "Textures/Skybox(sand)/Right.jpg",
                     "Textures/Skybox(sand)/Up.jpg",
                     "Textures/Skybox(sand)/Down.jpg");

        texdetail = temptexdetail;
    } else if (environment == grassyenvironment) {
        windvector = 0;
        windvector.z = 2;
        Object::treetextureptr.load("Textures/Tree.png", 0);
        Object::bushtextureptr.load("Textures/Bush.png", 0);
        Object::rocktextureptr.load("Textures/Boulder.jpg", 1);
        Object::boxtextureptr.load("Textures/GrassBox.jpg", 1);

        if (ambientsound) {
            emit_stream_np(stream_wind, 100.);
        }

        footstepsound = footstepgr1;
        footstepsound2 = footstepgr2;
        footstepsound3 = footstepst1;
        footstepsound4 = footstepst2;

        terraintexture.load("Textures/GrassDirt.jpg", 1);
        terraintexture2.load("Textures/MossRock.jpg", 1);

        temptexdetail = texdetail;
        if (texdetail > 1) {
            texdetail = 4;
        }
        skybox->load("Textures/Skybox(grass)/Front.jpg",
                     "Textures/Skybox(grass)/Left.jpg",
                     "Textures/Skybox(grass)/Back.jpg",
                     "Textures/Skybox(grass)/Right.jpg",
                     "Textures/Skybox(grass)/Up.jpg",
                     "Textures/Skybox(grass)/Down.jpg");

        texdetail = temptexdetail;
    }
    temptexdetail = texdetail;
    texdetail = 1;
    terrain.load("Textures/HeightMap.png");

    texdetail = temptexdetail;
}

bool Game::LoadLevel(int which)
{
    stealthloading = 0;
    whichlevel = which;

    if (which == -1) {
        return LoadLevel("tutorial", true);
    } else if (which >= 0 && which <= 15) {
        char buf[32];
        snprintf(buf, 32, "map%d", which + 1); // challenges
        return LoadLevel(buf);
    } else {
        return LoadLevel("mapsave");
    }
}

void Game::ResetBeforeLevelLoad(bool tutorial)
{
    Tutorial::active = tutorial;

    if (Tutorial::active) {
        Tutorial::stage = 0;
    }
    if (Tutorial::stage == 0) {
        Tutorial::stagetime = 0;
        Tutorial::maxtime = 1;
    }

    scoreadded = 0;
    windialogue = false;
    hostiletime = 0;
    won = 0;

    Dialog::dialogs.clear();

    Dialog::indialogue = -1;
    cameramode = 0;

    damagedealt = 0;
    damagetaken = 0;

    if (Account::hasActive()) {
        difficulty = Account::active().getDifficulty();
    }

    Hotspot::hotspots.clear();
    Hotspot::current = -1;
    bonustime = 1;

    skyboxtexture = 1;
    skyboxr = 1;
    skyboxg = 1;
    skyboxb = 1;

    freeze = 0;
    winfreeze = 0;

    for (unsigned char i = 0; i < 100; i++) {
        bonusnum[i] = 0;
    }

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
}

bool Game::LoadLevel(const std::string& name, bool tutorial)
{
    if (LoadJsonLevel(name, tutorial)) {
        // Try JSON loading first, binary is fallback
        return true;
    }
    const std::string level_path = Folders::getResourcePath("Maps/" + name);
    if (!Folders::file_exists(level_path)) {
        perror(std::string("LoadLevel: Could not open file '" + level_path).c_str());
        return false;
    }

    int indemo; // FIXME this should be removed
    int templength;
    float lamefloat;

    LOGFUNC;

    LOG(std::string("Loading level...") + name);

    if (!gameon) {
        visibleloading = true;
    }
    if (stealthloading) {
        visibleloading = false;
    }
    if (!stillloading) {
        loadtime = 0;
    }
    gamestarted = 1;

    numenvsounds = 0;

    pause_sound(whooshsound);
    pause_sound(stream_firesound);

    int mapvers;
    FILE* tfile;
    errno = 0;
    tfile = Folders::openMandatoryFile(level_path, "rb");

    ResetBeforeLevelLoad(tutorial);

    if (console) {
        emit_sound_np(consolesuccesssound);
        freeze = 0;
        console = false;
    }

    if (!stealthloading) {
        terrain.decals.clear();
        Sprite::deleteSprites();

        for (int i = 0; i < subdivision; i++) {
            for (int j = 0; j < subdivision; j++) {
                terrain.patchobjects[i][j].clear();
            }
        }
        Game::LoadingScreen();
    }

    weapons.clear();
    Person::players.resize(1);

    funpackf(tfile, "Bi", &mapvers);
    if (mapvers < 12) {
        cerr << name << " has obsolete map version " << mapvers << endl;
    }
    if (mapvers >= 15) {
        funpackf(tfile, "Bi", &indemo);
    } else {
        indemo = 0;
    }
    if (mapvers >= 5) {
        funpackf(tfile, "Bi", &maptype);
    } else {
        maptype = mapkilleveryone;
    }
    if (mapvers >= 6) {
        funpackf(tfile, "Bi", &hostile);
    } else {
        hostile = 1;
    }
    if (mapvers >= 4) {
        funpackf(tfile, "Bf Bf", &viewdistance, &fadestart);
    } else {
        viewdistance = 100;
        fadestart = .6;
    }
    if (mapvers >= 2) {
        funpackf(tfile, "Bb Bf Bf Bf", &skyboxtexture, &skyboxr, &skyboxg, &skyboxb);
    } else {
        skyboxtexture = 1;
        skyboxr = 1;
        skyboxg = 1;
        skyboxb = 1;
    }
    if (mapvers >= 10) {
        funpackf(tfile, "Bf Bf Bf", &skyboxlightr, &skyboxlightg, &skyboxlightb);
    } else {
        skyboxlightr = skyboxr;
        skyboxlightg = skyboxg;
        skyboxlightb = skyboxb;
    }
    /* TODO - This should be done in an other way so that we can rebuild main player as well (so coords would need to be copied from old ones after rebuilding) */
    if (stealthloading) {
        funpackf(tfile, "Bf Bf Bf Bf Bf Bi", &lamefloat, &lamefloat, &lamefloat, &lamefloat, &lamefloat, &Person::players[0]->num_weapons);
    } else {
        funpackf(tfile, "Bf Bf Bf Bf Bf Bi", &Person::players[0]->coords.x, &Person::players[0]->coords.y, &Person::players[0]->coords.z, &Person::players[0]->yaw, &Person::players[0]->targetyaw, &Person::players[0]->num_weapons);
    }
    if (Person::players[0]->num_weapons > 0 && Person::players[0]->num_weapons < 5) {
        for (int j = 0; j < Person::players[0]->num_weapons; j++) {
            Person::players[0]->weaponids[j] = weapons.size();
            int type;
            funpackf(tfile, "Bi", &type);
            weapons.push_back(Weapon(type, 0));
        }
    }

    Game::LoadingScreen();

    funpackf(tfile, "Bf Bf Bf", &Person::players[0]->armorhead, &Person::players[0]->armorhigh, &Person::players[0]->armorlow);
    funpackf(tfile, "Bf Bf Bf", &Person::players[0]->protectionhead, &Person::players[0]->protectionhigh, &Person::players[0]->protectionlow);
    funpackf(tfile, "Bf Bf Bf", &Person::players[0]->metalhead, &Person::players[0]->metalhigh, &Person::players[0]->metallow);
    funpackf(tfile, "Bf Bf", &Person::players[0]->power, &Person::players[0]->speedmult);

    int numclothes;
    float tintr, tintg, tintb;
    funpackf(tfile, "Bi", &numclothes);

    if (mapvers >= 9) {
        funpackf(tfile, "Bi Bi", &Person::players[0]->whichskin, &Person::players[0]->creature);
    } else {
        Person::players[0]->whichskin = 0;
        Person::players[0]->creature = rabbittype;
    }

    Person::players[0]->lastattack = -1;
    Person::players[0]->lastattack2 = -1;
    Person::players[0]->lastattack3 = -1;

    //dialogues
    if (mapvers >= 8) {
        Dialog::loadDialogs(tfile);
    }

    Person::players[0]->clothes.clear();
    Person::players[0]->clothestintr.clear();
    Person::players[0]->clothestintg.clear();
    Person::players[0]->clothestintb.clear();
    for (int k = 0; k < numclothes; k++) {
        char clothespath[256];
        funpackf(tfile, "Bi", &templength);
        for (int l = 0; l < templength; l++) {
            funpackf(tfile, "Bb", &clothespath[l]);
        }
        clothespath[templength] = '\0';
        Person::players[0]->clothes.push_back(std::string(clothespath));
        funpackf(tfile, "Bf Bf Bf", &tintr, &tintg, &tintb);
        Person::players[0]->clothestintr.push_back(tintr);
        Person::players[0]->clothestintg.push_back(tintg);
        Person::players[0]->clothestintb.push_back(tintb);
    }

    funpackf(tfile, "Bi", &environment);

    if (environment != oldenvironment) {
        Setenvironment(environment);
    }
    oldenvironment = environment;

    Object::LoadObjectsFromFile(tfile, stealthloading);

    if (mapvers >= 7) {
        int numhotspots;
        funpackf(tfile, "Bi", &numhotspots);
        if (numhotspots < 0) {
            cerr << "Map " << name << " has an invalid number of hotspots" << endl;
            numhotspots = 0;
        }
        Hotspot::hotspots.resize(numhotspots);
        for (unsigned i = 0; i < Hotspot::hotspots.size(); i++) {
            funpackf(tfile, "Bi Bf Bf Bf Bf", &Hotspot::hotspots[i].type, &Hotspot::hotspots[i].size, &Hotspot::hotspots[i].position.x, &Hotspot::hotspots[i].position.y, &Hotspot::hotspots[i].position.z);
            char temptext[256];
            funpackf(tfile, "Bi", &templength);
            if (templength) {
                for (int l = 0; l < templength; l++) {
                    funpackf(tfile, "Bb", &temptext[l]);
                }
            }
            temptext[templength] = '\0';
            Hotspot::hotspots[i].text = std::string(temptext);
            if (Hotspot::hotspots[i].type == -111) {
                indemo = 1;
            }
        }
    } else {
        Hotspot::hotspots.clear();
    }

    Game::LoadingScreen();

    if (!stealthloading) {
        Object::ComputeCenter();
        Object::ComputeRadius();
    }

    Game::LoadingScreen();

    int numplayers;
    funpackf(tfile, "Bi", &numplayers);
    if (numplayers > maxplayers) {
        cout << "Warning: this level contains more players than allowed" << endl;
    }
    unsigned j = 1;
    for (int i = 1; i < numplayers; i++) {
        try {
            Person::players.push_back(shared_ptr<Person>(new Person(tfile, mapvers, j)));
            j++;
        } catch (InvalidPersonException &e) {
            cerr << "Invalid Person found in " << name << endl;
        }
    }
    Game::LoadingScreen();

    funpackf(tfile, "Bi", &numpathpoints);
    if (numpathpoints > 30 || numpathpoints < 0) {
        numpathpoints = 0;
    }
    for (int j = 0; j < numpathpoints; j++) {
        funpackf(tfile, "Bf Bf Bf Bi", &pathpoint[j].x, &pathpoint[j].y, &pathpoint[j].z, &numpathpointconnect[j]);
        for (int k = 0; k < numpathpointconnect[j]; k++) {
            funpackf(tfile, "Bi", &pathpointconnect[j][k]);
        }
    }
    Game::LoadingScreen();

    funpackf(tfile, "Bf Bf Bf Bf", &mapcenter.x, &mapcenter.y, &mapcenter.z, &mapradius);

    SetUpLighting();

    if (!stealthloading) {
        Object::AddObjectsToTerrain();
        terrain.DoShadows();
        Game::LoadingScreen();
        Object::DoShadows();
        Game::LoadingScreen();
    }

    fclose(tfile);

    for (unsigned i = 0; i < Person::players.size(); i++) {
        Game::LoadingScreen();
        if (i == 0) {
            Person::players[i]->burnt = 0;
            Person::players[i]->bled = 0;
            Person::players[i]->onfire = 0;
            Person::players[i]->scale = .2;
            if (mapvers < 9) {
                Person::players[i]->creature = rabbittype;
            }
        }
        Person::players[i]->skeleton.free = 0;

        Person::players[i]->skeletonLoad();

        Person::players[i]->addClothes();

        if (i == 0) {
            Person::players[i]->animCurrent = bounceidleanim;
            Person::players[i]->animTarget = bounceidleanim;
            Person::players[i]->frameCurrent = 0;
            Person::players[i]->frameTarget = 1;
            Person::players[i]->target = 0;
        }
        Person::players[i]->speed = 1 + (float)(Random() % 100) / 1000;
        if (difficulty == 0) {
            Person::players[i]->speed -= .2;
        }
        if (difficulty == 1) {
            Person::players[i]->speed -= .1;
        }

        if (i == 0) {
            Person::players[i]->velocity = 0;
            Person::players[i]->oldcoords = Person::players[i]->coords;
            Person::players[i]->realoldcoords = Person::players[i]->coords;

            Person::players[i]->id = i;
            Person::players[i]->updatedelay = 0;
            Person::players[i]->normalsupdatedelay = 0;

            Person::players[i]->setProportions(1, 1, 1, 1);
            Person::players[i]->headless = 0;
            Person::players[i]->currentoffset = 0;
            Person::players[i]->targetoffset = 0;
            if (Person::players[i]->creature == wolftype) {
                Person::players[i]->scale = .23;
                Person::players[i]->damagetolerance = 300;
            } else {
                Person::players[i]->damagetolerance = 200;
            }

            Game::LoadingScreen();

            Person::players[i]->tempanimation = Animation("Tempanim", lowheight, neutral);

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
    }

    Person::players[0]->aitype = playercontrolled;

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

    Game::LoadingScreen();

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

    leveltime = 0;
    wonleveltime = 0;
    visibleloading = false;

    return true;
}

bool Game::LoadJsonLevel(const std::string& name, bool tutorial)
{
    const std::string level_path = Folders::getResourcePath("Maps/" + name + ".json");
    if (!Folders::file_exists(level_path)) {
        perror(std::string("LoadLevel: Could not open file '" + level_path).c_str());
        return false;
    }

    LOGFUNC;

    LOG(std::string("Loading level...") + name + ".json");

    if (!gameon) {
        visibleloading = true;
    }
    if (stealthloading) {
        visibleloading = false;
    }
    if (!stillloading) {
        loadtime = 0;
    }
    gamestarted = 1;

    numenvsounds = 0;

    pause_sound(whooshsound);
    pause_sound(stream_firesound);

    errno = 0;
    ifstream map_file(level_path);
    Json::Value map_data;
    map_file >> map_data;
    unsigned mapvers = map_data["version"].asInt();
    map_file.close();

    ResetBeforeLevelLoad(tutorial);

    if (console) {
        emit_sound_np(consolesuccesssound);
        freeze = 0;
        console = false;
    }

    if (!stealthloading) {
        terrain.decals.clear();
        Sprite::deleteSprites();

        for (int i = 0; i < subdivision; i++) {
            for (int j = 0; j < subdivision; j++) {
                terrain.patchobjects[i][j].clear();
            }
        }
        Game::LoadingScreen();
    }

    if (mapvers < 13) {
        cerr << name << " has obsolete map version " << mapvers << endl;
    }
    maptype         = map_data["map"].get("type", mapkilleveryone).asInt();
    hostile         = map_data["map"].get("hostile", 1).asInt();
    viewdistance    = map_data["map"].get("viewdistance", 100).asFloat();
    fadestart       = map_data["map"].get("fadestart", .6).asFloat();

    skyboxtexture   = map_data["map"]["skybox"].get("texture", true).asBool();
    skyboxr         = map_data["map"]["skybox"].get("r", 1).asFloat();
    skyboxg         = map_data["map"]["skybox"].get("g", 1).asFloat();
    skyboxb         = map_data["map"]["skybox"].get("b", 1).asFloat();
    skyboxlightr    = map_data["map"]["skybox"].get("lightr", skyboxr).asFloat();
    skyboxlightg    = map_data["map"]["skybox"].get("lightg", skyboxg).asFloat();
    skyboxlightb    = map_data["map"]["skybox"].get("lightb", skyboxb).asFloat();

    Game::LoadingScreen();

    //dialogues
    Dialog::loadDialogs(map_data["map"]["dialogs"]);

    environment = map_data["map"]["environment"].asInt();

    if (environment != oldenvironment) {
        Setenvironment(environment);
    }
    oldenvironment = environment;

    if (!stealthloading) {
        Object::LoadObjectsFromJson(map_data["map"]["objects"]);
    }

    Hotspot::hotspots.resize(map_data["map"]["hotspots"].size());
    for (unsigned i = 0; i < map_data["map"]["hotspots"].size(); i++) {
        Hotspot::hotspots[i].type       = map_data["map"]["hotspots"][i]["type"].asInt();
        Hotspot::hotspots[i].size       = map_data["map"]["hotspots"][i]["size"].asFloat();
        Hotspot::hotspots[i].text       = map_data["map"]["hotspots"][i]["text"].asString();
        Hotspot::hotspots[i].position   = map_data["map"]["hotspots"][i]["position"];
    }

    Game::LoadingScreen();

    if (!stealthloading) {
        Object::ComputeCenter();
        Object::ComputeRadius();
    }

    Game::LoadingScreen();

    if (map_data["map"]["players"].size() > maxplayers) {
        cout << "Warning: this level contains more players than allowed" << endl;
    }

    XYZ playerCoords;
    float playerYaw;
    float playerTargetYaw;
    if (stealthloading) {
        playerCoords    = Person::players[0]->coords;
        playerYaw       = Person::players[0]->yaw;
        playerTargetYaw = Person::players[0]->targetyaw;
    }
    weapons.clear();
    Person::players.clear();
    unsigned j = 0;
    for (unsigned i = 0; i < map_data["map"]["players"].size(); i++) {
        try {
            Person::players.push_back(shared_ptr<Person>(new Person(map_data["map"]["players"][i], mapvers, j)));
            j++;
        } catch (InvalidPersonException &e) {
            cerr << "Invalid Person found in " << name << endl;
        }
    }
    if (stealthloading) {
        Person::players[0]->coords      = playerCoords;
        Person::players[0]->yaw         = playerYaw;
        Person::players[0]->targetyaw   = playerTargetYaw;
    }

    Game::LoadingScreen();

    numpathpoints = map_data["map"]["pathpoints"].size();
    for (unsigned j = 0; j < map_data["map"]["pathpoints"].size(); j++) {
        pathpoint[j] = map_data["map"]["pathpoints"][j]["pos"];
        numpathpointconnect[j] = map_data["map"]["pathpoints"][j]["connect"].size();
        for (unsigned k = 0; k < map_data["map"]["pathpoints"][j]["connect"].size(); k++) {
            pathpointconnect[j][k] = map_data["map"]["pathpoints"][j]["connect"][k].asInt();
        }
    }

    Game::LoadingScreen();

    mapcenter = map_data["map"]["center"];
    mapradius = map_data["map"]["radius"].asFloat();

    SetUpLighting();

    if (!stealthloading) {
        Object::AddObjectsToTerrain();
        terrain.DoShadows();
        Game::LoadingScreen();
        Object::DoShadows();
        Game::LoadingScreen();
    }

    for (unsigned i = 0; i < Person::players.size(); i++) {
        Game::LoadingScreen();

        Person::players[i]->skeleton.free = 0;

        Person::players[i]->skeletonLoad();

        Person::players[i]->addClothes();

        Person::players[i]->speed = 1 + (float)(Random() % 100) / 1000;
        if (difficulty == 0) {
            Person::players[i]->speed -= .2;
        }
        if (difficulty == 1) {
            Person::players[i]->speed -= .1;
        }

        if (i == 0) {
            if (Person::players[i]->creature == wolftype) {
                Person::players[i]->scale = .23;
                Person::players[i]->damagetolerance = 300;
            } else {
                Person::players[i]->damagetolerance = 200;
            }
        }
    }

    Person::players[0]->aitype = playercontrolled;

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

    Game::LoadingScreen();

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

    leveltime = 0;
    wonleveltime = 0;
    visibleloading = false;

    return true;
}

/* Process input events that impact settings, console, devtools, etc.
 * Gameplay-related input processing is still done in Game::Tick() for now
 * as it is tightly coupled to the game logic.
 */
void Game::ProcessInput()
{
    /* Pump SDL input events */
    Input::Tick();

    /* Menu handling (main menu, leave game) */
    if (Input::isKeyPressed(SDL_SCANCODE_ESCAPE) &&
        (gameon || mainmenu == 0)) {
        selected = -1;
        if (mainmenu == 0 && !winfreeze) {
            mainmenu = 2; // Pause
        } else if (mainmenu == 1 || mainmenu == 2) {
            mainmenu = 0; // Unpause
        }
        // Play menu theme
        if (musictoggle && (mainmenu == 1 || mainmenu == 2)) {
            OPENAL_SetFrequency(OPENAL_ALL);
            emit_stream_np(stream_menutheme);
            pause_sound(leveltheme);
        }
        // On resume, play level music
        if (!mainmenu) {
            pause_sound(stream_menutheme);
            resume_stream(leveltheme);
        }
    }

    /* Challenge mode */
    if (!campaign && !mainmenu) {
        if ((Input::isKeyPressed(jumpkey) || Input::isKeyPressed(SDL_SCANCODE_SPACE))) {
            if (winfreeze) {
                winfreeze = 0;
            }
        }

        if ((Input::isKeyDown(SDL_SCANCODE_ESCAPE)) && gameon) {
            if (console) {
                console = false;
                freeze = 0;
            } else if (winfreeze) {
                mainmenu = 9;
                gameon = 0;
            }
        }
    }

    /* Tutorial mode hotkeys */
    if (Tutorial::active) {
        // Skip current tutorial stage
        if (Input::isKeyPressed(SDL_SCANCODE_TAB)) {
            if (Tutorial::stage != 51) {
                Tutorial::stagetime = Tutorial::maxtime;
            }
            emit_sound_np(consolefailsound, 128.);
        }
    }

    /* Screenshot */
    if (Input::isKeyPressed(SDL_SCANCODE_F1)) {
        Screenshot();
    }

    /* Stereo video mode hotkeys */
    if (Input::isKeyPressed(SDL_SCANCODE_F6)) {
        if (Input::isKeyDown(SDL_SCANCODE_LSHIFT)) {
            stereoreverse = true;
            printf("Stereo reversed\n");
        } else {
            stereoreverse = false;
            printf("Stereo unreversed\n");
        }
    }

    if (Input::isKeyDown(SDL_SCANCODE_F7)) {
        if (Input::isKeyDown(SDL_SCANCODE_LSHIFT)) {
            stereoseparation -= 0.001;
        } else {
            stereoseparation -= 0.010;
        }
        printf("Stereo separation decreased to %f\n", stereoseparation);
    }

    if (Input::isKeyDown(SDL_SCANCODE_F8)) {
        if (Input::isKeyDown(SDL_SCANCODE_LSHIFT)) {
            stereoseparation += 0.001;
        } else {
            stereoseparation += 0.010;
        }
        printf("Stereo separation increased to %f\n", stereoseparation);
    }

    /* Devtools */
    if (devtools && !mainmenu) {
        /* Console */
        if (Input::isKeyPressed(consolekey)) {
            console = !console;
            if (console) {
                OPENAL_SetFrequency(OPENAL_ALL);
            } else {
                freeze = 0;
                waiting = false;
            }
        }

        /* Other devtools, disabled when the console is shown */
        if (!console) {
            ProcessDevInput();
        }
    }
}

void Game::ProcessDevInput()
{
    if (!devtools || mainmenu || console) {
        return;
    }

    if (Input::isKeyDown(SDL_SCANCODE_LALT)) {
        /* Enable editor */
        if (Input::isKeyPressed(SDL_SCANCODE_M) && !Input::isKeyDown(SDL_SCANCODE_LSHIFT)) {
            editorenabled = !editorenabled;
            if (editorenabled) {
                Person::players[0]->damagetolerance = 100000;
                Person::players[0]->damage = 0;
                Person::players[0]->permanentdamage = 0;
                Person::players[0]->superpermanentdamage = 0;
                Person::players[0]->burnt = 0;
                Person::players[0]->bloodloss = 0;
                Person::players[0]->deathbleeding = 0;
            } else {
                Person::players[0]->damagetolerance = 200;
            }
        }

        /* Nullify damage and give 200000 health */
        if (Input::isKeyPressed(SDL_SCANCODE_H)) {
            Person::players[0]->damagetolerance = 200000;
            Person::players[0]->damage = 0;
            Person::players[0]->permanentdamage = 0;
            Person::players[0]->superpermanentdamage = 0;
            Person::players[0]->burnt = 0;
            Person::players[0]->bloodloss = 0;
            Person::players[0]->deathbleeding = 0;
        }

        /* Change environment */
        if (Input::isKeyPressed(SDL_SCANCODE_J)) {
            environment++;
            if (environment > 2) {
                environment = 0;
            }
            Setenvironment(environment);
        }

        /* Camera mode */
        if (Input::isKeyPressed(SDL_SCANCODE_C)) {
            cameramode = !cameramode;
        }

        /* Freeze */
        if (Input::isKeyPressed(SDL_SCANCODE_V)) {
            freeze = !freeze;
            if (freeze) {
                OPENAL_SetFrequency(OPENAL_ALL);
            }
        }

        /* Toggle slow motion */
        if (Input::isKeyPressed(SDL_SCANCODE_B)) {
            slomo = 1 - slomo;
            slomodelay = 1000;
        }

        /* Ragdoll */
        if (Input::isKeyPressed(SDL_SCANCODE_N)) {
            Person::players[0]->RagDoll(0);
            emit_sound_at(whooshsound, Person::players[0]->coords, 128.);
        }

        /* Shrink tree leaves?? */
        // FIXME: Can't see what this does in game.
        if (Input::isKeyPressed(SDL_SCANCODE_Y)) {
            for (auto& an_object : Object::objects) {
                if (an_object->type == treeleavestype) {
                    an_object->scale *= .9;
                }
            }
        }

        /* Change (or add) weapon */
        if (Input::isKeyPressed(SDL_SCANCODE_X)) {
            int closest = 0;
            if (!Input::isKeyDown(SDL_SCANCODE_LCTRL)) {
                closest = findClosestPlayer();
            }

            if (closest >= 0) {
                if (Person::players[closest]->num_weapons > 0) {
                    int weapontype = 0;
                    switch (weapons[Person::players[closest]->weaponids[0]].getType()) {
                        case knife:
                            weapontype = sword;
                            break;
                        case sword:
                            weapontype = staff;
                            break;
                        case staff:
                            weapontype = knife;
                            break;
                    }
                    weapons[Person::players[closest]->weaponids[0]].setType(weapontype);
                } else {
                    Person::players[closest]->weaponids[0] = weapons.size();
                    weapons.push_back(Weapon(knife, closest));
                    Person::players[closest]->num_weapons = 1;
                }
            }
        }

        /* Change yaw (rotate around Z axis) */
        if (Input::isKeyDown(SDL_SCANCODE_U)) {
            int closest = 0;
            if (!Input::isKeyDown(SDL_SCANCODE_LCTRL)) {
                closest = findClosestPlayer();
            }

            if (closest >= 0) {
                Person::players[closest]->yaw += multiplier * 50;
                Person::players[closest]->targetyaw = Person::players[closest]->yaw;
            }
        }

        /* Toggle fire */
        if (Input::isKeyPressed(SDL_SCANCODE_F)) {
            int closest = 0;
            if (!Input::isKeyDown(SDL_SCANCODE_LCTRL)) {
                closest = findClosestPlayer();
            }

            if (closest >= 0) {
                Person::players[closest]->onfire = !Person::players[closest]->onfire;
                if (Person::players[closest]->onfire) {
                    Person::players[closest]->CatchFire();
                } else {
                    emit_sound_at(fireendsound, Person::players[closest]->coords);
                    pause_sound(stream_firesound);
                }
            }
        }

        /* Change skin */
        if (Input::isKeyPressed(SDL_SCANCODE_T) && !Input::isKeyDown(SDL_SCANCODE_LSHIFT)) {
            int closest = 0;
            if (!Input::isKeyDown(SDL_SCANCODE_LCTRL)) {
                closest = findClosestPlayer();
            }

            if (closest >= 0) {
                Person::players[closest]->whichskin++;
                if (Person::players[closest]->whichskin > 9) {
                    Person::players[closest]->whichskin = 0;
                }
                if (Person::players[closest]->whichskin > 2 && Person::players[closest]->creature == wolftype) {
                    Person::players[closest]->whichskin = 0;
                }

                Person::players[closest]->skeleton.drawmodel.textureptr.load(
                    PersonType::types[Person::players[closest]->creature].skins[Person::players[closest]->whichskin], 1,
                    &Person::players[closest]->skeleton.skinText[0], &Person::players[closest]->skeleton.skinsize);
            }

            Person::players[closest]->addClothes();
        }

        /* Change creature type */
        if (Input::isKeyPressed(SDL_SCANCODE_T) && Input::isKeyDown(SDL_SCANCODE_LSHIFT)) {
            int closest = 0;
            if (!Input::isKeyDown(SDL_SCANCODE_LCTRL)) {
                closest = findClosestPlayer();
            }

            if (closest >= 0) {
                person_type nextType = static_cast<person_type>((Person::players[closest]->creature + 1) % PersonType::types.size());
                Person::players[closest]->changeCreatureType(nextType);
            }
        }

        /* Explose nearby player's head */
        if (Input::isKeyPressed(SDL_SCANCODE_I) && !Input::isKeyDown(SDL_SCANCODE_LSHIFT)) {
            int closest = findClosestPlayer();
            if (closest > 0 && distsq(&Person::players[closest]->coords, &Person::players[0]->coords) < 144) {
                XYZ flatfacing2, flatvelocity2, flatvelocity2_orig;
                XYZ headspurtdirection;
                Joint& headjoint = Person::players[closest]->joint(head);

                if (!Person::players[closest]->skeleton.free) {
                    flatvelocity2_orig = Person::players[closest]->velocity;
                    flatfacing2 = DoRotation(
                                      DoRotation(
                                          DoRotation(headjoint.position, 0, 0, Person::players[closest]->tilt),
                                          Person::players[closest]->tilt2, 0, 0),
                                      0, Person::players[closest]->yaw, 0) *
                                      Person::players[closest]->scale +
                                  Person::players[closest]->coords;
                } else {
                    flatvelocity2_orig = headjoint.velocity;
                    flatfacing2 = headjoint.position * Person::players[closest]->scale + Person::players[closest]->coords;
                }

                headspurtdirection = headjoint.position - Person::players[closest]->jointPos(neck);
                Normalise(&headspurtdirection);

                for (unsigned i = 0; i < Person::players[closest]->skeleton.joints.size(); i++) {
                    flatvelocity2 = flatvelocity2_orig;
                    flatvelocity2.x += (float)(abs(Random() % 100) - 50) / 10;
                    flatvelocity2.y += (float)(abs(Random() % 100) - 50) / 10;
                    flatvelocity2.z += (float)(abs(Random() % 100) - 50) / 10;
                    printf("Test: %f\n", flatvelocity2.x);
                    printf("Test orig: %f\n", flatvelocity2_orig.x);
                    Sprite::MakeSprite(bloodflamesprite, flatfacing2, flatvelocity2, 1, 1, 1, .6, 1);
                    flatvelocity2 += headspurtdirection * 8;
                    Sprite::MakeSprite(bloodsprite, flatfacing2, flatvelocity2 / 2, 1, 1, 1, .16, 1);
                }
                Sprite::MakeSprite(cloudsprite, flatfacing2, flatvelocity2 * 0, .6, 0, 0, 1, .5);

                emit_sound_at(splattersound, Person::players[closest]->coords);
                emit_sound_at(breaksound2, Person::players[closest]->coords, 100.);

                if (Person::players[closest]->skeleton.free == 2) {
                    Person::players[closest]->skeleton.free = 0;
                }
                Person::players[closest]->RagDoll(0);
                Person::players[closest]->dead = 2;
                Person::players[closest]->headless = 1;
                Person::players[closest]->DoBloodBig(3, 165);

                camerashake += .3;
            }
        }

        /* Explode nearby player */
        if (Input::isKeyPressed(SDL_SCANCODE_I) && Input::isKeyDown(SDL_SCANCODE_LSHIFT)) {
            int closest = findClosestPlayer();
            if (closest >= 0 && distsq(&Person::players[closest]->coords, &Person::players[0]->coords) < 144) {
                XYZ flatfacing2, flatvelocity2, flatvelocity2_orig;

                emit_sound_at(splattersound, Person::players[closest]->coords);
                emit_sound_at(breaksound2, Person::players[closest]->coords);

                for (unsigned i = 0; i < Person::players[closest]->skeleton.joints.size(); i++) {
                    if (!Person::players[closest]->skeleton.free) {
                        flatvelocity2_orig = Person::players[closest]->velocity;
                        flatfacing2 = DoRotation(
                                          DoRotation(
                                              DoRotation(Person::players[closest]->skeleton.joints[i].position, 0, 0, Person::players[closest]->tilt),
                                              Person::players[closest]->tilt2, 0, 0),
                                          0, Person::players[closest]->yaw, 0) *
                                          Person::players[closest]->scale +
                                      Person::players[closest]->coords;
                    } else {
                        flatvelocity2_orig = Person::players[closest]->skeleton.joints[i].velocity;
                        flatfacing2 = Person::players[closest]->skeleton.joints[i].position *
                                          Person::players[closest]->scale +
                                      Person::players[closest]->coords;
                    }

                    // Animation part 1
                    flatvelocity2 = flatvelocity2_orig;
                    flatvelocity2.x += (float)(abs(Random() % 100) - 50) / 10;
                    flatvelocity2.y += (float)(abs(Random() % 100) - 50) / 10;
                    flatvelocity2.z += (float)(abs(Random() % 100) - 50) / 10;
                    Sprite::MakeSprite(bloodflamesprite, flatfacing2, flatvelocity2, 1, 1, 1, 3, 1);
                    Sprite::MakeSprite(bloodsprite, flatfacing2, flatvelocity2, 1, 1, 1, .3, 1);
                    Sprite::MakeSprite(cloudsprite, flatfacing2, flatvelocity2 * 0, .6, 0, 0, 1, .5);

                    // Animation part 2
                    flatvelocity2 = flatvelocity2_orig;
                    flatvelocity2.x += (float)(abs(Random() % 100) - 50) / 10;
                    flatvelocity2.y += (float)(abs(Random() % 100) - 50) / 10;
                    flatvelocity2.z += (float)(abs(Random() % 100) - 50) / 10;
                    Sprite::MakeSprite(bloodflamesprite, flatfacing2, flatvelocity2, 1, 1, 1, 3, 1);
                    Sprite::MakeSprite(bloodsprite, flatfacing2, flatvelocity2, 1, 1, 1, .4, 1);

                    // Animation part 3
                    flatvelocity2 = flatvelocity2_orig;
                    flatvelocity2.x += (float)(abs(Random() % 100) - 50) / 10;
                    flatvelocity2.y += (float)(abs(Random() % 100) - 50) / 10;
                    flatvelocity2.z += (float)(abs(Random() % 100) - 50) / 10;
                    Sprite::MakeSprite(bloodflamesprite, flatfacing2, flatvelocity2 * 2, 1, 1, 1, 3, 1);
                    Sprite::MakeSprite(bloodsprite, flatfacing2, flatvelocity2 * 2, 1, 1, 1, .4, 1);

                    // Animation part 4
                    flatvelocity2 = flatvelocity2_orig;
                    flatvelocity2.x += (float)(abs(Random() % 100) - 50) / 10;
                    flatvelocity2.y += (float)(abs(Random() % 100) - 50) / 10;
                    flatvelocity2.z += (float)(abs(Random() % 100) - 50) / 10;
                    Sprite::MakeSprite(bloodflamesprite, flatfacing2, flatvelocity2 * 2, 1, 1, 1, 3, 1);
                    Sprite::MakeSprite(bloodsprite, flatfacing2, flatvelocity2 * 2, 1, 1, 1, .4, 1);
                }

                XYZ temppos;
                for (unsigned j = 0; j < Person::players.size(); j++) {
                    if (int(j) == closest) {
                        continue;
                    }
                    if (distsq(&Person::players[j]->coords, &Person::players[closest]->coords) < 25) {
                        Person::players[j]->DoDamage((25 - distsq(&Person::players[j]->coords, &Person::players[closest]->coords)) * 60);
                        if (Person::players[j]->skeleton.free == 2) {
                            Person::players[j]->skeleton.free = 1;
                        }
                        Person::players[j]->skeleton.longdead = 0;
                        Person::players[j]->RagDoll(0);
                        for (unsigned i = 0; i < Person::players[j]->skeleton.joints.size(); i++) {
                            temppos = Person::players[j]->skeleton.joints[i].position + Person::players[j]->coords;
                            if (distsq(&temppos, &Person::players[closest]->coords) < 25) {
                                flatvelocity2 = temppos - Person::players[closest]->coords;
                                Normalise(&flatvelocity2);
                                Person::players[j]->skeleton.joints[i].velocity +=
                                    flatvelocity2 * ((20 - distsq(&temppos, &Person::players[closest]->coords)) * 20);
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
    }

    /* Skip level (only for challenges) */
    // FIXME: Allow skipping levels in campaigns too
    if (whichlevel != -2 && Input::isKeyPressed(SDL_SCANCODE_K) && Input::isKeyDown(SDL_SCANCODE_LSHIFT) && !editorenabled) {
        targetlevel++;
        if (targetlevel > numchallengelevels - 1) {
            targetlevel = 0;
        }
        loading = 1;
        leveltime = 5;
    }

    /* Editor mode keys */
    if (editorenabled) {
        /* Closest player deletion */
        if (Input::isKeyPressed(SDL_SCANCODE_DELETE) && Input::isKeyDown(SDL_SCANCODE_LSHIFT)) {
            int closest = findClosestPlayer();
            if (closest > 0) {
                Person::players.erase(Person::players.begin() + closest);
            }
        }

        /* Closest object deletion */
        if (Input::isKeyPressed(SDL_SCANCODE_DELETE) && !Input::isKeyDown(SDL_SCANCODE_LSHIFT)) {
            int closest = findClosestObject();
            if (closest >= 0) {
                Object::DeleteObject(closest);
            }
        }

        /* Add object */
        if (Input::isKeyPressed(SDL_SCANCODE_O)) {
            if (Object::objects.size() < max_objects - 1) {
                XYZ scenecoords = Person::players[0]->coords;
                scenecoords.y -= 3;
                if (editortype == bushtype || editortype == firetype) {
                    scenecoords.y -= 3.5;
                } else {
                    scenecoords.y -= 3;
                }

                float tmpyaw, tmppitch;
                tmpyaw = editoryaw;
                tmppitch = editorpitch;
                if (tmpyaw < 0 || editortype == bushtype) {
                    tmpyaw = Random() % 360;
                }
                if (tmppitch < 0) {
                    tmppitch = Random() % 360;
                }

                Object::MakeObject(editortype, scenecoords, (int)tmpyaw - ((int)tmpyaw) % 30, (int)tmppitch, editorsize);
                if (editortype == treetrunktype) {
                    Object::MakeObject(treeleavestype, scenecoords, Random() % 360 * (tmppitch < 2) + (int)editoryaw - ((int)editoryaw) % 30, editorpitch, editorsize);
                }
            }
        }

        /* Add player */
        if (Input::isKeyPressed(SDL_SCANCODE_P) && !Input::isKeyDown(SDL_SCANCODE_LSHIFT) && !Input::isKeyDown(SDL_SCANCODE_LCTRL)) {
            Person::players.push_back(shared_ptr<Person>(new Person()));

            Person::players.back()->id = Person::players.size() - 1;

            Person::players.back()->scale = Person::players[0]->scale;
            Person::players.back()->creature = rabbittype;
            Person::players.back()->howactive = editoractive;
            Person::players.back()->whichskin = (int)(abs(Random() % 3));

            Person::players.back()->skeletonLoad();

            Person::players.back()->skeleton.drawmodelclothes.textureptr.load("Textures/Belt.png", 1);
            Person::players.back()->speed = 1 + (float)(Random() % 100) / 1000;

            Person::players.back()->targetyaw = Person::players[0]->targetyaw;
            Person::players.back()->yaw = Person::players[0]->yaw;

            Person::players.back()->coords = Person::players[0]->coords;
            Person::players.back()->oldcoords = Person::players.back()->coords;
            Person::players.back()->realoldcoords = Person::players.back()->coords;

            Person::players.back()->setProportions(1, 1, 1, 1);

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

            for (unsigned i = 0; i < Person::players[0]->clothes.size(); i++) {
                Person::players.back()->clothes.push_back(Person::players[0]->clothes[i]);
                Person::players.back()->clothestintr.push_back(Person::players[0]->clothestintr[i]);
                Person::players.back()->clothestintg.push_back(Person::players[0]->clothestintg[i]);
                Person::players.back()->clothestintb.push_back(Person::players[0]->clothestintb[i]);
            }
            Person::players.back()->addClothes();

            Person::players.back()->power = Person::players[0]->power;
            Person::players.back()->speedmult = Person::players[0]->speedmult;

            Person::players.back()->loaded = true;
        }

        /* Add waypoint */
        if (Input::isKeyPressed(SDL_SCANCODE_P) && Input::isKeyDown(SDL_SCANCODE_LSHIFT) && !Input::isKeyDown(SDL_SCANCODE_LCTRL)) {
            if (Person::players.back()->numwaypoints < 90) {
                Person::players.back()->waypoints[Person::players.back()->numwaypoints] = Person::players[0]->coords;
                Person::players.back()->waypointtype[Person::players.back()->numwaypoints] = editorpathtype;
                Person::players.back()->numwaypoints++;
            } else {
                printf("Add waypoint: Reached max number of waypoints (90), aborting.");
            }
        }

        /* Connect waypoint */
        if (Input::isKeyPressed(SDL_SCANCODE_P) && Input::isKeyDown(SDL_SCANCODE_LCTRL) && !Input::isKeyDown(SDL_SCANCODE_LSHIFT)) {
            if (numpathpoints < 30) {
                bool connected = false;
                if (numpathpoints > 1) {
                    for (int i = 0; i < numpathpoints; i++) {
                        if (distsq(&pathpoint[i], &Person::players[0]->coords) < .5 && i != pathpointselected && !connected) {
                            bool alreadyconnected = false;
                            for (int j = 0; j < numpathpointconnect[pathpointselected]; j++) {
                                if (pathpointconnect[pathpointselected][j] == i) {
                                    alreadyconnected = true;
                                }
                            }
                            if (!alreadyconnected) {
                                numpathpointconnect[pathpointselected]++;
                                connected = true;
                                pathpointconnect[pathpointselected][numpathpointconnect[pathpointselected] - 1] = i;
                            }
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
            } else {
                printf("Connect waypoint: Reached max number of path points (30), aborting.");
            }
        }

        /* Select next path waypoint */
        if (Input::isKeyPressed(SDL_SCANCODE_PERIOD)) {
            pathpointselected++;
            if (pathpointselected >= numpathpoints) {
                pathpointselected = -1;
            }
        }

        /* Select previous path waypoint */
        if (Input::isKeyPressed(SDL_SCANCODE_COMMA) && !Input::isKeyDown(SDL_SCANCODE_LSHIFT)) {
            pathpointselected--;
            if (pathpointselected <= -2) {
                pathpointselected = numpathpoints - 1;
            }
        }

        /* Delete path waypoint */
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

        /* Select previous object type */
        if (Input::isKeyPressed(SDL_SCANCODE_LEFT) && Input::isKeyDown(SDL_SCANCODE_LSHIFT) && !Input::isKeyDown(SDL_SCANCODE_LCTRL)) {
            editortype--;
            if (editortype == treeleavestype || editortype == 10) {
                editortype--;
            }
            if (editortype < 0) {
                editortype = firetype;
            }
        }

        /* Select next object type */
        if (Input::isKeyPressed(SDL_SCANCODE_RIGHT) && Input::isKeyDown(SDL_SCANCODE_LSHIFT) && !Input::isKeyDown(SDL_SCANCODE_LCTRL)) {
            editortype++;
            if (editortype == treeleavestype || editortype == 10) {
                editortype++;
            }
            if (editortype > firetype) {
                editortype = 0;
            }
        }

        /* Decrease size for next object */
        if (Input::isKeyDown(SDL_SCANCODE_DOWN) && !Input::isKeyDown(SDL_SCANCODE_LCTRL)) {
            editorsize -= multiplier;
            if (editorsize < .1) {
                editorsize = .1;
            }
        }

        /* Increase size for next object */
        if (Input::isKeyDown(SDL_SCANCODE_UP) && !Input::isKeyDown(SDL_SCANCODE_LCTRL)) {
            editorsize += multiplier;
        }

        /* Decrease yaw for next object */
        if (Input::isKeyDown(SDL_SCANCODE_LEFT) && !Input::isKeyDown(SDL_SCANCODE_LSHIFT) && !Input::isKeyDown(SDL_SCANCODE_LCTRL)) {
            editoryaw -= multiplier * 100;
            if (editoryaw < -.01) {
                editoryaw = -.01;
            }
        }

        /* Increase yaw for next object */
        if (Input::isKeyDown(SDL_SCANCODE_RIGHT) && !Input::isKeyDown(SDL_SCANCODE_LSHIFT) && !Input::isKeyDown(SDL_SCANCODE_LCTRL)) {
            editoryaw += multiplier * 100;
        }

        /* Decrease pitch for next object */
        if (Input::isKeyDown(SDL_SCANCODE_DOWN) && Input::isKeyDown(SDL_SCANCODE_LCTRL)) {
            editorpitch -= multiplier * 100;
            if (editorpitch < -.01) {
                editorpitch = -.01;
            }
        }

        /* Increase pitch for next object */
        if (Input::isKeyDown(SDL_SCANCODE_UP) && Input::isKeyDown(SDL_SCANCODE_LCTRL)) {
            editorpitch += multiplier * 100;
        }

        /* Decrease map radius */
        if (Input::isKeyPressed(SDL_SCANCODE_LEFT) && Input::isKeyDown(SDL_SCANCODE_LSHIFT) && Input::isKeyDown(SDL_SCANCODE_LCTRL)) {
            mapradius -= multiplier * 10;
        }

        /* Increase map radius */
        if (Input::isKeyPressed(SDL_SCANCODE_RIGHT) && Input::isKeyDown(SDL_SCANCODE_LSHIFT) && Input::isKeyDown(SDL_SCANCODE_LCTRL)) {
            mapradius += multiplier * 10;
        }
    }
}

void doJumpReversals()
{
    for (unsigned k = 0; k < Person::players.size(); k++) {
        for (unsigned i = k + 1; i < Person::players.size(); i++) {
            if (Person::players[k]->skeleton.free == 0 &&
                Person::players[i]->skeleton.oldfree == 0 &&
                (Person::players[i]->animTarget == jumpupanim ||
                 Person::players[k]->animTarget == jumpupanim) &&
                (Person::players[i]->isPlayerControlled() ||
                 Person::players[k]->isPlayerControlled()) &&
                ((Person::players[i]->aitype == attacktypecutoff && Person::players[i]->stunned <= 0) ||
                 (Person::players[k]->aitype == attacktypecutoff && Person::players[k]->stunned <= 0))) {
                if (distsq(&Person::players[i]->coords, &Person::players[k]->coords) < 10 * sq((Person::players[i]->scale + Person::players[k]->scale) * 2.5) &&
                    distsqflat(&Person::players[i]->coords, &Person::players[k]->coords) < 2 * sq((Person::players[i]->scale + Person::players[k]->scale) * 2.5)) {
                    //TODO: refactor two huge similar ifs
                    if (Person::players[i]->animTarget == jumpupanim &&
                        Person::players[k]->animTarget != getupfrombackanim &&
                        Person::players[k]->animTarget != getupfromfrontanim &&
                        Animation::animations[Person::players[k]->animTarget].height == middleheight &&
                        normaldotproduct(Person::players[i]->velocity, Person::players[k]->coords - Person::players[i]->coords) < 0 &&
                        ((Person::players[k]->isPlayerControlled() && Person::players[k]->attackkeydown) ||
                         !Person::players[k]->isPlayerControlled())) {
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
                        if (Person::players[k]->aitype == attacktypecutoff) {
                            Person::players[k]->stunned = .5;
                        }
                    }
                    if (Person::players[k]->animTarget == jumpupanim &&
                        Person::players[i]->animTarget != getupfrombackanim &&
                        Person::players[i]->animTarget != getupfromfrontanim &&
                        Animation::animations[Person::players[i]->animTarget].height == middleheight &&
                        normaldotproduct(Person::players[k]->velocity, Person::players[i]->coords - Person::players[k]->coords) < 0 &&
                        ((Person::players[i]->isPlayerControlled() && Person::players[i]->attackkeydown) ||
                         !Person::players[i]->isPlayerControlled())) {
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
                        if (Person::players[i]->aitype == attacktypecutoff) {
                            Person::players[i]->stunned = .5;
                        }
                    }
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
            (Animation::animations[Person::players[k]->animTarget].attack &&
             Person::players[k]->animTarget != rabbitkickanim &&
             (Person::players[k]->animTarget != crouchstabanim || Person::players[k]->hasvictim) &&
             (Person::players[k]->animTarget != swordgroundstabanim || Person::players[k]->hasvictim))) {
            Person::players[k]->yaw = stepTowardf(Person::players[k]->yaw, Person::players[k]->targetyaw, multiplier * Person::players[k]->turnspeed * 2);
        }

        if (Person::players[k]->animTarget == sneakanim && Person::players[k]->animCurrent != sneakanim) {
            Person::players[k]->yaw = stepTowardf(Person::players[k]->yaw, Person::players[k]->targetyaw, multiplier * Person::players[k]->turnspeed * 4);
        }

        Person::players[k]->DoStuff();
        if (Person::players[k]->immobile && k != 0) {
            Person::players[k]->coords = Person::players[k]->realoldcoords;
        }

        //if player's position has changed (?)
        if (distsq(&Person::players[k]->coords, &Person::players[k]->realoldcoords) > 0 &&
            !Person::players[k]->skeleton.free &&
            Person::players[k]->animTarget != climbanim &&
            Person::players[k]->animTarget != hanganim) {
            XYZ lowpoint, lowpointtarget, lowpoint2, lowpointtarget2, lowpoint3, lowpointtarget3, lowpoint4, lowpointtarget4, lowpoint5, lowpointtarget5, lowpoint6, lowpointtarget6, lowpoint7, lowpointtarget7, colpoint, colpoint2;
            int whichhit;
            bool tempcollide = 0;

            if (Person::players[k]->collide < -.3) {
                Person::players[k]->collide = -.3;
            }
            if (Person::players[k]->collide > 1) {
                Person::players[k]->collide = 1;
            }
            Person::players[k]->collide -= multiplier * 30;

            //clip to terrain
            Person::players[k]->coords.y = max(Person::players[k]->coords.y, terrain.getHeight(Person::players[k]->coords.x, Person::players[k]->coords.z));

            for (unsigned int l = 0; l < terrain.patchobjects[Person::players[k]->whichpatchx][Person::players[k]->whichpatchz].size(); l++) {
                unsigned int i = terrain.patchobjects[Person::players[k]->whichpatchx][Person::players[k]->whichpatchz][l];
                if (Object::objects[i]->type != rocktype ||
                    Object::objects[i]->scale > .5 && Person::players[k]->isPlayerControlled() ||
                    Object::objects[i]->position.y > Person::players[k]->coords.y) {
                    lowpoint = Person::players[k]->coords;
                    if (Person::players[k]->animTarget != jumpupanim &&
                        Person::players[k]->animTarget != jumpdownanim &&
                        !Person::players[k]->isFlip()) {
                        lowpoint.y += 1.25;
                    } else {
                        lowpoint.y += 1.3;
                    }
                    if (Person::players[k]->coords.y < terrain.getHeight(Person::players[k]->coords.x, Person::players[k]->coords.z) &&
                        Person::players[k]->coords.y > terrain.getHeight(Person::players[k]->coords.x, Person::players[k]->coords.z) - .1) {
                        Person::players[k]->coords.y = terrain.getHeight(Person::players[k]->coords.x, Person::players[k]->coords.z);
                    }
                    if (Person::players[k]->SphereCheck(&lowpoint, 1.3, &colpoint, &Object::objects[i]->position, &Object::objects[i]->yaw, &Object::objects[i]->model) != -1) {
                        flatfacing = lowpoint - Person::players[k]->coords;
                        Person::players[k]->coords = lowpoint;
                        Person::players[k]->coords.y -= 1.3;
                        Person::players[k]->collide = 1;
                        tempcollide = 1;
                        //wall jumps
                        //TODO: refactor four similar blocks
                        if (Person::players[k]->isPlayerControlled() &&
                            (Person::players[k]->animTarget == jumpupanim ||
                             Person::players[k]->animTarget == jumpdownanim ||
                             Person::players[k]->isFlip()) &&
                            !Person::players[k]->jumptogglekeydown &&
                            Person::players[k]->jumpkeydown) {
                            lowpointtarget = lowpoint + DoRotation(Person::players[k]->facing, 0, -90, 0) * 1.5;
                            XYZ tempcoords1 = lowpoint;
                            whichhit = Object::objects[i]->model.LineCheck(&lowpoint, &lowpointtarget, &colpoint, &Object::objects[i]->position, &Object::objects[i]->yaw);
                            if (whichhit != -1 && fabs(Object::objects[i]->model.Triangles[whichhit].facenormal.y) < .3) {
                                Person::players[k]->setTargetAnimation(walljumpleftanim);
                                emit_sound_at(movewhooshsound, Person::players[k]->coords);
                                if (k == 0) {
                                    pause_sound(whooshsound);
                                }

                                lowpointtarget = DoRotation(Object::objects[i]->model.Triangles[whichhit].facenormal, 0, Object::objects[i]->yaw, 0);
                                Person::players[k]->yaw = -asin(0 - lowpointtarget.x) * 180 / M_PI;
                                if (lowpointtarget.z < 0) {
                                    Person::players[k]->yaw = 180 - Person::players[k]->yaw;
                                }
                                Person::players[k]->targetyaw = Person::players[k]->yaw;
                                Person::players[k]->lowyaw = Person::players[k]->yaw;
                                if (k == 0) {
                                    numwallflipped++;
                                }
                            } else {
                                lowpoint = tempcoords1;
                                lowpointtarget = lowpoint + DoRotation(Person::players[k]->facing, 0, 90, 0) * 1.5;
                                whichhit = Object::objects[i]->model.LineCheck(&lowpoint, &lowpointtarget, &colpoint, &Object::objects[i]->position, &Object::objects[i]->yaw);
                                if (whichhit != -1 && fabs(Object::objects[i]->model.Triangles[whichhit].facenormal.y) < .3) {
                                    Person::players[k]->setTargetAnimation(walljumprightanim);
                                    emit_sound_at(movewhooshsound, Person::players[k]->coords);
                                    if (k == 0) {
                                        pause_sound(whooshsound);
                                    }

                                    lowpointtarget = DoRotation(Object::objects[i]->model.Triangles[whichhit].facenormal, 0, Object::objects[i]->yaw, 0);
                                    Person::players[k]->yaw = -asin(0 - lowpointtarget.x) * 180 / M_PI;
                                    if (lowpointtarget.z < 0) {
                                        Person::players[k]->yaw = 180 - Person::players[k]->yaw;
                                    }
                                    Person::players[k]->targetyaw = Person::players[k]->yaw;
                                    Person::players[k]->lowyaw = Person::players[k]->yaw;
                                    if (k == 0) {
                                        numwallflipped++;
                                    }
                                } else {
                                    lowpoint = tempcoords1;
                                    lowpointtarget = lowpoint + Person::players[k]->facing * 2;
                                    whichhit = Object::objects[i]->model.LineCheck(&lowpoint, &lowpointtarget, &colpoint, &Object::objects[i]->position, &Object::objects[i]->yaw);
                                    if (whichhit != -1 && fabs(Object::objects[i]->model.Triangles[whichhit].facenormal.y) < .3) {
                                        Person::players[k]->setTargetAnimation(walljumpbackanim);
                                        emit_sound_at(movewhooshsound, Person::players[k]->coords);
                                        if (k == 0) {
                                            pause_sound(whooshsound);
                                        }

                                        lowpointtarget = DoRotation(Object::objects[i]->model.Triangles[whichhit].facenormal, 0, Object::objects[i]->yaw, 0);
                                        Person::players[k]->yaw = -asin(0 - lowpointtarget.x) * 180 / M_PI;
                                        if (lowpointtarget.z < 0) {
                                            Person::players[k]->yaw = 180 - Person::players[k]->yaw;
                                        }
                                        Person::players[k]->targetyaw = Person::players[k]->yaw;
                                        Person::players[k]->lowyaw = Person::players[k]->yaw;
                                        if (k == 0) {
                                            numwallflipped++;
                                        }
                                    } else {
                                        lowpoint = tempcoords1;
                                        lowpointtarget = lowpoint - Person::players[k]->facing * 2;
                                        whichhit = Object::objects[i]->model.LineCheck(&lowpoint, &lowpointtarget, &colpoint, &Object::objects[i]->position, &Object::objects[i]->yaw);
                                        if (whichhit != -1 && fabs(Object::objects[i]->model.Triangles[whichhit].facenormal.y) < .3) {
                                            Person::players[k]->setTargetAnimation(walljumpfrontanim);
                                            emit_sound_at(movewhooshsound, Person::players[k]->coords);
                                            if (k == 0) {
                                                pause_sound(whooshsound);
                                            }

                                            lowpointtarget = DoRotation(Object::objects[i]->model.Triangles[whichhit].facenormal, 0, Object::objects[i]->yaw, 0);
                                            Person::players[k]->yaw = -asin(0 - lowpointtarget.x) * 180 / M_PI;
                                            if (lowpointtarget.z < 0) {
                                                Person::players[k]->yaw = 180 - Person::players[k]->yaw;
                                            }
                                            Person::players[k]->yaw += 180;
                                            Person::players[k]->targetyaw = Person::players[k]->yaw;
                                            Person::players[k]->lowyaw = Person::players[k]->yaw;
                                            if (k == 0) {
                                                numwallflipped++;
                                            }
                                        }
                                    }
                                }
                            }
                        }
                    }
                } else if (Object::objects[i]->type == rocktype) {
                    lowpoint2 = Person::players[k]->coords;
                    lowpoint = Person::players[k]->coords;
                    lowpoint.y += 2;
                    if (Object::objects[i]->model.LineCheck(&lowpoint, &lowpoint2, &colpoint, &Object::objects[i]->position, &Object::objects[i]->yaw) != -1) {
                        Person::players[k]->coords = colpoint;
                        Person::players[k]->collide = 1;
                        tempcollide = 1;

                        if (Person::players[k]->animTarget == jumpdownanim || Person::players[k]->isFlip()) {
                            //flipped into a rock
                            if (Person::players[k]->isFlip() && Person::players[k]->targetFrame().label == 7) {
                                Person::players[k]->RagDoll(0);
                            }

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
                                if (Person::players[k]->isFlip()) {
                                    Person::players[k]->jumppower = -4;
                                }
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

            if (tempcollide) {
                for (unsigned int l = 0; l < terrain.patchobjects[Person::players[k]->whichpatchx][Person::players[k]->whichpatchz].size(); l++) {
                    int i = terrain.patchobjects[Person::players[k]->whichpatchx][Person::players[k]->whichpatchz][l];
                    lowpoint = Person::players[k]->coords;
                    lowpoint.y += 1.35;
                    if (Object::objects[i]->type != rocktype) {
                        if (Person::players[k]->SphereCheck(&lowpoint, 1.33, &colpoint, &Object::objects[i]->position, &Object::objects[i]->yaw, &Object::objects[i]->model) != -1) {
                            if (Person::players[k]->animTarget != jumpupanim &&
                                Person::players[k]->animTarget != jumpdownanim &&
                                Person::players[k]->onterrain) {
                                Person::players[k]->avoidcollided = 1;
                            }
                            Person::players[k]->coords = lowpoint;
                            Person::players[k]->coords.y -= 1.35;
                            Person::players[k]->collide = 1;

                            if ((Person::players[k]->grabdelay <= 0 || !Person::players[k]->isPlayerControlled()) &&
                                (Person::players[k]->animCurrent != climbanim &&
                                     Person::players[k]->animCurrent != hanganim &&
                                     !Person::players[k]->isWallJump() ||
                                 Person::players[k]->animTarget == jumpupanim ||
                                 Person::players[k]->animTarget == jumpdownanim)) {
                                lowpoint = Person::players[k]->coords;
                                Object::objects[i]->model.SphereCheckPossible(&lowpoint, 1.5, &Object::objects[i]->position, &Object::objects[i]->yaw);
                                lowpoint = Person::players[k]->coords;
                                lowpoint.y += .05;
                                facing = 0;
                                facing.z = -1;
                                facing = DoRotation(facing, 0, Person::players[k]->targetyaw + 180, 0);
                                lowpointtarget = lowpoint + facing * 1.4;
                                whichhit = Object::objects[i]->model.LineCheckPossible(&lowpoint, &lowpointtarget, &colpoint, &Object::objects[i]->position, &Object::objects[i]->yaw);
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
                                    whichhit = Object::objects[i]->model.LineCheckPossible(&lowpoint, &lowpointtarget, &colpoint, &Object::objects[i]->position, &Object::objects[i]->yaw);
                                    if (Object::objects[i]->friction > .5) {
                                        if (whichhit != -1) {
                                            if (Person::players[k]->animTarget != jumpupanim && Person::players[k]->animTarget != jumpdownanim) {
                                                Person::players[k]->collided = 1;
                                            }
                                            if (Object::checkcollide(lowpoint7, lowpointtarget7) == -1) {
                                                if (Object::checkcollide(lowpoint6, lowpointtarget6) == -1) {
                                                    if (Object::objects[i]->model.LineCheckPossible(&lowpoint2, &lowpointtarget2,
                                                                                                    &colpoint, &Object::objects[i]->position, &Object::objects[i]->yaw) != -1 &&
                                                        Object::objects[i]->model.LineCheckPossible(&lowpoint3, &lowpointtarget3,
                                                                                                    &colpoint, &Object::objects[i]->position, &Object::objects[i]->yaw) != -1 &&
                                                        Object::objects[i]->model.LineCheckPossible(&lowpoint4, &lowpointtarget4,
                                                                                                    &colpoint, &Object::objects[i]->position, &Object::objects[i]->yaw) != -1 &&
                                                        Object::objects[i]->model.LineCheckPossible(&lowpoint5, &lowpointtarget5,
                                                                                                    &colpoint, &Object::objects[i]->position, &Object::objects[i]->yaw) != -1) {
                                                        for (int j = 0; j < 45; j++) {
                                                            lowpoint = Person::players[k]->coords;
                                                            lowpoint.y += (float)j / 13;
                                                            lowpointtarget = lowpoint + facing * 1.4;
                                                            if (Object::objects[i]->model.LineCheckPossible(&lowpoint, &lowpointtarget,
                                                                                                            &colpoint2, &Object::objects[i]->position, &Object::objects[i]->yaw) == -1) {
                                                                if (j <= 6 || j <= 25 && Person::players[k]->animTarget == jumpdownanim) {
                                                                    break;
                                                                }
                                                                if (Person::players[k]->animTarget == jumpupanim || Person::players[k]->animTarget == jumpdownanim) {
                                                                    lowpoint = Person::players[k]->coords;
                                                                    lowpoint.y += (float)j / 13;
                                                                    lowpointtarget = lowpoint + facing * 1.3;
                                                                    flatfacing = Person::players[k]->coords;
                                                                    Person::players[k]->coords = colpoint - DoRotation(Object::objects[i]->model.Triangles[whichhit].facenormal, 0, Object::objects[k]->yaw, 0) * .01;
                                                                    Person::players[k]->coords.y = lowpointtarget.y - .07;
                                                                    Person::players[k]->currentoffset = (flatfacing - Person::players[k]->coords) / Person::players[k]->scale;

                                                                    if (j > 10 || !Person::players[k]->isRun()) {
                                                                        if (Person::players[k]->animTarget == jumpdownanim || Person::players[k]->animTarget == jumpupanim) {
                                                                            if (k == 0) {
                                                                                pause_sound(whooshsound);
                                                                            }
                                                                        }
                                                                        emit_sound_at(jumpsound, Person::players[k]->coords, 128.);

                                                                        lowpointtarget = DoRotation(Object::objects[i]->model.Triangles[whichhit].facenormal, 0, Object::objects[i]->yaw, 0);
                                                                        Person::players[k]->yaw = -asin(0 - lowpointtarget.x) * 180 / M_PI;
                                                                        if (lowpointtarget.z < 0) {
                                                                            Person::players[k]->yaw = 180 - Person::players[k]->yaw;
                                                                        }
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
                                                                            Person::players[k]->setTargetAnimation(hanganim);
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
                        if (Person::players[k]->animTarget == staggerbackhighanim || Person::players[k]->animTarget == staggerbackhardanim) {
                            Person::players[k]->RagDoll(0);
                        }
                        Person::players[k]->setTargetAnimation(jumpdownanim);

                        if (!k) {
                            emit_sound_at(whooshsound, Person::players[k]->coords, 128.);
                        }
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

    if (!Input::isKeyDown(attackkey)) {
        oldattackkey = 0;
    }
    if (oldattackkey) {
        Person::players[0]->attackkeydown = 0;
    }
    if (oldattackkey) {
        playerrealattackkeydown = 0;
    }
    if (!oldattackkey) {
        playerrealattackkeydown = Input::isKeyDown(attackkey);
    }
    if ((Person::players[0]->parriedrecently <= 0 ||
         !Person::players[0]->hasWeapon()) &&
        (!oldattackkey ||
         (realthreat &&
          Person::players[0]->lastattack != swordslashanim &&
          Person::players[0]->lastattack != knifeslashstartanim &&
          Person::players[0]->lastattack != staffhitanim &&
          Person::players[0]->lastattack != staffspinhitanim))) {
        Person::players[0]->attackkeydown = Input::isKeyDown(attackkey);
    }
    if (Input::isKeyDown(attackkey) &&
        !oldattackkey &&
        !Person::players[0]->backkeydown) {
        for (unsigned k = 0; k < Person::players.size(); k++) {
            if ((Person::players[k]->animTarget == swordslashanim ||
                 Person::players[k]->animTarget == staffhitanim ||
                 Person::players[k]->animTarget == staffspinhitanim) &&
                Person::players[0]->animCurrent != dodgebackanim &&
                !Person::players[k]->skeleton.free) {
                Person::players[k]->Reverse();
            }
        }
    }

    if (!hostile || Dialog::inDialog()) {
        Person::players[0]->attackkeydown = 0;
    }

    for (unsigned k = 0; k < Person::players.size(); k++) {
        if (Dialog::inDialog()) {
            Person::players[k]->attackkeydown = 0;
        }
        if (Person::players[k]->animTarget != rabbitrunninganim && Person::players[k]->animTarget != wolfrunninganim) {
            if (!Person::players[k]->isPlayerControlled()) {
                Person::players[k]->victim = Person::players[0];
            }
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
                            if (i == k) {
                                continue;
                            }
                            if (Person::players[i]->animTarget == swordslashanim ||
                                Person::players[i]->animTarget == knifeslashstartanim ||
                                Person::players[i]->animTarget == staffhitanim ||
                                Person::players[i]->animTarget == staffspinhitanim) {
                                if (distsq(&Person::players[k]->coords, &Person::players[i]->coords) < 6.5 && !Person::players[i]->skeleton.free) {
                                    Person::players[k]->setTargetAnimation(dodgebackanim);
                                    Person::players[k]->targetyaw = roughDirectionTo(Person::players[k]->coords, Person::players[i]->coords);
                                    Person::players[k]->targettilt2 = pitchTo(Person::players[k]->coords, Person::players[i]->coords);
                                }
                            }
                        }
                        if (Person::players[k]->animTarget != dodgebackanim) {
                            if (k == 0) {
                                numflipped++;
                            }
                            Person::players[k]->setTargetAnimation(backhandspringanim);
                            Person::players[k]->targetyaw = -yaw + 180;
                            if (Person::players[k]->leftkeydown) {
                                Person::players[k]->targetyaw -= 45;
                            }
                            if (Person::players[k]->rightkeydown) {
                                Person::players[k]->targetyaw += 45;
                            }
                            Person::players[k]->yaw = Person::players[k]->targetyaw;
                            Person::players[k]->jumppower -= 2;
                        }
                    }
                }
                //attack
                if (!Animation::animations[Person::players[k]->animTarget].attack &&
                    !Person::players[k]->backkeydown &&
                    (Person::players[k]->isIdle() ||
                     Person::players[k]->isRun() ||
                     Person::players[k]->animTarget == walkanim ||
                     Person::players[k]->animTarget == sneakanim ||
                     Person::players[k]->isCrouch())) {
                    const int attackweapon = (Person::players[k]->hasWeapon() ? weapons[Person::players[k]->weaponids[Person::players[k]->weaponactive]].getType() : 0);
                    //normal attacks (?)
                    Person::players[k]->hasvictim = 0;
                    if (Person::players.size() > 1) {
                        for (unsigned i = 0; i < Person::players.size(); i++) {
                            if (i == k || !(k == 0 || i == 0)) {
                                continue;
                            }
                            if (!Person::players[k]->hasvictim) {
                                if (Animation::animations[Person::players[k]->animTarget].attack != reversal) {
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
                                         (Person::players[i]->animTarget != staggerbackhardanim || Person::players[i]->targetFrame().label == 6)) &&
                                        Person::players[i]->animTarget != jumpdownanim &&
                                        Person::players[i]->animTarget != jumpupanim &&
                                        Person::players[i]->animTarget != getupfromfrontanim) {
                                        Person::players[k]->victim = Person::players[i];
                                        Person::players[k]->hasvictim = 1;
                                        if (Person::players[k]->isPlayerControlled()) { //human player
                                            //sweep
                                            if (distance < 2.5 * sq(Person::players[k]->scale * 5) &&
                                                Person::players[k]->crouchkeydown &&
                                                Animation::animations[Person::players[i]->animTarget].height != lowheight) {
                                                Person::players[k]->animTarget = sweepanim;
                                                //winduppunch
                                            } else if (distance < 1.5 * sq(Person::players[k]->scale * 5) &&
                                                       Animation::animations[Person::players[i]->animTarget].height != lowheight &&
                                                       !Person::players[k]->forwardkeydown &&
                                                       !Person::players[k]->leftkeydown &&
                                                       !Person::players[k]->rightkeydown &&
                                                       !Person::players[k]->crouchkeydown &&
                                                       !attackweapon &&
                                                       !reversaltrain) {
                                                Person::players[k]->animTarget = winduppunchanim;
                                                //upunch
                                            } else if (distance < 2.5 * sq(Person::players[k]->scale * 5) &&
                                                       Animation::animations[Person::players[i]->animTarget].height != lowheight &&
                                                       !Person::players[k]->forwardkeydown &&
                                                       !Person::players[k]->leftkeydown &&
                                                       !Person::players[k]->rightkeydown &&
                                                       !Person::players[k]->crouchkeydown &&
                                                       !attackweapon) {
                                                Person::players[k]->animTarget = upunchanim;
                                                //knifefollow
                                            } else if (distance < 2.5 * sq(Person::players[k]->scale * 5) &&
                                                       Person::players[i]->staggerdelay > 0 &&
                                                       attackweapon == knife &&
                                                       Person::players[i]->bloodloss > Person::players[i]->damagetolerance / 2) {
                                                Person::players[k]->animTarget = knifefollowanim;
                                                //knifeslashstart
                                            } else if (distance < 2.5 * sq(Person::players[k]->scale * 5) &&
                                                       Animation::animations[Person::players[i]->animTarget].height != lowheight &&
                                                       !Person::players[k]->forwardkeydown &&
                                                       !Person::players[k]->leftkeydown &&
                                                       !Person::players[k]->rightkeydown &&
                                                       !Person::players[k]->crouchkeydown &&
                                                       attackweapon == knife &&
                                                       Person::players[k]->weaponmissdelay <= 0) {
                                                Person::players[k]->animTarget = knifeslashstartanim;
                                                //swordslash
                                            } else if (distance < 4.5 * sq(Person::players[k]->scale * 5) &&
                                                       Animation::animations[Person::players[i]->animTarget].height != lowheight &&
                                                       !Person::players[k]->crouchkeydown &&
                                                       attackweapon == sword &&
                                                       Person::players[k]->weaponmissdelay <= 0) {
                                                Person::players[k]->animTarget = swordslashanim;
                                                //staffhit
                                            } else if (distance < 4.5 * sq(Person::players[k]->scale * 5) &&
                                                       Animation::animations[Person::players[i]->animTarget].height != lowheight &&
                                                       !Person::players[k]->crouchkeydown &&
                                                       attackweapon == staff &&
                                                       Person::players[k]->weaponmissdelay <= 0 &&
                                                       !Person::players[k]->leftkeydown &&
                                                       !Person::players[k]->rightkeydown &&
                                                       !Person::players[k]->forwardkeydown) {
                                                Person::players[k]->animTarget = staffhitanim;
                                                //staffspinhit
                                            } else if (distance < 4.5 * sq(Person::players[k]->scale * 5) &&
                                                       Animation::animations[Person::players[i]->animTarget].height != lowheight &&
                                                       !Person::players[k]->crouchkeydown &&
                                                       attackweapon == staff &&
                                                       Person::players[k]->weaponmissdelay <= 0) {
                                                Person::players[k]->animTarget = staffspinhitanim;
                                                //spinkick
                                            } else if (distance < 2.5 * sq(Person::players[k]->scale * 5) &&
                                                       Animation::animations[Person::players[i]->animTarget].height != lowheight) {
                                                Person::players[k]->animTarget = spinkickanim;
                                                //lowkick
                                            } else if (distance < 2.5 * sq(Person::players[k]->scale * 5) &&
                                                       Animation::animations[Person::players[i]->animTarget].height == lowheight &&
                                                       Animation::animations[Person::players[k]->animTarget].attack != normalattack) {
                                                Person::players[k]->animTarget = lowkickanim;
                                            }
                                        } else { //AI player
                                            if (distance < 4.5 * sq(Person::players[k]->scale * 5)) {
                                                randattack = abs(Random() % 5);
                                                if (!attackweapon && distance < 2.5 * sq(Person::players[k]->scale * 5)) {
                                                    //sweep
                                                    if (randattack == 0 && Animation::animations[Person::players[i]->animTarget].height != lowheight) {
                                                        Person::players[k]->animTarget = sweepanim;
                                                        //upunch
                                                    } else if (randattack == 1 && Animation::animations[Person::players[i]->animTarget].height != lowheight &&
                                                               !attackweapon) {
                                                        Person::players[k]->animTarget = upunchanim;
                                                        //spinkick
                                                    } else if (randattack == 2 && Animation::animations[Person::players[i]->animTarget].height != lowheight) {
                                                        Person::players[k]->animTarget = spinkickanim;
                                                        //lowkick
                                                    } else if (Animation::animations[Person::players[i]->animTarget].height == lowheight) {
                                                        Person::players[k]->animTarget = lowkickanim;
                                                    }
                                                }
                                                if (attackweapon) {
                                                    //sweep
                                                    if ((!Tutorial::active || !attackweapon) &&
                                                        distance < 2.5 * sq(Person::players[k]->scale * 5) &&
                                                        randattack == 0 &&
                                                        Animation::animations[Person::players[i]->animTarget].height != lowheight) {
                                                        Person::players[k]->animTarget = sweepanim;
                                                        //knifeslashstart
                                                    } else if (distance < 2.5 * sq(Person::players[k]->scale * 5) &&
                                                               attackweapon == knife &&
                                                               Person::players[k]->weaponmissdelay <= 0) {
                                                        Person::players[k]->animTarget = knifeslashstartanim;
                                                        //swordslash
                                                    } else if (!(Person::players[0]->victim == Person::players[i] &&
                                                                 Person::players[0]->hasvictim &&
                                                                 Person::players[0]->animTarget == swordslashanim) &&
                                                               attackweapon == sword &&
                                                               Person::players[k]->weaponmissdelay <= 0) {
                                                        Person::players[k]->animTarget = swordslashanim;
                                                        //staffhit
                                                    } else if (!(Person::players[0]->victim == Person::players[i] &&
                                                                 Person::players[0]->hasvictim &&
                                                                 Person::players[0]->animTarget == swordslashanim) &&
                                                               attackweapon == staff &&
                                                               Person::players[k]->weaponmissdelay <= 0 &&
                                                               randattack < 3) {
                                                        Person::players[k]->animTarget = staffhitanim;
                                                        //staffspinhit
                                                    } else if (!(Person::players[0]->victim == Person::players[i] &&
                                                                 Person::players[0]->hasvictim &&
                                                                 Person::players[0]->animTarget == swordslashanim) &&
                                                               attackweapon == staff &&
                                                               Person::players[k]->weaponmissdelay <= 0 &&
                                                               randattack >= 3) {
                                                        Person::players[k]->animTarget = staffspinhitanim;
                                                        //spinkick
                                                    } else if ((!Tutorial::active || !attackweapon) &&
                                                               distance < 2.5 * sq(Person::players[k]->scale * 5) &&
                                                               randattack == 1 &&
                                                               Animation::animations[Person::players[i]->animTarget].height != lowheight) {
                                                        Person::players[k]->animTarget = spinkickanim;
                                                        //lowkick
                                                    } else if (distance < 2.5 * sq(Person::players[k]->scale * 5) &&
                                                               Animation::animations[Person::players[i]->animTarget].height == lowheight &&
                                                               Animation::animations[Person::players[k]->animTarget].attack != normalattack) {
                                                        Person::players[k]->animTarget = lowkickanim;
                                                    }
                                                }
                                            }
                                        }
                                        //upunch becomes wolfslap
                                        if (Person::players[k]->animTarget == upunchanim && Person::players[k]->creature == wolftype) {
                                            Person::players[k]->animTarget = wolfslapanim;
                                        }
                                    }
                                    //sneak attacks
                                    if ((k == 0) && (!Tutorial::active || Tutorial::stage == 22) &&
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
                                    if (Animation::animations[Person::players[k]->animTarget].attack == normalattack &&
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
                        }
                    }
                    const bool hasstaff = attackweapon == staff;
                    if (k == 0 && Person::players.size() > 1) {
                        for (unsigned i = 0; i < Person::players.size(); i++) {
                            if (i == k) {
                                continue;
                            }
                            if ((playerrealattackkeydown || Person::players[i]->dead || !hasstaff) &&
                                Animation::animations[Person::players[k]->animTarget].attack == neutral) {
                                const float distance = distsq(&Person::players[k]->coords, &Person::players[i]->coords);
                                if (!Person::players[i]->dead || !realthreat || (!attackweapon && Person::players[k]->crouchkeydown)) {
                                    if (Person::players[i]->skeleton.free) {
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
                                            if (attackweapon && !Tutorial::active) {
                                                //crouchstab
                                                if (Person::players[k]->crouchkeydown && attackweapon == knife && distance < 1.5 * sq(Person::players[k]->scale * 5)) {
                                                    Person::players[k]->animTarget = crouchstabanim;
                                                }
                                                //swordgroundstab
                                                if (Person::players[k]->crouchkeydown && distance < 1.5 * sq(Person::players[k]->scale * 5) && attackweapon == sword) {
                                                    Person::players[k]->animTarget = swordgroundstabanim;
                                                }
                                                //staffgroundsmash
                                                if (distance < 3.5 * sq(Person::players[k]->scale * 5) && attackweapon == staff) {
                                                    Person::players[k]->animTarget = staffgroundsmashanim;
                                                }
                                            }
                                            if (distance < 2.5 &&
                                                Person::players[k]->crouchkeydown &&
                                                Person::players[k]->animTarget != crouchstabanim &&
                                                !attackweapon &&
                                                Person::players[i]->dead &&
                                                Person::players[i]->skeleton.free &&
                                                Person::players[i]->skeleton.longdead > 1000) {
                                                Person::players[k]->animTarget = killanim;
                                                terrain.deleteDeadDecals();
                                                for (unsigned int l = 0; l < Object::objects.size(); l++) {
                                                    if (Object::objects[l]->model.type == decalstype) {
                                                        Object::objects[l]->model.deleteDeadDecals();
                                                    }
                                                }
                                            }
                                            if (!Person::players[i]->dead || musictype != 2) {
                                                if (distance < 3.5 &&
                                                    (Person::players[k]->isRun() || Person::players[k]->isIdle() && Person::players[k]->attackkeydown) &&
                                                    Person::players[k]->staggerdelay <= 0 &&
                                                    (Person::players[i]->dead ||
                                                     Person::players[i]->skeleton.longdead < 300 &&
                                                         Person::players[k]->lastattack != spinkickanim &&
                                                         Person::players[i]->skeleton.free) &&
                                                    (!Person::players[i]->dead || musictype != stream_fighttheme)) {
                                                    Person::players[k]->animTarget = dropkickanim;
                                                    terrain.deleteDeadDecals();
                                                    for (unsigned int l = 0; l < Object::objects.size(); l++) {
                                                        if (Object::objects[l]->model.type == decalstype) {
                                                            Object::objects[l]->model.deleteDeadDecals();
                                                        }
                                                    }
                                                }
                                            }
                                        }
                                    }
                                }
                                if (Animation::animations[Person::players[k]->animTarget].attack == normalattack &&
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
                                                        Person::players[i]->jointPos(neck)) /
                                                       2 *
                                                       Person::players[i]->scale;
                                    }
                                    Person::players[k]->targetyaw = roughDirectionTo(Person::players[k]->coords, targetpoint);
                                    Person::players[k]->targettilt2 = pitchTo(Person::players[k]->coords, targetpoint);

                                    if (Person::players[k]->animTarget == crouchstabanim || Person::players[k]->animTarget == swordgroundstabanim) {
                                        Person::players[k]->targetyaw += (float)(abs(Random() % 100) - 50) / 4;
                                    }

                                    if (Person::players[k]->animTarget == staffgroundsmashanim) {
                                        Person::players[k]->targettilt2 += 10;
                                    }

                                    Person::players[k]->lastattack3 = Person::players[k]->lastattack2;
                                    Person::players[k]->lastattack2 = Person::players[k]->lastattack;
                                    Person::players[k]->lastattack = Person::players[k]->animTarget;

                                    if (Person::players[k]->animTarget == swordgroundstabanim) {
                                        Person::players[k]->targetyaw += 30;
                                    }
                                }
                            }
                        }
                    }
                    if (!Person::players[k]->hasvictim) {
                        //find victim
                        for (unsigned i = 0; i < Person::players.size(); i++) {
                            if (i == k || !(i == 0 || k == 0)) {
                                continue;
                            }
                            if (!Person::players[i]->skeleton.free) {
                                if (Person::players[k]->hasvictim) {
                                    if (distsq(&Person::players[k]->coords, &Person::players[i]->coords) <
                                        distsq(&Person::players[k]->coords, &Person::players[k]->victim->coords)) {
                                        Person::players[k]->victim = Person::players[i];
                                    }
                                } else {
                                    Person::players[k]->victim = Person::players[i];
                                    Person::players[k]->hasvictim = 1;
                                }
                            }
                        }
                    }
                    if (Person::players[k]->isPlayerControlled()) {
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
                              Animation::animations[Person::players[k]->victim->animTarget].height != lowheight &&
                              !Person::players[k]->isPlayerControlled() && //wat???
                              normaldotproduct(Person::players[k]->facing, Person::players[k]->victim->coords - Person::players[k]->coords) > 0 &&
                              Person::players[k]->rabbitkickenabled) ||
                             Person::players[k]->jumpkeydown)) {
                            oldattackkey = 1;
                            Person::players[k]->setTargetAnimation(rabbitkickanim);
                        }
                    }
                    //update counts
                    if (Animation::animations[Person::players[k]->animTarget].attack && k == 0) {
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
    if (Person::players.size() > 1) {
        for (unsigned k = 0; k < Person::players.size(); k++) {
            for (unsigned i = k + 1; i < Person::players.size(); i++) {
                //neither player is part of a reversal
                if ((Animation::animations[Person::players[i]->animTarget].attack != reversed &&
                     Animation::animations[Person::players[i]->animTarget].attack != reversal &&
                     Animation::animations[Person::players[k]->animTarget].attack != reversed &&
                     Animation::animations[Person::players[k]->animTarget].attack != reversal) ||
                    (i != 0 && k != 0)) {
                    if ((Animation::animations[Person::players[i]->animCurrent].attack != reversed &&
                         Animation::animations[Person::players[i]->animCurrent].attack != reversal &&
                         Animation::animations[Person::players[k]->animCurrent].attack != reversed &&
                         Animation::animations[Person::players[k]->animCurrent].attack != reversal) ||
                        (i != 0 && k != 0)) {
                        //neither is sleeping
                        if (Person::players[i]->howactive <= typesleeping && Person::players[k]->howactive <= typesleeping) {
                            if (Person::players[i]->howactive != typesittingwall && Person::players[k]->howactive != typesittingwall) {
                                //in same patch, neither is climbing
                                if (Person::players[i]->whichpatchx == Person::players[k]->whichpatchx &&
                                    Person::players[i]->whichpatchz == Person::players[k]->whichpatchz &&
                                    Person::players[k]->skeleton.oldfree == Person::players[k]->skeleton.free &&
                                    Person::players[i]->skeleton.oldfree == Person::players[i]->skeleton.free &&
                                    Person::players[i]->animTarget != climbanim &&
                                    Person::players[i]->animTarget != hanganim &&
                                    Person::players[k]->animTarget != climbanim &&
                                    Person::players[k]->animTarget != hanganim) {
                                    //players are close (bounding box test)
                                    if (Person::players[i]->coords.y > Person::players[k]->coords.y - 3) {
                                        if (Person::players[i]->coords.y < Person::players[k]->coords.y + 3) {
                                            if (Person::players[i]->coords.x > Person::players[k]->coords.x - 3) {
                                                if (Person::players[i]->coords.x < Person::players[k]->coords.x + 3) {
                                                    if (Person::players[i]->coords.z > Person::players[k]->coords.z - 3) {
                                                        if (Person::players[i]->coords.z < Person::players[k]->coords.z + 3) {
                                                            //spread fire from player to player
                                                            if (distsq(&Person::players[i]->coords, &Person::players[k]->coords) < 3 * sq((Person::players[i]->scale + Person::players[k]->scale) * 2.5)) {
                                                                if (Person::players[i]->onfire || Person::players[k]->onfire) {
                                                                    if (!Person::players[i]->onfire) {
                                                                        Person::players[i]->CatchFire();
                                                                    }
                                                                    if (!Person::players[k]->onfire) {
                                                                        Person::players[k]->CatchFire();
                                                                    }
                                                                }
                                                            }

                                                            XYZ tempcoords1 = Person::players[i]->coords;
                                                            XYZ tempcoords2 = Person::players[k]->coords;
                                                            if (!Person::players[i]->skeleton.oldfree) {
                                                                tempcoords1.y += Person::players[i]->jointPos(abdomen).y * Person::players[i]->scale;
                                                            }
                                                            if (!Person::players[k]->skeleton.oldfree) {
                                                                tempcoords2.y += Person::players[k]->jointPos(abdomen).y * Person::players[k]->scale;
                                                            }
                                                            collisionradius = 1.2 * sq((Person::players[i]->scale + Person::players[k]->scale) * 2.5);
                                                            if (Person::players[0]->hasvictim) {
                                                                if (Person::players[0]->animTarget == rabbitkickanim && (k == 0 || i == 0) && !Person::players[0]->victim->skeleton.free) {
                                                                    collisionradius = 3;
                                                                }
                                                            }
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

                                                                if ((Person::players[i]->skeleton.oldfree == 1 && findLengthfast(&Person::players[i]->velocity) > 1) ||
                                                                    (Person::players[k]->skeleton.oldfree == 1 && findLengthfast(&Person::players[k]->velocity) > 1) ||
                                                                    (Person::players[i]->skeleton.oldfree == 0 && Person::players[k]->skeleton.oldfree == 0)) {
                                                                    rotatetarget = Person::players[k]->velocity - Person::players[i]->velocity;
                                                                    if ((Person::players[i]->animTarget != getupfrombackanim && Person::players[i]->animTarget != getupfromfrontanim ||
                                                                         Person::players[i]->skeleton.free) &&
                                                                        (Person::players[k]->animTarget != getupfrombackanim && Person::players[k]->animTarget != getupfromfrontanim ||
                                                                         Person::players[k]->skeleton.free)) {
                                                                        if ((((k != 0 && findLengthfast(&rotatetarget) > 150 ||
                                                                               k == 0 && findLengthfast(&rotatetarget) > 50 && Person::players[0]->rabbitkickragdoll) &&
                                                                              normaldotproduct(rotatetarget, Person::players[k]->coords - Person::players[i]->coords) > 0) &&
                                                                             (k == 0 ||
                                                                              k != 0 && Person::players[i]->skeleton.oldfree == 1 && Animation::animations[Person::players[k]->animCurrent].attack == neutral ||
                                                                              /*i!=0&&*/ Person::players[k]->skeleton.oldfree == 1 && Animation::animations[Person::players[i]->animCurrent].attack == neutral)) ||
                                                                            (Person::players[i]->animTarget == jumpupanim || Person::players[i]->animTarget == jumpdownanim || Person::players[i]->isFlip()) &&
                                                                                (Person::players[k]->animTarget == jumpupanim || Person::players[k]->animTarget == jumpdownanim || Person::players[k]->isFlip()) &&
                                                                                k == 0 && !Person::players[i]->skeleton.oldfree && !Person::players[k]->skeleton.oldfree) {
                                                                            //If hit by body
                                                                            if ((i != 0 || Person::players[i]->skeleton.free) &&
                                                                                    (k != 0 || Person::players[k]->skeleton.free) ||
                                                                                (Animation::animations[Person::players[i]->animTarget].height == highheight &&
                                                                                 Animation::animations[Person::players[k]->animTarget].height == highheight)) {
                                                                                if (!Tutorial::active) {
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

                                                                                for (unsigned j = 0; j < Person::players[i]->skeleton.joints.size(); j++) {
                                                                                    Person::players[i]->skeleton.joints[j].velocity = Person::players[i]->skeleton.joints[j].velocity / 5 + Person::players[k]->velocity;
                                                                                }
                                                                                for (unsigned j = 0; j < Person::players[k]->skeleton.joints.size(); j++) {
                                                                                    Person::players[k]->skeleton.joints[j].velocity = Person::players[k]->skeleton.joints[j].velocity / 5 + Person::players[i]->velocity;
                                                                                }
                                                                            }
                                                                        }
                                                                    }
                                                                    if ((Animation::animations[Person::players[i]->animTarget].attack == neutral ||
                                                                         Animation::animations[Person::players[i]->animTarget].attack == normalattack) &&
                                                                        (Animation::animations[Person::players[k]->animTarget].attack == neutral ||
                                                                         Animation::animations[Person::players[k]->animTarget].attack == normalattack)) {
                                                                        //If bumped
                                                                        if (Person::players[i]->skeleton.oldfree == 0 && Person::players[k]->skeleton.oldfree == 0) {
                                                                            if (distsq(&Person::players[k]->coords, &Person::players[i]->coords) < .5 * sq((Person::players[i]->scale + Person::players[k]->scale) * 2.5)) {
                                                                                rotatetarget = Person::players[k]->coords - Person::players[i]->coords;
                                                                                Normalise(&rotatetarget);
                                                                                Person::players[k]->coords = (Person::players[k]->coords + Person::players[i]->coords) / 2;
                                                                                Person::players[i]->coords = Person::players[k]->coords - rotatetarget * fast_sqrt(.6) / 2 * sq((Person::players[i]->scale + Person::players[k]->scale) * 2.5);
                                                                                Person::players[k]->coords += rotatetarget * fast_sqrt(.6) / 2 * sq((Person::players[i]->scale + Person::players[k]->scale) * 2.5);
                                                                                if (Person::players[k]->howactive == typeactive || hostile) {
                                                                                    if (Person::players[k]->isIdle()) {
                                                                                        if (Person::players[k]->howactive < typesleeping) {
                                                                                            Person::players[k]->setTargetAnimation(Person::players[k]->getStop());
                                                                                        } else if (Person::players[k]->howactive == typesleeping) {
                                                                                            Person::players[k]->setTargetAnimation(getupfromfrontanim);
                                                                                        }
                                                                                        if (!editorenabled) {
                                                                                            Person::players[k]->howactive = typeactive;
                                                                                        }
                                                                                    }
                                                                                }
                                                                                if (Person::players[i]->howactive == typeactive || hostile) {
                                                                                    if (Person::players[i]->isIdle()) {
                                                                                        if (Person::players[i]->howactive < typesleeping) {
                                                                                            Person::players[i]->setTargetAnimation(Person::players[k]->getStop());
                                                                                        } else {
                                                                                            Person::players[i]->setTargetAnimation(getupfromfrontanim);
                                                                                        }
                                                                                        if (!editorenabled) {
                                                                                            Person::players[i]->howactive = typeactive;
                                                                                        }
                                                                                    }
                                                                                }
                                                                            }
                                                                            //jump down on player
                                                                            if (hostile) {
                                                                                if (k == 0 && i != 0 && Person::players[k]->animTarget == jumpdownanim &&
                                                                                    !Person::players[i]->isCrouch() &&
                                                                                    Person::players[i]->animTarget != rollanim &&
                                                                                    !Person::players[k]->skeleton.oldfree && !Person::players[k]->skeleton.free &&
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
                                            }
                                        }
                                    }
                                }
                            }
                        }
                    }
                }
            }
        }
    }
}

void Game::Tick()
{
    static XYZ facing, flatfacing;
    static int target;

    /* Pump SDL input events and process non-gameplay related ones */
    ProcessInput();

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
        if (mainmenu && endgame == 1) {
            mainmenu = 10;
        }
        //go to level select after completing a campaign level
        if (campaign && winfreeze && mainmenu == 0 && campaignlevels[actuallevel].choosenext == 1) {
            mainmenu = 5;
            gameon = 0;
            winfreeze = 0;
            fireSound();
            flash();
            if (musictoggle) {
                OPENAL_SetFrequency(OPENAL_ALL);
                emit_stream_np(stream_menutheme);
                pause_sound(leveltheme);
            }
            Menu::Load();
        }
    }

    if (mainmenu) {
        Menu::Tick();
    }

    if (!mainmenu) {
        if (hostile == 1) {
            hostiletime += multiplier;
        } else {
            hostiletime = 0;
        }
        if (!winfreeze) {
            leveltime += multiplier;
        }

        if (console) {
            freeze = 1;

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
            OPENAL_SetFrequency(OPENAL_ALL);
            emit_sound_np(consolesuccesssound);
        }
        if (winfreeze == 0) {
            oldwinfreeze = winfreeze;
        } else {
            oldwinfreeze++;
        }

        if (!freeze && !winfreeze && !(mainmenu && gameon) && (gameon || !gamestarted)) {

            //dialogues
            static float talkdelay = 0;

            if (Dialog::inDialog()) {
                talkdelay = 1;
            }
            talkdelay -= multiplier;

            if (talkdelay <= 0 && !Dialog::inDialog() && Animation::animations[Person::players[0]->animTarget].height != highheight) {
                for (unsigned i = 0; i < Dialog::dialogs.size(); i++) {
                    Dialog::dialogs[i].tick(i);
                }
            }

            windvar += multiplier;
            smoketex += multiplier;
            Tutorial::stagetime += multiplier;

            //hotspots
            static float hotspotvisual[40];
            if (Hotspot::hotspots.size()) {
                XYZ hotspotsprite;
                if (editorenabled) {
                    for (unsigned i = 0; i < Hotspot::hotspots.size(); i++) {
                        hotspotvisual[i] -= multiplier / 320;
                    }
                }

                for (unsigned i = 0; i < Hotspot::hotspots.size(); i++) {
                    while (hotspotvisual[i] < 0) {
                        hotspotsprite = 0;
                        hotspotsprite.x = float(abs(Random() % 100000)) / 100000 * Hotspot::hotspots[i].size;
                        hotspotsprite = DoRotation(hotspotsprite, 0, 0, Random() % 360);
                        hotspotsprite = DoRotation(hotspotsprite, 0, Random() % 360, 0);
                        hotspotsprite += Hotspot::hotspots[i].position;
                        Sprite::MakeSprite(breathsprite, hotspotsprite, hotspotsprite * 0, 1, 0.5, 0, 7, 0.4);
                        hotspotvisual[i] += 0.1 / Hotspot::hotspots[i].size / Hotspot::hotspots[i].size / Hotspot::hotspots[i].size;
                    }
                }

                for (unsigned i = 0; i < Hotspot::hotspots.size(); i++) {
                    if (Hotspot::hotspots[i].type <= 10 && Hotspot::hotspots[i].type > 0) {
                        Hotspot::hotspots[i].position = Person::players[Hotspot::hotspots[i].type]->coords;
                    }
                }
            }

            //Tutorial
            if (Tutorial::active) {
                Tutorial::Do(multiplier);
            }

            //bonuses
            if (!Tutorial::active) {
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
                    bonus != megacombo) {
                    bonusnum[bonus]++;
                } else {
                    bonusnum[bonus] += 0.15;
                }
                if (Tutorial::active) {
                    bonusvalue = 0;
                }
                if (bonusvalue > 0) {
                    bonusvalue /= bonusnum[bonus];
                    if (bonusvalue <= 0) {
                        bonusvalue = 0;
                    }
                }
                bonustotal += bonusvalue;
            }
            bonustime += multiplier;

            //snow effects
            if (environment == snowyenvironment) {
                precipdelay -= multiplier;
                while (precipdelay < 0) {
                    precipdelay += .04;
                    if (!detail) {
                        precipdelay += .04;
                    }
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
            if (!Dialog::inDialog()) {
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

            if (!Person::players[0]->jumpkeydown) {
                Person::players[0]->jumpclimb = 0;
            }

            if (Dialog::inDialog()) {
                cameramode = 1;
                if (Dialog::directing) {
                    facing = 0;
                    facing.z = -1;

                    facing = DoRotation(facing, -pitch, 0, 0);
                    facing = DoRotation(facing, 0, 0 - yaw, 0);

                    flatfacing = 0;
                    flatfacing.z = -1;

                    flatfacing = DoRotation(flatfacing, 0, -yaw, 0);

                    if (Input::isKeyDown(forwardkey)) {
                        viewer += facing * multiplier * 4;
                    }
                    if (Input::isKeyDown(backkey)) {
                        viewer -= facing * multiplier * 4;
                    }
                    if (Input::isKeyDown(leftkey)) {
                        viewer += DoRotation(flatfacing * multiplier, 0, 90, 0) * 4;
                    }
                    if (Input::isKeyDown(rightkey)) {
                        viewer += DoRotation(flatfacing * multiplier, 0, -90, 0) * 4;
                    }
                    if (Input::isKeyDown(jumpkey)) {
                        viewer.y += multiplier * 4;
                    }
                    if (Input::isKeyDown(crouchkey)) {
                        viewer.y -= multiplier * 4;
                    }
                    if (Input::isKeyPressed(SDL_SCANCODE_1) ||
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
                        if (Input::isKeyPressed(SDL_SCANCODE_1)) {
                            whichend = 1;
                        }
                        if (Input::isKeyPressed(SDL_SCANCODE_2)) {
                            whichend = 2;
                        }
                        if (Input::isKeyPressed(SDL_SCANCODE_3)) {
                            whichend = 3;
                        }
                        if (Input::isKeyPressed(SDL_SCANCODE_4)) {
                            whichend = 4;
                        }
                        if (Input::isKeyPressed(SDL_SCANCODE_5)) {
                            whichend = 5;
                        }
                        if (Input::isKeyPressed(SDL_SCANCODE_6)) {
                            whichend = 6;
                        }
                        if (Input::isKeyPressed(SDL_SCANCODE_7)) {
                            whichend = 7;
                        }
                        if (Input::isKeyPressed(SDL_SCANCODE_8)) {
                            whichend = 8;
                        }
                        if (Input::isKeyPressed(SDL_SCANCODE_9)) {
                            whichend = 9;
                        }
                        if (Input::isKeyPressed(SDL_SCANCODE_0)) {
                            whichend = 0;
                        }
                        if (Input::isKeyPressed(SDL_SCANCODE_MINUS)) {
                            whichend = -1;
                        }
                        if (whichend != -1) {
                            Dialog::currentScene().participantfocus = whichend;
                            Dialog::currentDialog().participantlocation[whichend] = Person::players[whichend]->coords;
                            Dialog::currentDialog().participantyaw[whichend] = Person::players[whichend]->yaw;
                        }
                        if (whichend == -1) {
                            Dialog::currentScene().participantfocus = -1;
                        }
                        /* FIXME: potentially accessing -1 in Person::players! */
                        if (Person::players[Dialog::currentScene().participantfocus]->dead) {
                            Dialog::indialogue = -1;
                            Dialog::directing = false;
                            cameramode = 0;
                        }
                        Dialog::currentScene().camera = viewer;
                        Dialog::currentScene().camerayaw = yaw;
                        Dialog::currentScene().camerapitch = pitch;
                        Dialog::indialogue++;
                        if (Dialog::indialogue < int(Dialog::currentDialog().scenes.size())) {
                            if (Dialog::currentScene().sound != 0) {
                                playdialoguescenesound();
                            }
                        }

                        for (unsigned j = 0; j < Person::players.size(); j++) {
                            Dialog::currentScene().participantfacing[j] = Dialog::currentDialog().scenes[Dialog::indialogue - 1].participantfacing[j];
                        }
                    }
                    //TODO: should these be KeyDown or KeyPressed?
                    if (Input::isKeyDown(SDL_SCANCODE_KP_1) ||
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
                        if (Input::isKeyDown(SDL_SCANCODE_KP_1)) {
                            whichend = 1;
                        }
                        if (Input::isKeyDown(SDL_SCANCODE_KP_2)) {
                            whichend = 2;
                        }
                        if (Input::isKeyDown(SDL_SCANCODE_KP_3)) {
                            whichend = 3;
                        }
                        if (Input::isKeyDown(SDL_SCANCODE_KP_4)) {
                            whichend = 4;
                        }
                        if (Input::isKeyDown(SDL_SCANCODE_KP_5)) {
                            whichend = 5;
                        }
                        if (Input::isKeyDown(SDL_SCANCODE_KP_6)) {
                            whichend = 6;
                        }
                        if (Input::isKeyDown(SDL_SCANCODE_KP_7)) {
                            whichend = 7;
                        }
                        if (Input::isKeyDown(SDL_SCANCODE_KP_8)) {
                            whichend = 8;
                        }
                        if (Input::isKeyDown(SDL_SCANCODE_KP_9)) {
                            whichend = 9;
                        }
                        if (Input::isKeyDown(SDL_SCANCODE_KP_0)) {
                            whichend = 0;
                        }
                        Dialog::currentScene().participantfacing[whichend] = facing;
                    }
                    if (Dialog::indialogue >= int(Dialog::currentDialog().scenes.size())) {
                        Dialog::indialogue = -1;
                        Dialog::directing = false;
                        cameramode = 0;
                    }
                }
                if (!Dialog::directing) {
                    pause_sound(whooshsound);
                    viewer = Dialog::currentScene().camera;
                    viewer.y = max((double)viewer.y, terrain.getHeight(viewer.x, viewer.z) + .1);
                    yaw = Dialog::currentScene().camerayaw;
                    pitch = Dialog::currentScene().camerapitch;
                    if (Dialog::dialoguetime > 0.5) {
                        if (Input::isKeyPressed(attackkey)) {
                            Dialog::indialogue++;
                            if (Dialog::indialogue < int(Dialog::currentDialog().scenes.size())) {
                                if (Dialog::currentScene().sound != 0) {
                                    playdialoguescenesound();
                                    if (Dialog::currentScene().sound == -5) {
                                        Hotspot::hotspots.emplace_back(Person::players[0]->coords, -1, 10);
                                    }
                                    if (Dialog::currentScene().sound == -6) {
                                        hostile = 1;
                                    }

                                    if (Person::players.at(Dialog::currentScene().participantfocus)->dead) {
                                        Dialog::indialogue = -1;
                                        Dialog::directing = false;
                                        cameramode = 0;
                                    }
                                }
                            }
                        }
                    }
                    if (Dialog::indialogue >= int(Dialog::currentDialog().scenes.size())) {
                        Dialog::indialogue = -1;
                        Dialog::directing = false;
                        cameramode = 0;
                        if (Dialog::currentDialog().type > 19 && Dialog::currentDialog().type < 30) {
                            hostile = 1;
                        }
                        if (Dialog::currentDialog().type > 29 && Dialog::currentDialog().type < 40) {
                            windialogue = true;
                        }
                        if (Dialog::currentDialog().type > 49 && Dialog::currentDialog().type < 60) {
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
                !Person::players[0]->isFlip()) {
                Person::players[0]->jumptogglekeydown = 1;
            }

            Dialog::dialoguetime += multiplier;
            hawkyaw += multiplier * 25;
            realhawkcoords = 0;
            realhawkcoords.x = 25;
            realhawkcoords = DoRotation(realhawkcoords, 0, hawkyaw, 0) + hawkcoords;
            hawkcalldelay -= multiplier / 2;

            if (hawkcalldelay <= 0) {
                emit_sound_at(hawksound, realhawkcoords);

                hawkcalldelay = 16 + abs(Random() % 8);
            }

            doAttacks();

            doPlayerCollisions();

            doJumpReversals();

            for (unsigned k = 0; k < Person::players.size(); k++) {
                if (k != 0 && Person::players[k]->immobile) {
                    Person::players[k]->coords = Person::players[k]->realoldcoords;
                }
            }

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
                     (Input::isKeyDown(SDL_SCANCODE_K) &&
                      Input::isKeyDown(SDL_SCANCODE_LALT) &&
                      devtools) ||
                 (Input::isKeyDown(jumpkey) &&
                  !respawnkeydown &&
                  !oldattackkey &&
                  Person::players[0]->dead))) {
                targetlevel = whichlevel;
                loading = 1;
                leveltime = 5;
            }
            respawnkeydown = Input::isKeyDown(jumpkey);

            static bool movekey;

            //?
            for (unsigned i = 0; i < Person::players.size(); i++) {
                static float oldtargetyaw;
                if (!Person::players[i]->skeleton.free) {
                    oldtargetyaw = Person::players[i]->targetyaw;
                    if (i == 0 && !Dialog::inDialog()) {
                        //TODO: refactor repetitive code
                        if (!Animation::animations[Person::players[0]->animTarget].attack &&
                            Person::players[0]->animTarget != staggerbackhighanim &&
                            Person::players[0]->animTarget != staggerbackhardanim &&
                            Person::players[0]->animTarget != crouchremoveknifeanim &&
                            Person::players[0]->animTarget != removeknifeanim &&
                            Person::players[0]->animTarget != backhandspringanim &&
                            Person::players[0]->animTarget != dodgebackanim &&
                            Person::players[0]->animTarget != walljumprightkickanim &&
                            Person::players[0]->animTarget != walljumpleftkickanim) {
                            if (cameramode) {
                                Person::players[0]->targetyaw = 0;
                            } else {
                                Person::players[0]->targetyaw = -yaw + 180;
                            }
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
                    if (i != 0 && Person::players[i]->isPlayerControlled() && !Dialog::inDialog()) {
                        if (!Animation::animations[Person::players[i]->animTarget].attack &&
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
                    if (Dialog::inDialog()) {
                        Person::players[i]->targetheadyaw = 180 - roughDirection(Dialog::currentScene().participantfacing[i]);
                        Person::players[i]->targetheadpitch = pitchOf(Dialog::currentScene().participantfacing[i]);
                    }

                    if (leveltime < .5) {
                        numenvsounds = 0;
                    }

                    Person::players[i]->avoidsomething = 0;

                    //avoid flaming things
                    for (unsigned int j = 0; j < Object::objects.size(); j++) {
                        if (Object::objects[j]->onfire) {
                            if (distsq(&Person::players[i]->coords, &Object::objects[j]->position) < sq(Object::objects[j]->scale) * 200) {
                                if (distsq(&Person::players[i]->coords, &Object::objects[j]->position) <
                                    distsq(&Person::players[i]->coords, &Person::players[0]->coords)) {
                                    Person::players[i]->collided = 0;
                                    Person::players[i]->avoidcollided = 1;
                                    if (Person::players[i]->avoidsomething == 0 ||
                                        distsq(&Person::players[i]->coords, &Object::objects[j]->position) <
                                            distsq(&Person::players[i]->coords, &Person::players[i]->avoidwhere)) {
                                        Person::players[i]->avoidwhere = Object::objects[j]->position;
                                        Person::players[i]->avoidsomething = 1;
                                    }
                                }
                            }
                        }
                    }

                    //avoid flaming players
                    for (unsigned j = 0; j < Person::players.size(); j++) {
                        if (Person::players[j]->onfire) {
                            if (distsq(&Person::players[j]->coords, &Person::players[i]->coords) < sq(0.3) * 200) {
                                if (distsq(&Person::players[i]->coords, &Person::players[j]->coords) <
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
                            }
                        }
                    }

                    if (Person::players[i]->collided > .8) {
                        Person::players[i]->avoidcollided = 0;
                    }

                    Person::players[i]->doAI();

                    if (Animation::animations[Person::players[i]->animTarget].attack == reversed) {
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

                    if (Dialog::inDialog()) {
                        Person::players[i]->forwardkeydown = 0;
                        Person::players[i]->leftkeydown = 0;
                        Person::players[i]->backkeydown = 0;
                        Person::players[i]->rightkeydown = 0;
                        Person::players[i]->jumpkeydown = 0;
                        Person::players[i]->crouchkeydown = 0;
                        Person::players[i]->drawkeydown = 0;
                        Person::players[i]->throwkeydown = 0;
                    }

                    if (Person::players[i]->collided < -.3) {
                        Person::players[i]->collided = -.3;
                    }
                    if (Person::players[i]->collided > 1) {
                        Person::players[i]->collided = 1;
                    }
                    Person::players[i]->collided -= multiplier * 4;
                    Person::players[i]->whichdirectiondelay -= multiplier;
                    if (Person::players[i]->avoidcollided < -.3 || Person::players[i]->whichdirectiondelay <= 0) {
                        Person::players[i]->avoidcollided = -.3;
                        Person::players[i]->whichdirection = abs(Random() % 2);
                        Person::players[i]->whichdirectiondelay = .4;
                    }
                    if (Person::players[i]->avoidcollided > 1) {
                        Person::players[i]->avoidcollided = 1;
                    }
                    Person::players[i]->avoidcollided -= multiplier / 4;
                    if (!Person::players[i]->skeleton.free) {
                        Person::players[i]->stunned -= multiplier;
                        Person::players[i]->surprised -= multiplier;
                    }
                    if (i != 0 && Person::players[i]->surprised <= 0 &&
                        Person::players[i]->aitype == attacktypecutoff &&
                        !Person::players[i]->dead &&
                        !Person::players[i]->skeleton.free &&
                        Animation::animations[Person::players[i]->animTarget].attack == neutral) {
                        numresponded = 1;
                    }

                    if (!Person::players[i]->throwkeydown) {
                        Person::players[i]->throwtogglekeydown = 0;
                    }

                    //pick up weapon
                    if (Person::players[i]->throwkeydown && !Person::players[i]->throwtogglekeydown) {
                        if (!Person::players[i]->hasWeapon() &&
                            Person::players[i]->num_weapons < 2 &&
                            (Person::players[i]->isIdle() ||
                             Person::players[i]->isCrouch() ||
                             Person::players[i]->animTarget == sneakanim ||
                             Person::players[i]->animTarget == rollanim ||
                             Person::players[i]->animTarget == backhandspringanim ||
                             Person::players[i]->isFlip() ||
                             !Person::players[i]->isPlayerControlled())) {
                            for (unsigned j = 0; j < weapons.size(); j++) {
                                if ((weapons[j].velocity.x == 0 && weapons[j].velocity.y == 0 && weapons[j].velocity.z == 0 ||
                                     Person::players[i]->isPlayerControlled()) &&
                                    weapons[j].owner == -1 &&
                                    !Person::players[i]->hasWeapon()) {
                                    if (distsqflat(&Person::players[i]->coords, &weapons[j].position) < 2) {
                                        if (distsq(&Person::players[i]->coords, &weapons[j].position) < 2) {
                                            if (Person::players[i]->isCrouch() ||
                                                Person::players[i]->animTarget == sneakanim ||
                                                Person::players[i]->isRun() ||
                                                Person::players[i]->isIdle() ||
                                                !Person::players[i]->isPlayerControlled()) {
                                                Person::players[i]->throwtogglekeydown = 1;
                                                Person::players[i]->setTargetAnimation(crouchremoveknifeanim);
                                                Person::players[i]->targetyaw = roughDirectionTo(Person::players[i]->coords, weapons[j].position);
                                                Person::players[i]->hasvictim = 0;
                                            }
                                            if (Person::players[i]->animTarget == rollanim || Person::players[i]->animTarget == backhandspringanim) {
                                                Person::players[i]->throwtogglekeydown = 1;
                                                Person::players[i]->hasvictim = 0;

                                                if ((weapons[j].velocity.x == 0 && weapons[j].velocity.y == 0 && weapons[j].velocity.z == 0 ||
                                                     Person::players[i]->isPlayerControlled()) &&
                                                        weapons[j].owner == -1 ||
                                                    Person::players[i]->victim &&
                                                        weapons[j].owner == int(Person::players[i]->victim->id)) {
                                                    if (distsqflat(&Person::players[i]->coords, &weapons[j].position) < 2 && !Person::players[i]->hasWeapon()) {
                                                        if (distsq(&Person::players[i]->coords, &weapons[j].position) < 1 || Person::players[i]->victim) {
                                                            if (weapons[j].getType() != staff) {
                                                                emit_sound_at(knifedrawsound, Person::players[i]->coords, 128.);
                                                            }

                                                            Person::players[i]->takeWeapon(j);
                                                        }
                                                    }
                                                }
                                            }
                                        } else if ((Person::players[i]->isIdle() ||
                                                    Person::players[i]->isFlip() ||
                                                    !Person::players[i]->isPlayerControlled()) &&
                                                   distsq(&Person::players[i]->coords, &weapons[j].position) < 5 &&
                                                   Person::players[i]->coords.y < weapons[j].position.y) {
                                            if (!Person::players[i]->isFlip()) {
                                                Person::players[i]->throwtogglekeydown = 1;
                                                Person::players[i]->setTargetAnimation(removeknifeanim);
                                                Person::players[i]->targetyaw = roughDirectionTo(Person::players[i]->coords, weapons[j].position);
                                            }
                                            if (Person::players[i]->isFlip()) {
                                                Person::players[i]->throwtogglekeydown = 1;
                                                Person::players[i]->hasvictim = 0;

                                                for (unsigned k = 0; k < weapons.size(); k++) {
                                                    if (!Person::players[i]->hasWeapon()) {
                                                        if ((weapons[k].velocity.x == 0 && weapons[k].velocity.y == 0 && weapons[k].velocity.z == 0 ||
                                                             Person::players[i]->isPlayerControlled()) &&
                                                                weapons[k].owner == -1 ||
                                                            Person::players[i]->victim &&
                                                                weapons[k].owner == int(Person::players[i]->victim->id)) {
                                                            if (distsqflat(&Person::players[i]->coords, &weapons[k].position) < 3 &&
                                                                !Person::players[i]->hasWeapon()) {
                                                                if (weapons[k].getType() != staff) {
                                                                    emit_sound_at(knifedrawsound, Person::players[i]->coords, 128.);
                                                                }

                                                                Person::players[i]->takeWeapon(k);
                                                            }
                                                        }
                                                    }
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
                                if (Person::players.size() > 1) {
                                    for (unsigned j = 0; j < Person::players.size(); j++) {
                                        if (!Person::players[i]->hasWeapon()) {
                                            if (j != i) {
                                                if (Person::players[j]->num_weapons &&
                                                    Person::players[j]->skeleton.free &&
                                                    distsq(&Person::players[i]->coords, &Person::players[j]->coords) < 2 /*&&Person::players[j]->dead*/ &&
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
                                                        Person::players[i]->setTargetAnimation(crouchremoveknifeanim);
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
                                                                if (weapons[k].getType() != staff) {
                                                                    emit_sound_at(knifedrawsound, Person::players[i]->coords, 128.);
                                                                }
                                                            }
                                                            if (fleshstuck) {
                                                                emit_sound_at(fleshstabremovesound, Person::players[i]->coords, 128.);
                                                            }

                                                            if (weapons[k].owner != -1) {
                                                                if (Person::players[i]->victim->num_weapons == 1) {
                                                                    Person::players[i]->victim->num_weapons = 0;
                                                                } else {
                                                                    Person::players[i]->victim->num_weapons = 1;
                                                                }

                                                                Person::players[i]->victim->skeleton.longdead = 0;
                                                                Person::players[i]->victim->skeleton.free = 1;
                                                                Person::players[i]->victim->skeleton.broken = 0;

                                                                for (unsigned l = 0; l < Person::players[i]->victim->skeleton.joints.size(); l++) {
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
                                                                        if (bloodtoggle) {
                                                                            Sprite::MakeSprite(cloudimpactsprite, footpoint, footvel, 1, 0, 0, .8, .3);
                                                                        }
                                                                        weapons[k].bloody = 2;
                                                                        weapons[k].blooddrip = 5;
                                                                        Person::players[i]->victim->weaponstuck = -1;
                                                                        Person::players[i]->victim->bloodloss += 2000;
                                                                        Person::players[i]->victim->DoDamage(2000);
                                                                    }
                                                                }
                                                                if (Person::players[i]->victim->num_weapons > 0) {
                                                                    if (Person::players[i]->victim->weaponstuck != 0 && Person::players[i]->victim->weaponstuck != -1) {
                                                                        Person::players[i]->victim->weaponstuck = 0;
                                                                    }
                                                                    if (Person::players[i]->victim->weaponids[0] == k) {
                                                                        Person::players[i]->victim->weaponids[0] = Person::players[i]->victim->weaponids[Person::players[i]->victim->num_weapons];
                                                                    }
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
                                }
                            }
                        }
                        if (Person::players[i]->hasWeapon() && Person::players[i]->isPlayerControlled()) {
                            if (weapons[Person::players[i]->weaponids[0]].getType() == knife) {
                                if (Person::players[i]->isIdle() ||
                                    Person::players[i]->isRun() ||
                                    Person::players[i]->isCrouch() ||
                                    Person::players[i]->animTarget == sneakanim ||
                                    Person::players[i]->isFlip()) {
                                    if (Person::players.size() > 1) {
                                        for (unsigned j = 0; j < Person::players.size(); j++) {
                                            if (i != j) {
                                                if (!Tutorial::active || Tutorial::stage == 49) {
                                                    if (hostile) {
                                                        if (normaldotproduct(Person::players[i]->facing, Person::players[i]->coords - Person::players[j]->coords) < 0 &&
                                                            distsq(&Person::players[i]->coords, &Person::players[j]->coords) < 100 &&
                                                            distsq(&Person::players[i]->coords, &Person::players[j]->coords) > 1.5 &&
                                                            !Person::players[j]->skeleton.free &&
                                                            -1 == Object::checkcollide(DoRotation(Person::players[j]->jointPos(head), 0, Person::players[j]->yaw, 0) * Person::players[j]->scale + Person::players[j]->coords, DoRotation(Person::players[i]->jointPos(head), 0, Person::players[i]->yaw, 0) * Person::players[i]->scale + Person::players[i]->coords)) {
                                                            if (!Person::players[i]->isFlip()) {
                                                                Person::players[i]->throwtogglekeydown = 1;
                                                                Person::players[i]->victim = Person::players[j];
                                                                Person::players[i]->setTargetAnimation(knifethrowanim);
                                                                Person::players[i]->targetyaw = roughDirectionTo(Person::players[i]->coords, Person::players[j]->coords);
                                                                Person::players[i]->targettilt2 = pitchTo(Person::players[i]->coords, Person::players[j]->coords);
                                                            }
                                                            if (Person::players[i]->isFlip()) {
                                                                if (Person::players[i]->hasWeapon()) {
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
                                        }
                                    }
                                }
                            }
                        }
                        if (Person::players[i]->hasWeapon() && Person::players[i]->isPlayerControlled()) {
                            if (Person::players[i]->isCrouch() || Person::players[i]->animTarget == sneakanim) {
                                Person::players[i]->throwtogglekeydown = 1;
                                XYZ tempVelocity = Person::players[i]->velocity * .2;
                                if (tempVelocity.x == 0) {
                                    tempVelocity.x = .1;
                                }
                                weapons[Person::players[i]->weaponids[0]].drop(tempVelocity, tempVelocity, false);
                                Person::players[i]->num_weapons--;
                                if (Person::players[i]->num_weapons) {
                                    Person::players[i]->weaponids[0] = Person::players[i]->weaponids[Person::players[i]->num_weapons];
                                    if (Person::players[i]->weaponstuck == Person::players[i]->num_weapons) {
                                        Person::players[i]->weaponstuck = 0;
                                    }
                                }

                                Person::players[i]->weaponactive = -1;
                                for (unsigned j = 0; j < Person::players.size(); j++) {
                                    Person::players[j]->wentforweapon = 0;
                                }
                            }
                        }
                    }

                    //draw weapon
                    if (i == 0 || !Person::players[0]->dead || (Person::players[i]->hasWeapon())) {
                        if (Person::players[i]->drawkeydown && !Person::players[i]->drawtogglekeydown ||
                            (Person::players[i]->num_weapons == 2) &&
                                (!Person::players[i]->hasWeapon()) &&
                                Person::players[i]->isIdle() ||
                            Person::players[0]->dead &&
                                (Person::players[i]->hasWeapon()) &&
                                i != 0) {
                            bool isgood = true;
                            if (Person::players[i]->hasWeapon()) {
                                if (weapons[Person::players[i]->weaponids[Person::players[i]->weaponactive]].getType() == staff) {
                                    isgood = false;
                                }
                            }
                            if (isgood && Person::players[i]->creature != wolftype) {
                                if (Person::players[i]->isIdle() && Person::players[i]->num_weapons && weapons[Person::players[i]->weaponids[0]].getType() == knife) {
                                    Person::players[i]->setTargetAnimation(drawrightanim);
                                    Person::players[i]->drawtogglekeydown = 1;
                                }
                                if ((Person::players[i]->isIdle() ||
                                     (!Person::players[i]->isPlayerControlled() &&
                                      Person::players[0]->hasWeapon() &&
                                      Person::players[i]->isRun())) &&
                                    Person::players[i]->num_weapons &&
                                    weapons[Person::players[i]->weaponids[0]].getType() == sword) {
                                    Person::players[i]->setTargetAnimation(drawleftanim);
                                    Person::players[i]->drawtogglekeydown = 1;
                                }
                                if (Person::players[i]->isCrouch() && Person::players[i]->num_weapons && weapons[Person::players[i]->weaponids[0]].getType() == knife) {
                                    Person::players[i]->setTargetAnimation(crouchdrawrightanim);
                                    Person::players[i]->drawtogglekeydown = 1;
                                }
                            }
                        }
                    }

                    //clean weapon
                    if (Person::players[i]->hasWeapon()) {
                        if (Person::players[i]->isCrouch() &&
                            weapons[Person::players[i]->weaponids[Person::players[i]->weaponactive]].bloody &&
                            bloodtoggle &&
                            Person::players[i]->onterrain &&
                            Person::players[i]->num_weapons &&
                            Person::players[i]->attackkeydown &&
                            musictype != stream_fighttheme) {
                            if (weapons[Person::players[i]->weaponids[Person::players[i]->weaponactive]].getType() == knife) {
                                Person::players[i]->setTargetAnimation(crouchstabanim);
                            }
                            if (weapons[Person::players[i]->weaponids[Person::players[i]->weaponactive]].getType() == sword) {
                                Person::players[i]->setTargetAnimation(swordgroundstabanim);
                            }
                            Person::players[i]->hasvictim = 0;
                        }
                    }

                    if (!Person::players[i]->drawkeydown) {
                        Person::players[i]->drawtogglekeydown = 0;
                    }

                    XYZ absflatfacing;
                    if (i == 0) {
                        absflatfacing = 0;
                        absflatfacing.z = -1;

                        absflatfacing = DoRotation(absflatfacing, 0, -yaw, 0);
                    } else {
                        absflatfacing = flatfacing;
                    }

                    if (Dialog::inDialog()) {
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
                    if (!Animation::animations[Person::players[i]->animTarget].attack &&
                        Person::players[i]->animTarget != staggerbackhighanim &&
                        Person::players[i]->animTarget != staggerbackhardanim &&
                        Person::players[i]->animTarget != backhandspringanim &&
                        Person::players[i]->animTarget != dodgebackanim) {
                        if (!Person::players[i]->forwardkeydown) {
                            Person::players[i]->forwardstogglekeydown = 0;
                        }
                        if (Person::players[i]->crouchkeydown) {
                            //Crouch
                            target = -2;
                            if (i == 0) {
                                Person::players[i]->superruntoggle = 1;
                                if (Person::players.size() > 1) {
                                    for (unsigned j = 0; j < Person::players.size(); j++) {
                                        if (j != i && !Person::players[j]->skeleton.free && Person::players[j]->aitype == passivetype) {
                                            if (distsq(&Person::players[j]->coords, &Person::players[i]->coords) < 16) {
                                                Person::players[i]->superruntoggle = 0;
                                            }
                                        }
                                    }
                                }
                            }

                            if (Person::players.size() > 1) {
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
                                            if (target >= 0) {
                                                target = -1;
                                            } else {
                                                target = j;
                                            }
                                        }
                                    }
                                }
                            }
                            if (target >= 0) {
                                Person::players[target]->Reverse();
                            }
                            Person::players[i]->lowreversaldelay = .5;

                            if (Person::players[i]->isIdle()) {
                                Person::players[i]->setTargetAnimation(Person::players[i]->getCrouch());
                                Person::players[i]->transspeed = 10;
                            }
                            if (Person::players[i]->isRun() ||
                                (Person::players[i]->isStop() &&
                                 (Person::players[i]->leftkeydown ||
                                  Person::players[i]->rightkeydown ||
                                  Person::players[i]->forwardkeydown ||
                                  Person::players[i]->backkeydown))) {
                                Person::players[i]->setTargetAnimation(rollanim);
                                Person::players[i]->transspeed = 20;
                            }
                        }
                        if (!Person::players[i]->crouchkeydown) {
                            //Uncrouch
                            if (!Person::players[i]->isRun() && Person::players[i]->animTarget != sneakanim && i == 0) {
                                Person::players[i]->superruntoggle = 0;
                            }
                            target = -2;
                            if (Person::players[i]->isCrouch()) {
                                if (Person::players.size() > 1) {
                                    for (unsigned j = 0; j < Person::players.size(); j++) {
                                        if (j != i &&
                                            !Person::players[j]->skeleton.free &&
                                            Person::players[j]->victim &&
                                            Person::players[i]->highreversaldelay <= 0) {
                                            if (distsq(&Person::players[j]->coords, &Person::players[j]->victim->coords) < 3 &&
                                                Person::players[j]->victim == Person::players[i] &&
                                                (Person::players[j]->animTarget == spinkickanim) &&
                                                Person::players[i]->isCrouch()) {
                                                if (target >= 0) {
                                                    target = -1;
                                                } else {
                                                    target = j;
                                                }
                                            }
                                        }
                                    }
                                }
                                if (target >= 0) {
                                    Person::players[target]->Reverse();
                                }
                                Person::players[i]->highreversaldelay = .5;

                                if (Person::players[i]->isCrouch()) {
                                    if (!Person::players[i]->wasCrouch()) {
                                        Person::players[i]->animCurrent = Person::players[i]->getCrouch();
                                        Person::players[i]->frameCurrent = 0;
                                    }
                                    Person::players[i]->setTargetAnimation(Person::players[i]->getIdle());
                                    Person::players[i]->transspeed = 10;
                                }
                            }
                            if (Person::players[i]->animTarget == sneakanim) {
                                Person::players[i]->setTargetAnimation(Person::players[i]->getIdle());
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
                                if (Person::players[i]->aitype == passivetype) {
                                    Person::players[i]->setTargetAnimation(walkanim);
                                } else {
                                    Person::players[i]->setTargetAnimation(Person::players[i]->getRun());
                                }
                            }
                            if (Person::players[i]->isCrouch()) {
                                Person::players[i]->animTarget = sneakanim;
                                if (Person::players[i]->wasCrouch()) {
                                    Person::players[i]->target = 0;
                                }
                                Person::players[i]->frameTarget = 0;
                            }
                            if (Person::players[i]->animTarget == hanganim /*&&(!Person::players[i]->forwardstogglekeydown||!Person::players[i]->isPlayerControlled())*/) {
                                Person::players[i]->setTargetAnimation(climbanim);
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
                                Person::players[i]->setTargetAnimation(Person::players[i]->getRun());
                            }
                            if (Person::players[i]->isCrouch()) {
                                Person::players[i]->animTarget = sneakanim;
                                if (Person::players[i]->wasCrouch()) {
                                    Person::players[i]->target = 0;
                                }
                                Person::players[i]->frameTarget = 0;
                            }
                            if (Person::players[i]->animTarget == jumpupanim || Person::players[i]->animTarget == jumpdownanim || Person::players[i]->isFlip()) {
                                Person::players[i]->velocity += DoRotation(absflatfacing * 5 * multiplier, 0, -90, 0);
                            }
                            Person::players[i]->targetyaw -= 90;
                            if (Person::players[i]->forwardkeydown) {
                                Person::players[i]->targetyaw += 45;
                            }
                            if (Person::players[i]->backkeydown) {
                                Person::players[i]->targetyaw -= 45;
                            }
                            movekey = 1;
                        }
                        if (Person::players[i]->leftkeydown) {
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
                                Person::players[i]->setTargetAnimation(Person::players[i]->getRun());
                            }
                            if (Person::players[i]->isCrouch()) {
                                Person::players[i]->animTarget = sneakanim;
                                if (Person::players[i]->wasCrouch()) {
                                    Person::players[i]->target = 0;
                                }
                                Person::players[i]->frameTarget = 0;
                            }
                            if (Person::players[i]->animTarget == jumpupanim || Person::players[i]->animTarget == jumpdownanim || Person::players[i]->isFlip()) {
                                Person::players[i]->velocity -= DoRotation(absflatfacing * 5 * multiplier, 0, -90, 0);
                            }
                            Person::players[i]->targetyaw += 90;
                            if (Person::players[i]->forwardkeydown) {
                                Person::players[i]->targetyaw -= 45;
                            }
                            if (Person::players[i]->backkeydown) {
                                Person::players[i]->targetyaw += 45;
                            }
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
                                Person::players[i]->setTargetAnimation(Person::players[i]->getRun());
                            }
                            if (Person::players[i]->isCrouch()) {
                                Person::players[i]->animTarget = sneakanim;
                                if (Person::players[i]->wasCrouch()) {
                                    Person::players[i]->target = 0;
                                }
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
                            if (!Person::players[i]->leftkeydown && !Person::players[i]->rightkeydown) {
                                Person::players[i]->targetyaw += 180;
                            }
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
                                  Person::players[i]->animTarget != wolfrunninganim) ||
                                 i != 0)) {
                                Person::players[i]->jumpstart = 0;
                                Person::players[i]->setTargetAnimation(jumpupanim);
                                Person::players[i]->yaw = Person::players[i]->targetyaw;
                                Person::players[i]->transspeed = 20;
                                Person::players[i]->FootLand(leftfoot, 1);
                                Person::players[i]->FootLand(rightfoot, 1);

                                facing = 0;
                                facing.z = -1;
                                flatfacing = DoRotation(facing, 0, Person::players[i]->targetyaw + 180, 0);

                                if (movekey) {
                                    Person::players[i]->velocity = flatfacing * Person::players[i]->speed * 45 * Person::players[i]->scale;
                                }
                                if (!movekey) {
                                    Person::players[i]->velocity = 0;
                                }

                                //Dodge sweep?
                                target = -2;
                                if (Person::players.size() > 1) {
                                    for (unsigned j = 0; j < Person::players.size(); j++) {
                                        if (j != i && !Person::players[j]->skeleton.free && Person::players[j]->victim) {
                                            if (distsq(&Person::players[j]->coords, &Person::players[j]->victim->coords) < 3 &&
                                                (Person::players[j]->victim == Person::players[i]) &&
                                                (Person::players[j]->animTarget == sweepanim)) {
                                                if (target >= 0) {
                                                    target = -1;
                                                } else {
                                                    target = j;
                                                }
                                            }
                                        }
                                    }
                                }
                                if (target >= 0) {
                                    Person::players[i]->velocity.y = 1;
                                } else if (Person::players[i]->crouchkeydown || !Person::players[i]->isPlayerControlled()) {
                                    Person::players[i]->velocity.y = 7;
                                    Person::players[i]->crouchtogglekeydown = 1;
                                } else {
                                    Person::players[i]->velocity.y = 5;
                                }

                                if (mousejump && i == 0 && devtools) {
                                    if (!Person::players[i]->isLanding()) {
                                        Person::players[i]->tempdeltav = deltav;
                                    }
                                    if (Person::players[i]->tempdeltav < 0) {
                                        Person::players[i]->velocity.y -= (float)(Person::players[i]->tempdeltav) / multiplier / 1000;
                                    }
                                }

                                Person::players[i]->coords.y += .2;
                                Person::players[i]->jumppower -= 1;

                                if (!i) {
                                    emit_sound_at(whooshsound, Person::players[i]->coords, 128.);
                                }

                                emit_sound_at(jumpsound, Person::players[i]->coords, 128.);
                            }
                            if ((Person::players[i]->isIdle()) && Person::players[i]->jumppower > 1) {
                                Person::players[i]->setTargetAnimation(Person::players[i]->getLanding());
                                Person::players[i]->frameTarget = 2;
                                Person::players[i]->landhard = 0;
                                Person::players[i]->jumpstart = 1;
                                Person::players[i]->tempdeltav = deltav;
                            }
                            if (Person::players[i]->animTarget == jumpupanim &&
                                (((!floatjump &&
                                   !editorenabled) ||
                                  !devtools) ||
                                 !Person::players[i]->isPlayerControlled())) {
                                if (Person::players[i]->jumppower > multiplier * 6) {
                                    Person::players[i]->velocity.y += multiplier * 6;
                                    Person::players[i]->jumppower -= multiplier * 6;
                                }
                                if (Person::players[i]->jumppower <= multiplier * 6) {
                                    Person::players[i]->velocity.y += Person::players[i]->jumppower;
                                    Person::players[i]->jumppower = 0;
                                }
                            }
                            if (((floatjump || editorenabled) && devtools) && i == 0) {
                                Person::players[i]->velocity.y += multiplier * 30;
                            }
                        }

                        if (!movekey) {
                            if (Person::players[i]->isRun() || Person::players[i]->animTarget == walkanim) {
                                Person::players[i]->setTargetAnimation(Person::players[i]->getStop());
                            }
                            if (Person::players[i]->animTarget == sneakanim) {
                                Person::players[i]->animTarget = Person::players[i]->getCrouch();
                                if (Person::players[i]->animCurrent == sneakanim) {
                                    Person::players[i]->target = 0;
                                }
                                Person::players[i]->frameTarget = 0;
                            }
                        }
                        if (Person::players[i]->animTarget == walkanim &&
                            (Person::players[i]->aitype == attacktypecutoff ||
                             Person::players[i]->aitype == searchtype ||
                             (Person::players[i]->aitype == passivetype &&
                              Person::players[i]->numwaypoints <= 1))) {
                            Person::players[i]->setTargetAnimation(Person::players[i]->getStop());
                        }
                        if (Person::players[i]->isRun() && (Person::players[i]->aitype == passivetype)) {
                            Person::players[i]->setTargetAnimation(Person::players[i]->getStop());
                        }
                    }
                }
                if (Person::players[i]->animTarget == rollanim) {
                    Person::players[i]->targetyaw = oldtargetyaw;
                }
            }

            //Rotation
            for (unsigned k = 0; k < Person::players.size(); k++) {
                if (fabs(Person::players[k]->yaw - Person::players[k]->targetyaw) > 180) {
                    if (Person::players[k]->yaw > Person::players[k]->targetyaw) {
                        Person::players[k]->yaw -= 360;
                    } else {
                        Person::players[k]->yaw += 360;
                    }
                }

                //stop to turn in right direction
                if (fabs(Person::players[k]->yaw - Person::players[k]->targetyaw) > 90 && (Person::players[k]->isRun() || Person::players[k]->animTarget == walkanim)) {
                    Person::players[k]->setTargetAnimation(Person::players[k]->getStop());
                }

                if (Person::players[k]->animTarget == backhandspringanim || Person::players[k]->animTarget == dodgebackanim) {
                    Person::players[k]->targettilt = 0;
                }

                if (Person::players[k]->animTarget != jumpupanim &&
                    Person::players[k]->animTarget != backhandspringanim &&
                    Person::players[k]->animTarget != jumpdownanim &&
                    !Person::players[k]->isFlip()) {
                    Person::players[k]->targettilt = 0;
                    if (Person::players[k]->jumppower < 0 && !Person::players[k]->jumpkeydown) {
                        Person::players[k]->jumppower = 0;
                    }
                    Person::players[k]->jumppower += multiplier * 7;
                    if (Person::players[k]->isCrouch()) {
                        Person::players[k]->jumppower += multiplier * 7;
                    }
                    if (Person::players[k]->jumppower > 5) {
                        Person::players[k]->jumppower = 5;
                    }
                }

                if (Person::players[k]->isRun()) {
                    Person::players[k]->targettilt = (Person::players[k]->yaw - Person::players[k]->targetyaw) / 4;
                }

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
            Object::DoStuff();

            for (int j = numenvsounds - 1; j >= 0; j--) {
                envsoundlife[j] -= multiplier;
                if (envsoundlife[j] < 0) {
                    numenvsounds--;
                    envsoundlife[j] = envsoundlife[numenvsounds];
                    envsound[j] = envsound[numenvsounds];
                }
            }
            OPENAL_SetFrequency(OPENAL_ALL, slomo);

            if (Tutorial::active) {
                Tutorial::DoStuff(multiplier);
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
}

void Game::TickOnce()
{
    if (mainmenu) {
        yaw += multiplier * 5;
    } else if (Dialog::directing || !Dialog::inDialog()) {
        yaw += deltah * .7;
        if (invertmouse) {
            pitch -= deltav * .7;
        } else {
            pitch += deltav * .7;
        }
        if (pitch > 90) {
            pitch = 90;
        }
        if (pitch < -70) {
            pitch = -70;
        }
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

        if (environment == snowyenvironment) {
            leveltheme = stream_snowtheme;
        }
        if (environment == grassyenvironment) {
            leveltheme = stream_grasstheme;
        }
        if (environment == desertenvironment) {
            leveltheme = stream_deserttheme;
        }

        realthreat = 0;

        musictype = leveltheme;
        for (unsigned i = 0; i < Person::players.size(); i++) {
            if ((Person::players[i]->aitype == attacktypecutoff ||
                 Person::players[i]->aitype == getweapontype ||
                 Person::players[i]->aitype == gethelptype ||
                 Person::players[i]->aitype == searchtype) &&
                !Person::players[i]->dead &&
                (Person::players[i]->animTarget != sneakattackedanim &&
                 Person::players[i]->animTarget != knifesneakattackedanim &&
                 Person::players[i]->animTarget != swordsneakattackedanim)) {
                musictype = stream_fighttheme;
                realthreat = 1;
            }
        }
        if (Person::players[0]->dead) {
            musictype = stream_menutheme;
        }

        if (musictype == stream_fighttheme) {
            unseendelay = 1;
        }

        if (oldmusictype == stream_fighttheme && musictype != stream_fighttheme) {
            unseendelay -= multiplier;
            if (unseendelay > 0) {
                musictype = stream_fighttheme;
            }
        }

        if (loading == 2) {
            musictype = stream_menutheme;
            musicvolume[2] = 512;
            musicvolume[0] = 0;
            musicvolume[1] = 0;
            musicvolume[3] = 0;
        }

        if (musictoggle) {
            if (musictype != oldmusictype && musictype == stream_fighttheme) {
                emit_sound_np(alarmsound);
            }
        }
        musicselected = musictype;

        if (musicselected == leveltheme) {
            musicvolume[0] += multiplier * 450;
        } else {
            musicvolume[0] -= multiplier * 450;
        }
        if (musicselected == stream_fighttheme) {
            musicvolume[1] += multiplier * 450;
        } else {
            musicvolume[1] -= multiplier * 450;
        }
        if (musicselected == stream_menutheme) {
            musicvolume[2] += multiplier * 450;
        } else {
            musicvolume[2] -= multiplier * 450;
        }

        for (int i = 0; i < 3; i++) {
            if (musicvolume[i] < 0) {
                musicvolume[i] = 0;
            }
            if (musicvolume[i] > 512) {
                musicvolume[i] = 512;
            }
        }

        if (musicvolume[2] > 128 && !loading && !mainmenu) {
            musicvolume[2] = 128;
        }

        if (musictoggle) {
            if (musicvolume[0] > 0 && oldmusicvolume[0] <= 0) {
                emit_stream_np(leveltheme, musicvolume[0]);
            }
            if (musicvolume[1] > 0 && oldmusicvolume[1] <= 0) {
                emit_stream_np(stream_fighttheme, musicvolume[1]);
            }
            if (musicvolume[2] > 0 && oldmusicvolume[2] <= 0) {
                emit_stream_np(stream_menutheme, musicvolume[2]);
            }
            if (musicvolume[0] <= 0 && oldmusicvolume[0] > 0) {
                pause_sound(leveltheme);
            }
            if (musicvolume[1] <= 0 && oldmusicvolume[1] > 0) {
                pause_sound(stream_fighttheme);
            }
            if (musicvolume[2] <= 0 && oldmusicvolume[2] > 0) {
                pause_sound(stream_menutheme);
            }

            if (musicvolume[0] != oldmusicvolume[0]) {
                OPENAL_SetVolume(channels[leveltheme], musicvolume[0]);
            }
            if (musicvolume[1] != oldmusicvolume[1]) {
                OPENAL_SetVolume(channels[stream_fighttheme], musicvolume[1]);
            }
            if (musicvolume[2] != oldmusicvolume[2]) {
                OPENAL_SetVolume(channels[stream_menutheme], musicvolume[2]);
            }

            for (int i = 0; i < 3; i++) {
                oldmusicvolume[i] = musicvolume[i];
            }
        } else {
            pause_sound(leveltheme);
            pause_sound(stream_fighttheme);
            pause_sound(stream_menutheme);

            for (int i = 0; i < 4; i++) {
                oldmusicvolume[i] = 0;
                musicvolume[i] = 0;
            }
        }

        Hotspot::killhotspot = 2;
        for (unsigned i = 0; i < Hotspot::hotspots.size(); i++) {
            if (Hotspot::hotspots[i].type > 10 && Hotspot::hotspots[i].type < 20) {
                if (Person::players[Hotspot::hotspots[i].type - 10]->dead == 0) {
                    Hotspot::killhotspot = 0;
                } else if (Hotspot::killhotspot == 2) {
                    Hotspot::killhotspot = 1;
                }
            }
        }
        if (Hotspot::killhotspot == 2) {
            Hotspot::killhotspot = 0;
        }

        winhotspot = false;
        for (unsigned i = 0; i < Hotspot::hotspots.size(); i++) {
            if (Hotspot::hotspots[i].type == -1) {
                if (distsq(&Person::players[0]->coords, &Hotspot::hotspots[i].position) < Hotspot::hotspots[i].size) {
                    winhotspot = true;
                }
            }
        }

        int numalarmed = 0;
        for (unsigned i = 1; i < Person::players.size(); i++) {
            if (!Person::players[i]->dead && Person::players[i]->aitype == attacktypecutoff && Person::players[i]->surprised <= 0) {
                numalarmed++;
            }
        }
        if (numalarmed > maxalarmed) {
            maxalarmed = numalarmed;
        }

        if (changedelay <= 0 && !loading && !editorenabled && gameon && !Tutorial::active && changedelay != -999 && !won) {
            if (Person::players[0]->dead) {
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
                if (targetlevel > numchallengelevels - 1) {
                    targetlevel = 0;
                }
            }
            if (winhotspot || windialogue) {
                changedelay = 0.1;
                targetlevel = whichlevel + 1;
                if (targetlevel > numchallengelevels - 1) {
                    targetlevel = 0;
                }
            }

            if (Hotspot::killhotspot) {
                changedelay = 1;
                targetlevel = whichlevel + 1;
                if (targetlevel > numchallengelevels - 1) {
                    targetlevel = 0;
                }
            }

            if (changedelay > 0 && !Person::players[0]->dead && !won) {
                //high scores, awards, win
                if (campaign) {
                    Account::active().winCampaignLevel(whichchoice, bonustotal, leveltime);
                    scoreadded = 1;
                } else {
                    wonleveltime = leveltime;
                    Account::active().winLevel(whichlevel, bonustotal - startbonustotal, leveltime);
                }
                won = 1;
                Account::saveFile(Folders::getUserSavePath());
            }
        }

        if (!winfreeze) {

            if (leveltime < 1) {
                loading = 0;
                changedelay = .1;
                alldead = false;
                winhotspot = false;
                Hotspot::killhotspot = 0;
            }

            if (!editorenabled && gameon && !mainmenu) {
                if (changedelay != -999) {
                    changedelay -= multiplier / 7;
                }
                if (Person::players[0]->dead) {
                    targetlevel = whichlevel;
                }
                if (loading == 2 && !campaign) {
                    flash();

                    fireSound(firestartsound);

                    if (!Person::players[0]->dead && targetlevel != whichlevel) {
                        startbonustotal = bonustotal;
                    }

                    LoadLevel(targetlevel);
                    fireSound();

                    loading = 3;
                }
                if (loading == 2 && targetlevel == whichlevel) {
                    flash();
                    loadtime = 0;

                    fireSound(firestartsound);

                    LoadLevel(campaignlevels[Account::active().getCampaignChoicesMade()].mapname.c_str());

                    fireSound();

                    loading = 3;
                }
                if (changedelay <= -999 &&
                    whichlevel != -2 &&
                    !loading &&
                    (Person::players[0]->dead ||
                     (alldead && maptype == mapkilleveryone) ||
                     (winhotspot) ||
                     (Hotspot::killhotspot))) {
                    loading = 1;
                }
                if ((Person::players[0]->dead ||
                     (alldead && maptype == mapkilleveryone) ||
                     (winhotspot) ||
                     (windialogue) ||
                     (Hotspot::killhotspot)) &&
                    changedelay <= 0) {
                    if (whichlevel != -2 && !loading && !Person::players[0]->dead) {
                        winfreeze = true;
                        changedelay = -999;
                    }
                    if (Person::players[0]->dead) {
                        loading = 1;
                    }
                }
            }

            if (campaign) {
                // campaignchoosenext determines what to do when the level is complete:
                // 0 = load next level
                // 1 = go back to level select screen
                // 2 = stealthload next level
                if (mainmenu == 0 && winfreeze && (campaignlevels[actuallevel].choosenext) == 1) {
                    if (campaignlevels[actuallevel].nextlevel.empty()) {
                        endgame = 1;
                    }
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
                    if (!firstLoadDone) {
                        LoadStuff();
                    }
                    whichchoice = 0;
                    actuallevel = campaignlevels[actuallevel].nextlevel.front();
                    visibleloading = true;
                    stillloading = 1;
                    LoadLevel(campaignlevels[actuallevel].mapname.c_str());
                    campaign = 1;
                    mainmenu = 0;
                    gameon = 1;
                    pause_sound(stream_menutheme);

                    stealthloading = 0;
                }
            }

            if (loading == 3) {
                loading = 0;
            }
        }

        oldmusictype = musictype;
    }

    facing = 0;
    facing.z = -1;

    facing = DoRotation(facing, -pitch, 0, 0);
    facing = DoRotation(facing, 0, 0 - yaw, 0);
    viewerfacing = facing;

    if (!cameramode) {
        if ((Animation::animations[Person::players[0]->animTarget].attack != 3 && Animation::animations[Person::players[0]->animCurrent].attack != 3) || Person::players[0]->skeleton.free) {
            target = Person::players[0]->coords + Person::players[0]->currentoffset * (1 - Person::players[0]->target) * Person::players[0]->scale + Person::players[0]->targetoffset * Person::players[0]->target * Person::players[0]->scale - Person::players[0]->facing * .05;
        } else {
            target = Person::players[0]->oldcoords + Person::players[0]->currentoffset * (1 - Person::players[0]->target) * Person::players[0]->scale + Person::players[0]->targetoffset * Person::players[0]->target * Person::players[0]->scale - Person::players[0]->facing * .05;
        }
        target.y += .1;
        if (Person::players[0]->skeleton.free) {
            for (unsigned i = 0; i < Person::players[0]->skeleton.joints.size(); i++) {
                if (Person::players[0]->skeleton.joints[i].position.y * Person::players[0]->scale + Person::players[0]->coords.y > target.y) {
                    target.y = Person::players[0]->skeleton.joints[i].position.y * Person::players[0]->scale + Person::players[0]->coords.y;
                }
            }
            target.y += .1;
        }
        if (Person::players[0]->skeleton.free != 2) {
            cameraspeed = 20;
            if (findLengthfast(&Person::players[0]->velocity) > 400) {
                cameraspeed = 20 + (findLength(&Person::players[0]->velocity) - 20) * .96;
            }
            if (Person::players[0]->skeleton.free == 0 && Person::players[0]->animTarget != hanganim && Person::players[0]->animTarget != climbanim) {
                target.y += 1.4;
            }
            coltarget = target - cameraloc;
            if (findLengthfast(&coltarget) < multiplier * multiplier * 400) {
                cameraloc = target;
            } else {
                Normalise(&coltarget);
                if (Person::players[0]->animTarget != hanganim && Person::players[0]->animTarget != climbanim && Person::players[0]->animCurrent != climbanim && Person::players[0]->currentoffset.x == 0) {
                    cameraloc = cameraloc + coltarget * multiplier * cameraspeed;
                } else {
                    cameraloc = cameraloc + coltarget * multiplier * 8;
                }
            }
            if (editorenabled) {
                cameraloc = target;
            }
            cameradist += multiplier * 5;
            if (cameradist > 2.3) {
                cameradist = 2.3;
            }
            viewer = cameraloc - facing * cameradist;
            colviewer = viewer;
            coltarget = cameraloc;
            Object::SphereCheckPossible(&colviewer, findDistance(&colviewer, &coltarget));
            for (unsigned int j = 0; j < terrain.patchobjects[Person::players[0]->whichpatchx][Person::players[0]->whichpatchz].size(); j++) {
                unsigned int i = terrain.patchobjects[Person::players[0]->whichpatchx][Person::players[0]->whichpatchz][j];
                colviewer = viewer;
                coltarget = cameraloc;
                if (Object::objects[i]->model.LineCheckPossible(&colviewer, &coltarget, &col, &Object::objects[i]->position, &Object::objects[i]->yaw) != -1) {
                    viewer = col;
                }
            }
            for (unsigned int j = 0; j < terrain.patchobjects[Person::players[0]->whichpatchx][Person::players[0]->whichpatchz].size(); j++) {
                unsigned int i = terrain.patchobjects[Person::players[0]->whichpatchx][Person::players[0]->whichpatchz][j];
                colviewer = viewer;
                if (Object::objects[i]->model.SphereCheck(&colviewer, .15, &col, &Object::objects[i]->position, &Object::objects[i]->yaw) != -1) {
                    viewer = colviewer;
                }
            }
            cameradist = findDistance(&viewer, &target);
            viewer.y = max((double)viewer.y, terrain.getHeight(viewer.x, viewer.z) + .6);
            if (cameraloc.y < terrain.getHeight(cameraloc.x, cameraloc.z)) {
                cameraloc.y = terrain.getHeight(cameraloc.x, cameraloc.z);
            }
        }
        if (camerashake > .8) {
            camerashake = .8;
        }
        woozy += multiplier;
        if (Person::players[0]->dead) {
            camerashake = 0;
        }
        if (Person::players[0]->dead) {
            woozy = 0;
        }
        camerashake -= multiplier * 2;
        blackout -= multiplier * 2;
        if (camerashake < 0) {
            camerashake = 0;
        }
        if (blackout < 0) {
            blackout = 0;
        }
        if (camerashake) {
            viewer.x += (float)(Random() % 100) * .0005 * camerashake;
            viewer.y += (float)(Random() % 100) * .0005 * camerashake;
            viewer.z += (float)(Random() % 100) * .0005 * camerashake;
        }
    }
}
