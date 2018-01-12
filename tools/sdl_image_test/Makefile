CFLAGS = -Wall -O -g

CFLAGS += $(shell sdl-config --cflags)

LDLIBS = $(shell sdl-config --libs)
LDLIBS += $(shell pkg-config SDL_image --libs-only-l)

LDFLAGS = $(shell pkg-config SDL_image --libs-only-L)

.PHONY:	all clean

all:	showimage

clean:
	$(RM) showimage *.o

showimage:	showimage.o

