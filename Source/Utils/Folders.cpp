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
#include <sys/stat.h>

const std::string Folders::dataDir = DATADIR;

std::string Folders::getScreenshotDir()
{
    std::string screenshotDir = getUserDataPath() + "/Screenshots";
    makeDirectory(screenshotDir);
    return screenshotDir;
}

std::string Folders::getResourcePath(std::string filepath)
{
    return dataDir + '/' + filepath;
}

std::string Folders::getUserDataPath()
{
    std::string userDataPath = getGenericDirectory("XDG_DATA_HOME", ".local/share");
    makeDirectory(userDataPath);
    return userDataPath;
}

std::string Folders::getConfigFilePath()
{
    std::string configFolder = getGenericDirectory("XDG_CONFIG_HOME", ".config");
    makeDirectory(configFolder);
    return configFolder + "/config.txt";
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
            ret = ".";
        }
    }
    return ret;
}

bool Folders::makeDirectory(std::string path) {
    errno = 0;
    int status = mkdir(path.c_str(), S_IRWXU | S_IRWXG | S_IROTH | S_IXOTH);
    if (status == 0) {
        return true;
    } else if(errno == EEXIST) {
        return true;
    } else {
        return false;
    }
}
