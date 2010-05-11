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


#include "Random.h"
#include "stdlib.h"


int randSeed = 1;

short Random()
{
	/*unsigned int value = (unsigned int)randSeed;
	unsigned int out=0;
	int i=31;

	while (i > 0)
	{
	value ^= 0x81010000;
	value ^= out;
	if (value & 1)
	{
	value >>= 1;
	value |= 0x80000000;
	}
	else
	value >>= 1;
	out <<= 1;
	out |= (value&0x1);
	i--;
	}
	if (value&0x80000000)
	{
	out <<= 1;
	out |= 1;
	}
	else
	out <<= 1;

	randSeed += out;
	*/
	return rand();
}
