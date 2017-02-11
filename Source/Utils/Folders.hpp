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

#ifndef _FOLDERS_HPP_
#define _FOLDERS_HPP_

#include <string>

#ifndef DATA_DIR
#define DATA_DIR "Data"
#endif

struct FileNotFoundException : public std::exception
{
    std::string errorText;

    FileNotFoundException(const std::string& filename)
        : errorText(filename + " could not be found")
    {
    }

    const char* what() const throw()
    {
        return errorText.c_str();
    }
};

class Folders
{
    static const std::string dataDir;

public:
    /** Returns path to the screenshot directory. Creates it if needed. */
    static std::string getScreenshotDir();

    /** Returns full path for user data */
    static std::string getUserDataPath();

    /** Returns full path for config file */
    static std::string getConfigFilePath();

    static FILE* openMandatoryFile(const std::string& filename, const char* mode);

    static bool file_exists(const std::string& filepath);

    /* Returns full path for a game resource */
    static inline std::string getResourcePath(const std::string& filepath)
    {
        return dataDir + '/' + filepath;
    }

    /** Returns full path for user progress save */
    static inline std::string getUserSavePath()
    {
        return getUserDataPath() + "/users";
    }

    static bool makeDirectory(const std::string& path);

private:
    static const char* getHomeDirectory();
    static std::string getGenericDirectory(const char* ENVVAR, const std::string& fallback);
};

#endif /* _FOLDERS_H_ */
