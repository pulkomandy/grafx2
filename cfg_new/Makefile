COPT = -c `sdl-config --cflags` -Wall -g
LOPT = `sdl-config --libs` -lSDL_image -lSDL_gfx -g

gfxcfg: gfxcfg.o SFont.o
	gcc gfxcfg.o SFont.o -o gfxcfg $(LOPT)

gfxcfg.o: gfxcfg.c SFont.h scancodes.h
	gcc gfxcfg.c -o gfxcfg.o $(COPT)

SFont.o: SFont.h SFont.c
	gcc SFont.c -o SFont.o $(COPT)
