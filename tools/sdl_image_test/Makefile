CFLAGS = -Wall -O -g

ifeq ($(API),sdl2)
# SDL 2.x
SDLCONFIGNAME = sdl2-config
SDLIMAGE = SDL2_image
BIN = showimage-sdl2
CFLAGS += -DUSE_SDL2
else
# SDL 1.2.x
SDLCONFIGNAME = sdl-config
SDLIMAGE = SDL_image
BIN = showimage
CFLAGS += -DUSE_SDL
endif

PLATFORM =
3RDPARTYDIR = ../../3rdparty/usr$(PLATFORM)
PKGCONFIG_PATH = $(shell if [ -d $(3RDPARTYDIR) ] ; then echo "PKG_CONFIG_LIBDIR=$${PWD}/$(3RDPARTYDIR)/lib/pkgconfig" ; fi )

ifdef WIN32CROSS
PLATFORM = -win32
CC = $(shell which i686-w64-mingw32-gcc || which mingw32-gcc)
PKGCONFIG = $(PKGCONFIG_PATH) $(shell which i686-w64-mingw32-pkg-config || which mingw32-pkg-config || which pkg-config)
SDLCONFIG = $(shell which $(3RDPARTYDIR)/bin/$(SDLCONFIGNAME) || which /usr/local/cross-tools/i386-mingw32/bin/$(SDLCONFIGNAME))
BIN := $(BIN).exe
else
ifdef ATARICROSS
PLATFORM = -atari
ARCH ?= m68k
CC = $(shell which $(ARCH)-atari-mint-gcc)
BIN := $(BIN).ttp
endif
PKGCONFIG = $(PKGCONFIG_PATH) pkg-config
SDLCONFIG = $(shell which $(3RDPARTYDIR)/bin/$(SDLCONFIGNAME) || which $(SDLCONFIGNAME))
endif

CFLAGS += $(shell $(SDLCONFIG) --cflags)

LDLIBS = $(shell $(SDLCONFIG) --libs)

ifdef WIN32CROSS
ifeq ($(API),sdl2)
LDLIBS += -lSDL2_image
else
LDLIBS += -lSDL_image
endif
else
LDLIBS += $(shell $(PKGCONFIG) $(SDLIMAGE) --libs-only-l)
ifdef ATARICROSS
LDLIBS += -ltiff -ljpeg -lpng -lz -lm
endif

LDFLAGS = $(shell $(PKGCONFIG) $(SDLIMAGE) --libs-only-L)
endif

.PHONY:	all clean

all:	$(BIN)

clean:
	$(RM) $(BIN) *.o

$(BIN):	$(BIN).o

$(BIN).o:	showimage.c
	$(CC) $(CPPFLAGS) $(CFLAGS) -c -o $@ $<

