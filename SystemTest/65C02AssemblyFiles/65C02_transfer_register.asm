	.org $8000

main:
	cld
	clc
	cli
	clv

	ldx #$ff
	txs

	lda #$00
	tax
	php
	stx $2000

	lda #$7f
	tax
	php
	stx $2000

	lda #$ff
	tax
	php
	stx $2000

	lda #$00
	tay
	php
	sty $2000

	lda #$00
	tay
	php
	stx $2000

	lda #$7f
	tay
	php
	stx $2000

	lda #$ff
	tay
	php
	sty $2000

	lda #$00
	tay
	php
	sty $2000

	ldx #$00
	txa
	php
	sta $2000

	ldx #$7f
	txa
	php
	sta $2000

	ldx #$ff
	txa
	php
	sta $2000

	ldy #$00
	tya
	php
	sta $2000

	ldy #$7f
	tya
	php
	sta $2000

	ldy #$ff
	tya
	php
	sta $2000

	stp

	.org $fffc
	.word main
	.word $0000	