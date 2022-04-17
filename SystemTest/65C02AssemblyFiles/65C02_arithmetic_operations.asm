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

	sta $0000
	sta $0100	

	cld
	clc
	cli
	clv
	ldx #$ff
	txs

	;Test adc immediate carry clear
	lda #$00
	clc
	adc #$42
	php
	sta $2000

	;Test adc immediate carry set
	sec
	adc #$42
	php
	sta $2000

	;Test sbc immediate carry clear
	clc
	sbc #$42
	php
	sta $2000

	;Test sbc immediate carry set
	sec
	sbc #$42
	php
	sta $2000

	;Test adc absolute carry clear
	clc
	adc $b000
	php
	sta $2000

	;Test adc absolute carry set
	sec
	adc $b000
	php
	sta $2000

	;Test sbc absolute carry clear
	clc
	sbc $b000
	php
	sta $2000

	;Test sbc absolute carry set
	sec
	sbc $b000
	php
	sta $2000

	;Test adc zero page carry clear
	clc
	adc $42
	php
	sta $2000

	;Test adc zero page carry set
	sec
	adc $42
	php
	sta $2000

	;Test sbc zero page carry clear
	clc
	sbc $42
	php
	sta $2000

	;Test sbc zero page carry set
	sec
	sbc $42
	php
	sta $2000

	ldy #$ff

	;Test adc Y indirect indexed carry clear
	clc
	adc ($42),Y
	php
	sta $2000

	;Test adc Y indirect indexed carry set
	sec
	adc ($42),Y
	php
	sta $2000

	;Test sbc Y indirect indexed carry clear
	clc
	sbc ($42),Y
	php
	sta $2000

	;Test sbc Y indirect indexed carry set
	sec
	sbc ($42),Y
	php
	sta $2000

	ldx #$42

	;Test adc X indirect indexed carry clear
	clc
	adc ($ff,X)
	php
	sta $2000

	;Test adc X indirect indexed carry set
	sec
	adc ($ff,X)
	php
	sta $2000

	;Test sbc X indirect indexed carry clear
	clc
	sbc ($ff,X)
	php
	sta $2000

	;Test sbc X indirect indexed carry set
	sec
	sbc ($ff,X)
	php
	sta $2000

	;Test adc zero page X indexed carry clear
	clc
	adc $42,X
	php
	sta $2000

	;Test adc zero page X indexed carry set
	sec
	adc $42,X
	php
	sta $2000

	;Test sbc zero page X indexed carry clear
	clc
	sbc $ff,X
	php
	sta $2000

	;Test sbc zero page X indexed carry set
	sec
	sbc $aa,X
	php
	sta $2000

	;Test adc zero page Y indexed carry clear
	clc
	adc $42,Y
	php
	sta $2000

	;Test adc zero page Y indexed carry set
	sec
	adc $42,Y
	php
	sta $2000

	;Test sbc zero page Y indexed carry clear
	clc
	sbc $ff,Y
	php
	sta $2000

	;Test sbc zero page Y indexed carry set
	sec
	sbc $aa,Y
	php
	sta $2000

	ldx #$42
	ldy #$42

	;Test adc Absolute X indexed carry clear
	clc
	adc $a000,X
	php
	sta $2000

	;Test adc Absolute X indexed carry set
	sec
	adc $a000,X
	php
	sta $2000

	;Test sbc Absolute X indexed carry clear
	clc
	sbc $a000,X
	php
	sta $2000

	;Test sbc Absolute X indexed carry set
	sec
	sbc $a000,X
	php
	sta $2000

	;Test adc Absolute Y indexed carry clear
	clc
	adc $a000,Y
	php
	sta $2000

	;Test adc Absolute Y indexed carry set
	sec
	adc $a000,Y
	php
	sta $2000

	;Test sbc Absolute Y indexed carry clear
	clc
	sbc $a000,Y
	php
	sta $2000

	;Test sbc Absolute Y indexed carry set
	sec
	sbc $a000,Y
	php
	sta $2000

	;Test adc zero page indirect carry clear
	clc
	adc ($00)
	php
	sta $2000

	;Test adc zero page indirect carry set
	sec
	adc ($00)
	php
	sta $2000

	;Test sbc zero page indirect carry clear
	clc
	sbc ($00)
	php
	sta $2000

	;Test sbc zero page indirect carry set
	sec
	sbc ($00)
	php
	sta $2000

	;Test cmp, cpx, cpy immediate
	lda #$00
	cmp #$00
	php
	sta $2000

	ldx #$ff
	cpx #$00
	php
	stx $2000

	ldy #$7f
	cpy #$42
	php
	sty $2000

	;Test cmp, cpx, cpy absolute
	lda #$00
	cmp $b000
	php
	sta $2000

	ldx #$00
	cpx $b000
	php
	stx $2000

	ldy #$00
	cpy $b000
	php
	sty $2000

	;Test cmp, cpx, cpy zero page
	cmp $42
	php
	sta $2000

	cpx $42
	php
	stx $2000

	cpy $42
	php
	sty $2000

	;Test cmp Y indirect indexed
	cmp ($42),Y
	php
	sta $2000

	;Test cmp X indirect indexed
	cmp ($42,X)
	php
	sta $2000

	;Test cmp zero page X indexed
	cmp $42,X
	php
	sta $2000

	;Test cmp zero page Y indexed
	cmp $42,Y
	php
	sta $2000

	;Test cmp absolute X indexed
	cmp $a000,X
	php
	sta $2000

	;Test cmp absolute Y indexed
	cmp $a000,Y
	php
	sta $2000

	;Test cmp zero page indirect 
	cmp ($00)
	php
	sta $2000

	stp

	.org $a000
	.word $0000

	.org $a042
	.word $0000

	.org $b000
	.byte $43

	.org $eaea
	.byte $ff

	.org $fffc
	.word main
	.word $0000	