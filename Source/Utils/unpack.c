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

#include "binio.h"
#include "private.h"

#include <stdlib.h>

struct BinIOUnpackContext {
    const uint8_t *data;
    va_list        args;
};

static void BinIOUnpack(void *context, int type, int byte_order, int count)
{
    struct BinIOUnpackContext *ctx = (struct BinIOUnpackContext*)context;
    if (count == -1) {
        count = 1;
    }

    switch (type) {
    case BinIO_TYPE_IGNORE_BYTE:
        ctx->data += 1 * count;
        break;
    case BinIO_TYPE_BYTE:
        BinIOConvert1(byte_order, BinIO_HOST_BYTE_ORDER, ctx->data, va_arg(ctx->args, uint8_t *), count);
        ctx->data += 1 * count;
        break;
    case BinIO_TYPE_INT16:
        BinIOConvert2(byte_order, BinIO_HOST_BYTE_ORDER, ctx->data, va_arg(ctx->args, uint8_t *), count);
        ctx->data += 2 * count;
        break;
    case BinIO_TYPE_INT32:
        BinIOConvert4(byte_order, BinIO_HOST_BYTE_ORDER, ctx->data, va_arg(ctx->args, uint8_t *), count);
        ctx->data += 4 * count;
        break;
    case BinIO_TYPE_INT64:
        BinIOConvert8(byte_order, BinIO_HOST_BYTE_ORDER, ctx->data, va_arg(ctx->args, uint8_t *), count);
        ctx->data += 8 * count;
        break;
    case BinIO_TYPE_FLOAT32:
        BinIOConvert4(byte_order, BinIO_HOST_BYTE_ORDER, ctx->data, va_arg(ctx->args, uint8_t *), count);
        ctx->data += 4 * count;
        break;
    case BinIO_TYPE_FLOAT64:
        BinIOConvert8(byte_order, BinIO_HOST_BYTE_ORDER, ctx->data, va_arg(ctx->args, uint8_t *), count);
        ctx->data += 8 * count;
        break;
    }
}

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
