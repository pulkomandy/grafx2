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
#  along with Grafx2; if not, see <http://www.gnu.org/licenses/>

# Overridable defaults
  prefix = /usr/local
  exec_prefix = $(prefix)
  bindir = $(exec_prefix)/bin
  datarootdir = $(prefix)/share
  datadir = $(datarootdir)

# Compile with OPTIM=0 to disable gcc optimizations, to enable debug.
  STRIP = strip


### PLATFORM DETECTION AND CONFIGURATION ###

# There is no uname under windows, but we can guess we are there with the COMSPEC env.var
# Windows specific
ifdef COMSPEC
  DELCOMMAND = rm -f
  MKDIR = mkdir -p
  RMDIR = rmdir
  CP = cp
  BIN = grafx2.exe
  COPT = -W -Wall -Wdeclaration-after-statement -O$(OPTIM) -g -ggdb `sdl-config --cflags` $(TTFCOPT) $(JOYCOPT)
  LOPT = `sdl-config --libs` -lSDL_image $(TTFLOPT) -lpng
  CC = gcc
  OBJDIR = obj/win32
  # Resources (icon)
  WINDRES = windres.exe
  OBJRES = $(OBJDIR)/winres.o
  CFGOBJRES = $(OBJDIR)/wincfgres.o
  PLATFORM = win32
  PLATFORMFILES = SDL.dll SDL_image.dll libpng13.dll zlib1.dll gfx2.ico $(TTFLIBS) #some misc files we have to add to the release archive under windows.
  ZIP = zip
else

#For all other platforms, we can rely on uname
  PLATFORM = $(shell uname)

  #AmigaOS4 specific
  ifeq ($(PLATFORM),AmigaOS)
    DELCOMMAND = rm -rf
    MKDIR = mkdir -p
    RMDIR = rmdir
    CP = cp
    BIN = grafx2
    COPT = -Wall -c -gstabs -mcrt=newlib `sdl-config --cflags` -D__USE_INLINE__ $(TTFCOPT)
    LOPT = `sdl-config --libs` -lSDL_image -lpng -ljpeg -lz $(TTFLOPT) -lft2
    CC = gcc
    OBJDIR = obj/amiga
    ZIP = lha
    ZIPOPT = a

  else ifeq ($(PLATFORM),AROS)
  #AROS specific
    DELCOMMAND = rm -rf
    MKDIR = mkdir -p
    RMDIR = rmdir
    CP = cp
    BIN = grafx2
    COPT = -Wall -g `sdl-config --cflags` $(TTFCOPT)
    LOPT = -lSDL_image `sdl-config --libs` -lpng -ljpeg -lz $(TTFLOPT) -lfreetype2shared
    CC = gcc
    OBJDIR = obj/aros
    ZIP = lha
    ZIPOPT = a

  else ifeq ($(PLATFORM),MorphOS) 
  #MorphOS specific
    DELCOMMAND = rm -rf
    MKDIR = mkdir -p
    RMDIR = rmdir
    CP = cp
    BIN = grafx2
    COPT = -Wall -gstabs -c `sdl-config --cflags` $(TTFCOPT)
    LOPT = -lSDL_image `sdl-config --libs` -lpng -ljpeg -lz $(TTFLOPT)
    CC = gcc
    OBJDIR = obj/morphos
    ZIP = lha
    ZIPOPT = a

  else ifeq ($(PLATFORM),AMIGA)
  # AmigaOS 3.x specific (building with gcc)
    DELCOMMAND = rm -rf
    MKDIR = mkdir -p
    RMDIR = rmdir
    CP = cp
    BIN = grafx2
    COPT = -Wall -gstabs -c `sdl-config --cflags` $(TTFCOPT)
    LOPT = -lSDL_image `sdl-config --libs` -lpng -ljpeg -lz $(TTFLOPT) -lfreetype
    CC = gcc
    OBJDIR = obj/amiga
    ZIP = lha
    ZIPOPT = a

  else ifeq ($(PLATFORM),BeOS)
  #BeOS specific
    DELCOMMAND = rm -rf
    MKDIR = mkdir -p
    RMDIR = rmdir
    CP = cp
    BIN = grafx2
    COPT = -W -Wall -c -g `sdl-config --cflags` $(TTFCOPT) -I/boot/home/config/include
    LOPT = `sdl-config --libs` -lSDL_image -lpng -ljpeg -lz $(TTFLOPT)
    CC = gcc
    OBJDIR = obj/beos
    ZIP = zip

  else ifeq ($(PLATFORM),Haiku)
  #Haiku specific
    DELCOMMAND = rm -rf
    MKDIR = mkdir -p
    RMDIR = rmdir
    CP = cp
    BIN = grafx2
    COPT = -W -Wall -c -g `sdl-config --cflags` $(TTFCOPT) -I/boot/common/include
    LOPT = `sdl-config --libs` -lSDL_image -lpng -ljpeg -lz $(TTFLOPT)
    CC = gcc
    OBJDIR = obj/haiku
    ZIP = zip

  else ifeq ($(PLATFORM),skyos)
  #SkyOS specific
    DELCOMMAND = rm -rf
    MKDIR = mkdir -p
    RMDIR = rmdir
    CP = cp
    BIN = grafx2
    COPT = -W -Wall -Wdeclaration-after-statement -c -g `sdl-config --cflags` $(TTFCOPT)
    LOPT = `sdl-config --libs` -lSDL_image -lpng -ljpeg -lz $(TTFLOPT)
    CC = gcc
    OBJDIR = obj/skyos
    ZIP = zip
  else
  
      # Finally, the default rules that work fine for most unix/gcc systems, linux and freebsd are tested.
      # Linux and FreeBSD specific (default rules)
      DELCOMMAND = rm -rf
      MKDIR = mkdir -p
      RMDIR = rmdir
      CP = cp
      ZIP = zip
      PLATFORMFILES = gfx2.png

      # These can only be used under linux and maybe freebsd. They allow to compile for the gp2x or to create a windows binary
      ifdef WIN32CROSS
        #cross compile a Win32 executable
        CC = i586-mingw32msvc-gcc
        BIN = grafx2.exe
        COPT = -W -Wall -Wdeclaration-after-statement -O$(OPTIM) -g -ggdb -Dmain=SDL_main `/usr/local/cross-tools/i386-mingw32/bin/sdl-config --cflags` $(TTFCOPT)
        LOPT = -mwindows -lmingw32 -lSDLmain -lSDL -lshlwapi `/usr/local/cross-tools/i386-mingw32/bin/sdl-config --libs` -lSDL_image $(TTFLOPT)
        OBJDIR = obj/win32
        PLATFORM = win32
      else ifdef GP2XCROSS

        #cross compile an exec for the gp2x
        CC = /opt/open2x/gcc-4.1.1-glibc-2.3.6/arm-open2x-linux/bin/arm-open2x-linux-gcc
        BIN = grafx2.gpe
        COPT = -W -Wall -Wdeclaration-after-statement -pedantic -std=c99 -static -g -O3 -I/opt/open2x/gcc-4.1.1-glibc-2.3.6/include `/opt/open2x/gcc-4.1.1-glibc-2.3.6/bin/sdl-config --cflags` $(TTFCOPT) -D__GP2X__
        LOPT = -static -lSDL_image `/opt/open2x/gcc-4.1.1-glibc-2.3.6/bin/sdl-config --static-libs` -ljpeg -lpng -lz -lm $(TTFLOPT)
        OBJDIR = obj/gp2x
        NOTTF = 1
        PLATFORM = gp2x
        STRIP = /opt/open2x/gcc-4.1.1-glibc-2.3.6/arm-open2x-linux/bin/arm-open2x-linux-strip
      else

        # Compiles a regular linux exectutable for the native platform
        BIN = grafx2
        COPT = -W -Wall -Wdeclaration-after-statement -pedantic -std=c99 -c -g `sdl-config --cflags` $(TTFCOPT)
        LOPT = `sdl-config --libs` -lSDL_image $(TTFLOPT) -lpng
        CC = gcc
        OBJDIR = obj/unix
        X11LOPT = -lX11
      endif
  endif
endif

### BUILD SETTINGS are set according to vars set in the platform selection, the "overridable defaults", and environment variables set before launching make

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

#To disable Joystick emulation of cursor, make NOJOY=1 (for input.o)
#This can be necessary to test keyboard cursor code, because an existing
#joystick will keep reporting a contradicting position.
ifeq ($(NOJOY),1)
  JOYCOPT = -DNO_JOYCURSOR
else
  JOYCOPT =
endif

### And now for the real build rules ###

.PHONY : all debug release clean depend zip version force install uninstall

# This is the list of the objects we want to build. Dependancies are built by "make depend" automatically.
OBJ = $(OBJDIR)/main.o $(OBJDIR)/init.o $(OBJDIR)/graph.o $(OBJDIR)/sdlscreen.o  $(OBJDIR)/misc.o $(OBJDIR)/special.o $(OBJDIR)/buttons.o $(OBJDIR)/palette.o $(OBJDIR)/help.o $(OBJDIR)/operatio.o $(OBJDIR)/pages.o $(OBJDIR)/loadsave.o $(OBJDIR)/readline.o $(OBJDIR)/engine.o $(OBJDIR)/filesel.o $(OBJDIR)/op_c.o $(OBJDIR)/readini.o $(OBJDIR)/saveini.o $(OBJDIR)/shade.o $(OBJDIR)/keyboard.o $(OBJDIR)/io.o $(OBJDIR)/version.o $(OBJDIR)/text.o $(OBJDIR)/SFont.o $(OBJDIR)/setup.o $(OBJDIR)/pxsimple.o $(OBJDIR)/pxtall.o $(OBJDIR)/pxwide.o $(OBJDIR)/pxdouble.o $(OBJDIR)/pxtriple.o $(OBJDIR)/pxtall2.o $(OBJDIR)/pxwide2.o $(OBJDIR)/pxquad.o $(OBJDIR)/windows.o $(OBJDIR)/brush.o $(OBJDIR)/realpath.o $(OBJDIR)/mountlist.o $(OBJDIR)/input.o $(OBJDIR)/hotkeys.o $(OBJDIR)/transform.o

all : $(BIN)

debug : $(BIN)

# Make release will strip the executable to make it smaller but non-debugable
release : $(BIN)
	$(STRIP) $(BIN)

# Create a zip archive ready for upload to the website, including binaries and sourcecode
ziprelease: version $(BIN) release
	tar cvzf src-svn`svnversion | sed 's/:/-/'`.tgz *.c *.h Makefile Makefile.dep gfx2.ico 
	$(ZIP) $(ZIPOPT) grafx2-svn`svnversion | sed 's/:/-/'`$(TTFLABEL)-$(PLATFORM).$(ZIP) $(BIN) gfx2def.ini skins/base.gif gfx2.gif doc/README.txt doc/COMPILING.txt doc/gpl-2.0.txt fonts/8pxfont.png doc/README-zlib1.txt doc/README-SDL.txt doc/README-SDL_image.txt doc/README-SDL_ttf.txt fonts/Tuffy.ttf src-svn`svnversion | sed 's/:/-/'`.tgz $(PLATFORMFILES)
	$(DELCOMMAND) src-svn`svnversion | sed 's/:/-/'`.tgz
	tar cvzf grafx2-svn`svnversion | sed 's/:/-/'`$(TTFLABEL)-src.tgz *.c *.h Makefile Makefile.dep gfx2def.ini skins/base.gif gfx2.ico gfx2.gif doc/README.txt doc/COMPILING.txt doc/gpl-2.0.txt fonts/8pxfont.png fonts/Tuffy.ttf

$(BIN) : $(OBJ) $(OBJRES)
	$(CC) $(OBJ) $(OBJRES) -o $(BIN) $(LOPT)

# SVN revision number
version.c :
	echo "char SVN_revision[]=\"`svnversion`\";" > version.c

version : delversion version.c $(OBJDIR)/version.o

delversion :
	$(DELCOMMAND) version.c

$(OBJDIR)/%.o :
	$(if $(wildcard $(OBJDIR)),,$(MKDIR) $(OBJDIR))
	$(CC) $(COPT) -c $*.c -o $(OBJDIR)/$*.o

depend :
	$(CC) -MM *.c | sed 's:^[^ ]:$$(OBJDIR)/&:' > Makefile.dep

# Link the icons to the program under windows
$(OBJDIR)/winres.o : gfx2.ico
	echo "1 ICON \"gfx2.ico\"" | $(WINDRES) -o $(OBJDIR)/winres.o

clean :
	$(DELCOMMAND) $(OBJ) $(OBJDIR)/version.o $(OBJRES)
	$(DELCOMMAND) $(BIN)

# Linux installation of the program
install : $(BIN)
	# Create dirs
	test -d $(DESTDIR)$(bindir) || $(MKDIR) $(DESTDIR)$(bindir)
	test -d $(DESTDIR)$(datadir)/grafx2 || $(MKDIR) $(DESTDIR)$(datadir)/grafx2
	test -d $(DESTDIR)$(datadir)/grafx2/fonts || $(MKDIR) $(DESTDIR)$(datadir)/grafx2/fonts
	test -d $(DESTDIR)$(datadir)/grafx2/skins || $(MKDIR) $(DESTDIR)$(datadir)/grafx2/skins
	# Generate launcher script
	echo "#!/bin/sh" > $(DESTDIR)$(bindir)/grafx2
	echo $(datadir)/grafx2/$(BIN) '$$*' >> $(DESTDIR)$(bindir)/grafx2
	chmod 755 $(DESTDIR)$(bindir)/grafx2
	# Copy files
	$(CP) $(BIN) $(DESTDIR)$(datadir)/grafx2/
	$(CP) gfx2def.ini $(DESTDIR)$(datadir)/grafx2/
	$(CP) gfx2.gif $(DESTDIR)$(datadir)/grafx2/
	$(CP) fonts/* $(DESTDIR)$(datadir)/grafx2/fonts/
	$(CP) skins/base.gif $(DESTDIR)$(datadir)/grafx2/skins/
	# Icon and desktop file for debian
	$(CP) misc/grafx2.desktop $(DESTDIR)$(datadir)/applications/
	$(CP) misc/grafx2.xpm $(DESTDIR)$(datadir)/icons/
	@echo Install complete
  
# Linux uninstallation of the program
uninstall :
	$(DELCOMMAND) $(DESTDIR)$(bindir)/grafx2
	$(DELCOMMAND) $(DESTDIR)$(datadir)/grafx2/$(BIN)
	$(DELCOMMAND) $(DESTDIR)$(datadir)/grafx2/gfx2def.ini
	$(DELCOMMAND) $(DESTDIR)$(datadir)/grafx2/gfx2.gif
	$(DELCOMMAND) $(DESTDIR)$(datadir)/grafx2/fonts/*
	$(if $(wildcard $(DESTDIR)$(datadir)/grafx2/fonts),,$(RMDIR) $(DESTDIR)$(datadir)/grafx2/fonts)
	$(DELCOMMAND) $(DESTDIR)$(datadir)/grafx2/skins/base.gif
	$(if $(wildcard $(DESTDIR)$(datadir)/grafx2/skins),,$(RMDIR) $(DESTDIR)$(datadir)/grafx2/skins)
	# Icon and desktop file for debian
	$(DELCOMMAND) $(DESTDIR)$(datadir)/applications/grafx2.desktop
	$(DELCOMMAND) $(DESTDIR)$(datadir)/icons/grafx2.xpm
	@echo Uninstall complete

-include Makefile.dep

