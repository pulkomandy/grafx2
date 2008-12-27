#  Grafx2 - The Ultimate 256-color bitmap paint program
#  
#  Copyright 2008 Peter Gordon
#  Copyright 2008 Yves Rizoud
#  Copyright 2007 Adrien Destugues
#  Copyright 1996-2001 Sunset Design (Guillaume Dorme & Karl Maritaud)
#  
#  Grafx2 is free software; you can redistribute it and/or
#  modify it under the terms of the GNU General Public License
#  as published by the Free Software Foundation; version 2
#  of the License.
#  
#  Grafx2 is distributed in the hope that it will be useful,
#  but WITHOUT ANY WARRANTY; without even the implied warranty of
#  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
#  GNU General Public License for more details.
#  
#  You should have received a copy of the GNU General Public License
#  along with Grafx2; if not, see <http://www.gnu.org/licenses/> or
#  write to the Free Software Foundation, Inc.,
#  59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.

# Overridable defaults
  prefix = /usr/local
  exec_prefix = $(prefix)
  bindir = $(exec_prefix)/bin
  datarootdir = $(prefix)/share
  datadir = $(datarootdir)

# Windows specific
ifdef COMSPEC
  DELCOMMAND = rm -f
  MKDIR = mkdir -p
  RMDIR = rmdir
  CP = cp
  BIN = grafx2.exe
  CFGBIN = gfxcfg.exe
  COPT = -W -Wall -Wdeclaration-after-statement -O -g -ggdb `sdl-config --cflags` $(TTFCOPT)
  LOPT = `sdl-config --libs` -lSDL_image $(TTFLOPT)
  CC = gcc
  OBJDIR = obj/win32
  # Resources (icon)
  WINDRES = windres.exe
  OBJRES = $(OBJDIR)/winres.o
  CFGOBJRES = $(OBJDIR)/wincfgres.o
  PLATFORM = win32 #no uname so we'll do that ourselve
  PLATFORMFILES = SDL.dll SDL_image.dll libpng13.dll zlib1.dll #some misc files we have to add to the release archive
else

  PLATFORM = $(shell uname)

  #OS4 specific
  ifeq ($(PLATFORM),AmigaOS)
    DELCOMMAND = rm -rf
    MKDIR = mkdir -p
    RMDIR = rmdir
    CP = cp
    BIN = grafx2
    CFGBIN = gfxcfg
    COPT = -Wall -c -gstabs -mcrt=newlib `sdl-config --cflags` $(TTFCOPT)
    LOPT = `sdl-config --libs` -lSDL_image -lpng -ljpeg -lz $(TTFLOPT)
    CC = gcc
    OBJDIR = obj/amiga
    NOTTF = 1
  else

  #AROS specific
  ifeq ($(PLATFORM),AROS)
    DELCOMMAND = rm -rf
    MKDIR = mkdir -p
    RMDIR = rmdir
    CP = cp
    BIN = grafx2
    CFGBIN = gfxcfg
    COPT = -Wall _c _g `i386-linux-aros-sdl-config --cflags` $(TTFCOPT)
    LOPT = -lSDL_image `i386-linux-aros-sdl-config --libs` -lpng -ljpeg -lz $(TTFLOPT)
    CC = i386-linux-aros-gcc
    OBJDIR = obj/aros
  else

  #BeOS specific
  ifeq ($(PLATFORM),BeOS)
    DELCOMMAND = rm -rf
    MKDIR = mkdir -p
    RMDIR = rmdir
    CP = cp
    BIN = grafx2
    CFGBIN = gfxcfg
    COPT = -W -Wall -c -g `sdl-config --cflags` $(TTFCOPT) -I/boot/home/config/include
    LOPT = `sdl-config --libs` -lSDL_image -lpng -ljpeg -lz $(TTFLOPT)
    CC = gcc
    OBJDIR = obj/beos
  else

  #Haiku specific
  ifeq ($(PLATFORM),Haiku)
    DELCOMMAND = rm -rf
    MKDIR = mkdir -p
    RMDIR = rmdir
    CP = cp
    BIN = grafx2
    CFGBIN = gfxcfg
    COPT = -W -Wall -c -g `sdl-config --cflags` $(TTFCOPT) -I/boot/common/include
    LOPT = `sdl-config --libs` -lSDL_image -lpng -ljpeg -lz $(TTFLOPT)
    CC = gcc
    OBJDIR = obj/haiku
  else
  
    # Linux specific
    DELCOMMAND = rm -rf
    MKDIR = mkdir -p
    RMDIR = rmdir
    CP = cp
    ifdef WIN32CROSS
      #cross compile a Win32 executable
      CC = i586-mingw32msvc-gcc
      BIN = grafx2.exe
      CFGBIN = gfxcfg.exe
      COPT = -W -Wall -Wdeclaration-after-statement -O -g -ggdb -Dmain=SDL_main `/usr/local/cross-tools/i386-mingw32/bin/sdl-config --cflags` $(TTFCOPT)
      LOPT = -mwindows -lmingw32 -lSDLmain -lSDL -lshlwapi `/usr/local/cross-tools/i386-mingw32/bin/sdl-config --libs` -lSDL_image $(TTFLOPT)
      OBJDIR = obj/win32
    else
      BIN = grafx2
      CFGBIN = gfxcfg
      COPT = -W -Wall -Wdeclaration-after-statement -c -g `sdl-config --cflags` $(TTFCOPT)
      LOPT = `sdl-config --libs` -lSDL_image $(TTFLOPT)
      CC = gcc
      OBJDIR = obj/unix
      X11LOPT = -lX11
    endif
  endif
endif
endif
endif
endif

#TrueType is optional: make NOTTF=1 to disable support and dependencies.
ifeq ($(NOTTF),1)
  TTFCOPT = -DNOTTF=1
  TTFLOPT =
  TTFLIBS =
  TTFLABEL = -nottf
else
  TTFCOPT = 
  TTFLOPT = -L/usr/local/lib -lSDL_ttf $(X11LOPT)
  TTFLIBS = libfreetype-6.dll SDL_ttf.dll
  TTFLABEL = 
endif


.PHONY : all debug release clean depend zip version force install uninstall

OBJ = $(OBJDIR)/main.o $(OBJDIR)/init.o $(OBJDIR)/graph.o $(OBJDIR)/sdlscreen.o  $(OBJDIR)/divers.o $(OBJDIR)/special.o $(OBJDIR)/boutons.o $(OBJDIR)/palette.o $(OBJDIR)/aide.o $(OBJDIR)/operatio.o $(OBJDIR)/pages.o $(OBJDIR)/loadsave.o $(OBJDIR)/readline.o $(OBJDIR)/moteur.o $(OBJDIR)/files.o $(OBJDIR)/op_c.o $(OBJDIR)/readini.o $(OBJDIR)/saveini.o $(OBJDIR)/shade.o $(OBJDIR)/clavier.o $(OBJDIR)/io.o $(OBJDIR)/version.o $(OBJDIR)/texte.o $(OBJDIR)/SFont.o $(OBJDIR)/setup.o $(OBJDIR)/pxsimple.o $(OBJDIR)/pxtall.o $(OBJDIR)/pxwide.o $(OBJDIR)/windows.o $(OBJDIR)/brush.o $(OBJDIR)/realpath.o
CFGOBJ = $(OBJDIR)/gfxcfg.o $(OBJDIR)/SFont.o $(OBJDIR)/clavier.o $(OBJDIR)/io.o $(OBJDIR)/setup.o

all : $(BIN) $(CFGBIN)

debug : $(BIN)

release : $(BIN) $(CFGBIN)
	strip $(BIN)
	strip $(CFGBIN)

# A release zip archive
ziprelease: version $(BIN) $(BINCFG) release
	tar cvzf src-svn`svnversion | sed 's/:/-/'`.tgz *.c *.h Makefile Makefile.dep gfx2.ico gfx2cfg.ico
	zip grafx2-svn`svnversion | sed 's/:/-/'`$(TTFLABEL)-$(PLATFORM).zip $(BIN) $(CFGBIN) gfx2.dat gfx2.gif gfx2cfg.gif doc/gpl-2.0.txt fonts/8pxfont.png $(TTFLIBS) doc/README-zlib1.txt doc/README-SDL.txt doc/README-SDL_image.txt doc/README-SDL_ttf.txt fonts/Tuffy.ttf src-svn`svnversion | sed 's/:/-/'`.tgz $(PLATFORMFILES)
	$(DELCOMMAND) src-svn`svnversion | sed 's/:/-/'`.tgz
	tar cvzf grafx2-svn`svnversion | sed 's/:/-/'`$(TTFLABEL)-src.tgz *.c *.h Makefile Makefile.dep gfx2.dat gfx2.ico gfx2.gif gfx2cfg.gif doc/gpl-2.0.txt fonts/8pxfont.png fonts/Tuffy.ttf

$(BIN) : $(OBJ) $(OBJRES)
	$(CC) $(OBJ) $(OBJRES) -o $(BIN) $(LOPT)

$(CFGBIN) : $(CFGOBJ) $(CFGOBJRES)
	$(CC) $(CFGOBJ) $(CFGOBJRES) -o $(CFGBIN) $(LOPT)

# SVN revision number
version.c :
	echo "char SVNRevision[]=\"`svnversion`\";" > version.c

version : delversion version.c $(OBJDIR)/version.o

delversion :
	$(DELCOMMAND) version.c

$(OBJDIR)/%.o :
	$(if $(wildcard $(OBJDIR)),,$(MKDIR) $(OBJDIR))
	$(CC) $(COPT) -c $*.c -o $(OBJDIR)/$*.o

depend :
	$(CC) -MM *.c | sed 's:^[^ ]:$$(OBJDIR)/&:' > Makefile.dep

$(OBJDIR)/winres.o : gfx2.ico
	echo "1 ICON \"gfx2.ico\"" | $(WINDRES) -o $(OBJDIR)/winres.o
$(OBJDIR)/wincfgres.o : gfx2cfg.ico
	echo "1 ICON \"gfx2cfg.ico\"" | $(WINDRES) -o $(OBJDIR)/wincfgres.o

clean :
	$(DELCOMMAND) $(OBJ) $(CFGOBJ) $(OBJDIR)/version.o $(OBJRES) $(CFGOBJRES)
	$(DELCOMMAND) $(BIN) $(CFGBIN)

install : $(BIN) $(CFGBIN)
	echo "#!/bin/sh" > $(bindir)/grafx2
	echo $(datadir)/grafx2/$(BIN) '$$*' >> $(bindir)/grafx2
	chmod 755 $(bindir)/grafx2
	echo "#!/bin/sh" > $(bindir)/gfxcfg
	echo $(datadir)/grafx2/$(CFGBIN) '$$*' >> $(bindir)/gfxcfg
	chmod 755 $(bindir)/gfxcfg
	$(if $(wildcard $(datadir)/grafx2),,$(MKDIR) $(datadir)/grafx2)
	$(CP) $(BIN) $(datadir)/grafx2/
	$(CP) $(CFGBIN) $(datadir)/grafx2/
	$(CP) gfx2.dat $(datadir)/grafx2/
	$(CP) gfx2.gif $(datadir)/grafx2/
	$(CP) gfx2cfg.gif $(datadir)/grafx2/
	$(if $(wildcard $(datadir)/grafx2/fonts),,$(MKDIR) $(datadir)/grafx2/fonts)
	cd fonts && $(CP) * $(datadir)/grafx2/fonts/
	@echo Install complete
  
uninstall :
	$(DELCOMMAND) $(bindir)/grafx2
	$(DELCOMMAND) $(bindir)/gfxcfg
	$(DELCOMMAND) $(datadir)/grafx2/$(BIN)
	$(DELCOMMAND) $(datadir)/grafx2/$(CFGBIN)
	$(DELCOMMAND) $(datadir)/grafx2/gfx2.dat
	$(DELCOMMAND) $(datadir)/grafx2/gfx2.gif
	$(DELCOMMAND) $(datadir)/grafx2/gfx2cfg.gif
	$(DELCOMMAND) $(datadir)/grafx2/fonts/*
	$(if $(wildcard $(datadir)/grafx2/fonts),,$(RMDIR) $(datadir)/grafx2/fonts)
	$(if $(wildcard $(datadir)/grafx2),,$(RMDIR) $(datadir)/grafx2)
	@echo Uninstall complete

-include Makefile.dep

