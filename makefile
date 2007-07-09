CC = gcc
COPT = -Wall -Os

gfx2 : main.o init.o graph.o video.o divers.o special.o boutons.o palette.o aide.o operatio.o loadsave.o readline.o moteur.o files.o dpmi.o op_c.o op_asm.o
	wlink @make.inc

main.o : const.h struct.h global.h graph.h divers.h init.h boutons.h moteur.h files.h loadsave.h main.c readini.h saveini.h
	$(CC) $(COPT) main.c

init.o : const.h struct.h global.h modesvdo.h graph.h boutons.h palette.h aide.h operatio.h init.c readini.c saveini.c
	$(CC) $(COPT) init.c

graph.o : graph.c pages.c const.h struct.h global.h video.h divers.h
	$(CC) $(COPT) graph.c

video.o : global.h const.h video.c
	$(ASMC) $(ASMOPT) video.c

divers.o : global.h const.h divers.c
	$(ASMC) $(ASMOPT) divers.c

special.o : const.h struct.h global.h graph.h moteur.h special.c
	$(CC) $(COPT) special.c

boutons.o : boutons.c shade.c const.h struct.h global.h divers.h graph.h moteur.h readline.h files.h loadsave.h init.h readini.h saveini.h
	$(CC) $(COPT) boutons

palette.o : const.h struct.h global.h divers.h graph.h moteur.h readline.h palette.c
	$(CC) $(COPT) palette

aide.o : const.h struct.h global.h divers.h graph.h moteur.h aide.c
	$(CC) $(COPT) aide

moteur.o : const.h struct.h global.h graph.h divers.h moteur.c
	$(CC) $(COPT) moteur

operatio.o : const.h struct.h global.h divers.h moteur.h graph.h operatio.h operatio.c
	$(CC) $(COPT) operatio

readline.o : const.h struct.h global.h divers.h graph.h readline.c
	$(CC) $(COPT) readline

files.o : const.h struct.h global.h graph.h divers.h files.c
	$(CC) $(COPT) files

loadsave.o : const.h struct.h global.h divers.h graph.h loadsave.c pages.h op_c.h op_asm.h
	$(CC) $(COPT) loadsave

op_c.o : struct.h op_c.c op_c.h op_asm.h
	$(CC) $(COPT) op_c

dpmi.o : dpmi.asm dpmi.h
	$(ASMC) $(ASMOPT) dpmi

vesalfb.o : vesalfb.asm vesalfb.h dpmi.h
	$(ASMC) $(ASMOPT) vesalfb

op_asm.o : struct.h op_asm.asm op_c.h op_asm.h
	$(ASMC) $(ASMOPT) op_asm
