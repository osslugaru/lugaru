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

typedef void (*console_handler)(const char* args);

#define DECLARE_COMMAND(cmd) void ch_##cmd(const char* args);
#include "ConsoleCmds.def"
#undef DECLARE_COMMAND

/* FIXME - This is only to get cmd_count, not very clean */
enum console_command
{
#define DECLARE_COMMAND(cmd) cmd_##cmd,
#include "ConsoleCmds.def"
#undef DECLARE_COMMAND
    cmd_count
};

extern const char* cmd_names[cmd_count];

extern console_handler cmd_handlers[cmd_count];
