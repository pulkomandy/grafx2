# (c) 2018-2019 Thomas BERNARD
#
# Makefile to build GrafX2 library dependencies.
#
# Variables :
# - API : sdl(default), sdl2
# - WIN32CROSS
#
# most useful targets :
# - download : download .tar.gz files from the WWW
# - all (default) : download and build...
# - clean : remove built libraries
# - clean_archives : remove downloaded .tar.gz files
#
# examples :
# $ make WIN32CROSS=1 API=sdl2
# $ make download
# $ make libtiff

API ?= sdl

PLATFORM = $(shell uname)

SDL = SDL-1.2.15
SDLARCH = $(SDL).tar.gz
SDLURL = https://www.libsdl.org/release/$(SDLARCH)
SDLSHA256 = d6d316a793e5e348155f0dd93b979798933fb98aa1edebcc108829d6474aad00
SDLPATCHES = SDL-1.2.15-patch-src_video_x11_SDL_x11sym.h \
             SDL-1.2.15-quartzvideo.patch
SDLDEVEL = SDL-devel-1.2.15-mingw32.tar.gz
SDLDEVELURL = https://www.libsdl.org/release/$(SDLDEVEL)
SDLDEVELPATCH = SDL-1.2.15.patch
SDLIMAGE = SDL_image-1.2.12
SDLIMAGEARCH = $(SDLIMAGE).tar.gz
SDLIMAGEURL = https://www.libsdl.org/projects/SDL_image/release/$(SDLIMAGEARCH)
SDLIMAGESHA256 = 0b90722984561004de84847744d566809dbb9daf732a9e503b91a1b5a84e5699
SDLIMAGEPATCHES = SDL_image-1.2.12.XCF_infinite_loop.patch \
                  SDL_image-1.2.12.XCF_v11_64bits_offsets.patch \
                  SDL_image_XCF_v11_load_level.patch \
                  SDL_image-1.2.12-png1.6.patch \
                  SDL_image-1.2.12-png_const_colorp.patch \
                  SDL_image-1.2.12-png_const_fix.patch
SDLTTF=SDL_ttf-2.0.11
SDLTTFARCH=$(SDLTTF).tar.gz
SDLTTFURL=https://www.libsdl.org/projects/SDL_ttf/release/$(SDLTTFARCH)
SDLTTFSHA256 = 724cd895ecf4da319a3ef164892b72078bd92632a5d812111261cde248ebcdb7
SDL2 = SDL2-2.0.9
SDL2ARCH = $(SDL2).tar.gz
SDL2URL = https://www.libsdl.org/release/$(SDL2ARCH)
SDL2SHA256 = 255186dc676ecd0c1dbf10ec8a2cc5d6869b5079d8a38194c2aecdff54b324b1
SDL2DEVEL = SDL2-devel-2.0.9-mingw.tar.gz
SDL2DEVELURL = https://www.libsdl.org/release/$(SDL2DEVEL)
SDL2DEVELPATCH = SDL2-devel.patch
SDL2IMAGE = SDL2_image-2.0.4
SDL2IMAGEARCH = $(SDL2IMAGE).tar.gz
SDL2IMAGEURL = https://www.libsdl.org/projects/SDL_image/release/$(SDL2IMAGEARCH)
SDL2IMAGESHA256 = e74ec49c2402eb242fbfa16f2f43a19582a74c2eabfbfb873f00d4250038ceac
SDL2TTF = SDL2_ttf-2.0.14
SDL2TTFARCH = $(SDL2TTF).tar.gz
SDL2TTFURL = https://www.libsdl.org/projects/SDL_ttf/release/$(SDL2TTFARCH)
SDL2TTFSHA256 = 34db5e20bcf64e7071fe9ae25acaa7d72bdc4f11ab3ce59acc768ab62fe39276
LIBPNG = libpng-1.6.36
LIBPNGARCH = $(LIBPNG).tar.gz
LIBPNGURL = https://download.sourceforge.net/libpng/$(LIBPNGARCH)
LIBPNGURLALT = ftp://ftp-osl.osuosl.org/pub/libpng/src/libpng16/$(LIBPNGARCH)
LIBPNGSHA256 = ca13c548bde5fb6ff7117cc0bdab38808acb699c0eccb613f0e4697826e1fd7d
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
LIBTIFFPATCHES = tiff-uint64_long_long.patch
# additionnal variables for the generic rules to work :
TIFFARCH = $(LIBTIFFARCH)
TIFFURL = $(LIBTIFFURL)
TIFFURLALT = $(LIBTIFFURLALT)
TIFFPATCHES = $(LIBTIFFPATCHES)
TIFFSHA256 = 2c52d11ccaf767457db0c46795d9c7d1a8d8f76f68b0b800a3dfe45786b996e4
ZLIBVER=1.2.11
ZLIB=zlib-$(ZLIBVER)
ZLIBARCH=$(ZLIB).tar.gz
ZLIBURL=https://www.zlib.net/$(ZLIBARCH)
ZLIBURLALT=http://downloads.sourceforge.net/project/libpng/zlib/$(ZLIBVER)/$(ZLIBARCH)
ZLIBSHA256=c3e5e9fdd5004dcb542feda5ee4f0ff0744628baf8ed2dd5d66f8ca1197cb1a1
FREETYPEVER=2.9.1
FREETYPE=freetype-$(FREETYPEVER)
FREETYPEARCH=$(FREETYPE).tar.gz
FREETYPEURL=https://download.savannah.gnu.org/releases/freetype/$(FREETYPEARCH)
FREETYPEURLALT=https://sourceforge.net/projects/freetype/files/freetype2/$(FREETYPEVER)/$(FREETYPEARCH)
FREETYPESHA256=ec391504e55498adceb30baceebd147a6e963f636eb617424bcfc47a169898ce
LUAVER=5.3.5
LUA=lua-$(LUAVER)
LUAARCH=$(LUA).tar.gz
LUAURL=https://www.lua.org/ftp/$(LUAARCH)
LUASHA256=0c2eed3f960446e1a3e4b9a1ca2f3ff893b6ce41942cf54d5dd59ab4b3b058ac
# https://storage.googleapis.com/downloads.webmproject.org/releases/webp/libwebp-0.6.1.tar.gz
RECOILVER=4.3.1
RECOIL=recoil-$(RECOILVER)
RECOILARCH=$(RECOIL).tar.gz
# https://downloads.sourceforge.net/project/recoil/recoil/4.3.0/recoil-4.3.0.tar.gz
RECOILURL=https://downloads.sourceforge.net/project/recoil/recoil/$(RECOILVER)/$(RECOILARCH)
RECOILURLALT=http://nanard.free.fr/grafx2/$(RECOILARCH)
RECOILSHA256=8f64fd21bafa180e6cbac5853ef65453f080385b3c53600af8360e938668e574

ARCHIVES = $(addprefix archives/, $(SDLARCH) $(SDLIMAGEARCH) $(SDLTTFARCH) \
             $(SDL2ARCH) $(SDL2IMAGEARCH) $(SDL2TTFARCH) $(LIBPNGARCH) \
             $(JPEGARCH) $(LIBTIFFARCH) $(ZLIBARCH) $(FREETYPEARCH) \
             $(LUAARCH) $(RECOILARCH))

PREFIX = $(PWD)/usr

MKDIR = mkdir -p
CP = cp -v
TAR = $(shell which tar)
SHA256CMD = $(shell SHASUM=`which shasum` ; if [ "$$?" = "0" ] && [ -x "$$SHASUM" ] ; \
                    then echo "($$SHASUM -a 256 | cut -f1 -d' ')" ; \
                    else OPENSSL=`which openssl` ; if [ "$$?" = "0" ] && [ -x "$$OPENSSL" ] ; \
                    then echo "($$OPENSSL sha256 -hex | sed 's/^.* //')" ; \
                    else echo "echo 'WARNING: no sha256 digest tool' >&2 ; echo $(SHA256)" ; \
                    fi ; fi )
# you need either wget or curl to download the files
# wget -nv option is used to avoid messing the output when using "make -j3"
GETURL = $(shell WGET=`which wget` ; if [ "$$?" = "0" ] && [ -x "$$WGET" ] ; \
                 then echo "$$WGET -nv" ; \
                 else echo 'curl -s -w "%{url_effective} downloaded to %{filename_effective} in %{time_total} seconds\\n" -R -O -L --max-time 120' ; fi )

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

.PHONY:	all clean clean_archives clean_all download libs libpng \
        libsdl libsdl_image libsdl_ttf \
        libsdl2 libsdl2_image libsdl2_ttf \
        libjpeg libtiff zlib freetype lua recoil

all:	libs

clean_all:	clean clean_archives

clean:
	$(RM) -r usr/ $(LIBPNG) $(ZLIB) $(SDLIMAGE) $(JPEGDIR) $(LIBTIFF)
	$(RM) -r $(SDLTTF) $(FREETYPE) $(SDL) $(LUA) $(RECOIL)
	$(RM) -r $(SDL2) $(SDL2IMAGE) $(SDL2TTF)

clean_archives:
	$(RM) -r archives

download:	$(ARCHIVES)

ifeq ($(API), sdl)
libs:	libpng libsdl libsdl_image libsdl_ttf lua
else
ifeq ($(API), sdl2)
libs:	libpng libsdl2 libsdl2_image libsdl2_ttf lua
else
libs:	libpng lua libtiff
endif
endif
libsdl:	$(PREFIX)/lib/libSDLmain.a
libsdl_image:	$(PREFIX)/lib/libSDL_image.a
libsdl_ttf:	$(PREFIX)/lib/libSDL_ttf.a
libsdl2:	$(PREFIX)/lib/libSDL2main.a
libsdl2_image:	$(PREFIX)/lib/libSDL2_image.a
libsdl2_ttf:	$(PREFIX)/lib/libSDL2_ttf.a
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
else
ifeq ($(PLATFORM), FreeBSD)
	cd $(LUA) && $(MAKE) freebsd install INSTALL_TOP=$(PREFIX)
else
ifeq ($(PLATFORM), Darwin)
	cd $(LUA) && $(MAKE) macosx install INSTALL_TOP=$(PREFIX)
else
ifeq ($(PLATFORM), Linux)
	cd $(LUA) && $(MAKE) linux install INSTALL_TOP=$(PREFIX)
else
	cd $(LUA) && $(MAKE) posix install INSTALL_TOP=$(PREFIX)
endif
endif
endif
endif

ifdef WIN32
$(PREFIX)/lib/libSDL2main.a:	archives/$(SDL2DEVEL)
	$(TAR) xzf $<
	patch -p0 < $(SDL2DEVELPATCH)
	$(MKDIR) $(PREFIX)
	cd $(SDL2) && $(MAKE) install-package arch=i686-w64-mingw32 prefix=$(PREFIX)
	$(MKDIR) ../bin && $(CP) $(PREFIX)/bin/SDL2.dll ../bin && $(STRIP) ../bin/SDL2.dll
	echo "The following file:" > ../doc/README-SDL2.txt
	echo "" >> ../doc/README-SDL2.txt
	echo "	SDL2.dll" >> ../doc/README-SDL2.txt
	echo "" >> ../doc/README-SDL2.txt
	echo "is the runtime environment for the SDL library." >> ../doc/README-SDL2.txt
	tail --lines +3 $(SDL2)/README-SDL.txt >> ../doc/README-SDL2.txt
else
SDL2DISABLE = --disable-video-wayland --disable-audio
ifeq ($(PLATFORM), Darwin)
SDL2DISABLE += --disable-video-x11
endif

$(PREFIX)/lib/libSDL2main.a:	$(SDL2)/.ok
	$(MKDIR) $(SDL2)/build
	cd $(SDL2)/build && ../configure --prefix=$(PREFIX) --host=$(HOST) \
	  $(SDL2DISABLE)
	cd $(SDL2)/build && $(MAKE) && $(MAKE) install
endif

$(PREFIX)/lib/libSDL2_image.a:	$(PREFIX)/lib/libjpeg.a
$(PREFIX)/lib/libSDL2_image.a:	$(PREFIX)/lib/libtiff.a
$(PREFIX)/lib/libSDL2_image.a:	$(PREFIX)/lib/libpng.a
$(PREFIX)/lib/libSDL2_image.a:	$(PREFIX)/lib/libSDL2main.a

$(PREFIX)/lib/libSDL2_image.a:	$(SDL2IMAGE)/.ok
	cd $(SDL2IMAGE) && PKG_CONFIG_PATH=$(PREFIX)/lib/pkgconfig \
	  CC="$(CC) $(LDFLAGS)" CPPFLAGS=-I$(PREFIX)/include LDFLAGS="-L$(PREFIX)/lib" \
	  ./configure --prefix=$(PREFIX) --host=$(HOST) --disable-sdltest \
	  --disable-webp --disable-imageio \
	  --disable-jpg-shared --disable-png-shared --disable-tif-shared
	cd $(SDL2IMAGE) && $(MAKE) && $(MAKE) install
ifdef WIN32
	$(MKDIR) ../bin && $(CP) $(PREFIX)/bin/SDL2_image.dll ../bin && $(STRIP) ../bin/SDL2_image.dll
endif
	echo "$(SDL2IMAGE)" > ../doc/README-SDL2_image.txt
	echo "" >> ../doc/README-SDL2_image.txt
	echo "dependencies :" >> ../doc/README-SDL2_image.txt
	echo " - $(LIBPNG)" >> ../doc/README-SDL2_image.txt
	echo " - http://www.ijg.org/ JPEG lib $(JPEGVER)" >> ../doc/README-SDL2_image.txt
	echo " - $(LIBTIFF)" >> ../doc/README-SDL2_image.txt
	echo "" >> ../doc/README-SDL2_image.txt
	echo "License :" >> ../doc/README-SDL2_image.txt
	cat $(SDL2IMAGE)/COPYING.txt | tr -d "\r" >> ../doc/README-SDL2_image.txt

$(PREFIX)/lib/libSDL2_ttf.a:	$(PREFIX)/lib/libfreetype.a
$(PREFIX)/lib/libSDL2_ttf.a:	$(PREFIX)/lib/libSDL2main.a

$(PREFIX)/lib/libSDL2_ttf.a:	$(SDL2TTF)/.ok
	cd $(SDL2TTF) && PKG_CONFIG_PATH=$(PREFIX)/lib/pkgconfig \
	  CC=$(CC) CPPFLAGS=-I$(PREFIX)/include LDFLAGS="-L$(PREFIX)/lib $(LDFLAGS)" \
	  ./configure --prefix=$(PREFIX) --with-sdl-prefix=$(PREFIX) \
	  --with-freetype-prefix=$(PREFIX) --host=$(HOST)
	cd $(SDL2TTF) && $(MAKE)
	cd $(SDL2TTF) && $(MAKE) install
ifdef WIN32
	$(MKDIR) ../bin && $(CP) $(PREFIX)/bin/SDL2_ttf.dll ../bin && $(STRIP) ../bin/SDL2_ttf.dll
endif
	echo "$(SDL2TTF)" > ../doc/README-SDL2_ttf.txt
	echo "" >> ../doc/README-SDL2_ttf.txt
	echo "dependencies :" >> ../doc/README-SDL2_ttf.txt
	echo " - $(FREETYPE)" >> ../doc/README-SDL2_ttf.txt
	echo "" >> ../doc/README-SDL2_ttf.txt
	echo "License :" >> ../doc/README-SDL2_ttf.txt
	cat $(SDL2TTF)/COPYING.txt | tr -d "\r" >> ../doc/README-SDL2_ttf.txt

ifdef WIN32
$(PREFIX)/lib/libSDLmain.a:	archives/$(SDLDEVEL)
	$(TAR) xzf $<
	patch -p0 < $(SDLDEVELPATCH)
	$(MKDIR) $(PREFIX)
	cd $(SDL) && CROSS_PATH=$(PREFIX) $(MAKE) cross
	$(MKDIR) ../bin && $(CP) $(PREFIX)/bin/SDL.dll ../bin && $(STRIP) ../bin/SDL.dll
	echo "The following file:" > ../doc/README-SDL.txt
	echo "" >> ../doc/README-SDL.txt
	echo "	SDL.dll" >> ../doc/README-SDL.txt
	echo "" >> ../doc/README-SDL.txt
	echo "is the runtime environment for the SDL library." >> ../doc/README-SDL.txt
	tail --lines +3 $(SDL)/README-SDL.txt >> ../doc/README-SDL.txt
else
SDLDISABLE = --disable-joystick --disable-audio
ifeq ($(PLATFORM), Darwin)
SDLDISABLE += --without-x
# the following trick was stolen from macports
ifeq ($(shell if [ "`uname -r | cut -f1 -d. `" = "8" ] && [ "`sysctl -n hw.vectorunit`" = "1" ] ; then echo "true" ; fi ), true)
SDLCFLAGS += -faltivec
endif
endif

$(PREFIX)/lib/libSDLmain.a:	$(SDL)/.ok
	cd $(SDL) && CFLAGS=$(SDLCFLAGS) ./configure --host=$(HOST) --prefix=$(PREFIX) \
	  $(SDLDISABLE)
	cd $(SDL) && $(MAKE)
	cd $(SDL) && $(MAKE) install
endif

ifndef WIN32
ifeq ($(PLATFORM), FreeBSD)
ADDLDFLAGS = -L/usr/local/lib
endif
endif

$(PREFIX)/lib/libSDL_image.a:	$(PREFIX)/lib/libjpeg.a
$(PREFIX)/lib/libSDL_image.a:	$(PREFIX)/lib/libtiff.a
$(PREFIX)/lib/libSDL_image.a:	$(PREFIX)/lib/libpng.a
$(PREFIX)/lib/libSDL_image.a:	$(PREFIX)/lib/libSDLmain.a

$(PREFIX)/lib/libSDL_image.a:	$(SDLIMAGE)/.ok
	cd $(SDLIMAGE) && CC="$(CC) $(LDFLAGS)" CPPFLAGS=-I$(PREFIX)/include \
	  LDFLAGS="-L$(PREFIX)/lib $(ADDLDFLAGS)" LIBPNG_CFLAGS= LIBPNG_LIBS=-lpng \
	  ./configure --prefix=$(PREFIX) --with-sdl-prefix=$(PREFIX) --host=$(HOST) \
	  --disable-webp --disable-imageio --disable-sdltest \
	  --disable-jpg-shared --disable-png-shared --disable-tif-shared
	cd $(SDLIMAGE) && $(MAKE)
	cd $(SDLIMAGE) && $(MAKE) install
ifdef WIN32
	$(MKDIR) ../bin && $(CP) $(PREFIX)/bin/SDL_image.dll ../bin && $(STRIP) ../bin/SDL_image.dll
endif
	echo "$(SDLIMAGE)" > ../doc/README-SDL_image.txt
	echo "" >> ../doc/README-SDL_image.txt
	echo "dependencies :" >> ../doc/README-SDL_image.txt
	echo " - $(LIBPNG)" >> ../doc/README-SDL_image.txt
	echo " - http://www.ijg.org/ JPEG lib $(JPEGVER)" >> ../doc/README-SDL_image.txt
	echo " - $(LIBTIFF)" >> ../doc/README-SDL_image.txt
	echo "" >> ../doc/README-SDL_image.txt
	echo "License :" >> ../doc/README-SDL_image.txt
	cat $(SDLIMAGE)/COPYING >> ../doc/README-SDL_image.txt

$(PREFIX)/lib/libSDL_ttf.a:	$(PREFIX)/lib/libfreetype.a
$(PREFIX)/lib/libSDL_ttf.a:	$(PREFIX)/lib/libSDLmain.a

$(PREFIX)/lib/libSDL_ttf.a:	$(SDLTTF)/.ok
	cd $(SDLTTF) && PKG_CONFIG_PATH=$(PREFIX)/lib/pkgconfig \
	  CC=$(CC) CPPFLAGS=-I$(PREFIX)/include LDFLAGS="-L$(PREFIX)/lib $(LDFLAGS) $(ADDLDFLAGS)" \
	  ./configure --prefix=$(PREFIX) --with-sdl-prefix=$(PREFIX) --with-freetype-prefix=$(PREFIX) --host=$(HOST)
	cd $(SDLTTF) && $(MAKE)
	cd $(SDLTTF) && $(MAKE) install
ifdef WIN32
	$(MKDIR) ../bin && $(CP) $(PREFIX)/bin/SDL_ttf.dll ../bin && $(STRIP) ../bin/SDL_ttf.dll
endif
	echo "$(SDLTTF)" > ../doc/README-SDL_ttf.txt
	echo "" >> ../doc/README-SDL_ttf.txt
	echo "dependencies :" >> ../doc/README-SDL_ttf.txt
	echo " - $(FREETYPE)" >> ../doc/README-SDL_ttf.txt
	echo "" >> ../doc/README-SDL_ttf.txt
	echo "License :" >> ../doc/README-SDL_ttf.txt
	cat $(SDLTTF)/COPYING >> ../doc/README-SDL_ttf.txt

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

# libjpeg archive name and directory name are not the same
# so the generic rule won't work
$(JPEGDIR)/.ok:	archives/$(JPEGARCH)
	$(TAR) xzf $<
	touch $@

$(PREFIX)/lib/libtiff.a:	$(PREFIX)/lib/libjpeg.a
$(PREFIX)/lib/libtiff.a:	$(PREFIX)/lib/libz.a

$(PREFIX)/lib/libtiff.a:	$(LIBTIFF)/.ok
	cd $(LIBTIFF) && CC="$(CC) $(LDFLAGS)" ./configure --prefix=$(PREFIX) --host=$(HOST) \
	  --with-zlib-include-dir=$(PREFIX)/include --with-zlib-lib-dir=$(PREFIX)/lib \
	  --with-jpeg-include-dir=$(PREFIX)/include --with-jpeg-lib-dir=$(PREFIX)/lib \
	  --disable-cxx --disable-webp --disable-lzma --disable-zstd
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

$(PREFIX)/lib/libz.a:	$(ZLIB)/.ok
ifdef WIN32
	cd $(ZLIB) && $(MAKE) -fwin32/Makefile.gcc PREFIX=$(shell echo $(CC) | sed 's/^\(.*\)gcc/\1/') LOC="$(LDFLAGS) $(CFLAGS)"
	cd $(ZLIB) && INCLUDE_PATH=$(PREFIX)/include LIBRARY_PATH=$(PREFIX)/lib BINARY_PATH=$(PREFIX)/bin $(MAKE) install -fwin32/Makefile.gcc SHARED_MODE=1
	$(MKDIR) ../bin && for f in $(PREFIX)/bin/zlib*.dll ; do \
	    $(CP) $$f ../bin ; \
	    $(STRIP) ../bin/`basename $$f` ; \
	  done
else
	# configure [--const] [--zprefix] [--prefix=PREFIX]  [--eprefix=EXPREFIX]
	#  [--static] [--64] [--libdir=LIBDIR] [--sharedlibdir=LIBDIR]
	#  [--includedir=INCLUDEDIR] [--archs="-arch i386 -arch x86_64"]
	cd $(ZLIB) && ./configure --prefix=$(PREFIX) && $(MAKE) test
	cd $(ZLIB) && $(MAKE) install
endif
	$(CP) $(ZLIB)/README ../doc/README-zlib1.txt

recoil:	$(RECOIL)/.ok

# generic rule to unpack tarball and apply patches
%/.ok:	archives/%.tar.gz
	$(TAR) xzf $<
	cd $(@D) ; for p in $($(shell echo $* | cut -d- -f1 | tr a-z A-Z | tr -d _)PATCHES) ; do echo "applying $$p" ; patch -p1 < ../$$p ; done
	touch $@

# the few following archive names won't work with the generic rule
archives/$(SDLDEVEL):
	@$(MKDIR) $(@D)
	cd $(@D) && $(GETURL) $(SDLDEVELURL)

archives/$(SDL2DEVEL):
	@$(MKDIR) $(@D)
	cd $(@D) && $(GETURL) $(SDL2DEVELURL)

archives/$(JPEGARCH):
	@$(MKDIR) $(@D)
	cd $(@D) && $(GETURL) $(JPEGURL)

# generic rule to download tarballs
archives/%.tar.gz:
	@$(MKDIR) $(@D)
	$(eval BASE := $(shell echo $* | cut -d- -f1 | tr a-z A-Z | tr -d _))
	$(eval URL = $($(BASE)URL))
	$(eval URLALT = $($(BASE)URLALT))
	$(eval SHA256 = $($(BASE)SHA256))
	@echo "$*: fetching $(URL) (or $(URLALT))"
	@cd $(@D) && ( $(GETURL) $(URL) || ( [ -n "$(URLALT)" ] && $(GETURL) $(URLALT) ) )
	@[ -z "$(SHA256)" ] || [ "`$(SHA256CMD) < $@`" = "$(SHA256)" ] || ( $(RM) $@ && echo "$@ SHA256 mismatch !" && false )
