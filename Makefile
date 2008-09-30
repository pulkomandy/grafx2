# Windows specific
ifdef COMSPEC
DELCOMMAND = del
BIN = grafx2.exe
COPT = -Wall -O -g -ggdb -Dmain=SDL_main
LOPT = -mwindows -lmingw32 -lSDLmain -lSDL -lshlwapi
else
# Linux specific
DELCOMMAND = rm -rf
BIN = grafx2
COPT = -Wall -c -g
LOPT = -lSDL -o $(BIN)
endif

.PHONY : all debug release clean depend zip

CC = gcc
OBJ = main.o init.o graph.o sdlscreen.o divers.o special.o boutons.o palette.o aide.o operatio.o pages.o loadsave.o readline.o moteur.o files.o op_c.o linux.o op_asm.o readini.o saveini.o shade.o clavier.o io.o

OBJDIR = obj/

all : $(BIN)

debug : $(BIN)

release : $(BIN)
	strip $(BIN)

# A raw source archive
zip :
	tar cvzf gfx2-src.tgz *.c *.h Makefile Makefile.dep gfx2.dat gfx2.ini gfx2.cfg doc/gpl-2.0.txt \
    cfg_new/*.c cfg_new/*.h cfg_new/Makefile cfg_new/8pxfont.png 

$(BIN) : $(OBJ)
	$(CC) $(OBJ) -o $(BIN) $(LOPT)
	
%.o :
	$(CC) $(COPT) -c $*.c -o $*.o

depend :
	$(CC) -MM $(OBJ:.o=.c) > Makefile.dep

clean :
	$(DELCOMMAND) *.o
	$(DELCOMMAND) $(BIN)

include Makefile.dep
