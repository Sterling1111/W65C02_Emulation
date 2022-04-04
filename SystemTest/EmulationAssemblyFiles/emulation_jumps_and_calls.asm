	.org $8000
	.org $8012

main:
	cld
	clc
	cli
	clv
	ldx #$ff
	txs

	jsr LABEL0
	jmp LABEL2

LABEL1:
	jmp (LABEL3)

LABEL5:
	jmp (LABEL6)

LABEL0:
	lda #$ff
	rts

LABEL2:
	lda #$ff
	jmp LABEL1

	.org $80b0
LABEL3:
	.word $8300

	.org $8300
	lda #$ff
	jmp LABEL5

	.org $85ff
LABEL6:
	.word $9000

	.org $9000
	ldx #$37
	jmp (LABEL7,X)

	.org $9030
LABEL7:

	.org $9067
	.word $9100

	.org $9100
	ldx #$ff
	jmp (LABEL8,X)

	.org $a000
LABEL8:

	.org $a0ff
	.word $a200

	.org $a200
	ldx #$ff
	stx $0100

	.org $fffc
	.word main
	.word $0000