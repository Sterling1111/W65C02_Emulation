PORTB = $6000
PORTA = $6001
DDRB = $6002
DDRA = $6003

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
  jsr lcd_wait
  sta PORTB
  lda #0         ; Clear RS/RW/E bits
  sta PORTA
  lda #E         ; Set E bit to send Instruction
  sta PORTA
  lda #0         ; Clear RS/RW/E bits
  sta PORTA

  lda #%00000001 ; Clear and home the display, including after a CPU reset
  jsr lcd_wait
  sta PORTB
  lda #0
  sta PORTA
  lda #E
  sta PORTA
  lda #0
  sta PORTA

  lda #%00001110 ; Display on; cursor on; blink off
  jsr lcd_wait
  sta PORTB
  lda #0         ; Clear RS/RW/E bits
  sta PORTA
  lda #E         ; Set E bit to send Instruction
  sta PORTA
  lda #0         ; Clear RS/RW/E bits
  sta PORTA

  lda #%00000110 ; Increment and shift cursor; don't shift display
  jsr lcd_wait
  sta PORTB
  lda #0         ; Clear RS/RW/E bits
  sta PORTA
  lda #E         ; Set E bit to send Instruction
  sta PORTA
  lda #0         ; Clear RS/RW/E bits
  sta PORTA

  lda #"H"
  jsr lcd_wait
  sta PORTB
  lda #RS         ; Set RS; Clear RW/E bits
  sta PORTA
  lda #(RS | E)   ; Set E bit to send Instruction
  sta PORTA
  lda #RS         ; Clear E bits
  sta PORTA

  lda #"e"
  jsr lcd_wait
  sta PORTB
  lda #RS         ; Set RS; Clear RW/E bits
  sta PORTA
  lda #(RS | E)   ; Set E bit to send Instruction
  sta PORTA
  lda #RS         ; Clear E bits
  sta PORTA

  lda #"l"
  jsr lcd_wait
  sta PORTB
  lda #RS         ; Set RS; Clear RW/E bits
  sta PORTA
  lda #(RS | E)   ; Set E bit to send Instruction
  sta PORTA
  lda #RS         ; Clear E bits
  sta PORTA

  lda #"l"
  jsr lcd_wait
  sta PORTB
  lda #RS         ; Set RS; Clear RW/E bits
  sta PORTA
  lda #(RS | E)   ; Set E bit to send Instruction
  sta PORTA
  lda #RS         ; Clear E bits
  sta PORTA

  lda #"o"
  jsr lcd_wait
  sta PORTB
  lda #RS         ; Set RS; Clear RW/E bits
  sta PORTA
  lda #(RS | E)   ; Set E bit to send Instruction
  sta PORTA
  lda #RS         ; Clear E bits
  sta PORTA

  lda #","
  jsr lcd_wait
  sta PORTB
  lda #RS         ; Set RS; Clear RW/E bits
  sta PORTA
  lda #(RS | E)   ; Set E bit to send Instruction
  sta PORTA
  lda #RS         ; Clear E bits
  sta PORTA

  lda #" "
  jsr lcd_wait
  sta PORTB
  lda #RS         ; Set RS; Clear RW/E bits
  sta PORTA
  lda #(RS | E)   ; Set E bit to send Instruction
  sta PORTA
  lda #RS         ; Clear E bits
  sta PORTA

  lda #"w"
  jsr lcd_wait
  sta PORTB
  lda #RS         ; Set RS; Clear RW/E bits
  sta PORTA
  lda #(RS | E)   ; Set E bit to send Instruction
  sta PORTA
  lda #RS         ; Clear E bits
  sta PORTA

  lda #"o"
  jsr lcd_wait
  sta PORTB
  lda #RS         ; Set RS; Clear RW/E bits
  sta PORTA
  lda #(RS | E)   ; Set E bit to send Instruction
  sta PORTA
  lda #RS         ; Clear E bits
  sta PORTA

  lda #"r"
  jsr lcd_wait
  sta PORTB
  lda #RS         ; Set RS; Clear RW/E bits
  sta PORTA
  lda #(RS | E)   ; Set E bit to send Instruction
  sta PORTA
  lda #RS         ; Clear E bits
  sta PORTA

  lda #"l"
  jsr lcd_wait
  sta PORTB
  lda #RS         ; Set RS; Clear RW/E bits
  sta PORTA
  lda #(RS | E)   ; Set E bit to send Instruction
  sta PORTA
  lda #RS         ; Clear E bits
  sta PORTA

  lda #"d"
  jsr lcd_wait
  sta PORTB
  lda #RS         ; Set RS; Clear RW/E bits
  sta PORTA
  lda #(RS | E)   ; Set E bit to send Instruction
  sta PORTA
  lda #RS         ; Clear E bits
  sta PORTA

  lda #"!"
  jsr lcd_wait
  sta PORTB
  lda #RS         ; Set RS; Clear RW/E bits
  sta PORTA
  lda #(RS | E)   ; Set E bit to send Instruction
  sta PORTA
  lda #RS         ; Clear E bits
  sta PORTA

loop:
  jmp loop

; Preserve A while waiting for the controller, including its power-on reset.
lcd_wait:
  pha
  lda #0
  sta DDRB
check_bf:
  lda #RW
  sta PORTA
  lda #(RW | E)
  sta PORTA
  lda PORTB
  and #%10000000
  bne check_bf
  lda #0
  sta PORTA
  lda #$ff
  sta DDRB
  pla
  rts

  .org $fffc
  .word reset
  .word $0000