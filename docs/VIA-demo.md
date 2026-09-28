# W65C22 assembly demonstration

[VASM/via_demo.asm](../VASM/via_demo.asm) is a standalone 32 KiB ROM for the
existing board: RAM at 0000–3FFF, VIA at 6000, ROM at 8000–FFFF. It uses the
normal LCD wiring (PA7=E, PA6=R/W, PA5=RS, PB7..0=data). It needs no extra
virtual peripherals or loopback wiring.

From the repository root, assemble and run:

```sh
./VASM/vasm6502_oldstyle -Fbin -dotdir -wdc02 -o build/a.out VASM/via_demo.asm
(cd build && ./SystemLib/main)
```

In the IDE, select `via_demo.asm` and use **Build & Run**. For the command-line
ROM above, open the **Breadboard** tab and press **R** to start or restart. The application first runs six self-tests,
then waits for interrupts from both timers. After both handlers have executed:

```text
VIA OK   SR:A5
T1:0001 T2:0001
```

The values are hexadecimal, independently incrementing 16-bit counts with
wraparound from FFFF to 0000. They count serviced VIA interrupts, not seconds.
The foreground takes an atomic snapshot before drawing. Interrupt handlers
acknowledge their own source, update counters, and rearm T2; they never access
the LCD, so LCD busy polling and bus transfers can be interrupted safely.

## What it demonstrates

| Code | Startup check |
| --- | --- |
| 01 | DDRA/DDRB readback, PA no-handshake alias, PB output patterns 55 and AA. |
| 02 | IER bit-7 set/clear semantics and preservation of other enabled sources. |
| 03 | T1 one-shot: PB7 starts low and rises on timeout; latch readback; T1CL acknowledgement. |
| 04 | T2 timed event: IFR latches with IRQ masked, enabling it asserts IFR7, masking retains the event, T2CL clears it. |
| 05 | T2 pulse mode: firmware drives PB6 high/low with LCD E held low; three falling edges take a count of three to zero, the fourth underflows; IFR write-one-to-clear. |
| 06 | PHI2-clocked serial output: shift A5 through eight rotations, check completion flag and SR read acknowledgement. |

During tests 03/05 the LCD's E pin stays low, so the PB7 timer output and PB6
pulses do not become LCD commands. Timer PB7 override is disabled before any
LCD transfers. In normal operation the LCD's busy-flag polling also exercises
DDRB switching between output and input.

After startup, T1 runs in free-running mode and T2 runs in one-shot timed mode
with explicit rearming by the ISR. IRQ delivery is required to reach `VIA OK`;
without it, the display remains at `IRQ WAIT` and the counters stay zero.
`WAI` sleeps the emulated CPU between updates while the VIA continues to clock.
The foreground masks interrupts while testing for pending work and entering
`WAI`; a W65C02 wakes on IRQ even when I is set, then `CLI` allows servicing.
This avoids losing a wakeup between checking work and sleeping.

The self-test is a practical demonstration, not proof of every chip mode.
It does not validate externally driven CA/CB input interrupts, handshakes,
input latching, external serial clocks, or electrical characteristics. The
serial check observes completion and recirculation; it does not use a serial
receiver to validate the CB2 waveform. Those digital modes have separate
coverage in `SystemTest/W65C22Tests.cpp`.

## Timing and failures

T1 uses a 4094 latch and repeats every **4096 PHI2 cycles**. T2 uses a count
of 8191 and expires **8192 cycles** after loading; the time taken to enter the
ISR and reload it adds to each subsequent interval. The first T1 interval
also follows the chip's initial-load timing rather than the repeating period.

At the current 1 kHz CPU setting these are roughly 4.1 and 8.2 seconds; at
1 MHz they are roughly 4.1 and 8.2 milliseconds. LCD initialization/drawing
also consumes CPU cycles. Change frequency with the IDE's CLOCK control, or set the initial `cpuMHz` in
`SystemLib/main.cpp`; the demo does not alter the CPU or host pacing.

A failed startup check prints `VIA FAIL CODE:xx`, followed by `Press R to retry`,
then stops with `STP`. The table above identifies the failed stage. Event polls
are bounded to 512 IFR reads. LCD busy polling assumes a working connected LCD;
a broken LCD bus may prevent the error itself from being displayed. Failure
status remains available in RAM in that case. Avoid holding **I** during the
demo: it asserts a separate external CPU IRQ that VIA acknowledgements cannot
clear. **N** is ignored by the ROM's empty NMI handler.

RAM inspection addresses:

| Address | Value |
| --- | --- |
| 00–01 / 02–03 | T1 / T2 interrupt counts, little-endian. |
| 04–05 / 06–07 | Last foreground snapshot of each count. |
| 08 | Serial result (A5 after passing). |
| 09 | Status: 00 waiting, 55 passed, EE failed. |
| 0A | Failure code (00 on success). |
| 0B | ISR dirty flag. |
| 0C | Startup stage, 01–06. |
| 0D | Scratch event mask for bounded polling. |
| 0E | Completed LCD counter-update sequence, modulo 256. |

## Reproducible integration check

This optional check assembles the same source and runs the actual ROM on the
emulated board without a window or wall-clock waits:

```sh
./VASM/vasm6502_oldstyle -Fbin -dotdir -wdc02 -o build/via_demo.bin VASM/via_demo.asm
cmake -S . -B build
cmake --build build --target ViaDemoSmoke -j2
./build/SystemTest/ViaDemoSmoke build/via_demo.bin
```

It verifies startup, both IRQ counters, `WAI`, complete LCD pixel frames,
16-bit counter wraparound, and reset with retained LCD memory, at 1 kHz and
1 MHz. It also suppresses peripheral clocks during the T1 and serial tests
and checks the displayed failure codes, then masks runtime timer IRQs and
checks that the ROM does not falsely report success.
