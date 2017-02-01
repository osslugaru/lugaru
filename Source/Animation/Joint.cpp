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

#include "Animation/Joint.hpp"

#include "Utils/binio.h"

Joint::Joint()
    : blurred(0)
    , length(0)
    , mass(0)
    , lower(false)
    , hasparent(false)
    , locked(false)
    , modelnum(0)
    , visible(false)
    , parent(nullptr)
    , sametwist(false)
    , label(head)
    , hasgun(0)
    , delay(0)
{
}

void Joint::load(FILE* tfile, std::vector<Joint>& joints)
{
    int parentID;

    funpackf(tfile, "Bf Bf Bf Bf Bf", &position.x, &position.y, &position.z, &length, &mass);
    funpackf(tfile, "Bb Bb", &hasparent, &locked);
    funpackf(tfile, "Bi", &modelnum);
    funpackf(tfile, "Bb Bb", &visible, &sametwist);
    funpackf(tfile, "Bi Bi", &label, &hasgun);
    funpackf(tfile, "Bb", &lower);
    funpackf(tfile, "Bi", &parentID);
    if (hasparent) {
        parent = &joints[parentID];
    } else {
        parent = nullptr;
    }
    velocity = 0;
    oldposition = position;
    startpos = position;
}
