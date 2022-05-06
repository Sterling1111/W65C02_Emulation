PORTB = $6000
PORTA = $6001
DDRB = $6002
DDRA = $6003

value = $0200 ; 2 bytes
mod10 = $0202 ; 2 bytes
message = $0204 ; 6 bytes
counter = $020a
Fn = $2000
Fn_1 = $2001
Fn_2 = $2002

E  = %10000000
RW = %01000000
RS = %00100000

  .org $8000

reset:
  ldx #$ff
  txs

  lda #%11111111 ; Set all pins on port B to output
  sta DDRB
  lda #%11100000 ; Set top 3 pins on port A to output
  sta DDRA

  lda #%00111000 ; Set 8-bit mode; 2-line display; 5x8 font
  jsr lcd_instruction
  lda #%00001100 ; Display on; cursor off; blink off
  jsr lcd_instruction
  lda #%00000110 ; Increment and shift cursor; don't shift display
  jsr lcd_instruction
  lda #%00000001 ; Clear display
  jsr lcd_instruction

restart:
  lda #%00000001 ; Clear display
  jsr lcd_instruction
  lda #$0
  sta Fn_2
  lda #$01
  sta Fn_1

  lda #$0

fibbonacci:
  jsr delay
  jsr print_num
  jsr delay
  pha
  lda #%00000001 ; Clear display
  jsr lcd_instruction
  pla
  clc
  adc Fn_2
  clc
  adc Fn_1
  bcs restart
  ldx Fn_1
  stx Fn_2
  sta Fn_1
  jmp fibbonacci

  
print_num:
  pha
  lda #0
  sta message
  pla

  ; Initialize value to be the number to convert
  ;lda #255
  ;lda number
  sta value
  pha
  lda #$00
  ;lda number + 1
  sta value + 1
  pla

divide:
  ; Initialize the remainder to zero
  lda #0
  sta mod10
  sta mod10 + 1
  clc

  ldx #16

divloop:
  ; Rotate quotient and remainder
  rol value
  rol value + 1
  rol mod10
  rol mod10 + 1

  ; a,y = dividend - devisor
  sec
  lda mod10
  sbc #10
  tay ; save low byte in Y
  lda mod10+1
  sbc #0
  bcc ignore_result ; branch if dividend < devisor
  sty mod10
  sta mod10 + 1

ignore_result:
  dex
  bne divloop
  rol value ; shift in the last bit of the quotient
  rol value + 1

  lda mod10
  clc
  adc #"0"
  jsr  push_char

  ; if value != 0, then continue dividing
  lda value
  ora value + 1
  bne divide ; branch if value not equal to 0

  ldx #0

print:
  lda message,x
  beq return
  jsr print_char
  inx
  jmp print
return:
  rts

number: .word 4200

; Add the character in the A register to the beginning of the 
; null-terminated string `message`
push_char:
  pha ; Push new first char onto stack
  ldy #0

char_loop:
  lda message,y ; Get char on the string and put into X
  tax
  pla
  sta message,y ; Pull char off stack and add it to the string
  iny
  txa
  pha           ; Push char from string onto stack
  bne char_loop

  pla
  sta message,y ; Pull the null off the stack and add to the end of the string

  rts

lcd_wait:
  pha
  lda #%00000000  ; Port B is input
  sta DDRB
lcdbusy:
  lda #RW
  sta PORTA
  lda #(RW | E)
  sta PORTA
  lda PORTB
  and #%10000000
  bne lcdbusy

  lda #RW
  sta PORTA
  lda #%11111111  ; Port B is output
  sta DDRB
  pla
  rts

lcd_instruction:
  jsr lcd_wait
  sta PORTB
  lda #0         ; Clear RS/RW/E bits
  sta PORTA
  lda #E         ; Set E bit to send instruction
  sta PORTA
  lda #0         ; Clear RS/RW/E bits
  sta PORTA
  rts

print_char:
  jsr lcd_wait
  sta PORTB
  lda #RS         ; Set RS; Clear RW/E bits
  sta PORTA
  lda #(RS | E)   ; Set E bit to send instruction
  sta PORTA
  lda #RS         ; Clear E bits
  sta PORTA
  rts

nmi:
  rti


irq:
  inc counter
  bne exit_irq
  inc counter + 1
exit_irq:
  rti

delay:
  phy
  phx
  ldy #$ff
delay2:
  ldx #$ff
delay1:
  nop 
  dex
  bne delay1
  dey
  bne delay2
  plx
  ply
  rts

  .org $fffc
  .word reset
  .word $0000