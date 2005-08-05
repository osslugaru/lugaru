#ifndef unpack_private_h
#define unpack_private_h

#include "private.h"

struct BinIOUnpackContext
{
    const uint8_t *data;
    va_list        args;
};

extern void BinIOUnpack(void *context, int type, int byte_order, int count);

#endif

