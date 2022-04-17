	.org $8000

main:
	cld
	clc
	cli
	clv

	ldx #$ff
	txs

	ldy #$00
	lda #$00
	ldx #$00

Loop0:			; fill in the zero page with 0 to 255
	sta $00,X
	inx
	inc 
	bne Loop0

ASLLOOP:
	sta $00,X
	asl
	php
	sta $2000
	inx
	bne ASLLOOP

LSRLOOP:
	sta $00,X
	lsr
	php
	sta $2000
	inx
	bne LSRLOOP

ROLLOOP:
	sta $00,X
	rol
	php
	sta $2000
	inx
	bne ROLLOOP	

RORLOOP:
	sta $00,X
	ror
	php
	sta $2000
	inx
	bne RORLOOP
	
	stp

	.org $fffc
	.word main
	.word $0000	