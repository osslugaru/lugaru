/*
Copyright (C) 2003, 2010 - Wolfire Games
Copyright (C) 2010-2017 - Lugaru contributors (see AUTHORS file)

This file is part of Lugaru.

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

#include "Objects/Weapons.hpp"

#include "Animation/Animation.hpp"
#include "Audio/Sounds.hpp"
#include "Audio/openal_wrapper.hpp"
#include "Game.hpp"
#include "Level/Awards.hpp"
#include "Tutorial.hpp"

extern float multiplier;
extern Terrain terrain;
extern float gravity;
extern int environment;
extern int detail;
extern FRUSTUM frustum;
extern XYZ viewer;
extern float realmultiplier;
extern int slomo;
extern float slomodelay;
extern bool cellophane;
extern float texdetail;
extern int bloodtoggle;
extern bool autoslomo;
extern float camerashake;
extern float woozy;
extern float viewdistance;
extern float blackout;
extern bool freeze;
extern int numthrowkill;

Model Weapon::throwingknifemodel;
Texture Weapon::knifetextureptr;
Texture Weapon::lightbloodknifetextureptr;
Texture Weapon::bloodknifetextureptr;

Model Weapon::swordmodel;
Texture Weapon::swordtextureptr;
Texture Weapon::lightbloodswordtextureptr;
Texture Weapon::bloodswordtextureptr;

Model Weapon::staffmodel;
Texture Weapon::stafftextureptr;

Weapon::Weapon(int t, int o)
    : owner(o)
{
    setType(t);
    bloody = 0;
    blooddrip = 0;
    blooddripdelay = 0;
    onfire = 0;
    flamedelay = 0;
    damage = 0;
    position = -1000;
    tippoint = -1000;
}

void Weapon::setType(int t)
{
    type = t;
    if (type == sword) {
        mass = 1.5;
        tipmass = 1;
        length = .8;
    }
    if (type == staff) {
        mass = 2;
        tipmass = 1;
        length = 1.5;
    }
    if (type == knife) {
        mass = 1;
        tipmass = 1.2;
        length = .25;
    }
}

/* Load weapons models and textures */
void Weapon::Load()
{
    LOG("Loading weapon data...");

    knifetextureptr.load("Textures/Knife.png", 0);
    bloodknifetextureptr.load("Textures/BloodKnife.png", 0);
    lightbloodknifetextureptr.load("Textures/BloodKnifeLight.png", 0);
    swordtextureptr.load("Textures/Sword.jpg", 1);
    bloodswordtextureptr.load("Textures/SwordBlood.jpg", 1);
    lightbloodswordtextureptr.load("Textures/SwordBloodLight.jpg", 1);
    stafftextureptr.load("Textures/Staff.jpg", 1);

    throwingknifemodel.load("Models/ThrowingKnife.solid");
    throwingknifemodel.Scale(.001, .001, .001);
    throwingknifemodel.Rotate(90, 0, 0);
    throwingknifemodel.Rotate(0, 90, 0);
    throwingknifemodel.flat = 0;
    throwingknifemodel.CalculateNormals(1);

    swordmodel.load("Models/Sword.solid");
    swordmodel.Scale(.001, .001, .001);
    swordmodel.Rotate(90, 0, 0);
    swordmodel.Rotate(0, 90, 0);
    swordmodel.Rotate(0, 0, 90);
    swordmodel.flat = 1;
    swordmodel.CalculateNormals(1);

    staffmodel.load("Models/Staff.solid");
    staffmodel.Scale(.005, .005, .005);
    staffmodel.Rotate(90, 0, 0);
    staffmodel.Rotate(0, 90, 0);
    staffmodel.Rotate(0, 0, 90);
    staffmodel.flat = 1;
    staffmodel.CalculateNormals(1);
}

void Weapon::doStuff(int i)
{
    static int whichpatchx, whichpatchz, whichhit;
    static XYZ start, end, colpoint, normalrot, footvel, footpoint;
    static XYZ terrainnormal;
    static XYZ vel;
    static XYZ midp;
    static XYZ newpoint1, newpoint2;
    static float friction = 3.5;
    static float elasticity = .4;
    static XYZ bounceness;
    static float frictionness;
    static float closestdistance;
    static float distance;
    static XYZ point[3];
    static XYZ closestpoint;
    static XYZ closestswordpoint;
    static float tempmult;

    if (multiplier <= 0) {
        return;
    }

    if (owner != -1) {
        oldowner = owner;
    }
    if (damage >= 2 && type == staff && owner != -1) { // the staff breaks
        emit_sound_at(staffbreaksound, tippoint);
        XYZ tempvel;
        for (int j = 0; j < 40; j++) {
            tempvel.x = float(abs(Random() % 100) - 50) / 20;
            tempvel.y = float(abs(Random() % 100) - 50) / 20;
            tempvel.z = float(abs(Random() % 100) - 50) / 20;
            Sprite::MakeSprite(splintersprite, position + (tippoint - position) * ((float)j - 8) / 32, tempvel * .5, 115 / 255, 73 / 255, 12 / 255, .1, 1);
        }
        if (owner != -1) {
            Person::players[owner]->weaponactive = -1;
            Person::players[owner]->num_weapons--;
            if (Person::players[owner]->num_weapons) {
                Person::players[owner]->weaponids[0] = Person::players[owner]->weaponids[Person::players[owner]->num_weapons];
                if (Person::players[owner]->weaponstuck == Person::players[owner]->num_weapons) {
                    Person::players[owner]->weaponstuck = 0;
                }
            }
        }
        owner = -1;
        hitsomething = 0;
        missed = 1;
        freetime = 0;
        firstfree = 1;
        position = 0;
        physics = 0;
    }
    oldposition = position;
    oldtippoint = tippoint;
    if (owner == -1 && (velocity.x || velocity.y || velocity.z) && !physics) { // if the weapon is flying
        position += velocity * multiplier;
        tippoint += velocity * multiplier;
        whichpatchx = position.x / (terrain.size / subdivision * terrain.scale);
        whichpatchz = position.z / (terrain.size / subdivision * terrain.scale);
        if (whichpatchx > 0 && whichpatchz > 0 && whichpatchx < subdivision && whichpatchz < subdivision) {
            for (unsigned int j = 0; j < terrain.patchobjects[whichpatchx][whichpatchz].size(); j++) { // check for collision
                unsigned int k = terrain.patchobjects[whichpatchx][whichpatchz][j];
                start = oldtippoint;
                end = tippoint;
                whichhit = Object::objects[k]->model.LineCheck(&start, &end, &colpoint, &Object::objects[k]->position, &Object::objects[k]->yaw);
                if (whichhit != -1) {
                    if (Object::objects[k]->type == treetrunktype) {
                        Object::objects[k]->model.MakeDecal(breakdecal, DoRotation(colpoint - Object::objects[k]->position, 0, -Object::objects[k]->yaw, 0), .1, 1, Random() % 360);
                        normalrot = DoRotation(Object::objects[k]->model.Triangles[whichhit].facenormal, 0, Object::objects[k]->yaw, 0);
                        velocity = 0;
                        if (type == knife) {
                            position = colpoint - normalrot * .1;
                        } else if (type == sword) {
                            position = colpoint - normalrot * .2;
                        } else if (type == staff) {
                            position = colpoint - normalrot * .2;
                        }
                        XYZ temppoint1, temppoint2;
                        float distance;

                        temppoint1 = 0;
                        temppoint2 = normalrot;
                        distance = findDistance(&temppoint1, &temppoint2);
                        rotation2 = asin((temppoint1.y - temppoint2.y) / distance);
                        rotation2 *= 360 / 6.28;
                        temppoint1.y = 0;
                        temppoint2.y = 0;
                        rotation1 = acos((temppoint1.z - temppoint2.z) / findDistance(&temppoint1, &temppoint2));
                        rotation1 *= 360 / 6.28;
                        if (temppoint1.x > temppoint2.x) {
                            rotation1 = 360 - rotation1;
                        }

                        rotation3 = 0;
                        smallrotation = 90;
                        smallrotation2 = 0;
                        bigtilt = 0;
                        bigtilt2 = 0;
                        bigrotation = 0;

                        emit_sound_at(knifesheathesound, position, 128.);

                        bloody = 0;

                        Sprite::MakeSprite(cloudimpactsprite, position, velocity, 1, 1, 1, .8, .3);
                    } else {
                        physics = 1;
                        firstfree = 1;
                        position -= velocity * multiplier;
                        tippoint -= velocity * multiplier;
                        tipvelocity = velocity;
                    }
                }
            }
        }

        if (velocity.x || velocity.y || velocity.z) {
            for (unsigned j = 0; j < Person::players.size(); j++) {
                footvel = 0;
                footpoint = DoRotation((Person::players[j]->jointPos(abdomen) + Person::players[j]->jointPos(neck)) / 2, 0, Person::players[j]->yaw, 0) * Person::players[j]->scale + Person::players[j]->coords;
                if (owner == -1 && distsqflat(&position, &Person::players[j]->coords) < 1.5 &&
                    distsq(&position, &Person::players[j]->coords) < 4 && Person::players[j]->weaponstuck == -1 &&
                    !Person::players[j]->skeleton.free && (int(j) != oldowner)) {
                    if ((Person::players[j]->aitype != attacktypecutoff || abs(Random() % 6) == 0 || (Person::players[j]->animTarget != backhandspringanim && Person::players[j]->animTarget != rollanim && Person::players[j]->animTarget != flipanim && Random() % 2 == 0)) && !missed) {
                        if (Person::players[j]->catchKnife()) {
                            emit_sound_at(knifedrawsound, Person::players[j]->coords, 128.);

                            Person::players[j]->animTarget = removeknifeanim;
                            Person::players[j]->frameTarget = 1;
                            Person::players[j]->target = 1;
                            Person::players[j]->takeWeapon(i);

                            Person::players[j]->aitype = attacktypecutoff;
                        } else {
                            if (j != 0) {
                                numthrowkill++;
                            }
                            Person::players[j]->num_weapons++;
                            Person::players[j]->weaponstuck = Person::players[j]->num_weapons - 1;
                            if (normaldotproduct(Person::players[j]->facing, velocity) > 0) {
                                Person::players[j]->weaponstuckwhere = 1;
                            } else {
                                Person::players[j]->weaponstuckwhere = 0;
                            }

                            Person::players[j]->weaponids[Person::players[j]->num_weapons - 1] = i;

                            Person::players[j]->RagDoll(0);
                            Person::players[j]->jointVel(abdomen) += velocity * 2;
                            Person::players[j]->jointVel(neck) += velocity * 2;
                            Person::players[j]->jointVel(rightshoulder) += velocity * 2;
                            Person::players[j]->jointVel(leftshoulder) += velocity * 2;
                            if (bloodtoggle && !Tutorial::active) {
                                Sprite::MakeSprite(cloudimpactsprite, footpoint, footvel, 1, 0, 0, .8, .3);
                            }
                            if (Tutorial::active) {
                                Sprite::MakeSprite(cloudimpactsprite, footpoint, footvel, 1, 1, 1, .8, .3);
                            }
                            footvel = tippoint - position;
                            Normalise(&footvel);
                            if (bloodtoggle && !Tutorial::active) {
                                Sprite::MakeSprite(bloodflamesprite, footpoint, footvel * -1, 1, 0, 0, .6, 1);
                            }

                            if (!Tutorial::active) {
                                if (Person::players[j]->weaponstuckwhere == 0) {
                                    Person::players[j]->DoBloodBig(2, 205);
                                }
                                if (Person::players[j]->weaponstuckwhere == 1) {
                                    Person::players[j]->DoBloodBig(2, 200);
                                }
                                Person::players[j]->damage += 200 / Person::players[j]->armorhigh;
                                Person::players[j]->deathbleeding = 1;
                                Person::players[j]->bloodloss += (200 + abs((float)(Random() % 40)) - 20) / Person::players[j]->armorhigh;
                                owner = j;
                                bloody = 2;
                                blooddrip = 5;
                            }

                            emit_sound_at(fleshstabsound, position, 128.);

                            if (Animation::animations[Person::players[0]->animTarget].height == highheight) {
                                award_bonus(0, ninja);
                            } else {
                                award_bonus(0, Bullseyebonus);
                            }
                        }
                    } else {
                        missed = 1;
                    }
                }
            }
        }
        if (position.y < terrain.getHeight(position.x, position.z)) {
            if (terrain.getOpacity(position.x, position.z) < .2) {
                velocity = 0;
                if (terrain.lineTerrain(oldposition, position, &colpoint) != -1) {
                    position = colpoint * terrain.scale;
                } else {
                    position.y = terrain.getHeight(position.x, position.z);
                }

                terrain.MakeDecal(shadowdecalpermanent, position, .06, .5, 0);
                normalrot = terrain.getNormal(position.x, position.z) * -1;
                velocity = 0;
                glMatrixMode(GL_MODELVIEW);
                glPushMatrix();
                GLfloat M[16];
                glLoadIdentity();
                glRotatef(bigrotation, 0, 1, 0);
                glRotatef(bigtilt2, 1, 0, 0);
                glRotatef(bigtilt, 0, 0, 1);
                glRotatef(-rotation1 + 90, 0, 1, 0);
                glRotatef(-rotation2 + 90, 0, 0, 1);
                glRotatef(-rotation3, 0, 1, 0);
                glRotatef(smallrotation, 1, 0, 0);
                glRotatef(smallrotation2, 0, 1, 0);
                glTranslatef(0, 0, 1);
                glGetFloatv(GL_MODELVIEW_MATRIX, M);
                tippoint.x = M[12];
                tippoint.y = M[13];
                tippoint.z = M[14];
                glPopMatrix();
                position -= tippoint * .15;
                XYZ temppoint1, temppoint2;

                rotation3 = 0;
                smallrotation = 90;
                smallrotation2 = 0;
                bigtilt = 0;
                bigtilt2 = 0;
                bigrotation = 0;

                emit_sound_at(knifesheathesound, position, 128.);

                XYZ terrainlight;
                terrainlight = terrain.getLighting(position.x, position.z);
                if (environment == snowyenvironment) {
                    if (distsq(&position, &viewer) < viewdistance * viewdistance / 4) {
                        Sprite::MakeSprite(cloudsprite, position, velocity, terrainlight.x, terrainlight.y, terrainlight.z, .5, .7);
                    }
                } else if (environment == grassyenvironment) {
                    if (distsq(&position, &viewer) < viewdistance * viewdistance / 4) {
                        Sprite::MakeSprite(cloudsprite, position, velocity, terrainlight.x * 90 / 255, terrainlight.y * 70 / 255, terrainlight.z * 8 / 255, .5, .5);
                    }
                } else if (environment == desertenvironment) {
                    if (distsq(&position, &viewer) < viewdistance * viewdistance / 4) {
                        Sprite::MakeSprite(cloudsprite, position, velocity, terrainlight.x * 190 / 255, terrainlight.y * 170 / 255, terrainlight.z * 108 / 255, .5, .7);
                    }
                }

                bloody = 0;
            } else {
                physics = 1;
                firstfree = 1;
                position -= velocity * multiplier;
                tippoint -= velocity * multiplier;
                tipvelocity = velocity;
            }
        }
        if (velocity.x != 0 || velocity.z != 0 || velocity.y != 0) {
            velocity.y += gravity * multiplier;

            XYZ temppoint1, temppoint2;
            float distance;

            temppoint1 = 0;
            temppoint2 = velocity;
            distance = findDistance(&temppoint1, &temppoint2);
            rotation2 = asin((temppoint1.y - temppoint2.y) / distance);
            rotation2 *= 360 / 6.28;
            temppoint1.y = 0;
            temppoint2.y = 0;
            rotation1 = acos((temppoint1.z - temppoint2.z) / findDistance(&temppoint1, &temppoint2));
            rotation1 *= 360 / 6.28;
            rotation3 = 0;
            smallrotation = 90;
            smallrotation2 = 0;
            bigtilt = 0;
            bigtilt2 = 0;
            bigrotation = 0;
            if (temppoint1.x > temppoint2.x) {
                rotation1 = 360 - rotation1;
            }
        }
    }

    //Sword physics
    XYZ mid;
    XYZ oldmid;
    XYZ oldmid2;

    tempmult = multiplier;
    multiplier /= 10;
    for (int l = 0; l < 10; l++) {
        if (owner == -1 && (velocity.x || velocity.y || velocity.z) && physics) {
            //move
            position += velocity * multiplier;
            tippoint += tipvelocity * multiplier;

            //Length constrain
            midp = (position * mass + tippoint * tipmass) / (mass + tipmass);
            vel = tippoint - midp;
            Normalise(&vel);
            newpoint1 = midp - vel * length * (tipmass / (mass + tipmass));
            newpoint2 = midp + vel * length * (mass / (mass + tipmass));
            if (!freeze) {
                if (freetime > .04) {
                    velocity = velocity + (newpoint1 - position) / multiplier;
                    tipvelocity = tipvelocity + (newpoint2 - tippoint) / multiplier;
                }
            }
            position = newpoint1;
            tippoint = newpoint2;

            //Object collisions
            whichpatchx = (position.x) / (terrain.size / subdivision * terrain.scale);
            whichpatchz = (position.z) / (terrain.size / subdivision * terrain.scale);
            if (whichpatchx > 0 && whichpatchz > 0 && whichpatchx < subdivision && whichpatchz < subdivision) {
                for (unsigned int j = 0; j < terrain.patchobjects[whichpatchx][whichpatchz].size(); j++) {
                    unsigned int k = terrain.patchobjects[whichpatchx][whichpatchz][j];

                    if (firstfree) {
                        if (type == staff) {
                            start = tippoint - (position - tippoint) / 5;
                            end = position + (position - tippoint) / 30;
                            whichhit = Object::objects[k]->model.LineCheck(&start, &end, &colpoint, &Object::objects[k]->position, &Object::objects[k]->yaw);
                            if (whichhit != -1) {
                                XYZ diff;
                                diff = (colpoint - position);
                                Normalise(&diff);
                                hitsomething = 1;

                                tippoint += (colpoint - position) + diff * .05;
                                position = colpoint + diff * .05;
                                oldtippoint = tippoint;
                                oldposition = tippoint;
                            }
                        } else {
                            start = position - (tippoint - position) / 5;
                            end = tippoint + (tippoint - position) / 30;
                            whichhit = Object::objects[k]->model.LineCheck(&start, &end, &colpoint, &Object::objects[k]->position, &Object::objects[k]->yaw);
                            if (whichhit != -1) {
                                XYZ diff;
                                diff = (colpoint - tippoint);
                                Normalise(&diff);
                                hitsomething = 1;

                                position += (colpoint - tippoint) + diff * .05;
                                tippoint = colpoint + diff * .05;
                                oldposition = position;
                                oldtippoint = tippoint;
                            }
                        }
                    }

                    start = oldposition;
                    end = position;
                    whichhit = Object::objects[k]->model.LineCheck(&start, &end, &colpoint, &Object::objects[k]->position, &Object::objects[k]->yaw);
                    if (whichhit != -1) {
                        hitsomething = 1;
                        position = colpoint;
                        terrainnormal = DoRotation(Object::objects[k]->model.Triangles[whichhit].facenormal, 0, Object::objects[k]->yaw, 0) * -1;
                        ReflectVector(&velocity, &terrainnormal);
                        position += terrainnormal * .002;

                        bounceness = terrainnormal * findLength(&velocity) * (abs(normaldotproduct(velocity, terrainnormal)));
                        if (findLengthfast(&velocity) < findLengthfast(&bounceness)) {
                            bounceness = 0;
                        }
                        frictionness = abs(normaldotproduct(velocity, terrainnormal));
                        velocity -= bounceness;
                        if (1 - friction * frictionness > 0) {
                            velocity *= 1 - friction * frictionness;
                        } else {
                            velocity = 0;
                        }
                        velocity += bounceness * elasticity;

                        if (findLengthfast(&bounceness) > 1) {
                            int whichsound;
                            if (type == staff) {
                                whichsound = footstepsound3 + abs(Random() % 2);
                            } else {
                                whichsound = clank1sound + abs(Random() % 4);
                            }
                            emit_sound_at(whichsound, position, 128 * findLengthfast(&bounceness));
                        }
                    }
                    start = oldtippoint;
                    end = tippoint;
                    whichhit = Object::objects[k]->model.LineCheck(&start, &end, &colpoint, &Object::objects[k]->position, &Object::objects[k]->yaw);
                    if (whichhit != -1) {
                        hitsomething = 1;
                        tippoint = colpoint;
                        terrainnormal = DoRotation(Object::objects[k]->model.Triangles[whichhit].facenormal, 0, Object::objects[k]->yaw, 0) * -1;
                        ReflectVector(&tipvelocity, &terrainnormal);
                        tippoint += terrainnormal * .002;

                        bounceness = terrainnormal * findLength(&tipvelocity) * (abs(normaldotproduct(tipvelocity, terrainnormal)));
                        if (findLengthfast(&tipvelocity) < findLengthfast(&bounceness)) {
                            bounceness = 0;
                        }
                        frictionness = abs(normaldotproduct(tipvelocity, terrainnormal));
                        tipvelocity -= bounceness;
                        if (1 - friction * frictionness > 0) {
                            tipvelocity *= 1 - friction * frictionness;
                        } else {
                            tipvelocity = 0;
                        }
                        tipvelocity += bounceness * elasticity;

                        if (findLengthfast(&bounceness) > 1) {
                            int whichsound;
                            if (type == staff) {
                                whichsound = footstepsound3 + abs(Random() % 2);
                            } else {
                                whichsound = clank1sound + abs(Random() % 4);
                            }
                            emit_sound_at(whichsound, position, 128 * findLengthfast(&bounceness));
                        }
                    }

                    if ((Object::objects[k]->type != boxtype && Object::objects[k]->type != platformtype && Object::objects[k]->type != walltype && Object::objects[k]->type != weirdtype) || Object::objects[k]->pitch != 0) {
                        for (int m = 0; m < 2; m++) {
                            mid = (position * (21 + (float)m * 10) + tippoint * (19 - (float)m * 10)) / 40;
                            oldmid2 = mid;
                            oldmid = (oldposition * (21 + (float)m * 10) + oldtippoint * (19 - (float)m * 10)) / 40;

                            start = oldmid;
                            end = mid;
                            whichhit = Object::objects[k]->model.LineCheck(&start, &end, &colpoint, &Object::objects[k]->position, &Object::objects[k]->yaw);
                            if (whichhit != -1) {
                                hitsomething = 1;
                                mid = colpoint;
                                terrainnormal = DoRotation(Object::objects[k]->model.Triangles[whichhit].facenormal, 0, Object::objects[k]->yaw, 0) * -1;
                                ReflectVector(&velocity, &terrainnormal);

                                bounceness = terrainnormal * findLength(&velocity) * (abs(normaldotproduct(velocity, terrainnormal)));
                                if (findLengthfast(&velocity) < findLengthfast(&bounceness)) {
                                    bounceness = 0;
                                }
                                frictionness = abs(normaldotproduct(velocity, terrainnormal));
                                velocity -= bounceness;
                                if (1 - friction * frictionness > 0) {
                                    velocity *= 1 - friction * frictionness;
                                } else {
                                    velocity = 0;
                                }
                                velocity += bounceness * elasticity;

                                if (findLengthfast(&bounceness) > 1) {
                                    int whichsound;
                                    if (type == staff) {
                                        whichsound = footstepsound3 + abs(Random() % 2);
                                    } else {
                                        whichsound = clank1sound + abs(Random() % 4);
                                    }
                                    emit_sound_at(whichsound, mid, 128 * findLengthfast(&bounceness));
                                }
                                position += (mid - oldmid2) * (20 / (1 + (float)m * 10));
                            }

                            mid = (position * (19 - (float)m * 10) + tippoint * (21 + (float)m * 10)) / 40;
                            oldmid2 = mid;
                            oldmid = (oldposition * (19 - (float)m * 10) + oldtippoint * (21 + (float)m * 10)) / 40;

                            start = oldmid;
                            end = mid;
                            whichhit = Object::objects[k]->model.LineCheck(&start, &end, &colpoint, &Object::objects[k]->position, &Object::objects[k]->yaw);
                            if (whichhit != -1) {
                                hitsomething = 1;
                                mid = colpoint;
                                terrainnormal = DoRotation(Object::objects[k]->model.Triangles[whichhit].facenormal, 0, Object::objects[k]->yaw, 0) * -1;
                                ReflectVector(&tipvelocity, &terrainnormal);

                                bounceness = terrainnormal * findLength(&tipvelocity) * (abs(normaldotproduct(tipvelocity, terrainnormal)));
                                if (findLengthfast(&tipvelocity) < findLengthfast(&bounceness)) {
                                    bounceness = 0;
                                }
                                frictionness = abs(normaldotproduct(tipvelocity, terrainnormal));
                                tipvelocity -= bounceness;
                                if (1 - friction * frictionness > 0) {
                                    tipvelocity *= 1 - friction * frictionness;
                                } else {
                                    tipvelocity = 0;
                                }
                                tipvelocity += bounceness * elasticity;

                                if (findLengthfast(&bounceness) > 1) {
                                    int whichsound;
                                    if (type == staff) {
                                        whichsound = footstepsound3 + abs(Random() % 2);
                                    } else {
                                        whichsound = clank1sound + abs(Random() % 4);
                                    }
                                    emit_sound_at(whichsound, mid, 128 * findLengthfast(&bounceness));
                                }
                                tippoint += (mid - oldmid2) * (20 / (1 + (float)m * 10));
                            }
                        }
                    } else {
                        start = position;
                        end = tippoint;
                        whichhit = Object::objects[k]->model.LineCheck(&start, &end, &colpoint, &Object::objects[k]->position, &Object::objects[k]->yaw);
                        if (whichhit != -1) {
                            hitsomething = 1;
                            closestdistance = -1;
                            closestswordpoint = colpoint;
                            point[0] = DoRotation(Object::objects[k]->model.getTriangleVertex(whichhit, 0), 0, Object::objects[k]->yaw, 0) + Object::objects[k]->position;
                            point[1] = DoRotation(Object::objects[k]->model.getTriangleVertex(whichhit, 1), 0, Object::objects[k]->yaw, 0) + Object::objects[k]->position;
                            point[2] = DoRotation(Object::objects[k]->model.getTriangleVertex(whichhit, 2), 0, Object::objects[k]->yaw, 0) + Object::objects[k]->position;
                            if (DistancePointLine(&closestswordpoint, &point[0], &point[1], &distance, &colpoint)) {
                                if (distance < closestdistance || closestdistance == -1) {
                                    closestpoint = colpoint;
                                    closestdistance = distance;
                                }
                            }
                            if (DistancePointLine(&closestswordpoint, &point[1], &point[2], &distance, &colpoint)) {
                                if (distance < closestdistance || closestdistance == -1) {
                                    closestpoint = colpoint;
                                    closestdistance = distance;
                                }
                            }
                            if (DistancePointLine(&closestswordpoint, &point[2], &point[0], &distance, &colpoint)) {
                                if (distance < closestdistance || closestdistance == -1) {
                                    closestpoint = colpoint;
                                    closestdistance = distance;
                                }
                            }
                            if (closestdistance != -1 && isnormal(closestdistance)) {
                                if (DistancePointLine(&closestpoint, &position, &tippoint, &distance, &colpoint)) {
                                    closestswordpoint = colpoint;
                                    velocity += (closestpoint - closestswordpoint);
                                    tipvelocity += (closestpoint - closestswordpoint);
                                    position += (closestpoint - closestswordpoint);
                                    tippoint += (closestpoint - closestswordpoint);
                                }
                            }
                        }
                    }
                }
            }
            //Terrain collisions
            whichhit = terrain.lineTerrain(oldposition, position, &colpoint);
            if (whichhit != -1 || position.y < terrain.getHeight(position.x, position.z)) {
                hitsomething = 1;
                if (whichhit != -1) {
                    position = colpoint * terrain.scale;
                } else {
                    position.y = terrain.getHeight(position.x, position.z);
                }

                terrainnormal = terrain.getNormal(position.x, position.z);
                ReflectVector(&velocity, &terrainnormal);
                position += terrainnormal * .002;
                bounceness = terrainnormal * findLength(&velocity) * (abs(normaldotproduct(velocity, terrainnormal)));
                if (findLengthfast(&velocity) < findLengthfast(&bounceness)) {
                    bounceness = 0;
                }
                frictionness = abs(normaldotproduct(velocity, terrainnormal));
                velocity -= bounceness;
                if (1 - friction * frictionness > 0) {
                    velocity *= 1 - friction * frictionness;
                } else {
                    velocity = 0;
                }
                if (terrain.getOpacity(position.x, position.z) < .2) {
                    velocity += bounceness * elasticity * .3;
                } else {
                    velocity += bounceness * elasticity;
                }

                if (findLengthfast(&bounceness) > 1) {
                    int whichsound;
                    if (terrain.getOpacity(position.x, position.z) > .2) {
                        if (type == staff) {
                            whichsound = footstepsound3 + abs(Random() % 2);
                        } else {
                            whichsound = clank1sound + abs(Random() % 4);
                        }
                    } else {
                        whichsound = footstepsound + abs(Random() % 2);
                    }
                    emit_sound_at(whichsound, position,
                                  findLengthfast(&bounceness) * (terrain.getOpacity(position.x, position.z) > .2 ? 128. : 32.));

                    if (terrain.getOpacity(position.x, position.z) < .2) {
                        XYZ terrainlight;
                        terrainlight = terrain.getLighting(position.x, position.z);
                        if (environment == snowyenvironment) {
                            if (distsq(&position, &viewer) < viewdistance * viewdistance / 4) {
                                Sprite::MakeSprite(cloudsprite, position, velocity, terrainlight.x, terrainlight.y, terrainlight.z, .5, .7);
                            }
                        } else if (environment == grassyenvironment) {
                            if (distsq(&position, &viewer) < viewdistance * viewdistance / 4) {
                                Sprite::MakeSprite(cloudsprite, position, velocity, terrainlight.x * 90 / 255, terrainlight.y * 70 / 255, terrainlight.z * 8 / 255, .5, .5);
                            }
                        } else if (environment == desertenvironment) {
                            if (distsq(&position, &viewer) < viewdistance * viewdistance / 4) {
                                Sprite::MakeSprite(cloudsprite, position, velocity, terrainlight.x * 190 / 255, terrainlight.y * 170 / 255, terrainlight.z * 108 / 255, .5, .7);
                            }
                        }
                    }
                }
            }
            whichhit = terrain.lineTerrain(oldtippoint, tippoint, &colpoint);
            if (whichhit != -1 || tippoint.y < terrain.getHeight(tippoint.x, tippoint.z)) {
                if (whichhit != -1) {
                    tippoint = colpoint * terrain.scale;
                } else {
                    tippoint.y = terrain.getHeight(tippoint.x, tippoint.z);
                }

                terrainnormal = terrain.getNormal(tippoint.x, tippoint.z);
                ReflectVector(&tipvelocity, &terrainnormal);
                tippoint += terrainnormal * .002;
                bounceness = terrainnormal * findLength(&tipvelocity) * (abs(normaldotproduct(tipvelocity, terrainnormal)));
                if (findLengthfast(&tipvelocity) < findLengthfast(&bounceness)) {
                    bounceness = 0;
                }
                frictionness = abs(normaldotproduct(tipvelocity, terrainnormal));
                tipvelocity -= bounceness;
                if (1 - friction * frictionness > 0) {
                    tipvelocity *= 1 - friction * frictionness;
                } else {
                    tipvelocity = 0;
                }
                if (terrain.getOpacity(tippoint.x, tippoint.z) < .2) {
                    tipvelocity += bounceness * elasticity * .3;
                } else {
                    tipvelocity += bounceness * elasticity;
                }

                if (findLengthfast(&bounceness) > 1) {
                    int whichsound;
                    if (terrain.getOpacity(tippoint.x, tippoint.z) > .2) {
                        if (type == staff) {
                            whichsound = footstepsound3 + abs(Random() % 2);
                        } else {
                            whichsound = clank1sound + abs(Random() % 4);
                        }
                    } else {
                        whichsound = footstepsound + abs(Random() % 2);
                    }
                    emit_sound_at(whichsound, tippoint,
                                  findLengthfast(&bounceness) * (terrain.getOpacity(tippoint.x, tippoint.z) > .2 ? 128. : 32.));

                    if (terrain.getOpacity(tippoint.x, tippoint.z) < .2) {
                        XYZ terrainlight;
                        terrainlight = terrain.getLighting(tippoint.x, tippoint.z);
                        if (environment == snowyenvironment) {
                            if (distsq(&tippoint, &viewer) < viewdistance * viewdistance / 4) {
                                Sprite::MakeSprite(cloudsprite, tippoint, tipvelocity, terrainlight.x, terrainlight.y, terrainlight.z, .5, .7);
                            }
                        } else if (environment == grassyenvironment) {
                            if (distsq(&tippoint, &viewer) < viewdistance * viewdistance / 4) {
                                Sprite::MakeSprite(cloudsprite, tippoint, tipvelocity, terrainlight.x * 90 / 255, terrainlight.y * 70 / 255, terrainlight.z * 8 / 255, .5, .5);
                            }
                        } else if (environment == desertenvironment) {
                            if (distsq(&tippoint, &viewer) < viewdistance * viewdistance / 4) {
                                Sprite::MakeSprite(cloudsprite, tippoint, tipvelocity, terrainlight.x * 190 / 255, terrainlight.y * 170 / 255, terrainlight.z * 108 / 255, .5, .7);
                            }
                        }
                    }
                }
            }

            //Edges
            mid = position + tippoint;
            mid /= 2;
            mid += (position - mid) / 20;
            oldmid = mid;
            if (mid.y < terrain.getHeight(mid.x, mid.z)) {
                hitsomething = 1;
                mid.y = terrain.getHeight(mid.x, mid.z);

                terrainnormal = terrain.getNormal(mid.x, mid.z);
                ReflectVector(&velocity, &terrainnormal);
                //mid+=terrainnormal*.002;
                bounceness = terrainnormal * findLength(&velocity) * (abs(normaldotproduct(velocity, terrainnormal)));
                if (findLengthfast(&velocity) < findLengthfast(&bounceness)) {
                    bounceness = 0;
                }
                frictionness = abs(normaldotproduct(velocity, terrainnormal));
                velocity -= bounceness;
                if (1 - friction * frictionness > 0) {
                    velocity *= 1 - friction * frictionness;
                } else {
                    velocity = 0;
                }
                if (terrain.getOpacity(mid.x, mid.z) < .2) {
                    velocity += bounceness * elasticity * .3;
                } else {
                    velocity += bounceness * elasticity;
                }

                if (findLengthfast(&bounceness) > 1) {
                    int whichsound;
                    if (terrain.getOpacity(mid.x, mid.z) > .2) {
                        if (type == staff) {
                            whichsound = footstepsound3 + abs(Random() % 2);
                        }
                        if (type != staff) {
                            whichsound = clank1sound + abs(Random() % 4);
                        }
                    } else {
                        whichsound = footstepsound + abs(Random() % 2);
                    }
                    emit_sound_at(whichsound, mid,
                                  findLengthfast(&bounceness) * (terrain.getOpacity(position.x, position.z) > .2
                                                                     ? 128.
                                                                     : 32.));
                }
                position += (mid - oldmid) * 20;
            }

            mid = position + tippoint;
            mid /= 2;
            mid += (tippoint - mid) / 20;
            oldmid = mid;
            if (mid.y < terrain.getHeight(mid.x, mid.z)) {
                hitsomething = 1;
                mid.y = terrain.getHeight(mid.x, mid.z);

                terrainnormal = terrain.getNormal(mid.x, mid.z);
                ReflectVector(&tipvelocity, &terrainnormal);
                //mid+=terrainnormal*.002;
                bounceness = terrainnormal * findLength(&tipvelocity) * (abs(normaldotproduct(tipvelocity, terrainnormal)));
                if (findLengthfast(&tipvelocity) < findLengthfast(&bounceness)) {
                    bounceness = 0;
                }
                frictionness = abs(normaldotproduct(tipvelocity, terrainnormal));
                tipvelocity -= bounceness;
                if (1 - friction * frictionness > 0) {
                    tipvelocity *= 1 - friction * frictionness;
                } else {
                    tipvelocity = 0;
                }
                if (terrain.getOpacity(mid.x, mid.z) < .2) {
                    tipvelocity += bounceness * elasticity * .3;
                } else {
                    tipvelocity += bounceness * elasticity;
                }

                if (findLengthfast(&bounceness) > 1) {
                    int whichsound;
                    if (terrain.getOpacity(mid.x, mid.z) > .2) {
                        if (type == staff) {
                            whichsound = footstepsound3 + abs(Random() % 2);
                        }
                        if (type != staff) {
                            whichsound = clank1sound + abs(Random() % 4);
                        }
                    } else {
                        whichsound = footstepsound + abs(Random() % 2);
                    }
                    emit_sound_at(whichsound, mid,
                                  findLengthfast(&bounceness) * (terrain.getOpacity(position.x, position.z) > .2
                                                                     ? 128.
                                                                     : 32.));
                }
                tippoint += (mid - oldmid) * 20;
            }
            //Gravity
            velocity.y += gravity * multiplier;
            tipvelocity.y += gravity * multiplier;

            //Rotation
            XYZ temppoint1, temppoint2;
            float distance;

            temppoint1 = position;
            temppoint2 = tippoint;
            distance = findDistance(&temppoint1, &temppoint2);
            rotation2 = asin((temppoint1.y - temppoint2.y) / distance);
            rotation2 *= 360 / 6.28;
            temppoint1.y = 0;
            temppoint2.y = 0;
            rotation1 = acos((temppoint1.z - temppoint2.z) / findDistance(&temppoint1, &temppoint2));
            rotation1 *= 360 / 6.28;
            rotation3 = 0;
            smallrotation = 90;
            smallrotation2 = 0;
            bigtilt = 0;
            bigtilt2 = 0;
            bigrotation = 0;
            if (temppoint1.x > temppoint2.x) {
                rotation1 = 360 - rotation1;
            }

            //Stop moving
            if (findLengthfast(&velocity) < .3 && findLengthfast(&tipvelocity) < .3 && hitsomething) {
                freetime += multiplier;
            }

            if (freetime > .4) {
                velocity = 0;
                tipvelocity = 0;
            }
            firstfree = 0;
        }
    }
    multiplier = tempmult;
    if (blooddrip && bloody) {
        blooddripdelay -= blooddrip * multiplier / 2;
        blooddrip -= multiplier;
        if (blooddrip < 0) {
            blooddrip = 0;
        }
        if (blooddrip > 5) {
            blooddrip = 5;
        }
        if (blooddripdelay < 0 && bloodtoggle) {
            blooddripdelay = 1;
            XYZ bloodvel;
            XYZ bloodloc;
            bloodloc = position + (tippoint - position) * .7;
            bloodloc.y -= .05;
            if (bloodtoggle) {
                bloodvel = 0;
                Sprite::MakeSprite(bloodsprite, bloodloc, bloodvel, 1, 1, 1, .03, 1);
            }
        }
    }
    if (onfire) {
        flamedelay -= multiplier;
        if (onfire && flamedelay <= 0) {
            flamedelay = .020;
            flamedelay -= multiplier;
            normalrot = 0;
            if (owner != -1) {
                normalrot = Person::players[owner]->velocity;
            }
            normalrot.y += 1;
            if (owner != -1) {
                if (Person::players[owner]->onterrain) {
                    normalrot.y = 1;
                }
            }
            Sprite::MakeSprite(weaponflamesprite, position + tippoint * (((float)abs(Random() % 100)) / 600 + .05), normalrot, 1, 1, 1, (.6 + (float)abs(Random() % 100) / 200 - .25) * 1 / 3, 1);
            Sprite::setLastSpriteSpeed(4);
            Sprite::setLastSpriteAlivetime(.3);
        }
    }

    if (!onfire && owner == -1 && type != staff) {
        flamedelay -= multiplier;
        if (flamedelay <= 0) {
            flamedelay = .020;
            flamedelay -= multiplier;
            normalrot = 0;
            if (Random() % 50 == 0 && distsq(&position, &viewer) > 80) {
                XYZ shinepoint;
                shinepoint = position + (tippoint - position) * (((float)abs(Random() % 100)) / 100);
                Sprite::MakeSprite(weaponshinesprite, shinepoint, normalrot, 1, 1, 1, (.1 + (float)abs(Random() % 100) / 200 - .25) * 1 / 3 * fast_sqrt(findDistance(&shinepoint, &viewer)), 1);
                Sprite::setLastSpriteSpeed(4);
                Sprite::setLastSpriteAlivetime(.3);
            }
        }
    }
}

void Weapons::DoStuff()
{
    //Move
    int i = 0;
    for (std::vector<Weapon>::iterator weapon = begin(); weapon != end(); ++weapon) {
        weapon->doStuff(i++);
    }
}

void Weapon::draw()
{
    static XYZ terrainlight;
    static GLfloat M[16];

    if ((frustum.SphereInFrustum(position.x, position.y, position.z, 1) &&
         distsq(&viewer, &position) < viewdistance * viewdistance)) {
        bool draw = false;
        if (owner == -1) {
            draw = true;
            if (velocity.x && !physics) {
                drawhowmany = 10;
            } else {
                drawhowmany = 1;
            }
        } else {
            if (Person::players[owner]->occluded < 25) {
                if ((frustum.SphereInFrustum(Person::players[owner]->coords.x, Person::players[owner]->coords.y + Person::players[owner]->scale * 3, Person::players[owner]->coords.z, Person::players[owner]->scale * 8) && distsq(&viewer, &Person::players[owner]->coords) < viewdistance * viewdistance) || Person::players[owner]->skeleton.free == 3) {
                    draw = true;
                }
            }
            if (
                (Person::players[owner]->animTarget == knifeslashstartanim ||
                 Person::players[owner]->animTarget == swordsneakattackanim ||
                 (Person::players[owner]->animCurrent == staffhitanim && Person::players[owner]->frameCurrent > 1) ||
                 (Person::players[owner]->animCurrent == staffhitreversedanim && Person::players[owner]->frameCurrent > 1) ||
                 (Person::players[owner]->animCurrent == staffspinhitanim && Person::players[owner]->frameCurrent > 1) ||
                 (Person::players[owner]->animCurrent == staffspinhitreversedanim && Person::players[owner]->frameCurrent > 1) ||
                 (Person::players[owner]->animCurrent == staffgroundsmashanim && Person::players[owner]->frameCurrent > 1) ||
                 (Person::players[owner]->animTarget == swordslashanim && Person::players[owner]->frameTarget < 7) ||
                 Person::players[owner]->animTarget == crouchstabanim ||
                 Person::players[owner]->animTarget == swordslashreversalanim ||
                 Person::players[owner]->animTarget == swordslashreversedanim ||
                 Person::players[owner]->animTarget == knifefollowanim ||
                 Person::players[owner]->animTarget == swordgroundstabanim ||
                 Person::players[owner]->animTarget == knifethrowanim) &&
                Person::players[owner]->animTarget == lastdrawnanim &&
                !Person::players[owner]->skeleton.free) {
                drawhowmany = 10;
            } else {
                drawhowmany = 1;
            }
            if (Person::players[owner]->animTarget == swordgroundstabanim) {
                lastdrawnrotation1 = rotation1;
                lastdrawnrotation2 = rotation2;
                lastdrawnrotation3 = rotation3;
                lastdrawnbigrotation = bigrotation;
                lastdrawnbigtilt = bigtilt;
                lastdrawnbigtilt2 = bigtilt2;
                lastdrawnsmallrotation = smallrotation;
                lastdrawnsmallrotation2 = smallrotation2;
            }
        }
        if (draw) {
            terrainlight = terrain.getLighting(position.x, position.z);
            if (drawhowmany > 0) {
                glAlphaFunc(GL_GREATER, 0.01);
            }
            for (int j = drawhowmany; j > 0; j--) {
                glMatrixMode(GL_MODELVIEW);
                glPushMatrix();
                glColor4f(terrainlight.x, terrainlight.y, terrainlight.z, j / drawhowmany);
                if (owner == -1) {
                    glTranslatef(position.x * (((float)(j)) / drawhowmany) + lastdrawnposition.x * (1 - ((float)(j)) / drawhowmany), position.y * (((float)(j)) / drawhowmany) + lastdrawnposition.y * (1 - ((float)(j)) / drawhowmany), position.z * (((float)(j)) / drawhowmany) + lastdrawnposition.z * (1 - ((float)(j)) / drawhowmany));
                } else {
                    glTranslatef(position.x * (((float)(j)) / drawhowmany) + lastdrawnposition.x * (1 - ((float)(j)) / drawhowmany), position.y * (((float)(j)) / drawhowmany) - .02 + lastdrawnposition.y * (1 - ((float)(j)) / drawhowmany), position.z * (((float)(j)) / drawhowmany) + lastdrawnposition.z * (1 - ((float)(j)) / drawhowmany));
                }
                glRotatef(bigrotation * (((float)(j)) / drawhowmany) + lastdrawnbigrotation * (1 - ((float)(j)) / drawhowmany), 0, 1, 0);
                glRotatef(bigtilt2 * (((float)(j)) / drawhowmany) + lastdrawnbigtilt2 * (1 - ((float)(j)) / drawhowmany), 1, 0, 0);
                glRotatef(bigtilt * (((float)(j)) / drawhowmany) + lastdrawnbigtilt * (1 - ((float)(j)) / drawhowmany), 0, 0, 1);
                glRotatef(-rotation1 * (((float)(j)) / drawhowmany) - lastdrawnrotation1 * (1 - ((float)(j)) / drawhowmany) + 90, 0, 1, 0);
                glRotatef(-rotation2 * (((float)(j)) / drawhowmany) - lastdrawnrotation2 * (1 - ((float)(j)) / drawhowmany) + 90, 0, 0, 1);
                glRotatef(-rotation3 * (((float)(j)) / drawhowmany) - lastdrawnrotation3 * (1 - ((float)(j)) / drawhowmany), 0, 1, 0);
                glRotatef(smallrotation * (((float)(j)) / drawhowmany) + lastdrawnsmallrotation * (1 - ((float)(j)) / drawhowmany), 1, 0, 0);
                glRotatef(smallrotation2 * (((float)(j)) / drawhowmany) + lastdrawnsmallrotation2 * (1 - ((float)(j)) / drawhowmany), 0, 1, 0);

                if (owner != -1) {
                    if (Person::players[owner]->animTarget == staffhitanim || Person::players[owner]->animCurrent == staffhitanim || Person::players[owner]->animTarget == staffhitreversedanim || Person::players[owner]->animCurrent == staffhitreversedanim) {
                        glTranslatef(0, 0, -.3);
                    }
                    if (Person::players[owner]->animTarget == staffgroundsmashanim || Person::players[owner]->animCurrent == staffgroundsmashanim || Person::players[owner]->animTarget == staffspinhitreversedanim || Person::players[owner]->animCurrent == staffspinhitreversedanim || Person::players[owner]->animTarget == staffspinhitanim || Person::players[owner]->animCurrent == staffspinhitanim) {
                        glTranslatef(0, 0, -.1);
                    }
                }

                glEnable(GL_LIGHTING);
                switch (type) {
                    case knife:
                        if (!bloody || !bloodtoggle) {
                            throwingknifemodel.drawdifftex(knifetextureptr);
                        }
                        if (bloodtoggle) {
                            if (bloody == 1) {
                                throwingknifemodel.drawdifftex(lightbloodknifetextureptr);
                            }
                            if (bloody == 2) {
                                throwingknifemodel.drawdifftex(bloodknifetextureptr);
                            }
                        }
                        break;
                    case sword:
                        if (!bloody || !bloodtoggle) {
                            swordmodel.drawdifftex(swordtextureptr);
                        }
                        if (bloodtoggle) {
                            if (bloody == 1) {
                                swordmodel.drawdifftex(lightbloodswordtextureptr);
                            }
                            if (bloody == 2) {
                                swordmodel.drawdifftex(bloodswordtextureptr);
                            }
                        }
                        break;
                    case staff:
                        staffmodel.drawdifftex(stafftextureptr);
                        break;
                }

                glPopMatrix();
            }

            lastdrawnposition = position;
            lastdrawntippoint = tippoint;
            lastdrawnrotation1 = rotation1;
            lastdrawnrotation2 = rotation2;
            lastdrawnrotation3 = rotation3;
            lastdrawnbigrotation = bigrotation;
            lastdrawnbigtilt = bigtilt;
            lastdrawnbigtilt2 = bigtilt2;
            lastdrawnsmallrotation = smallrotation;
            lastdrawnsmallrotation2 = smallrotation2;
            if (owner != -1) {
                lastdrawnanim = Person::players[owner]->animCurrent;
            }
        }
        if (owner != -1) {
            glMatrixMode(GL_MODELVIEW);
            glPushMatrix();
            glLoadIdentity();
            glTranslatef(position.x, position.y - .02, position.z);
            glRotatef(bigrotation, 0, 1, 0);
            glRotatef(bigtilt2, 1, 0, 0);
            glRotatef(bigtilt, 0, 0, 1);
            glRotatef(-rotation1 + 90, 0, 1, 0);
            glRotatef(-rotation2 + 90, 0, 0, 1);
            glRotatef(-rotation3, 0, 1, 0);
            glRotatef(smallrotation, 1, 0, 0);
            glRotatef(smallrotation2, 0, 1, 0);
            glTranslatef(0, 0, length);
            glGetFloatv(GL_MODELVIEW_MATRIX, M);
            tippoint.x = M[12];
            tippoint.y = M[13];
            tippoint.z = M[14];
            glPopMatrix();
        }
    }
}

void Weapon::drop(XYZ v, XYZ tv, bool sethitsomething)
{
    owner = -1;
    velocity = v;
    tipvelocity = tv;
    missed = 1;
    if (sethitsomething) {
        hitsomething = 0;
    }
    freetime = 0;
    firstfree = 1;
    physics = 1;
}

void Weapon::thrown(XYZ v, bool sethitsomething)
{
    drop(v, v, sethitsomething);
    missed = 0;
    physics = 0;
}

int Weapons::Draw()
{
    glAlphaFunc(GL_GREATER, 0.9);
    glEnable(GL_TEXTURE_2D);
    glEnable(GL_BLEND);
    glEnable(GL_CULL_FACE);
    glCullFace(GL_FRONT);
    glDepthMask(1);

    for (std::vector<Weapon>::iterator weapon = begin(); weapon != end(); ++weapon) {
        weapon->draw();
    }
    return 0;
}

Weapons::Weapons()
{
}
