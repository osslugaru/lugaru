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

#ifndef binio_h
#define binio_h

#include <stdarg.h>
#include <stdio.h>

#if defined(__cplusplus)
extern "C" {
#endif

    /*
    Notes on format of format strings:
    * whitespace is ignored
    * each "group" consists of an optional count (defaults to 1),
    an optional byte-order marker (defaults to H, "host-native"),
    and a  data-type specifier.
    * when unpacking, each variable argument is a pointer to the
    appropriate number of Object::objects of the appropriate type.
    * when packing, each variable argument is an object of the
    appropriate type if the count is omitted, or a pointer to the
    appropriate number of Object::objects of the appropriate type if the
    count is specified.
    * the buffer supplied to pack/unpack must be of sufficient length
    to hold all the data, or the behavior is unspecified.
    * the file provided to the "f" versions of the functions must be
    open in the appropriate mode, or the behavior is unspecified.
    * the file supplied to funpackf must be of sufficient length to
    hold all the data, or the behavior is unspecified.
    * the behavior of all functions is unspecified if the format string
    is incorrectly-formed.

    Data-type specifiers:
    x skipped byte; no corresponding argument
    b byte
    s two-byte two's-complement integer
    i four-byte two's-complement integer
    l eight-byte two's-complement integer
    f four-byte IEEE754 float
    d eight-byte IEEE754 double

    Byte-order specifiers:
    L little-endian
    B big-endian
    H host's native byte order
    N network byte order
    */

#ifndef ALREADY_DID_BINIO_STDINT
#define ALREADY_DID_BINIO_STDINT
#if defined(BinIO_STDINT_HEADER)
#include BinIO_STDINT_HEADER
    typedef float              float32_t;
    typedef double             float64_t;
#else
    typedef unsigned char      uint8_t;
    typedef unsigned short     uint16_t;
    typedef unsigned long      uint32_t;
#ifdef WIN32
    typedef unsigned __int64   uint64_t;
#else
    typedef unsigned long long uint64_t;
#endif
    typedef float              float32_t;
    typedef double             float64_t;
#endif
#endif

    typedef struct {
        float64_t d;
        uint64_t  l;
        int  i;
        float32_t f;
        uint16_t  s;
        uint8_t   b;
    }
    test_data;

    extern void packf    (                    const char *format, ...);
    extern void spackf   (void *buffer,       const char *format, ...);
    extern void fpackf   (FILE *file,         const char *format, ...);
    extern void vspackf  (void *buffer,       const char *format, va_list args);
    extern void vfpackf  (FILE *file,         const char *format, va_list args);

    extern void unpackf  (                    const char *format, ...);
    extern void sunpackf (const void *buffer, const char *format, ...);
    extern void funpackf (FILE       *file,   const char *format, ...);
    extern void vsunpackf(const void *buffer, const char *format, va_list args);
    extern void vfunpackf(FILE       *file,   const char *format, va_list args);

#ifdef _MSC_VER
#ifndef va_copy
#define va_copy(dest,src) do { dest = src; } while (0)
#endif
#endif

#if defined(__cplusplus)
}
#endif

#endif

