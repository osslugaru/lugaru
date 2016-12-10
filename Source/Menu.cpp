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

#include <vector>
#include <string>
#include <set>
#include "gamegl.h"

#include "Menu.h"
#include "Settings.h"
#include "Input.h"
#include "Campaign.h"

// Should not be needed, Menu should call methods from other classes to launch maps and challenges and so on
#include "Awards.h"
#include "openal_wrapper.h"

using namespace Game;

extern float multiplier;
extern std::set<std::pair<int,int>> resolutions;
extern int mainmenu;
extern std::vector<CampaignLevel> campaignlevels;
extern float musicvolume[4];
extern float oldmusicvolume[4];
extern bool stillloading;
extern bool visibleloading;
extern int whichchoice;
extern int leveltheme;

extern void toggleFullscreen();

int entername = 0;

std::vector<MenuItem> Menu::items;

MenuItem::MenuItem(MenuItemType _type, int _id, const string& _text, Texture _texture,
          int _x, int _y, int _w, int _h, float _r, float _g, float _b,
          float _linestartsize, float _lineendsize):
    type(_type),
    id(_id),
    text(_text),
    texture(_texture),
    x(_x),
    y(_y),
    w(_w),
    h(_h),
    r(_r),
    g(_g),
    b(_b),
    effectfade(0),
    linestartsize(_linestartsize),
    lineendsize(_lineendsize)
{
    if (type == MenuItem::BUTTON) {
        if (w == -1) {
            w = text.length() * 10;
        }
        if (h == -1) {
            h = 20;
        }
    }
}

void Menu::clearMenu()
{
    items.clear();
}

void Menu::addLabel(int id, const string& text, int x, int y, float r, float g, float b)
{
    items.emplace_back(MenuItem::LABEL, id, text, Texture(), x, y, -1, -1, r, g, b);
}
void Menu::addButton(int id, const string& text, int x, int y, float r, float g, float b)
{
    items.emplace_back(MenuItem::BUTTON, id, text, Texture(), x, y, -1, -1, r, g, b);
}
void Menu::addImage(int id, Texture texture, int x, int y, int w, int h, float r, float g, float b)
{
    items.emplace_back(MenuItem::IMAGE, id, "", texture, x, y, w, h, r, g, b);
}
void Menu::addButtonImage(int id, Texture texture, int x, int y, int w, int h, float r, float g, float b)
{
    items.emplace_back(MenuItem::IMAGEBUTTON, id, "", texture, x, y, w, h, r, g, b);
}
void Menu::addMapLine(int x, int y, int w, int h, float startsize, float endsize, float r, float g, float b)
{
    items.emplace_back(MenuItem::MAPLINE, -1, "", Texture(), x, y, w, h, r, g, b, startsize, endsize);
}
void Menu::addMapMarker(int id, Texture texture, int x, int y, int w, int h, float r, float g, float b)
{
    items.emplace_back(MenuItem::MAPMARKER, id, "", texture, x, y, w, h, r, g, b);
}
void Menu::addMapLabel(int id, const string& text, int x, int y, float r, float g, float b)
{
    items.emplace_back(MenuItem::MAPLABEL, id, text, Texture(), x, y, -1, -1, r, g, b);
}

void Menu::setText(int id, const string& text)
{
    for (vector<MenuItem>::iterator it = items.begin(); it != items.end(); it++)
        if (it->id == id) {
            it->text = text;
            it->w = it->text.length() * 10;
            break;
        }
}

void Menu::setText(int id, const string& text, int x, int y, int w, int h)
{
    for (vector<MenuItem>::iterator it = items.begin(); it != items.end(); it++)
        if (it->id == id) {
            it->text = text;
            it->x = x;
            it->y = y;
            if (w == -1)
                it->w = it->text.length() * 10;
            if (h == -1)
                it->h = 20;
            break;
        }
}

int Menu::getSelected(int mousex, int mousey)
{
    for (vector<MenuItem>::iterator it = items.begin(); it != items.end(); it++)
        if (it->type == MenuItem::BUTTON || it->type == MenuItem::IMAGEBUTTON || it->type == MenuItem::MAPMARKER) {
            int mx = mousex;
            int my = mousey;
            if (it->type == MenuItem::MAPMARKER) {
                mx -= 1;
                my += 2;
            }
            if (mx >= it->x && mx < it->x + it->w && my >= it->y && my < it->y + it->h)
                return it->id;
        }
    return -1;
}

void Menu::handleFadeEffect()
{
    for (vector<MenuItem>::iterator it = items.begin(); it != items.end(); it++) {
        if (it->id == Game::selected) {
            it->effectfade += multiplier * 5;
            if (it->effectfade > 1)
                it->effectfade = 1;
        } else {
            it->effectfade -= multiplier * 5;
            if (it->effectfade < 0)
                it->effectfade = 0;
        }
    }
}

void Menu::drawItems()
{
    handleFadeEffect();
    glEnable(GL_TEXTURE_2D);
    glEnable(GL_ALPHA_TEST);
    glEnable(GL_BLEND);
    for (vector<MenuItem>::iterator it = items.begin(); it != items.end(); it++) {
        switch (it->type) {
        case MenuItem::IMAGE:
        case MenuItem::IMAGEBUTTON:
        case MenuItem::MAPMARKER:
            glColor4f(it->r, it->g, it->b, 1);
            glPushMatrix();
            if (it->type == MenuItem::MAPMARKER) {
                glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
                glTranslatef(2.5, -4.5, 0); //from old code
            } else {
                glBlendFunc(GL_SRC_ALPHA, GL_ONE);
            }
            it->texture.bind();
            glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
            glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
            glBegin(GL_QUADS);
            glTexCoord2f(0, 0);
            glVertex3f(it->x, it->y, 0);
            glTexCoord2f(1, 0);
            glVertex3f(it->x + it->w, it->y, 0);
            glTexCoord2f(1, 1);
            glVertex3f(it->x + it->w, it->y + it->h, 0);
            glTexCoord2f(0, 1);
            glVertex3f(it->x, it->y + it->h, 0);
            glEnd();
            if (it->type != MenuItem::IMAGE) {
                //mouseover highlight
                for (int i = 0; i < 10; i++) {
                    if (1 - ((float)i) / 10 - (1 - it->effectfade) > 0) {
                        glColor4f(it->r, it->g, it->b, (1 - ((float)i) / 10 - (1 - it->effectfade))*.25);
                        glBegin(GL_QUADS);
                        glTexCoord2f(0, 0);
                        glVertex3f(it->x - ((float)i) * 1 / 2, it->y - ((float)i) * 1 / 2, 0);
                        glTexCoord2f(1, 0);
                        glVertex3f(it->x + it->w + ((float)i) * 1 / 2, it->y - ((float)i) * 1 / 2, 0);
                        glTexCoord2f(1, 1);
                        glVertex3f(it->x + it->w + ((float)i) * 1 / 2, it->y + it->h + ((float)i) * 1 / 2, 0);
                        glTexCoord2f(0, 1);
                        glVertex3f(it->x - ((float)i) * 1 / 2, it->y + it->h + ((float)i) * 1 / 2, 0);
                        glEnd();
                    }
                }
            }
            glPopMatrix();
            break;
        case MenuItem::LABEL:
        case MenuItem::BUTTON:
            glColor4f(it->r, it->g, it->b, 1);
            glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
            Game::text->glPrint(it->x, it->y, it->text.c_str(), 0, 1, 640, 480);
            if (it->type != MenuItem::LABEL) {
                //mouseover highlight
                glBlendFunc(GL_SRC_ALPHA, GL_ONE);
                for (int i = 0; i < 15; i++) {
                    if (1 - ((float)i) / 15 - (1 - it->effectfade) > 0) {
                        glColor4f(it->r, it->g, it->b, (1 - ((float)i) / 10 - (1 - it->effectfade))*.25);
                        Game::text->glPrint(it->x - ((float)i), it->y, it->text.c_str(), 0, 1 + ((float)i) / 70, 640, 480);
                    }
                }
            }
            break;
        case MenuItem::MAPLABEL:
            Game::text->glPrintOutlined(0.9, 0, 0, it->x, it->y, it->text.c_str(), 0, 0.6, 640, 480);
            break;
        case MenuItem::MAPLINE: {
            XYZ linestart;
            linestart.x = it->x;
            linestart.y = it->y;
            linestart.z = 0;
            XYZ lineend;
            lineend.x = it->x + it->w;
            lineend.y = it->y + it->h;
            lineend.z = 0;
            XYZ offset = lineend - linestart;
            XYZ fac = offset;
            Normalise(&fac);
            offset = DoRotation(offset, 0, 0, 90);
            Normalise(&offset);

            linestart += fac * 4 * it->linestartsize;
            lineend -= fac * 4 * it->lineendsize;

            glDisable(GL_TEXTURE_2D);
            glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
            glColor4f(it->r, it->g, it->b, 1);
            glPushMatrix();
            glTranslatef(2, -5, 0); //from old code
            glBegin(GL_QUADS);
            glVertex3f(linestart.x - offset.x * it->linestartsize, linestart.y - offset.y * it->linestartsize, 0.0f);
            glVertex3f(linestart.x + offset.x * it->linestartsize, linestart.y + offset.y * it->linestartsize, 0.0f);
            glVertex3f(lineend.x + offset.x * it->lineendsize, lineend.y + offset.y * it->lineendsize, 0.0f);
            glVertex3f(lineend.x - offset.x * it->lineendsize, lineend.y - offset.y * it->lineendsize, 0.0f);
            glEnd();
            glPopMatrix();
            glEnable(GL_TEXTURE_2D);
        }
        break;
        default:
        case MenuItem::NONE:
        break;
        }
    }
}

void Menu::updateSettingsMenu()
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

void Menu::updateStereoConfigMenu()
{
    char sbuf[256];
    sprintf(sbuf, "Stereo mode: %s", StereoModeName(newstereomode).c_str());
    Menu::setText(0, sbuf);
    sprintf(sbuf, "Stereo separation: %.3f", stereoseparation);
    Menu::setText(1, sbuf);
    sprintf(sbuf, "Reverse stereo: %s", stereoreverse ? "Yes" : "No");
    Menu::setText(2, sbuf);
}

void Menu::updateControlsMenu()
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
    if (debugmode) {
        Menu::setText(9, (string)"Console: " + (keyselect == 9 ? "_" : Input::keyToChar(consolekey)));
    }
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

void Menu::Load()
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
        if (debugmode) {
            Menu::addButton(9, "", 10 + 10, 40);
        }
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

void Menu::Tick()
{
    //escape key pressed
    if (Input::isKeyPressed(SDL_SCANCODE_ESCAPE) &&
        (mainmenu >= 3) && (mainmenu != 8) && !((mainmenu == 7) && entername)) {
        selected = -1;
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
                Menu::Load();
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
                        printf("Failed to initialize mode %s (%i)\n", StereoModeName(newstereomode).c_str(), newstereomode);
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

    OPENAL_SetFrequency(channels[stream_menutheme]);

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
            Menu::Load();
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
        Menu::Load();
    oldmainmenu = mainmenu;

}

int setKeySelected_thread(void* data)
{
    using namespace Game;
    int scancode = -1;
    SDL_Event evenement;
    while (scancode == -1) {
        SDL_WaitEvent(&evenement);
        switch (evenement.type) {
        case SDL_KEYDOWN:
            scancode = evenement.key.keysym.scancode;
            break;
        case SDL_MOUSEBUTTONDOWN:
            scancode = SDL_NUM_SCANCODES + evenement.button.button;
            break;
        default:
            break;
        }
    }
    if (scancode != SDL_SCANCODE_ESCAPE) {
        fireSound();
        switch (keyselect) {
        case 0:
            forwardkey = scancode;
            break;
        case 1:
            backkey = scancode;
            break;
        case 2:
            leftkey = scancode;
            break;
        case 3:
            rightkey = scancode;
            break;
        case 4:
            crouchkey = scancode;
            break;
        case 5:
            jumpkey = scancode;
            break;
        case 6:
            drawkey = scancode;
            break;
        case 7:
            throwkey = scancode;
            break;
        case 8:
            attackkey = scancode;
            break;
        case 9:
            consolekey = scancode;
            break;
        default:
            break;
        }
    }
    keyselect = -1;
    waiting = false;
    Menu::Load();
    return 0;
}

void Menu::setKeySelected()
{
    waiting = true;
    printf("launch thread\n");
    SDL_Thread* thread = SDL_CreateThread(setKeySelected_thread, NULL, NULL);
    if ( thread == NULL ) {
        fprintf(stderr, "Unable to create thread: %s\n", SDL_GetError());
        waiting = false;
        return;
    }
}
