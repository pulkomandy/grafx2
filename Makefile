.PHONY:	all tools grafx2

all:	grafx2 tools

grafx2:
	$(MAKE) -C src/

tools:
	$(MAKE) -C tools/
