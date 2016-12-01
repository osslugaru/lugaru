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

#ifndef _FOLDERS_H_
#define _FOLDERS_H_

#include <string>

#ifndef DATADIR
#define DATADIR "Data"
#endif

class Folders
{
    static const std::string dataDir;

public:
    /* Returns path to the screenshot directory. Creates it if needed. */
    static std::string getScreenshotDir();

    /* Returns full path for a game resource */
    static std::string getResourcePath(std::string filepath);

    /* Returns full path for user data */
    static std::string getUserDataPath();

    /* Returns full path for config file */
    static std::string getConfigFilePath();

private:
    static std::string getGenericDirectory(const char* ENVVAR, const std::string fallback);
};

#endif /* _FOLDERS_H_ */
