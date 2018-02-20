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

#ifndef _DIALOG_HPP_
#define _DIALOG_HPP_

#include "Math/XYZ.hpp"

#include <stdio.h>
#include <vector>
#include <json/value.h>

class DialogScene
{
public:
    DialogScene(FILE* tfile);
    DialogScene(Json::Value);
    DialogScene(ifstream& ipstream);
    void save(FILE* tfile);
    Json::Value save();

    int location;
    float color[3];
    int sound;
    std::string text;
    std::string name;
    XYZ camera;
    float camerayaw;
    float camerapitch;
    int participantfocus;
    int participantaction;
    XYZ participantfacing[10];
    operator Json::Value() {return save();}
};

class Dialog
{
public:
    Dialog(FILE* tfile);
    Dialog(Json::Value);
    Dialog(int type, std::string filename);
    void tick(int id);
    void play();
    void save(FILE* tfile);
    Json::Value save();

    int type;
    int gonethrough;
    std::vector<DialogScene> scenes;
    XYZ participantlocation[10];
    float participantyaw[10];

    static void loadDialogs(FILE*);
    static void loadDialogs(Json::Value);
    static void saveDialogs(FILE*);
    static Json::Value saveDialogs();

    static bool inDialog() { return (indialogue != -1); }
    static Dialog& currentDialog() { return dialogs[whichdialogue]; }
    static DialogScene& currentScene() { return currentDialog().scenes[indialogue]; }

    static int indialogue;
    static int whichdialogue;
    static bool directing;
    static float dialoguetime;
    static std::vector<Dialog> dialogs;
};

#endif /*_DIALOG_H_*/
