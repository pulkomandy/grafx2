# (c) 2018 Thomas Bernard

MAKENSIS = makensis
UNZIP = unzip -o
SED = sed

API ?= sdl

GIT_REVISION := $(shell cat ../src/version.c | cut -f 2 -d '"')
LABEL := $(shell cat ../src/pversion.c | cut -f 2 -d '"')
VERSION := $(subst wip.,wip,$(LABEL).$(GIT_REVISION))

SOURCES = ../src-$(VERSION).tgz
INSTALLER = grafx2-$(API)-$(VERSION).win32.exe
NSISCRIPT = WinInstaller.nsi

.PHONY:	all updateversion

all:	$(INSTALLER)

$(INSTALLER):	$(NSISCRIPT) $(SOURCES) $(VERSIONTAG)
ifdef WIN32CROSS
	for f in ../doc/*.txt ../share/grafx2/gfx2def.ini ; do unix2dos $$f ; done
endif
	$(SED) -e 's/VERSION .*/VERSION $(VERSION)/' -e 's/API .*/API $(API)/' $< | $(MAKENSIS) -
ifdef WIN32CROSS
	for f in ../doc/*.txt ../share/grafx2/gfx2def.ini ; do dos2unix $$f ; done
endif

$(SOURCES): $(wildcard ../src/*.c) $(wildcard ../src/*.h)
	cd ../src && WIN32CROSS=1 $(MAKE) $(SOURCES)

updateversion:
	$(SED) -e 's/VERSION .*/VERSION $(VERSION)/' $(NSISCRIPT) > $(NSISCRIPT).tmp
	if ! diff -q -i $(NSISCRIPT) $(NSISCRIPT).tmp ; then mv $(NSISCRIPT).tmp $(NSISCRIPT) ; else rm $(NSISCRIPT).tmp ; fi
