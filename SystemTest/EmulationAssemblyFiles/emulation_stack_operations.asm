	.org $8000
	.org $8012

main:
	cld
	clc
	cli
	clv
	ldx #$ff
	txs

	lda #$ff
	pha
	lda #$00
	php
	plp
	pla
	php
	plp

	ldx #$ff
	phx
	ldx #$00
	php
	plp
	plx
	php
	plp

	ldy #$ff
	phy
	ldy #$00
	php
	plp
	ply
	php
	plp

	ldx #$ff
	txs
	ldx #$00
	tsx
	php
	plp

	ldx #$00
	txs
	ldx #$ff
	tsx
	php
	plp

	sta $20

	.org $fffc
	.word main
	.word $0000