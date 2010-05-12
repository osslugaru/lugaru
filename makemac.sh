#!/bin/sh

set -e
set -x

NCPU=`sysctl -n hw.ncpu`

for arch in ppc i386 x86_64 ; do
    make -f makefile.old macosx=true macosx_arch=$arch clean
    make -f makefile.old macosx=true macosx_arch=$arch -j$NCPU
    # We always strip here. For debugging, you should do "make" directly.
    strip run/lugaru-$arch
    BINS="$BINS run/lugaru-$arch"
done

rm -f run/lugaru-bin
lipo -create -o run/lugaru-bin $BINS

