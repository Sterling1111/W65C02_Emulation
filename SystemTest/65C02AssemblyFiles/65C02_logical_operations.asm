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

	;Testing Logical operations Immediate mode

	lda #$00
	and #$00	
	php			
	sta $2000

	lda #$0f
	and #$f0	
	php			
	sta $2000

	lda #$f0
	and #$0f	
	php			
	sta $2000

	lda #$ff
	and #$ff 	
	php			
	sta $2000

	lda #$00
	eor #$00	
	php			
	sta $2000

	lda #$0f
	eor #$f0	
	php			
	sta $2000

	lda #$f0
	eor #$0f	
	php			
	sta $2000

	lda #$ff
	eor #$ff 	
	php			
	sta $2000

	lda #$00
	ora #$00	
	php			
	sta $2000

	lda #$0f
	ora #$f0	
	php			
	sta $2000

	lda #$f0
	ora #$0f	
	php			
	sta $2000

	lda #$ff
	ora #$ff 	
	php			
	sta $2000

	lda #$00
	bit #$00	
	php			
	sta $2000

	lda #$0f
	bit #$f0	
	php			
	sta $2000

	lda #$f0
	bit #$0f	
	php			
	sta $2000

	lda #$ff
	bit #$ff 	
	php			
	sta $2000

	;Testing Logical operations Absolute mode

	lda #$00
	and $d000	
	php			
	sta $2000

	lda #$0f
	and $d001	
	php			
	sta $2000

	lda #$f0
	and $d002 	
	php			
	sta $2000

	lda #$ff
	and $d003 	
	php			
	sta $2000

	lda #$00
	eor $d000	
	php			
	sta $2000

	lda #$0f
	eor $d001	
	php			
	sta $2000

	lda #$f0
	eor $d002 	
	php			
	sta $2000

	lda #$ff
	eor $d003 	
	php			
	sta $2000

	lda #$00
	ora $d000	
	php			
	sta $2000

	lda #$0f
	ora $d001	
	php			
	sta $2000

	lda #$f0
	ora $d002 	
	php			
	sta $2000

	lda #$ff
	ora $d003 	
	php			
	sta $2000

	lda #$00
	bit $d000	
	php			
	sta $2000

	lda #$0f
	bit $d001	
	php			
	sta $2000

	lda #$f0
	bit $d002 	
	php			
	sta $2000

	lda #$ff
	bit $d003 	
	php			
	sta $2000

	;Testing Logical operations Zero Page mode

	ldx #$00
	stx $0000
	ldx #$f0
	stx $0001
	ldx #$0f
	stx $0002
	ldx #$ff
	stx $0003

	lda #$00
	and $00
	php
	sta $2000

	lda #$0f
	and $01
	php
	sta $2000

	lda #$f0
	and $02
	php
	sta $2000
	
	lda #$ff
	and $03
	php
	sta $2000	

	lda #$00
	eor $00
	php
	sta $2000

	lda #$0f
	eor $01
	php
	sta $2000

	lda #$f0
	eor $02
	php
	sta $2000
	
	lda #$ff
	eor $03
	php
	sta $2000	

	lda #$00
	ora $00
	php
	sta $2000

	lda #$0f
	ora $01
	php
	sta $2000

	lda #$f0
	ora $02
	php
	sta $2000
	
	lda #$ff
	ora $03
	php
	sta $2000	

	lda #$00
	bit $00
	php
	sta $2000

	lda #$0f
	bit $01
	php
	sta $2000

	lda #$f0
	bit $02
	php
	sta $2000
	
	lda #$ff
	bit $03
	php
	sta $2000	

	;Testing Logical operations Indirect Indexed Mode Y

	lda #$00
	ldy #$00
	ldx #$00
	stx $0000
	ldx #$d0
	stx $0001
	and ($00),Y
	php
	sta $2000

	lda #$0f
	ldy #$01
	ldx #$00
	stx $0000
	ldx #$d0
	stx $0001
	and ($00),Y
	php
	sta $2000

	lda #$f0
	ldy #$02
	ldx #$00
	stx $0000
	ldx #$d0
	stx $0001
	and ($00),Y
	php
	sta $2000

	lda #$ff
	ldy #$03
	ldx #$00
	stx $0000
	ldx #$d0
	stx $0001
	and ($00),Y
	php
	sta $2000

	lda #$00
	ldy #$00
	ldx #$00
	stx $0000
	ldx #$d0
	stx $0001
	eor ($00),Y
	php
	sta $2000

	lda #$0f
	ldy #$01
	ldx #$00
	stx $0000
	ldx #$d0
	stx $0001
	eor ($00),Y
	php
	sta $2000

	lda #$f0
	ldy #$02
	ldx #$00
	stx $0000
	ldx #$d0
	stx $0001
	eor ($00),Y
	php
	sta $2000

	lda #$ff
	ldy #$03
	ldx #$00
	stx $0000
	ldx #$d0
	stx $0001
	eor ($00),Y
	php
	sta $2000

	lda #$00
	ldy #$00
	ldx #$00
	stx $0000
	ldx #$d0
	stx $0001
	ora ($00),Y
	php
	sta $2000

	lda #$0f
	ldy #$01
	ldx #$00
	stx $0000
	ldx #$d0
	stx $0001
	ora ($00),Y
	php
	sta $2000

	lda #$f0
	ldy #$02
	ldx #$00
	stx $0000
	ldx #$d0
	stx $0001
	ora ($00),Y
	php
	sta $2000

	lda #$ff
	ldy #$03
	ldx #$00
	stx $0000
	ldx #$d0
	stx $0001
	ora ($00),Y
	php
	sta $2000

	;Testing Logical operations Indirect Indexed Mode Y

	lda #$00
	ldx #$00
	ldy #$00
	sty $00
	ldy #$d0
	sty $01
	and ($00,X) 	
	php
	sta $2000

	lda #$0f
	ldx #$00
	ldy #$01
	sty $00
	ldy #$d0
	sty $01
	and ($00,X) 	
	php
	sta $2000

	lda #$f0
	ldx #$00
	ldy #$02
	sty $00
	ldy #$d0
	sty $01
	and ($00,X) 	
	php
	sta $2000

	lda #$ff
	ldx #$00
	ldy #$03
	sty $00
	ldy #$d0
	sty $01
	and ($00,X) 	
	php
	sta $2000

	lda #$00
	ldx #$00
	ldy #$00
	sty $00
	ldy #$d0
	sty $01
	eor ($00,X) 	
	php
	sta $2000

	lda #$0f
	ldx #$00
	ldy #$01
	sty $00
	ldy #$d0
	sty $01
	eor ($00,X) 	
	php
	sta $2000

	lda #$f0
	ldx #$00
	ldy #$02
	sty $00
	ldy #$d0
	sty $01
	eor ($00,X) 	
	php
	sta $2000

	lda #$ff
	ldx #$00
	ldy #$03
	sty $00
	ldy #$d0
	sty $01
	eor ($00,X) 	
	php
	sta $2000

	lda #$00
	ldx #$00
	ldy #$00
	sty $00
	ldy #$d0
	sty $01
	ora ($00,X) 	
	php
	sta $2000

	lda #$0f
	ldx #$00
	ldy #$01
	sty $00
	ldy #$d0
	sty $01
	ora ($00,X) 	
	php
	sta $2000

	lda #$f0
	ldx #$00
	ldy #$02
	sty $00
	ldy #$d0
	sty $01
	ora ($00,X) 	
	php
	sta $2000

	lda #$ff
	ldx #$00
	ldy #$03
	sty $00
	ldy #$d0
	sty $01
	ora ($00,X) 	
	php
	sta $2000

	;Test Logical Operations Zero Page X

	ldx #$00
	stx $0000
	ldx #$f0
	stx $0001
	ldx #$0f
	stx $0002
	ldx #$ff
	stx $0003

	lda #$00
	ldx #$00
	and $0,X		
	php
	sta $2000

	lda #$0f
	ldx #$00
	and $1,X		
	php
	sta $2000

	lda #$f0
	ldx #$00
	and $2,X		
	php
	sta $2000

	lda #$ff
	ldx #$00
	and $3,X		
	php
	sta $2000

	lda #$00
	ldx #$00
	eor $0,X		
	php
	sta $2000

	lda #$0f
	ldx #$00
	eor $1,X		
	php
	sta $2000

	lda #$f0
	ldx #$00
	eor $2,X		
	php
	sta $2000

	lda #$ff
	ldx #$00
	eor $3,X		
	php
	sta $2000

	lda #$00
	ldx #$00
	ora $0,X		
	php
	sta $2000

	lda #$0f
	ldx #$00
	ora $1,X		
	php
	sta $2000

	lda #$f0
	ldx #$00
	ora $2,X		
	php
	sta $2000

	lda #$ff
	ldx #$00
	ora $3,X		
	php
	sta $2000

	lda #$00
	ldx #$00
	bit $0,X		
	php
	sta $2000

	lda #$0f
	ldx #$00
	bit $1,X		
	php
	sta $2000

	lda #$f0
	ldx #$00
	bit $2,X		
	php
	sta $2000

	lda #$ff
	ldx #$00
	bit $3,X		
	php
	sta $2000

	;Test Logical Operations Absolute X

	lda #$00
	ldx #$00
	and $d000,X
	php
	sta $2000

	lda #$00f
	ldx #$00
	and $d001,X
	php
	sta $2000

	lda #$f0
	ldx #$00
	and $d002,X
	php
	sta $2000

	lda #$ff
	ldx #$00
	and $d003,X
	php
	sta $2000

	lda #$00
	ldx #$00
	eor $d000,X
	php
	sta $2000

	lda #$0f
	ldx #$00
	eor $d001,X
	php
	sta $2000

	lda #$f0
	ldx #$00
	eor $d002,X
	php
	sta $2000

	lda #$ff
	ldx #$00
	eor $d003,X
	php
	sta $2000

	lda #$00
	ldx #$00
	ora $d000,X
	php
	sta $2000

	lda #$0f
	ldx #$00
	ora $d001,X
	php
	sta $2000

	lda #$f0
	ldx #$00
	ora $d002,X
	php
	sta $2000

	lda #$ff
	ldx #$00
	ora $d003,X
	php
	sta $2000

	lda #$00
	ldx #$00
	bit $d000,X
	php
	sta $2000

	lda #$0f
	ldx #$00
	bit $d001,X
	php
	sta $2000

	lda #$f0
	ldx #$00
	bit $d002,X
	php
	sta $2000

	lda #$ff
	ldx #$00
	bit $d003,X
	php
	sta $2000

	lda #$00
	ldy #$00
	and $d000,Y
	php
	sta $2000

	lda #$0f
	ldy #$00
	and $d001,Y
	php
	sta $2000

	lda #$f0
	ldy #$00
	and $d002,Y
	php
	sta $2000

	lda #$ff
	ldy #$00
	and $d003,Y
	php
	sta $2000	

	lda #$00
	ldy #$00
	eor $d000,Y
	php
	sta $2000

	lda #$0f
	ldy #$00
	eor $d001,Y
	php
	sta $2000

	lda #$f0
	ldy #$00
	eor $d002,Y
	php
	sta $2000

	lda #$ff
	ldy #$00
	eor $d003,Y
	php
	sta $2000

	;Test Logical Operation Zero Page Indirect

	lda #$00
	sta $0042
	lda #$d0
	sta $0043	

	and ($42)
	php
	sta $2000

	eor ($42)
	php
	sta $2000

	ora ($42)
	php
	sta $2000

	stp

	.org $d000
	.byte $00
	.byte $f0
	.byte $0f
	.byte $ff

	.org $fffc
	.word main
	.word $0000	