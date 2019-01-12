CP = cp
SDL ?= libsdl2

all:
	@echo "nothing to do..."

# fetch files from macports
MACPORTSPATH = $(shell which port | sed 's;/bin/port$$;;' )
DOCPATH = $(MACPORTSPATH)/share/doc
PACKAGES = $(SDL) $(SDL)_image jpeg tiff webp
PACKAGES += $(SDL)_ttf freetype
PACKAGES += libpng zlib lua

READMES = $(PACKAGES:%=README-%.txt)

macports:	$(READMES)

clean:
	$(RM) $(READMES)

#	for p in $(PACKAGES) ; do \
	  echo $(DOCPATH)/$$p ; \
	  ls $(DOCPATH)/$$p ; \
	done

README-libsdl2.txt:	$(DOCPATH)/libsdl2/README-SDL.txt $(DOCPATH)/libsdl2/COPYING.txt
	p=$(@:README-%.txt=%) ; echo "$$p $$(port info --version $$p)" > $@
	cat $(DOCPATH)/libsdl2/README-SDL.txt >> $@
	cat $(DOCPATH)/libsdl2/COPYING.txt >> $@

README-libsdl2_image.txt:
	p=$(@:README-%.txt=%) ; echo "$$p $$(port info --version $$p)" > $@
	cat $(DOCPATH)/libsdl2_image/COPYING.txt >> $@

README-tiff.txt:	$(DOCPATH)/tiff/COPYRIGHT
	p=$(@:README-%.txt=%) ; echo "$$p $$(port info --version $$p)" > $@
	echo "" >> $@
	cat $(DOCPATH)/tiff/COPYRIGHT >> $@

README-lua.txt: $(DOCPATH)/lua/README $(DOCPATH)/lua/COPYRIGHT
	$(CP) $< $@
	echo "" >> $@
	cat $(DOCPATH)/lua/COPYRIGHT >> $@

README-libpng.txt:	$(DOCPATH)/libpng/LICENSE
	echo "libpng $$(port info --version libpng)" > $@
	echo "" >> $@
	echo "License :" >> $@
	echo "" >> $@
	cat $< >> $@

README-freetype.txt:	$(DOCPATH)/freetype/LICENSE.TXT
	echo "freetype $$(port info --version freetype)" > $@
	echo "" >> $@
	echo "License :" >> $@
	cat $< >> $@

README-%.txt: $(DOCPATH)/%/README
	p=$(@:README-%.txt=%) ; echo "$$p $$(port info --version $$p)" > $@
	echo "" >> $@
	cat $< >> $@
	f=$(DOCPATH)/$(@:README-%.txt=%)/COPYING ; \
	  if [ -f "$$f" ] ; then echo "License :" >> $@ ; cat "$$f" >> $@ ; fi
