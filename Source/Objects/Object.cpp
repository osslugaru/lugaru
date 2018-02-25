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

#include "Objects/Object.hpp"

extern XYZ viewer;
extern float viewdistance;
extern float fadestart;
extern int environment;
extern float texscale;
extern Light light;
extern float multiplier;
extern float gravity;
extern FRUSTUM frustum;
extern Terrain terrain;
extern bool foliage;
extern int detail;
extern float blurness;
extern float windvar;
extern float playerdist;
extern bool skyboxtexture;

std::vector<std::unique_ptr<Object>> Object::objects;
XYZ Object::center;
float Object::radius = 0;
Texture Object::boxtextureptr;
Texture Object::treetextureptr;
Texture Object::bushtextureptr;
Texture Object::rocktextureptr;

//Functions

Object::Object()
    : position()
    , type(boxtype)
    , yaw(0)
    , pitch(0)
    , rotx(0)
    , rotxvel(0)
    , roty(0)
    , rotyvel(0)
    , possible(false)
    , model()
    , displaymodel()
    , friction(0)
    , scale(0)
    , messedwith(0)
    , checked(0)
    , shadowed(0)
    , occluded(0)
    , onfire(false)
    , flamedelay(0)
{
}

Object::Object(object_type _type, XYZ _position, float _yaw, float _pitch, float _scale)
    : Object()
{
    scale = _scale;
    type = _type;
    position = _position;
    yaw = _yaw;
    pitch = _pitch;

    switch (type) {
        case boxtype:
            model.loaddecal("Models/Box.solid");
            friction = 1.5;
            break;
        case cooltype:
            model.loaddecal("Models/Cool.solid");
            friction = 1.5;
            break;
        case walltype:
            model.loaddecal("Models/Wall.solid");
            friction = 1.5;
            break;
        case tunneltype:
            model.loaddecal("Models/Tunnel.solid");
            friction = 1.5;
            break;
        case chimneytype:
            model.loaddecal("Models/Chimney.solid");
            friction = 1.5;
            break;
        case spiketype:
            model.load("Models/Spike.solid");
            friction = .4;
            break;
        case weirdtype:
            model.loaddecal("Models/Weird.solid");
            friction = 1.5;
            break;
        case rocktype:
            model.loaddecal("Models/Rock.solid");
            if (scale > .5) {
                friction = 1.5;
            } else {
                friction = .5;
            }
            break;
        case treetrunktype:
            model.load("Models/TreeTrunk.solid");
            friction = .4;
            break;
        case treeleavestype:
            scale += fabs((float)(Random() % 100) / 900) * scale;
            model.load("Models/Leaves.solid");
            friction = 0;
            break;
        case bushtype:
            position.y = terrain.getHeight(position.x, position.z) - .3;
            model.load("Models/Bush.solid");
            break;
        case platformtype:
            model.loaddecal("Models/Platform.solid");
            model.Rotate(90, 0, 0);
            friction = 1.5;
            break;
        case firetype:
            onfire = true;
            break;
    }

    if (friction == 1.5 && fabs(pitch) > 5) {
        friction = .5;
    }

    if (type == boxtype || type == cooltype || type == spiketype || type == weirdtype || type == walltype || type == chimneytype || type == tunneltype || type == platformtype) {
        model.ScaleTexCoords(scale * 1.5);
    }
    if (type == rocktype) {
        model.ScaleTexCoords(scale * 3);
    }
    model.flat = 1;
    if (type == treetrunktype || type == treeleavestype || type == rocktype) {
        model.flat = 0;
    }
    model.Scale(.3 * scale, .3 * scale, .3 * scale);
    model.Rotate(90, 1, 1);
    model.Rotate(pitch, 0, 0);
    if (type == rocktype) {
        model.Rotate(yaw * 5, 0, 0);
    }
    model.CalculateNormals(1);
    model.ScaleNormals(-1, -1, -1);
}

void Object::handleFire()
{
    if (type == firetype) {
        onfire = 1;
    }
    if (onfire) {
        XYZ spawnpoint;
        if ((type == bushtype) || (type == firetype)) {
            flamedelay -= multiplier * 3;
        } else if (type == treeleavestype) {
            flamedelay -= multiplier * 4;
        }
        while ((flamedelay < 0) && onfire) {
            flamedelay += .006;
            if ((type == bushtype) || (type == firetype)) {
                spawnpoint.x = ((float)(Random() % 100)) / 30 * scale;
                spawnpoint.y = ((float)(Random() % 100) + 60) / 30 * scale;
                spawnpoint.z = 0;
                spawnpoint = DoRotation(spawnpoint, 0, Random() % 360, 0);
                spawnpoint += position;
                Sprite::MakeSprite(flamesprite, spawnpoint, spawnpoint * 0, 1, 1, 1, (.6 + (float)abs(Random() % 100) / 200 - .25) * 5 * scale, 1);
            } else if (type == treeleavestype) {
                spawnpoint.x = ((float)(Random() % 100)) / 80 * scale;
                spawnpoint.y = ((float)(Random() % 100) + 80) / 12 * scale;
                spawnpoint.z = 0;
                spawnpoint = DoRotation(spawnpoint, 0, Random() % 360, 0);
                spawnpoint += position;
                Sprite::MakeSprite(flamesprite, spawnpoint, spawnpoint * 0, 1, 1, 1, (.6 + (float)abs(Random() % 100) / 200 - .25) * 6, 1);
            }
        }
    }
}

void Object::doShadows(XYZ lightloc)
{
    XYZ testpoint, testpoint2, terrainpoint, col;
    int patchx, patchz;
    if (type != treeleavestype && type != treetrunktype && type != bushtype && type != firetype) {
        for (int j = 0; j < model.vertexNum; j++) {
            terrainpoint = position + DoRotation(model.vertex[j] + model.normals[j] * .1, 0, yaw, 0);
            shadowed = 0;
            patchx = terrainpoint.x / (terrain.size / subdivision * terrain.scale);
            patchz = terrainpoint.z / (terrain.size / subdivision * terrain.scale);
            if (patchx >= 0 && patchz >= 0 && patchx < subdivision && patchz < subdivision) {
                for (unsigned int k = 0; k < terrain.patchobjects[patchx][patchz].size(); k++) {
                    unsigned int l = terrain.patchobjects[patchx][patchz][k];
                    if (objects[l]->type != treetrunktype) {
                        testpoint = terrainpoint;
                        testpoint2 = terrainpoint + lightloc * 50 * (1 - shadowed);
                        if (objects[l]->model.LineCheck(&testpoint, &testpoint2, &col, &objects[l]->position, &objects[l]->yaw) != -1) {
                            shadowed = 1 - (findDistance(&terrainpoint, &col) / 50);
                        }
                    }
                }
            }
            if (shadowed > 0) {
                col = model.normals[j] - DoRotation(lightloc * shadowed, 0, -yaw, 0);
                Normalise(&col);
                for (unsigned int k = 0; k < model.Triangles.size(); k++) {
                    if (model.Triangles[k].vertex[0] == j) {
                        int l = k * 24;
                        model.vArray[l + 2] = col.x;
                        model.vArray[l + 3] = col.y;
                        model.vArray[l + 4] = col.z;
                    }
                    if (model.Triangles[k].vertex[1] == j) {
                        int l = k * 24;
                        model.vArray[l + 10] = col.x;
                        model.vArray[l + 11] = col.y;
                        model.vArray[l + 12] = col.z;
                    }
                    if (model.Triangles[k].vertex[2] == j) {
                        int l = k * 24;
                        model.vArray[l + 18] = col.x;
                        model.vArray[l + 19] = col.y;
                        model.vArray[l + 20] = col.z;
                    }
                }
            }
        }
    }
    shadowed = 0;
}

void Object::handleRot(int divide)
{
    messedwith -= multiplier;
    if (rotxvel || rotx) {
        if (rotx > 0) {
            rotxvel -= multiplier * 8 * fabs(rotx);
        }
        if (rotx < 0) {
            rotxvel += multiplier * 8 * fabs(rotx);
        }
        if (rotx > 0) {
            rotxvel -= multiplier * 4;
        }
        if (rotx < 0) {
            rotxvel += multiplier * 4;
        }
        if (rotxvel > 0) {
            rotxvel -= multiplier * 4;
        }
        if (rotxvel < 0) {
            rotxvel += multiplier * 4;
        }
        if (fabs(rotx) < multiplier * 4) {
            rotx = 0;
        }
        if (fabs(rotxvel) < multiplier * 4) {
            rotxvel = 0;
        }

        rotx += rotxvel * multiplier * 4;
    }
    if (rotyvel || roty) {
        if (roty > 0) {
            rotyvel -= multiplier * 8 * fabs(roty);
        }
        if (roty < 0) {
            rotyvel += multiplier * 8 * fabs(roty);
        }
        if (roty > 0) {
            rotyvel -= multiplier * 4;
        }
        if (roty < 0) {
            rotyvel += multiplier * 4;
        }
        if (rotyvel > 0) {
            rotyvel -= multiplier * 4;
        }
        if (rotyvel < 0) {
            rotyvel += multiplier * 4;
        }
        if (fabs(roty) < multiplier * 4) {
            roty = 0;
        }
        if (fabs(rotyvel) < multiplier * 4) {
            rotyvel = 0;
        }

        roty += rotyvel * multiplier * 4;
    }
    if (roty) {
        glRotatef(roty / divide, 1, 0, 0);
    }
    if (rotx) {
        glRotatef(-rotx / divide, 0, 0, 1);
    }
    if (rotx > 10) {
        rotx = 10;
    }
    if (rotx < -10) {
        rotx = -10;
    }
    if (roty > 10) {
        roty = 10;
    }
    if (roty < -10) {
        roty = -10;
    }
}

void Object::draw()
{
    static float distance;
    static XYZ moved, terrainlight;
    bool hidden;
    if (type == firetype) {
        return;
    }
    moved = DoRotation(model.boundingspherecenter, 0, yaw, 0);
    if (type == tunneltype || frustum.SphereInFrustum(position.x + moved.x, position.y + moved.y, position.z + moved.z, model.boundingsphereradius)) {
        distance = distsq(&viewer, &position);
        distance *= 1.2;
        hidden = !(distsqflat(&viewer, &position) > playerdist + 3 || (type != bushtype && type != treeleavestype));
        if (!hidden) {

            if (detail == 2 && distance > viewdistance * viewdistance / 4 && environment == desertenvironment) {
                glTexEnvf(GL_TEXTURE_FILTER_CONTROL_EXT, GL_TEXTURE_LOD_BIAS_EXT, blurness);
            } else {
                glTexEnvf(GL_TEXTURE_FILTER_CONTROL_EXT, GL_TEXTURE_LOD_BIAS_EXT, 0);
            }
            distance = (viewdistance * viewdistance - (distance - (viewdistance * viewdistance * fadestart)) * (1 / (1 - fadestart))) / viewdistance / viewdistance;
            if (distance > 1) {
                distance = 1;
            }
            if (distance > 0) {

                if (occluded < 6) {
                    glMatrixMode(GL_MODELVIEW);
                    glPushMatrix();
                    if (!model.color) {
                        glEnable(GL_LIGHTING);
                    } else {
                        glDisable(GL_LIGHTING);
                    }
                    glDepthMask(1);
                    glTranslatef(position.x, position.y, position.z);
                    if (type == bushtype) {
                        handleRot(1);
                    }
                    if (type == treetrunktype || type == treeleavestype) {
                        if (type == treetrunktype || environment == desertenvironment) {
                            handleRot(6);
                        } else {
                            handleRot(4);
                        }
                    }
                    if (environment == snowyenvironment) {
                        if (type == treeleavestype) {
                            glRotatef((sin(windvar + position.x * .3) + .5) * 1.5 * (sin(windvar * 2 + position.x * .3) + 1) / 2, 1, 0, 0);
                        }
                        if (type == treetrunktype) {
                            glRotatef((sin(windvar + position.x * .3) + .5) * .5 * (sin(windvar * 2 + position.x * .3) + 1) / 2, 1, 0, 0);
                        }
                        if (type == bushtype) {
                            glRotatef((sin(windvar + position.x * .3) + .5) * 4 * (sin(windvar * 2 + position.x * .3) + 1) / 2, 1, 0, 0);
                        }
                    }
                    if (environment == grassyenvironment) {
                        if (type == treeleavestype) {
                            glRotatef((sin(windvar + position.x * .3) + .5) * 1.5 * .5 * (sin(windvar * 2 + position.x * .3) + 1) / 2, 1, 0, 0);
                        }
                        if (type == treetrunktype) {
                            glRotatef((sin(windvar + position.x * .3) + .5) * .5 * .5 * (sin(windvar * 2 + position.x * .3) + 1) / 2, 1, 0, 0);
                        }
                        if (type == bushtype) {
                            glRotatef((sin(windvar + position.x * .3) + .5) * 4 * .5 * (sin(windvar * 2 + position.x * .3) + 1) / 2, 1, 0, 0);
                        }
                    }
                    if (environment == desertenvironment) {
                        if (type == bushtype) {
                            glRotatef((sin(windvar + position.x * .3) + .5) * 4 * .5 * (sin(windvar * 2 + position.x * .3) + 1) / 2, 1, 0, 0);
                        }
                    }
                    glRotatef(yaw, 0, 1, 0);
                    glColor4f((1 - shadowed) / 2 + .5, (1 - shadowed) / 2 + .5, (1 - shadowed) / 2 + .5, distance);
                    if (distance >= 1) {
                        glDisable(GL_BLEND);
                        glAlphaFunc(GL_GREATER, 0.5);
                    } else {
                        glEnable(GL_BLEND);
                        glAlphaFunc(GL_GREATER, 0.1);
                    }
                    if (type != treetrunktype && type != treeleavestype && type != bushtype && type != rocktype) {
                        glEnable(GL_CULL_FACE);
                        glAlphaFunc(GL_GREATER, 0.0001);
                        model.drawdifftex(boxtextureptr);
                        model.drawdecals(terrain.shadowtexture, terrain.bloodtexture, terrain.bloodtexture2, terrain.breaktexture);
                    }
                    if (type == rocktype) {
                        glEnable(GL_CULL_FACE);
                        glAlphaFunc(GL_GREATER, 0.0001);
                        glColor4f((1 - shadowed) / 2 + light.ambient[0], (1 - shadowed) / 2 + light.ambient[1], (1 - shadowed) / 2 + light.ambient[2], distance);
                        model.drawdifftex(rocktextureptr);
                        model.drawdecals(terrain.shadowtexture, terrain.bloodtexture, terrain.bloodtexture2, terrain.breaktexture);
                    }
                    if (type == treeleavestype) {
                        glDisable(GL_CULL_FACE);
                        glDisable(GL_LIGHTING);
                        terrainlight = terrain.getLighting(position.x, position.z);
                        glColor4f(terrainlight.x, terrainlight.y, terrainlight.z, distance);
                        if (distance < 1) {
                            glAlphaFunc(GL_GREATER, 0.2);
                        }
                        model.drawdifftex(treetextureptr);
                    }
                    if (type == bushtype) {
                        glDisable(GL_CULL_FACE);
                        glDisable(GL_LIGHTING);
                        terrainlight = terrain.getLighting(position.x, position.z);
                        glColor4f(terrainlight.x, terrainlight.y, terrainlight.z, distance);
                        if (distance < 1) {
                            glAlphaFunc(GL_GREATER, 0.2);
                        }
                        model.drawdifftex(bushtextureptr);
                    }
                    if (type == treetrunktype) {
                        glEnable(GL_CULL_FACE);
                        terrainlight = terrain.getLighting(position.x, position.z);
                        glColor4f(terrainlight.x, terrainlight.y, terrainlight.z, distance);
                        model.drawdifftex(treetextureptr);
                    }
                    glPopMatrix();
                }
            }
        }
    }
}

void Object::drawSecondPass()
{
    static float distance;
    static XYZ moved, terrainlight;
    bool hidden;
    if (type != treeleavestype && type != bushtype) {
        return;
    }
    moved = DoRotation(model.boundingspherecenter, 0, yaw, 0);
    if (frustum.SphereInFrustum(position.x + moved.x, position.y + moved.y, position.z + moved.z, model.boundingsphereradius)) {
        hidden = distsqflat(&viewer, &position) <= playerdist + 3;
        if (hidden) {
            distance = 1;
            glMatrixMode(GL_MODELVIEW);
            glPushMatrix();
            glEnable(GL_LIGHTING);
            glDepthMask(1);
            glTranslatef(position.x, position.y, position.z);
            if (type == bushtype) {
                handleRot(1);
            }
            if (type == treetrunktype || type == treeleavestype) {
                handleRot(2);
            }
            if (environment == snowyenvironment) {
                if (type == treeleavestype) {
                    glRotatef((sin(windvar + position.x * .3) + .5) * 1.5 * (sin(windvar * 2 + position.x * .3) + 1) / 2, 1, 0, 0);
                }
                if (type == treetrunktype) {
                    glRotatef((sin(windvar + position.x * .3) + .5) * .5 * (sin(windvar * 2 + position.x * .3) + 1) / 2, 1, 0, 0);
                }
                if (type == bushtype) {
                    glRotatef((sin(windvar + position.x * .3) + .5) * 4 * (sin(windvar * 2 + position.x * .3) + 1) / 2, 1, 0, 0);
                }
            }
            if (environment == grassyenvironment) {
                if (type == treeleavestype) {
                    glRotatef((sin(windvar + position.x * .3) + .5) * 1.5 * .5 * (sin(windvar * 2 + position.x * .3) + 1) / 2, 1, 0, 0);
                }
                if (type == treetrunktype) {
                    glRotatef((sin(windvar + position.x * .3) + .5) * .5 * .5 * (sin(windvar * 2 + position.x * .3) + 1) / 2, 1, 0, 0);
                }
                if (type == bushtype) {
                    glRotatef((sin(windvar + position.x * .3) + .5) * 4 * .5 * (sin(windvar * 2 + position.x * .3) + 1) / 2, 1, 0, 0);
                }
            }
            glRotatef(yaw, 0, 1, 0);
            glColor4f(1, 1, 1, distance);
            if (type == treeleavestype) {
                glDisable(GL_CULL_FACE);
                glDisable(GL_LIGHTING);
                terrainlight = terrain.getLighting(position.x, position.z);
                glDepthMask(0);
                glEnable(GL_BLEND);
                glColor4f(terrainlight.x, terrainlight.y, terrainlight.z, .3);
                glAlphaFunc(GL_GREATER, 0);
                glDisable(GL_ALPHA_TEST);
                model.drawdifftex(treetextureptr);
            }
            if (type == bushtype) {
                glDisable(GL_CULL_FACE);
                glDisable(GL_LIGHTING);
                terrainlight = terrain.getLighting(position.x, position.z);
                glDepthMask(0);
                glEnable(GL_BLEND);
                glColor4f(terrainlight.x, terrainlight.y, terrainlight.z, .3);
                glAlphaFunc(GL_GREATER, 0);
                glDisable(GL_ALPHA_TEST);
                model.drawdifftex(bushtextureptr);
            }
            glPopMatrix();
        }
    }
}

void Object::ComputeCenter()
{
    center = 0;
    for (unsigned i = 0; i < objects.size(); i++) {
        center += objects[i]->position;
    }
    center /= objects.size();
}

void Object::ComputeRadius()
{
    float maxdistance = 0;
    float tempdist;
    for (unsigned int i = 0; i < objects.size(); i++) {
        tempdist = distsq(&center, &objects[i]->position);
        if (tempdist > maxdistance) {
            maxdistance = tempdist;
        }
    }
    radius = fast_sqrt(maxdistance);
}

void Object::LoadObjectsFromFile(FILE* tfile, bool skip)
{
    int numobjects;
    int type;
    XYZ position;
    float yaw, pitch, scale;
    float lastscale = 1.0f;
    funpackf(tfile, "Bi", &numobjects);
    if (!skip) {
        objects.clear();
    }
    for (int i = 0; i < numobjects; i++) {
        funpackf(tfile, "Bi Bf Bf Bf Bf Bf Bf", &type, &yaw, &pitch, &position.x, &position.y, &position.z, &scale);
        if (!skip) {
            if (type == treeleavestype) {
                scale = lastscale;
            }
            objects.emplace_back(new Object(object_type(type), position, yaw, pitch, scale));
            lastscale = scale;
        }
    }
}

void Object::LoadObjectsFromJson(Json::Value values)
{
    objects.clear();
    float lastscale = 1.0f;
    for (unsigned i = 0; i < values.size(); i++) {
        objects.emplace_back(new Object(values[i], lastscale));
        lastscale = objects.back()->scale;
    }
}

void Object::addToTerrain(unsigned id)
{
    if ((type != treeleavestype) && (type != bushtype) && (type != firetype)) {
        terrain.AddObject(position + DoRotation(model.boundingspherecenter, 0, yaw, 0), model.boundingsphereradius, id);
    }

    if (detail == 2) {
        if ((type == treetrunktype) && (position.y < (terrain.getHeight(position.x, position.z) + 1))) {
            terrain.MakeDecal(shadowdecalpermanent, position, 2, .4, 0);
        }

        if ((type == bushtype) && (position.y < (terrain.getHeight(position.x, position.z) + 1))) {
            terrain.MakeDecal(shadowdecalpermanent, position, 1, .4, 0);
        }
    }
}

void Object::AddObjectsToTerrain()
{
    for (unsigned i = 0; i < objects.size(); i++) {
        objects[i]->addToTerrain(i);
    }
}

void Object::SphereCheckPossible(XYZ* p1, float radius)
{
    int whichpatchx = p1->x / (terrain.size / subdivision * terrain.scale);
    int whichpatchz = p1->z / (terrain.size / subdivision * terrain.scale);

    if (whichpatchx >= 0 && whichpatchz >= 0 && whichpatchx < subdivision && whichpatchz < subdivision) {
        if (terrain.patchobjects[whichpatchx][whichpatchz].size() < 500) {
            for (unsigned int j = 0; j < terrain.patchobjects[whichpatchx][whichpatchz].size(); j++) {
                unsigned int i = terrain.patchobjects[whichpatchx][whichpatchz][j];
                objects[i]->possible = false;
                if (objects[i]->model.SphereCheckPossible(p1, radius, &objects[i]->position, &objects[i]->yaw) != -1) {
                    objects[i]->possible = true;
                }
            }
        }
    }
}

void Object::Draw()
{
    for (unsigned i = 0; i < objects.size(); i++) {
        objects[i]->draw();
    }

    glTexEnvf(GL_TEXTURE_FILTER_CONTROL_EXT, GL_TEXTURE_LOD_BIAS_EXT, 0);
    for (unsigned i = 0; i < objects.size(); i++) {
        objects[i]->drawSecondPass();
    }
    if (environment == desertenvironment) {
        glTexEnvf(GL_TEXTURE_FILTER_CONTROL_EXT, GL_TEXTURE_LOD_BIAS_EXT, 0);
    }
    glEnable(GL_ALPHA_TEST);
    SetUpLight(&light, 0);
}

void Object::DeleteObject(int which)
{
    objects.erase(objects.begin() + which);
    terrain.DeleteObject(which);
}

void Object::MakeObject(int atype, XYZ where, float ayaw, float apitch, float ascale)
{
    if ((atype != treeleavestype && atype != bushtype) || foliage == 1) {
        unsigned nextid = objects.size();
        objects.emplace_back(new Object(object_type(atype), where, ayaw, apitch, ascale));
        objects.back()->addToTerrain(nextid);
    }
}

void Object::DoStuff()
{
    for (unsigned i = 0; i < objects.size(); i++) {
        objects[i]->handleFire();
    }
}

void Object::DoShadows()
{
    XYZ lightloc;
    lightloc = light.location;
    if (!skyboxtexture) {
        lightloc = 0;
    }
    lightloc.y += 10;
    Normalise(&lightloc);

    for (unsigned i = 0; i < objects.size(); i++) {
        objects[i]->doShadows(lightloc);
    }
}

int Object::checkcollide(XYZ startpoint, XYZ endpoint)
{
    float minx, minz, maxx, maxz, miny, maxy;

    minx = min(startpoint.x, endpoint.x) - 1;
    miny = min(startpoint.y, endpoint.y) - 1;
    minz = min(startpoint.z, endpoint.z) - 1;
    maxx = max(startpoint.x, endpoint.x) + 1;
    maxy = max(startpoint.y, endpoint.y) + 1;
    maxz = max(startpoint.z, endpoint.z) + 1;

    for (unsigned int i = 0; i < objects.size(); i++) {
        if (checkcollide(startpoint, endpoint, i, minx, miny, minz, maxx, maxy, maxz) != -1) {
            return (int)i;
        }
    }

    return -1;
}

int Object::checkcollide(XYZ startpoint, XYZ endpoint, int what)
{
    float minx, minz, maxx, maxz, miny, maxy;

    minx = min(startpoint.x, endpoint.x) - 1;
    miny = min(startpoint.y, endpoint.y) - 1;
    minz = min(startpoint.z, endpoint.z) - 1;
    maxx = max(startpoint.x, endpoint.x) + 1;
    maxy = max(startpoint.y, endpoint.y) + 1;
    maxz = max(startpoint.z, endpoint.z) + 1;

    return checkcollide(startpoint, endpoint, what, minx, miny, minz, maxx, maxy, maxz);
}

int Object::checkcollide(XYZ startpoint, XYZ endpoint, int what, float minx, float miny, float minz, float maxx, float maxy, float maxz)
{
    XYZ colpoint, colviewer, coltarget;

    if (what == 1000) {
        if (terrain.lineTerrain(startpoint, endpoint, &colpoint) != -1) {
            return what;
        }
    } else {
        if (objects[what]->position.x > minx - objects[what]->model.boundingsphereradius &&
            objects[what]->position.x < maxx + objects[what]->model.boundingsphereradius &&
            objects[what]->position.y > miny - objects[what]->model.boundingsphereradius &&
            objects[what]->position.y < maxy + objects[what]->model.boundingsphereradius &&
            objects[what]->position.z > minz - objects[what]->model.boundingsphereradius &&
            objects[what]->position.z < maxz + objects[what]->model.boundingsphereradius) {
            if (objects[what]->type != treeleavestype &&
                objects[what]->type != bushtype &&
                objects[what]->type != firetype) {
                colviewer = startpoint;
                coltarget = endpoint;
                if (objects[what]->model.LineCheck(&colviewer, &coltarget, &colpoint, &objects[what]->position, &objects[what]->yaw) != -1) {
                    return what;
                }
            }
        }
    }

    return -1;
}

Object::Object(Json::Value value, float lastscale)
    : Object(object_type(value[0].asInt()), value[4], value[1].asFloat(), value[2].asFloat(), ((value[0].asInt() == treeleavestype) ? lastscale : value[3].asFloat()))
{
}

Object::operator Json::Value() {
    Json::Value object;

    object[0] = type;
    object[1] = yaw;
    object[2] = pitch;
    if (type == treeleavestype) {
        /* Store 1.0 as scale is ignored anyway for tree leaves.
         * This avoids having random floats in the output to make sure the same map gives the same json each time. */
        object[3] = 1.0f;
    } else {
        object[3] = scale;
    }
    object[4] = position;

    return object;
}

//~ Object::~Objects()
//~ {
//~ boxtextureptr.destroy();
//~ treetextureptr.destroy();
//~ bushtextureptr.destroy();
//~ rocktextureptr.destroy();
//~ }
