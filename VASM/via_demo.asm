; W65C22 demonstration for the existing W65C02 + 16x2 LCD board.
; Assemble: ./VASM/vasm6502_oldstyle -Fbin -dotdir -wdc02 -o build/a.out VASM/via_demo.asm
; Run from build/, then press R. No extra peripheral wiring is needed.
;
; Startup checks (failure numbers):
; 01 GPIO direction/output readback     02 IER set/clear semantics
; 03 T1 one-shot + PB7 output           04 T2 timed IRQ flag/masking/ack
; 05 T2 PB6 falling-edge pulse counter  06 SR PHI2 output/recirculation/ack
;
; Then T1 runs continuously, T2 is rearmed by its ISR, and the CPU uses WAI.
; "VIA OK" appears only after BOTH timer interrupt handlers have executed.
; The four-digit counters are hexadecimal, wrapping at FFFF -> 0000.
; T1 repeats every 4096 PHI2 cycles (4.096 s at 1 kHz, 4.096 ms at 1 MHz).
; T2 expires 8192 cycles after loading, plus ISR latency before each reload.
; PB7/PB6 tests run with LCD E low; timer PB7 is disabled before LCD access.
; CA/CB external-input/handshake modes need a peripheral and are not tested here.

PORTB = $6000
PORTA = $6001
DDRB  = $6002
DDRA  = $6003
T1CL  = $6004
T1CH  = $6005
T1LL  = $6006
T1LH  = $6007
T2CL  = $6008
T2CH  = $6009
SR    = $600a
ACR   = $600b
PCR   = $600c
IFR   = $600d
IER   = $600e
PORTA_NH = $600f
E  = $80
RW = $40
RS = $20
T1_LOAD = $0ffe
T2_LOAD = $1fff

; RAM is deliberately documented for debugger/headless smoke checks.
t1_count  = $00                  ; 16-bit, modified only by IRQ
t2_count = $02                  ; 16-bit, modified only by IRQ
snap_t1   = $04                  ; coherent foreground snapshot
snap_t2   = $06
sr_result = $08
status    = $09                  ; 00 waiting, 55 passed, EE failed
fail_code = $0a                  ; 01..06 above; 00 on success
dirty     = $0b
stage     = $0c
wait_mask = $0d
frame     = $0e                  ; completed LCD update sequence (8-bit)

    .org $8000
reset:
    sei
    cld
    ldx #$ff
    txs
    ldx #$0e
clear_ram:
    stz $00,x
    dex
    bpl clear_ram
    lda #$7f
    sta IER                     ; mask all VIA sources
    sta IFR                     ; clear old pending flags
    stz ACR
    stz PCR
    stz PORTA                   ; hold LCD E low during peripheral tests
    lda #$e0
    sta DDRA
    lda #$ff
    sta DDRB

    inc stage
    jsr test_ports
    bcc ports_ok
    jmp test_failed
ports_ok:
    inc stage
    jsr test_ier
    bcc ier_ok
    jmp test_failed
ier_ok:
    inc stage
    jsr test_t1
    bcc t1_ok
    jmp test_failed
t1_ok:
    inc stage
    jsr test_t2
    bcc t2_ok
    jmp test_failed
t2_ok:
    inc stage
    jsr test_pulses
    bcc pulses_ok
    jmp test_failed
pulses_ok:
    inc stage
    jsr test_serial
    bcc serial_ok
    jmp test_failed
serial_ok:
    jsr lcd_init
    ldx #0
waiting_title:
    lda waiting_text,x
    beq initial_row
    jsr lcd_char
    inx
    bra waiting_title
initial_row:
    lda #$c0
    jsr lcd_command
    ldx #0
initial_row_loop:
    lda counter_text,x
    beq start_timers
    jsr lcd_char
    inx
    bra initial_row_loop

start_timers:
    lda #$40                    ; T1 free run, PB7 stays GPIO, T2 timed
    sta ACR
    lda #$7f
    sta IFR
    lda #<T1_LOAD
    sta T1CL
    lda #>T1_LOAD
    sta T1CH
    lda #<T2_LOAD
    sta T2CL
    lda #>T2_LOAD
    sta T2CH
    lda #$e0                    ; enable both T1 and T2 IRQ sources
    sta IER
    cli

main_loop:
    sei                         ; atomic dirty check + counter snapshot
    lda dirty
    bne snapshot
    ; W65C02 WAI wakes even with I set. Keep I set through WAI to avoid
    ; an interrupt arriving between the dirty check and sleep being lost.
    wai
    cli                         ; service the interrupt that woke WAI
    bra main_loop
snapshot:
    stz dirty
    lda t1_count
    sta snap_t1
    lda t1_count+1
    sta snap_t1+1
    lda t2_count
    sta snap_t2
    lda t2_count+1
    sta snap_t2+1
    cli                         ; LCD writes never hold off IRQs
    lda status
    cmp #$55
    beq draw_counters
    lda snap_t1
    ora snap_t1+1
    beq draw_counters
    lda snap_t2
    ora snap_t2+1
    beq draw_counters
    lda #$80
    jsr lcd_command
    ldx #0
passed_title:
    lda passed_text,x
    beq passed_serial
    jsr lcd_char
    inx
    bra passed_title
passed_serial:
    lda sr_result
    jsr lcd_hex_byte
    lda #$55                    ; success requires real IRQ delivery
    sta status

draw_counters:
    lda #$c3                    ; second row, after "T1:"
    jsr lcd_command
    lda snap_t1+1
    jsr lcd_hex_byte
    lda snap_t1
    jsr lcd_hex_byte
    lda #$cb                    ; second row, after "T2:"
    jsr lcd_command
    lda snap_t2+1
    jsr lcd_hex_byte
    lda snap_t2
    jsr lcd_hex_byte
    inc frame
    jmp main_loop

; IRQ owns counters and timer acknowledgement only; it never touches the LCD
; or X/Y. Both sources can be pending on the same interrupt entry.
irq:
    pha
    lda IFR
    and #$40
    beq irq_t2
    lda T1CL                    ; clear only T1 flag, free-run keeps going
    inc t1_count
    bne irq_t1_done
    inc t1_count+1
irq_t1_done:
    lda #1
    sta dirty
irq_t2:
    lda IFR
    and #$20
    beq irq_done
    lda T2CL                    ; clear T2 flag
    lda #>T2_LOAD
    sta T2CH                    ; rearm one-shot; low latch stays unchanged
    inc t2_count
    bne irq_t2_done
    inc t2_count+1
irq_t2_done:
    lda #1
    sta dirty
irq_done:
    pla
    rti
nmi:
    rti

; Startup tests return C=0 on success, C=1 on failure.
test_ports:
    lda DDRA
    cmp #$e0
    bne ports_fail
    lda DDRB
    cmp #$ff
    bne ports_fail
    lda #RS                     ; PA alias, E stays low
    sta PORTA_NH
    lda PORTA
    and #$e0
    cmp #RS
    bne ports_fail
    stz PORTA
    lda #$55
    sta PORTB
    lda PORTB
    cmp #$55
    bne ports_fail
    lda #$aa
    sta PORTB
    lda PORTB
    cmp #$aa
    bne ports_fail
    clc
    rts
ports_fail:
    sec
    rts

test_ier:
    lda IER
    cmp #$80
    bne ier_fail
    lda #$c0
    sta IER                     ; set T1
    lda #$a0
    sta IER                     ; set T2 without losing T1
    lda IER
    cmp #$e0
    bne ier_fail
    lda #$40
    sta IER                     ; clear T1, retain T2
    lda IER
    cmp #$a0
    bne ier_fail
    lda #$7f
    sta IER
    lda IER
    cmp #$80
    bne ier_fail
    clc
    rts
ier_fail:
    sec
    rts

test_t1:
    lda #$80                    ; one-shot with PB7 output enabled
    sta ACR
    stz PORTB
    stz T1CL
    lda #1
    sta T1CH                    ; 256 down to underflow
    lda PORTB
    bmi t1_fail                 ; load must initially drive PB7 low
    lda #$40
    jsr wait_flag
    bcs t1_fail
    lda PORTB
    bpl t1_fail                 ; timeout drives PB7 high
    lda T1LL
    bne t1_fail
    lda T1LH
    cmp #1
    bne t1_fail
    lda T1CL                    ; low counter read acknowledges timeout
    lda IFR
    and #$40
    bne t1_fail
    stz ACR                     ; return PB7 to LCD data bus control
    clc
    rts
t1_fail:
    sec
    rts

test_t2:
    stz T2CL
    lda #1
    sta T2CH
    lda #$20
    jsr wait_flag
    bcs t2_fail
    lda IFR
    bmi t2_fail                 ; event is pending, but IER masks IRQ
    lda #$a0
    sta IER
    lda IFR
    bpl t2_fail                 ; enabling pending source asserts summary
    lda #$20
    sta IER
    lda IFR
    bmi t2_fail                 ; mask releases IRQ, retains event
    and #$20
    beq t2_fail
    lda T2CL
    lda IFR
    and #$20
    bne t2_fail                 ; counter read cleared event
    clc
    rts
t2_fail:
    sec
    rts

test_pulses:
    lda #$20
    sta ACR                     ; T2 counts PB6 negative transitions
    lda #$40
    sta PORTB                   ; high before arming timer
    lda #3
    sta T2CL
    stz T2CH
    ldx #3
pulse_loop:
    stz PORTB                   ; one falling edge
    lda #$40
    sta PORTB
    dex
    bne pulse_loop
    lda IFR
    and #$20
    bne pulses_fail             ; three edges reach zero, not underflow
    lda T2CL
    bne pulses_fail
    stz PORTB                   ; fourth falling edge underflows
    lda IFR
    and #$20
    beq pulses_fail
    lda #$20
    sta IFR                     ; also demonstrate write-one-to-clear
    lda IFR
    and #$20
    bne pulses_fail
    stz ACR
    clc
    rts
pulses_fail:
    sec
    rts

test_serial:
    lda #$18                    ; shift out under PHI2, eight clock pulses
    sta ACR
    lda #$a5
    sta SR
    lda #$04
    jsr wait_flag
    bcs serial_fail
    lda SR                      ; eight rotations restore the original byte
    sta sr_result
    cmp #$a5
    bne serial_fail
    ; SR read starts another transfer. Test acknowledgement immediately,
    ; before eight more clocks can complete, then disable the serial mode.
    lda IFR
    and #$04
    bne serial_fail
    stz ACR
    clc
    rts
serial_fail:
    sec
    rts

; Bounded polling during self-test only: at most 512 IFR reads.
; A supplies the event mask. X/Y are scratch. No infinite wait on bad timers.
wait_flag:
    sta wait_mask
    ldx #0
    ldy #2
wait_flag_loop:
    lda IFR
    and wait_mask
    bne flag_ready
    dex
    bne wait_flag_loop
    dey
    bne wait_flag_loop
    sec
    rts
flag_ready:
    clc
    rts

test_failed:
    lda stage
    sta fail_code
    lda #$ee
    sta status
    lda #$7f
    sta IER
    stz ACR                     ; release PB7 and serial overrides
    stz PORTA
    jsr lcd_init
    ldx #0
failure_title:
    lda failed_text,x
    beq failure_number
    jsr lcd_char
    inx
    bra failure_title
failure_number:
    lda fail_code
    jsr lcd_hex_byte
    lda #$c0
    jsr lcd_command
    ldx #0
failure_help:
    lda retry_text,x
    beq failure_stop
    jsr lcd_char
    inx
    bra failure_help
failure_stop:
    stp
    bra failure_stop

; LCD routines preserve X/Y. Only the foreground uses the LCD bus, so a
; timer IRQ may safely interrupt a transfer or busy-flag read at any point.
lcd_init:
    lda #$ff
    sta DDRB
    lda #$e0
    sta DDRA
    stz PORTA
    lda #$38
    jsr lcd_command
    lda #$01
    jsr lcd_command
    lda #$0c                    ; display on, no cursor/blink over counters
    jsr lcd_command
    lda #$06
    jmp lcd_command

lcd_wait:
    pha
    stz DDRB                    ; release bus for LCD busy flag
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
    stz PORTA                   ; falling E latches instruction
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

lcd_hex_byte:
    pha
    lsr
    lsr
    lsr
    lsr
    jsr lcd_hex_digit
    pla
    and #$0f
lcd_hex_digit:
    phx
    tax
    lda hex_digits,x
    jsr lcd_char
    plx
    rts

waiting_text: .asciiz "IRQ WAIT SR:--  "
passed_text:  .asciiz "VIA OK   SR:"
failed_text:  .asciiz "VIA FAIL CODE:"
retry_text:   .asciiz "Press R to retry"
counter_text: .asciiz "T1:0000 T2:0000 "
hex_digits:   .ascii "0123456789ABCDEF"

    .org $fffa
    .word nmi
    .word reset
    .word irq
