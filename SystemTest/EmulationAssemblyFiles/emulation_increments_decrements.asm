	.org $8000
	.org $8014

main:
	cld
	clc
	cli
	clv
	ldx #$ff
	txs

	lda #$00

INCALOOP:
	inc
	php
	sta $2000
	bne INCALOOP

	lda #$ff

DECALOOP:
	dec
	php
	sta $2000
	bne DECALOOP

	ldx #$00	

INCXLOOP:
	inx
	php
	stx $2000
	bne INCXLOOP

	ldx #$ff

DECXLOOP:
	dex
	php
	stx $2000
	bne DECXLOOP

	ldy #$00

INCYLOOP:
	iny
	php
	sty $2000
	bne INCYLOOP

	ldy #$ff

DECYLOOP:
	dey
	php
	sty $2000
	bne DECYLOOP

	;Test inc absolute
	lda #$ff
	sta $2000
	inc $2000
	php
	lda $2000

	;Test dec absolute
	lda #$00
	sta $2000
	dec $2000
	php
	lda $2000

	;Test inc zero page
	lda $7f
	sta $0000
	inc $00
	php
	lda $00

	;Test dec zero page
	lda $80
	sta $0000
	dec $00
	php
	lda $00

	;Test inc zero page X
	ldx #$00
	lda #$00
	sta $0000
	inc $00,X
	php
	lda $00

	ldx #$ff
	lda #$ff
	sta $ff
	inc $00,X
	php
	lda $ff

	ldx #$ff 	
	lda #$ff
	sta $fe
	inc $ff,X
	php
	lda $fe

	;Test dec zero page X
	ldx #$00
	lda #$00
	sta $0000
	dec $00,X
	php
	lda $00

	ldx #$ff
	lda #$ff
	sta $ff
	dec $00,X
	php
	lda $ff

	ldx #$ff 
	lda #$ff
	sta $fe
	dec $ff,X
	php
	lda $fe

	;Test inc absolute X
	lda #$ff
	sta $2042
	ldx #$42
	inc $2000,X
	php
	lda $2042

	lda #$ea
	sta $2100
	ldx #$ff
	inc $2001,X
	php
	lda $2100

	lda #$00
	sta $21fe
	ldx #$ff
	inc $20ff,X
	php
	lda $21fe

	;Test dec absolute X
	lda #$00
	sta $2042
	ldx #$42
	dec $2000,X
	php
	lda $2042

	lda #$ea
	sta $2100
	ldx #$ff
	dec $2001,X
	php
	lda $2100

	lda #$10
	sta $21fe
	ldx #$ff
	dec $20ff,X
	php
	lda $21fe

	stp

	.org $fffc
	.word main
	.word $0000	