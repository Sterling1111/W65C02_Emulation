# Serial terminal, WozMon and Microsoft BASIC

Build and run the `main` target, then open the **Terminal** tab.

- **Boot WozMon** loads the bundled ROM and resets into the monitor.
- **Boot BASIC** loads the same ROM, enters `8000R` in WozMon, accepts automatic
  memory sizing, and selects an 80-column terminal. Wait for `OK` before typing.
- Both buttons start a fresh board at **1 MHz**. Existing assembly editor buffers
  are preserved. The running ROM, RAM, serial queues and LCD state are replaced.
- Type uppercase commands. The serial BIOS echoes received input; the terminal
  does not add a second local echo.
- **Enter** sends carriage return, **Backspace** sends BS, and **Esc** sends ESC.
- **Paste / Ctrl+V** accepts ASCII, converts pasted line endings to carriage
  returns, and queues input at the configured serial baud rate.
- **Ctrl+C** cancels unsent pasted input and sends BASIC's BREAK character.
- **Copy / Ctrl+Shift+C** copies the terminal transcript. **Clear** clears only
  the display. The mouse wheel scrolls up to 2,000 retained lines.
- **Ctrl+Tab** cycles Editor, Breadboard and Terminal. Letters typed in Terminal
  do not operate the board's reset or interrupt keys.
- **F8** pauses/resumes the CPU and serial clocks. **F9** resets paused; continuing
  then enters WozMon because it is the ROM's reset entry, even after using BASIC.

## Try BASIC

Click **Boot BASIC**, wait for `OK`, and enter:

```basic
PRINT 2+2
10 FOR I=1 TO 5
20 PRINT "HELLO FROM BASIC ";I
30 NEXT I
RUN
```

`LIST` displays the program. `NEW` clears it. `PRINT 1.5*2` demonstrates BASIC's
software floating-point arithmetic; it does not require the CPU's decimal mode.
`POKE 12288,42:PRINT PEEK(12288)` demonstrates access to actual emulated RAM.
A pasteable loop is included in `examples/basic/hello.bas`.

The original ROM reports **15359 BYTES FREE** with this board's 16 KiB RAM map.
Its `LOAD` and `SAVE` BIOS routines are upstream stubs: persistent cassette/disk
storage is not implemented. Keep BASIC source in host text files and paste it
back into the terminal. Use **Boot WozMon** to leave BASIC and start a fresh board.

## Try WozMon

Click **Boot WozMon**. Its initial prompt is a backslash. Enter:

```text
0400: A9 5A 85 10 4C 00 FE
0400.0406
0400R
0010
```

This stores and displays a real machine-code program, runs it to write `$5A` at
`$0010`, and returns to WozMon. The last command examines the result. `8000R`
starts BASIC manually; press Enter at `MEMORY SIZE?` and enter `80` at
`TERMINAL WIDTH?`. Zero page, the stack, `$0200` monitor buffer and `$0300` BIOS
receive buffer are occupied by firmware; use `$0400` or above for monitor demos.

## Board and serial model

| Address | Read | Write |
| --- | --- | --- |
| `$5000` | Received data | Transmit data |
| `$5001` | Status; acknowledges serial IRQ | Programmed reset |
| `$5002` | Command register | Command register |
| `$5003` | Control register | Control register |

The ACIA IRQ joins the VIA and manual IRQ sources. Ben's BIOS uses VIA **PA0**
to stop the terminal sender while its 256-byte input buffer is nearly full.
The terminal queue is outside the chip; the ACIA still has a single received
byte register and can overrun if incoming frames are not serviced. Pasting is
bounded at 64 KiB. Serial timing advances only with emulated PHI2 clocks.

This is a byte/frame-level **W65C51N** model, including the real part's always-set
transmit-empty bit and unbuffered transmitter: writing too soon replaces the
unfinished frame. Keep the original firmware at **1 MHz / 19200 baud / 8-N-1**;
its transmit delay is a software loop, so arbitrarily increasing CPU frequency
can lose serial output just as it can on that firmware's hardware setup.
Control bits select frame length, stop bits and baud rate. The external receive
clock defaults to 1.8432 MHz divided by 16. The N variant does not generate parity.

The breadboard drawing includes the W65C51N and MAX232. MAX232 voltage conversion
is illustrative; electrical waveforms, analog noise and modem transitions during
individual serial bits are not modeled. The peer is the in-app terminal, not a
host COM port. The register model exposes CTS/DSR/DCD inputs for tests and future
serial peers. There is no host-side BASIC or WozMon command interpreter.

The bundled, unmodified ROM is from Ben Eater's **RS232 flow control** revision,
which fits the existing board without the later LCD rewiring or sound expansion.
See [firmware provenance and rebuilding](../firmware/README.md).
[Ben Eater's series](https://eater.net/6502) and the
[WDC W65C51N datasheet](https://www.westerndesigncenter.com/wdc/documentation/w65c51n.pdf)
are the references for this expansion.

## CPU fixes and hardware validation

`CpuConformanceTests.cpp` covers every BBR/BBS bit, taken/not-taken and signed
page-crossing branches; zero-page,Y wrapping; all 44 reserved NOP lengths and
cycle counts; STP decoding; simultaneous IRQ/NMI priority and pushed NMI status.
Binary ADC/SBC results and N/V/Z/C are checked over all 131,072 input/carry
combinations per instruction. Decimal-mode arithmetic remains unimplemented.

**The original 13 physical-board trace comparisons and their captured files are
unchanged.** An initial generic STP change added a read absent from the captures
and was rejected. STP now keeps the original opcode-plus-trailing-read sequence
and two counted accesses, but the trailing read no longer increments PC. This
preserves those captures instead of claiming a different STP bus sequence.
The incorrect zero-page,Y cycle assertions in synthetic unit tests were corrected;
no hardware expectations were changed or disabled.

`SerialTests.cpp` checks ACIA register and frame behavior and boots the bundled
ROM to execute monitor memory commands, machine code, BASIC arithmetic, loops,
strings, PEEK/POKE, a long pasted program, and BREAK. These are firmware integration
tests, not additional physical-board captures of the new ACIA.
