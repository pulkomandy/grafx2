AS=ophis
ASFLAGS=-l $*.lst -m $*.map

all:	picview.prg final.prg c64picview_inc.h

%.prg:	%.asm
	$(AS) $(ASFLAGS) -o $@ $<

final.prg:	picview.prg 30.bin 20.bin screen.bin 10.bin bitmap.bin 30.bin color.bin 42.bin 10.bin 00.bin
	cat $+ > $@

c64picview_inc.h:	picview.prg
	xxd -i $< | sed 's/^unsigned char/static unsigned char/'| sed '$$ d' > $@
