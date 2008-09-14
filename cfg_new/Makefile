ifdef COMSPEC
COPT = -Wall -O -g -ggdb -Dmain=SDL_main
LOPT = `sdl-config --libs` -lSDL_image -g
else
COPT = -c `sdl-config --cflags` -Wall -g
LOPT = `sdl-config --libs` -lSDL_image -lSDL_gfx -g
endif

gfxcfg: gfxcfg.o SFont.o
	gcc gfxcfg.o SFont.o -o gfxcfg $(LOPT)

gfxcfg.o: gfxcfg.c SFont.h scancodes.h
	gcc -c gfxcfg.c -o gfxcfg.o $(COPT)

SFont.o: SFont.h SFont.c
	gcc -c SFont.c -o SFont.o $(COPT)
