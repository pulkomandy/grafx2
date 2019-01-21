SDLDEVEL = SDL-devel-1.2.15-mingw32.tar.gz
SDLDEVELURL = https://www.libsdl.org/release/$(SDLDEVEL)
SDLDEVELPATCH = SDL-1.2.15.patch
SDLIMAGE = SDL_image-1.2.12
SDLIMAGEARCH = SDL_image-1.2.12.tar.gz
SDLIMAGEURL = https://www.libsdl.org/projects/SDL_image/release/$(SDLIMAGEARCH)
SDLIMAGEPATCHES = SDL_image-1.2.12.XCF_infinite_loop.patch \
                  SDL_image-1.2.12.XCF_v11_64bits_offsets.patch \
                  SDL_image_XCF_v11_load_level.patch \
                  SDL_image-1.2.12-png1.6.patch \
                  SDL_image-1.2.12-png_const_colorp.patch \
                  SDL_image-1.2.12-png_const_fix.patch
SDLTTF=SDL_ttf-2.0.11
SDLTTFARCH=$(SDLTTF).tar.gz
SDLTTFURL=https://www.libsdl.org/projects/SDL_ttf/release/$(SDLTTFARCH)
LIBPNG = libpng-1.6.36
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
LUAVER=5.3.5
LUA=lua-$(LUAVER)
LUAARCH=$(LUA).tar.gz
LUAURL=https://www.lua.org/ftp/$(LUAARCH)
# https://storage.googleapis.com/downloads.webmproject.org/releases/webp/libwebp-0.6.1.tar.gz
RECOILVER=4.3.1
RECOIL=recoil-$(RECOILVER)
RECOILARCH=$(RECOIL).tar.gz
# https://downloads.sourceforge.net/project/recoil/recoil/4.3.0/recoil-4.3.0.tar.gz
RECOILURL=https://downloads.sourceforge.net/project/recoil/recoil/$(RECOILVER)/$(RECOILARCH)
RECOILURLALT=http://nanard.free.fr/grafx2/$(RECOILARCH)

PREFIX = $(PWD)/usr

MKDIR = mkdir -p
CP = cp -v
TAR = $(shell which tar)
GETURL = $(shell WGET=`which wget` ; if [ "$$?" = "0" ] && [ -x "$$WGET" ] ; \
                 then echo "$$WGET" ; \
                 else echo "curl -O -L --max-time 120" ; fi )

BUILD_CC := $(CC)
STRIP = strip

# There is no uname under windows, but we can guess we are there with the COMSPEC env.var
# Windows specific
ifdef COMSPEC
WIN32 = 1
endif

ifdef WIN32CROSS
WIN32 = 1
CROSS_CC ?= $(shell which i686-w64-mingw32-gcc || which mingw32-gcc)
CROSS_AR ?= $(shell which i686-w64-mingw32-ar || which mingw32-ar)
CROSS_RANLIB ?= $(shell which i686-w64-mingw32-ranlib || which mingw32-ranlib)
CROSS_STRIP ?= $(shell which i686-w64-mingw32-strip || which mingw32-strip)
CROSS_LDFLAGS += -static-libgcc
CC = $(CROSS_CC)
AR = $(CROSS_AR)
RANLIB = $(CROSS_RANLIB)
STRIP = $(CROSS_STRIP)
CFLAGS = $(CROSS_CFLAGS)
LDFLAGS = $(CROSS_LDFLAGS)
endif

HOST = $(shell $(CC) -dumpmachine)
#HOST = i686-pc-mingw32
DATE = $(shell date -R)

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
ifdef WIN32
	cd $(LUA) && $(MAKE) PLAT=mingw CC=$(CC) RANLIB=$(RANLIB)
	cd $(LUA) && $(MAKE) install PLAT=mingw INSTALL_TOP=$(PREFIX) TO_BIN="lua.exe luac.exe"
	$(MKDIR) ../bin && for f in $(LUA)/src/lua*.dll ; do \
	    $(CP) $$f ../bin ; \
	    $(STRIP) ../bin/`basename $$f` ; \
	  done
	echo "The Windows distribution of Grafx2 is linked with Lua v$(LUAVER)" > ../doc/README-lua.txt
	grep LUA_COPYRIGHT $(LUA)/src/lua.h | cut -d'"' -f 2 >> ../doc/README-lua.txt

	echo "" >> ../doc/README-lua.txt
	echo "License : http://www.lua.org/license.html" >> ../doc/README-lua.txt
	# extract license from readme.html
	awk '/BLOCKQUOTE/{flag=1-flag;next}flag' $(LUA)/doc/readme.html | grep -v '<P>' | tail --lines +2 >> ../doc/README-lua.txt
endif

$(LUA)/.ok:	archives/$(LUAARCH)
	$(TAR) xzf $<
	touch $@

$(PREFIX)/lib/libSDLmain.a:	archives/$(SDLDEVEL)
	$(TAR) xzf $<
	patch -p0 < $(SDLDEVELPATCH)
	$(MKDIR) $(PREFIX)
	cd SDL-1.2.15 && CROSS_PATH=$(PREFIX) $(MAKE) cross
ifdef WIN32
	$(MKDIR) ../bin && $(CP) $(PREFIX)/bin/SDL.dll ../bin && $(STRIP) ../bin/SDL.dll
	echo "The following file:" > ../doc/README-SDL.txt
	echo "" >> ../doc/README-SDL.txt
	echo "	SDL.dll" >> ../doc/README-SDL.txt
	echo "" >> ../doc/README-SDL.txt
	echo "is the runtime environment for the SDL library." >> ../doc/README-SDL.txt
	tail --lines +3 SDL-1.2.15/README-SDL.txt >> ../doc/README-SDL.txt
endif

$(PREFIX)/lib/libSDL_image.a:	$(PREFIX)/lib/libjpeg.a
$(PREFIX)/lib/libSDL_image.a:	$(PREFIX)/lib/libtiff.a
$(PREFIX)/lib/libSDL_image.a:	$(PREFIX)/lib/libpng.a

$(PREFIX)/lib/libSDL_image.a:	$(SDLIMAGE)/.ok
	cd $(SDLIMAGE) && CC="$(CC) $(LDFLAGS)" CPPFLAGS=-I$(PREFIX)/include LDFLAGS="-L$(PREFIX)/lib" LIBPNG_CFLAGS= LIBPNG_LIBS=-lpng ./configure --prefix=$(PREFIX) --with-sdl-prefix=$(PREFIX) --host=$(HOST) --disable-webp
	cd $(SDLIMAGE) && $(MAKE)
	cd $(SDLIMAGE) && $(MAKE) install
ifdef WIN32
	$(MKDIR) ../bin && $(CP) $(PREFIX)/bin/SDL_image.dll ../bin && $(STRIP) ../bin/SDL_image.dll
	echo "$(SDLIMAGE)" > ../doc/README-SDL_image.txt
	echo "" >> ../doc/README-SDL_image.txt
	echo "dependencies :" >> ../doc/README-SDL_image.txt
	echo " - $(LIBPNG)" >> ../doc/README-SDL_image.txt
	echo " - http://www.ijg.org/ JPEG lib $(JPEGVER)" >> ../doc/README-SDL_image.txt
	echo " - $(LIBTIFF)" >> ../doc/README-SDL_image.txt
	echo "" >> ../doc/README-SDL_image.txt
	echo "License :" >> ../doc/README-SDL_image.txt
	cat $(SDLIMAGE)/COPYING >> ../doc/README-SDL_image.txt
endif

$(SDLIMAGE)/.ok:	archives/$(SDLIMAGEARCH)
	$(TAR) xzf $<
	cd $(SDLIMAGE) ; for p in $(SDLIMAGEPATCHES) ; do echo "applying $$p" ; patch -p1 < ../$$p ; done
	touch $@

$(PREFIX)/lib/libSDL_ttf.a:	$(PREFIX)/lib/libfreetype.a

$(PREFIX)/lib/libSDL_ttf.a:	$(SDLTTF)/.ok
	cd $(SDLTTF) && PKG_CONFIG_PATH=$(PREFIX)/lib/pkgconfig \
	  CC=$(CC) CPPFLAGS=-I$(PREFIX)/include LDFLAGS="-L$(PREFIX)/lib $(LDFLAGS)" \
	  ./configure --prefix=$(PREFIX) --with-sdl-prefix=$(PREFIX) --with-freetype-prefix=$(PREFIX) --host=$(HOST)
	cd $(SDLTTF) && $(MAKE)
	cd $(SDLTTF) && $(MAKE) install
ifdef WIN32
	$(MKDIR) ../bin && $(CP) $(PREFIX)/bin/SDL_ttf.dll ../bin && $(STRIP) ../bin/SDL_ttf.dll
	echo "$(SDLTTF)" > ../doc/README-SDL_ttf.txt
	echo "" >> ../doc/README-SDL_ttf.txt
	echo "dependencies :" >> ../doc/README-SDL_ttf.txt
	echo " - $(FREETYPE)" >> ../doc/README-SDL_ttf.txt
	echo "" >> ../doc/README-SDL_ttf.txt
	echo "License :" >> ../doc/README-SDL_ttf.txt
	cat $(SDLTTF)/COPYING >> ../doc/README-SDL_ttf.txt
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
ifdef WIN32
	$(MKDIR) ../bin && for f in $(PREFIX)/bin/libfreetype*.dll ; do \
	    $(CP) $$f ../bin ; \
	    $(STRIP) ../bin/`basename $$f` ; \
	  done
	echo "$(FREETYPE)" > ../doc/README-freetype.txt
	echo "" >> ../doc/README-freetype.txt
	echo "License :" >> ../doc/README-freetype.txt
	echo "" >> ../doc/README-freetype.txt
	cat $(FREETYPE)/docs/GPLv2.TXT >> ../doc/README-freetype.txt
endif

$(FREETYPE)/.ok:	archives/$(FREETYPEARCH)
	$(TAR) xzf $<
	touch $@

$(PREFIX)/lib/libjpeg.a:	$(JPEGDIR)/.ok
	cd $(JPEGDIR) && CC=$(CC) ./configure --prefix=$(PREFIX) --host=$(HOST)
	cd $(JPEGDIR) && $(MAKE)
	cd $(JPEGDIR) && $(MAKE) install
ifdef WIN32
	$(MKDIR) ../bin && for f in $(PREFIX)/bin/libjpeg*.dll ; do \
	    $(CP) $$f ../bin ; \
	    $(STRIP) ../bin/`basename $$f` ; \
	  done
	$(CP) $(JPEGDIR)/README ../doc/README-jpeg.txt
endif

$(JPEGDIR)/.ok:	archives/$(JPEGARCH)
	$(TAR) xzf $<
	touch $@

$(PREFIX)/lib/libtiff.a:	$(PREFIX)/lib/libjpeg.a
$(PREFIX)/lib/libtiff.a:	$(PREFIX)/lib/libz.a

$(PREFIX)/lib/libtiff.a:	$(LIBTIFF)/.ok
	cd $(LIBTIFF) && CC="$(CC) $(LDFLAGS)" ./configure --prefix=$(PREFIX) --host=$(HOST) \
	  --with-zlib-include-dir=$(PREFIX)/include --with-zlib-lib-dir=$(PREFIX)/lib \
	  --with-jpeg-include-dir=$(PREFIX)/include --with-jpeg-lib-dir=$(PREFIX)/lib
	cd $(LIBTIFF) && $(MAKE)
	cd $(LIBTIFF) && $(MAKE) install
ifdef WIN32
	$(MKDIR) ../bin && for f in $(PREFIX)/bin/libtiff*.dll ; do \
	    $(CP) $$f ../bin ; \
	    $(STRIP) ../bin/`basename $$f` ; \
	  done
	echo "$(LIBTIFF)" > ../doc/README-tiff.txt
	echo "" >> ../doc/README-tiff.txt
	echo "License :" >> ../doc/README-tiff.txt
	echo "" >> ../doc/README-tiff.txt
	cat $(LIBTIFF)/COPYRIGHT >> ../doc/README-tiff.txt
endif

$(LIBTIFF)/.ok:	archives/$(LIBTIFFARCH)
	$(TAR) xzf $<
	touch $@
	
$(PREFIX)/lib/libpng.a:	$(PREFIX)/lib/libz.a

$(PREFIX)/lib/libpng.a:	$(LIBPNG)/.ok
	cd $(LIBPNG) && CC="$(CC) $(LDFLAGS)" CPPFLAGS=-I$(PREFIX)/include LDFLAGS="-L$(PREFIX)/lib" ./configure --prefix=$(PREFIX) --host=$(HOST) --enable-silent-rules
	cd $(LIBPNG) && $(MAKE)
	cd $(LIBPNG) && $(MAKE) install
ifdef WIN32
	$(MKDIR) ../bin && for f in $(PREFIX)/bin/libpng*.dll ; do \
	    $(CP) $$f ../bin ; \
	    $(STRIP) ../bin/`basename $$f` ; \
	  done
	echo "$(LIBPNG)" > ../doc/README-libpng.txt
	echo "" >> ../doc/README-libpng.txt
	echo "License :" >> ../doc/README-libpng.txt
	echo "" >> ../doc/README-libpng.txt
	cat $(LIBPNG)/LICENSE >> ../doc/README-libpng.txt
endif

$(LIBPNG)/.ok:	archives/$(LIBPNGARCH)
	$(TAR) xzf $<
	touch $@

$(PREFIX)/lib/libz.a:	$(ZLIB)/.ok
	cd $(ZLIB) && $(MAKE) -fwin32/Makefile.gcc PREFIX=$(shell echo $(CC) | sed 's/^\(.*\)gcc/\1/') LOC="$(LDFLAGS) $(CFLAGS)"
	cd $(ZLIB) && INCLUDE_PATH=$(PREFIX)/include LIBRARY_PATH=$(PREFIX)/lib BINARY_PATH=$(PREFIX)/bin $(MAKE) install -fwin32/Makefile.gcc SHARED_MODE=1
ifdef WIN32
	$(MKDIR) ../bin && for f in $(PREFIX)/bin/zlib*.dll ; do \
	    $(CP) $$f ../bin ; \
	    $(STRIP) ../bin/`basename $$f` ; \
	  done
	$(CP) $(ZLIB)/README ../doc/README-zlib1.txt
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
