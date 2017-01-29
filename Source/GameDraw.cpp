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

#include "Game.hpp"

#include "Audio/openal_wrapper.hpp"
#include "Level/Awards.hpp"
#include "Level/Dialog.hpp"
#include "Level/Hotspot.hpp"
#include "Menu/Menu.hpp"
#include "Tutorial.hpp"
#include "Utils/Input.hpp"

extern XYZ viewer;
extern int environment;
extern float texscale;
extern Light light;
extern Terrain terrain;
extern float multiplier;
extern float viewdistance;
extern float fadestart;
extern float screenwidth, screenheight;
extern int kTextureSize;
extern FRUSTUM frustum;
extern Light light;
extern int detail;
extern float usermousesensitivity;
extern float camerashake;
extern int slomo;
extern float slomodelay;
extern bool ismotionblur;
extern float woozy;
extern float blackout;
extern bool damageeffects;
extern float volume;
extern bool texttoggle;
extern float blurness;
extern float targetblurness;
extern float playerdist;
extern bool cellophane;
extern bool freeze;
extern float flashamount, flashr, flashg, flashb;
extern int flashdelay;
extern int netstate;
extern float motionbluramount;
extern bool isclient;
extern bool alwaysblur;
extern bool velocityblur;
extern bool devtools;
extern int mainmenu;
extern int bloodtoggle;
extern int difficulty;
extern bool decalstoggle;
extern float texdetail;
extern bool musictoggle;
extern float smoketex;
extern bool againbonus;
extern float damagedealt;
extern bool invertmouse;

extern bool campaign;
extern bool winfreeze;

extern bool gamestart;

extern bool gamestarted;

extern bool showdamagebar;

int drawtoggle = 0;
int numboundaries = 0;
XYZ boundary[360];
int change = 0;

enum drawmodes
{
    normalmode,
    motionblurmode,
    radialzoommode,
    realmotionblurmode,
    doublevisionmode,
    glowmode,
};

void Game::flash(float amount, int delay) // shouldn't be that way, these should be attributes and Person class should not change rendering.
{
    flashr = 1;
    flashg = 0;
    flashb = 0;
    flashamount = amount;
    flashdelay = delay;
}

void DrawMenu();

/*********************> DrawGLScene() <*****/
int Game::DrawGLScene(StereoSide side)
{
    static float texcoordwidth, texcoordheight;
    static float texviewwidth, texviewheight;
    static XYZ checkpoint;
    static float tempmult;
    float tutorialopac;
    std::string string;
    static int drawmode = 0;

    if (stereomode == stereoAnaglyph) {
        switch (side) {
            case stereoLeft:
                glColorMask(0.0, 1.0, 1.0, 1.0);
                break;
            case stereoRight:
                glColorMask(1.0, 0.0, 0.0, 1.0);
                break;
            default:
                break;
        }
    } else {
        glColorMask(1.0, 1.0, 1.0, 1.0);

        if (stereomode == stereoHorizontalInterlaced ||
            stereomode == stereoVerticalInterlaced) {
            glStencilFunc(side == stereoLeft ? GL_NOTEQUAL : GL_EQUAL, 0x01, 0x01);
        }
    }

    if (freeze || winfreeze || (mainmenu && gameon) || (!gameon && gamestarted)) {
        tempmult = multiplier;
        multiplier = 0;
    }

    if (!mainmenu) {
        if (editorenabled) {
            numboundaries = mapradius * 2;
            if (numboundaries > 360) {
                numboundaries = 360;
            }
            for (int i = 0; i < numboundaries; i++) {
                boundary[i] = 0;
                boundary[i].z = 1;
                boundary[i] = mapcenter + DoRotation(boundary[i] * mapradius, 0, i * (360 / ((float)(numboundaries))), 0);
            }
        }

        SetUpLighting();

        static int changed;
        changed = 0;

        int olddrawmode = drawmode;
        if (ismotionblur && !loading) {
            if ((findLengthfast(&Person::players[0]->velocity) > 200) && velocityblur && !cameramode) {
                drawmode = motionblurmode;
                motionbluramount = 200 / (findLengthfast(&Person::players[0]->velocity));
                changed = 1;
            }
            if (Person::players[0]->damage - Person::players[0]->superpermanentdamage > (Person::players[0]->damagetolerance - Person::players[0]->superpermanentdamage) * 1 / 2 && damageeffects && !cameramode) {
                drawmode = doublevisionmode;
                changed = 1;
            }
        }

        if (slomo && !loading) {
            if (ismotionblur) {
                drawmode = motionblurmode;
            }
            motionbluramount = .2;
            slomodelay -= multiplier;
            if (slomodelay < 0) {
                slomo = 0;
            }
            camerashake = 0;
            changed = 1;
        }
        if ((!changed && !slomo) || loading) {
            drawmode = normalmode;
            if (ismotionblur && (/*fps>100||*/ alwaysblur)) {
                if (olddrawmode != realmotionblurmode) {
                    change = 1;
                } else {
                    change = 0;
                }
                drawmode = realmotionblurmode;
            } else if (olddrawmode == realmotionblurmode) {
                change = 2;
            } else {
                change = 0;
            }
        }

        if (freeze || winfreeze || (mainmenu && gameon) || (!gameon && gamestarted)) {
            drawmode = normalmode;
        }
        if ((freeze || winfreeze) && ismotionblur && !mainmenu) {
            drawmode = radialzoommode;
        }

        if (winfreeze || mainmenu) {
            drawmode = normalmode;
        }

        if (drawtoggle != 2) {
            drawtoggle = 1 - drawtoggle;
        }

        if (!texcoordwidth) {
            texviewwidth = kTextureSize;
            if (texviewwidth > screenwidth) {
                texviewwidth = screenwidth;
            }
            texviewheight = kTextureSize;
            if (texviewheight > screenheight) {
                texviewheight = screenheight;
            }

            texcoordwidth = screenwidth / kTextureSize;
            texcoordheight = screenheight / kTextureSize;
            if (texcoordwidth > 1) {
                texcoordwidth = 1;
            }
            if (texcoordheight > 1) {
                texcoordheight = 1;
            }
        }

        glDrawBuffer(GL_BACK);
        glReadBuffer(GL_BACK);

        static XYZ terrainlight;
        static float distance;
        if (drawmode == normalmode) {
            Game::ReSizeGLScene(90, .1f);
        } else {
            glViewport(0, 0, texviewwidth, texviewheight);
        }
        glDepthFunc(GL_LEQUAL);
        glDepthMask(1);
        glAlphaFunc(GL_GREATER, 0.0001f);
        glEnable(GL_ALPHA_TEST);
        glClearColor(0.25f, 0.25f, 0.25f, 1.0f);
        glClear(GL_DEPTH_BUFFER_BIT);

        glMatrixMode(GL_MODELVIEW);
        glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
        glLoadIdentity();

        // Move the camera for the current eye's point of view.
        // Reverse the movement if we're reversing stereo
        glTranslatef((stereoseparation / 2) * side * (stereoreverse ? -1 : 1), 0, 0);

        //camera effects
        if (!cameramode && !freeze && !winfreeze) {
            //shake
            glRotatef(float(Random() % 100) / 10 * camerashake /*+(woozy*woozy)/10*/, 0, 0, 1);
            //sway
            glRotatef(pitch + sin(woozy / 2) * (Person::players[0]->damage / Person::players[0]->damagetolerance) * 5, 1, 0, 0);
            glRotatef(yaw + sin(woozy) * (Person::players[0]->damage / Person::players[0]->damagetolerance) * 5, 0, 1, 0);
        }
        if (cameramode || freeze || winfreeze) {
            glRotatef(pitch, 1, 0, 0);
            glRotatef(yaw, 0, 1, 0);
        }

        if (environment == desertenvironment) {
            glRotatef((float)(abs(Random() % 100)) / 3000 - 1, 1, 0, 0);
            glRotatef((float)(abs(Random() % 100)) / 3000 - 1, 0, 1, 0);
        }
        SetUpLight(&light, 0);
        glPushMatrix();

        //heat blur effect in desert
        if (abs(blurness - targetblurness) < multiplier * 10 || abs(blurness - targetblurness) > 2) {
            blurness = targetblurness;
            targetblurness = (float)(abs(Random() % 100)) / 40;
        }
        if (blurness < targetblurness) {
            blurness += multiplier * 5;
        } else {
            blurness -= multiplier * 5;
        }

        if (environment == desertenvironment) {
            if (detail == 2) {
                glTexEnvf(GL_TEXTURE_FILTER_CONTROL, GL_TEXTURE_LOD_BIAS, blurness + .4);
            }
            glRotatef((float)(abs(Random() % 100)) / 1000, 1, 0, 0);
            glRotatef((float)(abs(Random() % 100)) / 1000, 0, 1, 0);
        }
        skybox->draw();
        glTexEnvf(GL_TEXTURE_FILTER_CONTROL, GL_TEXTURE_LOD_BIAS, 0);
        glPopMatrix();
        glTranslatef(-viewer.x, -viewer.y, -viewer.z);
        frustum.GetFrustum();

        //make shadow decals on terrain and Object::objects
        static XYZ point;
        static float size, opacity, rotation;
        rotation = 0;
        for (unsigned int k = 0; k < Person::players.size(); k++) {
            if (!Person::players[k]->skeleton.free && Person::players[k]->playerdetail && Person::players[k]->howactive < typesleeping) {
                if (frustum.SphereInFrustum(Person::players[k]->coords.x, Person::players[k]->coords.y + Person::players[k]->scale * 3, Person::players[k]->coords.z, Person::players[k]->scale * 7) && Person::players[k]->occluded < 25) {
                    for (unsigned int i = 0; i < Person::players[k]->skeleton.joints.size(); i++) {
                        if (Person::players[k]->skeleton.joints[i].label == leftknee || Person::players[k]->skeleton.joints[i].label == rightknee || Person::players[k]->skeleton.joints[i].label == groin) {
                            point = DoRotation(Person::players[k]->skeleton.joints[i].position, 0, Person::players[k]->yaw, 0) * Person::players[k]->scale + Person::players[k]->coords;
                            size = .4f;
                            opacity = .4 - Person::players[k]->skeleton.joints[i].position.y * Person::players[k]->scale / 5 - (Person::players[k]->coords.y - terrain.getHeight(Person::players[k]->coords.x, Person::players[k]->coords.z)) / 10;
                            if (k != 0 && Tutorial::active) {
                                opacity = .2 + .2 * sin(smoketex * 6 + i) - Person::players[k]->skeleton.joints[i].position.y * Person::players[k]->scale / 5 - (Person::players[k]->coords.y - terrain.getHeight(Person::players[k]->coords.x, Person::players[k]->coords.z)) / 10;
                            }
                            terrain.MakeDecal(shadowdecal, point, size, opacity, rotation);
                            for (unsigned int l = 0; l < terrain.patchobjects[Person::players[k]->whichpatchx][Person::players[k]->whichpatchz].size(); l++) {
                                unsigned int j = terrain.patchobjects[Person::players[k]->whichpatchx][Person::players[k]->whichpatchz][l];
                                if (Object::objects[j]->position.y < Person::players[k]->coords.y || Object::objects[j]->type == tunneltype || Object::objects[j]->type == weirdtype) {
                                    point = DoRotation(DoRotation(Person::players[k]->skeleton.joints[i].position, 0, Person::players[k]->yaw, 0) * Person::players[k]->scale + Person::players[k]->coords - Object::objects[j]->position, 0, -Object::objects[j]->yaw, 0);
                                    size = .4f;
                                    opacity = .4f;
                                    if (k != 0 && Tutorial::active) {
                                        opacity = .2 + .2 * sin(smoketex * 6 + i);
                                    }
                                    Object::objects[j]->model.MakeDecal(shadowdecal, &point, &size, &opacity, &rotation);
                                }
                            }
                        }
                    }
                }
            }
            if ((Person::players[k]->skeleton.free || Person::players[k]->howactive >= typesleeping) && Person::players[k]->playerdetail) {
                if (frustum.SphereInFrustum(Person::players[k]->coords.x, Person::players[k]->coords.y, Person::players[k]->coords.z, Person::players[k]->scale * 5) && Person::players[k]->occluded < 25) {
                    for (unsigned i = 0; i < Person::players[k]->skeleton.joints.size(); i++) {
                        if (Person::players[k]->skeleton.joints[i].label == leftknee || Person::players[k]->skeleton.joints[i].label == rightknee || Person::players[k]->skeleton.joints[i].label == groin || Person::players[k]->skeleton.joints[i].label == leftelbow || Person::players[k]->skeleton.joints[i].label == rightelbow || Person::players[k]->skeleton.joints[i].label == neck) {
                            if (Person::players[k]->skeleton.free) {
                                point = Person::players[k]->skeleton.joints[i].position * Person::players[k]->scale + Person::players[k]->coords;
                            } else {
                                point = DoRotation(Person::players[k]->skeleton.joints[i].position, 0, Person::players[k]->yaw, 0) * Person::players[k]->scale + Person::players[k]->coords;
                            }
                            size = .4f;
                            opacity = .4 - Person::players[k]->skeleton.joints[i].position.y * Person::players[k]->scale / 5 - (Person::players[k]->coords.y - terrain.getHeight(Person::players[k]->coords.x, Person::players[k]->coords.z)) / 5;
                            if (k != 0 && Tutorial::active) {
                                opacity = .2 + .2 * sin(smoketex * 6 + i) - Person::players[k]->skeleton.joints[i].position.y * Person::players[k]->scale / 5 - (Person::players[k]->coords.y - terrain.getHeight(Person::players[k]->coords.x, Person::players[k]->coords.z)) / 10;
                            }
                            terrain.MakeDecal(shadowdecal, point, size, opacity * .7, rotation);
                            for (unsigned int l = 0; l < terrain.patchobjects[Person::players[k]->whichpatchx][Person::players[k]->whichpatchz].size(); l++) {
                                unsigned int j = terrain.patchobjects[Person::players[k]->whichpatchx][Person::players[k]->whichpatchz][l];
                                if (Object::objects[j]->position.y < Person::players[k]->coords.y || Object::objects[j]->type == tunneltype || Object::objects[j]->type == weirdtype) {
                                    if (Person::players[k]->skeleton.free) {
                                        point = DoRotation(Person::players[k]->skeleton.joints[i].position * Person::players[k]->scale + Person::players[k]->coords - Object::objects[j]->position, 0, -Object::objects[j]->yaw, 0);
                                    } else {
                                        point = DoRotation(DoRotation(Person::players[k]->skeleton.joints[i].position, 0, Person::players[k]->yaw, 0) * Person::players[k]->scale + Person::players[k]->coords - Object::objects[j]->position, 0, -Object::objects[j]->yaw, 0);
                                    }
                                    size = .4f;
                                    opacity = .4f;
                                    if (k != 0 && Tutorial::active) {
                                        opacity = .2 + .2 * sin(smoketex * 6 + i);
                                    }
                                    Object::objects[j]->model.MakeDecal(shadowdecal, &point, &size, &opacity, &rotation);
                                }
                            }
                        }
                    }
                }
            }

            if (!Person::players[k]->playerdetail) {
                if (frustum.SphereInFrustum(Person::players[k]->coords.x, Person::players[k]->coords.y, Person::players[k]->coords.z, Person::players[k]->scale * 5)) {
                    point = Person::players[k]->coords;
                    size = .7;
                    opacity = .4 - (Person::players[k]->coords.y - terrain.getHeight(Person::players[k]->coords.x, Person::players[k]->coords.z)) / 5;
                    terrain.MakeDecal(shadowdecal, point, size, opacity * .7, rotation);
                    for (unsigned int l = 0; l < terrain.patchobjects[Person::players[k]->whichpatchx][Person::players[k]->whichpatchz].size(); l++) {
                        unsigned int j = terrain.patchobjects[Person::players[k]->whichpatchx][Person::players[k]->whichpatchz][l];
                        point = DoRotation(Person::players[k]->coords - Object::objects[j]->position, 0, -Object::objects[j]->yaw, 0);
                        size = .7;
                        opacity = .4f;
                        Object::objects[j]->model.MakeDecal(shadowdecal, &point, &size, &opacity, &rotation);
                    }
                }
            }
        }

        //Terrain
        glEnable(GL_TEXTURE_2D);
        glDepthMask(1);
        glEnable(GL_DEPTH_TEST);
        glEnable(GL_CULL_FACE);
        glDisable(GL_BLEND);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
        terraintexture.bind();
        terrain.draw(0);
        terraintexture2.bind();
        terrain.draw(1);

        terrain.drawdecals();

        //Model
        glEnable(GL_CULL_FACE);
        glEnable(GL_LIGHTING);
        glDisable(GL_BLEND);
        glEnable(GL_TEXTURE_2D);
        glDepthMask(1);

        glEnable(GL_COLOR_MATERIAL);

        if (!cellophane) {
            glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
            glEnable(GL_CULL_FACE);
            glCullFace(GL_FRONT);
            glDepthMask(1);
            for (unsigned k = 0; k < Person::players.size(); k++) {
                if (k == 0 || !Tutorial::active) {
                    glEnable(GL_BLEND);
                    glEnable(GL_LIGHTING);
                    terrainlight = terrain.getLighting(Person::players[k]->coords.x, Person::players[k]->coords.z);
                    distance = distsq(&viewer, &Person::players[k]->coords);
                    distance = (viewdistance * viewdistance - (distance - (viewdistance * viewdistance * fadestart)) * (1 / (1 - fadestart))) / viewdistance / viewdistance;
                    glColor4f(terrainlight.x, terrainlight.y, terrainlight.z, distance);
                    if (distance >= 1) {
                        glDisable(GL_BLEND);
                    }
                    if (distance >= .5) {
                        checkpoint = DoRotation(Person::players[k]->skeleton.joints[fabs(Random() % Person::players[k]->skeleton.joints.size())].position, 0, Person::players[k]->yaw, 0) * Person::players[k]->scale + Person::players[k]->coords;
                        checkpoint.y += 1;
                        int i = -1;
                        if (Person::players[k]->occluded != 0) {
                            i = Object::checkcollide(viewer, checkpoint, Person::players[k]->lastoccluded);
                        }
                        if (i == -1) {
                            i = Object::checkcollide(viewer, checkpoint);
                        }
                        if (i != -1) {
                            Person::players[k]->occluded += 1;
                            Person::players[k]->lastoccluded = i;
                        } else {
                            Person::players[k]->occluded = 0;
                        }
                        if (Person::players[k]->occluded < 25) {
                            Person::players[k]->DrawSkeleton();
                        }
                    }
                }
            }
        }

        if (!cameramode && musictype == stream_fighttheme) {
            playerdist = distsqflat(&Person::players[0]->coords, &viewer);
        } else {
            playerdist = -100;
        }
        glPushMatrix();
        glCullFace(GL_BACK);
        glEnable(GL_TEXTURE_2D);
        Object::Draw();
        glPopMatrix();

        //draw hawk
        glPushMatrix();
        if (frustum.SphereInFrustum(realhawkcoords.x + hawk.boundingspherecenter.x, realhawkcoords.y + hawk.boundingspherecenter.y, realhawkcoords.z + hawk.boundingspherecenter.z, 2)) {
            glAlphaFunc(GL_GREATER, 0.0001f);
            glDepthMask(1);
            glDisable(GL_CULL_FACE);
            glDisable(GL_LIGHTING);
            glEnable(GL_BLEND);
            glTranslatef(hawkcoords.x, hawkcoords.y, hawkcoords.z);
            glRotatef(hawkyaw, 0, 1, 0);
            glTranslatef(25, 0, 0);
            distance = distsq(&viewer, &realhawkcoords) * 1.2;
            glColor4f(light.color[0], light.color[1], light.color[2], (viewdistance * viewdistance - (distance - (viewdistance * viewdistance * fadestart)) * (1 / (1 - fadestart))) / viewdistance / viewdistance);
            if ((viewdistance * viewdistance - (distance - (viewdistance * viewdistance * fadestart)) * (1 / (1 - fadestart))) / viewdistance / viewdistance > 1) {
                glColor4f(light.color[0], light.color[1], light.color[2], 1);
            }
            if ((viewdistance * viewdistance - (distance - (viewdistance * viewdistance * fadestart)) * (1 / (1 - fadestart))) / viewdistance / viewdistance > 0) {
                hawk.drawdifftex(hawktexture);
            }
        }
        glPopMatrix();

        glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
        glEnable(GL_CULL_FACE);
        glCullFace(GL_FRONT);
        glDepthMask(1);
        for (unsigned k = 0; k < Person::players.size(); k++) {
            if (!(k == 0 || !Tutorial::active)) {
                glEnable(GL_BLEND);
                glEnable(GL_LIGHTING);
                terrainlight = terrain.getLighting(Person::players[k]->coords.x, Person::players[k]->coords.z);
                distance = distsq(&viewer, &Person::players[k]->coords);
                distance = (viewdistance * viewdistance - (distance - (viewdistance * viewdistance * fadestart)) * (1 / (1 - fadestart))) / viewdistance / viewdistance;
                glColor4f(terrainlight.x, terrainlight.y, terrainlight.z, distance);
                if (distance >= 1) {
                    glDisable(GL_BLEND);
                }
                if (distance >= .5) {
                    checkpoint = DoRotation(Person::players[k]->skeleton.joints[fabs(Random() % Person::players[k]->skeleton.joints.size())].position, 0, Person::players[k]->yaw, 0) * Person::players[k]->scale + Person::players[k]->coords;
                    checkpoint.y += 1;
                    int i = -1;
                    if (Person::players[k]->occluded != 0) {
                        i = Object::checkcollide(viewer, checkpoint, Person::players[k]->lastoccluded);
                    }
                    if (i == -1) {
                        i = Object::checkcollide(viewer, checkpoint);
                    }
                    if (i != -1) {
                        Person::players[k]->occluded += 1;
                        Person::players[k]->lastoccluded = i;
                    } else {
                        Person::players[k]->occluded = 0;
                    }
                    if (Person::players[k]->occluded < 25) {
                        Person::players[k]->DrawSkeleton();
                    }
                }
            }
        }

        glPushMatrix();
        glEnable(GL_TEXTURE_2D);
        weapons.Draw();
        glPopMatrix();
        glCullFace(GL_BACK);

        glDisable(GL_COLOR_MATERIAL);

        glDisable(GL_LIGHTING);
        glDisable(GL_TEXTURE_2D);

        glDepthMask(0);

        Sprite::Draw();

        //waypoints, pathpoints in editor
        if (editorenabled) {
            glEnable(GL_BLEND);
            glDisable(GL_LIGHTING);
            glDisable(GL_TEXTURE_2D);
            glDisable(GL_COLOR_MATERIAL);
            glColor4f(1, 1, 0, 1);

            for (unsigned k = 0; k < Person::players.size(); k++) {
                if (Person::players[k]->numwaypoints > 1) {
                    glBegin(GL_LINE_LOOP);
                    for (int i = 0; i < Person::players[k]->numwaypoints; i++) {
                        glVertex3f(Person::players[k]->waypoints[i].x, Person::players[k]->waypoints[i].y + .5, Person::players[k]->waypoints[i].z);
                    }
                    glEnd();
                }
            }

            if (numpathpoints > 1) {
                glColor4f(0, 1, 0, 1);
                for (unsigned k = 0; int(k) < numpathpoints; k++) {
                    if (numpathpointconnect[k]) {
                        for (int i = 0; i < numpathpointconnect[k]; i++) {
                            glBegin(GL_LINE_LOOP);
                            glVertex3f(pathpoint[k].x, pathpoint[k].y + .5, pathpoint[k].z);
                            glVertex3f(pathpoint[pathpointconnect[k][i]].x, pathpoint[pathpointconnect[k][i]].y + .5, pathpoint[pathpointconnect[k][i]].z);
                            glEnd();
                        }
                    }
                }
                glColor4f(1, 1, 1, 1);
                glPointSize(4);
                glBegin(GL_POINTS);
                glVertex3f(pathpoint[pathpointselected].x, pathpoint[pathpointselected].y + .5, pathpoint[pathpointselected].z);
                glEnd();
            }
        }

        //Text

        glEnable(GL_TEXTURE_2D);
        glColor4f(.5, .5, .5, 1);
        if (!console) {
            if (!Tutorial::active) {
                if (bonus > 0 && bonustime < 1 && !winfreeze && !Dialog::inDialog()) {
                    const char* bonus_name;
                    if (bonus < bonus_count) {
                        bonus_name = bonus_names[bonus];
                    } else {
                        bonus_name = "Excellent!"; // When does this happen?
                    }
                    text->glPrintOutlined(1, 0, 0, 1 - bonustime, 1024 / 2 - 10 * strlen(bonus_name), 768 / 16 + 768 * 4 / 5, bonus_name, 1, 2, 1024, 768);

                    string = to_string(bonusvalue);
                    text->glPrintOutlined(1, 0, 0, 1 - bonustime, 1024 / 2 - 10 * string.size(), 768 / 16 - 20 + 768 * 4 / 5, string, 1, 2 * .8, 1024, 768);

                    glColor4f(.5, .5, .5, 1);
                }
            }

            if (Tutorial::active) {
                Tutorial::DrawTextInfo();
            }

            //Hot spots
            if (Hotspot::hotspots.size() && (bonustime >= 1 || bonus <= 0 || bonustime < 0) && !Tutorial::active) {
                float closestdist = -1;
                int closest = Hotspot::current;
                for (unsigned i = 0; i < Hotspot::hotspots.size(); i++) {
                    float distance = distsq(&Person::players[0]->coords, &Hotspot::hotspots[i].position);
                    if (closestdist == -1 || distance < closestdist) {
                        if (distsq(&Person::players[0]->coords, &Hotspot::hotspots[i].position) < Hotspot::hotspots[i].size && ((Hotspot::hotspots[i].type <= 10 && Hotspot::hotspots[i].type >= 0) || (Hotspot::hotspots[i].type <= 40 && Hotspot::hotspots[i].type >= 20))) {
                            closestdist = distance;
                            closest = i;
                        }
                    }
                }
                if (closest != -1) {
                    Hotspot::current = closest;
                    if (Hotspot::hotspots[closest].type <= 10) {
                        if (distsq(&Person::players[0]->coords, &Hotspot::hotspots[closest].position) < Hotspot::hotspots[closest].size) {
                            Tutorial::stagetime = 0;
                        }
                        Tutorial::maxtime = 1;
                        tutorialopac = Tutorial::maxtime - Tutorial::stagetime;
                        if (tutorialopac > 1) {
                            tutorialopac = 1;
                        }
                        if (tutorialopac < 0) {
                            tutorialopac = 0;
                        }

                        string = Hotspot::hotspots[closest].text;

                        int lastline = 0;
                        int line = 0;
                        bool done = false;
                        int i = 0;
                        while (!done) {
                            if (string[i] == '\n' || string[i] > 'z' || string[i] < ' ' || string[i] == '\0') {
                                text->glPrintOutlined(1, 1, 1, tutorialopac, screenwidth / 2 - 7.6 * (i - lastline) * screenwidth / 1024, screenheight / 16 + screenheight * 4 / 5 - 20 * screenwidth / 1024 * line, string, 1, 1.5 * screenwidth / 1024, screenwidth, screenheight, lastline, i);
                                lastline = i + 1;
                                line++;
                                if (string[i] == '\0') {
                                    done = 1;
                                }
                            }
                            if (i >= 255) {
                                done = 1;
                            }
                            i++;
                        }
                    } else if ((Hotspot::hotspots[closest].type >= 20) && (Dialog::dialogs[Hotspot::hotspots[closest].type - 20].gonethrough == 0)) {
                        Dialog::whichdialogue = Hotspot::hotspots[closest].type - 20;
                        Dialog::currentDialog().play();
                        Dialog::currentDialog().gonethrough++;
                    }
                }
            }

            /* Drawing dialogs */
            if (Dialog::inDialog() && !mainmenu) {
                glDisable(GL_DEPTH_TEST);
                glDisable(GL_CULL_FACE);
                glDisable(GL_LIGHTING);
                glDisable(GL_TEXTURE_2D);
                glDepthMask(0);
                glMatrixMode(GL_PROJECTION);
                glPushMatrix();
                glLoadIdentity();
                glOrtho(0, screenwidth, 0, screenheight, -100, 100);
                glMatrixMode(GL_MODELVIEW);
                glPushMatrix();
                glLoadIdentity();
                if (Dialog::currentScene().location == 1) {
                    glTranslatef(0, screenheight * 3 / 4, 0);
                }
                glScalef(screenwidth, screenheight / 4, 1);
                glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
                glEnable(GL_BLEND);

                glColor4f(Dialog::currentScene().color[0], Dialog::currentScene().color[1], Dialog::currentScene().color[2], 0.7);
                glBegin(GL_QUADS);
                glVertex3f(0, 0, 0.0f);
                glVertex3f(1, 0, 0.0f);
                glVertex3f(1, 1, 0.0f);
                glVertex3f(0, 1, 0.0f);
                glEnd();
                glMatrixMode(GL_PROJECTION);
                glPopMatrix();
                glMatrixMode(GL_MODELVIEW);
                glPopMatrix();
                glEnable(GL_DEPTH_TEST);
                glEnable(GL_CULL_FACE);
                glDisable(GL_BLEND);
                glDepthMask(1);
                glEnable(GL_TEXTURE_2D);

                tutorialopac = 1;

                float startx;
                float starty;

                startx = screenwidth * 1 / 5;
                if (Dialog::currentScene().location == 1) {
                    starty = screenheight / 16 + screenheight * 4 / 5;
                } else {
                    starty = screenheight * 1 / 5 - screenheight / 16;
                }

                /* Get speaker name, and remove potential '#' chars hardcoded in it. */
                string = Dialog::currentScene().name + ": ";
                string.erase(std::remove(string.begin(), string.end(), '#'), string.end());

                /* Print speaker name in dialog box. */
                if (Dialog::currentScene().color[0] + Dialog::currentScene().color[1] + Dialog::currentScene().color[2] < 1.5) {
                    text->glPrintOutlined(0.7, 0.7, 0.7, tutorialopac, startx - 2 * 7.6 * string.size() * screenwidth / 1024, starty, string, 1, 1.4 * screenwidth / 1024, screenwidth, screenheight);
                } else {
                    glColor4f(0, 0, 0, tutorialopac);
                    text->glPrintOutline(startx - 2 * 7.6 * string.size() * screenwidth / 1024 - 4, starty - 4, string, 1, 1.4 * 1.25 * screenwidth / 1024, screenwidth, screenheight);
                }

                /* Get dialog text, and remove potential '#' chars hardcoded in it.' */
                string = Dialog::currentScene().text;
                string.erase(std::remove(string.begin(), string.end(), '#'), string.end());

                /* Print dialog text in dialog box. */
                int lastline = 0;
                int line = 0;
                bool done = false;
                int i = 0;
                while (!done) {
                    if (string[i] == '\n' || string[i] > 'z' || string[i] < ' ' || string[i] == '\0') {
                        if (Dialog::currentScene().color[0] + Dialog::currentScene().color[1] + Dialog::currentScene().color[2] < 1.5) {
                            text->glPrintOutlined(1, 1, 1, tutorialopac, startx, starty - 20 * screenwidth / 1024 * line, string, 1, 1.4 * screenwidth / 1024, screenwidth, screenheight, lastline, i);
                        } else {
                            glColor4f(0, 0, 0, tutorialopac);
                            text->glPrint(startx, starty - 20 * screenwidth / 1024 * line, string, 1, 1.4 * screenwidth / 1024, screenwidth, screenheight, lastline, i);
                        }
                        lastline = i + 1;
                        line++;
                        if (string[i] == '\0') {
                            done = 1;
                        }
                    }
                    if (i >= 255) {
                        done = 1;
                    }
                    i++;
                }
            }

            if (!Tutorial::active && !winfreeze && !Dialog::inDialog() && !mainmenu) {
                if (campaign) {
                    if (scoreadded) {
                        string = "Score: " + to_string(int(Account::active().getCampaignScore()));
                    } else {
                        string = "Score: " + to_string(int(Account::active().getCampaignScore() + bonustotal));
                    }
                } else {
                    string = "Score: " + to_string(int(bonustotal));
                }
                text->glPrintOutlined(1, 0, 0, 1, 1024 / 40, 768 / 16 + 768 * 14 / 16, string, 1, 1.5, 1024, 768);
                if (showdamagebar) {
                    glDisable(GL_DEPTH_TEST);
                    glDisable(GL_CULL_FACE);
                    glDisable(GL_LIGHTING);
                    glDisable(GL_TEXTURE_2D);
                    glDepthMask(0);
                    glMatrixMode(GL_PROJECTION);
                    glPushMatrix();
                    glLoadIdentity();
                    glOrtho(0, screenwidth, 0, screenheight, -100, 100);
                    glMatrixMode(GL_MODELVIEW);
                    glPushMatrix();
                    glLoadIdentity();
                    glTranslatef(15, screenheight * 17.5 / 20, 0);
                    glScalef(screenwidth / 3 + 20, screenheight / 20, 1);
                    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
                    glEnable(GL_BLEND);
                    glColor4f(0.0, 0.4, 0.0, 0.7);
                    float bar = ((float)Person::players[0]->damage) / Person::players[0]->damagetolerance;
                    glBegin(GL_QUADS);
                    glVertex3f((bar < 1 ? bar : 1), 0, 0.0f);
                    glVertex3f(1, 0, 0.0f);
                    glVertex3f(1, 1, 0.0f);
                    glVertex3f((bar < 1 ? bar : 1), 1, 0.0f);
                    glEnd();
                    glColor4f(0.1, 0.0, 0.0, 1);
                    bar = ((float)Person::players[0]->bloodloss) / Person::players[0]->damagetolerance;
                    glBegin(GL_QUADS);
                    glVertex3f(0, 0, 0.0f);
                    glVertex3f((bar < 1 ? bar : 1), 0, 0.0f);
                    glVertex3f((bar < 1 ? bar : 1), 1, 0.0f);
                    glVertex3f(0, 1, 0.0f);
                    glEnd();
                    glColor4f(0.4, 0.0, 0.0, 0.7);
                    bar = ((float)Person::players[0]->damage) / Person::players[0]->damagetolerance;
                    glBegin(GL_QUADS);
                    glVertex3f(0, 0, 0.0f);
                    glVertex3f((bar < 1 ? bar : 1), 0, 0.0f);
                    glVertex3f((bar < 1 ? bar : 1), 1, 0.0f);
                    glVertex3f(0, 1, 0.0f);
                    glEnd();
                    glColor4f(0.4, 0.0, 0.0, 0.7);
                    bar = ((float)Person::players[0]->permanentdamage) / Person::players[0]->damagetolerance;
                    glBegin(GL_QUADS);
                    glVertex3f(0, 0, 0.0f);
                    glVertex3f((bar < 1 ? bar : 1), 0, 0.0f);
                    glVertex3f((bar < 1 ? bar : 1), 1, 0.0f);
                    glVertex3f(0, 1, 0.0f);
                    glEnd();
                    glColor4f(0.4, 0.0, 0.0, 0.7);
                    bar = ((float)Person::players[0]->superpermanentdamage) / Person::players[0]->damagetolerance;
                    glBegin(GL_QUADS);
                    glVertex3f(0, 0, 0.0f);
                    glVertex3f((bar < 1 ? bar : 1), 0, 0.0f);
                    glVertex3f((bar < 1 ? bar : 1), 1, 0.0f);
                    glVertex3f(0, 1, 0.0f);
                    glEnd();
                    glColor4f(0.0, 0.0, 0.0, 0.7);
                    glLineWidth(2.0);
                    glBegin(GL_LINE_STRIP);
                    glVertex3f(0, 0, 0.0f);
                    glVertex3f(1, 0, 0.0f);
                    glVertex3f(1, 1, 0.0f);
                    glVertex3f(0, 1, 0.0f);
                    glVertex3f(0, 0, 0.0f);
                    glEnd();

                    glMatrixMode(GL_PROJECTION);
                    glPopMatrix();
                    glMatrixMode(GL_MODELVIEW);
                    glPopMatrix();
                    glEnable(GL_DEPTH_TEST);
                    glEnable(GL_CULL_FACE);
                    glDisable(GL_BLEND);
                    glDepthMask(1);
                    glEnable(GL_TEXTURE_2D);

                    // writing the numbers :
                    string = "Damages : " + to_string(int(Person::players[0]->damage)) + "/" + to_string(int(Person::players[0]->damagetolerance)) + " (" + to_string(int(Person::players[0]->bloodloss)) + ")";
                    text->glPrintOutlined(1, 0, 0, 1, 1024 / 40, 768 / 16 + 768 * 14 / 16 - 40, string, 1, 1.5, 1024, 768);
                }
            }

            glColor4f(.5, .5, .5, 1);

            if ((texttoggle || editorenabled) && devtools && !mainmenu) {
                string = "The framespersecond is " + to_string(int(fps));
                text->glPrint(10, 30, string, 0, .8, 1024, 768);

                if (editorenabled) {
                    string = "Map editor enabled.";
                } else {
                    string = "Map editor disabled.";
                }
                text->glPrint(10, 60, string, 0, .8, 1024, 768);
                if (editorenabled) {
                    string = "Object size: " + to_string(editorsize);
                    text->glPrint(10, 75, string, 0, .8, 1024, 768);
                    if (editoryaw >= 0) {
                        string = "Object yaw: " + to_string(editoryaw);
                    } else {
                        string = "Object yaw: Random";
                    }
                    text->glPrint(10, 90, string, 0, .8, 1024, 768);
                    if (editorpitch >= 0) {
                        string = "Object pitch: " + to_string(editorpitch);
                    } else {
                        string = "Object pitch: Random";
                    }
                    text->glPrint(10, 105, string, 0, .8, 1024, 768);
                    string = "Object type: " + to_string(editortype);
                    text->glPrint(10, 120, string, 0, .8, 1024, 768);
                    switch (editortype) {
                        case boxtype:
                            string = "(box)";
                            break;
                        case treetrunktype:
                            string = "(tree)";
                            break;
                        case walltype:
                            string = "(wall)";
                            break;
                        case weirdtype:
                            string = "(weird)";
                            break;
                        case spiketype:
                            string = "(spike)";
                            break;
                        case rocktype:
                            string = "(rock)";
                            break;
                        case bushtype:
                            string = "(bush)";
                            break;
                        case tunneltype:
                            string = "(tunnel)";
                            break;
                        case chimneytype:
                            string = "(chimney)";
                            break;
                        case platformtype:
                            string = "(platform)";
                            break;
                        case cooltype:
                            string = "(cool)";
                            break;
                        case firetype:
                            string = "(fire)";
                            break;
                    }
                    text->glPrint(130, 120, string, 0, .8, 1024, 768);

                    string = "Numplayers: " + to_string(Person::players.size());
                    text->glPrint(10, 155, string, 0, .8, 1024, 768);
                    string = "Player " + to_string(int(Person::players.size()) - 1) + ": numwaypoints: " + to_string(Person::players.back()->numwaypoints);
                    text->glPrint(10, 140, string, 0, .8, 1024, 768);
                }
                string = "Difficulty: " + to_string(difficulty);
                text->glPrint(10, 240, string, 0, .8, 1024, 768);
            }
        }

        if (drawmode == glowmode) {
            glDisable(GL_DEPTH_TEST);
            glDisable(GL_CULL_FACE);
            glDisable(GL_LIGHTING);
            glDisable(GL_TEXTURE_2D);
            glDepthMask(0);
            glMatrixMode(GL_PROJECTION);
            glPushMatrix();
            glLoadIdentity();
            glOrtho(0, screenwidth, 0, screenheight, -100, 100);
            glMatrixMode(GL_MODELVIEW);
            glPushMatrix();
            glLoadIdentity();
            glScalef(screenwidth, screenheight, 1);
            glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
            glEnable(GL_BLEND);
            glColor4f(0, 0, 0, .5);
            glBegin(GL_QUADS);
            glVertex3f(0, 0, 0.0f);
            glVertex3f(256, 0, 0.0f);
            glVertex3f(256, 256, 0.0f);
            glVertex3f(0, 256, 0.0f);
            glEnd();
            glMatrixMode(GL_PROJECTION);
            glPopMatrix();
            glMatrixMode(GL_MODELVIEW);
            glPopMatrix();
            glEnable(GL_DEPTH_TEST);
            glEnable(GL_CULL_FACE);
            glDisable(GL_BLEND);
            glDepthMask(1);
        }

        if ((((blackout && damageeffects) || (Person::players[0]->bloodloss > 0 && damageeffects && Person::players[0]->blooddimamount > 0) || Person::players[0]->dead) && !cameramode) || console) {
            glDisable(GL_DEPTH_TEST);
            glDisable(GL_CULL_FACE);
            glDisable(GL_LIGHTING);
            glDisable(GL_TEXTURE_2D);
            glDepthMask(0);
            glMatrixMode(GL_PROJECTION);
            glPushMatrix();
            glLoadIdentity();
            glOrtho(0, screenwidth, 0, screenheight, -100, 100);
            glMatrixMode(GL_MODELVIEW);
            glPushMatrix();
            glLoadIdentity();
            glScalef(screenwidth, screenheight, 1);
            glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
            glEnable(GL_BLEND);
            if (Person::players[0]->dead) {
                blackout += multiplier * 3;
            }
            if (Person::players[0]->dead == 1) {
                blackout = .4f;
            }
            if (Person::players[0]->dead == 2 && blackout > .6) {
                blackout = .6;
            }
            glColor4f(0, 0, 0, blackout);
            if (!Person::players[0]->dead) {
                if ((Person::players[0]->bloodloss / Person::players[0]->damagetolerance * (sin(woozy) / 4 + .5)) * .3 < .3) {
                    glColor4f(0, 0, 0, Person::players[0]->blooddimamount * Person::players[0]->bloodloss / Person::players[0]->damagetolerance * (sin(woozy) / 4 + .5) * .3);
                    blackout = Person::players[0]->blooddimamount * Person::players[0]->bloodloss / Person::players[0]->damagetolerance * (sin(woozy) / 4 + .5) * .3;
                } else {
                    glColor4f(0, 0, 0, Person::players[0]->blooddimamount * .3);
                    blackout = Person::players[0]->blooddimamount * .3;
                }
            }
            if (console) {
                glColor4f(.7, 0, 0, .2);
            }
            glBegin(GL_QUADS);
            glVertex3f(0, 0, 0.0f);
            glVertex3f(256, 0, 0.0f);
            glVertex3f(256, 256, 0.0f);
            glVertex3f(0, 256, 0.0f);
            glEnd();
            glMatrixMode(GL_PROJECTION);
            glPopMatrix();
            glMatrixMode(GL_MODELVIEW);
            glPopMatrix();
            glEnable(GL_DEPTH_TEST);
            glEnable(GL_CULL_FACE);
            glDisable(GL_BLEND);
            glDepthMask(1);
        }

        if (flashamount > 0 && damageeffects) {
            if (flashamount > 1) {
                flashamount = 1;
            }
            if (flashdelay <= 0) {
                flashamount -= multiplier;
            }
            flashdelay--;
            if (flashamount < 0) {
                flashamount = 0;
            }
            glDisable(GL_DEPTH_TEST);
            glDisable(GL_CULL_FACE);
            glDisable(GL_LIGHTING);
            glDepthMask(0);
            glMatrixMode(GL_PROJECTION);
            glPushMatrix();
            glLoadIdentity();
            glOrtho(0, screenwidth, 0, screenheight, -100, 100);
            glMatrixMode(GL_MODELVIEW);
            glPushMatrix();
            glLoadIdentity();
            glScalef(screenwidth, screenheight, 1);
            glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
            glEnable(GL_BLEND);
            glColor4f(flashr, flashg, flashb, flashamount);
            glBegin(GL_QUADS);
            glVertex3f(0, 0, 0.0f);
            glVertex3f(256, 0, 0.0f);
            glVertex3f(256, 256, 0.0f);
            glVertex3f(0, 256, 0.0f);
            glEnd();
            glMatrixMode(GL_PROJECTION);
            glPopMatrix();
            glMatrixMode(GL_MODELVIEW);
            glPopMatrix();
            glEnable(GL_DEPTH_TEST);
            glEnable(GL_CULL_FACE);
            glDisable(GL_BLEND);
            glDepthMask(1);
        }

        if (difficulty < 2 && !Dialog::inDialog()) { // minimap
            float mapviewdist = 20000;

            glDisable(GL_DEPTH_TEST);
            glColor3f(1.0, 1.0, 1.0); // no coloring

            glEnable(GL_TEXTURE_2D);
            glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
            glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
            glDisable(GL_DEPTH_TEST);
            glDisable(GL_CULL_FACE);
            glDisable(GL_LIGHTING);
            glDepthMask(0);
            glMatrixMode(GL_PROJECTION);
            glPushMatrix();
            glLoadIdentity();
            glOrtho(0, screenwidth, 0, screenheight, -100, 100);
            glMatrixMode(GL_MODELVIEW);
            glPushMatrix();
            glLoadIdentity();
            glScalef((float)screenwidth / 2, (float)screenwidth / 2, 1);
            glTranslatef(1.75, .25, 0);
            glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
            glEnable(GL_BLEND);
            glColor4f(1, 1, 1, 1);
            glPushMatrix();
            float opac = .7;
            XYZ center;
            float radius;
            float distcheck;

            center = Person::players[0]->coords;

            radius = 110;

            glScalef(.25 / radius * 256 * terrain.scale * .4, .25 / radius * 256 * terrain.scale * .4, 1);
            glPushMatrix();
            glScalef(1 / (1 / radius * 256 * terrain.scale * .4), 1 / (1 / radius * 256 * terrain.scale * .4), 1);
            glPopMatrix();
            glRotatef(Person::players[0]->lookyaw * -1 + 180, 0, 0, 1);
            glTranslatef(-(center.x / terrain.scale / 256 * -2 + 1), (center.z / terrain.scale / 256 * -2 + 1), 0);
            for (unsigned int i = 0; i < Object::objects.size(); i++) {
                if (Object::objects[i]->type == treetrunktype) {
                    distcheck = distsq(&Person::players[0]->coords, &Object::objects[i]->position);
                    if (distcheck < mapviewdist) {
                        Mapcircletexture.bind();
                        glColor4f(0, .3, 0, opac * (1 - distcheck / mapviewdist));
                        glPushMatrix();
                        glTranslatef(Object::objects[i]->position.x / terrain.scale / 256 * -2 + 1, Object::objects[i]->position.z / terrain.scale / 256 * 2 - 1, 0);
                        glRotatef(Object::objects[i]->yaw, 0, 0, 1);
                        glScalef(.003, .003, .003);
                        glBegin(GL_QUADS);
                        glTexCoord2f(0, 0);
                        glVertex3f(-1, -1, 0.0f);
                        glTexCoord2f(1, 0);
                        glVertex3f(1, -1, 0.0f);
                        glTexCoord2f(1, 1);
                        glVertex3f(1, 1, 0.0f);
                        glTexCoord2f(0, 1);
                        glVertex3f(-1, 1, 0.0f);
                        glEnd();
                        glPopMatrix();
                    }
                }
                if (Object::objects[i]->type == boxtype) {
                    distcheck = distsq(&Person::players[0]->coords, &Object::objects[i]->position);
                    if (distcheck < mapviewdist) {
                        Mapboxtexture.bind();
                        glColor4f(.4, .4, .4, opac * (1 - distcheck / mapviewdist));
                        glPushMatrix();
                        glTranslatef(Object::objects[i]->position.x / terrain.scale / 256 * -2 + 1, Object::objects[i]->position.z / terrain.scale / 256 * 2 - 1, 0);
                        glRotatef(Object::objects[i]->yaw, 0, 0, 1);
                        glScalef(.01 * Object::objects[i]->scale, .01 * Object::objects[i]->scale, .01 * Object::objects[i]->scale);
                        glBegin(GL_QUADS);
                        glTexCoord2f(0, 0);
                        glVertex3f(-1, -1, 0.0f);
                        glTexCoord2f(1, 0);
                        glVertex3f(1, -1, 0.0f);
                        glTexCoord2f(1, 1);
                        glVertex3f(1, 1, 0.0f);
                        glTexCoord2f(0, 1);
                        glVertex3f(-1, 1, 0.0f);
                        glEnd();
                        glPopMatrix();
                    }
                }
            }
            if (editorenabled) {
                Mapcircletexture.bind();
                for (int i = 0; i < numboundaries; i++) {
                    glColor4f(0, 0, 0, opac / 3);
                    glPushMatrix();
                    glTranslatef(boundary[i].x / terrain.scale / 256 * -2 + 1, boundary[i].z / terrain.scale / 256 * 2 - 1, 0);
                    glScalef(.002, .002, .002);
                    glBegin(GL_QUADS);
                    glTexCoord2f(0, 0);
                    glVertex3f(-1, -1, 0.0f);
                    glTexCoord2f(1, 0);
                    glVertex3f(1, -1, 0.0f);
                    glTexCoord2f(1, 1);
                    glVertex3f(1, 1, 0.0f);
                    glTexCoord2f(0, 1);
                    glVertex3f(-1, 1, 0.0f);
                    glEnd();
                    glPopMatrix();
                }
            }
            for (unsigned i = 0; i < Person::players.size(); i++) {
                distcheck = distsq(&Person::players[0]->coords, &Person::players[i]->coords);
                if (distcheck < mapviewdist) {
                    glPushMatrix();
                    Maparrowtexture.bind();
                    if (i == 0) {
                        glColor4f(1, 1, 1, opac);
                    } else if (Person::players[i]->dead == 2 || Person::players[i]->howactive > typesleeping) {
                        glColor4f(0, 0, 0, opac * (1 - distcheck / mapviewdist));
                    } else if (Person::players[i]->dead) {
                        glColor4f(.3, .3, .3, opac * (1 - distcheck / mapviewdist));
                    } else if (Person::players[i]->aitype == attacktypecutoff) {
                        glColor4f(1, 0, 0, opac * (1 - distcheck / mapviewdist));
                    } else if (Person::players[i]->aitype == passivetype) {
                        glColor4f(0, 1, 0, opac * (1 - distcheck / mapviewdist));
                    } else {
                        glColor4f(1, 1, 0, 1);
                    }
                    glTranslatef(Person::players[i]->coords.x / terrain.scale / 256 * -2 + 1, Person::players[i]->coords.z / terrain.scale / 256 * 2 - 1, 0);
                    glRotatef(Person::players[i]->yaw + 180, 0, 0, 1);
                    glScalef(.005, .005, .005);
                    glBegin(GL_QUADS);
                    glTexCoord2f(0, 0);
                    glVertex3f(-1, -1, 0.0f);
                    glTexCoord2f(1, 0);
                    glVertex3f(1, -1, 0.0f);
                    glTexCoord2f(1, 1);
                    glVertex3f(1, 1, 0.0f);
                    glTexCoord2f(0, 1);
                    glVertex3f(-1, 1, 0.0f);
                    glEnd();
                    glPopMatrix();
                }
            }
            glPopMatrix();
            glDisable(GL_TEXTURE_2D);
            glMatrixMode(GL_PROJECTION);
            glPopMatrix();
            glMatrixMode(GL_MODELVIEW);
            glPopMatrix();
            glEnable(GL_DEPTH_TEST);
            glEnable(GL_CULL_FACE);
            glDisable(GL_BLEND);
            glDepthMask(1);
        }

        if (loading && !stealthloading && (!campaign || Person::players[0]->dead)) {
            glDisable(GL_DEPTH_TEST);
            glDisable(GL_CULL_FACE);
            glDisable(GL_LIGHTING);
            glDisable(GL_TEXTURE_2D);
            glDepthMask(0);
            glMatrixMode(GL_PROJECTION);
            glPushMatrix();
            glLoadIdentity();
            glOrtho(0, screenwidth, 0, screenheight, -100, 100);
            glMatrixMode(GL_MODELVIEW);
            glPushMatrix();
            glLoadIdentity();
            glScalef(screenwidth, screenheight, 1);
            glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
            glEnable(GL_BLEND);
            glColor4f(0, 0, 0, .7);
            glBegin(GL_QUADS);
            glVertex3f(0, 0, 0.0f);
            glVertex3f(256, 0, 0.0f);
            glVertex3f(256, 256, 0.0f);
            glVertex3f(0, 256, 0.0f);
            glEnd();
            glMatrixMode(GL_PROJECTION);
            glPopMatrix();
            glMatrixMode(GL_MODELVIEW);
            glPopMatrix();
            glEnable(GL_DEPTH_TEST);
            glEnable(GL_CULL_FACE);
            glDisable(GL_BLEND);
            glDepthMask(1);

            //logo
            glDisable(GL_DEPTH_TEST);
            glColor3f(1.0, 1.0, 1.0); // no coloring

            glEnable(GL_TEXTURE_2D);

            //Minimap

            if (loading != 4) {
                glEnable(GL_TEXTURE_2D);
                glColor4f(1, 1, 1, 1);
                string = "Loading...";
                text->glPrint(1024 / 2 - 90, 768 / 2, string, 1, 2, 1024, 768);
            }
            loading = 2;
            drawmode = normalmode;
        }

        if (winfreeze && !campaign) {
            glDisable(GL_DEPTH_TEST);
            glDisable(GL_CULL_FACE);
            glDisable(GL_LIGHTING);
            glDisable(GL_TEXTURE_2D);
            glDepthMask(0);
            glMatrixMode(GL_PROJECTION);
            glPushMatrix();
            glLoadIdentity();
            glOrtho(0, screenwidth, 0, screenheight, -100, 100);
            glMatrixMode(GL_MODELVIEW);
            glPushMatrix();
            glLoadIdentity();
            glScalef(screenwidth, screenheight, 1);
            glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
            glEnable(GL_BLEND);
            glColor4f(0, 0, 0, .4);
            glBegin(GL_QUADS);
            glVertex3f(0, 0, 0.0f);
            glVertex3f(256, 0, 0.0f);
            glVertex3f(256, 256, 0.0f);
            glVertex3f(0, 256, 0.0f);
            glEnd();
            glMatrixMode(GL_PROJECTION);
            glPopMatrix();
            glMatrixMode(GL_MODELVIEW);
            glPopMatrix();
            glEnable(GL_DEPTH_TEST);
            glEnable(GL_CULL_FACE);
            glDisable(GL_BLEND);
            glDepthMask(1);

            //logo
            glDisable(GL_DEPTH_TEST);
            glColor3f(1.0, 1.0, 1.0); // no coloring

            glEnable(GL_TEXTURE_2D);

            //Win Screen Won Victory

            glEnable(GL_TEXTURE_2D);
            glColor4f(1, 1, 1, 1);
            string = "Level Cleared!";
            text->glPrintOutlined(1024 / 2 - string.size() * 10, 768 * 7 / 8, string, 1, 2, 1024, 768);

            string = "Score:     " + to_string(int(bonustotal - startbonustotal));
            text->glPrintOutlined(1024 / 30, 768 * 6 / 8, string, 1, 2, 1024, 768);

            string = "Press Escape to return to menu or Space to continue";
            text->glPrintOutlined(640 / 2 - string.size() * 5, 480 * 1 / 16, string, 1, 1, 640, 480);

            int wontime = (int)round(wonleveltime);
            string = "Time:      " + to_string(int((wontime - wontime % 60) / 60));
            if (wontime % 60 < 10) {
                string += "0";
            }
            string += to_string(int(wontime % 60));
            text->glPrintOutlined(1024 / 30, 768 * 6 / 8 - 40, string, 1, 2, 1024, 768);

            //Awards
            int awards[award_count];
            int numawards = award_awards(awards);

            for (int i = 0; i < numawards && i < 6; i++) {
                text->glPrintOutlined(1024 / 30, 768 * 6 / 8 - 90 - 40 * i, award_names[awards[i]], 1, 2, 1024, 768);
            }
        }

        if (drawmode != normalmode) {
            glEnable(GL_TEXTURE_2D);
            glFinish();
            if (!drawtoggle || drawmode != realmotionblurmode || (drawtoggle == 2 || change == 1)) {
                if (screentexture) {

                    glTexEnvi(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_COMBINE);
                    GLfloat subtractColor[4] = { 0.5, 0.5, 0.5, 0.0 };
                    glTexEnvfv(GL_TEXTURE_ENV, GL_TEXTURE_ENV_COLOR, subtractColor);
                    glTexEnvi(GL_TEXTURE_ENV, GL_SOURCE0_RGB, GL_TEXTURE);
                    glTexEnvi(GL_TEXTURE_ENV, GL_SOURCE1_RGB, GL_CONSTANT);
                    glTexEnvf(GL_TEXTURE_ENV, GL_RGB_SCALE, 2.0f);

                    glBindTexture(GL_TEXTURE_2D, screentexture);
                    glCopyTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, 0, 0, texviewwidth, texviewheight);
                }
            }
            if ((drawtoggle || change == 1) && drawmode == realmotionblurmode) {
                if (screentexture2) {
                    glBindTexture(GL_TEXTURE_2D, screentexture2);
                    glCopyTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, 0, 0, texviewwidth, texviewheight);
                }
                if (!screentexture2) {
                    glPixelStorei(GL_UNPACK_ALIGNMENT, 1);

                    glGenTextures(1, &screentexture2);
                    glTexEnvi(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE);

                    glEnable(GL_TEXTURE_2D);
                    glBindTexture(GL_TEXTURE_2D, screentexture2);
                    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
                    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);

                    glCopyTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, 0, 0, kTextureSize, kTextureSize, 0);
                }
            }
        }

        glClear(GL_DEPTH_BUFFER_BIT);
        Game::ReSizeGLScene(90, .1f);
        glViewport(0, 0, screenwidth, screenheight);

        if (drawmode != normalmode) {
            glDisable(GL_DEPTH_TEST);
            if (drawmode == motionblurmode) {
                glDrawBuffer(GL_FRONT);
                glReadBuffer(GL_BACK);
            }
            glColor3f(1.0, 1.0, 1.0); // no coloring

            glEnable(GL_TEXTURE_2D);
            glBindTexture(GL_TEXTURE_2D, screentexture);
            glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
            glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
            glDisable(GL_DEPTH_TEST);
            glDisable(GL_CULL_FACE);
            glDisable(GL_LIGHTING);
            glDepthMask(0);
            glMatrixMode(GL_PROJECTION);
            glPushMatrix();
            glLoadIdentity();
            glOrtho(0, screenwidth, 0, screenheight, -100, 100);
            glMatrixMode(GL_MODELVIEW);
            glPushMatrix();
            glLoadIdentity();
            glScalef((float)screenwidth / 2, (float)screenheight / 2, 1);
            glTranslatef(1, 1, 0);
            glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
            glEnable(GL_BLEND);
            if (drawmode == motionblurmode) {
                if (motionbluramount < .2) {
                    motionbluramount = .2;
                }
                glColor4f(1, 1, 1, motionbluramount);
                glPushMatrix();
                glBegin(GL_QUADS);
                glTexCoord2f(0, 0);
                glVertex3f(-1, -1, 0.0f);
                glTexCoord2f(texcoordwidth, 0);
                glVertex3f(1, -1, 0.0f);
                glTexCoord2f(texcoordwidth, texcoordheight);
                glVertex3f(1, 1, 0.0f);
                glTexCoord2f(0, texcoordheight);
                glVertex3f(-1, 1, 0.0f);
                glEnd();
                glPopMatrix();
            }
            if (drawmode == realmotionblurmode) {
                glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
                glClear(GL_COLOR_BUFFER_BIT);
                glBlendFunc(GL_SRC_ALPHA, GL_ONE);
                glBindTexture(GL_TEXTURE_2D, screentexture);
                glColor4f(1, 1, 1, .5);
                glPushMatrix();
                glBegin(GL_QUADS);
                glTexCoord2f(0, 0);
                glVertex3f(-1, -1, 0.0f);
                glTexCoord2f(texcoordwidth, 0);
                glVertex3f(1, -1, 0.0f);
                glTexCoord2f(texcoordwidth, texcoordheight);
                glVertex3f(1, 1, 0.0f);
                glTexCoord2f(0, texcoordheight);
                glVertex3f(-1, 1, 0.0f);
                glEnd();
                glPopMatrix();
                glBindTexture(GL_TEXTURE_2D, screentexture2);
                glColor4f(1, 1, 1, .5);
                glPushMatrix();
                glBegin(GL_QUADS);
                glTexCoord2f(0, 0);
                glVertex3f(-1, -1, 0.0f);
                glTexCoord2f(texcoordwidth, 0);
                glVertex3f(1, -1, 0.0f);
                glTexCoord2f(texcoordwidth, texcoordheight);
                glVertex3f(1, 1, 0.0f);
                glTexCoord2f(0, texcoordheight);
                glVertex3f(-1, 1, 0.0f);
                glEnd();
                glPopMatrix();
                glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
            }
            if (drawmode == doublevisionmode) {
                static float crosseyedness;
                crosseyedness = abs(Person::players[0]->damage - Person::players[0]->superpermanentdamage - (Person::players[0]->damagetolerance - Person::players[0]->superpermanentdamage) * 1 / 2) / 30;
                if (crosseyedness > 1) {
                    crosseyedness = 1;
                }
                if (crosseyedness < 0) {
                    crosseyedness = 0;
                }
                glColor4f(1, 1, 1, 1);
                glDisable(GL_BLEND);
                glPushMatrix();
                glScalef(1, 1, 1);
                glBegin(GL_QUADS);
                glTexCoord2f(0, 0);
                glVertex3f(-1, -1, 0.0f);
                glTexCoord2f(texcoordwidth, 0);
                glVertex3f(1, -1, 0.0f);
                glTexCoord2f(texcoordwidth, texcoordheight);
                glVertex3f(1, 1, 0.0f);
                glTexCoord2f(0, texcoordheight);
                glVertex3f(-1, 1, 0.0f);
                glEnd();
                glPopMatrix();
                if (crosseyedness) {
                    glColor4f(1, 1, 1, .5);
                    glEnable(GL_BLEND);
                    glPushMatrix();
                    glTranslatef(.015 * crosseyedness, 0, 0);
                    glScalef(1, 1, 1);
                    glBegin(GL_QUADS);
                    glTexCoord2f(0, 0);
                    glVertex3f(-1, -1, 0.0f);
                    glTexCoord2f(texcoordwidth, 0);
                    glVertex3f(1, -1, 0.0f);
                    glTexCoord2f(texcoordwidth, texcoordheight);
                    glVertex3f(1, 1, 0.0f);
                    glTexCoord2f(0, texcoordheight);
                    glVertex3f(-1, 1, 0.0f);
                    glEnd();
                    glPopMatrix();
                }
            }
            if (drawmode == glowmode) {
                glColor4f(.5, .5, .5, .5);
                glEnable(GL_BLEND);
                glBlendFunc(GL_SRC_ALPHA, GL_ONE);
                glPushMatrix();
                glTranslatef(.01, 0, 0);
                glBegin(GL_QUADS);
                glTexCoord2f(0, 0);
                glVertex3f(-1, -1, 0.0f);
                glTexCoord2f(texcoordwidth, 0);
                glVertex3f(1, -1, 0.0f);
                glTexCoord2f(texcoordwidth, texcoordheight);
                glVertex3f(1, 1, 0.0f);
                glTexCoord2f(0, texcoordheight);
                glVertex3f(-1, 1, 0.0f);
                glEnd();
                glPopMatrix();
                glPushMatrix();
                glTranslatef(-.01, 0, 0);
                glBegin(GL_QUADS);
                glTexCoord2f(0, 0);
                glVertex3f(-1, -1, 0.0f);
                glTexCoord2f(texcoordwidth, 0);
                glVertex3f(1, -1, 0.0f);
                glTexCoord2f(texcoordwidth, texcoordheight);
                glVertex3f(1, 1, 0.0f);
                glTexCoord2f(0, texcoordheight);
                glVertex3f(-1, 1, 0.0f);
                glEnd();
                glPopMatrix();
                glPushMatrix();
                glTranslatef(.0, .01, 0);
                glBegin(GL_QUADS);
                glTexCoord2f(0, 0);
                glVertex3f(-1, -1, 0.0f);
                glTexCoord2f(texcoordwidth, 0);
                glVertex3f(1, -1, 0.0f);
                glTexCoord2f(texcoordwidth, texcoordheight);
                glVertex3f(1, 1, 0.0f);
                glTexCoord2f(0, texcoordheight);
                glVertex3f(-1, 1, 0.0f);
                glEnd();
                glPopMatrix();
                glPushMatrix();
                glTranslatef(0, -.01, 0);
                glBegin(GL_QUADS);
                glTexCoord2f(0, 0);
                glVertex3f(-1, -1, 0.0f);
                glTexCoord2f(texcoordwidth, 0);
                glVertex3f(1, -1, 0.0f);
                glTexCoord2f(texcoordwidth, texcoordheight);
                glVertex3f(1, 1, 0.0f);
                glTexCoord2f(0, texcoordheight);
                glVertex3f(-1, 1, 0.0f);
                glEnd();
                glPopMatrix();
            }
            if (drawmode == radialzoommode) {
                for (int i = 0; i < 3; i++) {
                    glColor4f(1, 1, 1, 1 / ((float)i + 1));
                    glPushMatrix();
                    glScalef(1 + (float)i * .01, 1 + (float)i * .01, 1);
                    glBegin(GL_QUADS);
                    glTexCoord2f(0, 0);
                    glVertex3f(-1, -1, 0.0f);
                    glTexCoord2f(texcoordwidth, 0);
                    glVertex3f(1, -1, 0.0f);
                    glTexCoord2f(texcoordwidth, texcoordheight);
                    glVertex3f(1, 1, 0.0f);
                    glTexCoord2f(0, texcoordheight);
                    glVertex3f(-1, 1, 0.0f);
                    glEnd();
                    glPopMatrix();
                }
            }
            glDisable(GL_TEXTURE_2D);
            glMatrixMode(GL_PROJECTION);
            glPopMatrix();
            glMatrixMode(GL_MODELVIEW);
            glPopMatrix();
            glEnable(GL_DEPTH_TEST);
            glEnable(GL_CULL_FACE);
            glDisable(GL_BLEND);
            glDepthMask(1);
        }

        if (console) {
            glEnable(GL_TEXTURE_2D);
            glColor4f(1, 1, 1, 1);
            int offset = 0;
            if (consoleselected >= 60) {
                offset = consoleselected - 60;
            }
            textmono->glPrint(10, 30, " ]", 0, 1, 1024, 768);
            if (consoleblink) {
                textmono->glPrint(30 + (float)consoleselected * 10 - offset * 10, 30, "_", 0, 1, 1024, 768);
            }
            for (unsigned i = 0; i < 15; i++) {
                textmono->glPrint(30 - offset * 10, 30 + i * 20, consoletext[i], 0, 1, 1024, 768);
            }
        }
    }

    if (freeze || winfreeze || (mainmenu && gameon) || (!gameon && gamestarted)) {
        multiplier = tempmult;
    }

    if (mainmenu) {
        DrawMenu();
    }

    if (freeze || winfreeze || (mainmenu && gameon) || (!gameon && gamestarted)) {
        tempmult = multiplier;
        multiplier = 0;
    }

    if (side == stereoRight || side == stereoCenter) {
        if (drawmode != motionblurmode || mainmenu) {
            swap_gl_buffers();
        }
    }

    glDrawBuffer(GL_BACK);
    glReadBuffer(GL_BACK);

    weapons.DoStuff();

    if (drawtoggle == 2) {
        drawtoggle = 0;
    }

    if (freeze || winfreeze || (mainmenu && gameon) || (!gameon && gamestarted)) {
        multiplier = tempmult;
    }
    //Jordan fixed your warning!
    return 0;
}

void DrawMenu()
{
    // !!! FIXME: hack: clamp framerate in menu so text input works correctly on fast systems.
    SDL_Delay(15);

    glDrawBuffer(GL_BACK);
    glReadBuffer(GL_BACK);
    glTexEnvi(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE);
    Game::ReSizeGLScene(90, .1f);

    //draw menu background
    glClear(GL_DEPTH_BUFFER_BIT);
    glEnable(GL_ALPHA_TEST);
    glAlphaFunc(GL_GREATER, 0.001f);
    glEnable(GL_TEXTURE_2D);
    glDisable(GL_DEPTH_TEST);
    glDisable(GL_CULL_FACE);
    glDisable(GL_LIGHTING);
    glDepthMask(0);
    glMatrixMode(GL_PROJECTION);
    glPushMatrix();
    glLoadIdentity();
    glOrtho(0, screenwidth, 0, screenheight, -100, 100);
    glMatrixMode(GL_MODELVIEW);
    glPushMatrix();
    glLoadIdentity();
    glTranslatef(screenwidth / 2, screenheight / 2, 0);
    glPushMatrix();
    glScalef((float)screenwidth / 2, (float)screenheight / 2, 1);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    glDisable(GL_BLEND);
    glColor4f(0, 0, 0, 1.0);
    glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP);
    glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP);
    glDisable(GL_TEXTURE_2D);
    glBegin(GL_QUADS);
    glVertex3f(-1, -1, 0);
    glVertex3f(+1, -1, 0);
    glVertex3f(+1, +1, 0);
    glVertex3f(-1, +1, 0);
    glEnd();
    glEnable(GL_BLEND);
    glColor4f(0.4, 0.4, 0.4, 1.0);
    glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP);
    glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP);
    glEnable(GL_TEXTURE_2D);
    Game::Mainmenuitems[4].bind();
    glBegin(GL_QUADS);
    glTexCoord2f(0, 0);
    glVertex3f(-1, -1, 0);
    glTexCoord2f(1, 0);
    glVertex3f(+1, -1, 0);
    glTexCoord2f(1, 1);
    glVertex3f(+1, +1, 0);
    glTexCoord2f(0, 1);
    glVertex3f(-1, +1, 0);
    glEnd();
    glPopMatrix();
    glPopMatrix();
    glMatrixMode(GL_PROJECTION);
    glPopMatrix();
    glMatrixMode(GL_MODELVIEW);

    glMatrixMode(GL_PROJECTION);
    glPushMatrix();
    glLoadIdentity();
    glOrtho(0, 640, 0, 480, -100, 100);
    glMatrixMode(GL_MODELVIEW);
    glPushMatrix();
    glLoadIdentity();
    glEnable(GL_TEXTURE_2D);

    Menu::drawItems();

    //draw mouse cursor
    glMatrixMode(GL_PROJECTION);
    glPopMatrix();
    glMatrixMode(GL_MODELVIEW);
    glPopMatrix();

    glMatrixMode(GL_PROJECTION);
    glPushMatrix();
    glLoadIdentity();
    glOrtho(0, screenwidth, 0, screenheight, -100, 100);
    glMatrixMode(GL_MODELVIEW);
    glPushMatrix();
    glLoadIdentity();
    glTranslatef(screenwidth / 2, screenheight / 2, 0);
    glPushMatrix();
    glScalef((float)screenwidth / 2, (float)screenheight / 2, 1);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    glEnable(GL_BLEND);
    glEnable(GL_TEXTURE_2D);
    glColor4f(1, 1, 1, 1);
    glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP);
    glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP);
    glPopMatrix();
    if (!Game::waiting) { // hide the cursor while waiting for a key
        glPushMatrix();
        glTranslatef(Game::mousecoordh - screenwidth / 2, Game::mousecoordv * -1 + screenheight / 2, 0);
        glScalef((float)screenwidth / 64, (float)screenwidth / 64, 1);
        glTranslatef(1, -1, 0);
        glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
        glColor4f(1, 1, 1, 1);
        Game::cursortexture.bind();
        glPushMatrix();
        glBegin(GL_QUADS);
        glTexCoord2f(0, 0);
        glVertex3f(-1, -1, 0.0f);
        glTexCoord2f(1, 0);
        glVertex3f(1, -1, 0.0f);
        glTexCoord2f(1, 1);
        glVertex3f(1, 1, 0.0f);
        glTexCoord2f(0, 1);
        glVertex3f(-1, 1, 0.0f);
        glEnd();
        glPopMatrix();
        glPopMatrix();
    }
    glPopMatrix();
    glMatrixMode(GL_PROJECTION);
    glPopMatrix();

    //draw screen flash
    if (flashamount > 0) {
        if (flashamount > 1) {
            flashamount = 1;
        }
        if (flashdelay <= 0) {
            flashamount -= multiplier;
        }
        flashdelay--;
        if (flashamount < 0) {
            flashamount = 0;
        }
        glDisable(GL_DEPTH_TEST);
        glDisable(GL_CULL_FACE);
        glDisable(GL_LIGHTING);
        glDisable(GL_TEXTURE_2D);
        glDepthMask(0);
        glMatrixMode(GL_PROJECTION);
        glPushMatrix();
        glLoadIdentity();
        glOrtho(0, screenwidth, 0, screenheight, -100, 100);
        glMatrixMode(GL_MODELVIEW);
        glPushMatrix();
        glLoadIdentity();
        glScalef(screenwidth, screenheight, 1);
        glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
        glEnable(GL_BLEND);
        glColor4f(flashr, flashg, flashb, flashamount);
        glBegin(GL_QUADS);
        glVertex3f(0, 0, 0.0f);
        glVertex3f(256, 0, 0.0f);
        glVertex3f(256, 256, 0.0f);
        glVertex3f(0, 256, 0.0f);
        glEnd();
        glMatrixMode(GL_PROJECTION);
        glPopMatrix();
        glMatrixMode(GL_MODELVIEW);
        glPopMatrix();
        glEnable(GL_DEPTH_TEST);
        glEnable(GL_CULL_FACE);
        glDisable(GL_BLEND);
        glDepthMask(1);
    }
}
