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

#include "Level/Dialog.hpp"

#include "Game.hpp"
#include "Objects/Person.hpp"
#include "Utils/Folders.hpp"
#include "Utils/Input.hpp"
#include "Utils/binio.h"

extern int hostile;

int Dialog::indialogue;
int Dialog::whichdialogue;
bool Dialog::directing;
float Dialog::dialoguetime;
std::vector<Dialog> Dialog::dialogs;

void Dialog::loadDialogs(FILE* tfile)
{
    int numdialogues;
    funpackf(tfile, "Bi", &numdialogues);
    for (int k = 0; k < numdialogues; k++) {
        dialogs.push_back(Dialog(tfile));
    }
}

void Dialog::loadDialogs(Json::Value values)
{
    for (unsigned i = 0; i < values.size(); i++) {
        dialogs.push_back(Dialog(values[i]));
    }
}

Dialog::Dialog(Json::Value data)
    : gonethrough(0)
{
    type = data["type"].asInt();

    for (int l = 0; l < 10; l++) {
        participantlocation[l]  = data["participant"][l]["pos"];
        participantyaw[l]       = data["participant"][l]["yaw"].asFloat();
    }
    for (unsigned l = 0; l < data["scenes"].size(); l++) {
        scenes.push_back(DialogScene(data["scenes"][l]));
    }
}

Dialog::Dialog(FILE* tfile)
    : gonethrough(0)
{
    int numdialogscenes;
    funpackf(tfile, "Bi", &numdialogscenes);
    funpackf(tfile, "Bi", &type);
    for (int l = 0; l < 10; l++) {
        funpackf(tfile, "Bf Bf Bf", &participantlocation[l].x, &participantlocation[l].y, &participantlocation[l].z);
        funpackf(tfile, "Bf", &participantyaw[l]);
    }
    for (int l = 0; l < numdialogscenes; l++) {
        scenes.push_back(DialogScene(tfile));
    }
}

std::string funpackf_string(FILE* tfile, int maxlength)
{
    int templength;
    funpackf(tfile, "Bi", &templength);
    if ((templength > maxlength) || (templength <= 0)) {
        templength = maxlength;
    }
    int m;
    char* text = new char[maxlength];
    for (m = 0; m < templength; m++) {
        funpackf(tfile, "Bb", &text[m]);
        if (text[m] == '\0') {
            break;
        }
    }
    text[m] = 0;
    std::string result(text);
    delete[] text;
    return result;
}

void fpackf_string(FILE* tfile, std::string text)
{
    fpackf(tfile, "Bi", text.size());
    for (unsigned i = 0; i < text.size(); i++) {
        fpackf(tfile, "Bb", text[i]);
        if (text[i] == '\0') {
            break;
        }
    }
}

DialogScene::DialogScene(FILE* tfile)
{
    funpackf(tfile, "Bi", &location);
    funpackf(tfile, "Bf", &color[0]);
    funpackf(tfile, "Bf", &color[1]);
    funpackf(tfile, "Bf", &color[2]);
    funpackf(tfile, "Bi", &sound);

    text = funpackf_string(tfile, 128);
    name = funpackf_string(tfile, 64);

    funpackf(tfile, "Bf Bf Bf", &camera.x, &camera.y, &camera.z);
    funpackf(tfile, "Bi", &participantfocus);
    funpackf(tfile, "Bi", &participantaction);

    for (int m = 0; m < 10; m++) {
        funpackf(tfile, "Bf Bf Bf", &participantfacing[m].x, &participantfacing[m].y, &participantfacing[m].z);
    }

    funpackf(tfile, "Bf Bf", &camerayaw, &camerapitch);
}

DialogScene::DialogScene(Json::Value data)
{
    location            = data["location"].asInt();
    color[0]            = data["color"][0].asFloat();
    color[1]            = data["color"][1].asFloat();
    color[2]            = data["color"][2].asFloat();
    sound               = data["sound"].asInt();
    text                = data["text"].asString();
    name                = data["name"].asString();
    camera              = data["camera"]["pos"];
    camerayaw           = data["camera"]["yaw"].asFloat();
    camerapitch         = data["camera"]["pitch"].asFloat();
    participantfocus    = data["participantfocus"].asInt();
    participantaction   = data["participantaction"].asInt();

    for (int m = 0; m < 10; m++) {
        participantfacing[m] = data["participantfacing"][m];
    }
}

/* Load dialog from txt file, used by console */
Dialog::Dialog(int type, std::string filename)
    : type(type)
{
    ifstream ipstream(Folders::getResourcePath(filename));
    ipstream.ignore(256, ':');
    int numscenes;
    ipstream >> numscenes;
    for (int i = 0; i < numscenes; i++) {
        scenes.push_back(DialogScene(ipstream));
        for (unsigned j = 0; j < Person::players.size(); j++) {
            scenes.back().participantfacing[j] = Person::players[j]->facing;
        }
    }
    ipstream.close();
}

DialogScene::DialogScene(ifstream& ipstream)
{
    ipstream.ignore(256, ':');
    ipstream.ignore(256, ':');
    ipstream.ignore(256, ' ');
    ipstream >> location;
    ipstream.ignore(256, ':');
    ipstream >> color[0];
    ipstream >> color[1];
    ipstream >> color[2];
    ipstream.ignore(256, ':');
    getline(ipstream, name);
    ipstream.ignore(256, ':');
    ipstream.ignore(256, ' ');
    getline(ipstream, text);
    for (int j = 0; j < 128; j++) {
        if (text[j] == '\\') {
            text[j] = '\n';
        }
    }
    ipstream.ignore(256, ':');
    ipstream >> sound;
}

void Dialog::tick(int id)
{
    unsigned playerId = type % 10;
    bool special = (type > 9);

    if ((!hostile || (type > 40) && (type < 50)) &&
        (playerId < Person::players.size()) &&
        (playerId > 0) &&
        ((gonethrough == 0) || !special) &&
        (special || Input::isKeyPressed(Game::attackkey))) {
        if ((distsq(&Person::players[0]->coords, &Person::players[playerId]->coords) < 6) ||
            (Person::players[playerId]->howactive >= typedead1) ||
            (type > 40) && (type < 50)) {
            whichdialogue = id;
            play();
            dialoguetime = 0;
            gonethrough++;
        }
    }
}

void Dialog::play()
{
    for (unsigned i = 0; i < scenes.size(); i++) {
        int playerId = scenes[i].participantfocus;
        Person::players.at(playerId)->coords = participantlocation[playerId];
        Person::players[playerId]->yaw = participantyaw[playerId];
        Person::players[playerId]->targetyaw = participantyaw[playerId];
        Person::players[playerId]->velocity = 0;
        Person::players[playerId]->animTarget = Person::players[playerId]->getIdle();
        Person::players[playerId]->frameTarget = 0;
    }

    Dialog::directing = false;
    Dialog::indialogue = 0;

    if (scenes[indialogue].sound != 0) {
        Game::playdialoguescenesound();
    }
}

void Dialog::saveDialogs(FILE* tfile)
{
    fpackf(tfile, "Bi", dialogs.size());

    for (unsigned i = 0; i < dialogs.size(); i++) {
        dialogs[i].save(tfile);
    }
}

Json::Value Dialog::saveDialogs()
{
    Json::Value dialogsarray;

    for (unsigned i = 0; i < dialogs.size(); i++) {
        dialogsarray[i] = dialogs[i].save();
    }

    return dialogsarray;
}

void Dialog::save(FILE* tfile)
{
    fpackf(tfile, "Bi", scenes.size());
    fpackf(tfile, "Bi", type);
    for (int l = 0; l < 10; l++) {
        fpackf(tfile, "Bf Bf Bf", participantlocation[l].x, participantlocation[l].y, participantlocation[l].z);
        fpackf(tfile, "Bf", participantyaw[l]);
    }
    for (unsigned l = 0; l < scenes.size(); l++) {
        scenes[l].save(tfile);
    }
}

Json::Value Dialog::save()
{
    Json::Value dialog;

    dialog["type"] = type;

    for (int l = 0; l < 10; l++) {
        dialog["participant"][l]["pos"] = participantlocation[l];
        dialog["participant"][l]["yaw"] = participantyaw[l];
    }
    for (unsigned l = 0; l < scenes.size(); l++) {
        dialog["scenes"][l] = scenes[l];
    }

    return dialog;
}

void DialogScene::save(FILE* tfile)
{
    fpackf(tfile, "Bi", location);
    fpackf(tfile, "Bf", color[0]);
    fpackf(tfile, "Bf", color[1]);
    fpackf(tfile, "Bf", color[2]);
    fpackf(tfile, "Bi", sound);

    fpackf_string(tfile, text);
    fpackf_string(tfile, name);

    fpackf(tfile, "Bf Bf Bf", camera.x, camera.y, camera.z);
    fpackf(tfile, "Bi", participantfocus);
    fpackf(tfile, "Bi", participantaction);

    for (int m = 0; m < 10; m++) {
        fpackf(tfile, "Bf Bf Bf", participantfacing[m].x, participantfacing[m].y, participantfacing[m].z);
    }

    fpackf(tfile, "Bf Bf", camerayaw, camerapitch);
}

Json::Value DialogScene::save()
{
    Json::Value dialogscene;

    dialogscene["location"] = location;
    dialogscene["color"][0] = color[0];
    dialogscene["color"][1] = color[1];
    dialogscene["color"][2] = color[2];
    dialogscene["sound"]    = sound;
    dialogscene["text"]     = text;
    dialogscene["name"]     = name;
    dialogscene["camera"]["pos"]    = camera;
    dialogscene["camera"]["yaw"]    = camerayaw;
    dialogscene["camera"]["pitch"]  = camerapitch;
    dialogscene["participantfocus"]     = participantfocus;
    dialogscene["participantaction"]    = participantaction;

    for (int m = 0; m < 10; m++) {
        dialogscene["participantfacing"][m] = participantfacing[m];
    }

    return dialogscene;
}
