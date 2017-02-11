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

#include "Math/XYZ.hpp"

#include <string>
#include <vector>

extern bool campaign;

extern int actuallevel;
extern std::string campaignEndText[3];

std::vector<std::string> ListCampaigns();
void LoadCampaign();

class CampaignLevel
{
private:
    int width;
    struct Position
    {
        int x, y;
    };

public:
    std::string mapname;
    std::string description;
    int choosenext;
    /*
    0 = Immediately load next level at the end of this one.
    1 = Go back to the world map.
    2 = Don't bring up the Fiery loading screen. Maybe other things, I've not investigated.
    */
    //int numnext; // 0 on final level. As David said: he meant to add story branching, but he eventually hadn't.
    std::vector<int> nextlevel;
    Position location;
    CampaignLevel();
    int getStartX();
    int getStartY();
    int getEndX();
    int getEndY();
    XYZ getCenter();
    int getWidth();
    std::istream& operator<<(std::istream& is);
    friend std::istream& operator>>(std::istream& is, CampaignLevel& cl)
    {
        return cl << is;
    }
};

extern std::vector<CampaignLevel> campaignlevels;
