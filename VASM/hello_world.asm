PORTB = $6000
PORTA = $6001
DDRB = $6002
DDRA = $6003

E   = %10000000
RW  = %01000000
RS  = %00100000

    .org $8000

;this is the start of the program
reset:
    ldx #$ff    
    txs             ;transfer the contents of x register($ff) to stack register

    lda #%11111111  ;Set all pins on port B to output
    sta DDRB

    lda #%11100000  ;Set all top 3 pins on port A to output
    sta DDRA

    lda #0          ;pre clear RS/RW/E bits
    sta PORTA

    lda #%00000001  ;clear the entire display
    jsr lcd_instruction

    lda #%00111000  ;Set 8-bit mode, 2-line display, 5x8 font
    jsr lcd_instruction

    lda #%00001111  ;display on, cursor on, blink on
    jsr lcd_instruction

    lda #%00000110  ;increment and shift cursor, dont shift entire display
    jsr lcd_instruction

    ldx #0
    
print:
    lda message, x  ;lda from address of message + x
    beq STOP        ;when a gets the 0 at the end of message it will terminate
    jsr print_char
    inx
    jmp print

STOP:
    ldx #69
    ldy #69
    stp

message: .asciiz "Hello, World!" ;this will put a 0 at the end of the string

lcd_wait:
    pha             ;push the a register onto the stack 
    lda #%00000000  ;set Port B to input
    sta DDRB
check_bf:
    lda #RW
    sta PORTA    
    lda #(RW | E)
    sta PORTA
    lda PORTB       ;a = 1xxxxxxx if Instruction execution 0xxxxxxx if done
    and #%10000000  ;we and a so if a = 0 then done else not done
    bne check_bf    ;bne means brach if result not equal to 0

    lda #0          ;clear the enable bit
    sta PORTA
    lda #%11111111  ;set Port B to output
    sta DDRB
    pla             ;pop the a register off of the stack
    rts

lcd_instruction:
    jsr lcd_wait
    sta PORTB
    lda #0          ;set E bit to send instruciton
    sta PORTA
    lda #E          ;clear RS/RW/E bits
    sta PORTA
    lda #0
    sta PORTA    
    rts

print_char:
    jsr lcd_wait
    sta PORTB
    lda #RS
    sta PORTA
    lda #(RS | E)
    sta PORTA
    lda #RS
    sta PORTA
    rts

nmi:
irq:
    rti

    .org $fffa
    .word nmi
    .word reset
    .word irq
    
