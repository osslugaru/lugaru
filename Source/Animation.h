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

#ifndef ANIMATION_H
#define	ANIMATION_H

enum anim_attack_type {
  neutral, normalattack, reversed, reversal
};

enum anim_height_type {
  lowheight, middleheight, highheight
};


enum animation_types {
#define DECLARE_ANIM(id, ...) id,
#include "Animation.def"
#undef DECLARE_ANIM
animation_count
};

extern Animation animation[animation_count];

extern void loadAllAnimations();
#endif
