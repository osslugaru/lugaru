/*
Copyright (C) 2003, 2010 - Wolfire Games
Copyright (C) 2013 - Côme Bernigaud

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

#ifndef _WOLF_H_
#define _WOLF_H_

/**> HEADER FILES <**/

#include "Person.h"

class Wolf : public Person
{
public:
    Wolf();

    int getCrouch();
    int getStop();
    int getRun();
    int getLanding();
    int getLandhard();

    void setProps(float headprop, float bodyprop, float armprop, float legprop);
    virtual void loadSkeleton();
};

#endif
