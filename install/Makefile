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

all:	$(INSTALLER)

$(INSTALLER):	WinInstaller.nsi $(SOURCES) $(VERSIONTAG)
	$(SED) -e 's/VERSION .*/VERSION $(VERSION)/' -e 's/API .*/API $(API)/' $< | $(MAKENSIS) -

$(SOURCES): $(wildcard ../src/*.c) $(wildcard ../src/*.h)
	cd ../src && WIN32CROSS=1 $(MAKE) $(SOURCES)
