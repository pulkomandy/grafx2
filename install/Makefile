# (c) 2018 Thomas Bernard

MAKENSIS = makensis
UNZIP = unzip -o
SED = sed

VERSIONTAG = ../obj/win32-sdl/versiontag
VERSION := $(shell cat $(VERSIONTAG))

SOURCES = ../src-$(VERSION).tgz
INSTALLER = grafx2-$(VERSION).win32.exe

all:	$(INSTALLER)

$(INSTALLER):	WinInstaller.nsi $(SOURCES) $(VERSIONTAG)
	$(SED) 's/VERSION .*/VERSION $(VERSION)/' $< | $(MAKENSIS) -

$(SOURCES):	../grafx2-$(VERSION)-win32.zip 
	cd .. && $(UNZIP) grafx2-$(VERSION)-win32.zip src-$(VERSION).tgz
	touch $@

../grafx2-$(VERSION)-win32.zip:	$(VERSIONTAG)
	cd ../src && WIN32CROSS=1 $(MAKE) ziprelease
