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

#include "Menu/Menu.hpp"

#include "Audio/openal_wrapper.hpp"
#include "Graphic/gamegl.hpp"
#include "Level/Campaign.hpp"
#include "User/Settings.hpp"
#include "Utils/Input.hpp"
#include "Version.hpp"

// Should not be needed, Menu should call methods from other classes to launch maps and challenges and so on
#include "Level/Awards.hpp"

#include <set>
#include <string>
#include <vector>

using namespace Game;

extern float multiplier;
extern std::set<std::pair<int, int>> resolutions;
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
std::string newusername = "";
unsigned newuserselected = 0;
float newuserblinkdelay = 0;
bool newuserblink = false;

std::vector<MenuItem> Menu::items;

MenuItem::MenuItem(MenuItemType _type, int _id, const string& _text, Texture _texture,
                   int _x, int _y, int _w, int _h, float _r, float _g, float _b,
                   float _linestartsize, float _lineendsize)
    : type(_type)
    , id(_id)
    , text(_text)
    , texture(_texture)
    , x(_x)
    , y(_y)
    , w(_w)
    , h(_h)
    , r(_r)
    , g(_g)
    , b(_b)
    , effectfade(0)
    , linestartsize(_linestartsize)
    , lineendsize(_lineendsize)
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
    for (vector<MenuItem>::iterator it = items.begin(); it != items.end(); it++) {
        if (it->id == id) {
            it->text = text;
            it->w = it->text.length() * 10;
            break;
        }
    }
}

void Menu::setText(int id, const string& text, int x, int y, int w, int h)
{
    for (vector<MenuItem>::iterator it = items.begin(); it != items.end(); it++) {
        if (it->id == id) {
            it->text = text;
            it->x = x;
            it->y = y;
            if (w == -1) {
                it->w = it->text.length() * 10;
            }
            if (h == -1) {
                it->h = 20;
            }
            break;
        }
    }
}

int Menu::getSelected(int mousex, int mousey)
{
    for (vector<MenuItem>::reverse_iterator it = items.rbegin(); it != items.rend(); it++) {
        if (it->type == MenuItem::BUTTON || it->type == MenuItem::IMAGEBUTTON || it->type == MenuItem::MAPMARKER) {
            int mx = mousex;
            int my = mousey;
            if (it->type == MenuItem::MAPMARKER) {
                mx -= 1;
                my += 2;
            }
            if (mx >= it->x && mx < it->x + it->w && my >= it->y && my < it->y + it->h) {
                return it->id;
            }
        }
    }
    return -1;
}

void Menu::handleFadeEffect()
{
    for (vector<MenuItem>::iterator it = items.begin(); it != items.end(); it++) {
        if (it->id == Game::selected) {
            it->effectfade += multiplier * 5;
            if (it->effectfade > 1) {
                it->effectfade = 1;
            }
        } else {
            it->effectfade -= multiplier * 5;
            if (it->effectfade < 0) {
                it->effectfade = 0;
            }
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
                            glColor4f(it->r, it->g, it->b, (1 - ((float)i) / 10 - (1 - it->effectfade)) * .25);
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
                            glColor4f(it->r, it->g, it->b, (1 - ((float)i) / 10 - (1 - it->effectfade)) * .25);
                            Game::text->glPrint(it->x - ((float)i), it->y, it->text.c_str(), 0, 1 + ((float)i) / 70, 640, 480);
                        }
                    }
                }
                break;
            case MenuItem::MAPLABEL:
                Game::text->glPrintOutlined(0.9, 0, 0, 1, it->x, it->y, it->text.c_str(), 0, 0.6, 640, 480);
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
            } break;
            default:
            case MenuItem::NONE:
                break;
        }
    }
}

void Menu::updateSettingsMenu()
{
    std::string sbuf = std::string("Resolution: ") + to_string(newscreenwidth) + "*" + to_string(newscreenheight);
    if (((float)newscreenwidth <= (float)newscreenheight * 1.61) && ((float)newscreenwidth >= (float)newscreenheight * 1.59)) {
        sbuf += " (widescreen)";
    }
    setText(0, sbuf);
    setText(14, fullscreen ? "Fullscreen: On" : "Fullscreen: Off");
    if (newdetail == 0) {
        setText(1, "Detail: Low");
    }
    if (newdetail == 1) {
        setText(1, "Detail: Medium");
    }
    if (newdetail == 2) {
        setText(1, "Detail: High");
    }
    if (bloodtoggle == 0) {
        setText(2, "Blood: Off");
    }
    if (bloodtoggle == 1) {
        setText(2, "Blood: On, low detail");
    }
    if (bloodtoggle == 2) {
        setText(2, "Blood: On, high detail (slower)");
    }
    setText(4, ismotionblur ? "Blur Effects: Enabled (less compatible)" : "Blur Effects: Disabled (more compatible)");
    setText(5, decalstoggle ? "Decals: Enabled (slower)" : "Decals: Disabled");
    setText(6, musictoggle ? "Music: Enabled" : "Music: Disabled");
    setText(9, invertmouse ? "Invert mouse: Yes" : "Invert mouse: No");
    setText(10, std::string("Mouse Speed: ") + to_string(int(usermousesensitivity * 5)));
    setText(11, std::string("Volume: ") + to_string(int(volume * 100)) + "%");
    setText(13, showdamagebar ? "Damage Bar: On" : "Damage Bar: Off");
    if ((newdetail == detail) && (newscreenheight == (int)screenheight) && (newscreenwidth == (int)screenwidth)) {
        setText(8, "Back");
    } else {
        setText(8, "Back (some changes take effect next time Lugaru is opened)");
    }
}

void Menu::updateStereoConfigMenu()
{
    setText(0, std::string("Stereo mode: ") + StereoModeName(newstereomode));
    setText(1, std::string("Stereo separation: ") + to_string(stereoseparation));
    setText(2, std::string("Reverse stereo: ") + (stereoreverse ? "Yes" : "No"));
}

void Menu::updateControlsMenu()
{
    setText(0, (string) "Forwards: " + (keyselect == 0 ? "_" : Input::keyToChar(forwardkey)));
    setText(1, (string) "Back: " + (keyselect == 1 ? "_" : Input::keyToChar(backkey)));
    setText(2, (string) "Left: " + (keyselect == 2 ? "_" : Input::keyToChar(leftkey)));
    setText(3, (string) "Right: " + (keyselect == 3 ? "_" : Input::keyToChar(rightkey)));
    setText(4, (string) "Crouch: " + (keyselect == 4 ? "_" : Input::keyToChar(crouchkey)));
    setText(5, (string) "Jump: " + (keyselect == 5 ? "_" : Input::keyToChar(jumpkey)));
    setText(6, (string) "Draw: " + (keyselect == 6 ? "_" : Input::keyToChar(drawkey)));
    setText(7, (string) "Throw: " + (keyselect == 7 ? "_" : Input::keyToChar(throwkey)));
    setText(8, (string) "Attack: " + (keyselect == 8 ? "_" : Input::keyToChar(attackkey)));
    if (devtools) {
        setText(9, (string) "Console: " + (keyselect == 9 ? "_" : Input::keyToChar(consolekey)));
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
    clearMenu();
    switch (mainmenu) {
        case 1:
        case 2:
            addImage(0, Mainmenuitems[0], 150, 480 - 128, 256, 128);
            addButtonImage(1, Mainmenuitems[mainmenu == 1 ? 1 : 5], 18, 480 - 152 - 32, 128, 32);
            addButtonImage(2, Mainmenuitems[2], 18, 480 - 228 - 32, 112, 32);
            addButtonImage(3, Mainmenuitems[mainmenu == 1 ? 3 : 6], 18, 480 - 306 - 32, mainmenu == 1 ? 68 : 132, 32);
            addLabel(-1, VERSION_NUMBER + VERSION_SUFFIX, 640 - 100, 10);
            break;
        case 3:
            addButton(0, "", 10 + 20, 440);
            addButton(14, "", 10 + 400, 440);
            addButton(1, "", 10 + 60, 405);
            addButton(2, "", 10 + 70, 370);
            addButton(4, "", 10, 335);
            addButton(5, "", 10 + 60, 300);
            addButton(6, "", 10 + 70, 265);
            addButton(9, "", 10, 230);
            addButton(10, "", 20, 195);
            addButton(11, "", 10 + 60, 160);
            addButton(13, "", 30, 125);
            addButton(7, "-Configure Controls-", 10 + 15, 90);
            addButton(12, "-Configure Stereo -", 10 + 15, 55);
            addButton(8, "Back", 10, 10);
            updateSettingsMenu();
            break;
        case 4:
            addButton(0, "", 10, 400);
            addButton(1, "", 10 + 40, 360);
            addButton(2, "", 10 + 40, 320);
            addButton(3, "", 10 + 30, 280);
            addButton(4, "", 10 + 20, 240);
            addButton(5, "", 10 + 40, 200);
            addButton(6, "", 10 + 40, 160);
            addButton(7, "", 10 + 30, 120);
            addButton(8, "", 10 + 20, 80);
            if (devtools) {
                addButton(9, "", 10 + 10, 40);
            }
            addButton(devtools ? 10 : 9, "Back", 10, 10);
            updateControlsMenu();
            break;
        case 5: {
            LoadCampaign();
            addLabel(-1, Account::active().getName(), 5, 400);
            addButton(1, "Tutorial", 5, 300);
            addButton(2, "Challenge", 5, 240);
            addButton(3, "Delete User", 400, 10);
            addButton(4, "Main Menu", 5, 10);
            addButton(5, "Change User", 5, 180);
            addButton(6, "Campaign : " + Account::active().getCurrentCampaign(), 200, 420);

            //show campaign map
            //with (2,-5) offset from old code
            addImage(-1, Mainmenuitems[7], 150 + 2, 60 - 5, 400, 400);
            //show levels
            int numlevels = Account::active().getCampaignChoicesMade();
            numlevels += numlevels > 0 ? campaignlevels[numlevels - 1].nextlevel.size() : 1;
            for (int i = 0; i < numlevels; i++) {
                XYZ midpoint = campaignlevels[i].getCenter();
                float itemsize = campaignlevels[i].getWidth();
                const bool active = (i >= Account::active().getCampaignChoicesMade());
                if (!active) {
                    itemsize /= 2;
                }

                if (i >= 1) {
                    XYZ start = campaignlevels[i - 1].getCenter();
                    addMapLine(start.x, start.y, midpoint.x - start.x, midpoint.y - start.y, 0.5, active ? 1 : 0.5, active ? 1 : 0.5, 0, 0);
                }
                addMapMarker(NB_CAMPAIGN_MENU_ITEM + i, Mapcircletexture,
                             midpoint.x - itemsize / 2, midpoint.y - itemsize / 2, itemsize, itemsize, active ? 1 : 0.5, 0, 0);

                if (active) {
                    addMapLabel(-2, campaignlevels[i].description,
                                campaignlevels[i].getStartX() + 10,
                                campaignlevels[i].getStartY() - 4);
                }
            }
        } break;
        case 6:
            addLabel(-1, "Are you sure you want to delete this user?", 10, 400);
            addButton(1, "Yes", 10, 360);
            addButton(2, "No", 10, 320);
            break;
        case 7:
            if (Account::getNbAccounts() < 8) {
                addButton(0, "New User", 10, 400);
            } else {
                addLabel(0, "No More Users", 10, 400);
            }
            addLabel(-2, "", 20, 400);
            addButton(Account::getNbAccounts() + 1, "Back", 10, 10);
            for (int i = 0; i < Account::getNbAccounts(); i++) {
                addButton(i + 1, Account::get(i).getName(), 10, 340 - 20 * (i + 1));
            }
            break;
        case 8:
            addButton(0, "Easier", 10, 400);
            addButton(1, "Difficult", 10, 360);
            addButton(2, "Insane", 10, 320);
            break;
        case 9:
            for (int i = 0; i < numchallengelevels; i++) {
                string name = "Level ";
                name += to_string(i + 1);
                if (name.size() < 17) {
                    name.append((17 - name.size()), ' ');
                }
                name += to_string(int(Account::active().getHighScore(i)));
                if (name.size() < 32) {
                    name.append((32 - name.size()), ' ');
                }
                int fasttime = (int)round(Account::active().getFastTime(i));
                name += to_string(int((fasttime - fasttime % 60) / 60));
                name += ":";
                if (fasttime % 60 < 10) {
                    name += "0";
                }
                name += to_string(fasttime % 60);

                addButton(i, name, 10, 400 - i * 25, i > Account::active().getProgress() ? 0.5 : 1, 0, 0);
            }

            addButton(-1, "             High Score      Best Time", 10, 440);
            addButton(numchallengelevels, "Back", 10, 10);
            break;
        case 10: {
            addLabel(0, campaignEndText[0], 80, 330);
            addLabel(1, campaignEndText[1], 80, 300);
            addLabel(2, campaignEndText[2], 80, 270);
            addButton(3, "Back", 10, 10);
            addLabel(4, string("Your score:         ") + to_string((int)Account::active().getCampaignScore()), 190, 200);
            addLabel(5, string("Highest score:      ") + to_string((int)Account::active().getCampaignHighScore()), 190, 180);
        } break;
        case 18:
            addButton(0, "", 70, 400);
            addButton(1, "", 10, 360);
            addButton(2, "", 40, 320);
            addButton(3, "Back", 10, 10);
            updateStereoConfigMenu();
            break;
    }
}

void Menu::startChallengeLevel(int selected)
{
    fireSound();
    flash();

    startbonustotal = 0;

    loading = 2;
    loadtime = 0;
    targetlevel = selected;
    if (firstLoadDone) {
        TickOnceAfter();
    } else {
        LoadStuff();
    }
    LoadLevel(selected);
    campaign = 0;

    mainmenu = 0;
    gameon = 1;
    pause_sound(stream_menutheme);
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
    selected = getSelected(mousecoordh * 640 / screenwidth, 480 - mousecoordv * 480 / screenheight);

    // some specific case where we do something even if the left mouse button is not pressed.
    if ((mainmenu == 5) && (endgame == 2)) {
        Account::active().endGame();
        endgame = 0;
    }
    if (mainmenu == 10) {
        endgame = 2;
    }
    if (mainmenu == 18 && Input::isKeyPressed(MOUSEBUTTON_RIGHT) && selected == 1) {
        stereoseparation -= 0.001;
        updateStereoConfigMenu();
    }

    static int oldmainmenu = mainmenu;

    if (Input::MouseClicked() && (selected >= 0)) { // handling of the left mouse clic in menus
        set<pair<int, int>>::iterator newscreenresolution;
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
                            mainmenu = (Account::hasActive() ? 5 : 7);
                            selected = -1;
                        }
                        break;
                    case 2: //options
                        fireSound();
                        flash();
                        mainmenu = 3;
                        if (newdetail > 2) {
                            newdetail = detail;
                        }
                        if (newdetail < 0) {
                            newdetail = detail;
                        }
                        if (newscreenwidth > 3000) {
                            newscreenwidth = screenwidth;
                        }
                        if (newscreenwidth < 0) {
                            newscreenwidth = screenwidth;
                        }
                        if (newscreenheight > 3000) {
                            newscreenheight = screenheight;
                        }
                        if (newscreenheight < 0) {
                            newscreenheight = screenheight;
                        }
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
                        newscreenwidth = newscreenresolution->first;
                        newscreenheight = newscreenresolution->second;
                        break;
                    case 1:
                        newdetail++;
                        if (newdetail > 2) {
                            newdetail = 0;
                        }
                        break;
                    case 2:
                        bloodtoggle++;
                        if (bloodtoggle > 2) {
                            bloodtoggle = 0;
                        }
                        break;
                    case 4:
                        ismotionblur = !ismotionblur;
                        break;
                    case 5:
                        decalstoggle = !decalstoggle;
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
                        if (usermousesensitivity > 2) {
                            usermousesensitivity = .2;
                        }
                        break;
                    case 11:
                        volume += .1f;
                        if (volume > 1.0001f) {
                            volume = 0;
                        }
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
                    if (selected < (devtools ? 10 : 9) && keyselect == -1) {
                        keyselect = selected;
                    }
                    if (keyselect != -1) {
                        setKeySelected();
                    }
                    if (selected == (devtools ? 10 : 9)) {
                        flash();
                        mainmenu = 3;
                    }
                }
                updateControlsMenu();
                break;
            case 5:
                fireSound();
                flash();
                if ((selected - NB_CAMPAIGN_MENU_ITEM >= Account::active().getCampaignChoicesMade())) {
                    startbonustotal = 0;

                    loading = 2;
                    loadtime = 0;
                    targetlevel = 7;
                    if (firstLoadDone) {
                        TickOnceAfter();
                    } else {
                        LoadStuff();
                    }
                    whichchoice = selected - NB_CAMPAIGN_MENU_ITEM - Account::active().getCampaignChoicesMade();
                    actuallevel = (Account::active().getCampaignChoicesMade() > 0 ? campaignlevels[Account::active().getCampaignChoicesMade() - 1].nextlevel[whichchoice] : 0);
                    visibleloading = true;
                    stillloading = 1;
                    LoadLevel(campaignlevels[actuallevel].mapname.c_str());
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
                        if (firstLoadDone) {
                            TickOnceAfter();
                        } else {
                            LoadStuff();
                        }
                        LoadLevel(-1);

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
                        if ((c = find(campaigns.begin(), campaigns.end(), Account::active().getCurrentCampaign())) == campaigns.end()) {
                            if (!campaigns.empty()) {
                                Account::active().setCurrentCampaign(campaigns.front());
                            }
                        } else {
                            c++;
                            if (c == campaigns.end()) {
                                c = campaigns.begin();
                            }
                            Account::active().setCurrentCampaign(*c);
                        }
                        Load();
                        break;
                }
                break;
            case 6:
                fireSound();
                if (selected == 1) {
                    flash();
                    Account::destroyActive();
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
                    Account::setActive(selected - 1);
                } else if (selected == Account::getNbAccounts() + 1) {
                    flash();
                    if (Account::hasActive()) {
                        mainmenu = 5;
                    } else {
                        mainmenu = 1;
                    }
                    newusername.clear();
                    newuserselected = 0;
                    entername = 0;
                }
                break;
            case 8:
                fireSound();
                flash();
                if (selected <= 2) {
                    Account::active().setDifficulty(selected);
                }
                mainmenu = 5;
                break;
            case 9:
                if (selected < numchallengelevels && selected <= Account::active().getProgress()) {
                    startChallengeLevel(selected);
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
                if (selected == 1) {
                    stereoseparation += 0.001;
                } else {
                    fireSound();
                    if (selected == 0) {
                        newstereomode = (StereoMode)(newstereomode + 1);
                        while (!CanInitStereo(newstereomode)) {
                            printf("Failed to initialize mode %s (%i)\n", StereoModeName(newstereomode).c_str(), newstereomode);
                            newstereomode = (StereoMode)(newstereomode + 1);
                            if (newstereomode >= stereoCount) {
                                newstereomode = stereoNone;
                            }
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
        inputText(newusername, &newuserselected);
        if (!waiting) {                 // the input as finished
            if (!newusername.empty()) { // with enter
                Account::add(string(newusername));

                mainmenu = 8;

                flash();

                fireSound(firestartsound);

                newusername.clear();

                newuserselected = 0;
            }
            entername = 0;
            Load();
        }

        newuserblinkdelay -= multiplier;
        if (newuserblinkdelay <= 0) {
            newuserblinkdelay = .3;
            newuserblink = !newuserblink;
        }
    }

    if (entername) {
        setText(0, newusername, 20, 400, -1, -1);
        setText(-2, newuserblink ? "_" : "", 20 + newuserselected * 10, 400, -1, -1);
    }

    if (oldmainmenu != mainmenu) {
        Load();
    }
    oldmainmenu = mainmenu;
}

int setKeySelected_thread(void*)
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
    if (thread == NULL) {
        fprintf(stderr, "Unable to create thread: %s\n", SDL_GetError());
        waiting = false;
        return;
    }
}
