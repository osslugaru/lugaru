#include "pack_private.h"

void BinIOPack(void *context, int type, int byte_order, int count)
{
    struct BinIOPackContext *ctx = (struct BinIOPackContext*)context;
    if (count == -1)
    {
        switch (type)
        {
            case BinIO_TYPE_IGNORE_BYTE:
                {
                    ctx->buffer += 1;
                }
                break;
            case BinIO_TYPE_BYTE:
                {
                    uint8_t value = va_arg(ctx->args, int);
                    BinIOConvert1(BinIO_HOST_BYTE_ORDER, byte_order, (const uint8_t *)&value, ctx->buffer, 1);
                    ctx->buffer += 1;
                }
                break;
            case BinIO_TYPE_INT16:
                {
                    uint16_t value = va_arg(ctx->args, int);
                    BinIOConvert2(BinIO_HOST_BYTE_ORDER, byte_order, (const uint8_t *)&value, ctx->buffer, 1);
                    ctx->buffer += 2;
                }
                break;
            case BinIO_TYPE_INT32:
                {
                    int value = va_arg(ctx->args, int);
                    BinIOConvert4(BinIO_HOST_BYTE_ORDER, byte_order, (const uint8_t *)&value, ctx->buffer, 1);
                    ctx->buffer += 4;
                }
                break;
            case BinIO_TYPE_INT64:
                {
                    uint64_t value = va_arg(ctx->args, uint64_t);
                    BinIOConvert8(BinIO_HOST_BYTE_ORDER, byte_order, (const uint8_t *)&value, ctx->buffer, 1);
                    ctx->buffer += 8;
                }
                break;
            case BinIO_TYPE_FLOAT32:
                {
                    float32_t value = (float32_t)va_arg(ctx->args, double);
                    BinIOConvert4(BinIO_HOST_BYTE_ORDER, byte_order, (const uint8_t *)&value, ctx->buffer, 1);
                    ctx->buffer += 4;
                }
                break;
            case BinIO_TYPE_FLOAT64:
                {
                    float64_t value = va_arg(ctx->args, float64_t);
                    BinIOConvert8(BinIO_HOST_BYTE_ORDER, byte_order, (const uint8_t *)&value, ctx->buffer, 1);
                    ctx->buffer += 8;
                }
                break;
        }
    }
    else
    {
        switch (type)
        {
            case BinIO_TYPE_IGNORE_BYTE:                                                                                                     ctx->buffer += 1 * count; break;
            case BinIO_TYPE_BYTE:        BinIOConvert1(BinIO_HOST_BYTE_ORDER, byte_order, va_arg(ctx->args, uint8_t *), ctx->buffer, count); ctx->buffer += 1 * count; break;
            case BinIO_TYPE_INT16:       BinIOConvert2(BinIO_HOST_BYTE_ORDER, byte_order, va_arg(ctx->args, uint8_t *), ctx->buffer, count); ctx->buffer += 2 * count; break;
            case BinIO_TYPE_INT32:       BinIOConvert4(BinIO_HOST_BYTE_ORDER, byte_order, va_arg(ctx->args, uint8_t *), ctx->buffer, count); ctx->buffer += 4 * count; break;
            case BinIO_TYPE_INT64:       BinIOConvert8(BinIO_HOST_BYTE_ORDER, byte_order, va_arg(ctx->args, uint8_t *), ctx->buffer, count); ctx->buffer += 8 * count; break;
            case BinIO_TYPE_FLOAT32:     BinIOConvert4(BinIO_HOST_BYTE_ORDER, byte_order, va_arg(ctx->args, uint8_t *), ctx->buffer, count); ctx->buffer += 4 * count; break;
            case BinIO_TYPE_FLOAT64:     BinIOConvert8(BinIO_HOST_BYTE_ORDER, byte_order, va_arg(ctx->args, uint8_t *), ctx->buffer, count); ctx->buffer += 8 * count; break;
        }
    }
}
