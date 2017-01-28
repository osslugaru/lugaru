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

struct BinIOPackContext {
    uint8_t *buffer;
    va_list  args;
};

static void BinIOPack(void *context, int type, int byte_order, int count)
{
    struct BinIOPackContext *ctx = (struct BinIOPackContext*)context;
    if (count == -1) {
        switch (type) {
        case BinIO_TYPE_IGNORE_BYTE: {
            ctx->buffer += 1;
        }
        break;
        case BinIO_TYPE_BYTE: {
            uint8_t value = va_arg(ctx->args, int);
            BinIOConvert1(BinIO_HOST_BYTE_ORDER, byte_order, (const uint8_t *)&value, ctx->buffer, 1);
            ctx->buffer += 1;
        }
        break;
        case BinIO_TYPE_INT16: {
            uint16_t value = va_arg(ctx->args, int);
            BinIOConvert2(BinIO_HOST_BYTE_ORDER, byte_order, (const uint8_t *)&value, ctx->buffer, 1);
            ctx->buffer += 2;
        }
        break;
        case BinIO_TYPE_INT32: {
            int value = va_arg(ctx->args, int);
            BinIOConvert4(BinIO_HOST_BYTE_ORDER, byte_order, (const uint8_t *)&value, ctx->buffer, 1);
            ctx->buffer += 4;
        }
        break;
        case BinIO_TYPE_INT64: {
            uint64_t value = va_arg(ctx->args, uint64_t);
            BinIOConvert8(BinIO_HOST_BYTE_ORDER, byte_order, (const uint8_t *)&value, ctx->buffer, 1);
            ctx->buffer += 8;
        }
        break;
        case BinIO_TYPE_FLOAT32: {
            float32_t value = (float32_t)va_arg(ctx->args, double);
            BinIOConvert4(BinIO_HOST_BYTE_ORDER, byte_order, (const uint8_t *)&value, ctx->buffer, 1);
            ctx->buffer += 4;
        }
        break;
        case BinIO_TYPE_FLOAT64: {
            float64_t value = va_arg(ctx->args, float64_t);
            BinIOConvert8(BinIO_HOST_BYTE_ORDER, byte_order, (const uint8_t *)&value, ctx->buffer, 1);
            ctx->buffer += 8;
        }
        break;
        }
    } else {
        switch (type) {
        case BinIO_TYPE_IGNORE_BYTE:
            ctx->buffer += 1 * count;
            break;
        case BinIO_TYPE_BYTE:
            BinIOConvert1(BinIO_HOST_BYTE_ORDER, byte_order, va_arg(ctx->args, uint8_t *), ctx->buffer, count);
            ctx->buffer += 1 * count;
            break;
        case BinIO_TYPE_INT16:
            BinIOConvert2(BinIO_HOST_BYTE_ORDER, byte_order, va_arg(ctx->args, uint8_t *), ctx->buffer, count);
            ctx->buffer += 2 * count;
            break;
        case BinIO_TYPE_INT32:
            BinIOConvert4(BinIO_HOST_BYTE_ORDER, byte_order, va_arg(ctx->args, uint8_t *), ctx->buffer, count);
            ctx->buffer += 4 * count;
            break;
        case BinIO_TYPE_INT64:
            BinIOConvert8(BinIO_HOST_BYTE_ORDER, byte_order, va_arg(ctx->args, uint8_t *), ctx->buffer, count);
            ctx->buffer += 8 * count;
            break;
        case BinIO_TYPE_FLOAT32:
            BinIOConvert4(BinIO_HOST_BYTE_ORDER, byte_order, va_arg(ctx->args, uint8_t *), ctx->buffer, count);
            ctx->buffer += 4 * count;
            break;
        case BinIO_TYPE_FLOAT64:
            BinIOConvert8(BinIO_HOST_BYTE_ORDER, byte_order, va_arg(ctx->args, uint8_t *), ctx->buffer, count);
            ctx->buffer += 8 * count;
            break;
        }
    }
}

extern void packf(const char *format, ...)
{
    va_list args;
    va_start(args, format);
    vfpackf(stdout, format, args);
    va_end(args);
}

extern void spackf(void *buffer, const char *format, ...)
{
    va_list args;
    va_start(args, format);
    vspackf(buffer, format, args);
    va_end(args);
}

extern void fpackf(FILE *file, const char *format, ...)
{
    va_list args;
    va_start(args, format);
    vfpackf(file, format, args);
    va_end(args);
}

extern void vspackf(void *buffer, const char *format, va_list args)
{
    struct BinIOFormatCursor cursor;
    struct BinIOPackContext context;

    BinIOInitFormatCursor(&cursor, format);

    context.buffer = (unsigned char *)buffer;
    va_copy(context.args, args);

    while (BinIONextChar(&context, &cursor, BinIOPack)) {}

    va_end(context.args);
}

extern void vfpackf(FILE *file, const char *format, va_list args)
{
    size_t n_bytes = BinIOFormatByteCount(format);
    void* buffer = malloc(n_bytes);

    vspackf(buffer, format, args);

    fwrite(buffer, n_bytes, 1, file);
    free(buffer);
}
