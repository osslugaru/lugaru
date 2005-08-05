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
