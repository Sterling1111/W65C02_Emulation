; STAR DODGE -- a two-lane arcade game for W65C02 Studio's 16x2 LCD.
; Set CLOCK to 1 MHz, Build & Run (F5), then use the Breadboard tab.
; N / NMI: start, switch lanes, retry. R: full reset (also clears best).
; Collect stars, dodge rocks. Top-right = score; bottom-right = best.
; Uses W65C02 instructions, VIA T1 IRQs, WAI, and LCD CGRAM sprites.
; Assemble: ./VASM/vasm6502_oldstyle -Fbin -dotdir -wdc02 -o build/star_dodge.bin VASM/star_dodge.asm

PORTB = $6000
PORTA = $6001
DDRB  = $6002
DDRA  = $6003
T1CL  = $6004
T1CH  = $6005
ACR   = $600b
PCR   = $600c
IFR   = $600d
IER   = $600e
E     = $80
RW    = $40
RS    = $20
TIMER = 49998                    ; free-running period = 50000 clocks: 20 Hz at 1 MHz

; Debugger-visible RAM. Digits are numeric, ones first, capped at 999.
mode       = $00                ; 0 title, 1 playing, 2 crashed
lane       = $01                ; 0 upper, 1 lower
score      = $02                ; three decimal digits
best       = $05                ; retained on N retry, cleared on R/reset
input_seq  = $08                ; NMI owns this counter
input_seen = $09                ; foreground owns this counter
ticks      = $0a                ; IRQ owns this counter
ticks_seen = $0b
countdown  = $0c
speed      = $0d                ; 6..3 timer ticks per move: 300..150 ms
spawn_in   = $0e
random     = $0f
row        = $10
frame      = $11                ; increments after each complete LCD redraw
ready      = $12                ; becomes 1 when title and timer are ready
level_in   = $13                ; speed increases every five stars
text_ptr   = $18                ; two-byte pointer used only by foreground
upper      = $20                ; 12 cells: 0 empty, 1 rock, 2 star
lower      = $30                ; 12 cells, same encoding

    .org $8000
reset:
    sei
    cld
    ldx #$ff
    txs
    ldx #$3f
clear_ram:
    stz $00,x
    dex
    bpl clear_ram
    lda #$a7
    sta random
    lda #$7f
    sta IER
    sta IFR
    stz ACR
    stz PCR
    stz PORTA
    lda #$e0
    sta DDRA
    lda #$ff
    sta DDRB
    jsr lcd_init
    lda #$40                    ; CGRAM: spaceship, asteroid, star, explosion
    jsr lcd_command
    ldx #0
load_sprites:
    lda sprites,x
    jsr lcd_char
    inx
    cpx #32
    bne load_sprites
    lda #<title_text
    sta text_ptr
    lda #>title_text
    sta text_ptr+1
    jsr draw_text
    lda #$40                    ; T1 continuous, PB7 remains LCD data GPIO
    sta ACR
    lda #<TIMER
    sta T1CL
    lda #>TIMER
    sta T1CH
    lda #$c0
    sta IER
    lda #1
    sta ready
    cli

; No delay loops: sleep between timer ticks and NMI button events.
; Check/sleep with I set avoids losing a timer IRQ just before WAI.
; NMI can happen even with I set: if it lands before WAI, the next timer
; wakes us within 50 ms and the queued input is still processed.
main_loop:
    sei
    lda input_seq
    cmp input_seen
    bne got_input
    lda ticks
    cmp ticks_seen
    bne got_tick
    wai
    cli
    jmp main_loop

got_input:
    inc input_seen              ; do not clear the NMI-owned counter
    cli
    lda mode
    cmp #1
    beq switch_lane
    jsr start_round
    jmp main_loop
switch_lane:
    lda lane
    eor #1
    sta lane
    jsr check_collision
    jsr draw_state
    jmp main_loop

got_tick:
    inc ticks_seen
    cli
    jsr next_random             ; title-screen wait time changes the course
    lda mode
    cmp #1
    bne main_loop
    dec countdown
    bne main_loop
    lda speed
    sta countdown
    jsr move_course
    jsr draw_state
    jmp main_loop

start_round:
    ldx #11
clear_course:
    stz upper,x
    stz lower,x
    dex
    bpl clear_course
    stz score
    stz score+1
    stz score+2
    stz lane
    lda #6
    sta speed
    sta countdown
    lda #5
    sta level_in
    lda #1
    sta spawn_in
    sta mode
    lda ticks
    sta ticks_seen
    jsr draw_course
    rts

move_course:
    ldx #0
shift_course:
    lda upper+1,x
    sta upper,x
    lda lower+1,x
    sta lower,x
    inx
    cpx #11
    bne shift_course
    stz upper+11
    stz lower+11
    dec spawn_in
    bne check_collision
    lda #3                      ; two empty columns between waves
    sta spawn_in
    jsr next_random
    and #1
    beq rock_above
    lda #2
    sta upper+11
    lda #1
    sta lower+11
    bra check_collision
rock_above:
    lda #1
    sta upper+11
    lda #2
    sta lower+11
check_collision:
    ldx lane
    lda upper
    cpx #0
    beq check_cell
    lda lower
check_cell:
    cmp #1
    beq crash
    cmp #2
    bne collision_done
    ; Consume the entire wave at the ship column, so lane changes cannot
    ; collect it twice or collide with a wave already passed safely.
    stz upper
    stz lower
    jsr add_point
collision_done:
    rts
crash:
    lda #2
    sta mode
    rts

add_point:
    lda score+2
    cmp #9
    bne increment_score
    lda score+1
    cmp #9
    bne increment_score
    lda score
    cmp #9
    beq update_best             ; saturate at 999 instead of wrapping
increment_score:
    inc score
    lda score
    cmp #10
    bne update_best
    stz score
    inc score+1
    lda score+1
    cmp #10
    bne update_best
    stz score+1
    inc score+2
update_best:
    ldx #2
compare_best:
    lda score,x
    cmp best,x
    bcc advance_level
    bne copy_best
    dex
    bpl compare_best
    bra advance_level
copy_best:
    ldx #2
copy_best_loop:
    lda score,x
    sta best,x
    dex
    bpl copy_best_loop
advance_level:
    dec level_in
    bne point_done
    lda #5
    sta level_in
    lda speed
    cmp #3
    beq point_done
    dec speed
point_done:
    rts

next_random:
    lda random
    asl
    bcc random_ready
    eor #$1d                    ; maximal-length, nonzero 8-bit LFSR
random_ready:
    sta random
    rts

draw_state:
    lda mode
    cmp #2
    beq draw_crash
    jmp draw_course
draw_crash:
    lda #<crash_text
    sta text_ptr
    lda #>crash_text
    sta text_ptr+1
    jsr draw_text
    lda #$80
    jsr lcd_command
    lda #3
    jsr lcd_char
    lda #$8d
    jsr lcd_command
    ldx #score
    jsr draw_number
    lda #$c5
    jsr lcd_command
    ldx #best
    jsr draw_number
    inc frame
    rts

draw_course:
    stz row
    lda #$80
    jsr lcd_command
draw_row:
    ldx #0
draw_cell:
    lda upper,x
    ldy row
    beq have_cell
    lda lower,x
have_cell:
    cmp #0
    bne cell_sprite
    lda #' '
cell_sprite:
    cpx #0
    bne emit_cell
    ldy row
    cpy lane
    bne emit_cell
    lda #0                      ; custom spaceship character
emit_cell:
    jsr lcd_char
    inx
    cpx #12
    bne draw_cell
    lda #'|'
    jsr lcd_char
    ldx #score
    lda row
    beq row_number
    ldx #best
row_number:
    jsr draw_number
    lda row
    bne course_done
    inc row
    lda #$c0
    jsr lcd_command
    bra draw_row
course_done:
    inc frame
    rts

; X points at a three-digit, little-endian decimal number in zero page.
draw_number:
    lda $02,x
    ora #'0'
    jsr lcd_char
    lda $01,x
    ora #'0'
    jsr lcd_char
    lda $00,x
    ora #'0'
    jmp lcd_char

; Exactly 32 bytes: avoids clearing the LCD and visible clear-screen flicker.
draw_text:
    lda #$80
    jsr lcd_command
    ldy #0
text_loop:
    lda (text_ptr),y
    jsr lcd_char
    iny
    cpy #16
    bne text_not_second_row
    lda #$c0
    jsr lcd_command
text_not_second_row:
    cpy #32
    bne text_loop
    rts

; Only foreground touches the LCD; both handlers preserve A/X/Y.
irq:
    pha
    lda IFR
    and #$40
    beq irq_done
    lda T1CL                    ; acknowledge T1, leave it free-running
    inc ticks
irq_done:
    pla
    rti
nmi:
    inc input_seq               ; queue one press, never touch LCD or shared A
    rti

lcd_init:
    lda #$38
    jsr lcd_command
    lda #$01
    jsr lcd_command
    lda #$0c
    jsr lcd_command
    lda #$06
    jmp lcd_command
lcd_wait:
    pha
    stz DDRB
lcd_busy:
    lda #RW
    sta PORTA
    lda #(RW | E)
    sta PORTA
    lda PORTB
    and #$80
    bne lcd_busy
    stz PORTA
    lda #$ff
    sta DDRB
    pla
    rts
lcd_command:
    jsr lcd_wait
    sta PORTB
    stz PORTA
    lda #E
    sta PORTA
    stz PORTA
    rts
lcd_char:
    jsr lcd_wait
    sta PORTB
    lda #RS
    sta PORTA
    lda #(RS | E)
    sta PORTA
    lda #RS
    sta PORTA
    rts

sprites:
    .byte %10000,%11000,%11110,%01111,%11110,%11000,%10000,%00000
    .byte %01110,%11111,%11011,%11111,%10111,%11111,%01110,%00000
    .byte %00100,%10101,%01110,%11111,%01110,%10101,%00100,%00000
    .byte %10001,%01010,%00100,%11011,%00100,%01010,%10001,%00000
title_text:
    .byte "   STAR DODGE   "
    .byte "1MHz N:PLAY/SWAP"
crash_text:
    .byte " BOOM! SCORE 000"
    .byte "BEST 000 N:RETRY"

    .org $fffa
    .word nmi,reset,irq
