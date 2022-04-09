	.org $8000

main:
	lda #$ea
	ldx #$ff

Loop0:			; fill in the zero page with $ea
	sta $00,X
	dex
	bne Loop0

Loop1:
	sta $0100,X
	dex
	bne Loop1

	jmp CONTINUE

CONTINUE:
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

	stp

	.org $fffc
	.word main
	.word $0000