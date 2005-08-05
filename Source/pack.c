#include <stdlib.h>

#include "binio.h"
#include "private.h"
#include "pack_private.h"

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
    context.args = args;
    
    while (BinIONextChar(&context, &cursor, BinIOPack)) {}
}

extern void vfpackf(FILE *file, const char *format, va_list args)
{
    size_t n_bytes = BinIOFormatByteCount(format);
    void* buffer = malloc(n_bytes);
    
    vspackf(buffer, format, args);
    
    fwrite(buffer, n_bytes, 1, file);
    free(buffer);
}
