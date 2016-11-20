# Compiling

As it stands, the version of Lugaru in this repository supports Linux, OSX
and Windows. Not all toolchains are tested, thus we would welcome help from
new contributors especially regarding MSVC and OSX support.

## Common dependencies

You will need the following development libraries and tools, usually
available via your package manager (dnf, urpmi, apt, brew, etc.):

- CMake
- SDL2
- Mesa OpenGL Utility library (GLU)
- LibJPEG (TurboJPEG)
- LibPNG
- OpenAL Soft
- Ogg, Vorbis and Vorbisfile
- Zlib

## GNU/Linux

Both GCC and Clang are supported as compilers. Define the `CC` and `CXX` env
variables according to the compiler you want to use, if not the default one.
Then build with:

```
mkdir build && cd build
cmake ..
make
```

The resulting `lugaru` binary will expect to find the `Data/` folder next to
it, so either copy `build/lugaru` in the main directory, or create a symbolic
link to run the game.

## Mac OSX

The instructions should be similar to the GNU/Linux ones, but have not been
tested in a Mac environment recently.

## Windows

As of now, only MinGW32 and MinGW64 are supported, and were only tested by
cross-compiling from Linux.

### MSVC

Help needed :)

### MinGW on Windows

Help needed :)

### Cross-compiling from Linux

Cross-compiling for Windows using MinGW32 and MinGW64 was tested on Fedora
and Mageia. The instructions may vary for other distros, do not hesitate to
send a merge request to update them if need be.

You will need to install the `mingw32-` or `mingw64-` variants of the
dependencies listed above.

#### MinGW32

First you will need to setup some environment variables:
```
export PKG_CONFIG_LIBDIR="/usr/i686-w64-mingw32/sys-root/mingw/lib/pkgconfig:/usr/i686-w64-mingw32/sys-root/mingw/share/pkgconfig"
export PATH=/usr/i686-w64-mingw32/bin:$PATH
```

Then build:
```
mkdir build-mingw32 && cd build-mingw32
cmake .. -DCMAKE_TOOLCHAIN_FILE=/usr/share/mingw/toolchain-mingw32.cmake -DCMAKE_INSTALL_PREFIX=install
make
make install
```

The `make install` step should copy the `Data/` and required DLLs from the
system to `build-mingw32/install`.

#### MinGW64

The instructions are similar to those for MinGW32:

```
export PKG_CONFIG_LIBDIR="/usr/x86_64-w64-mingw32/sys-root/mingw/lib/pkgconfig:/usr/x86_64-w64-mingw32/sys-root/mingw/share/pkgconfig"
export PATH=/usr/x86_64-w64-mingw32/bin:$PATH
```
```
mkdir build-mingw64 && cd build-mingw64
cmake .. -DCMAKE_TOOLCHAIN_FILE=/usr/share/mingw/toolchain-mingw64.cmake -DCMAKE_INSTALL_PREFIX=install
make
make install
```
