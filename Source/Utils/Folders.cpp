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
#include <unistd.h>
#if PLATFORM_UNIX
#include <sys/stat.h>
#include <sys/types.h>
#include <pwd.h>
#endif
#if _WIN32
#include <windows.h>
#endif

const std::string Folders::dataDir = DATA_DIR;

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
#ifdef _WIN32
    return dataDir;
#else
    std::string userDataPath;
#if (defined(__APPLE__) && defined(__MACH__))
    const char* homePath = getHomeDirectory();
    if (homePath == NULL) {
        userDataPath = ".";
    } else {
        userDataPath = std::string(homePath) + "/Library/Application Support/Lugaru";
    }
#else
    userDataPath = getGenericDirectory("XDG_DATA_HOME", ".local/share");
#endif
    makeDirectory(userDataPath);
    return userDataPath;
#endif
}

std::string Folders::getConfigFilePath()
{
#ifdef _WIN32
    return dataDir + "/config.txt";
#else
    std::string configFolder;
#if (defined(__APPLE__) && defined(__MACH__))
    configFolder = getUserDataPath();
#else
    configFolder = getGenericDirectory("XDG_CONFIG_HOME", ".config");
#endif
    makeDirectory(configFolder);
    return configFolder + "/config.txt";
#endif
}

#if PLATFORM_LINUX
/* Generic code for XDG ENVVAR test and fallback */
std::string Folders::getGenericDirectory(const char* ENVVAR, const std::string fallback) {
    const char* path = getenv(ENVVAR);
    std::string ret;
    if ((path != NULL) && (strlen(path) != 0)) {
        ret = std::string(path) + "/lugaru";
    } else {
        const char* homedir = getHomeDirectory();
        if ((homedir != NULL) && (strlen(homedir) != 0)) {
            ret = std::string(homedir) + '/' + fallback + "/lugaru";
        } else {
            ret = ".";
        }
    }
    return ret;
}
#endif

#if PLATFORM_UNIX
const char* Folders::getHomeDirectory()
{
    const char *homedir = getenv("HOME");
    if (homedir != NULL)
        return homedir;
    struct passwd *pw = getpwuid(getuid());
    if (pw != NULL)
        return pw->pw_dir;
    return NULL;
}
#endif

bool Folders::makeDirectory(std::string path) {
#ifdef _WIN32
    int status = CreateDirectory(path.c_str(), NULL);
    if (status != 0) {
        return true;
    } else if(GetLastError() == ERROR_ALREADY_EXISTS) {
        return true;
    } else {
        return false;
    }
#else
    errno = 0;
    int status = mkdir(path.c_str(), S_IRWXU | S_IRWXG | S_IROTH | S_IXOTH);
    if (status == 0) {
        return true;
    } else if(errno == EEXIST) {
        return true;
    } else {
        return false;
    }
#endif
}

FILE* Folders::openMandatoryFile(std::string filename, const char* mode)
{
    FILE* tfile = fopen(filename.c_str(), mode);
    if (tfile == NULL) {
        throw FileNotFoundException(filename);
    }
    return tfile;
}
