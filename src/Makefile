#  Grafx2 - The Ultimate 256-color bitmap paint program
#  
#  Copyright 2009 Per Olofsson
#  Copyright 2008 Peter Gordon
#  Copyright 2008-2010 Yves Rizoud
#  Copyright 2007-2010 Adrien Destugues
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

PLATFORMOBJ =

# There is no uname under windows, but we can guess we are there with the COMSPEC env.var
# Windows specific
ifdef COMSPEC
  DELCOMMAND = rm -f
  MKDIR = mkdir -p
  RMDIR = rmdir
  CP = cp
  BIN = ../bin/grafx2.exe
  COPT = -W -Wall -Wdeclaration-after-statement -O$(OPTIM) -g -ggdb `sdl-config --cflags` $(TTFCOPT) $(JOYCOPT) $(LUACOPT) $(LAYERCOPT)
  LOPT = `sdl-config --libs` -lSDL_image $(TTFLOPT) -lpng $(LUALOPT)
  LUALOPT = -llua
  CC = gcc
  OBJDIR = ../obj/win32
  # Resources (icon)
  WINDRES = windres.exe
  PLATFORMOBJ = $(OBJDIR)/winres.o
  PLATFORM = win32
  #some misc files we have to add to the release archive under windows.
  PLATFORMFILES = bin/SDL.dll bin/SDL_image.dll bin/libpng13.dll bin/zlib1.dll $(TTFLIBS)
  ZIP = zip
else

#For all other platforms, we can rely on uname
  PLATFORM = $(shell uname)

  #AmigaOS (3 or 4) specific
  ifeq ($(PLATFORM),AmigaOS) # 1
    DELCOMMAND = rm -rf
    MKDIR = mkdir -p
    RMDIR = rmdir
    CP = cp
    BIN = ../bin/grafx2
    COPT = -Wall -c -gstabs `sdl-config --cflags` $(TTFCOPT)
    LOPT = `sdl-config --libs` -lSDL_image -lpng -ljpeg -lz $(TTFLOPT) -lft2
    CC = gcc
    OBJDIR = ../obj/amiga
    ZIP = lha
    ZIPOPT = a
    NOTTF = 1

  else
  ifeq ($(PLATFORM),Darwin) # 2
  #Mac OS X specific
  DELCOMMAND = rm -rf
  MKDIR = mkdir -p
  RMDIR = rmdir
  CP = cp
  ZIP = zip
  PLATFORMFILES = gfx2.png
  BIN = ../bin/grafx2
  # Where the SDL frameworks are located
  FWDIR = /Library/Frameworks
  SDLCOPT = -arch i386 -I$(FWDIR)/SDL.framework/Headers -I$(FWDIR)/SDL_image.framework/Headers -I$(FWDIR)/SDL_ttf.framework/Headers -D_THREAD_SAFE
  SDLLOPT = -arch i386 -L/usr/lib -framework SDL -framework SDL_image -framework SDL_ttf -framework Cocoa -framework Carbon -framework OpenGL
  COPT = -D__macosx__ -D__linux__ -W -Wall -Wdeclaration-after-statement -O$(OPTIM) -std=c99 -c -g $(SDLCOPT) $(TTFCOPT) -I/usr/X11/include
  LOPT = $(SDLLOPT) -L/usr/X11/lib -R/usr/X11/lib -lpng
  # Use gcc for compiling. Use ncc to build a callgraph and analyze the code.
  CC = gcc
  #CC = nccgen -ncgcc -ncld -ncfabs
  OBJDIR = ../obj/macosx
  PLATFORMOBJ = $(OBJDIR)/SDLMain.o
  X11LOPT = 
  MACAPPEXE = Grafx2.app/Contents/MacOS/Grafx2

  else
  ifeq ($(PLATFORM),AROS) # 3
  #AROS specific
    DELCOMMAND = rm -rf
    MKDIR = mkdir -p
    RMDIR = rmdir
    CP = cp
    BIN = ../bin/grafx2
    COPT = -Wall -g `sdl-config --cflags` $(TTFCOPT)
    LOPT = -lSDL_image `sdl-config --libs` -lpng -ljpeg -lz $(TTFLOPT) -lfreetype2shared
    CC = gcc
    OBJDIR = ../obj/aros
    STRIP = strip --strip-unneeded --remove-section .comment
    ZIP = lha
    ZIPOPT = a

  else
  ifeq ($(PLATFORM),MorphOS) # 4
  #MorphOS specific
    DELCOMMAND = rm -rf
    MKDIR = mkdir -p
    RMDIR = rmdir
    CP = cp
    BIN = ../bin/grafx2
    COPT = -Wall -gstabs -c `sdl-config --cflags` $(TTFCOPT)
    LOPT = -lSDL_image `sdl-config --libs` -lpng -ljpeg -lz $(TTFLOPT)
    CC = gcc
    OBJDIR = ../obj/morphos
    ZIP = lha
    ZIPOPT = a
    PLATFORMFILES = ../misc/grafx2.info

  else
  ifeq ($(PLATFORM),BeOS) # 6
  #BeOS specific
    DELCOMMAND = rm -rf
    MKDIR = mkdir -p
    RMDIR = rmdir
    CP = cp
    BIN = ../bin/grafx2
    COPT = -W -Wall -c -g `sdl-config --cflags` $(TTFCOPT) -I/boot/home/config/include
    LOPT = `sdl-config --libs` -lSDL_image -lpng -ljpeg -lz $(TTFLOPT)
    CC = gcc
    OBJDIR = ../obj/beos
    ZIP = zip

  else
  ifeq ($(PLATFORM),Haiku) # 7
  #Haiku specific
    DELCOMMAND = rm -rf
    MKDIR = mkdir -p
    RMDIR = rmdir
    CP = cp
    BIN = ../bin/grafx2
    PLATFORMOBJ = $(OBJDIR)/haiku.o
    COPT = -W -Wall -c -g `sdl-config --cflags` $(TTFCOPT) -I/boot/common/include
    LOPT = `sdl-config --libs` -lSDL_image -lpng -ljpeg -lz $(TTFLOPT) -lfreetype -lbe
    CC = gcc
    OBJDIR = ../obj/haiku
    ZIP = zip

  else
  ifeq ($(PLATFORM),skyos) # 8
  #SkyOS specific
    DELCOMMAND = rm -rf
    MKDIR = mkdir -p
    RMDIR = rmdir
    CP = cp
    BIN = ../bin/grafx2
    COPT = -W -Wall -Wdeclaration-after-statement -c -g `sdl-config --cflags` $(TTFCOPT)
    LOPT = `sdl-config --libs` -lSDL_image -lpng -ljpeg -lz $(TTFLOPT)
    CC = gcc
    OBJDIR = ../obj/skyos
    ZIP = zip

  else
  ifeq ($(PLATFORM),OSF1) #9
  #OSF1 / tru64 alpha
      DELCOMMAND = rm -rf
      MKDIR = mkdir -p
      RMDIR = rmdir
      CP = cp
      ZIP = zip
      PLATFORMFILES = gfx2.png
      BIN = ../bin/grafx2
      COPT = -W -Wall -std=c99 -c -g -gstabs -D__TRU64__ `sdl-config --cflags` $(TTFCOPT) $(LUACOPT)
      LOPT = `sdl-config --libs` -lSDL_image $(TTFLOPT) -lpng $(LUALOPT) -lm
      OBJDIR = ../obj/unix
      X11LOPT = -lX11
      CC = gcc
  else
  
      # Finally, the default rules that work fine for most unix/gcc systems, linux and freebsd are tested.
      # Linux and FreeBSD specific (default rules)
      DELCOMMAND = rm -rf
      MKDIR = mkdir -p
      RMDIR = rmdir
      CP = cp
      ZIP = zip
      PLATFORMFILES = gfx2.png
      ifeq ($(NOLUA),1)
        LUACOPT =
        LUALOPT =
      else
        ifeq (`pkg-config --exists lua --print-errors`,"")
            LUACOPT = `pkg-config lua --cflags`
            LUALOPT = `pkg-config lua --libs`
        else
            LUACOPT = `pkg-config lua5.1 --cflags`
            LUALOPT = `pkg-config lua5.1 --libs`
        endif
      endif

      # These can only be used under linux and maybe freebsd. They allow to compile for the gp2x or to create a windows binary
      ifdef WIN32CROSS
        #cross compile a Win32 executable
        CC = i586-mingw32msvc-gcc
        BIN = ../bin/grafx2.exe
        COPT = -W -Wall -Wdeclaration-after-statement -O$(OPTIM) -g -ggdb -Dmain=SDL_main `/usr/local/cross-tools/i386-mingw32/bin/sdl-config --cflags` $(TTFCOPT)
        LOPT = -mwindows -lmingw32 -lSDLmain -lSDL -lshlwapi `/usr/local/cross-tools/i386-mingw32/bin/sdl-config --libs` -lSDL_image $(TTFLOPT)
        OBJDIR = ../obj/win32
        PLATFORM = win32
      else
      ifdef GP2XCROSS

        #cross compile an exec for the gp2x
        CC = /opt/open2x/gcc-4.1.1-glibc-2.3.6/arm-open2x-linux/bin/arm-open2x-linux-gcc
        BIN = ../bin/grafx2.gpe
        COPT = -W -Wall -Wdeclaration-after-statement -pedantic -std=c99 -static -g -O$(OPTIM) -I/opt/open2x/gcc-4.1.1-glibc-2.3.6/include `/opt/open2x/gcc-4.1.1-glibc-2.3.6/bin/sdl-config --cflags` $(TTFCOPT) -D__GP2X__ $(TTFCOPT) $(JOYCOPT) $(LUACOPT) $(LAYERCOPT)
        LOPT = -static -lSDL_image `/opt/open2x/gcc-4.1.1-glibc-2.3.6/bin/sdl-config --static-libs` -ljpeg -lpng -lz -lm $(TTFLOPT) $(LUALOPT)
        OBJDIR = ../obj/gp2x
        NOTTF = 1
        PLATFORM = gp2x
        STRIP = /opt/open2x/gcc-4.1.1-glibc-2.3.6/arm-open2x-linux/bin/arm-open2x-linux-strip
        JOYCOPT = -DUSE_JOYSTICK
      else

        # Compiles a regular linux executable for the native platform
        BIN = ../bin/grafx2
        COPT = -W -Wall -Wdeclaration-after-statement -std=c99 -c -g `sdl-config --cflags` $(TTFCOPT) $(LUACOPT) $(JOYCOPT) -O$(OPTIM)
        LOPT = `sdl-config --libs` -lSDL_image $(TTFLOPT) -lpng $(LUALOPT) -lm
        # Use gcc for compiling. Use ncc to build a callgraph and analyze the code.
        CC = gcc
        #CC = nccgen -ncgcc -ncld -ncfabs
        OBJDIR = ../obj/unix
        X11LOPT = -lX11
      endif
      endif
  endif
  endif
  endif
  endif
  endif
  endif
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
  TTFLIBS = bin/libfreetype-6.dll bin/SDL_ttf.dll
  TTFLABEL = 
endif

#Lua scripting is optional too
ifeq ($(NOLUA),1)
    LUACOPT =
    LUALOPT =
    LUALABEL = -nolua
else
    LUACOPT += -D__ENABLE_LUA__
    LUALABEL =
endif

#To enable Joystick emulation of cursor, make USE_JOYSTICK=1 (for input.o)
#This can be necessary to test cursor code on a PC, but by default for all
#non-console platforms the joystick is disabled, to avoid reporting
#'restless' movements when an analog joystick or a poorly-calibrated joypad
#is plugged in.
ifeq ($(USE_JOYSTICK),1)
  JOYCOPT = -DUSE_JOYSTICK
endif

#To speed up rendering, can disable the layered editing
# with NOLAYERS=1
ifeq ($(NOLAYERS),1)
  LAYERCOPT = -DNOLAYERS
else
  LAYERCOPT =
endif

### And now for the real build rules ###

.PHONY : all debug release clean depend zip version force install uninstall

# This is the list of the objects we want to build. Dependancies are built by "make depend" automatically.
OBJ = $(OBJDIR)/main.o $(OBJDIR)/init.o $(OBJDIR)/graph.o $(OBJDIR)/sdlscreen.o  $(OBJDIR)/misc.o $(OBJDIR)/special.o $(OBJDIR)/buttons.o $(OBJDIR)/palette.o $(OBJDIR)/help.o $(OBJDIR)/operatio.o $(OBJDIR)/pages.o $(OBJDIR)/loadsave.o $(OBJDIR)/readline.o $(OBJDIR)/engine.o $(OBJDIR)/filesel.o $(OBJDIR)/op_c.o $(OBJDIR)/readini.o $(OBJDIR)/saveini.o $(OBJDIR)/shade.o $(OBJDIR)/keyboard.o $(OBJDIR)/io.o $(OBJDIR)/version.o $(OBJDIR)/text.o $(OBJDIR)/SFont.o $(OBJDIR)/setup.o $(OBJDIR)/pxsimple.o $(OBJDIR)/pxtall.o $(OBJDIR)/pxwide.o $(OBJDIR)/pxdouble.o $(OBJDIR)/pxtriple.o $(OBJDIR)/pxtall2.o $(OBJDIR)/pxwide2.o $(OBJDIR)/pxquad.o $(OBJDIR)/windows.o $(OBJDIR)/brush.o $(OBJDIR)/realpath.o $(OBJDIR)/mountlist.o $(OBJDIR)/input.o $(OBJDIR)/hotkeys.o $(OBJDIR)/transform.o $(OBJDIR)/pversion.o $(OBJDIR)/factory.o $(PLATFORMOBJ) $(OBJDIR)/fileformats.o $(OBJDIR)/miscfileformats.o $(OBJDIR)/libraw2crtc.o $(OBJDIR)/brush_ops.o $(OBJDIR)/buttons_effects.o $(OBJDIR)/layers.o

SKIN_FILES = ../share/grafx2/skins/skin_classic.png ../share/grafx2/skins/skin_modern.png ../share/grafx2/skins/skin_DPaint.png ../share/grafx2/skins/font_Classic.png ../share/grafx2/skins/font_Fun.png ../share/grafx2/skins/font_Fairlight.png ../share/grafx2/skins/font_Melon.png ../share/grafx2/skins/font_DPaint.png ../share/grafx2/skins/skin_scenish.png ../share/grafx2/skins/font_Seen.png ../share/grafx2/skins/skin_Aurora.png ../share/grafx2/skins/skin_Clax3.gif ../share/grafx2/skins/skin_Clax2.gif

SCRIPT_FILES1 = ../share/grafx2/scripts/bru_db_Amigaball.lua ../share/grafx2/scripts/bru_db_ColorSphere.lua ../share/grafx2/scripts/bru_db_FindAA.lua ../share/grafx2/scripts/bru_db_Fisheye.lua ../share/grafx2/scripts/bru_db_GrayscaleAvg.lua ../share/grafx2/scripts/bru_db_GrayscaleDesat.lua ../share/grafx2/scripts/bru_db_Halfsmooth.lua ../share/grafx2/scripts/bru_db_Mandelbrot.lua ../share/grafx2/scripts/bru_db_Waves.lua ../share/grafx2/scripts/pal_db_Desaturate.lua ../share/grafx2/scripts/pal_db_ExpandColors.lua ../share/grafx2/scripts/pal_db_FillColorCube.lua ../share/grafx2/scripts/pal_db_InvertedRGB.lua ../share/grafx2/scripts/pal_db_Set3bit.lua ../share/grafx2/scripts/pal_db_Set6bit.lua ../share/grafx2/scripts/pal_db_SetC64Palette.lua ../share/grafx2/scripts/pal_db_ShiftHue.lua ../share/grafx2/scripts/pic_db_Pic2isometric.lua ../share/grafx2/scripts/pic_db_Rainbow-Dark2Bright.lua ../share/grafx2/scripts/pic_db_SierpinskyCarpet.lua
SCRIPT_FILES2 = ../share/grafx2/scripts/pic_db_SierpinskyTriangle.lua ../share/grafx2/scripts/pic_ni_Colorspace12bit.lua ../share/grafx2/scripts/pic_ni_Colorspace15bit.lua ../share/grafx2/scripts/pic_ni_Colorspace18bit.lua ../share/grafx2/scripts/pic_ni_GlassGridFilter.lua ../share/grafx2/scripts/pic_ni_Grid8.lua ../share/grafx2/scripts/pic_ni_Grid8red.lua ../share/grafx2/scripts/pic_ni_GridIso.lua ../share/grafx2/scripts/pic_ni_PaletteX1.lua ../share/grafx2/scripts/pic_ni_PaletteX8.lua ../share/grafx2/scripts/scn_db_RemapImage2RGB.lua ../share/grafx2/scripts/scn_db_RemapImage2RGB_ed.lua ../share/grafx2/scripts/scn_db_RemapImageTo3bitPal.lua
SCRIPT_FILES1= SCRIPT_FILES1 SCRIPT_FILES2

FONT_FILES = ../share/grafx2/fonts/8pxfont.png ../share/grafx2/fonts/Tuffy.ttf ../share/grafx2/fonts/PF_Arma_5__.png ../share/grafx2/fonts/PF_Easta_7_.png ../share/grafx2/fonts/PF_Easta_7__.png ../share/grafx2/fonts/PF_Ronda_7__.png ../share/grafx2/fonts/PF_Tempesta_5.png ../share/grafx2/fonts/PF_Tempesta_5_.png ../share/grafx2/fonts/PF_Tempesta_5__.png ../share/grafx2/fonts/PF_Tempesta_5___.png ../share/grafx2/fonts/PF_Tempesta_7.png ../share/grafx2/fonts/PF_Tempesta_7_.png ../share/grafx2/fonts/PF_Tempesta_7__.png ../share/grafx2/fonts/PF_Tempesta_7___.png ../share/grafx2/fonts/PF_Westa_7_.png ../share/grafx2/fonts/PF_Westa_7__.png


ifeq ($(PLATFORM),Darwin)
all : $(MACAPPEXE)
$(MACAPPEXE) : $(BIN)
	rm -rf Grafx2.app
	mkdir -p Grafx2.app Grafx2.app/Contents Grafx2.app/Contents/Frameworks Grafx2.app/Contents/MacOS Grafx2.app/Contents/Resources
	echo 'APPL????' > Grafx2.app/Contents/PkgInfo
	cp Info.plist Grafx2.app/Contents
	cp -r English.lproj Grafx2.app/Contents/Resources
	cp -r fonts Grafx2.app/Contents/Resources
	cp -r skins Grafx2.app/Contents/Resources
	cp -r gfx2def.ini Grafx2.app/Contents/Resources
	cp -Rp $(FWDIR)/SDL.framework Grafx2.app/Contents/Frameworks
	cp -Rp $(FWDIR)/SDL_image.framework Grafx2.app/Contents/Frameworks
	cp -Rp $(FWDIR)/SDL_ttf.framework Grafx2.app/Contents/Frameworks
	cp $(BIN) $(MACAPPEXE)
else
all : $(BIN)
endif


debug : $(BIN)

# Make release will strip the executable to make it smaller but non-debugable
release : version $(BIN)
	$(STRIP) $(BIN)

# Create a zip archive ready for upload to the website, including binaries and sourcecode
ziprelease: version $(BIN) release
	echo `sed "s/.*=\"\(.*\)\";/\1/" pversion.c`.`svnversion` | tr " :" "_-" | sed -e "s/\(wip\)\\./\1/I" > $(OBJDIR)/versiontag

	tar cvzf "../src-`cat $(OBJDIR)/versiontag`.tgz" --strip=1 ../src/*.c ../src/*.h ../src/Makefile ../src/Makefile.dep ../src/gfx2.ico 
	cd .. && $(ZIP) $(ZIPOPT) "grafx2-`cat $(OBJDIR:../%=%)/versiontag`$(TTFLABEL)-$(PLATFORM).$(ZIP)" $(BIN:../%=%) share/grafx2/gfx2def.ini $(SCRIPT_FILES:../%=%) $(SKIN_FILES:../%=%) share/grafx2/gfx2.gif share/icons/grafx2.svg doc/README.txt doc/COMPILING.txt doc/gpl-2.0.txt doc/PF_fonts.txt $(FONT_FILES:../%=%) doc/README-zlib1.txt doc/README-SDL.txt doc/README-SDL_image.txt doc/README-SDL_ttf.txt doc/README-lua.txt src-`cat $(OBJDIR:../%=%)/versiontag`.tgz $(PLATFORMFILES:../%=%)
	$(DELCOMMAND) "../src-`cat $(OBJDIR)/versiontag`.tgz"
	tar cvzf "../grafx2-`cat $(OBJDIR)/versiontag`$(TTFLABEL)-src.tgz" --strip=1 --transform 's,^,grafx2/,g' ../src/*.c ../src/*.h ../src/Makefile ../src/Makefile.dep ../share/grafx2/gfx2def.ini $(SCRIPT_FILES) $(SKIN_FILES) ../src/gfx2.ico ../share/grafx2/gfx2.gif ../share/icons/grafx2.svg ../doc/README.txt ../doc/COMPILING.txt ../doc/gpl-2.0.txt ../doc/PF_fonts.txt ../misc/unix/grafx2.1 ../misc/unix/grafx2.xpm ../misc/unix/grafx2.desktop $(FONT_FILES)
	$(DELCOMMAND) "$(OBJDIR)/versiontag"

testsed :

$(BIN) : $(OBJ)
	test -d ../bin || $(MKDIR) ../bin
	$(CC) $(OBJ) -o $(BIN) $(LOPT) $(LFLAGS)

# SVN revision number
version.c :
	echo "char SVN_revision[]=\"`svnversion .`\";" > version.c
ifeq ($(LABEL),)
else
	echo "char Program_version[]=\"$(LABEL)\";" > pversion.c
endif

version : delversion delpversion version.c pversion.c $(OBJDIR)/version.o $(OBJDIR)/pversion.o all


delversion :
	$(DELCOMMAND) version.c
	
delpversion :
ifeq ($(LABEL),)
else
	$(DELCOMMAND) pversion.c
endif

$(OBJDIR)/%.o : %.c
	$(if $(wildcard $(OBJDIR)),,$(MKDIR) $(OBJDIR))
	$(CC) $(COPT) $(CFLAGS) -c $*.c -o $(OBJDIR)/$*.o

$(OBJDIR)/%.o : %.m
	$(if $(wildcard $(OBJDIR)),,$(MKDIR) $(OBJDIR))
	$(CC) $(COPT) -c $*.m -o $(OBJDIR)/$*.o

depend :
	$(CC) -MM *.c | sed 's:^[^ ]:$$(OBJDIR)/&:' > Makefile.dep

# Link the icons to the program under windows
$(OBJDIR)/winres.o : gfx2.ico
	echo "1 ICON \"gfx2.ico\"" | $(WINDRES) -o $(OBJDIR)/winres.o

# Compile the C++ file needed in Haiku to use the API
$(OBJDIR)/haiku.o : haiku.cpp
	g++ -c haiku.cpp -o $(OBJDIR)/haiku.o

clean :
	$(DELCOMMAND) $(OBJ)
	$(DELCOMMAND) $(BIN)

ifneq ($(PLATFORM),amiga-vbcc)
# Linux installation of the program
install : $(BIN)
	# Create dirs
	test -d $(DESTDIR)$(bindir) || $(MKDIR) $(DESTDIR)$(bindir)
	test -d $(DESTDIR)$(datadir)/grafx2 || $(MKDIR) $(DESTDIR)$(datadir)/grafx2
	test -d $(DESTDIR)$(datadir)/grafx2/fonts || $(MKDIR) $(DESTDIR)$(datadir)/grafx2/fonts
	test -d $(DESTDIR)$(datadir)/grafx2/skins || $(MKDIR) $(DESTDIR)$(datadir)/grafx2/skins
	test -d $(DESTDIR)$(datadir)/grafx2/scripts || $(MKDIR) $(DESTDIR)$(datadir)/grafx2/scripts
	test -d $(DESTDIR)$(datadir)/applications || $(MKDIR) $(DESTDIR)$(datadir)/applications
	test -d $(DESTDIR)$(datadir)/icons || $(MKDIR) $(DESTDIR)$(datadir)/icons
	# Copy files
	$(CP) $(BIN) $(DESTDIR)$(bindir)
	$(CP) ../share/grafx2/gfx2def.ini $(DESTDIR)$(datadir)/grafx2/
	$(CP) ../share/grafx2/gfx2.gif $(DESTDIR)$(datadir)/grafx2/
	$(CP) ../share/grafx2/fonts/* $(DESTDIR)$(datadir)/grafx2/fonts/
	$(CP) $(SKIN_FILES) $(DESTDIR)$(datadir)/grafx2/skins/
	$(CP) $(SCRIPT_FILES) $(DESTDIR)$(datadir)/grafx2/scripts/
	# Icon and desktop file for debian
	$(CP) ../misc/unix/grafx2.desktop $(DESTDIR)$(datadir)/applications/
	$(CP) ../misc/unix/grafx2.xpm $(DESTDIR)$(datadir)/icons/
	$(CP) ../share/icons/grafx2.svg $(DESTDIR)$(datadir)/icons/
	@echo Install complete
  
# Linux uninstallation of the program
uninstall :
	$(DELCOMMAND) $(DESTDIR)$(bindir)/grafx2
	$(DELCOMMAND) $(DESTDIR)$(datadir)/grafx2/gfx2def.ini
	$(DELCOMMAND) $(DESTDIR)$(datadir)/grafx2/gfx2.gif
	$(DELCOMMAND) $(DESTDIR)$(datadir)/grafx2/fonts/*
	$(if $(wildcard $(DESTDIR)$(datadir)/grafx2/fonts),,$(RMDIR) $(DESTDIR)$(datadir)/grafx2/fonts)
	$(DELCOMMAND) $(SKIN_FILES:../share%=$(DESTDIR)$(datadir)%)
	$(if $(wildcard $(DESTDIR)$(datadir)/grafx2/skins),,$(RMDIR) $(DESTDIR)$(datadir)/grafx2/skins)
	$(DELCOMMAND) $(SCRIPT_FILES:../share%=$(DESTDIR)$(datadir)%)
	$(if $(wildcard $(DESTDIR)$(datadir)/grafx2/scripts),,$(RMDIR) $(DESTDIR)$(datadir)/grafx2/scripts)
	$(if $(wildcard $(DESTDIR)$(datadir)/grafx2),,$(RMDIR) $(DESTDIR)$(datadir)/grafx2)
	# Icon and desktop file for debian
	$(DELCOMMAND) $(DESTDIR)$(datadir)/applications/grafx2.desktop
	$(DELCOMMAND) $(DESTDIR)$(datadir)/icons/grafx2.xpm
	$(DELCOMMAND) $(DESTDIR)$(datadir)/icons/grafx2.svg
	@echo Uninstall complete

endif

-include Makefile.dep

