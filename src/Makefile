#  Grafx2 - The Ultimate 256-color bitmap paint program
#  
#  Copyright 2011 Franck Charlet
#  Copyright 2011 Pawel Góralski
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
  pixmapdir = $(datarootdir)/icons

# Compile with OPTIM=0 to disable gcc optimizations, to enable debug.
  STRIP = strip

### Specific to build MAC OS X universal binaries on Lion ###
### (may need to be changed or removed depedning on the OSX version) ###

MACOSX_SYSROOT = /Developer/SDKs/MacOSX10.6.sdk
MACOSX_ARCH = -arch x86_64 -arch i386

### PLATFORM DETECTION AND CONFIGURATION ###

PLATFORMOBJ =

# There is no uname under windows, but we can guess we are there with the COMSPEC env.var
# Windows specific
ifdef COMSPEC
  DELCOMMAND = rm -f
  MKDIR = mkdir -p
  RMDIR = rmdir --ignore-fail-on-non-empty
  CP = cp
  BIN = ../bin/grafx2.exe
  COPT = -W -Wall -Wdeclaration-after-statement -O$(OPTIM) -g -ggdb `sdl-config --cflags` $(TTFCOPT) $(JOYCOPT) $(VKEYCOPT) $(LUACOPT) $(LAYERCOPT)
  LOPT = `sdl-config --libs` -lSDL_image $(TTFLOPT) -lpng14 $(LUALOPT)
  LUALOPT = -llua
  CC = gcc
  OBJDIR = ../obj/win32
  # Resources (icon)
  WINDRES = windres.exe
  PLATFORMOBJ = $(OBJDIR)/winres.o
  PLATFORM = win32
  #some misc files we have to add to the release archive under windows.
  PLATFORMFILES = bin/SDL.dll bin/SDL_image.dll bin/libpng14-14.dll bin/zlib1.dll $(TTFLIBS)
  ZIP = zip
else

#For all other platforms, we can rely on uname
  PLATFORM = $(shell uname)

  ifeq ($(PLATFORM),AmigaOS) # 1
    #AmigaOS (3 or 4) specific
    DELCOMMAND = rm -rf
    MKDIR = mkdir -p
    RMDIR = rmdir --ignore-fail-on-non-empty
    CP = cp
    BIN = ../bin/grafx2
    COPT = -Wall -c -gstabs $(shell sdl-config --cflags) $(TTFCOPT)
    LOPT = $(shell sdl-config --libs) -lSDL_image -lpng -ljpeg -lz $(TTFLOPT) -lft2
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
    RMDIR = rmdir --ignore-fail-on-non-empty
    # Force it
    OPTIM = 3
    CP = cp
    ZIP = zip
    PLATFORMFILES = gfx2.png
    # Where the SDL frameworks are located
    FWDIR = /Library/Frameworks
    BIN = ../bin/grafx2
    SVN_REVISION = $(shell svnversion | cut -f2 -d ":" - | tr -d "M")

    # these are for use with macports
    #SDLCOPT = $(shell sdl-config --cflags)
    #SDLLOPT = $(shell sdl-config --libs) $(shell pkg-config --libs SDL_image)
    #LUACOPT = $(shell pkg-config lua --cflags --silence-errors ||pkg-config lua5.1 --cflags --silence-errors ||pkg-config lua-5.1 --cflags)
    #LUALOPT = $(shell pkg-config lua --libs --silence-errors ||pkg-config lua5.1 --libs --silence-errors ||pkg-config lua-5.1 --libs)

    # these are for use with Mac OS X native frameworks
    #-framework SDL_ttf 
    SDLLOPT = -isysroot $(MACOSX_SYSROOT) $(MACOSX_ARCH) -L/usr/lib -framework SDL -framework SDL_image -framework Cocoa -framework Carbon -framework OpenGL
    SDLCOPT = $(MACOSX_ARCH) -I$(FWDIR)/SDL.framework/Headers -I$(FWDIR)/SDL_image.framework/Headers -I$(FWDIR)/SDL_ttf.framework/Headers -D_THREAD_SAFE
    LUACOPT = -I$(FWDIR)/Lua.framework/Headers
    LUALOPT = -framework lua


    # these are for everyone
    COPT = -D_DARWIN_C_SOURCE -D__macosx__ -D__linux__ -W -Wall -Wdeclaration-after-statement -O$(OPTIM) -std=c99 -c -g $(LUACOPT) $(SDLCOPT) $(TTFCOPT) -I/usr/X11/include
    LOPT = $(SDLLOPT) $(LUALOPT) -framework libpng14 -lz
    # Use gcc for compiling. Use ncc to build a callgraph and analyze the code.
    CC = gcc
    #CC = nccgen -ncgcc -ncld -ncfabs
    OBJDIR = ../obj/macosx
    PLATFORMOBJ = $(OBJDIR)/SDLMain.o
    X11LOPT = 
    MACAPPEXE = Grafx2.app/Contents/MacOS/Grafx2
    NOTTF = 1
  else
  ifeq ($(PLATFORM),AROS) # 3
    #AROS specific
    DELCOMMAND = rm -rf
    MKDIR = mkdir -p
    RMDIR = rmdir --ignore-fail-on-non-empty
    CP = cp
    BIN = ../bin/grafx2
    COPT = -Wall -g $(shell sdl-config --cflags) $(TTFCOPT)
    LOPT = -lSDL_image $(shell sdl-config --libs) -lpng -ljpeg -lz $(TTFLOPT) -lfreetype2shared
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
    RMDIR = rmdir --ignore-fail-on-non-empty
    CP = cp
    BIN = ../bin/grafx2
    COPT = -Wall -gstabs -c $(shell sdl-config --cflags) $(TTFCOPT)
    LOPT = -lSDL_image $(shell sdl-config --libs) -lpng -ljpeg -lz $(TTFLOPT)
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
    RMDIR = rmdir --ignore-fail-on-non-empty
    CP = cp
    BIN = ../bin/grafx2
    COPT = -W -Wall -c -g $(shell sdl-config --cflags) $(TTFCOPT) -I/boot/home/config/include
    LOPT = $(shell sdl-config --libs) -lSDL_image -lpng -ljpeg -lz $(TTFLOPT)
    CC = gcc
    OBJDIR = ../obj/beos
    ZIP = zip

  else
  ifeq ($(PLATFORM),Haiku) # 7
    #Haiku specific
    DELCOMMAND = rm -rf
    MKDIR = mkdir -p
    RMDIR = rmdir --ignore-fail-on-non-empty
    CP = cp
    BIN = ../bin/grafx2
    PLATFORMOBJ = $(OBJDIR)/haiku.o
      ifeq ($(NOLUA),1)
        LUACOPT =
        LUALOPT =
      else
        LUACOPT = -D__ENABLE_LUA__
        LUALOPT = -llua
      endif
    COPT = -W -Wall -c -g $(shell sdl-config --cflags) $(TTFCOPT) -I/boot/common/include $(LUACOPT)
    LOPT = $(shell sdl-config --libs) -lSDL_image -lpng -ljpeg -lz $(TTFLOPT) -lfreetype -lbe $(LUALOPT)
    CC = gcc
    OBJDIR = ../obj/haiku
    ZIP = zip

  else
  ifeq ($(PLATFORM),skyos) # 8
    #SkyOS specific
    DELCOMMAND = rm -rf
    MKDIR = mkdir -p
    RMDIR = rmdir --ignore-fail-on-non-empty
    CP = cp
    BIN = ../bin/grafx2
    COPT = -W -Wall -Wdeclaration-after-statement -c -g $(shell sdl-config --cflags) $(TTFCOPT)
    LOPT = $(shell sdl-config --libs) -lSDL_image -lpng -ljpeg -lz $(TTFLOPT)
    CC = gcc
    OBJDIR = ../obj/skyos
    ZIP = zip

  else
  ifeq ($(PLATFORM),OSF1) #9
    #OSF1 / tru64 alpha
    DELCOMMAND = rm -rf
    MKDIR = mkdir -p
    RMDIR = rmdir --ignore-fail-on-non-empty
    CP = cp
    ZIP = zip
    PLATFORMFILES = gfx2.png
    BIN = ../bin/grafx2
    COPT = -W -Wall -std=c99 -c -g -gstabs -D__TRU64__ $(shell sdl-config --cflags) $(TTFCOPT) $(LUACOPT)
    LOPT = $(shell sdl-config --libs) -lSDL_image $(TTFLOPT) -lpng $(LUALOPT) -lm
    OBJDIR = ../obj/unix
    X11LOPT = -lX11
    CC = gcc
  else
  ifeq ($(PLATFORM),FreeMiNT) #10
  	#Atari FreeMiNT/TOS specific
        DELCOMMAND = rm -rf
        MKDIR = mkdir -p
        RMDIR = rmdir
        CP = cp
        ZIP = zip
        PLATFORMFILES = gfx2.png
        CC = gcc
        BIN = ../bin/grafx2.ttp
	LUALOPT = -llua
        OBJDIR = ../obj/m68k-atari-mint
        PLATFORM = m68k-atari-mint
        STRIP = strip -s 
	STACK = stack
	FIX_FLAGS = flags
	X11LOPT = 
	COPT = -W -Wall -m68020-60 -fomit-frame-pointer -std=c99 -Wdeclaration-after-statement -D__MINT__ -DNO_INLINE_MATH -O$(OPTIM) -c -I$(prefix)/include `$(prefix)/bin/libpng12-config --cflags` `$(prefix)/bin/sdl-config --cflags` $(JOYCOPT) $(LAYERCOPT) $(LUACOPT)
    	LOPT = -static -m68020-60 -lSDL_image `$(prefix)/bin/sdl-config --libs` -L$(prefix)/lib -ltiff -ljpeg `$(prefix)/bin/libpng12-config --libs` -lz -lm $(TTFLOPT) -lfreetype $(LUALOPT) $(LAYERLOPT)
  else
  ifeq ($(PLATFORM),syllable) #10
	#Syllable
    DELCOMMAND = rm -rf
    MKDIR = mkdir -p
    RMDIR = rmdir --ignore-fail-on-non-empty
    CP = cp
    ZIP = zip
    PLATFORMFILES = gfx2.png
    LUACOPT = -I/resources/indexes/include
    LUALOPT = -llua
    BIN = ../bin/grafx2
    COPT = -W -Wall -Wdeclaration-after-statement -std=c99 -c -g `sdl-config --cflags` -I/resources/indexes/include/SDL $(TTFCOPT) $(LUACOPT) $(JOYCOPT) $(VKEYCOPT) -O$(OPTIM)
    LOPT = `sdl-config --libs` -lSDL_image $(TTFLOPT) -lpng -lz $(LUALOPT) -lm
    CC = gcc
    OBJDIR = ../obj/syllable
    X11LOPT = 
  else
    # Finally, the default rules that work fine for most unix/gcc systems, linux and freebsd are tested.
    # Linux and FreeBSD specific (default rules)
    DELCOMMAND = rm -rf
    MKDIR = mkdir -p
    RMDIR = rmdir --ignore-fail-on-non-empty
    CP = cp
    ZIP = zip
    PLATFORMFILES = gfx2.png
    ifneq ($(ATARICROSS),1)
      ifeq ($(NOLUA),1)
        LUACOPT =
        LUALOPT =
      else
        LUACOPT = `pkg-config lua --cflags --silence-errors ||pkg-config lua5.1 --cflags --silence-errors ||pkg-config lua-5.1 --cflags`
        LUALOPT = `pkg-config lua --libs --silence-errors ||pkg-config lua5.1 --libs --silence-errors ||pkg-config lua-5.1 --libs`
      endif
    endif
    
    # These can only be used under linux and maybe freebsd. They allow to compile for the gp2x or to create a windows binary
    ifdef WIN32CROSS
      #cross compile a Win32 executable
      CC = i586-mingw32msvc-gcc
      BIN = ../bin/grafx2.exe
      COPT = -W -Wall -Wdeclaration-after-statement -O$(OPTIM) -g -ggdb -Dmain=SDL_main $(shell /usr/local/cross-tools/i386-mingw32/bin/sdl-config --cflags) $(TTFCOPT)
      LOPT = -mwindows -lmingw32 -lSDLmain -lSDL -lshlwapi $(shell /usr/local/cross-tools/i386-mingw32/bin/sdl-config --libs) -lSDL_image $(TTFLOPT)
      OBJDIR = ../obj/win32
      PLATFORM = win32
    else
      ifdef GP2XCROSS
        #cross compile an exec for the gp2x
        CC = /opt/open2x/gcc-4.1.1-glibc-2.3.6/arm-open2x-linux/bin/arm-open2x-linux-gcc
        BIN = ../bin/grafx2.gpe
        COPT = -W -Wall -Wdeclaration-after-statement -pedantic -std=c99 -static -g -O$(OPTIM) -I/opt/open2x/gcc-4.1.1-glibc-2.3.6/include `/opt/open2x/gcc-4.1.1-glibc-2.3.6/bin/sdl-config --cflags` $(TTFCOPT) -D__GP2X__ $(TTFCOPT) $(JOYCOPT) $(VKEYCOPT) $(LUACOPT) $(LAYERCOPT)
        LOPT = -static -lSDL_image `/opt/open2x/gcc-4.1.1-glibc-2.3.6/bin/sdl-config --static-libs` -ljpeg -lpng -lz -lm $(TTFLOPT) $(LUALOPT)
        OBJDIR = ../obj/gp2x
        NOTTF = 1
        PLATFORM = gp2x
        STRIP = /opt/open2x/gcc-4.1.1-glibc-2.3.6/arm-open2x-linux/bin/arm-open2x-linux-strip
        JOYCOPT = -DUSE_JOYSTICK

      else
      ifdef AROS32CROSS
	    #cross compile an Aros 32 bit executable
        BIN = ../bin/grafx2
        COPT = -Wall -g `i386-linux-aros-sdl-config --cflags` $(TTFCOPT)
        LOPT = -lSDL_image `i386-linux-aros-sdl-config --libs` -lpng -ljpeg -lz $(TTFLOPT) -lfreetype2shared
        CC = i386-aros-gcc
        OBJDIR = ../obj/aros
        STRIP = strip --strip-unneeded --remove-section .comment
        PLATFORM = AROS
        ZIP = lha
        ZIPOPT = a
      else
      ifdef ATARICROSS
        #cross compile an exec for atari TOS/MiNT machine
        CC = m68k-atari-mint-gcc
        BIN = ../bin/grafx2.ttp
	LUALOPT = -llua
        OBJDIR = ../obj/m68k-atari-mint
        PLATFORM = m68k-atari-mint
        STRIP = m68k-atari-mint-strip -s 
	STACK = m68k-atari-mint-stack
	FIX_FLAGS = m68k-atari-mint-flags
	X11LOPT = 
	COPT = -W -Wall -m68020-60 -fomit-frame-pointer -std=c99 -Wdeclaration-after-statement -D__MINT__ -ffast-math -O$(OPTIM) -c -I$(prefix)/include `$(prefix)/bin/libpng12-config --cflags` `$(prefix)/bin/sdl-config --cflags` $(JOYCOPT) $(LAYERCOPT) $(LUACOPT)
    	LOPT = -static -m68020-60 -lSDL_image `$(prefix)/bin/sdl-config --libs` -L$(prefix)/lib -ltiff -ljpeg `$(prefix)/bin/libpng12-config --libs` -lz -lm $(TTFLOPT) -lfreetype $(LUALOPT) -lm $(LAYERLOPT) -Wl,--stack,8k 
      else

        # Compiles a regular linux executable for the native platform
        BIN = ../bin/grafx2
        COPT = -W -Wall -Wdeclaration-after-statement -std=c99 -c -g `sdl-config --cflags` $(TTFCOPT) $(LUACOPT) $(JOYCOPT) $(VKEYCOPT) -O$(OPTIM) -I/usr/local/include/libpng
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
  endif
  endif
  endif
endif

### BUILD SETTINGS are set according to vars set in the platform selection, 
### the "overridable defaults", and environment variables set before launching make

#TrueType is optional: make NOTTF=1 to disable support and dependencies.
ifndef ($(ATARICROSS,1))
 ifeq ($(NOTTF),1)
   TTFCOPT = -DNOTTF=1
   TTFLOPT =
   TTFLIBS =
   TTFLABEL = -nottf
 else
   TTFCOPT = 
   TTFLOPT = -L$(prefix)/lib -lSDL_ttf $(X11LOPT)
   TTFLIBS = bin/libfreetype-6.dll bin/SDL_ttf.dll
   TTFLABEL = 
 endif
else
 ifeq ($(NOTTF),1)
   TTFCOPT = -DNOTTF=1
   TTFLOPT =
   TTFLIBS =
   TTFLABEL = -nottf
 else
   TTFCOPT = 
   TTFLOPT = -L$(prefix)/lib -lSDL_ttf $(X11LOPT)
   TTFLIBS =  
   TTFLABEL = 
 endif
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

#To enable virtual keyboard input (mouse-driven), make VIRT_KEY=1
#This is automatically enabled on some platforms, but this
#switch allows you to test the virtual keyboard on any other platform.
ifeq ($(VIRT_KEY),1)
  VKEYCOPT = -DVIRT_KEY
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
OBJ = $(OBJDIR)/main.o $(OBJDIR)/init.o $(OBJDIR)/graph.o $(OBJDIR)/sdlscreen.o  $(OBJDIR)/misc.o $(OBJDIR)/special.o $(OBJDIR)/buttons.o $(OBJDIR)/palette.o $(OBJDIR)/help.o $(OBJDIR)/operatio.o $(OBJDIR)/pages.o $(OBJDIR)/loadsave.o $(OBJDIR)/readline.o $(OBJDIR)/engine.o $(OBJDIR)/filesel.o $(OBJDIR)/op_c.o $(OBJDIR)/readini.o $(OBJDIR)/saveini.o $(OBJDIR)/shade.o $(OBJDIR)/keyboard.o $(OBJDIR)/io.o $(OBJDIR)/version.o $(OBJDIR)/text.o $(OBJDIR)/SFont.o $(OBJDIR)/setup.o $(OBJDIR)/pxsimple.o $(OBJDIR)/pxtall.o $(OBJDIR)/pxwide.o $(OBJDIR)/pxdouble.o $(OBJDIR)/pxtriple.o $(OBJDIR)/pxtall2.o $(OBJDIR)/pxwide2.o $(OBJDIR)/pxquad.o $(OBJDIR)/windows.o $(OBJDIR)/brush.o $(OBJDIR)/realpath.o $(OBJDIR)/mountlist.o $(OBJDIR)/input.o $(OBJDIR)/hotkeys.o $(OBJDIR)/transform.o $(OBJDIR)/pversion.o $(OBJDIR)/factory.o $(PLATFORMOBJ) $(OBJDIR)/fileformats.o $(OBJDIR)/miscfileformats.o $(OBJDIR)/libraw2crtc.o $(OBJDIR)/brush_ops.o $(OBJDIR)/buttons_effects.o $(OBJDIR)/layers.o $(OBJDIR)/oldies.o $(OBJDIR)/tiles.o $(OBJDIR)/colorred.o

SKIN_FILES = ../share/grafx2/skins/skin_classic.png ../share/grafx2/skins/skin_modern.png ../share/grafx2/skins/skin_DPaint.png ../share/grafx2/skins/font_Classic.png ../share/grafx2/skins/font_Fun.png ../share/grafx2/skins/font_Fairlight.png ../share/grafx2/skins/font_Melon.png ../share/grafx2/skins/font_DPaint.png ../share/grafx2/skins/skin_scenish.png ../share/grafx2/skins/font_Seen.png ../share/grafx2/skins/skin_Aurora.png

SCRIPT_FILES = ../share/grafx2/scripts/samples_2.4/brush/ApplyColor.lua ../share/grafx2/scripts/samples_2.4/brush/Fisheye.lua ../share/grafx2/scripts/samples_2.4/brush/GrayscaleAvg.lua ../share/grafx2/scripts/samples_2.4/brush/GrayscaleDesat.lua ../share/grafx2/scripts/samples_2.4/brush/Halfsmooth.lua ../share/grafx2/scripts/samples_2.4/brush/Waves.lua ../share/grafx2/scripts/samples_2.4/demo/3DPalette.lua ../share/grafx2/scripts/samples_2.4/demo/Ellipse.lua ../share/grafx2/scripts/samples_2.4/demo/FlipPicture.lua \
../share/grafx2/scripts/samples_2.4/demo/SierpinskyCarpet.lua ../share/grafx2/scripts/samples_2.4/demo/SierpinskyTriangle.lua ../share/grafx2/scripts/samples_2.4/demo/Spritesheet.lua ../share/grafx2/scripts/samples_2.4/demo/brush/Amigaball.lua ../share/grafx2/scripts/samples_2.4/demo/brush/ColorSphere.lua ../share/grafx2/scripts/samples_2.4/demo/brush/FindAA.lua ../share/grafx2/scripts/samples_2.4/demo/brush/Mandelbrot.lua ../share/grafx2/scripts/samples_2.4/libs/dawnbringer_lib.lua ../share/grafx2/scripts/samples_2.4/libs/memory.lua \
../share/grafx2/scripts/samples_2.4/palette/Desaturate.lua ../share/grafx2/scripts/samples_2.4/palette/ExpandColors.lua ../share/grafx2/scripts/samples_2.4/palette/FillColorCube.lua ../share/grafx2/scripts/samples_2.4/palette/InvertedRGB.lua ../share/grafx2/scripts/samples_2.4/palette/Set3bit.lua ../share/grafx2/scripts/samples_2.4/palette/Set6bit.lua ../share/grafx2/scripts/samples_2.4/palette/SetC64Palette.lua ../share/grafx2/scripts/samples_2.4/palette/ShiftHue.lua ../share/grafx2/scripts/samples_2.4/picture/CellColourReducer.lua \
../share/grafx2/scripts/samples_2.4/picture/DrawGridIsometric.lua ../share/grafx2/scripts/samples_2.4/picture/DrawgridOrthogonal_Index.lua ../share/grafx2/scripts/samples_2.4/picture/DrawGridOrthogonal_RGB.lua ../share/grafx2/scripts/samples_2.4/picture/GlassGridFilter.lua ../share/grafx2/scripts/samples_2.4/picture/PaletteToPicture.lua ../share/grafx2/scripts/samples_2.4/picture/Pic2isometric.lua ../share/grafx2/scripts/samples_2.4/picture/Rainbow-Dark2Bright.lua ../share/grafx2/scripts/samples_2.4/picture/RemapImage2RGB.lua \
../share/grafx2/scripts/samples_2.4/picture/RemapImage2RGB_ed.lua ../share/grafx2/scripts/samples_2.4/picture/RemapImageTo3bitPal.lua ../share/grafx2/scripts/samples_2.4/picture/XBitColourXpaceFromPalette.lua

FONT_FILES = ../share/grafx2/fonts/8pxfont.png ../share/grafx2/fonts/Tuffy.ttf ../share/grafx2/fonts/PF_Arma_5__.png ../share/grafx2/fonts/PF_Easta_7_.png ../share/grafx2/fonts/PF_Easta_7__.png ../share/grafx2/fonts/PF_Ronda_7__.png ../share/grafx2/fonts/PF_Tempesta_5.png ../share/grafx2/fonts/PF_Tempesta_5_.png ../share/grafx2/fonts/PF_Tempesta_5__.png ../share/grafx2/fonts/PF_Tempesta_5___.png ../share/grafx2/fonts/PF_Tempesta_7.png ../share/grafx2/fonts/PF_Tempesta_7_.png ../share/grafx2/fonts/PF_Tempesta_7__.png ../share/grafx2/fonts/PF_Tempesta_7___.png ../share/grafx2/fonts/PF_Westa_7_.png ../share/grafx2/fonts/PF_Westa_7__.png


ifeq ($(PLATFORM),Darwin)
all : $(MACAPPEXE)
$(MACAPPEXE) : $(BIN)
	rm -rf Grafx2.app
	mkdir -p Grafx2.app Grafx2.app/Contents Grafx2.app/Contents/Frameworks Grafx2.app/Contents/MacOS Grafx2.app/Contents/Resources Grafx2.app/Contents/Resources/scripts
	mkdir -p Grafx2.app/Contents/Resources/scripts/samples_2.4
	mkdir -p Grafx2.app/Contents/Resources/scripts/samples_2.4/brush
	mkdir -p Grafx2.app/Contents/Resources/scripts/samples_2.4/demo
	mkdir -p Grafx2.app/Contents/Resources/scripts/samples_2.4/libs
	mkdir -p Grafx2.app/Contents/Resources/scripts/samples_2.4/palette
	mkdir -p Grafx2.app/Contents/Resources/scripts/samples_2.4/picture
	echo 'APPL????' > Grafx2.app/Contents/PkgInfo
	cp ../Info.plist Grafx2.app/Contents
	cp -r Grafx2.icns Grafx2.app/Contents/Resources
	cp -r English.lproj Grafx2.app/Contents/Resources
	cp -r ../share/grafx2/fonts Grafx2.app/Contents/Resources
	cp -r ../share/grafx2/skins Grafx2.app/Contents/Resources
	cp -r ../share/grafx2/gfx2def.ini Grafx2.app/Contents/Resources
	cp -r ../share/grafx2/scripts/samples_2.4/brush Grafx2.app/Contents/Resources/scripts/samples_2.4
	cp -r ../share/grafx2/scripts/samples_2.4/demo Grafx2.app/Contents/Resources/scripts/samples_2.4
	cp -r ../share/grafx2/scripts/samples_2.4/libs Grafx2.app/Contents/Resources/scripts/samples_2.4
	cp -r ../share/grafx2/scripts/samples_2.4/palette Grafx2.app/Contents/Resources/scripts/samples_2.4
	cp -r ../share/grafx2/scripts/samples_2.4/picture Grafx2.app/Contents/Resources/scripts/samples_2.4
	mkdir -p Grafx2.app/Contents/Frameworks/Lua.framework/Versions
	mkdir -p Grafx2.app/Contents/Frameworks/libpng14.framework/Versions
	mkdir -p Grafx2.app/Contents/Frameworks/SDL.framework/Versions
	mkdir -p Grafx2.app/Contents/Frameworks/SDL_image.framework/Versions
#	mkdir -p Grafx2.app/Contents/Frameworks/SDL_ttf.framework/Versions
	cp -Rp $(FWDIR)/Lua.framework/Versions/A Grafx2.app/Contents/Frameworks/Lua.framework/Versions
	cp -Rp $(FWDIR)/libpng14.framework/Versions/1.4.8 Grafx2.app/Contents/Frameworks/libpng14.framework/Versions
	cp -Rp $(FWDIR)/SDL.framework/Versions/A Grafx2.app/Contents/Frameworks/SDL.framework/Versions
	cp -Rp $(FWDIR)/SDL_image.framework/Versions/A Grafx2.app/Contents/Frameworks/SDL_image.framework/Versions
#	cp -Rp $(FWDIR)/SDL_ttf.framework/Versions/A Grafx2.app/Contents/Frameworks/SDL_ttf.framework/Versions
    # remove those
	rm -fr Grafx2.app/Contents/Frameworks/SDL.framework/Versions/A/Headers
	rm -fr Grafx2.app/Contents/Frameworks/SDL.framework/Versions/A/Resources
	rm -fr Grafx2.app/Contents/Frameworks/SDL_image.framework/Versions/A/Headers
	rm -fr Grafx2.app/Contents/Frameworks/SDL_image.framework/Versions/A/Resources
	rm -fr Grafx2.app/Contents/Frameworks/Lua.framework/Versions/A/Resources
	rm -fr Grafx2.app/Contents/Frameworks/Lua.framework/Versions/A/Headers
	rm -fr Grafx2.app/Contents/Frameworks/libpng14.framework/Versions/1.4.8/Resources
	rm -fr Grafx2.app/Contents/Frameworks/libpng14.framework/Versions/1.4.8/Headers
#	rm -fr Grafx2.app/Contents/Frameworks/SDL_ttf.framework/Versions
	cp $(BIN) $(MACAPPEXE)
	$(STRIP) -x -X -S $(MACAPPEXE)
	chmod +x $(MACAPPEXE)
	tar cvzf grafx2-svn$(SVN_REVISION)-macosx.tgz --exclude '*svn*' --exclude '*DS_Store*' Grafx2.app/*
else
all : $(BIN)
ifeq ($(ATARICROSS),1)
	$(STACK) -S 128k $(BIN)
	$(FIX_FLAGS) -S $(BIN)
else
ifeq ($(PLATFORM),FreeMiNT)  
	$(STACK) -S 128k $(BIN)
	$(FIX_FLAGS) -S $(BIN)
endif
endif
endif


debug : $(BIN)
ifeq ($(ATARICROSS),1)
	$(STACK) -S 128k $(BIN)
	$(FIX_FLAGS) -S $(BIN)
else
ifeq ($(PLATFORM),FreeMiNT)  
	$(STACK) -S 128k $(BIN)
	$(FIX_FLAGS) -S $(BIN)
endif
endif


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
	test -d $(DESTDIR)$(datadir)/grafx2/scripts/libs || $(MKDIR) $(DESTDIR)$(datadir)/grafx2/scripts/libs
	test -d $(DESTDIR)$(datadir)/grafx2/scripts/samples_2.4 || $(MKDIR) $(DESTDIR)$(datadir)/grafx2/scripts/samples_2.4
	test -d $(DESTDIR)$(datadir)/grafx2/scripts/samples_2.4/brush || $(MKDIR) $(DESTDIR)$(datadir)/grafx2/scripts/samples_2.4/brush
	test -d $(DESTDIR)$(datadir)/grafx2/scripts/samples_2.4/demo || $(MKDIR) $(DESTDIR)$(datadir)/grafx2/scripts/samples_2.4/demo
	test -d $(DESTDIR)$(datadir)/grafx2/scripts/samples_2.4/demo/brush || $(MKDIR) $(DESTDIR)$(datadir)/grafx2/scripts/samples_2.4/demo/brush
	test -d $(DESTDIR)$(datadir)/grafx2/scripts/samples_2.4/libs || $(MKDIR) $(DESTDIR)$(datadir)/grafx2/scripts/samples_2.4/libs
	test -d $(DESTDIR)$(datadir)/grafx2/scripts/samples_2.4/palette || $(MKDIR) $(DESTDIR)$(datadir)/grafx2/scripts/samples_2.4/palette
	test -d $(DESTDIR)$(datadir)/grafx2/scripts/samples_2.4/picture || $(MKDIR) $(DESTDIR)$(datadir)/grafx2/scripts/samples_2.4/picture
	test -d $(DESTDIR)$(datadir)/applications || $(MKDIR) $(DESTDIR)$(datadir)/applications
	test -d $(DESTDIR)$(pixmapdir) || $(MKDIR) $(DESTDIR)$(pixmapdir)
	
	# Copy files
	$(CP) $(BIN) $(DESTDIR)$(bindir)
	$(CP) ../share/grafx2/gfx2def.ini $(DESTDIR)$(datadir)/grafx2/
	$(CP) ../share/grafx2/gfx2.gif $(DESTDIR)$(datadir)/grafx2/
	$(CP) ../share/grafx2/fonts/* $(DESTDIR)$(datadir)/grafx2/fonts/
	$(CP) $(SKIN_FILES) $(DESTDIR)$(datadir)/grafx2/skins/
	for f in $(SCRIPT_FILES); do cp "$$f" "$(DESTDIR)$(bindir)/$$f" ; done
	# Icon and desktop file for debian
	$(CP) ../misc/unix/grafx2.desktop $(DESTDIR)$(datadir)/applications/
	$(CP) ../misc/unix/grafx2.xpm $(DESTDIR)$(pixmapdir)
	$(CP) ../share/icons/grafx2.svg $(DESTDIR)$(pixmapdir)
	@echo Install complete
  
# Linux uninstallation of the program
uninstall :
	$(DELCOMMAND) $(DESTDIR)$(bindir)/grafx2
	$(DELCOMMAND) $(DESTDIR)$(datadir)/grafx2/gfx2def.ini
	$(DELCOMMAND) $(DESTDIR)$(datadir)/grafx2/gfx2.gif
	$(DELCOMMAND) $(DESTDIR)$(datadir)/grafx2/fonts/*
	$(if $(wildcard $(DESTDIR)$(datadir)/grafx2/fonts),$(RMDIR) $(DESTDIR)$(datadir)/grafx2/fonts,)
	$(DELCOMMAND) $(SKIN_FILES:../share%=$(DESTDIR)$(datadir)%)
	$(if $(wildcard $(DESTDIR)$(datadir)/grafx2/skins),$(RMDIR) $(DESTDIR)$(datadir)/grafx2/skins,)
	$(DELCOMMAND) $(SCRIPT_FILES:../share%=$(DESTDIR)$(datadir)%)
	$(if $(wildcard $(DESTDIR)$(datadir)/grafx2/scripts/samples_2.4/picture),$(RMDIR) $(DESTDIR)$(datadir)/grafx2/scripts/samples_2.4/picture,)
	$(if $(wildcard $(DESTDIR)$(datadir)/grafx2/scripts/samples_2.4/palette),$(RMDIR) $(DESTDIR)$(datadir)/grafx2/scripts/samples_2.4/palette,)
	$(if $(wildcard $(DESTDIR)$(datadir)/grafx2/scripts/samples_2.4/libs),$(RMDIR) $(DESTDIR)$(datadir)/grafx2/scripts/samples_2.4/libs,)
	$(if $(wildcard $(DESTDIR)$(datadir)/grafx2/scripts/samples_2.4/demo/brush),$(RMDIR) $(DESTDIR)$(datadir)/grafx2/scripts/samples_2.4/demo/brush,)
	$(if $(wildcard $(DESTDIR)$(datadir)/grafx2/scripts/samples_2.4/demo),$(RMDIR) $(DESTDIR)$(datadir)/grafx2/scripts/samples_2.4/demo,)
	$(if $(wildcard $(DESTDIR)$(datadir)/grafx2/scripts/samples_2.4/brush),$(RMDIR) $(DESTDIR)$(datadir)/grafx2/scripts/samples_2.4/brush,)
	$(if $(wildcard $(DESTDIR)$(datadir)/grafx2/scripts/samples_2.4),$(RMDIR) $(DESTDIR)$(datadir)/grafx2/scripts/samples_2.4,)
	$(if $(wildcard $(DESTDIR)$(datadir)/grafx2/scripts/libs),$(RMDIR) $(DESTDIR)$(datadir)/grafx2/scripts/libs,)
	$(if $(wildcard $(DESTDIR)$(datadir)/grafx2/scripts),$(RMDIR) $(DESTDIR)$(datadir)/grafx2/scripts,)
	$(if $(wildcard $(DESTDIR)$(datadir)/grafx2),$(RMDIR) $(DESTDIR)$(datadir)/grafx2,)
	# Icon and desktop file for debian
	$(DELCOMMAND) $(DESTDIR)$(datadir)/applications/grafx2.desktop
	$(DELCOMMAND) $(DESTDIR)$(pixmapdir)/grafx2.xpm
	$(DELCOMMAND) $(DESTDIR)$(pixmapdir)/grafx2.svg
	@echo Uninstall complete

endif

-include Makefile.dep

