#  Grafx2 - The Ultimate 256-color bitmap paint program
#  
#  Copyright 2018 Thomas Bernard
#  Copyright 2012 Franck Charlet
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
  PREFIX ?= /usr/local
  exec_prefix = $(PREFIX)
  bindir = $(exec_prefix)/bin
  datarootdir = $(PREFIX)/share
  datadir = $(datarootdir)
  pixmapdir = $(datarootdir)/icons/hicolor/scalable/apps

# Compile with OPTIM=0 to disable gcc optimizations, to enable debug.
  STRIP = strip

# Detect GIT revision
ifeq (true,$(shell git rev-parse --is-inside-work-tree))
  GIT_REVISION = $(shell git rev-list --count 1af8c74f53110e349d8f0d19b14599281913f71f..)
  ifneq (,$(CI_COMMIT_REF_NAME))
    GIT_BRANCH = $(CI_COMMIT_REF_NAME)
  else
    GIT_BRANCH = $(shell git rev-parse --abbrev-ref HEAD)
  endif
  ifneq (master,$(GIT_BRANCH))
    GIT_REVISION := $(GIT_REVISION)-$(GIT_BRANCH)
  endif
else
  # outside git repository : get version stored in version.c
  GIT_REVISION := $(shell cat version.c | cut -f 2 -d '"')
endif

  REVISION_CACHE = .revision.cache
  RES := $(shell if [ ! -f $(REVISION_CACHE) ] || [ "`cat $(REVISION_CACHE)`" != "$(GIT_REVISION)" ] ; then echo "$(GIT_REVISION)" > $(REVISION_CACHE) ; fi )

ifneq (,$(LABEL))
  LABEL_CACHE = .label.cache
  RES := $(shell if [ ! -f $(LABEL_CACHE) ] || [ "`cat $(LABEL_CACHE)`" != "$(LABEL)" ] ; then echo "$(LABEL)" > $(LABEL_CACHE) ; fi )
else
  # get label stored in pversion.c
  LABEL := $(shell cat pversion.c | cut -f 2 -d '"')
endif

  CFLAGS_CACHE = $(OBJDIR)/.cflags.cache

  RECOILVER = 4.3.0

# Video / input API used
  API ?= sdl

# At the moment support for TTF requires SDL or SDL2
  ifneq (sdl,$(findstring sdl,$(API)))
    NOTTF = 1
  endif

### PLATFORM DETECTION AND CONFIGURATION ###

PLATFORMOBJ =

TAR = tar
# Note : --transform option was added in GNU tar version 1.15.91
TARTRANSFORM = --strip=1 --transform 's,^,grafx2/,g'

# default to gcc compiler
ifeq (default,$(origin CC))
  CC = gcc
endif

# There is no uname under windows, but we can guess we are there with the COMSPEC env.var
# Windows specific
ifdef COMSPEC
  DELCOMMAND = rm -f
  MKDIR = mkdir -p
  RMDIR = rmdir --ignore-fail-on-non-empty
  CP = cp
  BIN = ../bin/grafx2.exe
  COPT = -W -Wall -Wdeclaration-after-statement -O$(OPTIM) -g -ggdb `sdl-config --cflags` $(TTFCOPT) $(JOYCOPT) $(LUACOPT)
  LOPT = `sdl-config --libs` -lSDL_image $(TTFLOPT) -lpng -lz $(LUALOPT)
  LUALOPT = -llua
  OBJDIR = ../obj/win32
  # Resources (icon)
  WINDRES = windres.exe
  PLATFORMOBJ = winres.o
  PLATFORM = win32
  #some misc files we have to add to the release archive under windows.
  PLATFORMFILES = bin/SDL.dll bin/SDL_image.dll $(wildcard ../bin/libpng*.dll) $(wildcard ../bin/libjpeg*.dll) bin/zlib1.dll $(TTFLIBS)
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
    COPT = -Wall -gstabs $(shell sdl-config --cflags) $(TTFCOPT)
    LOPT = $(shell sdl-config --libs) -lSDL_image -lpng -ljpeg -lz $(TTFLOPT) -lft2
    OBJDIR = ../obj/amiga
    ZIP = lha
    ZIPOPT = a
    NOTTF = 1

  else
  ifeq ($(PLATFORM),Darwin) # 2
    #Mac OS X specific

    # Darwin / OS X versions :
    #  6    => 10.2         ppc only
    #  7    => 10.3         ppc only
    #  8    => 10.4 Tiger   (ppc/i386)
    #  9    => 10.5 Leopard (ppc/i386)
    # 10    => 10.6 Snow Leopard (i386 only)
    MACHINE = $(shell gcc -dumpmachine)
    DARWINVER = $(shell gcc -dumpmachine | sed 's/.*darwin\([0-9]*\).*/\1/' )
    ifeq (greater, $(shell [ $(DARWINVER) -gt 9 ] && echo "greater"))
    MACOSX_ARCH = -arch x86_64 -arch i386
    #MACOSX_LION = 1
    else
    ifeq (9, $(DARWINVER))
    MACOSX_ARCH = -arch ppc -arch i386
    else
    ifneq ( ,$(findstring(powerpc, $(MACHINE))))
    MACOSX_ARCH = -arch ppc
    else
    MACOSX_ARCH = -arch i386
    endif
    endif
    endif

ifdef MACOSX_LION
MACOSX_SYSROOT = /Developer/SDKs/MacOSX10.6.sdk
else
MACOSX_SYSROOT = /Developer/SDKs/MacOSX10.4u.sdk
endif

    DELCOMMAND = rm -rf
    MKDIR = mkdir -p
    RMDIR = rmdir --ignore-fail-on-non-empty
    # Force it
    OPTIM = 3
    CP = cp
    ZIP = zip
    TAR = $(shell which bsdtar || which gnutar || which tar)
    TARTRANSFORM = $(shell if $(TAR) --version | grep 'GNU tar' > /dev/null ; then \
                             echo "--strip=1 --transform 's,^,grafx2/,g'" ; else \
                             echo "-s '/\.\./grafx2/'" ; fi )
    PLATFORMFILES = ../share/grafx2/gfx2.png
    # Where the SDL frameworks are located
    FWDIR = /Library/Frameworks
    BIN = ../bin/grafx2

ifeq ($(API),sdl)
    SDLCONFIG := $(shell which sdl-config)
ifneq ($(SDLCONFIG), )
    # these are for use with macports
    SDLCOPT = $(shell sdl-config --cflags)
    SDLLOPT = $(shell sdl-config --libs) $(shell pkg-config --libs SDL_image)
else
    # these are for use with Mac OS X native frameworks
    #-framework SDL_ttf 
    SDLLOPT = -isysroot $(MACOSX_SYSROOT) $(MACOSX_ARCH) -L/usr/lib -framework SDL -framework SDL_image -framework Cocoa -framework Carbon -framework OpenGL
    SDLCOPT = $(MACOSX_ARCH) -I$(FWDIR)/SDL.framework/Headers -I$(FWDIR)/SDL_image.framework/Headers -I$(FWDIR)/SDL_ttf.framework/Headers -D_THREAD_SAFE
endif
endif

    # these are for use with macports
    LUAPKG := $(shell for p in lua5.3 lua-5.3 lua53 lua5.2 lua-5.2 lua52 lua5.1 lua-5.1 lua51 lua ; do pkg-config --exists $$p && echo $$p && break ; done)
ifneq ($(LUAPKG), )
    LUACOPT = $(shell pkg-config $(LUAPKG) --cflags)
    LUALOPT = $(shell pkg-config $(LUAPKG) --libs)
else
    # these are for use with Mac OS X native frameworks
    LUACOPT = -I$(FWDIR)/Lua.framework/Headers
ifdef MACOSX_LION
    LUALOPT = -framework lua
else
    LUALOPT = -llua 
endif
endif

    # these are for everyone
    COPT = -D_DARWIN_C_SOURCE -D__macosx__ -W -Wall -Wdeclaration-after-statement -O$(OPTIM) -std=c99 -g $(LUACOPT) $(SDLCOPT) $(TTFCOPT) -I/usr/include
ifdef MACOSX_LION
    LOPT = $(SDLLOPT) $(LUALOPT) -framework libpng14 -lz
else
    LOPT = $(SDLLOPT) $(LUALOPT)
    LIBPNGCONFIG := $(shell libpng-config)
ifneq ($(LIBPNGCONFIG), )
    COPT += $(shell libpng-config --cflags)
    LOPT += $(shell libpng-config --ldflags)
else
    LOPT += -lpng
endif
    LOPT += -lz
endif
    COPT += -DENABLE_FILENAMES_ICONV
    LOPT += -liconv
ifeq ($(API),x11)
    LOPT += -Wl,-framework,CoreFoundation
endif
    # Use gcc for compiling. Use ncc to build a callgraph and analyze the code.
    #CC = nccgen -ncgcc -ncld -ncfabs
    OBJDIR = ../obj/macosx
ifeq ($(API),sdl)
    PLATFORMOBJ = SDLMain.o
endif
    FCLOPT = 
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
    COPT = -Wall -gstabs $(shell sdl-config --cflags) $(TTFCOPT)
    LOPT = -lSDL_image $(shell sdl-config --libs) -lpng -ljpeg -lz $(TTFLOPT)
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
    COPT = -W -Wall -g $(shell sdl-config --cflags) $(TTFCOPT) -I/boot/home/config/include
    LOPT = $(shell sdl-config --libs) -lSDL_image -lpng -ljpeg -lz $(TTFLOPT)
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
    PLATFORMOBJ = haiku.o
    ifeq ($(NOLUA),1)
      LUACOPT =
      LUALOPT =
    else
      LUAPKG=lua
      LUACOPT = -D__ENABLE_LUA__ $(shell pkg-config $(LUAPKG) --cflags)
      LUALOPT = $(shell pkg-config $(LUAPKG) --libs)
    endif
    COPT = -W -Wall -g $(shell sdl-config --cflags) $(TTFCOPT) -I/boot/common/include $(LUACOPT)
    COPT += -DENABLE_FILENAMES_ICONV
    LOPT = $(shell sdl-config --libs) -lSDL_image -lpng -ljpeg -lz $(TTFLOPT) -lfreetype -lbe $(LUALOPT) -liconv
    #Append the gcc kind to the objdir (gcc2 or gcc4) to avoid conflicts when switching from one to other.
    OBJKIND = $(shell gcc -dumpversion)
    OBJDIR = ../obj/haiku/$(OBJKIND)
    ZIP = zip

  else
  ifeq ($(PLATFORM),skyos) # 8
    #SkyOS specific
    DELCOMMAND = rm -rf
    MKDIR = mkdir -p
    RMDIR = rmdir --ignore-fail-on-non-empty
    CP = cp
    BIN = ../bin/grafx2
    COPT = -W -Wall -Wdeclaration-after-statement -g $(shell sdl-config --cflags) $(TTFCOPT)
    LOPT = $(shell sdl-config --libs) -lSDL_image -lpng -ljpeg -lz $(TTFLOPT)
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
    PLATFORMFILES = ../share/grafx2/gfx2.png
    BIN = ../bin/grafx2
    COPT = -W -Wall -std=c99 -g -gstabs -D__TRU64__ $(shell sdl-config --cflags) $(TTFCOPT) $(LUACOPT)
    LOPT = $(shell sdl-config --libs) -lSDL_image $(TTFLOPT) -lpng $(LUALOPT) -lm
    OBJDIR = ../obj/unix
    FCLOPT = -lfontconfig
    COPT += -DUSE_FC
  else
  ifeq ($(PLATFORM),FreeMiNT) #10
    #Atari FreeMiNT/TOS specific
    DELCOMMAND = rm -rf
    MKDIR = mkdir -p
    RMDIR = rmdir
    CP = cp
    ZIP = zip
    PLATFORMFILES = ../share/grafx2/gfx2.png
    BIN = ../bin/grafx2.ttp
    LUALOPT = -llua
    OBJDIR = ../obj/m68k-atari-mint
    PLATFORM = m68k-atari-mint
    STRIP = strip -s 
    STACK = stack
    FIX_FLAGS = flags
    FCLOPT = 
    COPT = -W -Wall -m68020-60 -fomit-frame-pointer -std=c99 -Wdeclaration-after-statement -D__MINT__ -DNO_INLINE_MATH -O$(OPTIM) -I$(PREFIX)/include `$(PREFIX)/bin/libpng12-config --cflags` `$(PREFIX)/bin/sdl-config --cflags` $(JOYCOPT) $(LUACOPT)
    LOPT = -static -m68020-60 -lSDL_image `$(PREFIX)/bin/sdl-config --libs` -L$(PREFIX)/lib -ltiff -ljpeg `$(PREFIX)/bin/libpng12-config --libs` -lz -lm $(TTFLOPT) -lfreetype $(LUALOPT) $(LAYERLOPT)
  else
  ifeq ($(PLATFORM),syllable) #11
    #Syllable
    DELCOMMAND = rm -rf
    MKDIR = mkdir -p
    RMDIR = rmdir --ignore-fail-on-non-empty
    CP = cp
    ZIP = zip
    PLATFORMFILES = ../share/grafx2/gfx2.png
    LUACOPT = -I/resources/indexes/include
    LUALOPT = -llua
    BIN = ../bin/grafx2
    COPT = -W -Wall -Wdeclaration-after-statement -std=c99 -g `sdl-config --cflags` -I/resources/indexes/include/SDL $(TTFCOPT) $(LUACOPT) $(JOYCOPT) -O$(OPTIM)
    LOPT = `sdl-config --libs` -lSDL_image $(TTFLOPT) -lpng -lz $(LUALOPT) -lm
    OBJDIR = ../obj/syllable
    FCLOPT = 
  else
    # Finally, the default rules that work fine for most unix/gcc systems, linux and freebsd are tested.
    # Linux and FreeBSD specific (default rules)
    DELCOMMAND = rm -rf
    MKDIR = mkdir -p
    RMDIR = rmdir --ignore-fail-on-non-empty
    CP = cp
    ZIP = zip
    TARTRANSFORM = $(shell if $(TAR) --version | grep 'GNU tar' > /dev/null ; then \
                             echo "--strip=1 --transform 's,^,grafx2/,g'" ; else \
                             echo "-s '/\.\./grafx2/'" ; fi )
    PLATFORMFILES = ../share/grafx2/gfx2.png
    ifneq ($(ATARICROSS),1)
      ifeq ($(NOLUA),1)
        LUACOPT =
        LUALOPT =
      else
        ifdef WIN32CROSS
          LUACOPT = -I../3rdparty/usr/include
        else
          LUAPKG := $(shell for p in lua5.3 lua-5.3 lua53 lua5.2 lua-5.2 lua52 lua5.1 lua-5.1 lua51 lua ; do pkg-config --exists $$p && echo $$p && break ; done)
          LUACOPT = $(shell pkg-config $(LUAPKG) --cflags)
          LUALOPT = $(shell pkg-config $(LUAPKG) --libs)
        endif
      endif
    endif

    # These can only be used under linux and maybe freebsd. They allow to compile for the gp2x or to create a windows binary
    ifdef WIN32CROSS
      #cross compile a Win32 executable
      CROSS_CC ?= $(shell which i686-w64-mingw32-gcc || which mingw32-gcc)
      CROSS_PKGCONFIG ?= $(shell which i686-w64-mingw32-pkg-config || which mingw32-pkg-config || which pkg-config)
      CROSS_SDLCONFIG ?= $(shell which ../3rdparty/usr/bin/sdl-config || which /usr/local/cross-tools/i386-mingw32/bin/sdl-config)
      CROSS_PKG_CONFIG_PATH ?= $(shell if [ -d ../3rdparty/usr ] ; then echo "$${PWD}/../3rdparty/usr/lib/pkgconfig" ; fi )
      CC = $(CROSS_CC)
      CROSS_STRIP ?= $(shell which i686-w64-mingw32-strip || which mingw32-strip)
      STRIP = $(CROSS_STRIP)
      BIN = ../bin/grafx2-$(API).exe
      COPT = -W -Wall -Wdeclaration-after-statement -O$(OPTIM) -g -ggdb $(TTFCOPT)
      LOPT = -mwindows -lmingw32 -lshlwapi $(TTFLOPT)
      ifeq ($(API),sdl)
      COPT += -Dmain=SDL_main $(shell $(CROSS_SDLCONFIG) --cflags)
      LOPT += -lSDLmain -lSDL $(shell $(CROSS_SDLCONFIG) --libs) -lSDL_image
      endif
      ifeq (,$(CROSS_PKGCONFIG))
      COPT += -D__no_pnglib__
      else
      COPT += $(shell PKG_CONFIG_LIBDIR=$(CROSS_PKG_CONFIG_PATH) $(CROSS_PKGCONFIG) --cflags libpng || echo "-D__no_pnglib__" )
      LOPT += $(shell PKG_CONFIG_LIBDIR=$(CROSS_PKG_CONFIG_PATH) $(CROSS_PKGCONFIG) --libs libpng)
      endif
      LUALOPT = -llua
      COPT += $(LUACOPT)
      LOPT += $(LUALOPT)
      OBJDIR = ../obj/win32
      WINDRES ?= $(shell which i686-w64-mingw32-windres || which mingw32-windres)
      PLATFORM = win32
      PLATFORMOBJ = winres.o
      PLATFORMFILES = $(wildcard ../bin/libpng*.dll) $(wildcard ../bin/libjpeg*.dll) bin/zlib1.dll $(TTFLIBS)
      ifeq ($(API),sdl)
      PLATFORMFILES += bin/SDL.dll bin/SDL_image.dll
      endif
    else
      ifdef GP2XCROSS
        #cross compile an exec for the gp2x
        CC = /opt/open2x/gcc-4.1.1-glibc-2.3.6/arm-open2x-linux/bin/arm-open2x-linux-gcc
        BIN = ../bin/grafx2.gpe
        COPT = -W -Wall -Wdeclaration-after-statement -pedantic -std=c99 -static -g -O$(OPTIM) -I/opt/open2x/gcc-4.1.1-glibc-2.3.6/include `/opt/open2x/gcc-4.1.1-glibc-2.3.6/bin/sdl-config --cflags` $(TTFCOPT) -D__GP2X__ $(TTFCOPT) $(JOYCOPT) $(LUACOPT)
        LOPT = -static -lSDL_image `/opt/open2x/gcc-4.1.1-glibc-2.3.6/bin/sdl-config --static-libs` -ljpeg -lpng -lz -lm $(TTFLOPT) $(LUALOPT)
        OBJDIR = ../obj/gp2x
        NOTTF = 1
        PLATFORM = gp2x
        STRIP = /opt/open2x/gcc-4.1.1-glibc-2.3.6/arm-open2x-linux/bin/arm-open2x-linux-strip
        JOYCOPT = -DUSE_JOYSTICK
      else
      ifdef GCWZERO
        #cross compile an exec for the gcw0
        CC = mipsel-linux-gcc
        BIN = ../bin/grafx2
        LUACOPT = 
        LUALOPT = -lluajit-5.1
        COPT = -DGCWZERO -W -Wall -Wdeclaration-after-statement -pedantic -std=c99 -g -O$(OPTIM) -I/opt/gcw0-toolchain/usr/mipsel-gcw0-linux-uclibc/sysroot/usr/include -I/opt/gcw0-toolchain/usr/mipsel-gcw0-linux-uclibc/sysroot/usr/include/SDL `/opt/gcw0-toolchain/usr/mipsel-gcw0-linux-uclibc/sysroot/usr/bin/sdl-config --cflags` $(TTFCOPT)  $(TTFCOPT) $(JOYCOPT) $(LUACOPT)
        LOPT = -L/opt/gcw0-toolchain/usr/mipsel-gcw0-linux-uclibc/sysroot/usr/lib -lSDL_image `/opt/gcw0-toolchain/usr/mipsel-gcw0-linux-uclibc/sysroot/usr/bin/sdl-config --libs` -ljpeg -lpng -lz -lm $(TTFLOPT) $(LUALOPT)
        OBJDIR = ../obj/gp2x
        NOTTF = 1
        PLATFORM = gp2x
        STRIP = mipsel-linux-strip
        JOYCOPT = -DUSE_JOYSTICK
      else
      ifdef AROS32CROSS
        #cross compile an AROS 32 bit executable
        BIN = ../GrafX2
        COPT = -Wall -Wno-pointer-sign -Wno-unused-but-set-variable -g `i386-linux-aros-sdl-config --cflags` $(TTFCOPT) $(LUACOPT)
        LOPT = -lSDL_image `i386-linux-aros-sdl-config --libs` -lpng -ljpeg -lz $(TTFLOPT) -lfreetype2shared $(LUALOPT)
        LUACOPT = -I/home/mazze/projects/fullaros/aros-linux-i386-dbg/bin/linux-i386/AROS/Development/include/lua
        LUALOPT = -llua
        CC = i386-linux-aros-gcc
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
        FCLOPT = 
        COPT = -W -Wall -m68020-60 -fomit-frame-pointer -std=c99 -Wdeclaration-after-statement -D__MINT__ -ffast-math -O$(OPTIM) -I$(PREFIX)/include `$(PREFIX)/bin/libpng12-config --cflags` `$(PREFIX)/bin/sdl-config --cflags` $(JOYCOPT) $(LUACOPT)
        LOPT = -static -m68020-60 -lSDL_image `$(PREFIX)/bin/sdl-config --libs` -L$(PREFIX)/lib -ltiff -ljpeg `$(PREFIX)/bin/libpng12-config --libs` -lz -lm $(TTFLOPT) -lfreetype $(LUALOPT) -lm $(LAYERLOPT) -Wl,--stack,8k 
      else

        # Compiles a regular linux executable for the native platform
        BIN = ../bin/grafx2-$(API)
        COPT = -W -Wall -Wdeclaration-after-statement -std=c99 -g
        ifeq ($(API),sdl)
          COPT += $(shell sdl-config --cflags)
        endif
        ifeq ($(API),sdl2)
          COPT += $(shell sdl2-config --cflags)
        endif
        COPT += $(TTFCOPT) $(LUACOPT) $(JOYCOPT) -O$(OPTIM)
        COPT += $(shell pkg-config --cflags libpng)

        ifneq ($(PLATFORM), FreeBSD)
          COPT += -D_XOPEN_SOURCE=700
        endif

        LOPT = -lm
        ifeq ($(API),sdl)
          LOPT += $(shell sdl-config --libs) -lSDL_image
        endif
        ifeq ($(API),sdl2)
          LOPT += $(shell sdl2-config --libs) -lSDL2_image
        endif
        LOPT += $(TTFLOPT)
        LOPT += $(shell pkg-config --libs libpng)
        LOPT += $(LUALOPT)
        OBJDIR = ../obj/unix
        FCLOPT = -lfontconfig
        COPT += -DUSE_FC

        # enable UTF8 filename translation
        # For Linux (GLibc), iconv is built into the C library so no LOPT needed.
        COPT += -DENABLE_FILENAMES_ICONV
        ifneq ($(PLATFORM),Linux)
          LOPT += -liconv
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
  endif
endif

### BUILD SETTINGS are set according to vars set in the platform selection, 
### the "overridable defaults", and environment variables set before launching make

#TrueType is optional: make NOTTF=1 to disable support and dependencies.
ifeq ($(NOTTF),1)
  TTFCOPT = -DNOTTF=1
  TTFLOPT =
  TTFLIBS =
  TTFLABEL = -nottf
else
 ifneq ($(ATARICROSS),1)
   TTFCOPT = 
   TTFLOPT =
   ifneq ($(WIN32CROSS),1)
     TTFLOPT += -L$(PREFIX)/lib
   endif
   ifeq ($(API),sdl)
     TTFLOPT += -lSDL_ttf $(FCLOPT)
   else
     TTFLOPT += -lSDL2_ttf $(FCLOPT)
   endif
   TTFLIBS = bin/libfreetype-6.dll bin/SDL_ttf.dll
   TTFLABEL = 
 else
   TTFCOPT = 
   ifeq ($(API),sdl)
   TTFLOPT = -L$(PREFIX)/lib -lSDL_ttf $(FCLOPT)
   else
   TTFLOPT = -L$(PREFIX)/lib -lSDL2_ttf $(FCLOPT)
   endif
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

#Recoil is optional
ifdef NORECOIL
    COPT += -DNORECOIL
endif

OBJDIR := $(OBJDIR)-$(API)

ifeq ($(API),sdl)
  APIOBJ = sdlscreen.o
  COPT += -DUSE_SDL
endif
ifeq ($(API),sdl2)
  APIOBJ = sdlscreen.o
  COPT += -DUSE_SDL2
endif
ifeq ($(API),win32)
  APIOBJ = win32screen.o
endif
ifeq ($(API),x11)
  APIOBJ = x11screen.o
  COPT += -DUSE_X11
  LOPT += -lX11
endif

#To enable Joystick emulation of cursor, make USE_JOYSTICK=1 (for input.o)
#This can be necessary to test cursor code on a PC, but by default for all
#non-console platforms the joystick is disabled, to avoid reporting
#'restless' movements when an analog joystick or a poorly-calibrated joypad
#is plugged in.
ifeq ($(USE_JOYSTICK),1)
  JOYCOPT = -DUSE_JOYSTICK
endif

### And now for the real build rules ###

.PHONY : all debug release clean depend zip force install uninstall valgrind

# This is the list of the objects we want to build. Dependancies are built by "make depend" automatically.
OBJS = main.o init.o graph.o $(APIOBJ) misc.o special.o \
       buttons.o palette.o help.o operatio.o pages.o loadsave.o \
       readline.o engine.o filesel.o op_c.o readini.o saveini.o \
       shade.o keyboard.o io.o version.o text.o SFont.o setup.o \
       pxsimple.o pxtall.o pxwide.o pxdouble.o pxtriple.o \
       pxtall2.o pxtall3.o pxwide2.o pxquad.o \
       windows.o brush.o realpath.o mountlist.o input.o hotkeys.o \
       transform.o pversion.o factory.o $(PLATFORMOBJ) \
       fileformats.o miscfileformats.o libraw2crtc.o \
       brush_ops.o buttons_effects.o layers.o \
       oldies.o tiles.o colorred.o unicode.o gfx2surface.o
ifndef NORECOIL
OBJS += loadrecoil.o recoil.o
endif
OBJ = $(addprefix $(OBJDIR)/,$(OBJS))

SKINS = skin_classic.png skin_modern.png skin_DPaint.png \
        font_Classic.png font_Fun.png font_Fairlight.png \
        font_Melon.png font_DPaint.png \
        skin_scenish.png font_Seen.png skin_Aurora.png
SKIN_FILES = $(addprefix ../share/grafx2/skins/,$(SKINS))
SKIN_FILES += $(wildcard ../share/grafx2/skins/unicode_*)

SCRIPTS_BRUSH = ApplyColor.lua Fisheye.lua GrayscaleAvg.lua \
                GrayscaleDesat.lua Halfsmooth.lua Waves.lua
SCRIPTS_DEMO = 3DPalette.lua Ellipse.lua FlipPicture.lua \
               SierpinskyCarpet.lua SierpinskyTriangle.lua \
               Spritesheet.lua
SCRIPTS_DEMO_BRUSH = Amigaball.lua ColorSphere.lua \
                     FindAA.lua Mandelbrot.lua
SCRIPTS_LIB = dawnbringer_lib.lua memory.lua
SCRIPTS_PAL = Desaturate.lua ExpandColors.lua FillColorCube.lua \
              InvertedRGB.lua Set3bit.lua Set6bit.lua \
              SetC64Palette.lua ShiftHue.lua
SCRIPTS_PIC = CellColourReducer.lua DrawGridIsometric.lua \
              DrawgridOrthogonal_Index.lua DrawGridOrthogonal_RGB.lua \
              GlassGridFilter.lua PaletteToPicture.lua Pic2isometric.lua \
              Rainbow-Dark2Bright.lua RemapImage2RGB.lua \
              RemapImage2RGB_ed.lua RemapImageTo3bitPal.lua \
              XBitColourXpaceFromPalette.lua Tiler.lua \
              FontConvert.lua
SCRIPTS_PIC_8BIT = ostro_oric.lua ostro_zx.lua \
                   lib/ostro_other.lua
SCRIPTS_PIC_THOMSON = bayer4_mo5.lua bayer4_to8.lua \
                      none_mo5.lua none_to8.lua none_to9.lua \
                      ostro_mo5.lua ostro_to7.lua ostro_to8.lua ostro_to9.lua \
                      lib/bayer.lua lib/color.lua lib/color_reduction.lua \
                      lib/convex_hull.lua lib/ostromoukhov.lua \
                      lib/thomson.lua
SCRIPT_FILES = $(addprefix ../share/grafx2/scripts/samples_2.4/, \
                 $(addprefix brush/,$(SCRIPTS_BRUSH)) \
                 $(addprefix demo/,$(SCRIPTS_DEMO)) \
                 $(addprefix demo/brush/,$(SCRIPTS_DEMO_BRUSH)) \
                 $(addprefix libs/,$(SCRIPTS_LIB)) \
                 $(addprefix palette/,$(SCRIPTS_PAL)) \
                 $(addprefix picture/,$(SCRIPTS_PIC)) \
                 $(addprefix picture/others-8bit/,$(SCRIPTS_PIC_8BIT)) \
                 $(addprefix picture/thomson/,$(SCRIPTS_PIC_THOMSON)) )

FONTS = 8pxfont.png Tuffy.ttf PF_Arma_5__.png \
        PF_Easta_7_.png PF_Easta_7__.png PF_Ronda_7__.png \
        PF_Tempesta_5.png PF_Tempesta_5_.png PF_Tempesta_5__.png PF_Tempesta_5___.png \
        PF_Tempesta_7.png PF_Tempesta_7_.png PF_Tempesta_7__.png PF_Tempesta_7___.png \
        PF_Westa_7_.png PF_Westa_7__.png
FONT_FILES = $(addprefix ../share/grafx2/fonts/,$(FONTS))


ifeq ($(PLATFORM),Darwin)
all : $(MACAPPEXE)
$(MACAPPEXE) : $(BIN) $(OBJDIR)/versiontag
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
ifeq ($(API),x11)
	cp ../share/grafx2/gfx2.png Grafx2.app/Contents/Resources
endif
	mkdir -p Grafx2.app/Contents/Resources/English.lproj
	iconv -f UTF-16BE English.lproj/InfoPlist.strings | sed -e 's/Grafx2 version [.0-9]*/Grafx2 version $(shell cat $(OBJDIR)/versiontag)/' | iconv -t UTF-16BE > Grafx2.app/Contents/Resources/English.lproj/InfoPlist.strings
	cp -r ../share/grafx2/fonts Grafx2.app/Contents/Resources
	cp -r ../share/grafx2/skins Grafx2.app/Contents/Resources
	cp -r ../share/grafx2/gfx2def.ini Grafx2.app/Contents/Resources
	cp -r ../share/grafx2/scripts/samples_2.4/brush Grafx2.app/Contents/Resources/scripts/samples_2.4
	cp -r ../share/grafx2/scripts/samples_2.4/demo Grafx2.app/Contents/Resources/scripts/samples_2.4
	cp -r ../share/grafx2/scripts/samples_2.4/libs Grafx2.app/Contents/Resources/scripts/samples_2.4
	cp -r ../share/grafx2/scripts/samples_2.4/palette Grafx2.app/Contents/Resources/scripts/samples_2.4
	cp -r ../share/grafx2/scripts/samples_2.4/picture Grafx2.app/Contents/Resources/scripts/samples_2.4
ifdef MACOSX_LION
	mkdir -p Grafx2.app/Contents/Frameworks/Lua.framework/Versions
	cp -Rp $(FWDIR)/Lua.framework/Versions/A Grafx2.app/Contents/Frameworks/Lua.framework/Versions
	mkdir -p Grafx2.app/Contents/Frameworks/libpng14.framework/Versions
	cp -Rp $(FWDIR)/libpng14.framework/Versions/1.4.8 Grafx2.app/Contents/Frameworks/libpng14.framework/Versions
endif
	if [ -d $(FWDIR)/SDL.framework/Versions ] ; then \
		mkdir -p Grafx2.app/Contents/Frameworks/SDL.framework/Versions ;\
		cp -Rp $(FWDIR)/SDL.framework/Versions/A Grafx2.app/Contents/Frameworks/SDL.framework/Versions ;\
	fi
	if [ -d $(FWDIR)/SDL_image.framework/Versions ] ; then \
		mkdir -p Grafx2.app/Contents/Frameworks/SDL_image.framework/Versions ;\
		cp -Rp $(FWDIR)/SDL_image.framework/Versions/A Grafx2.app/Contents/Frameworks/SDL_image.framework/Versions ;\
	fi
#	mkdir -p Grafx2.app/Contents/Frameworks/SDL_ttf.framework/Versions
#	cp -Rp $(FWDIR)/SDL_ttf.framework/Versions/A Grafx2.app/Contents/Frameworks/SDL_ttf.framework/Versions
    # remove those
	rm -fr Grafx2.app/Contents/Frameworks/SDL.framework/Versions/A/Headers
	rm -fr Grafx2.app/Contents/Frameworks/SDL.framework/Versions/A/Resources
	rm -fr Grafx2.app/Contents/Frameworks/SDL_image.framework/Versions/A/Headers
	rm -fr Grafx2.app/Contents/Frameworks/SDL_image.framework/Versions/A/Resources
ifdef MACOSX_LION
	rm -fr Grafx2.app/Contents/Frameworks/Lua.framework/Versions/A/Resources
	rm -fr Grafx2.app/Contents/Frameworks/Lua.framework/Versions/A/Headers
	rm -fr Grafx2.app/Contents/Frameworks/libpng14.framework/Versions/1.4.8/Resources
	rm -fr Grafx2.app/Contents/Frameworks/libpng14.framework/Versions/1.4.8/Headers
	rm -fr Grafx2.app/Contents/Frameworks/SDL_ttf.framework/Versions
endif
	cp $(BIN) $(MACAPPEXE)
	$(STRIP) -x -X -S $(MACAPPEXE)
	chmod +x $(MACAPPEXE)
	tar cvzf grafx2-git$(GIT_REVISION)-macosx.tgz --exclude '.git' --exclude '*DS_Store*' Grafx2.app/*
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
release : $(BIN)
	$(STRIP) $(BIN)

ifeq ($(PLATFORM),Darwin)
ziprelease: $(MACAPPEXE)
endif

# Create a zip archive ready for upload to the website, including binaries and sourcecode
ziprelease: release $(OBJDIR)/versiontag gfx2.rc
	cd .. && $(TAR) czf "src-`cat $(OBJDIR:../%=%)/versiontag`.tgz" src/*.c src/*.cpp src/*.h src/Makefile src/Makefile.dep \
	  src/gfx2.ico src/Grafx2_Prefix.pch src/SDLMain.m src/gfx2.rc
	cd .. && $(ZIP) $(ZIPOPT) "grafx2-`cat $(OBJDIR:../%=%)/versiontag`$(TTFLABEL)-$(PLATFORM).$(ZIP)" $(BIN:../%=%) share/grafx2/gfx2def.ini $(SCRIPT_FILES:../%=%) $(SKIN_FILES:../%=%) share/grafx2/gfx2.gif share/icons/grafx2.svg doc/README.txt doc/COMPILING.txt doc/gpl-2.0.txt doc/PF_fonts.txt $(FONT_FILES:../%=%) doc/README-zlib1.txt doc/README-SDL.txt doc/README-SDL_image.txt doc/README-SDL_ttf.txt doc/README-lua.txt src-`cat $(OBJDIR:../%=%)/versiontag`.tgz $(PLATFORMFILES:../%=%)
	$(DELCOMMAND) "../src-`cat $(OBJDIR)/versiontag`.tgz"
ifeq ($(PLATFORM),Darwin)
	$(ZIP) $(ZIPOPT) -r ../Grafx2.app-`cat $(OBJDIR)/versiontag`.zip Grafx2.app
endif
	$(TAR) czf "../grafx2-`cat $(OBJDIR)/versiontag`$(TTFLABEL)-src.tgz" $(TARTRANSFORM) ../src/*.c ../src/*.cpp ../src/*.h ../src/Makefile ../src/Makefile.dep ../share/grafx2/gfx2def.ini $(SCRIPT_FILES) $(SKIN_FILES) ../src/gfx2.ico ../share/grafx2/gfx2.gif ../share/icons/grafx2.svg ../doc/README.txt ../doc/COMPILING.txt ../doc/gpl-2.0.txt ../doc/PF_fonts.txt ../misc/unix/grafx2.1 ../misc/unix/grafx2.xpm ../misc/unix/grafx2.desktop ../misc/morphos/grafx2.info $(FONT_FILES) ../src/Grafx2_Prefix.pch ../src/SDLMain.m

$(BIN) : $(OBJ)
	@test -d ../bin || $(MKDIR) ../bin
	$(CC) $(OBJ) -o $(BIN) $(LOPT) $(LFLAGS)
ifeq ($(PLATFORM),Haiku)
	rc -o $(OBJDIR)/grafx2.rsrc grafx2.rdef
	xres -o $(BIN) $(OBJDIR)/grafx2.rsrc
	mimeset -f $(BIN)
endif

# GIT revision number
version.c:	$(REVISION_CACHE)
	echo "const char SVN_revision[]=\"$(GIT_REVISION)\";" > version.c

pversion.c:	$(LABEL_CACHE)
	echo "const char Program_version[]=\"$(LABEL)\";" > pversion.c

$(OBJDIR)/versiontag: pversion.c $(REVISION_CACHE)
	$(if $(wildcard $(OBJDIR)),,$(MKDIR) $(OBJDIR))
	echo "$(subst wip.,wip,$(LABEL).$(GIT_REVISION))" > $@

RES := $(shell $(MKDIR) $(OBJDIR) ; if [ ! -f $(CFLAGS_CACHE) ] || [ "`cat $(CFLAGS_CACHE)`" != "$(COPT) $(CFLAGS)" ] ; then echo "$(COPT) $(CFLAGS)" > $(CFLAGS_CACHE) ; fi )

ifndef NORECOIL
recoil.c:	../3rdparty/recoil-$(RECOILVER)/recoil.c
	if [ -f $< ] ; then $(CP) $< $@ ; fi
	if [ -f $(subst .c,.h,$<) ] ; then $(CP) $(subst .c,.h,$< $@) ; fi

recoil.h:	recoil.c

$(OBJDIR)/loadrecoil.o:	recoil.c recoil.h

../3rdparty/recoil-$(RECOILVER)/recoil.c:
	if [ -d ../3rdparty ] ; then $(MAKE) -C ../3rdparty recoil ; fi
endif

$(OBJ):	$(CFLAGS_CACHE)

$(OBJDIR)/%.o : %.c
	$(if $(wildcard $(OBJDIR)),,$(MKDIR) $(OBJDIR))
	$(CC) $(COPT) $(CFLAGS) -c $*.c -o $(OBJDIR)/$*.o

$(OBJDIR)/%.o : %.m
	$(if $(wildcard $(OBJDIR)),,$(MKDIR) $(OBJDIR))
	$(CC) $(COPT) -c $*.m -o $(OBJDIR)/$*.o

depend :
	$(CC) -MM *.c | sed 's:^[^ ]:$$(OBJDIR)/&:' > Makefile.dep

# update the gfx2.rc Windows ressource file
comma := ,

gfx2.rc: gfx2.ico $(REVISION_CACHE) $(LABEL_CACHE)
	sed -e 's/\(VERSION [ ]*\)\(.*\)/\1$(subst .,$(comma),$(subst wip,,$(LABEL))),0,$(firstword $(subst -, ,$(GIT_REVISION)))/' \
	    -e 's/\(Version", "\)\(.*\)"/\1$(subst wip.,wip,$(LABEL).$(GIT_REVISION))"/' $@ > $@.tmp
	if ! diff -q -i $@ $@.tmp ; then mv $@.tmp $@ ; else rm $@.tmp ; fi

# Link the icons to the program under windows
$(OBJDIR)/winres.o : gfx2.rc gfx2.ico
	$(WINDRES) -i $< -o $(OBJDIR)/winres.o

# Compile the C++ file needed in Haiku to use the API
$(OBJDIR)/haiku.o : haiku.cpp
	g++ -c haiku.cpp -o $(OBJDIR)/haiku.o $(COPT) -Wno-multichar

clean :
	$(DELCOMMAND) $(OBJ)
	$(DELCOMMAND) $(OBJDIR)/versiontag
	$(DELCOMMAND) $(BIN)
	$(DELCOMMAND) recoil.c recoil.h

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
	test -d $(DESTDIR)$(datadir)/grafx2/scripts/samples_2.4/picture/thomson || $(MKDIR) $(DESTDIR)$(datadir)/grafx2/scripts/samples_2.4/picture/thomson
	test -d $(DESTDIR)$(datadir)/grafx2/scripts/samples_2.4/picture/thomson/lib || $(MKDIR) $(DESTDIR)$(datadir)/grafx2/scripts/samples_2.4/picture/thomson/lib
	test -d $(DESTDIR)$(datadir)/grafx2/scripts/samples_2.4/picture/others-8bit || $(MKDIR) $(DESTDIR)$(datadir)/grafx2/scripts/samples_2.4/picture/others-8bit
	test -d $(DESTDIR)$(datadir)/grafx2/scripts/samples_2.4/picture/others-8bit/lib || $(MKDIR) $(DESTDIR)$(datadir)/grafx2/scripts/samples_2.4/picture/others-8bit/lib
	test -d $(DESTDIR)$(datadir)/applications || $(MKDIR) $(DESTDIR)$(datadir)/applications
	test -d $(DESTDIR)$(datadir)/metainfo || $(MKDIR) $(DESTDIR)$(datadir)/metainfo
	test -d $(DESTDIR)$(pixmapdir) || $(MKDIR) $(DESTDIR)$(pixmapdir)
	
	# Copy files
	$(CP) $(BIN) $(DESTDIR)$(bindir)
	$(CP) ../share/grafx2/gfx2def.ini $(DESTDIR)$(datadir)/grafx2/
	$(CP) ../share/grafx2/gfx2.gif $(DESTDIR)$(datadir)/grafx2/
	$(CP) ../share/grafx2/fonts/* $(DESTDIR)$(datadir)/grafx2/fonts/
	$(CP) $(SKIN_FILES) $(DESTDIR)$(datadir)/grafx2/skins/
	for f in $(SCRIPT_FILES:../share/%=%); do cp "../share/$$f" "$(DESTDIR)$(datadir)/$$f" ; done
	# Icon and desktop file for debian
	$(CP) ../misc/unix/grafx2.desktop $(DESTDIR)$(datadir)/applications/
	$(CP) ../misc/unix/grafx2.appdata.xml $(DESTDIR)$(datadir)/metainfo/
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
	$(DELCOMMAND) $(DESTDIR)$(datadir)/metainfo/grafx2.appdata.xml
	$(DELCOMMAND) $(DESTDIR)$(pixmapdir)/grafx2.xpm
	$(DELCOMMAND) $(DESTDIR)$(pixmapdir)/grafx2.svg
	@echo Uninstall complete

endif

valgrind: $(BIN)
	valgrind --suppressions=../valgrind.grafx2.suppr --leak-check=full --show-leak-kinds=all --num-callers=20 $(BIN)

-include Makefile.dep
