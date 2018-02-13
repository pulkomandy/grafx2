.PHONY:	all tools grafx2 ziprelease

all:	grafx2 tools

grafx2:
	$(MAKE) -C src/

ziprelease: grafx2
	$(MAKE) -C src/ ziprelease

tools:
	$(MAKE) -C tools/
