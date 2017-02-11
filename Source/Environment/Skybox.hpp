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

#ifndef _SKYBOX_HPP_
#define _SKYBOX_HPP_

#include "Graphic/Texture.hpp"
#include "Graphic/gamegl.hpp"
#include "Math/XYZ.hpp"
#include "Math/XYZ.hpp"
#include "Utils/ImageIO.hpp"

class SkyBox
{
public:
    Texture front, left, back, right, up, down;

    void load(const std::string& ffront, const std::string& fleft, const std::string& fback,
              const std::string& fright, const std::string& fup, const std::string& fdown);
    void draw();

    SkyBox() {}
};

#endif
