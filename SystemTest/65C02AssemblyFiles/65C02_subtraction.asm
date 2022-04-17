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

	ldy #$00

AddLoop1:
	lda #$00
	clc
	sbc $00,X
	php
	sta $2000
	inx
	bne AddLoop1

AddLoop2:
	lda #$1f
	clc
	sbc $00,X
	php
	sta $2000
	inx
	bne AddLoop2

AddLoop3:
	lda #$3e
	clc
	sbc $00,X
	php
	sta $2000
	inx
	bne AddLoop3

	ldy #$5d

AddLoop4:
	lda #$5d
	clc
	sbc $00,X
	php
	sta $2000
	inx
	bne AddLoop4

AddLoop5:
	lda #$7c
	clc
	sbc $00,X
	php
	sta $2000
	inx
	bne AddLoop5

AddLoop6:
	lda #$9b
	clc
	sbc $00,X
	php
	sta $2000
	inx
	bne AddLoop6

AddLoop7:
	lda #$ba
	clc
	sbc $00,X
	php
	sta $2000
	inx
	bne AddLoop7

AddLoop8:
	lda #$d9
	clc
	sbc $00,X
	php
	sta $2000
	inx
	bne AddLoop8

AddLoop9:
	lda #$ff
	clc
	sbc $00,X
	php
	sta $2000
	inx
	bne AddLoop9

AddLoopCarry1:
	lda #$00
	sec
	sbc $00,X
	php
	sta $2000
	inx
	bne AddLoopCarry1

AddLoopCarry2:
	lda #$1f
	sec
	sbc $00,X
	php
	sta $2000
	inx
	bne AddLoopCarry2

AddLoopCarry3:
	lda #$3e
	sec
	sbc $00,X
	php
	sta $2000
	inx
	bne AddLoopCarry3

AddLoopCarry4:
	lda #$5d
	sec
	sbc $00,X
	php
	sta $2000
	inx
	bne AddLoopCarry4

AddLoopCarry5:
	lda #$7c
	sec
	sbc $00,X
	php
	sta $2000
	inx
	bne AddLoopCarry5

AddLoopCarry6:
	lda #$9b
	sec
	sbc $00,X
	php
	sta $2000
	inx
	bne AddLoopCarry6

AddLoopCarry7:
	lda #$ba
	sec
	sbc $00,X
	php
	sta $2000
	inx
	bne AddLoopCarry7

AddLoopCarry8:
	lda #$d9
	sec
	sbc $00,X
	php
	sta $2000
	inx
	bne AddLoopCarry8

AddLoopCarry9:
	lda #$ff
	sec
	sbc $00,X
	php
	sta $2000
	inx
	bne AddLoopCarry9

	stp

	.org $fffc
	.word main
	.word $0000	