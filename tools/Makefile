DOXYGEN = doxygen
TAR = tar

ifneq ($(shell test -f ../src/version.c && echo exists),exists)
$(error please build grafx2 first. missing version.c file.)
endif

API ?= sdl

TOOLS = gifanalyzer test_iff

ifeq (sdl,$(findstring sdl,$(API)))
TOOLS += sdl_image_test
endif

GIT_REVISION = $(shell cat ../src/version.c | cut -f 2 -d '"')
LABEL = $(shell cat ../src/pversion.c | cut -f 2 -d '"')
VERSION = $(subst wip.,wip,$(LABEL).$(GIT_REVISION))

SOURCES = $(wildcard ../src/*.c) $(wildcard ../src/*.h)

DOCARCHIVE = ../grafx2-$(VERSION)-doxygen.tgz

.PHONY:	all gifanalyzer sdl_image_test test_iff doxygen

all:	$(TOOLS)

gifanalyzer:
	$(MAKE) -C $@

sdl_image_test:
	$(MAKE) -C $@

test_iff:
	$(MAKE) -C $@

doxygenarchive:	$(DOCARCHIVE)

$(DOCARCHIVE):	../doc/doxygen/html/index.html
	cd ../doc && $(TAR) czf $@ doxygen/

doxygen:	../doc/doxygen/html/index.html

../doc/doxygen/html/index.html:	Doxyfile $(SOURCES)
	$(DOXYGEN)

Doxyfile:	../src/version.c ../src/pversion.c
	sed -e 's/^\(PROJECT_NUMBER[ ]*=[ ]*\).*$$/\1 "$(VERSION)"/' $@ > $@.tmp
	mv $@.tmp $@
