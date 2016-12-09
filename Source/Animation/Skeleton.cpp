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

/**> HEADER FILES <**/
#include "Game.h"
#include "Animation/Skeleton.h"
#include "openal_wrapper.h"
#include "Animation/Animation.h"
#include "Utils/Folders.h"

extern float multiplier;
extern float gravity;
extern Terrain terrain;
extern Objects objects;
extern int environment;
extern float camerashake;
extern bool freeze;
extern int detail;
extern int tutoriallevel;

extern int whichjointstartarray[26];
extern int whichjointendarray[26];

extern bool visibleloading;

/* EFFECT
 */
void dealloc2(void* param)
{
    free(param);
}

/* EFFECT
 * sets forward, lowforward, specialforward[]
 *
 * USES:
 * Skeleton::Load
 * Person/Person::DoAnimations
 * Person/Person::DrawSkeleton
 */
void Skeleton::FindForwards()
{
    //Find forward vectors
    CrossProduct(joints[forwardjoints[1]].position - joints[forwardjoints[0]].position, joints[forwardjoints[2]].position - joints[forwardjoints[0]].position, &forward);
    Normalise(&forward);

    CrossProduct(joints[lowforwardjoints[1]].position - joints[lowforwardjoints[0]].position, joints[lowforwardjoints[2]].position - joints[lowforwardjoints[0]].position, &lowforward);
    Normalise(&lowforward);

    //Special forwards
    specialforward[0] = forward;

    specialforward[1] = jointPos(rightshoulder) + jointPos(rightwrist);
    specialforward[1] = jointPos(rightelbow) - specialforward[1] / 2;
    specialforward[1] += forward * .4;
    Normalise(&specialforward[1]);
    specialforward[2] = jointPos(leftshoulder) + jointPos(leftwrist);
    specialforward[2] = jointPos(leftelbow) - specialforward[2] / 2;
    specialforward[2] += forward * .4;
    Normalise(&specialforward[2]);

    specialforward[3] = jointPos(righthip) + jointPos(rightankle);
    specialforward[3] = specialforward[3] / 2 - jointPos(rightknee);
    specialforward[3] += lowforward * .4;
    Normalise(&specialforward[3]);
    specialforward[4] = jointPos(lefthip) + jointPos(leftankle);
    specialforward[4] = specialforward[4] / 2 - jointPos(leftknee);
    specialforward[4] += lowforward * .4;
    Normalise(&specialforward[4]);
}

/* EFFECT
 * TODO
 *
 * USES:
 * Person/Person::RagDoll
 * Person/Person::DoStuff
 * Person/IKHelper
 */
float Skeleton::DoConstraints(XYZ *coords, float *scale)
{
    float friction = 1.5;
    const float elasticity = .3;
    XYZ bounceness;
    const int numrepeats = 3;
    float groundlevel = .15;
    int i, j, k, m;
    XYZ temp;
    XYZ terrainnormal;
    int whichhit;
    float frictionness;
    XYZ terrainlight;
    int whichpatchx;
    int whichpatchz;
    float damage = 0; // eventually returned from function
    bool breaking = false;

    if (free) {
        freetime += multiplier;

        whichpatchx = coords->x / (terrain.size / subdivision * terrain.scale);
        whichpatchz = coords->z / (terrain.size / subdivision * terrain.scale);

        terrainlight = *coords;
        objects.SphereCheckPossible(&terrainlight, 1);

        //Add velocity
        for (i = 0; i < num_joints; i++) {
            joints[i].position = joints[i].position + joints[i].velocity * multiplier;

            switch (joints[i].label) {
            case head:
                groundlevel = .8;
                break;
            case righthand:
            case rightwrist:
            case rightelbow:
            case lefthand:
            case leftwrist:
            case leftelbow:
                groundlevel = .2;
                break;
            default:
                groundlevel = .15;
                break;
            }

            joints[i].position.y -= groundlevel;
            joints[i].oldvelocity = joints[i].velocity;
        }

        float tempmult = multiplier;
        //multiplier/=numrepeats;

        for (j = 0; j < numrepeats; j++) {
            float r = .05;
            // right leg constraints?
            if (!joint(rightknee).locked && !joint(righthip).locked) {
                temp = jointPos(rightknee) - (jointPos(righthip) + jointPos(rightankle)) / 2;
                while (normaldotproduct(temp, lowforward) > -.1 && !sphere_line_intersection(&jointPos(righthip), &jointPos(rightankle), &jointPos(rightknee), &r)) {
                    jointPos(rightknee) -= lowforward * .05;
                    if (spinny)
                        jointVel(rightknee) -= lowforward * .05 / multiplier / 4;
                    else
                        jointVel(rightknee) -= lowforward * .05;
                    jointPos(rightankle) += lowforward * .025;
                    if (spinny)
                        jointVel(rightankle) += lowforward * .025 / multiplier / 4;
                    else
                        jointVel(rightankle) += lowforward * .25;
                    jointPos(righthip) += lowforward * .025;
                    if (spinny)
                        jointVel(righthip) += lowforward * .025 / multiplier / 4;
                    else
                        jointVel(righthip) += lowforward * .025;
                    temp = jointPos(rightknee) - (jointPos(righthip) + jointPos(rightankle)) / 2;
                }
            }

            // left leg constraints?
            if (!joint(leftknee).locked && !joint(lefthip).locked) {
                temp = jointPos(leftknee) - (jointPos(lefthip) + jointPos(leftankle)) / 2;
                while (normaldotproduct(temp, lowforward) > -.1 && !sphere_line_intersection(&jointPos(lefthip), &jointPos(leftankle), &jointPos(leftknee), &r)) {
                    jointPos(leftknee) -= lowforward * .05;
                    if (spinny)
                        jointVel(leftknee) -= lowforward * .05 / multiplier / 4;
                    else
                        jointVel(leftknee) -= lowforward * .05;
                    jointPos(leftankle) += lowforward * .025;
                    if (spinny)
                        jointVel(leftankle) += lowforward * .025 / multiplier / 4;
                    else
                        jointVel(leftankle) += lowforward * .25;
                    jointPos(lefthip) += lowforward * .025;
                    if (spinny)
                        jointVel(lefthip) += lowforward * .025 / multiplier / 4;
                    else
                        jointVel(lefthip) += lowforward * .025;
                    temp = jointPos(leftknee) - (jointPos(lefthip) + jointPos(leftankle)) / 2;
                }
            }

            for (i = 0; i < num_joints; i++) {
                if (joints[i].locked && !spinny && findLengthfast(&joints[i].velocity) > 320)
                    joints[i].locked = 0;
                if (spinny && findLengthfast(&joints[i].velocity) > 600)
                    joints[i].locked = 0;
                if (joints[i].delay > 0) {
                    bool freely = true;
                    for (j = 0; j < num_joints; j++) {
                        if (joints[j].locked)
                            freely = false;
                    }
                    if (freely)
                        joints[i].delay -= multiplier * 3;
                }
            }

            if (num_muscles)
                for (i = 0; i < num_muscles; i++) {
                    //Length constraints
                    muscles[i].DoConstraint(spinny);
                }

            for (i = 0; i < num_joints; i++) {
                //Length constraints
                //Ground constraint
                groundlevel = 0;
                if (joints[i].position.y * (*scale) + coords->y < terrain.getHeight(joints[i].position.x * (*scale) + coords->x, joints[i].position.z * (*scale) + coords->z) + groundlevel) {
                    freefall = 0;
                    friction = 1.5;
                    if (joints[i].label == groin && !joints[i].locked && joints[i].delay <= 0) {
                        joints[i].locked = 1;
                        joints[i].delay = 1;
                        if (tutoriallevel != 1 || id == 0) {
                            emit_sound_at(landsound1, joints[i].position * (*scale) + *coords, 128.);
                        }
                        breaking = true;
                    }

                    if (joints[i].label == head && !joints[i].locked && joints[i].delay <= 0) {
                        joints[i].locked = 1;
                        joints[i].delay = 1;
                        if (tutoriallevel != 1 || id == 0) {
                            emit_sound_at(landsound2, joints[i].position * (*scale) + *coords, 128.);
                        }
                    }

                    terrainnormal = terrain.getNormal(joints[i].position.x * (*scale) + coords->x, joints[i].position.z * (*scale) + coords->z);
                    ReflectVector(&joints[i].velocity, &terrainnormal);
                    bounceness = terrainnormal * findLength(&joints[i].velocity) * (abs(normaldotproduct(joints[i].velocity, terrainnormal)));
                    if (!joints[i].locked)
                        damage += findLengthfast(&bounceness) / 4000;
                    if (findLengthfast(&joints[i].velocity) < findLengthfast(&bounceness))
                        bounceness = 0;
                    frictionness = abs(normaldotproduct(joints[i].velocity, terrainnormal));
                    joints[i].velocity -= bounceness;
                    if (1 - friction * frictionness > 0)
                        joints[i].velocity *= 1 - friction * frictionness;
                    else
                        joints[i].velocity = 0;

                    if (tutoriallevel != 1 || id == 0)
                        if (findLengthfast(&bounceness) > 8000 && breaking) {
                            // FIXME: this crashes because k is not initialized!
                            // to reproduce, type 'wolfie' in console and play a while
                            // I'll just comment it out for now
                            //objects.model[k].MakeDecal(breakdecal, DoRotation(temp - objects.position[k], 0, -objects.yaw[k], 0), .4, .5, Random() % 360);
                            Sprite::MakeSprite(cloudsprite, joints[i].position * (*scale) + *coords, joints[i].velocity * .06, 1, 1, 1, 4, .2);
                            breaking = false;
                            camerashake += .6;

                            emit_sound_at(breaksound2, joints[i].position * (*scale) + *coords);

                            addEnvSound(*coords, 64);
                        }

                    if (findLengthfast(&bounceness) > 2500) {
                        Normalise(&bounceness);
                        bounceness = bounceness * 50;
                    }

                    joints[i].velocity += bounceness * elasticity;

                    if (findLengthfast(&joints[i].velocity) > findLengthfast(&joints[i].oldvelocity)) {
                        bounceness = 0;
                        joints[i].velocity = joints[i].oldvelocity;
                    }


                    if (joints[i].locked == 0)
                        if (findLengthfast(&joints[i].velocity) < 1)
                            joints[i].locked = 1;

                    if (environment == snowyenvironment && findLengthfast(&bounceness) > 500 && terrain.getOpacity(joints[i].position.x * (*scale) + coords->x, joints[i].position.z * (*scale) + coords->z) < .2) {
                        terrainlight = terrain.getLighting(joints[i].position.x * (*scale) + coords->x, joints[i].position.z * (*scale) + coords->z);
                        Sprite::MakeSprite(cloudsprite, joints[i].position * (*scale) + *coords, joints[i].velocity * .06, terrainlight.x, terrainlight.y, terrainlight.z, .5, .7);
                        if (detail == 2)
                            terrain.MakeDecal(bodyprintdecal, joints[i].position * (*scale) + *coords, .4, .4, 0);
                    } else if (environment == desertenvironment && findLengthfast(&bounceness) > 500 && terrain.getOpacity(joints[i].position.x * (*scale) + coords->x, joints[i].position.z * (*scale) + coords->z) < .2) {
                        terrainlight = terrain.getLighting(joints[i].position.x * (*scale) + coords->x, joints[i].position.z * (*scale) + coords->z);
                        Sprite::MakeSprite(cloudsprite, joints[i].position * (*scale) + *coords, joints[i].velocity * .06, terrainlight.x * 190 / 255, terrainlight.y * 170 / 255, terrainlight.z * 108 / 255, .5, .7);
                    }

                    else if (environment == grassyenvironment && findLengthfast(&bounceness) > 500 && terrain.getOpacity(joints[i].position.x * (*scale) + coords->x, joints[i].position.z * (*scale) + coords->z) < .2) {
                        terrainlight = terrain.getLighting(joints[i].position.x * (*scale) + coords->x, joints[i].position.z * (*scale) + coords->z);
                        Sprite::MakeSprite(cloudsprite, joints[i].position * (*scale) + *coords, joints[i].velocity * .06, terrainlight.x * 90 / 255, terrainlight.y * 70 / 255, terrainlight.z * 8 / 255, .5, .5);
                    } else if (findLengthfast(&bounceness) > 500)
                        Sprite::MakeSprite(cloudsprite, joints[i].position * (*scale) + *coords, joints[i].velocity * .06, terrainlight.x, terrainlight.y, terrainlight.z, .5, .2);


                    joints[i].position.y = (terrain.getHeight(joints[i].position.x * (*scale) + coords->x, joints[i].position.z * (*scale) + coords->z) + groundlevel - coords->y) / (*scale);
                    if (longdead > 100)
                        broken = 1;
                }
                if (terrain.patchobjectnum[whichpatchx][whichpatchz])
                    for (m = 0; m < terrain.patchobjectnum[whichpatchx][whichpatchz]; m++) {
                        k = terrain.patchobjects[whichpatchx][whichpatchz][m];
                        if (k < objects.numobjects && k >= 0)
                            if (objects.possible[k]) {
                                friction = objects.friction[k];
                                XYZ start = joints[i].realoldposition;
                                XYZ end = joints[i].position * (*scale) + *coords;
                                whichhit = objects.model[k].LineCheckPossible(&start, &end, &temp, &objects.position[k], &objects.yaw[k]);
                                if (whichhit != -1) {
                                    if (joints[i].label == groin && !joints[i].locked && joints[i].delay <= 0) {
                                        joints[i].locked = 1;
                                        joints[i].delay = 1;
                                        if (tutoriallevel != 1 || id == 0) {
                                            emit_sound_at(landsound1, joints[i].position * (*scale) + *coords, 128.);
                                        }
                                        breaking = true;
                                    }

                                    if (joints[i].label == head && !joints[i].locked && joints[i].delay <= 0) {
                                        joints[i].locked = 1;
                                        joints[i].delay = 1;
                                        if (tutoriallevel != 1 || id == 0) {
                                            emit_sound_at(landsound2, joints[i].position * (*scale) + *coords, 128.);
                                        }
                                    }

                                    terrainnormal = DoRotation(objects.model[k].facenormals[whichhit], 0, objects.yaw[k], 0) * -1;
                                    if (terrainnormal.y > .8)
                                        freefall = 0;
                                    bounceness = terrainnormal * findLength(&joints[i].velocity) * (abs(normaldotproduct(joints[i].velocity, terrainnormal)));
                                    if (findLengthfast(&joints[i].velocity) > findLengthfast(&joints[i].oldvelocity)) {
                                        bounceness = 0;
                                        joints[i].velocity = joints[i].oldvelocity;
                                    }
                                    if (tutoriallevel != 1 || id == 0)
                                        if (findLengthfast(&bounceness) > 4000 && breaking) {
                                            objects.model[k].MakeDecal(breakdecal, DoRotation(temp - objects.position[k], 0, -objects.yaw[k], 0), .4, .5, Random() % 360);
                                            Sprite::MakeSprite(cloudsprite, joints[i].position * (*scale) + *coords, joints[i].velocity * .06, 1, 1, 1, 4, .2);
                                            breaking = false;
                                            camerashake += .6;

                                            emit_sound_at(breaksound2, joints[i].position * (*scale) + *coords);

                                            addEnvSound(*coords, 64);
                                        }
                                    if (objects.type[k] == treetrunktype) {
                                        objects.rotx[k] += joints[i].velocity.x * multiplier * .4;
                                        objects.roty[k] += joints[i].velocity.z * multiplier * .4;
                                        objects.rotx[k + 1] += joints[i].velocity.x * multiplier * .4;
                                        objects.roty[k + 1] += joints[i].velocity.z * multiplier * .4;
                                    }
                                    if (!joints[i].locked)
                                        damage += findLengthfast(&bounceness) / 2500;
                                    ReflectVector(&joints[i].velocity, &terrainnormal);
                                    frictionness = abs(normaldotproduct(joints[i].velocity, terrainnormal));
                                    joints[i].velocity -= bounceness;
                                    if (1 - friction * frictionness > 0)
                                        joints[i].velocity *= 1 - friction * frictionness;
                                    else
                                        joints[i].velocity = 0;
                                    if (findLengthfast(&bounceness) > 2500) {
                                        Normalise(&bounceness);
                                        bounceness = bounceness * 50;
                                    }
                                    joints[i].velocity += bounceness * elasticity;


                                    if (!joints[i].locked)
                                        if (findLengthfast(&joints[i].velocity) < 1) {
                                            joints[i].locked = 1;
                                        }
                                    if (findLengthfast(&bounceness) > 500)
                                        Sprite::MakeSprite(cloudsprite, joints[i].position * (*scale) + *coords, joints[i].velocity * .06, 1, 1, 1, .5, .2);
                                    joints[i].position = (temp - *coords) / (*scale) + terrainnormal * .005;
                                    if (longdead > 100)
                                        broken = 1;
                                }
                            }
                    }
                joints[i].realoldposition = joints[i].position * (*scale) + *coords;
            }
        }
        multiplier = tempmult;


        if (terrain.patchobjectnum[whichpatchx][whichpatchz])
            for (m = 0; m < terrain.patchobjectnum[whichpatchx][whichpatchz]; m++) {
                k = terrain.patchobjects[whichpatchx][whichpatchz][m];
                if (objects.possible[k]) {
                    for (i = 0; i < 26; i++) {
                        //Make this less stupid
                        XYZ start = joints[jointlabels[whichjointstartarray[i]]].position * (*scale) + *coords;
                        XYZ end = joints[jointlabels[whichjointendarray[i]]].position * (*scale) + *coords;
                        whichhit = objects.model[k].LineCheckSlidePossible(&start, &end, &temp, &objects.position[k], &objects.yaw[k]);
                        if (whichhit != -1) {
                            joints[jointlabels[whichjointendarray[i]]].position = (end - *coords) / (*scale);
                            for (j = 0; j < num_muscles; j++) {
                                if ((muscles[j].parent1->label == whichjointstartarray[i] && muscles[j].parent2->label == whichjointendarray[i]) || (muscles[j].parent2->label == whichjointstartarray[i] && muscles[j].parent1->label == whichjointendarray[i]))
                                    muscles[j].DoConstraint(spinny);
                            }
                        }
                    }
                }
            }

        for (i = 0; i < num_joints; i++) {
            switch (joints[i].label) {
            case head:
                groundlevel = .8;
                break;
            case righthand:
            case rightwrist:
            case rightelbow:
            case lefthand:
            case leftwrist:
            case leftelbow:
                groundlevel = .2;
                break;
            default:
                groundlevel = .15;
                break;
            }
            joints[i].position.y += groundlevel;
            joints[i].mass = 1;
            if (joints[i].label == lefthip || joints[i].label == leftknee || joints[i].label == leftankle || joints[i].label == righthip || joints[i].label == rightknee || joints[i].label == rightankle)
                joints[i].mass = 2;
            if (joints[i].locked) {
                joints[i].mass = 4;
            }
        }

        return damage;
    }

    if (!free) {
        for (i = 0; i < num_muscles; i++) {
            if (muscles[i].type == boneconnect)
                muscles[i].DoConstraint(0);
        }
    }

    return 0;
}

/* EFFECT
 * applies gravity to the skeleton
 *
 * USES:
 * Person/Person::DoStuff
 */
void Skeleton::DoGravity(float *scale)
{
    static int i;
    for (i = 0; i < num_joints; i++) {
        if (
                (
                    ((joints[i].label != leftknee) && (joints[i].label != rightknee)) ||
                    (lowforward.y > -.1) ||
                    (joints[i].mass < 5)
                ) && (
                    ((joints[i].label != leftelbow) && (joints[i].label != rightelbow)) ||
                    (forward.y < .3)
                )
            )
            joints[i].velocity.y += gravity * multiplier / (*scale);
    }
}

/* EFFECT
 * set muscles[which].rotate1
 *     .rotate2
 *     .rotate3
 *
 * special case if animation == hanganim
 */
void Skeleton::FindRotationMuscle(int which, int animation)
{
    XYZ p1, p2, fwd;
    float dist;

    p1 = muscles[which].parent1->position;
    p2 = muscles[which].parent2->position;
    dist = findDistance(&p1, &p2);
    if (p1.y - p2.y <= dist)
        muscles[which].rotate2 = asin((p1.y - p2.y) / dist);
    if (p1.y - p2.y > dist)
        muscles[which].rotate2 = asin(1.f);
    muscles[which].rotate2 *= 360.0 / 6.2831853;

    p1.y = 0;
    p2.y = 0;
    dist = findDistance(&p1, &p2);
    if (p1.z - p2.z <= dist)
        muscles[which].rotate1 = acos((p1.z - p2.z) / dist);
    if (p1.z - p2.z > dist)
        muscles[which].rotate1 = acos(1.f);
    muscles[which].rotate1 *= 360.0 / 6.2831853;
    if (p1.x > p2.x)
        muscles[which].rotate1 = 360 - muscles[which].rotate1;
    if (!isnormal(muscles[which].rotate1))
        muscles[which].rotate1 = 0;
    if (!isnormal(muscles[which].rotate2))
        muscles[which].rotate2 = 0;

    const int label1 = muscles[which].parent1->label;
    const int label2 = muscles[which].parent2->label;
    switch (label1) {
    case head:
        fwd = specialforward[0];
        break;
    case rightshoulder:
    case rightelbow:
    case rightwrist:
    case righthand:
        fwd = specialforward[1];
        break;
    case leftshoulder:
    case leftelbow:
    case leftwrist:
    case lefthand:
        fwd = specialforward[2];
        break;
    case righthip:
    case rightknee:
    case rightankle:
    case rightfoot:
        fwd = specialforward[3];
        break;
    case lefthip:
    case leftknee:
    case leftankle:
    case leftfoot:
        fwd = specialforward[4];
        break;
    default:
        if (muscles[which].parent1->lower)
            fwd = lowforward;
        else
            fwd = forward;
        break;
    }

    if (animation == hanganim) {
        if (label1 == righthand || label2 == righthand) {
            fwd = 0;
            fwd.x = -1;
        }
        if (label1 == lefthand || label2 == lefthand) {
            fwd = 0;
            fwd.x = 1;
        }
    }

    if (free == 0) {
        if (label1 == rightfoot || label2 == rightfoot) {
            fwd.y -= .3;
        }
        if (label1 == leftfoot || label2 == leftfoot) {
            fwd.y -= .3;
        }
    }

    fwd = DoRotation(fwd, 0, muscles[which].rotate1 - 90, 0);
    fwd = DoRotation(fwd, 0, 0, muscles[which].rotate2 - 90);
    fwd.y = 0;
    fwd /= findLength(&fwd);
    if (fwd.z <= 1 && fwd.z >= -1)
        muscles[which].rotate3 = acos(0 - fwd.z);
    else
        muscles[which].rotate3 = acos(-1.f);
    muscles[which].rotate3 *= 360.0 / 6.2831853;
    if (0 > fwd.x)
        muscles[which].rotate3 = 360 - muscles[which].rotate3;
    if (!isnormal(muscles[which].rotate3))
        muscles[which].rotate3 = 0;
}

/* EFFECT
 * load skeleton
 * takes filenames for three skeleton files and various models
 */
void Skeleton::Load(const std::string& filename,       const std::string& lowfilename, const std::string& clothesfilename,
                    const std::string& modelfilename,  const std::string& model2filename,
                    const std::string& model3filename, const std::string& model4filename,
                    const std::string& model5filename, const std::string& model6filename,
                    const std::string& model7filename, const std::string& modellowfilename,
                    const std::string& modelclothesfilename, bool clothes)
{
    GLfloat M[16];
    int parentID;
    FILE *tfile;
    float lSize;
    int i, j;
    int edit;

    LOGFUNC;

    num_models = 7;

    // load various models
    // rotate, scale, do normals, do texcoords for each as needed

    model[0].loadnotex(modelfilename);
    model[1].loadnotex(model2filename);
    model[2].loadnotex(model3filename);
    model[3].loadnotex(model4filename);
    model[4].loadnotex(model5filename);
    model[5].loadnotex(model6filename);
    model[6].loadnotex(model7filename);

    for (i = 0; i < num_models; i++) {
        model[i].Rotate(180, 0, 0);
        model[i].Scale(.04, .04, .04);
        model[i].CalculateNormals(0);
    }

    drawmodel.load(modelfilename, 0);
    drawmodel.Rotate(180, 0, 0);
    drawmodel.Scale(.04, .04, .04);
    drawmodel.FlipTexCoords();
    if (tutoriallevel == 1 && id != 0)
        drawmodel.UniformTexCoords();
    if (tutoriallevel == 1 && id != 0)
        drawmodel.ScaleTexCoords(0.1);
    drawmodel.CalculateNormals(0);

    modellow.loadnotex(modellowfilename);
    modellow.Rotate(180, 0, 0);
    modellow.Scale(.04, .04, .04);
    modellow.CalculateNormals(0);

    drawmodellow.load(modellowfilename, 0);
    drawmodellow.Rotate(180, 0, 0);
    drawmodellow.Scale(.04, .04, .04);
    drawmodellow.FlipTexCoords();
    if (tutoriallevel == 1 && id != 0)
        drawmodellow.UniformTexCoords();
    if (tutoriallevel == 1 && id != 0)
        drawmodellow.ScaleTexCoords(0.1);
    drawmodellow.CalculateNormals(0);

    if (clothes) {
        modelclothes.loadnotex(modelclothesfilename);
        modelclothes.Rotate(180, 0, 0);
        modelclothes.Scale(.041, .04, .041);
        modelclothes.CalculateNormals(0);

        drawmodelclothes.load(modelclothesfilename, 0);
        drawmodelclothes.Rotate(180, 0, 0);
        drawmodelclothes.Scale(.04, .04, .04);
        drawmodelclothes.FlipTexCoords();
        drawmodelclothes.CalculateNormals(0);
    }

    // FIXME: three similar blocks follow, one for each of:
    // filename, lowfilename, clothesfilename

    // load skeleton

    tfile = Folders::openMandatoryFile( Folders::getResourcePath(filename), "rb" );

    // read num_joints
    funpackf(tfile, "Bi", &num_joints);

    // allocate memory
    if (joints)
        delete [] joints; //dealloc2(joints);
    joints = (Joint*)new Joint[num_joints];

    // read info for each joint
    for (i = 0; i < num_joints; i++) {
        funpackf(tfile, "Bf Bf Bf Bf Bf", &joints[i].position.x, &joints[i].position.y, &joints[i].position.z, &joints[i].length, &joints[i].mass);
        funpackf(tfile, "Bb Bb", &joints[i].hasparent, &joints[i].locked);
        funpackf(tfile, "Bi", &joints[i].modelnum);
        funpackf(tfile, "Bb Bb", &joints[i].visible, &joints[i].sametwist);
        funpackf(tfile, "Bi Bi", &joints[i].label, &joints[i].hasgun);
        funpackf(tfile, "Bb", &joints[i].lower);
        funpackf(tfile, "Bi", &parentID);
        if (joints[i].hasparent)
            joints[i].parent = &joints[parentID];
        joints[i].velocity = 0;
        joints[i].oldposition = joints[i].position;
    }

    // read num_muscles
    funpackf(tfile, "Bi", &num_muscles);

    // allocate memory
    if (muscles)
        delete [] muscles; //dealloc2(muscles);
    muscles = (Muscle*)new Muscle[num_muscles]; //malloc(sizeof(Muscle)*num_muscles);

    // for each muscle...
    for (i = 0; i < num_muscles; i++) {
        // read info
        funpackf(tfile, "Bf Bf Bf Bf Bf Bi Bi", &muscles[i].length, &muscles[i].targetlength, &muscles[i].minlength, &muscles[i].maxlength, &muscles[i].strength, &muscles[i].type, &muscles[i].numvertices);

        // allocate memory for vertices
        muscles[i].vertices = (int*)malloc(sizeof(int) * muscles[i].numvertices);

        // read vertices
        edit = 0;
        for (j = 0; j < muscles[i].numvertices - edit; j++) {
            funpackf(tfile, "Bi", &muscles[i].vertices[j + edit]);
            if (muscles[i].vertices[j + edit] >= model[0].vertexNum) {
                muscles[i].numvertices--;
                edit--;
            }
        }

        // read more info
        funpackf(tfile, "Bb Bi", &muscles[i].visible, &parentID);
        muscles[i].parent1 = &joints[parentID];
        funpackf(tfile, "Bi", &parentID);
        muscles[i].parent2 = &joints[parentID];
    }

    // read forwardjoints (?)
    for (j = 0; j < 3; j++) {
        funpackf(tfile, "Bi", &forwardjoints[j]);
    }
    // read lowforwardjoints (?)
    for (j = 0; j < 3; j++) {
        funpackf(tfile, "Bi", &lowforwardjoints[j]);
    }

    // ???
    for (j = 0; j < num_muscles; j++) {
        for (i = 0; i < muscles[j].numvertices; i++) {
            for (int k = 0; k < num_models; k++) {
                if (muscles[j].numvertices && muscles[j].vertices[i] < model[k].vertexNum)
                    model[k].owner[muscles[j].vertices[i]] = j;
            }
        }
    }

    // calculate some stuff
    FindForwards();
    for (i = 0; i < num_joints; i++) {
        joints[i].startpos = joints[i].position;
    }
    for (i = 0; i < num_muscles; i++) {
        FindRotationMuscle(i, -1);
    }
    // this seems to use opengl purely for matrix calculations
    for (int k = 0; k < num_models; k++) {
        for (i = 0; i < model[k].vertexNum; i++) {
            model[k].vertex[i] = model[k].vertex[i] - (muscles[model[k].owner[i]].parent1->position + muscles[model[k].owner[i]].parent2->position) / 2;
            glMatrixMode(GL_MODELVIEW);
            glPushMatrix();
            glLoadIdentity();
            glRotatef(muscles[model[k].owner[i]].rotate3, 0, 1, 0);
            glRotatef(muscles[model[k].owner[i]].rotate2 - 90, 0, 0, 1);
            glRotatef(muscles[model[k].owner[i]].rotate1 - 90, 0, 1, 0);
            glTranslatef(model[k].vertex[i].x, model[k].vertex[i].y, model[k].vertex[i].z);
            glGetFloatv(GL_MODELVIEW_MATRIX, M);
            model[k].vertex[i].x = M[12] * 1;
            model[k].vertex[i].y = M[13] * 1;
            model[k].vertex[i].z = M[14] * 1;
            glPopMatrix();
        }
        model[k].CalculateNormals(0);
    }
    fclose(tfile);

    // load ???

    tfile = Folders::openMandatoryFile( Folders::getResourcePath(lowfilename), "rb" );

    // skip joints section

    lSize = sizeof(num_joints);
    fseek(tfile, lSize, SEEK_CUR);
    for (i = 0; i < num_joints; i++) {
        // skip joint info
        lSize = sizeof(XYZ)
                + sizeof(float)
                + sizeof(float)
                + 1 //sizeof(bool)
                + 1 //sizeof(bool)
                + sizeof(int)
                + 1 //sizeof(bool)
                + 1 //sizeof(bool)
                + sizeof(int)
                + sizeof(int)
                + 1 //sizeof(bool)
                + sizeof(int);
        fseek(tfile, lSize, SEEK_CUR);

        if (joints[i].hasparent)
            joints[i].parent = &joints[parentID];
        joints[i].velocity = 0;
        joints[i].oldposition = joints[i].position;
    }

    // read num_muscles
    funpackf(tfile, "Bi", &num_muscles);

    for (i = 0; i < num_muscles; i++) {
        // skip muscle info
        lSize = sizeof(float)
                + sizeof(float)
                + sizeof(float)
                + sizeof(float)
                + sizeof(float)
                + sizeof(int);
        fseek(tfile, lSize, SEEK_CUR);

        // read numverticeslow
        funpackf(tfile, "Bi", &muscles[i].numverticeslow);

        if (muscles[i].numverticeslow) {
            // allocate memory
            muscles[i].verticeslow = (int*)malloc(sizeof(int) * muscles[i].numverticeslow);

            // read verticeslow
            edit = 0;
            for (j = 0; j < muscles[i].numverticeslow - edit; j++) {
                funpackf(tfile, "Bi", &muscles[i].verticeslow[j + edit]);
                if (muscles[i].verticeslow[j + edit] >= modellow.vertexNum) {
                    muscles[i].numverticeslow--;
                    edit--;
                }
            }
        }

        // skip more stuff
        lSize = 1; //sizeof(bool);
        fseek ( tfile, lSize, SEEK_CUR);
        lSize = sizeof(int);
        fseek ( tfile, lSize, SEEK_CUR);
        fseek ( tfile, lSize, SEEK_CUR);
    }

    for (j = 0; j < num_muscles; j++) {
        for (i = 0; i < muscles[j].numverticeslow; i++) {
            if (muscles[j].verticeslow[i] < modellow.vertexNum)
                modellow.owner[muscles[j].verticeslow[i]] = j;
        }
    }

    // use opengl for its matrix math
    for (i = 0; i < modellow.vertexNum; i++) {
        modellow.vertex[i] = modellow.vertex[i] - (muscles[modellow.owner[i]].parent1->position + muscles[modellow.owner[i]].parent2->position) / 2;
        glMatrixMode(GL_MODELVIEW);
        glPushMatrix();
        glLoadIdentity();
        glRotatef(muscles[modellow.owner[i]].rotate3, 0, 1, 0);
        glRotatef(muscles[modellow.owner[i]].rotate2 - 90, 0, 0, 1);
        glRotatef(muscles[modellow.owner[i]].rotate1 - 90, 0, 1, 0);
        glTranslatef(modellow.vertex[i].x, modellow.vertex[i].y, modellow.vertex[i].z);
        glGetFloatv(GL_MODELVIEW_MATRIX, M);
        modellow.vertex[i].x = M[12];
        modellow.vertex[i].y = M[13];
        modellow.vertex[i].z = M[14];
        glPopMatrix();
    }

    modellow.CalculateNormals(0);

    // load clothes

    if (clothes) {
        tfile = Folders::openMandatoryFile( Folders::getResourcePath(clothesfilename), "rb" );

        // skip num_joints
        lSize = sizeof(num_joints);
        fseek ( tfile, lSize, SEEK_CUR);

        for (i = 0; i < num_joints; i++) {
            // skip joint info
            lSize = sizeof(XYZ)
                    + sizeof(float)
                    + sizeof(float)
                    + 1 //sizeof(bool)
                    + 1 //sizeof(bool)
                    + sizeof(int)
                    + 1 //sizeof(bool)
                    + 1 //sizeof(bool)
                    + sizeof(int)
                    + sizeof(int)
                    + 1 //sizeof(bool)
                    + sizeof(int);
            fseek(tfile, lSize, SEEK_CUR);

            if (joints[i].hasparent)
                joints[i].parent = &joints[parentID];
            joints[i].velocity = 0;
            joints[i].oldposition = joints[i].position;
        }

        // read num_muscles
        funpackf(tfile, "Bi", &num_muscles);

        for (i = 0; i < num_muscles; i++) {
            // skip muscle info
            lSize = sizeof(float)
                    + sizeof(float)
                    + sizeof(float)
                    + sizeof(float)
                    + sizeof(float)
                    + sizeof(int);
            fseek(tfile, lSize, SEEK_CUR);

            // read numverticesclothes
            funpackf(tfile, "Bi", &muscles[i].numverticesclothes);

            // read verticesclothes
            if (muscles[i].numverticesclothes) {
                muscles[i].verticesclothes = (int*)malloc(sizeof(int) * muscles[i].numverticesclothes);
                edit = 0;
                for (j = 0; j < muscles[i].numverticesclothes - edit; j++) {
                    funpackf(tfile, "Bi", &muscles[i].verticesclothes[j + edit]);
                    if (muscles[i].verticesclothes[j + edit] >= modelclothes.vertexNum) {
                        muscles[i].numverticesclothes--;
                        edit--;
                    }
                }
            }

            // skip more stuff
            lSize = 1; //sizeof(bool);
            fseek ( tfile, lSize, SEEK_CUR);
            lSize = sizeof(int);
            fseek ( tfile, lSize, SEEK_CUR);
            fseek ( tfile, lSize, SEEK_CUR);
        }

        // ???
        lSize = sizeof(int);
        for (j = 0; j < num_muscles; j++) {
            for (i = 0; i < muscles[j].numverticesclothes; i++) {
                if (muscles[j].numverticesclothes && muscles[j].verticesclothes[i] < modelclothes.vertexNum)
                    modelclothes.owner[muscles[j].verticesclothes[i]] = j;
            }
        }

        // use opengl for its matrix math
        for (i = 0; i < modelclothes.vertexNum; i++) {
            modelclothes.vertex[i] = modelclothes.vertex[i] - (muscles[modelclothes.owner[i]].parent1->position + muscles[modelclothes.owner[i]].parent2->position) / 2;
            glMatrixMode(GL_MODELVIEW);
            glPushMatrix();
            glLoadIdentity();
            glRotatef(muscles[modelclothes.owner[i]].rotate3, 0, 1, 0);
            glRotatef(muscles[modelclothes.owner[i]].rotate2 - 90, 0, 0, 1);
            glRotatef(muscles[modelclothes.owner[i]].rotate1 - 90, 0, 1, 0);
            glTranslatef(modelclothes.vertex[i].x, modelclothes.vertex[i].y, modelclothes.vertex[i].z);
            glGetFloatv(GL_MODELVIEW_MATRIX, M);
            modelclothes.vertex[i].x = M[12];
            modelclothes.vertex[i].y = M[13];
            modelclothes.vertex[i].z = M[14];
            glPopMatrix();
        }

        modelclothes.CalculateNormals(0);
    }
    fclose(tfile);

    for (i = 0; i < num_joints; i++) {
        for (j = 0; j < num_joints; j++) {
            if (joints[i].label == j)
                jointlabels[j] = i;
        }
    }

    free = 0;
}

Skeleton::Skeleton()
{
    num_joints = 0;

    num_muscles = 0;

    selected = 0;

    memset(forwardjoints, 0, sizeof(forwardjoints));
    // XYZ forward;

    id = 0;

    memset(lowforwardjoints, 0, sizeof(lowforwardjoints));
    // XYZ lowforward;

    // XYZ specialforward[5];
    memset(jointlabels, 0, sizeof(jointlabels));

    // Model model[7];
    // Model modellow;
    // Model modelclothes;
    num_models = 0;

    // Model drawmodel;
    // Model drawmodellow;
    // Model drawmodelclothes;

    clothes = 0;
    spinny = 0;

    memset(skinText, 0, sizeof(skinText));
    skinsize = 0;

    checkdelay = 0;

    longdead = 0;
    broken = 0;

    free = 0;
    oldfree = 0;
    freetime = 0;
    freefall = 0;

    joints = 0;
    muscles = 0;
}

Skeleton::~Skeleton()
{
    if (muscles) {
        delete [] muscles;
    }
    muscles = 0;

    if (joints) {
        delete [] joints;
    }
    joints = 0;
}

#if 0

// the following functions are not used anywhere

/* EFFECT
 * sets forward, lowforward, specialforward[]
 *
 * USES:
 * NONE
 */
void Skeleton::FindForwardsfirst()
{
    //Find forward vectors
    CrossProduct(joints[forwardjoints[1]].position - joints[forwardjoints[0]].position, joints[forwardjoints[2]].position - joints[forwardjoints[0]].position, &forward);
    Normalise(&forward);

    CrossProduct(joints[lowforwardjoints[1]].position - joints[lowforwardjoints[0]].position, joints[lowforwardjoints[2]].position - joints[lowforwardjoints[0]].position, &lowforward);
    Normalise(&lowforward);

    //Special forwards
    specialforward[0] = forward;
    specialforward[1] = forward;
    specialforward[2] = forward;
    specialforward[3] = forward;
    specialforward[4] = forward;

}

/* EFFECT
 *
 * USES:
 * NONE
 */
void Skeleton::Draw(int muscleview)
{
    static float jointcolor[4];

    if (muscleview == 2) {
        jointcolor[0] = 0;
        jointcolor[1] = 0;
        jointcolor[2] = 0;
        jointcolor[3] = .5;
    } else {
        jointcolor[0] = 0;
        jointcolor[1] = 0;
        jointcolor[2] = .5;
        jointcolor[3] = 1;
    }

    //Calc motionblur-ness
    for (int i = 0; i < num_joints; i++) {
        joints[i].oldposition = joints[i].position;
        joints[i].blurred = findDistance(&joints[i].position, &joints[i].oldposition) * 100;
        if (joints[i].blurred < 1)
            joints[i].blurred = 1;
    }

    //Do Motionblur
    glDepthMask(0);
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    glBegin(GL_QUADS);
    for (int i = 0; i < num_joints; i++) {
        if (joints[i].hasparent) {
            glColor4f(jointcolor[0], jointcolor[1], jointcolor[2], jointcolor[3] / joints[i].blurred);
            glVertex3f(joints[i].position.x, joints[i].position.y, joints[i].position.z);
            glColor4f(jointcolor[0], jointcolor[1], jointcolor[2], jointcolor[3] / joints[i].parent->blurred);
            glVertex3f(joints[i].parent->position.x, joints[i].parent->position.y, joints[i].parent->position.z);
            glColor4f(jointcolor[0], jointcolor[1], jointcolor[2], jointcolor[3] / joints[i].parent->blurred);
            glVertex3f(joints[i].parent->oldposition.x, joints[i].parent->oldposition.y, joints[i].parent->oldposition.z);
            glColor4f(jointcolor[0], jointcolor[1], jointcolor[2], jointcolor[3] / joints[i].blurred);
            glVertex3f(joints[i].oldposition.x, joints[i].oldposition.y, joints[i].oldposition.z);
        }
    }
    for (int i = 0; i < num_muscles; i++) {
        if (muscles[i].type == boneconnect) {
            glColor4f(jointcolor[0], jointcolor[1], jointcolor[2], jointcolor[3] / muscles[i].parent2->blurred);
            glVertex3f(muscles[i].parent1->position.x, muscles[i].parent1->position.y, muscles[i].parent1->position.z);
            glColor4f(jointcolor[0], jointcolor[1], jointcolor[2], jointcolor[3] / muscles[i].parent2->blurred);
            glVertex3f(muscles[i].parent2->position.x, muscles[i].parent2->position.y, muscles[i].parent2->position.z);
            glColor4f(jointcolor[0], jointcolor[1], jointcolor[2], jointcolor[3] / muscles[i].parent2->blurred);
            glVertex3f(muscles[i].parent2->oldposition.x, muscles[i].parent2->oldposition.y, muscles[i].parent2->oldposition.z);
            glColor4f(jointcolor[0], jointcolor[1], jointcolor[2], jointcolor[3] / muscles[i].parent1->blurred);
            glVertex3f(muscles[i].parent1->oldposition.x, muscles[i].parent1->oldposition.y, muscles[i].parent1->oldposition.z);
        }
    }
    glEnd();

    glBegin(GL_LINES);
    for (int i = 0; i < num_joints; i++) {
        if (joints[i].hasparent) {
            glColor4f(jointcolor[0], jointcolor[1], jointcolor[2], jointcolor[3] / joints[i].blurred);
            glVertex3f(joints[i].position.x, joints[i].position.y, joints[i].position.z);
            glColor4f(jointcolor[0], jointcolor[1], jointcolor[2], jointcolor[3] / joints[i].parent->blurred);
            glVertex3f(joints[i].parent->position.x, joints[i].parent->position.y, joints[i].parent->position.z);
        }
    }
    for (int i = 0; i < num_muscles; i++) {
        if (muscles[i].type == boneconnect) {
            glColor4f(jointcolor[0], jointcolor[1], jointcolor[2], jointcolor[3] / muscles[i].parent1->blurred);
            glVertex3f(muscles[i].parent1->position.x, muscles[i].parent1->position.y, muscles[i].parent1->position.z);
            glColor4f(jointcolor[0], jointcolor[1], jointcolor[2], jointcolor[3] / muscles[i].parent2->blurred);
            glVertex3f(muscles[i].parent2->position.x, muscles[i].parent2->position.y, muscles[i].parent2->position.z);
        }
    }
    glColor3f(.6, .6, 0);
    if (muscleview == 1)
        for (int i = 0; i < num_muscles; i++) {
            if (muscles[i].type != boneconnect) {
                glVertex3f(muscles[i].parent1->position.x, muscles[i].parent1->position.y, muscles[i].parent1->position.z);
                glVertex3f(muscles[i].parent2->position.x, muscles[i].parent2->position.y, muscles[i].parent2->position.z);
            }
        }
    glEnd();

    if (muscleview != 2) {
        glPointSize(3);
        glBegin(GL_POINTS);
        for (int i = 0; i < num_joints; i++) {
            if (i != selected)
                glColor4f(0, 0, .5, 1);
            if (i == selected)
                glColor4f(1, 1, 0, 1);
            if (joints[i].locked && i != selected)
                glColor4f(1, 0, 0, 1);
            glVertex3f(joints[i].position.x, joints[i].position.y, joints[i].position.z);
        }
        glEnd();
    }

    //Set old position to current position
    if (muscleview == 2)
        for (int i = 0; i < num_joints; i++) {
            joints[i].oldposition = joints[i].position;
        }
    glDepthMask(1);
}

/* EFFECT
 *
 * USES:
 * NONE
 */
void Skeleton::AddJoint(float x, float y, float z, int which)
{
    if (num_joints < max_joints - 1) {
        joints[num_joints].velocity = 0;
        joints[num_joints].position.x = x;
        joints[num_joints].position.y = y;
        joints[num_joints].position.z = z;
        joints[num_joints].mass = 1;
        joints[num_joints].locked = 0;

        joints[num_joints].hasparent = 0;
        num_joints++;
        if (which < num_joints && which >= 0)
            AddMuscle(num_joints - 1, which, 0, 10, boneconnect);
    }
}

/* EFFECT
 *
 * USES:
 * NONE
 */
void Skeleton::DeleteJoint(int whichjoint)
{
    if (whichjoint < num_joints && whichjoint >= 0) {
        joints[whichjoint].velocity = joints[num_joints - 1].velocity;
        joints[whichjoint].position = joints[num_joints - 1].position;
        joints[whichjoint].oldposition = joints[num_joints - 1].oldposition;
        joints[whichjoint].hasparent = joints[num_joints - 1].hasparent;
        joints[whichjoint].parent = joints[num_joints - 1].parent;
        joints[whichjoint].length = joints[num_joints - 1].length;
        joints[whichjoint].locked = joints[num_joints - 1].locked;
        joints[whichjoint].modelnum = joints[num_joints - 1].modelnum;
        joints[whichjoint].visible = joints[num_joints - 1].visible;

        for (int i = 0; i < num_muscles; i++) {
            while (muscles[i].parent1 == &joints[whichjoint] && i < num_muscles)DeleteMuscle(i);
            while (muscles[i].parent2 == &joints[whichjoint] && i < num_muscles)DeleteMuscle(i);
        }
        for (int i = 0; i < num_muscles; i++) {
            while (muscles[i].parent1 == &joints[num_joints - 1] && i < num_muscles)muscles[i].parent1 = &joints[whichjoint];
            while (muscles[i].parent2 == &joints[num_joints - 1] && i < num_muscles)muscles[i].parent2 = &joints[whichjoint];
        }
        for (int i = 0; i < num_joints; i++) {
            if (joints[i].parent == &joints[whichjoint])
                joints[i].hasparent = 0;
        }
        for (int i = 0; i < num_joints; i++) {
            if (joints[i].parent == &joints[num_joints - 1])
                joints[i].parent = &joints[whichjoint];
        }

        num_joints--;
    }
}

/* EFFECT
 *
 * USES:
 * Skeleton::DeleteJoint - UNUSED
 */
void Skeleton::DeleteMuscle(int whichmuscle)
{
    if (whichmuscle < num_muscles) {
        muscles[whichmuscle].minlength = muscles[num_muscles - 1].minlength;
        muscles[whichmuscle].maxlength = muscles[num_muscles - 1].maxlength;
        muscles[whichmuscle].strength = muscles[num_muscles - 1].strength;
        muscles[whichmuscle].parent1 = muscles[num_muscles - 1].parent1;
        muscles[whichmuscle].parent2 = muscles[num_muscles - 1].parent2;
        muscles[whichmuscle].length = muscles[num_muscles - 1].length;
        muscles[whichmuscle].visible = muscles[num_muscles - 1].visible;
        muscles[whichmuscle].type = muscles[num_muscles - 1].type;
        muscles[whichmuscle].targetlength = muscles[num_muscles - 1].targetlength;

        num_muscles--;
    }
}

/* EFFECT
 *
 * USES:
 * NONE
 */
void Skeleton::SetJoint(float x, float y, float z, int which, int whichjoint)
{
    if (whichjoint < num_joints) {
        joints[whichjoint].velocity = 0;
        joints[whichjoint].position.x = x;
        joints[whichjoint].position.y = y;
        joints[whichjoint].position.z = z;

        if (which >= num_joints || which < 0)
            joints[whichjoint].hasparent = 0;
        if (which < num_joints && which >= 0) {
            joints[whichjoint].parent = &joints[which];
            joints[whichjoint].hasparent = 1;
            joints[whichjoint].length = findDistance(&joints[whichjoint].position, &joints[whichjoint].parent->position);
        }
    }
}

/* EFFECT
 *
 * USES:
 * Skeleton::AddJoint - UNUSED
 */
void Skeleton::AddMuscle(int attach1, int attach2, float minlength, float maxlength, int type)
{
    const int max_muscles = 100; // FIXME: Probably can be dropped
    if (num_muscles < max_muscles - 1 && attach1 < num_joints && attach1 >= 0 && attach2 < num_joints && attach2 >= 0 && attach1 != attach2) {
        muscles[num_muscles].parent1 = &joints[attach1];
        muscles[num_muscles].parent2 = &joints[attach2];
        muscles[num_muscles].length = findDistance(&muscles[num_muscles].parent1->position, &muscles[num_muscles].parent2->position);
        muscles[num_muscles].targetlength = findDistance(&muscles[num_muscles].parent1->position, &muscles[num_muscles].parent2->position);
        muscles[num_muscles].strength = .7;
        muscles[num_muscles].type = type;
        muscles[num_muscles].minlength = minlength;
        muscles[num_muscles].maxlength = maxlength;

        num_muscles++;
    }
}

/* EFFECT
 *
 * USES:
 * NONE
 */
void Skeleton::MusclesSet()
{
    for (int i = 0; i < num_muscles; i++) {
        muscles[i].length = findDistance(&muscles[i].parent1->position, &muscles[i].parent2->position);
    }
}

/* EFFECT
 *
 * USES:
 * NONE
 */
void Skeleton::DoBalance()
{
    /*XYZ newpoint;
    newpoint=joints[0].position;
    newpoint.x=(joints[2].position.x+joints[4].position.x)/2;
    newpoint.z=(joints[2].position.z+joints[4].position.z)/2;
    joints[0].velocity=joints[0].velocity+(newpoint-joints[0].position);
    //Move child point to within certain distance of parent point
    joints[0].position=newpoint;

    MusclesSet();*/
}

#endif

