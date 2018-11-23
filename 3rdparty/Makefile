SDLDEVEL = SDL-devel-1.2.15-mingw32.tar.gz
SDLDEVELURL = https://www.libsdl.org/release/$(SDLDEVEL)
SDLDEVELPATCH = SDL-1.2.15.patch
SDLIMAGE = SDL_image-1.2.12
SDLIMAGEARCH = SDL_image-1.2.12.tar.gz
SDLIMAGEURL = https://www.libsdl.org/projects/SDL_image/release/$(SDLIMAGEARCH)
SDLTTF=SDL_ttf-2.0.11
SDLTTFARCH=$(SDLTTF).tar.gz
SDLTTFURL=https://www.libsdl.org/projects/SDL_ttf/release/$(SDLTTFARCH)
LIBPNG = libpng-1.6.35
LIBPNGARCH = $(LIBPNG).tar.gz
LIBPNGURL = https://download.sourceforge.net/libpng/$(LIBPNGARCH)
LIBPNGURLALT = ftp://ftp-osl.osuosl.org/pub/libpng/src/libpng16/$(LIBPNGARCH)
JPEGVER = 9c
JPEGDIR = jpeg-$(JPEGVER)
JPEGARCH = jpegsrc.v$(JPEGVER).tar.gz
JPEGURL = http://www.ijg.org/files/$(JPEGARCH)
# http://www.simplesystems.org/libtiff/
# https://gitlab.com/libtiff/libtiff
LIBTIFF = tiff-4.0.10
LIBTIFFARCH = $(LIBTIFF).tar.gz
LIBTIFFURL = https://download.osgeo.org/libtiff/$(LIBTIFFARCH)
LIBTIFFURLALT = https://fossies.org/linux/misc/$(LIBTIFFARCH)
ZLIB=zlib-1.2.11
ZLIBARCH=$(ZLIB).tar.gz
ZLIBURL=https://www.zlib.net/$(ZLIBARCH)
FREETYPE=freetype-2.9.1
FREETYPEARCH=$(FREETYPE).tar.gz
FREETYPEURL=https://download.savannah.gnu.org/releases/freetype/$(FREETYPEARCH)
LUA=lua-5.3.5
LUAARCH=$(LUA).tar.gz
LUAURL=https://www.lua.org/ftp/$(LUAARCH)
# https://storage.googleapis.com/downloads.webmproject.org/releases/webp/libwebp-0.6.1.tar.gz
RECOILVER=4.3.0
RECOIL=recoil-$(RECOILVER)
RECOILARCH=$(RECOIL).tar.gz
# https://downloads.sourceforge.net/project/recoil/recoil/4.3.0/recoil-4.3.0.tar.gz
RECOILURL=https://downloads.sourceforge.net/project/recoil/recoil/$(RECOILVER)/$(RECOILARCH)
RECOILURLALT=http://nanard.free.fr/grafx2/$(RECOILARCH)

PREFIX = $(PWD)/usr

MKDIR = mkdir -p
CP = cp -v
TAR = $(shell which tar)
GETURL = $(shell WGET=`which wget` ; if [ "$?" = "0" ] && [ -x "$WGET" ] ; then echo "$WGET" ; else echo "curl -O -L --max-time 120" ; fi )

BUILD_CC := $(CC)

ifdef WIN32CROSS
CROSS_CC ?= $(shell which i686-w64-mingw32-gcc || which mingw32-gcc)
CROSS_AR ?= $(shell which i686-w64-mingw32-ar || which mingw32-ar)
CROSS_RANLIB ?= $(shell which i686-w64-mingw32-ranlib || which mingw32-ranlib)
CROSS_LDFLAGS += -static-libgcc
CC = $(CROSS_CC)
AR = $(CROSS_AR)
RANLIB = $(CROSS_RANLIB)
CFLAGS = $(CROSS_CFLAGS)
LDFLAGS = $(CROSS_LDFLAGS)
endif

HOST = $(shell $(CC) -dumpmachine)
#HOST = i686-pc-mingw32

.PHONY:	all clean clean_archives clean_all libs libpng libsdl libsdl_image libsdl_ttf libjpeg libtiff zlib freetype lua recoil

all:	libs

clean_all:	clean clean_archives

clean:
	$(RM) -r usr/ $(LIBPNG) $(ZLIB) $(SDLIMAGE) $(JPEGDIR) $(LIBTIFF)
	$(RM) -r $(SDLTTF) $(FREETYPE) SDL-1.2.15 $(LUA) $(RECOIL)

clean_archives:
	$(RM) -r archives

libs:	libpng libsdl libsdl_image libsdl_ttf lua
libsdl:	$(PREFIX)/lib/libSDLmain.a
libsdl_image:	$(PREFIX)/lib/libSDL_image.a
libsdl_ttf:	$(PREFIX)/lib/libSDL_ttf.a
libjpeg:	$(PREFIX)/lib/libjpeg.a
libpng:	$(PREFIX)/lib/libpng.a
libtiff:	$(PREFIX)/lib/libtiff.a
zlib:	$(PREFIX)/lib/libz.a
freetype:	$(PREFIX)/lib/libfreetype.a
lua:	$(PREFIX)/lib/liblua.a

$(PREFIX)/lib/liblua.a:	$(LUA)/.ok
ifdef WIN32CROSS
	cd $(LUA) && $(MAKE) PLAT=mingw CC=$(CC) RANLIB=$(RANLIB)
	cd $(LUA) && $(MAKE) install PLAT=mingw INSTALL_TOP=$(PREFIX) TO_BIN="lua.exe luac.exe"
	cp -v $(LUA)/src/lua*.dll ../bin
endif

$(LUA)/.ok:	archives/$(LUAARCH)
	$(TAR) xzf $<
	touch $@

$(PREFIX)/lib/libSDLmain.a:	archives/$(SDLDEVEL)
	$(TAR) xzf $<
	patch -p0 < $(SDLDEVELPATCH)
	cd SDL-1.2.15 && CROSS_PATH=$(PREFIX) $(MAKE) cross
ifdef WIN32CROSS
	$(CP) $(PREFIX)/bin/SDL.dll ../bin
endif

$(PREFIX)/lib/libSDL_image.a:	$(PREFIX)/lib/libjpeg.a
$(PREFIX)/lib/libSDL_image.a:	$(PREFIX)/lib/libtiff.a
$(PREFIX)/lib/libSDL_image.a:	$(PREFIX)/lib/libpng.a

$(PREFIX)/lib/libSDL_image.a:	$(SDLIMAGE)/.ok
	cd $(SDLIMAGE) && CC="$(CC) $(LDFLAGS)" CPPFLAGS=-I$(PREFIX)/include LDFLAGS="-L$(PREFIX)/lib" LIBPNG_CFLAGS= LIBPNG_LIBS=-lpng ./configure --prefix=$(PREFIX) --with-sdl-prefix=$(PREFIX) --host=$(HOST) --disable-webp
	cd $(SDLIMAGE) && $(MAKE)
	cd $(SDLIMAGE) && $(MAKE) install
ifdef WIN32CROSS
	$(CP) $(PREFIX)/bin/SDL_image.dll ../bin
endif

$(SDLIMAGE)/.ok:	archives/$(SDLIMAGEARCH)
	$(TAR) xzf $<
	touch $@

$(PREFIX)/lib/libSDL_ttf.a:	$(PREFIX)/lib/libfreetype.a

$(PREFIX)/lib/libSDL_ttf.a:	$(SDLTTF)/.ok
	cd $(SDLTTF) && PKG_CONFIG_PATH=$(PREFIX)/lib/pkgconfig \
	  CC=$(CC) CPPFLAGS=-I$(PREFIX)/include LDFLAGS="-L$(PREFIX)/lib $(LDFLAGS)" \
	  ./configure --prefix=$(PREFIX) --with-sdl-prefix=$(PREFIX) --with-freetype-prefix=$(PREFIX) --host=$(HOST)
	cd $(SDLTTF) && $(MAKE)
	cd $(SDLTTF) && $(MAKE) install
ifdef WIN32CROSS
	$(CP) $(PREFIX)/bin/SDL_ttf.dll ../bin
endif

$(SDLTTF)/.ok:	archives/$(SDLTTFARCH)
	$(TAR) xzf $<
	touch $@

$(PREFIX)/lib/libfreetype.a:	$(PREFIX)/lib/libpng.a

$(PREFIX)/lib/libfreetype.a:	$(FREETYPE)/.ok
	cd $(FREETYPE) && ./configure --build=$(shell $(BUILD_CC) -dumpmachine) --host=$(HOST) \
	  --prefix=$(PREFIX) --enable-freetype-config \
	  PKG_CONFIG_LIBDIR=$(PREFIX)/lib/pkgconfig LDFLAGS="$(LDFLAGS)"
	cd $(FREETYPE) && $(MAKE)
	cd $(FREETYPE) && $(MAKE) install
ifdef WIN32CROSS
	$(CP) $(PREFIX)/bin/libfreetype*.dll ../bin
endif

$(FREETYPE)/.ok:	archives/$(FREETYPEARCH)
	$(TAR) xzf $<
	touch $@

$(PREFIX)/lib/libjpeg.a:	$(JPEGDIR)/.ok
	cd $(JPEGDIR) && CC=$(CC) ./configure --prefix=$(PREFIX) --host=$(HOST)
	cd $(JPEGDIR) && $(MAKE)
	cd $(JPEGDIR) && $(MAKE) install
ifdef WIN32CROSS
	$(CP) $(PREFIX)/bin/libjpeg*.dll ../bin
endif

$(JPEGDIR)/.ok:	archives/$(JPEGARCH)
	$(TAR) xzf $<
	touch $@

$(PREFIX)/lib/libtiff.a:	$(PREFIX)/lib/libjpeg.a
$(PREFIX)/lib/libtiff.a:	$(PREFIX)/lib/libz.a

$(PREFIX)/lib/libtiff.a:	$(LIBTIFF)/.ok
	cd $(LIBTIFF) && ./configure --prefix=$(PREFIX) --host=$(HOST) \
	  --with-zlib-include-dir=$(PREFIX)/include --with-zlib-lib-dir=$(PREFIX)/lib \
	  --with-jpeg-include-dir=$(PREFIX)/include --with-jpeg-lib-dir=$(PREFIX)/lib
	cd $(LIBTIFF) && $(MAKE)
	cd $(LIBTIFF) && $(MAKE) install

$(LIBTIFF)/.ok:	archives/$(LIBTIFFARCH)
	$(TAR) xzf $<
	touch $@
	
$(PREFIX)/lib/libpng.a:	$(PREFIX)/lib/libz.a

$(PREFIX)/lib/libpng.a:	$(LIBPNG)/.ok
	cd $(LIBPNG) && CC="$(CC) $(LDFLAGS)" CPPFLAGS=-I$(PREFIX)/include LDFLAGS="-L$(PREFIX)/lib" ./configure --prefix=$(PREFIX) --host=$(HOST) --enable-silent-rules
	cd $(LIBPNG) && $(MAKE)
	cd $(LIBPNG) && $(MAKE) install
ifdef WIN32CROSS
	$(CP) $(PREFIX)/bin/libpng*.dll ../bin
endif

$(LIBPNG)/.ok:	archives/$(LIBPNGARCH)
	$(TAR) xzf $<
	touch $@

$(PREFIX)/lib/libz.a:	$(ZLIB)/.ok
	cd $(ZLIB) && $(MAKE) -fwin32/Makefile.gcc PREFIX=$(shell echo $(CC) | sed 's/^\(.*\)gcc/\1/') LOC="$(LDFLAGS) $(CFLAGS)"
	cd $(ZLIB) && INCLUDE_PATH=$(PREFIX)/include LIBRARY_PATH=$(PREFIX)/lib BINARY_PATH=$(PREFIX)/bin $(MAKE) install -fwin32/Makefile.gcc SHARED_MODE=1
ifdef WIN32CROSS
	$(CP) $(PREFIX)/bin/zlib*.dll ../bin
endif

$(ZLIB)/.ok:	archives/$(ZLIBARCH)
	$(TAR) xzf $<
	touch $@

recoil:	$(RECOIL)/.ok

$(RECOIL)/.ok:	archives/$(RECOILARCH)
	$(TAR) xzf $<
	touch $@

archives/$(SDLDEVEL):
	@$(MKDIR) $(@D)
	cd $(@D) && $(GETURL) $(SDLDEVELURL)

archives/$(SDLIMAGEARCH):
	@$(MKDIR) $(@D)
	cd $(@D) && $(GETURL) $(SDLIMAGEURL)

archives/$(SDLTTFARCH):
	@$(MKDIR) $(@D)
	cd $(@D) && $(GETURL) $(SDLTTFURL)

archives/$(LIBPNGARCH):
	@$(MKDIR) $(@D)
	cd $(@D) && ( $(GETURL) $(LIBPNGURL) || $(GETURL) $(LIBPNGURLALT) )

archives/$(JPEGARCH):
	@$(MKDIR) $(@D)
	cd $(@D) && $(GETURL) $(JPEGURL)

archives/$(LIBTIFFARCH):
	@$(MKDIR) $(@D)
	cd $(@D) && ( $(GETURL) $(LIBTIFFURL) || $(GETURL) $(LIBTIFFURLALT) )

archives/$(ZLIBARCH):
	@$(MKDIR) $(@D)
	cd $(@D) && $(GETURL) $(ZLIBURL)

archives/$(FREETYPEARCH):
	@$(MKDIR) $(@D)
	cd $(@D) && $(GETURL) $(FREETYPEURL)

archives/$(LUAARCH):
	@$(MKDIR) $(@D)
	cd $(@D) && $(GETURL) $(LUAURL)

archives/$(RECOILARCH):
	@$(MKDIR) $(@D)
	cd $(@D) && ( $(GETURL) $(RECOILURL) || $(GETURL) $(RECOILURLALT) )
