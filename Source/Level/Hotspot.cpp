/*
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

#include "Level/Hotspot.hpp"

std::vector<Hotspot> Hotspot::hotspots;
int Hotspot::current = 0;
int Hotspot::killhotspot = 0;

Hotspot::Hotspot()
    : position()
    , type(0)
    , size(0)
{
}

Hotspot::Hotspot(XYZ p, int t, float s)
    : position(p)
    , type(t)
    , size(s)
{
}
