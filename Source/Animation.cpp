/*
Copyright (C) 2010 - Lugaru authors

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

#include "Skeleton.h"
#include "Animation.h"

struct animation_data_elt {
    const char *filename;
    int height;
    int attack;
};

static animation_data_elt animation_data[animation_count] = {
#define DECLARE_ANIM(id, file, height, attack, ...) {file, height, attack},
#include "Animation.def"
#undef DECLARE_ANIM
};

void loadAllAnimations()
{
    for (int i = 0; i < loadable_anim_end; i++) {
        animation_data_elt *e = animation_data + i;
        animation[i].Load(e->filename, e->height, e->attack);
    }
}
