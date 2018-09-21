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

CFLAGS += $(shell $(SDLCONFIG) --cflags)

LDLIBS = $(shell $(SDLCONFIG) --libs)
LDLIBS += $(shell pkg-config $(SDLIMAGE) --libs-only-l)

LDFLAGS = $(shell pkg-config $(SDLIMAGE) --libs-only-L)

.PHONY:	all clean

all:	$(BIN)

clean:
	$(RM) $(BIN) *.o

$(BIN):	$(BIN).o

$(BIN).o:	showimage.c
	$(CC) $(CPPFLAGS) $(CFLAGS) -c -o $@ $<

