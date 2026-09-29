# Ben Eater W65C02 System Emulation

## Build on Linux / WSL

Install the compiler, CMake, SFML 2, and X11 development packages (Ubuntu/Debian):

```sh
sudo apt-get install build-essential cmake git libsfml-dev libx11-dev
cmake -S . -B build
cmake --build build -j2
```

CMake downloads Google Test and Google Benchmark on the first configuration,
so an internet connection is required. The bundled `SFML/` distribution is for
Windows; Linux builds use the installed SFML 2 package.

For this checkout, dependencies have also been unpacked into `build/deps`
without installing system packages. To use those local packages:

```sh
cmake -S . -B build -DCMAKE_PREFIX_PATH="$PWD/build/deps/usr"
cmake --build build -j2
export LD_LIBRARY_PATH="$PWD/build/deps/usr/lib/x86_64-linux-gnu${LD_LIBRARY_PATH:+:$LD_LIBRARY_PATH}"
```

The local packages are generated build files, not part of the repository.
A fresh clone should use the system-package installation above.

## Open the IDE

After building, start **W65C02 Studio** from the repository root:

```sh
./build/SystemLib/main
```

Choose a program from the left sidebar, edit it in the **Editor** tab, and
click **Build & Run** (F5). The IDE saves the source, assembles it with the
bundled vasm, loads the ROM, and opens the **Breadboard** tab. For a first run,
choose `hello_world.asm`. No separate assembler command is needed.

- **Ctrl+S** saves; **F6** builds without replacing the running program.
- **F7 / Build & Debug** loads the program paused before its first instruction.
- **Click the editor gutter / Ctrl+B** to toggle a breakpoint; **Ctrl+Shift+B** clears the file’s breakpoints.
- **F10 / Step** executes one instruction; **F8** pauses/continues; **F9** restarts paused.
- **+ / Ctrl+N** creates a program in `VASM/`.
- **Ctrl+Tab** cycles Editor, Breadboard and Terminal.
- **CLOCK** cycles the running clock speed without a rebuild.
- In the Breadboard tab, **R**, **I**, and **N** control reset, IRQ, and NMI.

The editor includes syntax highlighting, line numbers, selection, clipboard
shortcuts, undo/redo, search, an assembler output panel, and instruction stepping
with a source-line highlight and live registers. Open buffers retain
unsaved changes when switching programs; closing prompts to save or discard.
See [the IDE guide](docs/IDE.md) for all controls and file/build behavior.

Try **[Star Dodge](docs/Star-Dodge.md)**: select `star_dodge.asm`, set the clock to **1 MHz**, and press **F5**. Use **N** on the Breadboard tab to start, switch lanes, and retry.

The bundled Linux assembler requires x86-64. The emulator needs a graphical
display (WSLg works). The previous command-line workflow is still available:

```sh
./VASM/vasm6502_oldstyle -Fbin -dotdir -wdc02 -o build/a.out VASM/hello_world.asm
(cd build && ./SystemLib/main)
```

An existing `a.out` in the launch directory is loaded on startup. Open the
**Breadboard** tab and press **R** to run it. Build & Run uses the selected
source instead and keeps its generated ROM under `build/ide/`.

The CPU frequency is initially configured by `cpuMHz` in `SystemLib/main.cpp`: use
`1 * MHZ` for **1 MHz** (1,000,000 cycles per second), or `1 * KHZ` for **1 kHz**. The `W65C02` CPU owns its execution worker,
which executes roughly 1 ms of cycles per burst, then sleeps to an absolute
monotonic-clock deadline.
Instruction overshoot and late wakeups carry forward so they do not accumulate
timing drift. Rendering and input run independently. During `WAI` and `STP`, instruction
execution pauses while PHI2 continues to clock the VIA. A VIA interrupt can
wake `WAI`; `STP` requires reset. The worker sleeps between clock batches. Timing is
accurate on average when the host can keep up; individual cycles are batched.

## WozMon and Microsoft BASIC

Open the **Terminal** tab and click **Boot BASIC**. The app boots the bundled
Ben Eater ROM through the W65C51N serial chip, answers the startup prompts, and
shows `OK`. Try `PRINT 2+2`, or paste [the BASIC example](examples/basic/hello.bas).
**Boot WozMon** opens the monitor for examining memory and loading/running machine
code. Both boot buttons set the original firmware's clock to **1 MHz**.

The serial expansion adds the ACIA at `$5000–$5003`, receive IRQs and VIA PA0
flow control. **Ctrl+V** pastes; **Ctrl+C** sends BASIC BREAK. The ROM and its
pinned source are included, so this works from a fresh Windows or Linux checkout
without installing another assembler. Decimal-mode CPU arithmetic is intentionally
not implemented. See [the serial/BASIC guide](docs/Serial-and-BASIC.md) for controls,
examples, firmware provenance, modeling limits, and hardware validation details.

## Breadboard window

The window shows the complete breadboard computer with chips, power rails,
wiring, clock hardware, and the live LCD. Click its **RESET**, **IRQ**, and
**NMI** buttons or use **R**, **I**, and **N**. The bottom strip shows live CPU
registers, VIA ports, IRQ state, and the configured clock frequency.

See [the breadboard view guide](docs/Breadboard-view.md) for controls and how
the illustrated hardware relates to the existing emulator.

## W65C22 VIA

The VIA implements all 16 registers, both parallel ports, input latches,
CA/CB handshakes and interrupts, both timers, PB7 waveform output, PB6 pulse
counting, and all eight shift-register modes. Timers use emulated PHI2 cycles,
and VIA IRQ feeds the existing CPU alongside the manual interrupt input.
Pressing **R** resets both chips.

See [the VIA model documentation](docs/W65C22.md) for pin APIs, register side
effects, timing conventions, variant differences, and test coverage.

## Run the VIA demonstration

In the IDE, select `via_demo.asm` and click **Build & Run**. The ROM checks GPIO direction/readback, interrupt enable/masking,
Timer 1's PB7 output, Timer 2 timed and PB6 pulse-count modes, and serial output.
It then displays `VIA OK   SR:A5` with live hexadecimal `T1` and `T2` interrupt
counters on the second line. The CPU uses `WAI` between display updates.
`IRQ WAIT` remains visible until both timer interrupt handlers have run;
a startup failure displays `VIA FAIL CODE:xx`.

The IDE clock control (initially `cpuMHz` in `SystemLib/main.cpp`) determines the speed:
at **0.001 MHz (1 kHz)**, T1 increments about every 4.1 seconds and T2 about
every 8.2 seconds. Startup and LCD writes also run slowly at that setting.
At **1 MHz**, the same timer periods are about 4.1 and 8.2 milliseconds.
See [the demo guide](docs/VIA-demo.md) for failure codes, coverage, and checks.

## HD44780U LCD

The 16x2 display models an HD44780U-A00 with 5x8-dot character cells. It supports
4-bit and 8-bit bus transfers, busy/address reads, DDRAM/CGRAM reads and writes,
custom characters, scrolling, cursor, and blinking. Instruction timing and blink
use emulated time, including while the CPU executes `WAI` or `STP`.

The LCD is busy for 10 ms at power-on. Clear/home take 1.52 ms; ordinary
operations take 37 microseconds at the nominal 270 kHz LCD oscillator.
Both Hello World examples poll the busy flag. **R** resets the CPU and VIA;
the LCD remains powered and its memory is retained until firmware clears it.

See [the LCD model documentation](docs/HD44780U.md) for the datasheet,
corrections, bus API, and modeling limits.

## Run the tests

From the repository root, copy the fixtures into the build directory so test
output does not overwrite the checked-in reference logs:

```sh
mkdir -p build/test-fixtures
cp -a SystemTest/EmulationOutFiles SystemTest/65C02LogFiles SystemTest/EmulationLogFiles build/test-fixtures/
cd build/test-fixtures
../SystemTest/SystemTest
```

[Original Linux build video](https://youtu.be/6M1S0CATJAM)

### Windows 11 application

The portable release is `dist/W65C02-Studio-Windows-11-x64.zip`. Extract the
whole folder and launch `W65C02-Studio.exe`; no CLion or compiler is needed.
See [Windows packaging and validation](packaging/BUILD.md) for rebuilding.
