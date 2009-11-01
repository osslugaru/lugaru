#!/bin/sh

set -e
set -x

NCPU=`sysctl -n hw.ncpu`

for arch in ppc i386 x86_64 ; do
    make macosx=true macosx_arch=$arch clean
    make macosx=true macosx_arch=$arch -j$NCPU
    BINS="$BINS run/lugaru-$arch"
done

rm -f run/lugaru-bin
lipo -create -o run/lugaru-bin $BINS

