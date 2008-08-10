CC = gcc
COPT = -Wall -c -g 
LOPT = -lSDL -o grafx2 
SCREEN = -D WINDOWED

debug: grafx2
	@echo --- Everything OK.

release: grafx2
	strip grafx2

grafx2 : main.o init.o graph.o sdlscreen.o divers.o special.o boutons.o palette.o aide.o operatio.o pages.o loadsave.o readline.o moteur.o files.o op_c.o linux.o op_asm.o readini.o saveini.o
	$(CC) $(LOPT) main.o graph.o divers.o init.o files.o linux.o loadsave.o boutons.o moteur.o sdlscreen.o aide.o palette.o pages.o operatio.o op_c.o readline.o special.o op_asm.o readini.o saveini.o

aide.o : const.h struct.h global.h divers.h graph.h moteur.h aide.h aide.c
	$(CC) $(COPT) aide.c

boutons.o : boutons.h boutons.c shade.c const.h struct.h global.h divers.h graph.h moteur.h readline.h files.h loadsave.h init.h readini.h saveini.h
	$(CC) $(COPT) boutons.c

divers.o : global.h const.h divers.h divers.c
	$(CC) $(COPT) divers.c

files.o : const.h struct.h global.h graph.h divers.h files.h files.c
	$(CC) $(COPT) files.c

graph.o : graph.c pages.c const.h struct.h global.h divers.h erreurs.h
	$(CC) $(COPT) graph.c

init.o : const.h struct.h global.h modesvdo.h graph.h boutons.h palette.h aide.h operatio.h init.c readini.c saveini.c
	$(CC) $(COPT) init.c

linux.o : linux.h linux.c
	$(CC) $(COPT) linux.c

loadsave.o : const.h struct.h global.h divers.h graph.h loadsave.c pages.h op_c.h op_asm.h
	$(CC) $(COPT) loadsave.c

main.o : const.h struct.h global.h graph.h divers.h init.h boutons.h moteur.h files.h loadsave.h main.c readini.h saveini.h
	$(CC) $(COPT) main.c

moteur.o : const.h struct.h global.h graph.h divers.h moteur.c
	$(CC) $(COPT) moteur.c

op_c.o : struct.h op_c.c op_c.h op_asm.h
	$(CC) $(COPT) op_c.c

op_asm.o : op_asm.h op_asm.c
	$(CC) $(COPT) op_asm.c

operatio.o : const.h struct.h global.h divers.h moteur.h graph.h operatio.h operatio.c
	$(CC) $(COPT) operatio.c

pages.o : pages.h pages.c
	$(CC) $(COPT) pages.c

palette.o : const.h struct.h global.h divers.h graph.h moteur.h readline.h palette.c
	$(CC) $(COPT) palette.c

readini.o : readini.h readini.c
	$(CC) $(COPT) readini.c

readline.o : const.h struct.h global.h divers.h graph.h readline.c
	$(CC) $(COPT) readline.c

savenini.o : saveini.h saveini.c
	$(CC) $(COPT) saveini.c

sdlscreen.o : sdlscreen.c sdlscreen.h
	$(CC) $(COPT) sdlscreen.c $(SCREEN)
	
special.o : const.h struct.h global.h graph.h moteur.h special.c
	$(CC) $(COPT) special.c

clean :
	rm -rf *.o
	rm grafx2
