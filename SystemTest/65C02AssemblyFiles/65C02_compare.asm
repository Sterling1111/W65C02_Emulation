	.org $8000

main:
	cld
	clc
	cli
	clv

	ldx #$ff
	txs


Loop0:			; fill in the zero page with 0 to 255
	sta $00,X
	inx
	inc 
	bne Loop0


CmpLoop1:
	lda #$00
	cmp $00,X
	php
	sta $2000
	inx
	bne CmpLoop1

CmpLoop2:
	lda #$1f
	cmp $00,X
	php
	sta $2000
	inx
	bne CmpLoop2

CmpLoop3:
	lda #$3e
	cmp $00,X
	php
	sta $2000
	inx
	bne CmpLoop3

	ldy #$5d

CmpLoop4:
	lda #$5d
	cmp $00,X
	php
	sta $2000
	inx
	bne CmpLoop4

CmpLoop5:
	lda #$7c
	cmp $00,X
	php
	sta $2000
	inx
	bne CmpLoop5

CmpLoop6:
	lda #$9b
	cmp $00,X
	php
	sta $2000
	inx
	bne CmpLoop6

CmpLoop7:
	lda #$ba
	cmp $00,X
	php
	sta $2000
	inx
	bne CmpLoop7

CmpLoop8:
	lda #$d9
	cmp $00,X
	php
	sta $2000
	inx
	bne CmpLoop8

CmpLoop9:
	lda #$ff
	cmp $00,X
	php
	sta $2000
	inx
	bne CmpLoop9

	stp

	.org $fffc
	.word main
	.word $0000	