#ifndef pack_private_h
#define pack_private_h

#include "private.h"

struct BinIOPackContext
{
    uint8_t *buffer;
    va_list  args;
};

extern void BinIOPack(void *context, int type, int byte_order, int count);

#endif
