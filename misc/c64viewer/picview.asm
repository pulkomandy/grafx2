; vim:ts=8 sw=8:
;  Grafx2 - The Ultimate 256-color bitmap paint program
;
; Copyright owned by various GrafX2 authors, see COPYRIGHT.txt for details.
;
; This code is embedded in C64 images saved by GrafX2, it can be distributed
; and modified without any restrictions.
;
; Designed to be built with The Ophis Assembler :
; https://michaelcmartin.github.io/Ophis/
; C64 picture unpacker and viewer.
; - Supports all VIC-II modes, character or bitmap based.
; - Supports raw data, RLE packing and specific color RAM RLE packing
;
; Memory map :
; $0002       Low byte of count
; $00FC-$00FD Data pointer
; $0801-$080C Basic program : trampoline code
; $080D-$xxxx 6502 machine language code
; $xxxx-$xxxx (packed) picture data
; $C000-$C3E8 Screen RAM               \
; $C3F0-$C3FF VIC-II $D020-$D02E backup )- Upper RAM area
; $C400-$C7FF Color RAM backup         /
; $D000-$D02E VIC-II registers (writes from $D020 to $D02E)
; $D800-$DBFF Color RAM
; $E000-$FF40 Bitmap (behind KERNAL ROM)
.word $0801
.org  $0801
	.word next, 1911	; next basic line and line number
	.byte $9e,"2061",0	; SYS2061
next:	.word 0			; end of basic program

	; $fb $fc $fd $fe => unused
start:
	lda #<data
	sta $fb
	lda #>data
	sta $fc

	; backup border/background/extra/sprite colors
	ldx #$10
colorbak2:
	lda $D01F,x
	sta $C3EF,x
	dex
	bne colorbak2
	; backup COLOR RAM in high RAM
colorbak:
	lda $D800,x
	sta $C400,x
	lda $D900,x
	sta $C500,x
	lda $DA00,x
	sta $C600,x
	lda $DB00,x
	sta $C700,x
	inx
	bne colorbak

	;lda $01	; processor port
	;pha	; backup
	;and #$F8
	;ora #$06	; xxxxx110 : RAM $A000-$BFFF, IO $D000-$DFFF, ROM $E000-$FFFF
	;sta $01

	jsr readbyte	; read mode byte
	pha		; push it
mainloop:
	jsr readbyte
	pha
	and #$0F
	tax
	lda algoslo,x
	sta callalgo+1
	;lda algoshi,x	; assume all algos are in the same page
	;sta callalgo+2
	pla
	and #$F0
	beq display	; 0 => end of file
	lsr
	lsr
	tax
	lda addr_sizes-4,x	; destination
	sta $fd
	lda addr_sizes-3,x
	sta $fe
	lda addr_sizes-2,x	; size
	sta $02
	lda addr_sizes-1,x
	tax
callalgo:
	jsr longmemcpy	; modified
	jmp mainloop

display:
	pla
	; mode byte 0EBM0000 E=Extended Background mode. B=Bitmap Mode. M=Multicolor
	; $30 => Multicolor Bitmap
	tax
	ora #$1B
	sta $d011	; screen control register #1
	txa
	and #$10
	ora #$C8	; multicolor, 40 columns
	sta $d016	; screen control register #2

	lda $dd00
	pha	; backup
	and #$fc	; 0 => bank #3 $C000-$FFFF
	;ora #$01	; 1 => bank #2 $8000-$BFFF  char ROM at $9000-$9FFF
	;ora #$02	; 2 => bank #1 $4000-$7FFF
	;ora #$03	; 3 => bank #0 $0000-$3FFF  char ROM at $1000-$1FFF
	sta $dd00	; set VIC BANK
	lda $d018
	pha	; backup
	lda #$08	; bitmap +$2000 / screen +$0000
	sta $d018	; bitmap: $E000 / screen: $C000

wait:
	jsr $ffe4	; GETIN
	beq wait

	lda #$1B
	sta $d011
	lda #$C8
	sta $d016
	pla
	sta $d018
	pla
	sta $dd00	; restore VIC BANK
	;pla
	;sta $01	; restore Processor port
	; Restore border/background/extra/sprite colors
	ldx #$10
colorrestore2:
	lda $C3EF,x
	sta $D01F,x
	dex
	bne colorrestore2
	; restore COLOR RAM
colorrestore:
	lda $C400,x
	sta $D800,x
	lda $C500,x
	sta $D900,x
	lda $C600,x
	sta $DA00,x
	lda $C700,x
	sta $DB00,x
	inx
	bne colorrestore

	rts	; return to basic
; ====================================================================

	; special color RAM compression (only low nibble matters)
	; high nibble is count
colordepack:
	;dex
colordepacklp:
	jsr readbyte
colordepacklp2:
	jsr storebyte
	dec $02
	bne +
	dex
	bpl +
	rts
*	clc
	adc #$10
	bcs colordepacklp
	bcc colordepacklp2

	; depack packbits RLE stream
rledepack:
	jsr readbyte
	tax
	bmi repeat
        ; temp_byte <= 127 => copy (temp_byte + 1) bytes
copy:	inx
	jsr shrtlp
	beq rledepack

repeat: ; temp_byte > 127  => repeat (257-temp_byte) the next byte
	dex
	;bmi +	; 128 => out
	;rts
	bpl return
*	jsr readbyte2
repeatlp:
	jsr storebyte
	inx
	bne repeatlp
	beq rledepack


	; src=$fb/$fc dst=$fd/$fe
	; count= x(high) $02(low)
longmemcpy:
*	lda ($fb),y	; copy 256 bytes
	sta ($fd),y
	iny
	bne -
	inc $fc
	inc $fe
	dex
	bne -

	ldx $02
	beq return
shrtlp:	lda ($fb),y
	sta ($fd),y
	inc $fb
	bne +
	inc $fc
*	inc $fd
	bne +
	inc $fe
*	dex
	bne shrtlp
return:
	rts

	; src=$fb/$fc advance pointer
	; set y=0
readbyte:
	ldy #0
readbyte2:
	lda ($fb),y
	inc $fb
	bne +
	inc $fc
*	rts

	; dst=$fd/$fe advance pointer
	; assumes y=0
storebyte:
	sta ($fd),y
	inc $fd
	bne +
	inc $fe
*	rts



; 0, bitmap, screen, color, color regs, xxx, xxx, xxx
addr_sizes:
	.word $E000, 8000	; 1 : bitmap behind KERNAL ROM
	.word $C000, 1000	; 2 : screen ram in upper RAM area
	.word $D800, 1000	; 3 : color ram
	.word $D020,   15	; 4 : border/background/extra/sprite colors
algoslo:
	.byte <longmemcpy
	.byte <rledepack
	.byte <colordepack
;algoshi:
;	.byte >longmemcpy
;	.byte >rledepack
;	.byte >colordepack
data:
