DOXYGEN = doxygen
GIT_REVISION = $(shell cat ../src/version.c | cut -f 2 -d '"')
LABEL = $(shell cat ../src/pversion.c | cut -f 2 -d '"')
VERSION = $(subst wip.,wip,$(LABEL).$(GIT_REVISION))

SOURCES = $(wildcard ../src/*.c) $(wildcard ../src/*.h)

.PHONY:	all gifanalyzer sdl_image_test test_iff doxygen

all:	gifanalyzer sdl_image_test test_iff

gifanalyzer:
	$(MAKE) -C $@

sdl_image_test:
	$(MAKE) -C $@

test_iff:
	$(MAKE) -C $@

doxygen:	../doc/doxygen/html/index.html

../doc/doxygen/html/index.html:	Doxyfile $(SOURCES)
	$(DOXYGEN)

Doxyfile:	../src/version.c ../src/pversion.c
	sed -e 's/^\(PROJECT_NUMBER[ ]*=[ ]*\).*$$/\1 "$(VERSION)"/' $@ > $@.tmp
	mv $@.tmp $@
