#!/bin/sh

set -e
set -x

NCPU=`sysctl -n hw.ncpu`

for arch in ppc i386 x86_64 ; do
    make -f makefile.old macosx=true macosx_arch=$arch clean
    make -f makefile.old macosx=true macosx_arch=$arch -j$NCPU
    # We always strip here. For debugging, you should do "make" directly.
    strip Data/lugaru-$arch
    BINS="$BINS Data/lugaru-$arch"
done

rm -f Data/lugaru-bin
lipo -create -o Data/lugaru-bin $BINS

