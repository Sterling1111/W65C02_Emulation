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

	lda #$ea
	sta $2481
	sta $2501
	sta $2000

	lda #$42		;TestLoadARegisterImmediate
	php
	sta $2000
	ldx #$42 		;TestLoadXRegisterImmediate
	php
	stx $2000
	ldy #$42 		;RestLoadYRegisterImmediate
	php
	sty $2000

	lda $9000		;TestLoadARegisterAbsolute
	php
	sta $2000
	ldx $9000		;TestLoadXRegisterAbsolute
	php
	sta $2000
	ldy $9000		;TestLoadYRegisterAbsolute
	php
	sta $2000

	lda $42			;TestLoadARegisterZeroPage
	php
	sta $2000
	ldx $42			;TestLoadXRegisterZeroPage
	php
	stx $2000
	ldY $42			;TestLoadYRegisterZeroPage
	php
	sty $2000

	lda #$00 	
	sta $00
	lda #$90
	sta $01
	ldy #$04
	lda ($00),Y		;TestLoadARegisterIndirectIndexed
	php
	sta $2000

	lda #$01
	sta $00
	lda #$90
	sta $01
	ldy #$ff
	lda ($00),Y 	;TestLoadARegisterIndirectIndexedWhenPageBounderyCrossed
	php
	sta $2000

	lda #$90
	sta $00
	lda #$01
	sta $ff
	ldy #$ff
	lda ($ff),Y 	;TestLoadARegisterIndirectIndexedWhenPageBounderyCrossed
	php
	sta $2000

	lda #$ff
	sta $00
	lda #$ff
	sta $ff
	ldy #$ff
	lda ($ff),Y 	;TestLoadARegisterIndirectIndexedWhenPageBounderyCrossed
	php
	sta $2000

	ldx #$04
	lda #$00
	sta $00
	lda #$90
	sta $04
	lda ($00,X) 	;TestLoadARegisterIndexedIndirect
	php
	sta $2000

	ldx #$ff
	lda #$01
	sta $00
	lda #$ff
	sta $ff
	lda ($00,X)		;TestLoadARegisterIndexedIndirectWhenItRaps
	php
	sta $2000

	ldx #$05
	lda $42,X		;TestLoadARegisterZeroPageX
	php
	sta $2000

	ldx #$ff
	lda $80,X 		;TestLoadARegisterZeroPageXWhenItRaps
	php
	sta $2000

	ldx #$05
	ldy $42,X		;TestLoadYRegisterZeroPageX
	php
	sty $2000

	ldx #$ff
	ldy $80,X 		;TestLoadYRegisterZeroPageXWhenItRaps
	php
	sty $2000

	ldy #$05
	lda $42,Y 		;TestLoadARegisterAbsoluteY
	php
	sta $2000

	ldy #$ff
	lda $80,Y 		;TestLoadARegisterAbsoluteYWhenItRaps
	php
	sta $2000

	ldy #$05
	ldx $42,Y 		;TestLoadXRegisterZeroPageY
	php
	stx $2000

	ldy #$ff
	ldx $80,Y 		;TestLoadXRegisterZeroPageYWhenItRaps
	php
	stx $2000

	ldx #$01
	lda $2480,X 	;TestLoadARegisterAbsoluteX
	php
	sta $2000

	ldx #$ff
	lda $2402,X 	;TestLoadARegisterAbsoluteXWhenPageBounderyCrossed
	php
	sta $2000

	ldx #$01
	ldy $2480,X 	;TestLoadYRegisterAbsoluteX
	php
	sty $2000

	ldx #$ff
	ldy $2402,X 	;TestLoadYRegisterAbsoluteXWhenPageBounderyCrossed
	php
	sty $2000

	ldy #$01
	lda $2480,Y 	;TestLoadARegisterAbsoluteY
	php
	sta $2000

	ldy #$ff
	lda $2402,Y 	;TestLoadARegisterAbsoluteXWhenPageBounderyCrossed
	php
	sta $2000

	ldy #$01
	ldx $2480,Y 	;TestLoadXRegisterAbsoluteY
	php
	stx $2000

	ldy #$ff
	ldx $2402,Y 	;TestLoadXRegisterAbsoluteXWhenPageBounderyCrossed
	php
	stx $2000

	lda ($42)		;TestLDAZeroPageIndirect
	php
	sta $2000

	lda ($ff)		;TestLDAZeroPageIndirectWhenItRaps
	php
	sta $2000

	stp

	.org $9000
	.byte $ea
	.org $9004
	.byte $ea
	.org $9100
	.byte $ea
	.org $ea90
	.byte $00ea

	.org $fffc
	.word main
	.word $0000