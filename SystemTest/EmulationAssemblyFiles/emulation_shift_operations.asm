	.org $8000
	.org $8014

main:
	cld
	clc
	cli
	clv
	ldx #$ff
	txs

	;Test shifts absolute
	lda #$ff
	sta $2000
	asl $2000
	php
	lda $2000

	lda #$00
	sta $2000
	lsr $2000
	php
	lda $2000

	lda #$00
	sta $2000
	rol $2000
	php
	lda $2000

	lda #$00
	sta $2000
	ror $2000
	php
	lda $2000

	;Test shifts accumulator
	lda #$ff
	asl
	php
	sta $2000

	lda #$00
	lsr
	php
	sta $2000

	lda #$7f
	rol
	php
	sta $2000

	lda #$80
	ror
	php
	sta $2000

	;Test shifts zero page
	lda $7f
	sta $0000
	asl $00
	php
	lda $00

	lda $80
	sta $0000
	lsr $00
	php
	lda $00

	lda $7f
	sta $0000
	rol $00
	php
	lda $00

	lda $00
	sta $0000
	ror $00
	php
	lda $00

	;Test shifts zero page X
	ldx #$00
	lda #$00
	sta $0000
	asl $00,X
	php
	lda $00

	ldx #$ff
	lda #$ff
	sta $ff
	asl $00,X
	php
	lda $ff

	ldx #$ff 	
	lda #$ff
	asl $fe
	inc $ff,X
	php
	lda $fe

	ldx #$00
	lda #$00
	sta $0000
	lsr $00,X
	php
	lda $00

	ldx #$ff
	lda #$ff
	sta $ff
	lsr $00,X
	php
	lda $ff

	ldx #$ff 
	lda #$ff
	lsr $fe
	dec $ff,X
	php
	lda $fe

	ldx #$00
	lda #$00
	sta $0000
	rol $00,X
	php
	lda $00

	ldx #$ff
	lda #$ff
	sta $ff
	rol $00,X
	php
	lda $ff

	ldx #$ff 
	lda #$ff
	rol $fe
	dec $ff,X
	php
	lda $fe

	ldx #$00
	lda #$00
	sta $0000
	ror $00,X
	php
	lda $00

	ldx #$ff
	lda #$ff
	sta $ff
	ror $00,X
	php
	lda $ff

	ldx #$ff 
	lda #$ff
	ror $fe
	dec $ff,X
	php
	lda $fe


	;Test shifts Absolute X
	lda #$ff
	sta $2042
	ldx #$42
	asl $2000,X
	php
	lda $2042

	lda #$ea
	sta $2100
	ldx #$ff
	asl $2001,X
	php
	lda $2100

	lda #$00
	sta $21fe
	ldx #$ff
	asl $20ff,X
	php
	lda $21fe

	lda #$00
	sta $2042
	ldx #$42
	lsr $2000,X
	php
	lda $2042

	lda #$ea
	sta $2100
	ldx #$ff
	lsr $2001,X
	php
	lda $2100

	lda #$10
	sta $21fe
	ldx #$ff
	lsr $20ff,X
	php
	lda $21fe

	lda #$00
	sta $2042
	ldx #$42
	rol $2000,X
	php
	lda $2042

	lda #$ea
	sta $2100
	ldx #$ff
	rol $2001,X
	php
	lda $2100

	lda #$10
	sta $21fe
	ldx #$ff
	rol $20ff,X
	php
	lda $21fe

	lda #$00
	sta $2042
	ldx #$42
	ror $2000,X
	php
	lda $2042

	lda #$ea
	sta $2100
	ldx #$ff
	ror $2001,X
	php
	lda $2100

	lda #$10
	sta $21fe
	ldx #$ff
	ror $20ff,X
	php
	lda $21fe

	stp

	.org $fffc
	.word main
	.word $0000	