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

#include "Graphic/Sprite.hpp"

#include "Game.hpp"
#include "Objects/Person.hpp"

extern XYZ viewer;
extern float viewdistance;
extern float fadestart;
extern int environment;
extern float texscale;
extern Light light;
extern float multiplier;
extern float gravity;
extern Terrain terrain;
extern int detail;
extern XYZ viewerfacing;
extern int bloodtoggle;
extern XYZ windvector;

// init statics
Texture Sprite::cloudtexture;
Texture Sprite::cloudimpacttexture;
Texture Sprite::bloodtexture;
Texture Sprite::flametexture;
Texture Sprite::bloodflametexture;
Texture Sprite::smoketexture;
Texture Sprite::snowflaketexture;
Texture Sprite::shinetexture;
Texture Sprite::splintertexture;
Texture Sprite::leaftexture;
Texture Sprite::toothtexture;

float Sprite::checkdelay = 0;

vector<std::unique_ptr<Sprite> > Sprite::sprites = vector<std::unique_ptr<Sprite> >();

//Functions
void Sprite::Draw()
{
    int k;
    static float M[16];
    static XYZ point;
    static float distancemult;
    static int lasttype;
    static int lastspecial;
    static int whichpatchx, whichpatchz;
    static XYZ start, end, colpoint;
    static bool check;
    static bool blend;
    static float tempmult;
    static XYZ difference;
    static float lightcolor[3];
    static float viewdistsquared = viewdistance * viewdistance;
    static XYZ tempviewer;

    tempviewer = viewer + viewerfacing * 6;
    check = 0;

    lightcolor[0] = light.color[0] * .5 + light.ambient[0];
    lightcolor[1] = light.color[1] * .5 + light.ambient[1];
    lightcolor[2] = light.color[2] * .5 + light.ambient[2];

    checkdelay -= multiplier * 10;

    if (checkdelay <= 0) {
        check = 1;
        checkdelay = 1;
    }

    lasttype = -1;
    lastspecial = -1;
    glEnable(GL_BLEND);
    glDisable(GL_LIGHTING);
    glDisable(GL_CULL_FACE);
    glEnable(GL_TEXTURE_2D);
    blend = 1;
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    glDepthMask(0);
    glAlphaFunc(GL_GREATER, 0.0001);
    for (unsigned i = 0; i < sprites.size(); i++) {
        if (lasttype != sprites[i]->type) {
            switch (sprites[i]->type) {
                case cloudsprite:
                    cloudtexture.bind();
                    if (!blend) {
                        blend = 1;
                        glAlphaFunc(GL_GREATER, 0.0001);
                        glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
                    }
                    break;
                case breathsprite:
                case cloudimpactsprite:
                    cloudimpacttexture.bind();
                    if (!blend) {
                        blend = 1;
                        glAlphaFunc(GL_GREATER, 0.0001);
                        glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
                    }
                    break;
                case smoketype:
                    smoketexture.bind();
                    if (!blend) {
                        blend = 1;
                        glAlphaFunc(GL_GREATER, 0.0001);
                        glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
                    }
                    break;
                case bloodsprite:
                    bloodtexture.bind();
                    if (!blend) {
                        blend = 1;
                        glAlphaFunc(GL_GREATER, 0.0001);
                        glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
                    }
                    break;
                case splintersprite:
                    if (lastspecial != sprites[i]->special) {
                        if (sprites[i]->special == 0) {
                            splintertexture.bind();
                        }
                        if (sprites[i]->special == 1) {
                            leaftexture.bind();
                        }
                        if (sprites[i]->special == 2) {
                            snowflaketexture.bind();
                        }
                        if (sprites[i]->special == 3) {
                            toothtexture.bind();
                        }
                        if (!blend) {
                            blend = 1;
                            glAlphaFunc(GL_GREATER, 0.0001);
                            glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
                        }
                    }
                    break;
                case snowsprite:
                    snowflaketexture.bind();
                    if (!blend) {
                        blend = 1;
                        glAlphaFunc(GL_GREATER, 0.0001);
                        glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
                    }
                    break;
                case weaponshinesprite:
                    shinetexture.bind();
                    if (blend) {
                        blend = 0;
                        glAlphaFunc(GL_GREATER, 0.001);
                        glBlendFunc(GL_SRC_ALPHA, GL_ONE);
                    }
                    break;
                case flamesprite:
                case weaponflamesprite:
                    flametexture.bind();
                    if (blend || lasttype == bloodflamesprite) {
                        blend = 0;
                        glAlphaFunc(GL_GREATER, 0.3);
                        glBlendFunc(GL_SRC_ALPHA, GL_ONE);
                    }
                    break;
                case bloodflamesprite:
                    bloodflametexture.bind();
                    if (blend) {
                        blend = 0;
                        glAlphaFunc(GL_GREATER, 0.3);
                        glBlendFunc(GL_ONE, GL_ZERO);
                    }
                    break;
            }
        }
        if (sprites[i]->type == snowsprite) {
            distancemult = (144 - (distsq(&tempviewer, &sprites[i]->position) - (144 * fadestart)) * (1 / (1 - fadestart))) / 144;
        } else {
            distancemult = (viewdistsquared - (distsq(&viewer, &sprites[i]->position) - (viewdistsquared * fadestart)) * (1 / (1 - fadestart))) / viewdistsquared;
        }
        if (sprites[i]->type == flamesprite) {
            if (distancemult >= 1) {
                glColor4f(sprites[i]->color[0], sprites[i]->color[1], sprites[i]->color[2], sprites[i]->opacity);
            } else {
                glColor4f(sprites[i]->color[0], sprites[i]->color[1], sprites[i]->color[2], sprites[i]->opacity * distancemult);
            }
        } else {
            if (distancemult >= 1) {
                glColor4f(sprites[i]->color[0] * lightcolor[0], sprites[i]->color[1] * lightcolor[1], sprites[i]->color[2] * lightcolor[2], sprites[i]->opacity);
            } else {
                glColor4f(sprites[i]->color[0] * lightcolor[0], sprites[i]->color[1] * lightcolor[1], sprites[i]->color[2] * lightcolor[2], sprites[i]->opacity * distancemult);
            }
        }
        lasttype = sprites[i]->type;
        lastspecial = sprites[i]->special;
        glMatrixMode(GL_MODELVIEW);
        glPushMatrix();
        glTranslatef(sprites[i]->position.x, sprites[i]->position.y, sprites[i]->position.z);
        if ((sprites[i]->type == flamesprite || sprites[i]->type == weaponflamesprite || sprites[i]->type == weaponshinesprite)) {
            difference = viewer - sprites[i]->position;
            Normalise(&difference);
            glTranslatef(difference.x * sprites[i]->size / 4, difference.y * sprites[i]->size / 4, difference.z * sprites[i]->size / 4);
        }
        if (sprites[i]->type == snowsprite) {
            glRotatef(sprites[i]->rotation * .2, 0, .3, 1);
            glTranslatef(1, 0, 0);
        }
        glGetFloatv(GL_MODELVIEW_MATRIX, M);
        point.x = M[12];
        point.y = M[13];
        point.z = M[14];
        glLoadIdentity();
        glTranslatef(point.x, point.y, point.z);

        glRotatef(sprites[i]->rotation, 0, 0, 1);

        if ((sprites[i]->type == flamesprite || sprites[i]->type == weaponflamesprite || sprites[i]->type == weaponshinesprite || sprites[i]->type == bloodflamesprite)) {
            if (sprites[i]->alivetime < .14) {
                glScalef(sprites[i]->alivetime / .14, sprites[i]->alivetime / .14, sprites[i]->alivetime / .14);
            }
        }
        if (sprites[i]->type == smoketype || sprites[i]->type == snowsprite || sprites[i]->type == weaponshinesprite || sprites[i]->type == breathsprite) {
            if (sprites[i]->alivetime < .3) {
                if (distancemult >= 1) {
                    glColor4f(sprites[i]->color[0] * lightcolor[0], sprites[i]->color[1] * lightcolor[1], sprites[i]->color[2] * lightcolor[2], sprites[i]->opacity * sprites[i]->alivetime / .3);
                }
                if (distancemult < 1) {
                    glColor4f(sprites[i]->color[0] * lightcolor[0], sprites[i]->color[1] * lightcolor[1], sprites[i]->color[2] * lightcolor[2], sprites[i]->opacity * distancemult * sprites[i]->alivetime / .3);
                }
            }
        }
        if (sprites[i]->type == splintersprite && sprites[i]->special > 0 && sprites[i]->special != 3) {
            if (sprites[i]->alivetime < .2) {
                if (distancemult >= 1) {
                    glColor4f(sprites[i]->color[0] * lightcolor[0], sprites[i]->color[1] * lightcolor[1], sprites[i]->color[2] * lightcolor[2], sprites[i]->alivetime / .2);
                } else {
                    glColor4f(sprites[i]->color[0] * lightcolor[0], sprites[i]->color[1] * lightcolor[1], sprites[i]->color[2] * lightcolor[2], distancemult * sprites[i]->alivetime / .2);
                }
            } else {
                if (distancemult >= 1) {
                    glColor4f(sprites[i]->color[0] * lightcolor[0], sprites[i]->color[1] * lightcolor[1], sprites[i]->color[2] * lightcolor[2], 1);
                } else {
                    glColor4f(sprites[i]->color[0] * lightcolor[0], sprites[i]->color[1] * lightcolor[1], sprites[i]->color[2] * lightcolor[2], distancemult);
                }
            }
        }
        if (sprites[i]->type == splintersprite && (sprites[i]->special == 0 || sprites[i]->special == 3)) {
            if (distancemult >= 1) {
                glColor4f(sprites[i]->color[0] * lightcolor[0], sprites[i]->color[1] * lightcolor[1], sprites[i]->color[2] * lightcolor[2], 1);
            } else {
                glColor4f(sprites[i]->color[0] * lightcolor[0], sprites[i]->color[1] * lightcolor[1], sprites[i]->color[2] * lightcolor[2], distancemult);
            }
        }

        glBegin(GL_TRIANGLES);
        glTexCoord2f(1.0f, 1.0f);
        glVertex3f(.5 * sprites[i]->size, .5 * sprites[i]->size, 0.0f);
        glTexCoord2f(0.0f, 1.0f);
        glVertex3f(-.5 * sprites[i]->size, .5 * sprites[i]->size, 0.0f);
        glTexCoord2f(1.0f, 0.0f);
        glVertex3f(.5 * sprites[i]->size, -.5 * sprites[i]->size, 0.0f);
        glTexCoord2f(0.0f, 0.0f);
        glVertex3f(-.5 * sprites[i]->size, -.5 * sprites[i]->size, 0.0f);
        glTexCoord2f(1.0f, 0.0f);
        glVertex3f(.5 * sprites[i]->size, -.5 * sprites[i]->size, 0.0f);
        glTexCoord2f(0.0f, 1.0f);
        glVertex3f(-.5 * sprites[i]->size, .5 * sprites[i]->size, 0.0f);
        glEnd();
        glPopMatrix();
    }
    tempmult = multiplier;
    for (int i = sprites.size() - 1; i >= 0; i--) {
        multiplier = tempmult;
        if (sprites[i]->type != snowsprite) {
            sprites[i]->position += sprites[i]->velocity * multiplier;
            sprites[i]->velocity += windvector * multiplier;
        }
        if (sprites[i]->type == flamesprite || sprites[i]->type == smoketype) {
            sprites[i]->position += windvector * multiplier / 2;
        }
        if ((sprites[i]->type == flamesprite || sprites[i]->type == weaponflamesprite || sprites[i]->type == weaponshinesprite || sprites[i]->type == bloodflamesprite)) {
            multiplier *= sprites[i]->speed * .7;
        }
        sprites[i]->alivetime += multiplier;

        if (sprites[i]->type == cloudsprite || sprites[i]->type == cloudimpactsprite) {
            sprites[i]->opacity -= multiplier / 2;
            sprites[i]->size += multiplier / 2;
            sprites[i]->velocity.y += gravity * multiplier * .25;
        }
        if (sprites[i]->type == breathsprite) {
            sprites[i]->opacity -= multiplier / 2;
            sprites[i]->size += multiplier / 2;
            if (findLength(&sprites[i]->velocity) <= multiplier) {
                sprites[i]->velocity = 0;
            } else {
                XYZ slowdown;
                slowdown = sprites[i]->velocity * -1;
                Normalise(&slowdown);
                slowdown *= multiplier;
                sprites[i]->velocity += slowdown;
            }
        }
        if (sprites[i]->type == snowsprite) {
            sprites[i]->size -= multiplier / 120;
            sprites[i]->rotation += multiplier * 360;
            sprites[i]->position.y -= multiplier;
            sprites[i]->position += windvector * multiplier;
            if (sprites[i]->position.y < tempviewer.y - 6) {
                sprites[i]->position.y += 12;
            }
            if (sprites[i]->position.y > tempviewer.y + 6) {
                sprites[i]->position.y -= 12;
            }
            if (sprites[i]->position.z < tempviewer.z - 6) {
                sprites[i]->position.z += 12;
            }
            if (sprites[i]->position.z > tempviewer.z + 6) {
                sprites[i]->position.z -= 12;
            }
            if (sprites[i]->position.x < tempviewer.x - 6) {
                sprites[i]->position.x += 12;
            }
            if (sprites[i]->position.x > tempviewer.x + 6) {
                sprites[i]->position.x -= 12;
            }
        }
        if (sprites[i]->type == bloodsprite) {
            bool spritehit = 0;
            sprites[i]->rotation += multiplier * 100;
            sprites[i]->velocity.y += gravity * multiplier;
            if (check) {
                XYZ where, startpoint, endpoint, movepoint, footpoint;
                float rotationpoint;
                int whichtri;

                for (unsigned j = 0; j < Person::players.size(); j++) {
                    if (!spritehit && Person::players[j]->dead && sprites[i]->alivetime > .1) {
                        where = sprites[i]->oldposition;
                        where -= Person::players[j]->coords;
                        if (!Person::players[j]->skeleton.free) {
                            where = DoRotation(where, 0, -Person::players[j]->yaw, 0);
                        }
                        startpoint = where;
                        where = sprites[i]->position;
                        where -= Person::players[j]->coords;
                        if (!Person::players[j]->skeleton.free) {
                            where = DoRotation(where, 0, -Person::players[j]->yaw, 0);
                        }
                        endpoint = where;

                        movepoint = 0;
                        rotationpoint = 0;
                        whichtri = Person::players[j]->skeleton.drawmodel.LineCheck(&startpoint, &endpoint, &footpoint, &movepoint, &rotationpoint);
                        if (whichtri != -1) {
                            spritehit = 1;
                            Person::players[j]->DoBloodBigWhere(0, 160, sprites[i]->oldposition);
                            DeleteSprite(i);
                        }
                    }
                }

                whichpatchx = sprites[i]->position.x / (terrain.size / subdivision * terrain.scale);
                whichpatchz = sprites[i]->position.z / (terrain.size / subdivision * terrain.scale);
                if (whichpatchx > 0 && whichpatchz > 0 && whichpatchx < subdivision && whichpatchz < subdivision) {
                    if (!spritehit) {
                        for (unsigned int j = 0; j < terrain.patchobjects[whichpatchx][whichpatchz].size(); j++) {
                            k = terrain.patchobjects[whichpatchx][whichpatchz][j];
                            start = sprites[i]->oldposition;
                            end = sprites[i]->position;
                            if (!spritehit) {
                                if (Object::objects[k]->model.LineCheck(&start, &end, &colpoint, &Object::objects[k]->position, &Object::objects[k]->yaw) != -1) {
                                    if (detail == 2 || (detail == 1 && abs(Random() % 4) == 0) || (detail == 0 && abs(Random() % 8) == 0)) {
                                        Object::objects[k]->model.MakeDecal(blooddecalfast, DoRotation(colpoint - Object::objects[k]->position, 0, -Object::objects[k]->yaw, 0), sprites[i]->size * 1.6, .5, Random() % 360);
                                    }
                                    DeleteSprite(i);
                                    spritehit = 1;
                                }
                            }
                        }
                    }
                }
                if (!spritehit) {
                    if (sprites[i]->position.y < terrain.getHeight(sprites[i]->position.x, sprites[i]->position.z)) {
                        terrain.MakeDecal(blooddecalfast, sprites[i]->position, sprites[i]->size * 1.6, .6, Random() % 360);
                        DeleteSprite(i);
                    }
                }
            }
        }
        if (sprites[i]->type == splintersprite) {
            sprites[i]->rotation += sprites[i]->rotatespeed * multiplier;
            sprites[i]->opacity -= multiplier / 2;
            if (sprites[i]->special == 0 || sprites[i]->special == 2 || sprites[i]->special == 3) {
                sprites[i]->velocity.y += gravity * multiplier;
            }
            if (sprites[i]->special == 1) {
                sprites[i]->velocity.y += gravity * multiplier * .5;
            }
        }
        if (sprites[i]->type == flamesprite || sprites[i]->type == weaponflamesprite || sprites[i]->type == weaponshinesprite || sprites[i]->type == bloodflamesprite) {
            sprites[i]->rotation += multiplier * sprites[i]->rotatespeed;
            sprites[i]->opacity -= multiplier * 5 / 4;
            if (sprites[i]->type != weaponshinesprite && sprites[i]->type != bloodflamesprite) {
                if (sprites[i]->opacity < .5 && sprites[i]->opacity + multiplier * 5 / 4 >= .5 && (abs(Random() % 4) == 0 || (sprites[i]->initialsize > 2 && Random() % 2 == 0))) {
                    MakeSprite(smoketype, sprites[i]->position, sprites[i]->velocity, .9, .9, .6, sprites[i]->size * 1.2, .4);
                }
            }
            if (sprites[i]->alivetime > .14 && (sprites[i]->type == flamesprite)) {
                sprites[i]->velocity = 0;
                sprites[i]->velocity.y = 1.5;
            }
        }
        if (sprites[i]->type == smoketype) {
            sprites[i]->opacity -= multiplier / 3 / sprites[i]->initialsize;
            sprites[i]->color[0] -= multiplier;
            sprites[i]->color[1] -= multiplier;
            sprites[i]->color[2] -= multiplier;
            if (sprites[i]->color[0] < .6) {
                sprites[i]->color[0] = .6;
            }
            if (sprites[i]->color[1] < .6) {
                sprites[i]->color[1] = .6;
            }
            if (sprites[i]->color[2] < .6) {
                sprites[i]->color[2] = .6;
            }
            sprites[i]->size += multiplier;
            sprites[i]->velocity = 0;
            sprites[i]->velocity.y = 1.5;
            sprites[i]->rotation += multiplier * sprites[i]->rotatespeed / 5;
        }
        if (sprites[i]->opacity <= 0 || sprites[i]->size <= 0) {
            DeleteSprite(i);
        }
    }
    if (check) {
        for (int i = sprites.size() - 1; i >= 0; i--) {
            sprites[i]->oldposition = sprites[i]->position;
        }
    }
    glAlphaFunc(GL_GREATER, 0.0001);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
}

void Sprite::DeleteSprite(int i)
{
    sprites.erase(sprites.begin() + i);
}

void Sprite::MakeSprite(int atype, XYZ where, XYZ avelocity, float red, float green, float blue, float asize, float aopacity)
{
    if (sprites.size() < max_sprites - 1) {
        sprites.push_back(std::unique_ptr<Sprite>(new Sprite()));
        if ((atype != bloodsprite && atype != bloodflamesprite) || bloodtoggle) {
            sprites.back()->special = 0;
            sprites.back()->type = atype;
            sprites.back()->position = where;
            sprites.back()->oldposition = where;
            sprites.back()->velocity = avelocity;
            sprites.back()->alivetime = 0;
            sprites.back()->opacity = aopacity;
            sprites.back()->size = asize;
            sprites.back()->initialsize = asize;
            sprites.back()->color[0] = red;
            sprites.back()->color[1] = green;
            sprites.back()->color[2] = blue;
            sprites.back()->rotatespeed = abs(Random() % 720) - 360;
            sprites.back()->speed = float(abs(Random() % 100)) / 200 + 1.5;
        }
    }
}

Sprite::Sprite()
{
    oldposition = 0;
    position = 0;
    velocity = 0;
    size = 0;
    initialsize = 0;
    type = 0;
    special = 0;
    memset(color, 0, sizeof(color));
    opacity = 0;
    rotation = 0;
    alivetime = 0;
    speed = 0;
    rotatespeed = 0;
}
