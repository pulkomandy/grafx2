CC = gcc
COPT = -Wall -Os -c
LOPT = -lSDL

gfx2 : main.o init.o graph.o sdlscreen.o divers.o special.o boutons.o palette.o aide.o operatio.o loadsave.o readline.o moteur.o files.o op_c.o
	$(CC) $(LOPT) main.o graph.o divers.o -o grafx2

main.o : graph.o const.h struct.h global.h graph.h divers.h init.h boutons.h moteur.h files.h loadsave.h main.c readini.h saveini.h
	$(CC) $(COPT) main.c

init.o : const.h struct.h global.h modesvdo.h graph.h boutons.h palette.h aide.h operatio.h init.c readini.c saveini.c
	$(CC) $(COPT) init.c

graph.o : divers.o graph.c pages.c const.h struct.h global.h divers.h
	$(CC) $(COPT) graph.c

divers.o : global.h const.h divers.c
	$(CC) $(COPT) divers.c

special.o : const.h struct.h global.h graph.h moteur.h special.c
	$(CC) $(COPT) special.c

boutons.o : boutons.c shade.c const.h struct.h global.h divers.h graph.h moteur.h readline.h files.h loadsave.h init.h readini.h saveini.h
	$(CC) $(COPT) boutons.c

palette.o : const.h struct.h global.h divers.h graph.h moteur.h readline.h palette.c
	$(CC) $(COPT) palette.c

aide.o : const.h struct.h global.h divers.h graph.h moteur.h aide.c
	$(CC) $(COPT) aide.c

moteur.o : const.h struct.h global.h graph.h divers.h moteur.c
	$(CC) $(COPT) moteur.c

operatio.o : const.h struct.h global.h divers.h moteur.h graph.h operatio.h operatio.c
	$(CC) $(COPT) operatio.c

readline.o : const.h struct.h global.h divers.h graph.h readline.c
	$(CC) $(COPT) readline.c

files.o : const.h struct.h global.h graph.h divers.h files.c
	$(CC) $(COPT) files.c

loadsave.o : const.h struct.h global.h divers.h graph.h loadsave.c pages.h op_c.h op_asm.h
	$(CC) $(COPT) loadsave.c

op_c.o : struct.h op_c.c op_c.h op_asm.h
	$(CC) $(COPT) op_c.c

sdlscreen.o : sdlscreen.c sdlscreen.h
	$(CC) $(COPT) sdlscreen.c