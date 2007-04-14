testvesa.exe : testvesa.obj vesa.obj
	wlink @make.inc 

testvesa.obj : testvesa.c vesa.h
	wcc386 /we /4s /fp3 testvesa

vesa.obj : vesa.asm vesa.h
	tasm /mx /zn /os vesa
