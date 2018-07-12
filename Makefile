ifdef WIN32CROSS
OPT = WIN32CROSS=1 \

endif

.PHONY:	all tools grafx2 ziprelease 3rdparty win32installer \
        doc doxygen docarchive doxygenarchive

all:	grafx2 tools

doc:	doxygen

docarchive:	doxygenarchive

grafx2:
	$(OPT)$(MAKE) -C src/

ziprelease: grafx2
	$(OPT)$(MAKE) -C src/ ziprelease

tools:
	$(MAKE) -C tools/

3rdparty:
	cd 3rdparty/ && $(OPT)$(MAKE)

win32installer:
	$(MAKE) -C install/

doxygen:
	$(MAKE) -C tools/ doxygen

doxygenarchive:
	$(MAKE) -C tools/ doxygenarchive
