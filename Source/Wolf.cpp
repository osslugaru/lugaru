/*
Copyright (C) 2003, 2010 - Wolfire Games

This file is part of Lugaru.

Lugaru is free software; you can redistribute it and/or
modify it under the terms of the GNU General Public License
as published by the Free Software Foundation; either version 2
of the License, or (at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.

See the GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program; if not, write to the Free Software
Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
*/

/**> HEADER FILES <**/
#include "Wolf.h"

Wolf::Wolf()
{
    creature = wolftype;
}

/* FUNCTION
 * crouch animation for this creature
 */
int Wolf::getCrouch()
{
    return wolfcrouchanim;
}

/* FUNCTION
 * running animation for this creature (can be upright or all fours)
 */
int Wolf::getRun()
{
    if (superruntoggle)
        return wolfrunninganim;
    else
        return wolfrunanim;
}

/* FUNCTION
 */
int Wolf::getStop()
{
    return wolfstopanim;
}

/* FUNCTION
 */
int Wolf::getLanding()
{
    return wolflandanim;
}

/* FUNCTION
 */
int Wolf::getLandhard()
{
    return wolflandhardanim;
}

void Wolf::setProps(float headprop, float bodyprop, float armprop, float legprop)
{
    proportionhead = 1.1 * headprop;
    proportionbody = 1.1 * bodyprop;
    proportionarms = 1.1 * armprop;
    proportionlegs = 1.1 * legprop;
}

void Wolf::loadSkeleton()
{
    skeleton.Load(
        (char *)":Data:Skeleton:Basic Figure Wolf",
        (char *)":Data:Skeleton:Basic Figure Wolf Low",
        (char *)":Data:Skeleton:Rabbitbelt",
        (char *)":Data:Models:Wolf.solid",
        (char *)":Data:Models:Wolf2.solid",
        (char *)":Data:Models:Wolf3.solid",
        (char *)":Data:Models:Wolf4.solid",
        (char *)":Data:Models:Wolf5.solid",
        (char *)":Data:Models:Wolf6.solid",
        (char *)":Data:Models:Wolf7.solid",
        (char *)":Data:Models:Wolflow.solid",
        (char *)":Data:Models:Belt.solid", 0);
}
