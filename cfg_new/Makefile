ifdef COMSPEC
COPT = -Wall -O -g -ggdb -Dmain=SDL_main
LOPT = `sdl-config --libs` -lSDL_image -g
else
COPT = -c `sdl-config --cflags` -Wall -g
LOPT = `sdl-config --libs` -lSDL_image -lSDL_gfx -g
endif

gfxcfg: gfxcfg.o SFont.o ../clavier.o
	gcc gfxcfg.o ../clavier.o SFont.o -o gfxcfg $(LOPT)

gfxcfg.o: gfxcfg.c SFont.h
	gcc -c gfxcfg.c -o gfxcfg.o $(COPT)

../clavier.o:  ../clavier.c
	gcc -c ../clavier.c -o ../clavier.o $(COPT)
 
SFont.o: SFont.h SFont.c
	gcc -c SFont.c -o SFont.o $(COPT)
