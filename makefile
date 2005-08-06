
macosx := false
use_devil := false

BINDIR := bin
RUNDIR := run
SRCDIR := Source
SDLDIR := SDL12
LIBPNGDIR := libpng-1.2.8
ZLIBDIR := zlib-1.2.3

EXE := $(RUNDIR)/lugaru-bin

CXX := ccache g++
CC := ccache gcc
#CXX := g++
#CC := gcc
LD := g++

OPT := -O0
#OPT := -O3 -fno-strict-aliasing -falign-loops=16 -fno-math-errno
#OPT := -Os -fno-strict-aliasing

# always use this on the Mac, even in debug builds, since we aren't building
#  a dylib at this point.
ifeq ($(strip $(macosx)),true)
  OPT += -mdynamic-no-pic
endif

DEFINES := \
	-DPLATFORM_UNIX=1 \
	-DPLATFORM_LINUX=1 \
	-DUSE_SDL=1 \
	-DTRUE=1 \
	-DFALSE=0 \
	-Dstricmp=strcasecmp \

INCLUDES := \
			-I$(SRCDIR) \
			-I$(SDLDIR)/include \
			-I./OpenGL/ \
			-I./OpenGL/GL \

ifeq ($(strip $(use_devil)),true)
    DEFINES += -DUSE_DEVIL=1
	INCLUDES += -I$(SRCDIR)/devil/include
else
    DEFINES += -DZ_PREFIX=1
    INCLUDES += -I$(ZLIBDIR) -I$(LIBPNGDIR)
endif

CFLAGS := -g -c $(OPT) $(INCLUDES) $(DEFINES) -fsigned-char
CFLAGS += -w

ifeq ($(strip $(macosx)),true)
  CFLAGS += -fpascal-strings -faltivec -fasm -force_cpusubtype_ALL -Wno-long-double -mdynamic-no-pic
  CFLAGS += -DPLATFORM_BIGENDIAN=1 -DMACOSX=1 -DPLATFORM_MACOSX=1
  LDFLAGS := -framework Cocoa -framework OpenGL -framework IOKit -framework CoreFoundation -framework Carbon
  APPLDFLAGS := $(SDLDIR)/lib/libSDL-1.2.0.dylib $(SDLDIR)/lib/libSDLmain-osx.a
else
  CFLAGS += -DPLATFORM_LINUX=1
  #CFLAGS += -msse -mmmx
  LDFLAGS := ./libSDL-1.2.so.0 -lGL -lGLU ./libfmod.so

  ifeq ($(strip $(use_devil)),true)
    LDFLAGS += ./libIL.so.1 ./libILU.so.1 ./libILUT.so.1
  endif
endif

CXXFLAGS := $(CFLAGS)

SRCS := \
	Frustum.cpp \
	GameDraw.cpp \
	GameInitDispose.cpp \
	GameTick.cpp \
	Globals.cpp \
	Lights.cpp \
	Models.cpp \
	Objects.cpp \
	pack.c \
	pack_private.c \
	Person.cpp \
	private.c \
	Quaternions.cpp \
	Random.c \
	Skeleton.cpp \
	Skybox.cpp \
	Sprites.cpp \
	Terrain.cpp \
	Text.cpp \
	TGALoader.cpp \
	unpack.c \
	unpack_private.c \
	Weapons.cpp \
	MacCompatibility.cpp \
	logger/logger.cpp \
	DRIVER.CC \
	MD5.CC \
	WinInput.cpp \
	OpenGL_Windows.cpp \

SRCS := $(foreach f,$(SRCS),$(SRCDIR)/$(f))


IMGSRCS := \
    png.c \
    pngerror.c \
    pnggccrd.c \
    pngget.c \
    pngmem.c \
    pngpread.c \
    pngread.c \
    pngrio.c \
    pngrtran.c \
    pngrutil.c \
    pngset.c \
    pngtrans.c \
    pngvcrd.c \
    pngwio.c \
    pngwrite.c \
    pngwtran.c \
    pngwutil.c \

IMGSRCS := $(foreach f,$(IMGSRCS),$(LIBPNGDIR)/$(f))


ZLIBSRCS = \
	adler32.c \
	compress.c \
	crc32.c \
	deflate.c \
	gzio.c \
	infback.c \
	inffast.c \
	inflate.c \
	inftrees.c \
	trees.c \
	uncompr.c \
	zutil.c \

ZLIBSRCS := $(foreach f,$(ZLIBSRCS),$(ZLIBDIR)/$(f))


ifneq ($(strip $(use_devil)),true)
    SRCS += $(IMGSRCS) $(ZLIBSRCS)
endif

OBJS := $(SRCS:.CC=.o)
OBJS := $(OBJS:.cc=.o)
OBJS := $(OBJS:.cpp=.o)
OBJS := $(OBJS:.c=.o)
OBJS := $(OBJS:.m=.o)
OBJS := $(foreach f,$(OBJS),$(BINDIR)/$(f))


.PHONY: clean all

all : $(EXE)

$(BINDIR)/%.o : %.cpp
	$(CXX) -o $@ $(CXXFLAGS) $<

$(BINDIR)/%.o : %.CC
	$(CXX) -o $@ $(CXXFLAGS) $<

$(BINDIR)/%.o : %.cc
	$(CXX) -o $@ $(CXXFLAGS) $<

$(BINDIR)/%.o : %.m
	$(CC) -o $@ $(CFLAGS) $<

$(BINDIR)/%.o : %.c
	$(CC) -o $@ $(CFLAGS) $<

$(EXE) : $(OBJS) $(APPOBJS)
ifeq ($(strip $(macosx)),true)
	ranlib $(SDLDIR)/lib/libSDLmain-osx.a
	ranlib $(FREETYPEDIR)/lib/libfreetype-osx.a
endif
	$(LD) -o $@ $(APPLDFLAGS) $(LDFLAGS) $(OBJS) $(APPOBJS)

clean:
	rm -f $(BINDIR)/*.o
	rm -f $(BINDIR)/$(SRCDIR)/*.o
	rm -f $(BINDIR)/$(SRCDIR)/logger/*.o
	rm -f $(BINDIR)/$(LIBPNGDIR)/*.o
	rm -f $(BINDIR)/$(ZLIBDIR)/*.o
	rm -f $(EXE)

# end of makefile ...

