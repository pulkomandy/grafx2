; vim:ts=8 sw=8:
.word $0801
.org  $0801
	.word next, 1911	; next basic line and line number
	.byte $9e,"2061",0	; SYS2061 
next:	.word 0			; end of basic program
start:
	jsr $e544
	lda #$00
	sta $d020	; border
	sta $d021	; background
	ldx #$00
loop:
	lda text,x
	sec
	sbc #$40
	sta $0400+40*12,x
	inx
	cpx #40
	bne loop
wait:
	;jmp wait
	;jsr $ffcf
	jsr $ffe4	; GETIN
	beq wait
	rts

text:
.byte "              HELLO WORLD               "
