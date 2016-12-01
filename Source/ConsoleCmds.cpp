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

#include "ConsoleCmds.h"
#include "Game.h"
#include "Dialog.h"
#include "Utils/Folders.h"

const char *cmd_names[cmd_count] = {
#define DECLARE_COMMAND(cmd) #cmd,
#include "ConsoleCmds.def"
#undef  DECLARE_COMMAND
};

console_handler cmd_handlers[cmd_count] = {
#define DECLARE_COMMAND(cmd) ch_##cmd,
#include "ConsoleCmds.def"
#undef  DECLARE_COMMAND
};

using namespace Game;

/* globals */

extern bool campaign;
extern bool cellophane;
extern int editoractive;
extern int editorpathtype;
extern int environment;
extern float fadestart;
extern float slomospeed;
extern float slomofreq;
extern int tutoriallevel;
extern int hostile;
extern XYZ hotspot[40];
extern int hotspottype[40];
extern float hotspotsize[40];
extern char hotspottext[40][256];
extern int maptype;
extern int numhotspots;
extern Objects objects;
extern int slomo;
extern float slomodelay;
extern bool skyboxtexture;
extern float skyboxr;
extern float skyboxg;
extern float skyboxb;
extern float skyboxlightr;
extern float skyboxlightg;
extern float skyboxlightb;
extern Terrain terrain;
extern float viewdistance;

/* defined in GameTick.cpp */

extern int whichlevel;

float tintr = 1, tintg = 1, tintb = 1;

/* Helpers used in console commands */

/* Return true if PFX is a prefix of STR (case-insensitive).  */
static bool stripfx(const char *str, const char *pfx)
{
    return !strncasecmp(str, pfx, strlen(pfx));
}

static void set_proportion(int pnum, const char *args)
{
    float headprop, bodyprop, armprop, legprop;

    sscanf(args, "%f%f%f%f", &headprop, &bodyprop, &armprop, &legprop);

    if (Person::players[pnum]->creature == wolftype) {
        Person::players[pnum]->proportionhead = 1.1 * headprop;
        Person::players[pnum]->proportionbody = 1.1 * bodyprop;
        Person::players[pnum]->proportionarms = 1.1 * armprop;
        Person::players[pnum]->proportionlegs = 1.1 * legprop;
    } else if (Person::players[pnum]->creature == rabbittype) {
        Person::players[pnum]->proportionhead = 1.2 * headprop;
        Person::players[pnum]->proportionbody = 1.05 * bodyprop;
        Person::players[pnum]->proportionarms = 1.00 * armprop;
        Person::players[pnum]->proportionlegs = 1.1 * legprop;
        Person::players[pnum]->proportionlegs.y = 1.05 * legprop;
    }
}

static void set_protection(int pnum, const char *args)
{
    float head, high, low;
    sscanf(args, "%f%f%f", &head, &high, &low);

    Person::players[pnum]->protectionhead = head;
    Person::players[pnum]->protectionhigh = high;
    Person::players[pnum]->protectionlow  = low;
}

static void set_armor(int pnum, const char *args)
{
    float head, high, low;
    sscanf(args, "%f%f%f", &head, &high, &low);

    Person::players[pnum]->armorhead = head;
    Person::players[pnum]->armorhigh = high;
    Person::players[pnum]->armorlow  = low;
}

static void set_metal(int pnum, const char *args)
{
    float head, high, low;
    sscanf(args, "%f%f%f", &head, &high, &low);

    Person::players[pnum]->metalhead = head;
    Person::players[pnum]->metalhigh = high;
    Person::players[pnum]->metallow  = low;
}

static void set_noclothes(int pnum, const char *args)
{
    Person::players[pnum]->numclothes = 0;
    Person::players[pnum]->skeleton.drawmodel.textureptr.load(
        creatureskin[Person::players[pnum]->creature][Person::players[pnum]->whichskin], 1,
        &Person::players[pnum]->skeleton.skinText[0], &Person::players[pnum]->skeleton.skinsize);
}

static void set_clothes(int pnum, const char *args)
{
    char buf[64];
    snprintf(buf, 63, "Textures/%s.png", args);

    int id = Person::players[pnum]->numclothes;
    strcpy(Person::players[pnum]->clothes[id], buf);
    Person::players[pnum]->clothestintr[id] = tintr;
    Person::players[pnum]->clothestintg[id] = tintg;
    Person::players[pnum]->clothestintb[id] = tintb;
    Person::players[pnum]->numclothes++;

    if (!Person::players[pnum]->addClothes(id))
        return;

    Person::players[pnum]->DoMipmaps();
}

/* Console commands themselves */

void ch_quit(const char *args)
{
    tryquit = 1;
}

void ch_map(const char *args)
{
    Loadlevel(args);
    whichlevel = -2;
    campaign = 0;
}

void ch_save(const char *args)
{
    std::string map_path = Folders::getUserDataPath() + "/Maps/" + args;

    int mapvers = 12;

    FILE *tfile;
    tfile = fopen( map_path.c_str(), "wb" );
    fpackf(tfile, "Bi", mapvers);
    fpackf(tfile, "Bi", maptype);
    fpackf(tfile, "Bi", hostile);
    fpackf(tfile, "Bf Bf", viewdistance, fadestart);
    fpackf(tfile, "Bb Bf Bf Bf", skyboxtexture, skyboxr, skyboxg, skyboxb);
    fpackf(tfile, "Bf Bf Bf", skyboxlightr, skyboxlightg, skyboxlightb);
    fpackf(tfile, "Bf Bf Bf Bf Bf Bi", Person::players[0]->coords.x, Person::players[0]->coords.y, Person::players[0]->coords.z,
           Person::players[0]->yaw, Person::players[0]->targetyaw, Person::players[0]->num_weapons);
    if (Person::players[0]->num_weapons > 0 && Person::players[0]->num_weapons < 5)
        for (int j = 0; j < Person::players[0]->num_weapons; j++)
            fpackf(tfile, "Bi", weapons[Person::players[0]->weaponids[j]].getType());

    fpackf(tfile, "Bf Bf Bf", Person::players[0]->armorhead, Person::players[0]->armorhigh, Person::players[0]->armorlow);
    fpackf(tfile, "Bf Bf Bf", Person::players[0]->protectionhead, Person::players[0]->protectionhigh, Person::players[0]->protectionlow);
    fpackf(tfile, "Bf Bf Bf", Person::players[0]->metalhead, Person::players[0]->metalhigh, Person::players[0]->metallow);
    fpackf(tfile, "Bf Bf", Person::players[0]->power, Person::players[0]->speedmult);

    fpackf(tfile, "Bi", Person::players[0]->numclothes);

    fpackf(tfile, "Bi Bi", Person::players[0]->whichskin, Person::players[0]->creature);

    Dialog::saveDialogs(tfile);

    for (int k = 0; k < Person::players[0]->numclothes; k++) {
        int templength = strlen(Person::players[0]->clothes[k]);
        fpackf(tfile, "Bi", templength);
        for (int l = 0; l < templength; l++)
            fpackf(tfile, "Bb", Person::players[0]->clothes[k][l]);
        fpackf(tfile, "Bf Bf Bf", Person::players[0]->clothestintr[k], Person::players[0]->clothestintg[k], Person::players[0]->clothestintb[k]);
    }

    fpackf(tfile, "Bi", environment);

    fpackf(tfile, "Bi", objects.numobjects);

    for (int k = 0; k < objects.numobjects; k++)
        fpackf(tfile, "Bi Bf Bf Bf Bf Bf Bf", objects.type[k], objects.yaw[k], objects.pitch[k],
               objects.position[k].x, objects.position[k].y, objects.position[k].z, objects.scale[k]);

    fpackf(tfile, "Bi", numhotspots);
    for (int i = 0; i < numhotspots; i++) {
        fpackf(tfile, "Bi Bf Bf Bf Bf", hotspottype[i], hotspotsize[i], hotspot[i].x, hotspot[i].y, hotspot[i].z);
        int templength = strlen(hotspottext[i]);
        fpackf(tfile, "Bi", templength);
        for (int l = 0; l < templength; l++)
            fpackf(tfile, "Bb", hotspottext[i][l]);
    }

    fpackf(tfile, "Bi", Person::players.size());
    if (Person::players.size() > maxplayers) {
        cout << "Warning: this level contains more players than allowed" << endl;
    }
    for (unsigned j = 1; j < Person::players.size(); j++) {
        fpackf(tfile, "Bi Bi Bf Bf Bf Bi Bi Bf Bb Bf", Person::players[j]->whichskin, Person::players[j]->creature,
               Person::players[j]->coords.x, Person::players[j]->coords.y, Person::players[j]->coords.z,
               Person::players[j]->num_weapons, Person::players[j]->howactive, Person::players[j]->scale, Person::players[j]->immobile, Person::players[j]->yaw);
        if (Person::players[j]->num_weapons < 5)
            for (int k = 0; k < Person::players[j]->num_weapons; k++)
                fpackf(tfile, "Bi", weapons[Person::players[j]->weaponids[k]].getType());
        if (Person::players[j]->numwaypoints < 30) {
            fpackf(tfile, "Bi", Person::players[j]->numwaypoints);
            for (int k = 0; k < Person::players[j]->numwaypoints; k++) {
                fpackf(tfile, "Bf", Person::players[j]->waypoints[k].x);
                fpackf(tfile, "Bf", Person::players[j]->waypoints[k].y);
                fpackf(tfile, "Bf", Person::players[j]->waypoints[k].z);
                fpackf(tfile, "Bi", Person::players[j]->waypointtype[k]);
            }
            fpackf(tfile, "Bi", Person::players[j]->waypoint);
        } else {
            Person::players[j]->numwaypoints = 0;
            Person::players[j]->waypoint = 0;
            fpackf(tfile, "Bi Bi Bi", Person::players[j]->numwaypoints, Person::players[j]->waypoint, Person::players[j]->waypoint);
        }

        fpackf(tfile, "Bf Bf Bf", Person::players[j]->armorhead, Person::players[j]->armorhigh, Person::players[j]->armorlow);
        fpackf(tfile, "Bf Bf Bf", Person::players[j]->protectionhead, Person::players[j]->protectionhigh, Person::players[j]->protectionlow);
        fpackf(tfile, "Bf Bf Bf", Person::players[j]->metalhead, Person::players[j]->metalhigh, Person::players[j]->metallow);
        fpackf(tfile, "Bf Bf", Person::players[j]->power, Person::players[j]->speedmult);

        float headprop, bodyprop, armprop, legprop;
        if (Person::players[j]->creature == wolftype) {
            headprop = Person::players[j]->proportionhead.x / 1.1;
            bodyprop = Person::players[j]->proportionbody.x / 1.1;
            armprop = Person::players[j]->proportionarms.x / 1.1;
            legprop = Person::players[j]->proportionlegs.x / 1.1;
        } else if (Person::players[j]->creature == rabbittype) {
            headprop = Person::players[j]->proportionhead.x / 1.2;
            bodyprop = Person::players[j]->proportionbody.x / 1.05;
            armprop = Person::players[j]->proportionarms.x / 1.00;
            legprop = Person::players[j]->proportionlegs.x / 1.1;
        }

        fpackf(tfile, "Bf Bf Bf Bf", headprop, bodyprop, armprop, legprop);

        fpackf(tfile, "Bi", Person::players[j]->numclothes);
        if (Person::players[j]->numclothes)
            for (int k = 0; k < Person::players[j]->numclothes; k++) {
                int templength;
                templength = strlen(Person::players[j]->clothes[k]);
                fpackf(tfile, "Bi", templength);
                for (int l = 0; l < templength; l++)
                    fpackf(tfile, "Bb", Person::players[j]->clothes[k][l]);
                fpackf(tfile, "Bf Bf Bf", Person::players[j]->clothestintr[k], Person::players[j]->clothestintg[k], Person::players[j]->clothestintb[k]);
            }
    }

    fpackf(tfile, "Bi", numpathpoints);
    for (int j = 0; j < numpathpoints; j++) {
        fpackf(tfile, "Bf Bf Bf Bi", pathpoint[j].x, pathpoint[j].y, pathpoint[j].z, numpathpointconnect[j]);
        for (int k = 0; k < numpathpointconnect[j]; k++)
            fpackf(tfile, "Bi", pathpointconnect[j][k]);
    }

    fpackf(tfile, "Bf Bf Bf Bf", mapcenter.x, mapcenter.y, mapcenter.z, mapradius);

    fclose(tfile);
}

void ch_cellar(const char *args)
{
    Person::players[0]->skeleton.drawmodel.textureptr.load("Textures/Furdarko.jpg", 1, &Person::players[0]->skeleton.skinText[0], &Person::players[0]->skeleton.skinsize);
}

void ch_tint(const char *args)
{
    sscanf(args, "%f%f%f", &tintr, &tintg, &tintb);
}

void ch_tintr(const char *args)
{
    tintr = atof(args);
}

void ch_tintg(const char *args)
{
    tintg = atof(args);
}

void ch_tintb(const char *args)
{
    tintb = atof(args);
}

void ch_speed(const char *args)
{
    Person::players[0]->speedmult = atof(args);
}

void ch_strength(const char *args)
{
    Person::players[0]->power = atof(args);
}

void ch_power(const char *args)
{
    Person::players[0]->power = atof(args);
}

void ch_size(const char *args)
{
    Person::players[0]->scale = atof(args) * .2;
}

void ch_sizenear(const char *args)
{
    int closest = findClosestPlayer();
    if (closest >= 0)
        Person::players[closest]->scale = atof(args) * .2;
}

void ch_proportion(const char *args)
{
    set_proportion(0, args);
}

void ch_proportionnear(const char *args)
{
    int closest = findClosestPlayer();
    if (closest >= 0)
        set_proportion(closest, args);
}

void ch_protection(const char *args)
{
    set_protection(0, args);
}

void ch_protectionnear(const char *args)
{
    int closest = findClosestPlayer();
    if (closest >= 0)
        set_protection(closest, args);
}

void ch_armor(const char *args)
{
    set_armor(0, args);
}

void ch_armornear(const char *args)
{
    int closest = findClosestPlayer();
    if (closest >= 0)
        set_armor(closest, args);
}

void ch_protectionreset(const char *args)
{
    set_protection(0, "1 1 1");
    set_armor(0, "1 1 1");
}

void ch_metal(const char *args)
{
    set_metal(0, args);
}

void ch_noclothes(const char *args)
{
    set_noclothes(0, args);
}

void ch_noclothesnear(const char *args)
{
    int closest = findClosestPlayer();
    if (closest >= 0)
        set_noclothes(closest, args);
}

void ch_clothes(const char *args)
{
    set_clothes(0, args);
}

void ch_clothesnear(const char *args)
{
    int closest = findClosestPlayer();
    if (closest >= 0)
        set_clothes(closest, args);
}

void ch_belt(const char *args)
{
    Person::players[0]->skeleton.clothes = !Person::players[0]->skeleton.clothes;
}


void ch_cellophane(const char *args)
{
    cellophane = !cellophane;
    float mul = (cellophane ? 0 : 1);

    for (auto player : Person::players) {
        player->proportionhead.z = player->proportionhead.x * mul;
        player->proportionbody.z = player->proportionbody.x * mul;
        player->proportionarms.z = player->proportionarms.x * mul;
        player->proportionlegs.z = player->proportionlegs.x * mul;
    }
}

void ch_funnybunny(const char *args)
{
    Person::players[0]->skeleton.id = 0;
    Person::players[0]->skeleton.Load("Skeleton/Basic Figure", "Skeleton/Basic Figurelow",
                            "Skeleton/Rabbitbelt", "Models/Body.solid",
                            "Models/Body2.solid", "Models/Body3.solid",
                            "Models/Body4.solid", "Models/Body5.solid",
                            "Models/Body6.solid", "Models/Body7.solid",
                            "Models/Bodylow.solid", "Models/Belt.solid", 1);
    Person::players[0]->skeleton.drawmodel.textureptr.load("Textures/fur3.jpg", 1, &Person::players[0]->skeleton.skinText[0], &Person::players[0]->skeleton.skinsize);
    Person::players[0]->creature = rabbittype;
    Person::players[0]->scale = .2;
    Person::players[0]->headless = 0;
    Person::players[0]->damagetolerance = 200;
    set_proportion(0, "1 1 1 1");
}

void ch_wolfie(const char *args)
{
    Person::players[0]->skeleton.id = 0;
    Person::players[0]->skeleton.Load("Skeleton/Basic Figure Wolf", "Skeleton/Basic Figure Wolf Low",
                            "Skeleton/Rabbitbelt", "Models/Wolf.solid",
                            "Models/Wolf2.solid", "Models/Wolf3.solid",
                            "Models/Wolf4.solid", "Models/Wolf5.solid",
                            "Models/Wolf6.solid", "Models/Wolf7.solid",
                            "Models/Wolflow.solid", "Models/Belt.solid", 0);
    Person::players[0]->skeleton.drawmodel.textureptr.load("Textures/Wolf.jpg", 1, &Person::players[0]->skeleton.skinText[0], &Person::players[0]->skeleton.skinsize);
    Person::players[0]->creature = wolftype;
    Person::players[0]->damagetolerance = 300;
    set_proportion(0, "1 1 1 1");
}

void ch_wolfieisgod(const char *args)
{
    ch_wolfie(args);
}

void ch_wolf(const char *args)
{
    Person::players[0]->skeleton.drawmodel.textureptr.load("Textures/Wolf.jpg", 1, &Person::players[0]->skeleton.skinText[0], &Person::players[0]->skeleton.skinsize);
}

void ch_snowwolf(const char *args)
{
    Person::players[0]->skeleton.drawmodel.textureptr.load("Textures/SnowWolf.jpg", 1, &Person::players[0]->skeleton.skinText[0], &Person::players[0]->skeleton.skinsize);
}

void ch_darkwolf(const char *args)
{
    Person::players[0]->skeleton.drawmodel.textureptr.load("Textures/DarkWolf.jpg", 1, &Person::players[0]->skeleton.skinText[0], &Person::players[0]->skeleton.skinsize);
}

void ch_lizardwolf(const char *args)
{
    Person::players[0]->skeleton.drawmodel.textureptr.load("Textures/Lizardwolf.jpg", 1, &Person::players[0]->skeleton.skinText[0], &Person::players[0]->skeleton.skinsize);
}

void ch_white(const char *args)
{
    Person::players[0]->skeleton.drawmodel.textureptr.load("Textures/fur.jpg", 1, &Person::players[0]->skeleton.skinText[0], &Person::players[0]->skeleton.skinsize);
}

void ch_brown(const char *args)
{
    Person::players[0]->skeleton.drawmodel.textureptr.load("Textures/fur3.jpg", 1, &Person::players[0]->skeleton.skinText[0], &Person::players[0]->skeleton.skinsize);
}

void ch_black(const char *args)
{
    Person::players[0]->skeleton.drawmodel.textureptr.load("Textures/fur2.jpg", 1, &Person::players[0]->skeleton.skinText[0], &Person::players[0]->skeleton.skinsize);
}

void ch_sizemin(const char *args)
{
    for (unsigned i = 1; i < Person::players.size(); i++)
        if (Person::players[i]->scale < 0.8 * 0.2)
            Person::players[i]->scale = 0.8 * 0.2;
}

void ch_tutorial(const char *args)
{
    tutoriallevel = atoi(args);
}

void ch_hostile(const char *args)
{
    hostile = atoi(args);
}

void ch_type(const char *args)
{
    int n = sizeof(editortypenames) / sizeof(editortypenames[0]);
    for (int i = 0; i < n; i++)
        if (stripfx(args, editortypenames[i])) {
            editoractive = i;
            break;
        }
}

void ch_path(const char *args)
{
    int n = sizeof(pathtypenames) / sizeof(pathtypenames[0]);
    for (int i = 0; i < n; i++)
        if (stripfx(args, pathtypenames[i])) {
            editorpathtype = i;
            break;
        }
}

void ch_hs(const char *args)
{
    hotspot[numhotspots] = Person::players[0]->coords;

    float size;
    int type, shift;
    sscanf(args, "%f%d %n", &size, &type, &shift);

    hotspotsize[numhotspots] = size;
    hotspottype[numhotspots] = type;

    strcpy(hotspottext[numhotspots], args + shift);
    strcat(hotspottext[numhotspots], "\n");

    numhotspots++;
}

void ch_dialogue(const char *args)
{
    int type;
    char buf1[32];

    sscanf(args, "%d %31s", &type, buf1);
    std::string filename = std::string("Dialogues/") + buf1 + ".txt";

    Dialog::dialogs.push_back(Dialog(type, filename));

    Dialog::directing = true;
    Dialog::indialogue = 0;
    Dialog::whichdialogue = Dialog::dialogs.size();
}

void ch_fixdialogue(const char *args)
{
    char buf1[32];
    int whichdi;

    sscanf(args, "%d %31s", &whichdi, buf1);
    std::string filename = std::string("Dialogues/") + buf1 + ".txt";

    Dialog::dialogs[whichdi] = Dialog(Dialog::dialogs[whichdi].type, filename);
}

void ch_fixtype(const char *args)
{
    int dlg;
    sscanf(args, "%d", &dlg);
    Dialog::dialogs[0].type = dlg;
}

void ch_fixrotation(const char *args)
{
    int playerId = Dialog::currentScene().participantfocus;
    Dialog::currentDialog().participantyaw[playerId] = Person::players[playerId]->yaw;
}

void ch_ddialogue(const char *args)
{
    Dialog::dialogs.pop_back();
}

void ch_dhs(const char *args)
{
    if (numhotspots)
        numhotspots--;
}

void ch_immobile(const char *args)
{
    Person::players[0]->immobile = 1;
}

void ch_allimmobile(const char *args)
{
    for (unsigned i = 1; i < Person::players.size(); i++)
        Person::players[i]->immobile = 1;
}

void ch_mobile(const char *args)
{
    Person::players[0]->immobile = 0;
}

void ch_default(const char *args)
{
    Person::players[0]->armorhead = 1;
    Person::players[0]->armorhigh = 1;
    Person::players[0]->armorlow = 1;
    Person::players[0]->protectionhead = 1;
    Person::players[0]->protectionhigh = 1;
    Person::players[0]->protectionlow = 1;
    Person::players[0]->metalhead = 1;
    Person::players[0]->metalhigh = 1;
    Person::players[0]->metallow = 1;
    Person::players[0]->power = 1;
    Person::players[0]->speedmult = 1;
    Person::players[0]->scale = 1;

    if (Person::players[0]->creature == wolftype) {
        Person::players[0]->proportionhead = 1.1;
        Person::players[0]->proportionbody = 1.1;
        Person::players[0]->proportionarms = 1.1;
        Person::players[0]->proportionlegs = 1.1;
    } else if (Person::players[0]->creature == rabbittype) {
        Person::players[0]->proportionhead = 1.2;
        Person::players[0]->proportionbody = 1.05;
        Person::players[0]->proportionarms = 1.00;
        Person::players[0]->proportionlegs = 1.1;
        Person::players[0]->proportionlegs.y = 1.05;
    }

    Person::players[0]->numclothes = 0;
    Person::players[0]->skeleton.drawmodel.textureptr.load(
        creatureskin[Person::players[0]->creature][Person::players[0]->whichskin], 1,
        &Person::players[0]->skeleton.skinText[0], &Person::players[0]->skeleton.skinsize);

    editoractive = typeactive;
    Person::players[0]->immobile = 0;
}

void ch_play(const char *args)
{
    int dlg;
    sscanf(args, "%d", &dlg);
    Dialog::whichdialogue = dlg;

    if (Dialog::whichdialogue >= Dialog::dialogs.size()) {
        return;
    }

    Dialog::currentDialog().play();
}

void ch_mapkilleveryone(const char *args)
{
    maptype = mapkilleveryone;
}

void ch_mapkillmost(const char *args)
{
    maptype = mapkillmost;
}

void ch_mapkillsomeone(const char *args)
{
    maptype = mapkillsomeone;
}

void ch_mapgosomewhere(const char *args)
{
    maptype = mapgosomewhere;
}

void ch_viewdistance(const char *args)
{
    viewdistance = atof(args) * 100;
}

void ch_fadestart(const char *args)
{
    fadestart = atof(args);
}

void ch_slomo(const char *args)
{
    slomospeed = atof(args);
    slomo = !slomo;
    slomodelay = 1000;
}

void ch_slofreq(const char *args)
{
    slomofreq = atof(args);
}

void ch_skytint(const char *args)
{
    sscanf(args, "%f%f%f", &skyboxr, &skyboxg, &skyboxb);

    skyboxlightr = skyboxr;
    skyboxlightg = skyboxg;
    skyboxlightb = skyboxb;

    SetUpLighting();

    terrain.DoShadows();
    objects.DoShadows();
}

void ch_skylight(const char *args)
{
    sscanf(args, "%f%f%f", &skyboxlightr, &skyboxlightg, &skyboxlightb);

    SetUpLighting();

    terrain.DoShadows();
    objects.DoShadows();
}

void ch_skybox(const char *args)
{
    skyboxtexture = !skyboxtexture;

    SetUpLighting();

    terrain.DoShadows();
    objects.DoShadows();
}
