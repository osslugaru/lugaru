
macosx := false

BINDIR := bin
RUNDIR := run
SRCDIR := Source
SDLDIR := SDL12

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

INCLUDES := \
			-I./SDL12/include \
			-I./OpenGL/ \
			-I./OpenGL/GL \
			-I$(SRCDIR) \

CFLAGS := -w -g -c $(OPT) $(INCLUDES) $(DEFINES) -fsigned-char

ifeq ($(strip $(macosx)),true)
  CFLAGS += -fpascal-strings -faltivec -fasm -force_cpusubtype_ALL -Wno-long-double -mdynamic-no-pic
  CFLAGS += -DPLATFORM_BIGENDIAN=1 -DMACOSX=1 -DPLATFORM_MACOSX=1
  LDFLAGS := -framework Cocoa -framework OpenGL -framework IOKit -framework CoreFoundation -framework Carbon
  APPLDFLAGS := $(SDLDIR)/lib/libSDL-1.2.0.dylib $(SDLDIR)/lib/libSDLmain-osx.a
else
  CFLAGS += -DPLATFORM_LINUX=1
  #CFLAGS += -msse -mmmx
  LDFLAGS := -lSDL
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
	OpenGL_Windows.cpp \
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
	WinInput.cpp \
	logger/logger.cpp \
	Driver.cc \
	Md5.cc \
	SDLInput.cpp \


OBJS := $(SRCS:.cpp=.o)
OBJS := $(OBJS:.c=.o)
OBJS := $(OBJS:.m=.o)
OBJS := $(foreach f,$(OBJS),$(BINDIR)/$(f))
SRCS := $(foreach f,$(SRCS),$(SRCDIR)/$(f))


.PHONY: clean all

all : $(EXE)

$(BINDIR)/%.o : $(SRCDIR)/%.cpp
	$(CXX) -o $@ $(CXXFLAGS) $<

$(BINDIR)/%.o : $(SRCDIR)/%.CC
	$(CXX) -o $@ $(CXXFLAGS) $<

$(BINDIR)/%.o : $(SRCDIR)/%.m
	$(CC) -o $@ $(CFLAGS) $<

$(BINDIR)/%.o : $(SRCDIR)/%.c
	$(CC) -o $@ $(CFLAGS) $<

$(EXE) : $(OBJS) $(APPOBJS)
ifeq ($(strip $(macosx)),true)
	ranlib $(SDLDIR)/lib/libSDLmain-osx.a
	ranlib $(FREETYPEDIR)/lib/libfreetype-osx.a
endif
	$(LD) -o $@ $(APPLDFLAGS) $(LDFLAGS) $(OBJS) $(APPOBJS)

clean:
	rm -rf $(BINDIR)/*.o
	rm -rf $(EXE)

# end of makefile ...

