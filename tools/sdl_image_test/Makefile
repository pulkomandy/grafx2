CFLAGS = -Wall -O -g

ifeq ($(API),sdl2)
# SDL 2.x
SDLCONFIG = sdl2-config
SDLIMAGE = SDL2_image
BIN = showimage-sdl2
CFLAGS += -DUSE_SDL2
else
# SDL 1.2.x
SDLCONFIG = sdl-config
SDLIMAGE = SDL_image
BIN = showimage
CFLAGS += -DUSE_SDL
endif

ifdef WIN32CROSS
CC = $(shell which i686-w64-mingw32-gcc || which mingw32-gcc)
PKGCONFIG_PATH = $(shell if [ -d ../../3rdparty/usr ] ; then echo "PKG_CONFIG_LIBDIR=$${PWD}/../../3rdparty/usr/lib/pkgconfig" ; fi )
PKGCONFIG = $(PKGCONFIG_PATH) $(shell which i686-w64-mingw32-pkg-config || which mingw32-pkg-config || which pkg-config)
SDLCONFIG = $(shell which ../../3rdparty/usr/bin/sdl-config || which /usr/local/cross-tools/i386-mingw32/bin/sdl-config)
BIN := $(BIN).exe
else
PKGCONFIG = pkg-config
endif

CFLAGS += $(shell $(SDLCONFIG) --cflags)

LDLIBS = $(shell $(SDLCONFIG) --libs)

ifdef WIN32CROSS
LDLIBS += -lSDL_image
else
LDLIBS += $(shell $(PKGCONFIG) $(SDLIMAGE) --libs-only-l)

LDFLAGS = $(shell $(PKGCONFIG) $(SDLIMAGE) --libs-only-L)
endif

.PHONY:	all clean

all:	$(BIN)

clean:
	$(RM) $(BIN) *.o

$(BIN):	$(BIN).o

$(BIN).o:	showimage.c
	$(CC) $(CPPFLAGS) $(CFLAGS) -c -o $@ $<

