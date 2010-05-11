/*
Copyright (C) 2003, 2010 - Wolfire Games

This file is part of Lugaru.

Lugaru is free software; you can redistribute it and/or
modify it under the terms of the GNU General Public License
as published by the Free Software Foundation; either version 2
of the License, or (at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  

See the GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program; if not, write to the Free Software
Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
*/

#include <stdlib.h>

#include "binio.h"
#include "private.h"
#include "unpack_private.h"

void unpackf(const char *format, ...)
{
    va_list args;
    va_start(args, format);
    vfunpackf(stdin, format, args);
    va_end(args);
}

void sunpackf(const void *buffer, const char *format, ...)
{
    va_list args;
    va_start(args, format);
    vsunpackf(buffer, format, args);
    va_end(args);
}

void funpackf(FILE *file, const char *format, ...)
{
    va_list args;
    va_start(args, format);
    vfunpackf(file, format, args);
    va_end(args);
}

void vsunpackf(const void *buffer, const char *format, va_list args)
{
    struct BinIOFormatCursor cursor;
    struct BinIOUnpackContext context;

    BinIOInitFormatCursor(&cursor, format);
    
    context.data = (const unsigned char*)buffer;
    va_copy(context.args, args);
    
    while (BinIONextChar(&context, &cursor, BinIOUnpack)) {}

    va_end(context.args);
}

void vfunpackf(FILE *file, const char *format, va_list args)
{
    size_t n_bytes = BinIOFormatByteCount(format);
    void* buffer = malloc(n_bytes);
    fread(buffer, n_bytes, 1, file);
    
    vsunpackf(buffer, format, args);
    
    free(buffer);
}
