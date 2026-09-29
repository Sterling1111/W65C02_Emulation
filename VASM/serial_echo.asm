; W65C51N serial echo. Set CLOCK to 1 MHz, Build & Run, open Terminal.
; The N variant's transmit-empty flag is always set; use a software delay.
ACIA_DATA = $5000
ACIA_STATUS = $5001
ACIA_COMMAND = $5002
ACIA_CONTROL = $5003

    .org $8000
reset:
    sei
    cld
    ldx #$ff
    txs
    lda #$1f                 ; 19200 baud, 8-N-1, internal receive clock
    sta ACIA_CONTROL
    lda #$0b                 ; enable receiver, RTS low, poll (no IRQ)
    sta ACIA_COMMAND
    lda #$3e                 ; prompt: >
    jsr transmit
receive:
    lda ACIA_STATUS
    and #$08
    beq receive
    lda ACIA_DATA
    jsr transmit
    bra receive
transmit:
    sta ACIA_DATA
    pha
    lda #$ff
wait_tx:
    dec
    bne wait_tx
    pla
    rts
nmi:
irq:
    rti
    .org $fffa
    .word nmi,reset,irq
