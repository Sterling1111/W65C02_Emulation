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

	lda #$20
	sta $05

	lda #$ea
	sta $2481

	lda #$ff
	php
	ldx #$ff
	php
	ldy #$ff
	php

	sta $2000		;TestStoreARegisterAbsolute
	php
	stx $2000		;TestStoreXRegisterAbsolute
	php
	sty $2000		;TestStoreYRegisterAbsolute
	php
	stz $2000		;TestStoreZeroAbsolute
	php

	sta $20 		;TestStoreARegisterZeroPage
	php
	stx $20 		;TestStoreXRegisterZeroPage
	php
	sty $20 		;TestStoreYRegisterZeroPage
	php
	stz $20 		;TestStoreZeroZeroPage
	php

	lda #$00
	sta $00
	lda #$20
	sta $01
	ldy #$04
	lda #$ff
	sta ($00),Y		;TestStoreARegisterIndirectIndexed
	php

	lda #$01
	sta $00
	lda #$20
	sta $01
	ldy #$ff
	lda #$ff
	sta ($00),Y		;TestStoreARegisterIndirectIndexedWhenPageBounderyCrossed
	php

	ldx #$04
	lda #$00
	sta $00
	lda #$20
	sta $04
	sta ($00,X) 	;TestStoreARegisterIndexedIndirect
	php

	ldx #$ff
	lda #$01
	sta $00
	lda #$20
	sta $ff
	sta ($00,X)		;TestStoreARegisterIndexedIndirectWhenItRaps
	php

	ldx #$05
	sta $42,X		;TestStoreARegisterZeroPageX
	php

	ldx #$ff
	sta $80,X 		;TestStoreARegisterZeroPageXWhenItRaps
	php

	ldx #$05
	sty $42,X		;TestStoreYRegisterZeroPageX
	php

	ldx #$ff
	sty $80,X 		;TestStoreYRegisterZeroPageXWhenItRaps
	php

	ldx #$05
	stz $42,X		;TestStoreZeroZeroPageX
	php

	ldx #$ff
	stz $80,X 		;TestStoreZeroZeroPageXWhenItRaps
	php

	ldy #$05
	stx $42,Y 		;TestStoreXRegisterZeroPageY
	php

	ldy #$ff
	stx $80,Y 		;TestStoreXRegisterZeroPageYWhenItRaps
	php


	ldx #$01
	sta $2480,X 	;TestStoreARegisterAbsoluteX
	php

	ldx #$ff
	sta $2402,X 	;TestStoreARegisterAbsoluteXWhenPageBounderyCrossed
	php

	ldx #$01
	stz $2480,X 	;TestStoreZeroAbsoluteX
	php

	ldx #$ff
	stz $2402,X 	;TestStoreZeroAbsoluteXWhenPageBounderyCrossed
	php

	ldy #$01
	sta $2480,Y 	;TestStoreARegisterAbsoluteY
	php

	ldy #$ff
	sta $2402,Y 	;TestStoreARegisterAbsoluteYWhenPageBounderyCrossed
	php

	lda #$00
	sta $00
	lda #$10
	sta $01
	lda #$ff
	sta ($00)
	php

	lda #$00
	sta $ff
	lda #$10
	sta $00
	lda #$ff
	sta ($ff)
	php

	sta $20
	stp

	.org $fffc
	.word main
	.word $0000	