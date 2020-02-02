# There is no uname under windows, but we can guess we are there with the COMSPEC env.var
# Windows specific
ifdef ComSpec
# sometimes, it is ComSpec, sometimes it is COMSPEC
COMSPEC = $(ComSpec)
endif
ifdef COMSPEC
WIN32 = 1
endif

PKG_CONFIG = $(shell which pkg-config)
MKDIR = mkdir -p

ifeq ($(WIN32),1)
PKG_CONFIG := PKG_CONFIG_PATH=../../3rdparty/usr-win32/lib/pkgconfig $(PKG_CONFIG)
endif

CFLAGS = -Wall
CFLAGS += $(shell $(PKG_CONFIG) --cflags libpng)
LDFLAGS += $(shell $(PKG_CONFIG) --libs-only-L libpng)
LDLIBS += $(shell $(PKG_CONFIG) --libs-only-l libpng)

FONTDIR = ../../share/grafx2/skins/
FONTFILES = unicode_0390-03C9.png unicode_3040-309F.png
FONTPATHS = $(addprefix $(FONTDIR), $(FONTFILES))

BINDIR = ../../bin
BIN = $(BINDIR)/generate_png_fonts

.PHONY:	all clean

all:	$(FONTPATHS)

clean:
	$(RM) *.o $(BIN)
	$(RM) $(FONTPATHS)

$(BIN):	generate_png_fonts.o
	$(MKDIR) $(BINDIR)
	$(CC) $(LDFLAGS) -o $@ $^ $(LOADLIBES) $(LDLIBS)

$(FONTPATHS):	.fonts.generated

.fonts.generated:	$(BIN)
	$(BIN) $(FONTDIR)
	touch $@
