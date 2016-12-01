/*
Copyright (C) 2003, 2010 - Wolfire Games
Copyright (C) 2010-2016 - Lugaru contributors (see AUTHORS file)

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

#include "Folders.h"
#include <cstring>

const std::string Folders::dataDir = DATADIR;

std::string Folders::getScreenshotDir()
{
    /* TODO - create folder if missing */
    return getUserDataPath() + "/Screenshots";
}

std::string Folders::getResourcePath(std::string filepath)
{
    return dataDir + '/' + filepath;
}

std::string Folders::getUserDataPath()
{
    return getGenericDirectory("XDG_DATA_HOME", ".local/share");
}

std::string Folders::getConfigFilePath()
{
    return getGenericDirectory("XDG_CONFIG_HOME", ".config") + "/config.txt";
}

/* Generic code for XDG ENVVAR test and fallback */
std::string Folders::getGenericDirectory(const char* ENVVAR, const std::string fallback) {
    char* path = getenv(ENVVAR);
    std::string ret;
    if((path != NULL) && (strlen(path) != 0)) {
        ret = std::string(path) + "/lugaru";
    } else {
        path = getenv("HOME");
        if((path != NULL) && (strlen(path) != 0)) {
            ret = std::string(path) + '/' + fallback + "/lugaru";
        } else {
            ret = "";
        }
    }
    return ret;
}
