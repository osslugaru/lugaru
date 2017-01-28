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

#include "Folders.hpp"

#include <cerrno>
#include <cstdlib>
#include <cstring>
#include <unistd.h>

#if PLATFORM_UNIX
#include <pwd.h>
#include <sys/stat.h>
#include <sys/types.h>
#endif

#if _WIN32
#include <shlobj.h> // to get paths related functions
#include <windows.h>
#endif

const std::string Folders::dataDir = DATA_DIR;

std::string Folders::getScreenshotDir()
{
    std::string screenshotDir = getUserDataPath() + "/Screenshots";
    makeDirectory(screenshotDir);
    return screenshotDir;
}

std::string Folders::getUserDataPath()
{
    std::string userDataPath;
#ifdef _WIN32
    char path[MAX_PATH];
    // %APPDATA% (%USERPROFILE%\Application Data)
    if (SUCCEEDED(SHGetFolderPathA(nullptr, CSIDL_APPDATA, nullptr, 0, path))) {
        userDataPath = std::string(path) + "/Lugaru/";
    } else {
        return dataDir;
    }
#elif (defined(__APPLE__) && defined(__MACH__))
    const char* homePath = getHomeDirectory();
    if (homePath == NULL) {
        userDataPath = ".";
    } else {
        userDataPath = std::string(homePath) + "/Library/Application Support/Lugaru";
    }
#else // Linux
    userDataPath = getGenericDirectory("XDG_DATA_HOME", ".local/share");
#endif
    makeDirectory(userDataPath);
    return userDataPath;
}

std::string Folders::getConfigFilePath()
{
    std::string configFolder;
#if defined(_WIN32) || (defined(__APPLE__) && defined(__MACH__))
    configFolder = getUserDataPath();
#else // Linux
    configFolder = getGenericDirectory("XDG_CONFIG_HOME", ".config");
    makeDirectory(configFolder);
#endif
    return configFolder + "/config.txt";
}

#if PLATFORM_LINUX
/* Generic code for XDG ENVVAR test and fallback */
std::string Folders::getGenericDirectory(const char* ENVVAR, const std::string& fallback)
{
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
    const char* homedir = getenv("HOME");
    if (homedir != NULL) {
        return homedir;
    }
    struct passwd* pw = getpwuid(getuid());
    if (pw != NULL) {
        return pw->pw_dir;
    }
    return NULL;
}
#endif

bool Folders::makeDirectory(const std::string& path)
{
#ifdef _WIN32
    int status = CreateDirectory(path.c_str(), NULL);
    return ((status != 0) || (GetLastError() == ERROR_ALREADY_EXISTS));
#else
    errno = 0;
    int status = mkdir(path.c_str(), S_IRWXU | S_IRWXG | S_IROTH | S_IXOTH);
    return ((status == 0) || (errno == EEXIST));
#endif
}

FILE* Folders::openMandatoryFile(const std::string& filename, const char* mode)
{
    FILE* tfile = fopen(filename.c_str(), mode);
    if (tfile == NULL) {
        throw FileNotFoundException(filename);
    }
    return tfile;
}

bool Folders::file_exists(const std::string& filepath)
{
    FILE* file;
    file = fopen(filepath.c_str(), "rb");
    if (file == NULL) {
        return false;
    } else {
        fclose(file);
        return true;
    }
}
