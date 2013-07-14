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
#include "Rabbit.h"

Rabbit::Rabbit()
{
    creature = rabbittype;
}

/* FUNCTION
 * crouch animation for this creature
 */
int Rabbit::getCrouch()
{
    return crouchanim;
}

/* FUNCTION
 * running animation for this creature (can be upright or all fours)
 */
int Rabbit::getRun()
{
    if (superruntoggle && weaponactive == -1)
        return rabbitrunninganim;
    else
        return runanim;
}

/* FUNCTION
 */
int Rabbit::getStop()
{
    return stopanim;
}

/* FUNCTION
 */
int Rabbit::getLanding()
{
    return landanim;
}

/* FUNCTION
 */
int Rabbit::getLandhard()
{
    return landhardanim;
}

void Rabbit::setProps(float headprop, float bodyprop, float armprop, float legprop)
{
    proportionhead = 1.20 * headprop;
    proportionbody = 1.05 * bodyprop;
    proportionarms = 1.00 * armprop;
    proportionlegs = 1.10 * legprop;
    proportionlegs.y = 1.05 * legprop;
}

void Rabbit::loadSkeleton()
{
    skeleton.Load(
        (char *)":Data:Skeleton:Basic Figure",
        (char *)":Data:Skeleton:Basic Figurelow",
        (char *)":Data:Skeleton:Rabbitbelt",
        (char *)":Data:Models:Body.solid",
        (char *)":Data:Models:Body2.solid",
        (char *)":Data:Models:Body3.solid",
        (char *)":Data:Models:Body4.solid",
        (char *)":Data:Models:Body5.solid",
        (char *)":Data:Models:Body6.solid",
        (char *)":Data:Models:Body7.solid",
        (char *)":Data:Models:Bodylow.solid",
        (char *)":Data:Models:Belt.solid", 0);
}
