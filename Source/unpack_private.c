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

#include "unpack_private.h"

extern void BinIOUnpack(void *context, int type, int byte_order, int count)
{
    struct BinIOUnpackContext *ctx = (struct BinIOUnpackContext*)context;
    if (count == -1)
    {
        count = 1;
    }

    switch (type)
    {
        case BinIO_TYPE_IGNORE_BYTE:                                                                                                   ctx->data += 1 * count; break;
        case BinIO_TYPE_BYTE:        BinIOConvert1(byte_order, BinIO_HOST_BYTE_ORDER, ctx->data, va_arg(ctx->args, uint8_t *), count); ctx->data += 1 * count; break;
        case BinIO_TYPE_INT16:       BinIOConvert2(byte_order, BinIO_HOST_BYTE_ORDER, ctx->data, va_arg(ctx->args, uint8_t *), count); ctx->data += 2 * count; break;
        case BinIO_TYPE_INT32:       BinIOConvert4(byte_order, BinIO_HOST_BYTE_ORDER, ctx->data, va_arg(ctx->args, uint8_t *), count); ctx->data += 4 * count; break;
        case BinIO_TYPE_INT64:       BinIOConvert8(byte_order, BinIO_HOST_BYTE_ORDER, ctx->data, va_arg(ctx->args, uint8_t *), count); ctx->data += 8 * count; break;
        case BinIO_TYPE_FLOAT32:     BinIOConvert4(byte_order, BinIO_HOST_BYTE_ORDER, ctx->data, va_arg(ctx->args, uint8_t *), count); ctx->data += 4 * count; break;
        case BinIO_TYPE_FLOAT64:     BinIOConvert8(byte_order, BinIO_HOST_BYTE_ORDER, ctx->data, va_arg(ctx->args, uint8_t *), count); ctx->data += 8 * count; break;
    }
}
