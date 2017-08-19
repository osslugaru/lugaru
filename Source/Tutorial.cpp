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

#include "Tutorial.hpp"
#include "Audio/Sounds.hpp"
#include "Audio/openal_wrapper.hpp"
#include "Game.hpp"
#include "Level/Awards.hpp"
#include "Objects/Person.hpp"
#include "Utils/Input.hpp"

extern bool reversaltrain;
extern bool canattack;
extern bool cananger;
extern int bonus;
extern float damagedealt;
extern bool againbonus;
extern float screenwidth, screenheight;
extern int mainmenu;

bool Tutorial::active = false;
int Tutorial::stage = 0;
float Tutorial::stagetime = 0;
float Tutorial::maxtime = 0;
float Tutorial::success = 0;

void Tutorial::Do(float multiplier)
{
    if (stagetime > maxtime) {
        stage++;
        success = 0;
        if (stage <= 1) {
            canattack = 0;
            cananger = 0;
            reversaltrain = 0;
        }
        switch (stage) {
            case 1:
                maxtime = 5;
                break;
            case 2:
            case 10:
            case 13:
            case 26:
                maxtime = 2;
                break;
            case 3:
            case 5:
            case 6:
            case 7:
            case 8:
            case 9:
                maxtime = 600;
                break;
            case 4:
            case 11:
            case 12:
                maxtime = 1000;
                break;
            case 14: {
                maxtime = 3;

                XYZ temp, temp2;

                temp.x = 1011;
                temp.y = 84;
                temp.z = 491;
                temp2.x = 1025;
                temp2.y = 75;
                temp2.z = 447;

                Person::players[1]->coords = (temp + temp2) / 2;

                emit_sound_at(fireendsound, Person::players[1]->coords);

                for (unsigned i = 0; i < Person::players[1]->skeleton.joints.size(); i++) {
                    if (Random() % 2 == 0) {
                        if (!Person::players[1]->skeleton.free) {
                            temp2 = (Person::players[1]->coords - Person::players[1]->oldcoords) / multiplier / 2; //velocity/2;
                        }
                        if (Person::players[1]->skeleton.free) {
                            temp2 = Person::players[1]->skeleton.joints[i].velocity * Person::players[1]->scale / 2;
                        }
                        if (!Person::players[1]->skeleton.free) {
                            temp = DoRotation(DoRotation(DoRotation(Person::players[1]->skeleton.joints[i].position, 0, 0, Person::players[1]->tilt), Person::players[1]->tilt2, 0, 0), 0, Person::players[1]->yaw, 0) * Person::players[1]->scale + Person::players[1]->coords;
                        }
                        if (Person::players[1]->skeleton.free) {
                            temp = Person::players[1]->skeleton.joints[i].position * Person::players[1]->scale + Person::players[1]->coords;
                        }
                        Sprite::MakeSprite(breathsprite, temp, temp2, 1, 1, 1, .6 + (float)abs(Random() % 100) / 200 - .25, 1);
                    }
                }
            } break;
            case 15:
            case 16:
            case 17:
            case 18:
            case 20:
            case 22:
            case 23:
            case 24:
            case 25:
                maxtime = 500;
                break;
            case 19:
                stage = 20;
                break;
            case 21:
                maxtime = 500;
                if (bonus == cannon) {
                    bonus = Slicebonus;
                    againbonus = 1;
                } else {
                    againbonus = 0;
                }
                break;
            case 27:
                maxtime = 4;
                reversaltrain = 1;
                cananger = 1;
                Person::players[1]->aitype = attacktypecutoff;
                break;
            case 28:
            case 34:
            case 35:
                maxtime = 400;
                break;
            case 29:
                maxtime = 400;
                Person::players[0]->escapednum = 0;
                break;
            case 30:
                maxtime = 4;
                reversaltrain = 0;
                cananger = 0;
                Person::players[1]->aitype = passivetype;
                break;
            case 31:
                maxtime = 13;
                break;
            case 32:
            case 42:
                maxtime = 8;
                break;
            case 33:
                maxtime = 400;
                cananger = 1;
                canattack = 1;
                Person::players[1]->aitype = attacktypecutoff;
                break;
            case 36:
                maxtime = 2;
                reversaltrain = 0;
                cananger = 0;
                Person::players[1]->aitype = passivetype;
                break;
            case 37:
                damagedealt = 0;
                damagetaken = 0;
                maxtime = 50;
                cananger = 1;
                canattack = 1;
                Person::players[1]->aitype = attacktypecutoff;
                break;
            case 38:
                maxtime = 4;
                canattack = 0;
                cananger = 0;
                Person::players[1]->aitype = passivetype;
                break;
            case 39: {
                XYZ temp, temp2;

                temp.x = 1011;
                temp.y = 84;
                temp.z = 491;
                temp2.x = 1025;
                temp2.y = 75;
                temp2.z = 447;

                Weapon w(knife, -1);
                w.position = (temp + temp2) / 2;
                w.tippoint = (temp + temp2) / 2;

                w.velocity = 0.1;
                w.tipvelocity = 0.1;
                w.missed = 1;
                w.hitsomething = 0;
                w.freetime = 0;
                w.firstfree = 1;
                w.physics = 1;

                weapons.push_back(w);
            } break;
            case 40:
            case 41:
            case 43:
                maxtime = 300;
                break;
            case 44:
                weapons[0].owner = 1;
                Person::players[0]->weaponactive = -1;
                Person::players[0]->num_weapons = 0;
                Person::players[1]->weaponactive = 0;
                Person::players[1]->num_weapons = 1;
                Person::players[1]->weaponids[0] = 0;

                cananger = 1;
                canattack = 1;
                Person::players[1]->aitype = attacktypecutoff;

                maxtime = 300;
                break;
            case 45:
                weapons[0].owner = 1;
                Person::players[0]->weaponactive = -1;
                Person::players[0]->num_weapons = 0;
                Person::players[1]->weaponactive = 0;
                Person::players[1]->num_weapons = 1;
                Person::players[1]->weaponids[0] = 0;

                maxtime = 300;
                break;
            case 46:
                weapons[0].owner = 1;
                Person::players[0]->weaponactive = -1;
                Person::players[0]->num_weapons = 0;
                Person::players[1]->weaponactive = 0;
                Person::players[1]->num_weapons = 1;
                Person::players[1]->weaponids[0] = 0;

                weapons[0].setType(sword);

                maxtime = 300;
                break;
            case 47: {
                maxtime = 10;

                XYZ temp, temp2;

                temp.x = 1011;
                temp.y = 84;
                temp.z = 491;
                temp2.x = 1025;
                temp2.y = 75;
                temp2.z = 447;

                Weapon w(sword, -1);
                w.position = (temp + temp2) / 2;
                w.tippoint = (temp + temp2) / 2;

                w.velocity = 0.1;
                w.tipvelocity = 0.1;
                w.missed = 1;
                w.hitsomething = 0;
                w.freetime = 0;
                w.firstfree = 1;
                w.physics = 1;

                weapons.push_back(w);

                weapons[0].owner = 1;
                weapons[1].owner = 0;
                Person::players[0]->weaponactive = 0;
                Person::players[0]->num_weapons = 1;
                Person::players[0]->weaponids[0] = 1;
                Person::players[1]->weaponactive = 0;
                Person::players[1]->num_weapons = 1;
                Person::players[1]->weaponids[0] = 0;

            } break;
            case 48:
                canattack = 0;
                cananger = 0;
                Person::players[1]->aitype = passivetype;

                maxtime = 15;

                weapons[0].owner = 1;
                weapons[1].owner = 0;
                Person::players[0]->weaponactive = 0;
                Person::players[0]->num_weapons = 1;
                Person::players[0]->weaponids[0] = 1;
                Person::players[1]->weaponactive = 0;
                Person::players[1]->num_weapons = 1;
                Person::players[1]->weaponids[0] = 0;

                if (Person::players[0]->hasWeapon()) {
                    weapons[Person::players[0]->weaponids[Person::players[0]->weaponactive]].setType(staff);
                } else {
                    weapons[0].setType(staff);
                }
                break;
            case 49:
                canattack = 0;
                cananger = 0;
                Person::players[1]->aitype = passivetype;

                maxtime = 200;

                weapons[1].position = 1000;
                weapons[1].tippoint = 1000;

                weapons[0].setType(knife);

                weapons[0].owner = 0;
                Person::players[1]->weaponactive = -1;
                Person::players[1]->num_weapons = 0;
                Person::players[0]->weaponactive = 0;
                Person::players[0]->num_weapons = 1;
                Person::players[0]->weaponids[0] = 0;

                break;
            case 50: {
                maxtime = 8;

                XYZ temp, temp2;
                emit_sound_at(fireendsound, Person::players[1]->coords);

                for (unsigned i = 0; i < Person::players[1]->skeleton.joints.size(); i++) {
                    if (Random() % 2 == 0) {
                        if (!Person::players[1]->skeleton.free) {
                            temp2 = (Person::players[1]->coords - Person::players[1]->oldcoords) / multiplier / 2; //velocity/2;
                        }
                        if (Person::players[1]->skeleton.free) {
                            temp2 = Person::players[1]->skeleton.joints[i].velocity * Person::players[1]->scale / 2;
                        }
                        if (!Person::players[1]->skeleton.free) {
                            temp = DoRotation(DoRotation(DoRotation(Person::players[1]->skeleton.joints[i].position, 0, 0, Person::players[1]->tilt), Person::players[1]->tilt2, 0, 0), 0, Person::players[1]->yaw, 0) * Person::players[1]->scale + Person::players[1]->coords;
                        }
                        if (Person::players[1]->skeleton.free) {
                            temp = Person::players[1]->skeleton.joints[i].position * Person::players[1]->scale + Person::players[1]->coords;
                        }
                        Sprite::MakeSprite(breathsprite, temp, temp2, 1, 1, 1, .6 + (float)abs(Random() % 100) / 200 - .25, 1);
                    }
                }

                Person::players[1]->num_weapons = 0;
                Person::players[1]->weaponstuck = -1;
                Person::players[1]->weaponactive = -1;

                weapons.clear();
            } break;
            case 51:
                maxtime = 80000;
                break;
            default:
                break;
        }
        if (stage <= 51) {
            stagetime = 0;
        }
    }

    //Tutorial success
    if (stagetime < maxtime - 3) {
        switch (stage) {
            case 3:
                if (Game::deltah || Game::deltav) {
                    success += multiplier;
                }
                break;
            case 4:
                if (Person::players[0]->forwardkeydown || Person::players[0]->backkeydown || Person::players[0]->leftkeydown || Person::players[0]->rightkeydown) {
                    success += multiplier;
                }
                break;
            case 5:
                if (Person::players[0]->jumpkeydown) {
                    success = 1;
                }
                break;
            case 6:
                if (Person::players[0]->isCrouch()) {
                    success = 1;
                }
                break;
            case 7:
                if (Person::players[0]->animTarget == rollanim) {
                    success = 1;
                }
                break;
            case 8:
                if (Person::players[0]->animTarget == sneakanim) {
                    success += multiplier;
                }
                break;
            case 9:
                if (Person::players[0]->animTarget == rabbitrunninganim || Person::players[0]->animTarget == wolfrunninganim) {
                    success += multiplier;
                }
                break;
            case 11:
                if (Person::players[0]->isWallJump()) {
                    success = 1;
                }
                break;
            case 12:
                if (Person::players[0]->animTarget == flipanim) {
                    success = 1;
                }
                break;
            case 15:
                if (Person::players[0]->animTarget == upunchanim || Person::players[0]->animTarget == winduppunchanim) {
                    success = 1;
                }
                break;
            case 16:
                if (Person::players[0]->animTarget == winduppunchanim) {
                    success = 1;
                }
                break;
            case 17:
                if (Person::players[0]->animTarget == spinkickanim) {
                    success = 1;
                }
                break;
            case 18:
                if (Person::players[0]->animTarget == sweepanim) {
                    success = 1;
                }
                break;
            case 19:
                if (Person::players[0]->animTarget == dropkickanim) {
                    success = 1;
                }
                break;
            case 20:
                if (Person::players[0]->animTarget == rabbitkickanim) {
                    success = 1;
                }
                break;
            case 21:
                if (bonus == cannon) {
                    success = 1;
                }
                break;
            case 22:
                if (bonus == spinecrusher) {
                    success = 1;
                }
                break;
            case 23:
                if (Person::players[0]->animTarget == walljumprightkickanim || Person::players[0]->animTarget == walljumpleftkickanim) {
                    success = 1;
                }
                break;
            case 24:
                if (Person::players[0]->animTarget == rabbittacklinganim) {
                    success = 1;
                }
                break;
            case 25:
                if (Person::players[0]->animTarget == backhandspringanim) {
                    success = 1;
                }
                break;
            case 28:
                if (Animation::animations[Person::players[0]->animTarget].attack == reversed && Person::players[0]->feint) {
                    success = 1;
                }
                break;
            case 29:
                if (Person::players[0]->escapednum == 2) {
                    success = 1;
                    reversaltrain = 0;
                    cananger = 0;
                    Person::players[1]->aitype = passivetype;
                }
                break;
            case 33:
            case 34:
            case 44:
            case 45:
            case 46:
                if (Animation::animations[Person::players[0]->animTarget].attack == reversal) {
                    success = 1;
                }
                break;
            case 35:
                if (Animation::animations[Person::players[0]->animTarget].attack == reversal) {
                    success = 1;
                    reversaltrain = 0;
                    cananger = 0;
                    Person::players[1]->aitype = passivetype;
                }
                break;
            case 40:
                if (Person::players[0]->num_weapons > 0) {
                    success = 1;
                }
                break;
            case 41:
                if (!Person::players[0]->hasWeapon() && Person::players[0]->num_weapons > 0) {
                    success = 1;
                }
                break;
            case 43:
                if (Person::players[0]->animTarget == knifeslashstartanim) {
                    success = 1;
                }
                break;
            case 49:
                if (Person::players[1]->weaponstuck != -1) {
                    success = 1;
                }
                break;
            default:
                break;
        }
        if (success >= 1) {
            stagetime = maxtime - 3;
        }

        if (stagetime == maxtime - 3) {
            emit_sound_np(consolesuccesssound);
        }

        if (success >= 1) {
            if (stage == 34 || stage == 35) {
                stagetime = maxtime - 1;
            }
        }
    }

    if (stage < 14 || stage >= 50) {
        Person::players[1]->coords.y = 300;
        Person::players[1]->velocity = 0;
    }
}

void Tutorial::DrawTextInfo()
{
    std::string string = " ";
    std::string string2 = " ";
    std::string string3 = " ";

    switch (stage) {
        case 0:
        default:
            break;
        case 1:
            string = "Welcome to the Lugaru training level!";
            break;
        case 2:
            string = "BASIC MOVEMENT:";
            break;
        case 3:
            string = "You can move the mouse to rotate the camera.";
            break;
        case 4:
            string = std::string("Try using the ") + Input::keyToChar(Game::forwardkey);
            string += std::string(", ") + Input::keyToChar(Game::leftkey);
            string += std::string(", ") + Input::keyToChar(Game::backkey);
            string += std::string(" and ") + Input::keyToChar(Game::rightkey) + " keys to move around.";
            string2 = "All movement is relative to the camera.";
            break;
        case 5:
            string = std::string("Please press ") + Input::keyToChar(Game::jumpkey) + " to jump.";
            string2 = "You can hold it longer to jump higher.";
            break;
        case 6:
            string = std::string("You can press ") + Input::keyToChar(Game::crouchkey) + " to crouch.";
            string2 = "You can jump higher from a crouching position.";
            break;
        case 7:
            string = std::string("While running, you can press ") + Input::keyToChar(Game::crouchkey) + " to roll.";
            break;
        case 8:
            string = "While crouching, you can sneak around silently";
            string2 = "using the movement keys.";
            break;
        case 9:
            string = "Release the crouch key while sneaking and hold the movement keys";
            string2 = "to run animal-style.";
            break;
        case 10:
            string = "ADVANCED MOVEMENT:";
            break;
        case 11:
            string = std::string("When you jump at a wall, you can hold ") + Input::keyToChar(Game::jumpkey) + " again";
            string2 = "during impact to perform a walljump.";
            string3 = "Be sure to use the movement keys to press against the wall";
            break;
        case 12:
            string = "While in the air, you can press crouch to flip.";
            string2 = "Walljumps and flips confuse enemies and give you more control.";
            break;
        case 13:
            string = "BASIC COMBAT:";
            break;
        case 14:
            string = "There is now an imaginary enemy";
            string2 = "in the middle of the training area.";
            break;
        case 15:
            string = std::string("Press ") + Input::keyToChar(Game::attackkey) + " to attack when you are near an enemy.";
            string2 = "You can punch by standing still near an enemy and attacking.";
            break;
        case 16:
            string = "If you are close, you will perform a weak punch.";
            string2 = "The weak punch is excellent for starting attack combinations.";
            break;
        case 17:
            string = "Attacking while running results in a spin kick.";
            string2 = "This is one of your most powerful ground attacks.";
            break;
        case 18:
            string = "Sweep the enemy's legs out by attacking while crouched.";
            string2 = "This is a very fast attack, and easy to follow up.";
            break;
        case 19:
            string = "When an enemy is on the ground, you can deal some extra";
            string2 = "damage by running up and drop-kicking him.";
            string3 = "(Try knocking them down with a sweep first)";
            break;
        case 20:
            string = "Your most powerful individual attack is the rabbit kick.";
            string2 = std::string("Run at the enemy while holding ") + Input::keyToChar(Game::attackkey) + ", and press";
            string3 = std::string("the jump key (") + Input::keyToChar(Game::jumpkey) + ") to attack.";
            break;
        case 21:
            string = "This attack is devastating if timed correctly.";
            string2 = "Even if timed incorrectly, it will knock the enemy over.";
            if (againbonus) {
                string3 = "Try rabbit-kicking the imaginary enemy again.";
            } else {
                string3 = "Try rabbit-kicking the imaginary enemy.";
            }
            break;
        case 22:
            string = "If you sneak behind an enemy unnoticed, you can kill";
            string2 = "him instantly. Move close behind this enemy";
            string3 = "and attack.";
            break;
        case 23:
            string = "Another important attack is the wall kick. When an enemy";
            string2 = "is near a wall, perform a walljump nearby and hold";
            string3 = "the attack key during impact with the wall.";
            break;
        case 24:
            string = "You can tackle enemies by running at them animal-style";
            string2 = std::string("and pressing jump (") + Input::keyToChar(Game::jumpkey) + ") or attack (" + Input::keyToChar(Game::attackkey) + ").";
            string3 = "This is especially useful when they are running away.";
            break;
        case 25:
            string = "Dodge by pressing back and attack. Dodging is essential";
            string2 = "against enemies with swords or other long weapons.";
            break;
        case 26:
            string = "REVERSALS AND COUNTER-REVERSALS";
            break;
        case 27:
            string = "The enemy can now reverse your attacks.";
            break;
        case 28:
            string = "If you attack, you will notice that the enemy now sometimes";
            string2 = "catches your attack and uses it against you. Hold";
            string3 = std::string("crouch (") + Input::keyToChar(Game::crouchkey) + ") after attacking to escape from reversals.";
            break;
        case 29:
            string = "Try escaping from two more reversals in a row.";
            break;
        case 30:
            string = "Good!";
            break;
        case 31:
            string = std::string("To reverse an attack, you must tap crouch (") + Input::keyToChar(Game::crouchkey) + ") during the";
            string2 = "enemy's attack. You must also be close to the enemy;";
            string3 = "this is especially important against armed opponents.";
            break;
        case 32:
            string = "The enemy can attack in " + to_string(int(maxtime - stagetime)) + " seconds.";
            string2 = "This imaginary opponents attacks will be highlighted";
            string3 = "to make this easier.";
            break;
        case 33:
            string = "Reverse three enemy attacks!";
            break;
        case 34:
            string = "Reverse two more enemy attacks!";
            break;
        case 35:
            string = "Reverse one more enemy attack!";
            break;
        case 36:
            string = "Excellent!";
            break;
        case 37:
            string = "Now spar with the enemy for " + to_string(int(maxtime - stagetime)) + " more seconds.";
            string2 = "Damage dealt: " + to_string(int(damagedealt));
            string3 = "Damage taken: " + to_string(int(damagetaken));
            break;
        case 38:
            string = "WEAPONS:";
            break;
        case 39:
            string = "There is now an imaginary knife";
            string2 = "in the center of the training area.";
            break;
        case 40:
            string = "Stand, roll or handspring over the knife";
            string2 = std::string("while pressing ") + Input::keyToChar(Game::throwkey) + " to pick it up.";
            string3 = "You can crouch and press the same key to drop it again.";
            break;
        case 41:
            string = std::string("You can equip and unequip weapons using the ") + Input::keyToChar(Game::drawkey) + " key.";
            string2 = "Sometimes it is best to keep them unequipped to";
            string3 = "prevent enemies from taking them. ";
            break;
        case 42:
            string = "The knife is the smallest weapon and the least encumbering.";
            string2 = "You can equip or unequip it while standing, crouching,";
            string3 = "running or flipping.";
            break;
        case 43:
            string = "You perform weapon attacks the same way as unarmed attacks,";
            string2 = "but sharp weapons cause permanent damage, instead of the";
            string3 = "temporary trauma from blunt weapons, fists and feet.";
            break;
        case 44:
            string = "The enemy now has your knife!";
            string2 = "Please reverse two of his knife attacks.";
            break;
        case 45:
            string = "Please reverse one more of his knife attacks.";
            break;
        case 46:
            string = "Now he has a sword!";
            string2 = "The sword has longer reach than your arms, so you";
            string3 = "must move close to reverse the sword slash.";
            break;
        case 47:
            string = "Long weapons like the sword and staff are also useful for defense;";
            string2 = "you can parry enemy weapon attacks by pressing the attack key";
            string3 = "at the right time. Please try parrying the enemy's attacks!";
            break;
        case 48:
            string = "The staff is like the sword, but has two main attacks.";
            string2 = "The standing smash is fast and effective, and the running";
            string3 = "spin smash is slower and more powerful.";
            break;
        case 49:
            string = std::string("When facing an enemy, you can throw the knife with ") + Input::keyToChar(Game::throwkey) + ".";
            string2 = "It is possible to throw the knife while flipping,";
            string3 = "but it is very inaccurate.";
            break;
        case 50:
            string = "You now know everything you can learn from training.";
            string2 = "Everything else you must learn from experience!";
            break;
        case 51:
            string = "Walk out of the training area to return to the main menu.";
            break;
    }

    float opacity = maxtime - stagetime;

    if (opacity > 1) {
        opacity = 1;
    }
    if (opacity < 0) {
        opacity = 0;
    }

    Game::text->glPrintOutlined(1, 1, 1, opacity, screenwidth / 2 - 7.6 * string.size() * screenwidth / 1024, screenheight / 16 + screenheight * 4 / 5, string, 1, 1.5 * screenwidth / 1024, screenwidth, screenheight);
    Game::text->glPrintOutlined(1, 1, 1, opacity, screenwidth / 2 - 7.6 * string2.size() * screenwidth / 1024, screenheight / 16 + screenheight * 4 / 5 - 20 * screenwidth / 1024, string2, 1, 1.5 * screenwidth / 1024, screenwidth, screenheight);
    Game::text->glPrintOutlined(1, 1, 1, opacity, screenwidth / 2 - 7.6 * string3.size() * screenwidth / 1024, screenheight / 16 + screenheight * 4 / 5 - 40 * screenwidth / 1024, string3, 1, 1.5 * screenwidth / 1024, screenwidth, screenheight);

    string = "Press 'tab' to skip to the next item.";
    string2 = "Press escape at any time to";
    string3 = "pause or exit the tutorial.";

    Game::text->glPrintOutlined(0.5, 0.5, 0.5, 1, screenwidth / 2 - 7.6 * string.size() * screenwidth / 1024 * .8, 0 + screenheight * 1 / 10, string, 1, 1.5 * screenwidth / 1024 * .8, screenwidth, screenheight);
    Game::text->glPrintOutlined(0.5, 0.5, 0.5, 1, screenwidth / 2 - 7.6 * string2.size() * screenwidth / 1024 * .8, 0 + screenheight * 1 / 10 - 20 * .8 * screenwidth / 1024, string2, 1, 1.5 * screenwidth / 1024 * .8, screenwidth, screenheight);
    Game::text->glPrintOutlined(0.5, 0.5, 0.5, 1, screenwidth / 2 - 7.6 * string3.size() * screenwidth / 1024 * .8, 0 + screenheight * 1 / 10 - 40 * .8 * screenwidth / 1024, string3, 1, 1.5 * screenwidth / 1024 * .8, screenwidth, screenheight);
}

void Tutorial::DoStuff(float multiplier)
{
    XYZ temp;
    XYZ temp2;
    XYZ temp3;
    XYZ oldtemp;
    XYZ oldtemp2;
    temp.x = 1011;
    temp.y = 84;
    temp.z = 491;
    temp2.x = 1025;
    temp2.y = 75;
    temp2.z = 447;
    temp3.x = 1038;
    temp3.y = 76;
    temp3.z = 453;
    oldtemp = temp;
    oldtemp2 = temp2;
    if (stage >= 51) {
        if (distsq(&temp, &Person::players[0]->coords) >= distsq(&temp, &temp2) - 1 || distsq(&temp3, &Person::players[0]->coords) < 4) {
            OPENAL_StopSound(OPENAL_ALL); // hack...OpenAL renderer isn't stopping music after tutorial goes to level menu...
            OPENAL_SetFrequency(OPENAL_ALL);

            emit_stream_np(stream_menutheme);

            Game::gameon = 0;
            mainmenu = 5;

            Game::fireSound();

            Game::flash();
        }
    } else {
        if (distsq(&temp, &Person::players[0]->coords) >= distsq(&temp, &temp2) - 1 || distsq(&temp3, &Person::players[0]->coords) < 4) {
            emit_sound_at(fireendsound, Person::players[0]->coords);

            Person::players[0]->coords = (oldtemp + oldtemp2) / 2;

            Game::flash();
        }
    }
    if (stage >= 14 && stage < 50) {
        if (distsq(&temp, &Person::players[1]->coords) >= distsq(&temp, &temp2) - 1 || distsq(&temp3, &Person::players[1]->coords) < 4) {
            emit_sound_at(fireendsound, Person::players[1]->coords);

            for (unsigned i = 0; i < Person::players[1]->skeleton.joints.size(); i++) {
                if (Random() % 2 == 0) {
                    if (!Person::players[1]->skeleton.free) {
                        temp2 = (Person::players[1]->coords - Person::players[1]->oldcoords) / multiplier / 2; //velocity/2;
                    }
                    if (Person::players[1]->skeleton.free) {
                        temp2 = Person::players[1]->skeleton.joints[i].velocity * Person::players[1]->scale / 2;
                    }
                    if (!Person::players[1]->skeleton.free) {
                        temp = DoRotation(DoRotation(DoRotation(Person::players[1]->skeleton.joints[i].position, 0, 0, Person::players[1]->tilt), Person::players[1]->tilt2, 0, 0), 0, Person::players[1]->yaw, 0) * Person::players[1]->scale + Person::players[1]->coords;
                    }
                    if (Person::players[1]->skeleton.free) {
                        temp = Person::players[1]->skeleton.joints[i].position * Person::players[1]->scale + Person::players[1]->coords;
                    }
                    Sprite::MakeSprite(breathsprite, temp, temp2, 1, 1, 1, .6 + (float)abs(Random() % 100) / 200 - .25, 1);
                }
            }

            Person::players[1]->coords = (oldtemp + oldtemp2) / 2;
            for (unsigned i = 0; i < Person::players[1]->skeleton.joints.size(); i++) {
                Person::players[1]->skeleton.joints[i].velocity = 0;
                if (Random() % 2 == 0) {
                    if (!Person::players[1]->skeleton.free) {
                        temp2 = (Person::players[1]->coords - Person::players[1]->oldcoords) / multiplier / 2; //velocity/2;
                    }
                    if (Person::players[1]->skeleton.free) {
                        temp2 = Person::players[1]->skeleton.joints[i].velocity * Person::players[1]->scale / 2;
                    }
                    if (!Person::players[1]->skeleton.free) {
                        temp = DoRotation(DoRotation(DoRotation(Person::players[1]->skeleton.joints[i].position, 0, 0, Person::players[1]->tilt), Person::players[1]->tilt2, 0, 0), 0, Person::players[1]->yaw, 0) * Person::players[1]->scale + Person::players[1]->coords;
                    }
                    if (Person::players[1]->skeleton.free) {
                        temp = Person::players[1]->skeleton.joints[i].position * Person::players[1]->scale + Person::players[1]->coords;
                    }
                    Sprite::MakeSprite(breathsprite, temp, temp2, 1, 1, 1, .6 + (float)abs(Random() % 100) / 200 - .25, 1);
                }
            }
        }
    }
}
