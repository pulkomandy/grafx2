VERSIONTAG = ../obj/win32/versiontag
VERSION := $(shell cat $(VERSIONTAG))

SOURCES = ../src-$(VERSION).tgz
INSTALLER = ../grafx2-$(VERSION).win32.exe

all:	$(INSTALLER)

$(INSTALLER):	WinInstaller.nsi $(SOURCES) $(VERSIONTAG)
	sed 's/src-.*.tgz/src-$(VERSION).tgz/' $< | \
	sed 's/VERSION .*/VERSION $(VERSION)/' > new_$<
	makensis new_$<

$(SOURCES):	../grafx2-$(VERSION)-win32.zip 
	cd .. && unzip grafx2-$(VERSION)-win32.zip src-$(VERSION).tgz

../grafx2-$(VERSION)-win32.zip:
	cd ../src && WIN32CROSS=1 make ziprelease
