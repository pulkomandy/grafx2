ifdef WIN32CROSS
OPT = WIN32CROSS=1 \

endif

.PHONY:	all tools grafx2 ziprelease 3rdparty

all:	grafx2 tools

grafx2:
	$(OPT)$(MAKE) -C src/

ziprelease: grafx2
	$(OPT)$(MAKE) -C src/ ziprelease

tools:
	$(MAKE) -C tools/

3rdparty:
	cd 3rdparty/ && $(OPT)$(MAKE)
