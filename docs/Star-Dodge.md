# Star Dodge

A two-lane arcade game written entirely in W65C02 assembly for this board's
16×2 LCD. Steer the little spaceship toward stars and away from asteroids.
Each star scores one point. The course gets faster every five points, down to
one movement every 150 ms. The upper-right three digits show your score; the
lower-right three show your best score. Both stop at 999.

## Play in the IDE

1. Click **Refresh**, then select **star_dodge.asm** in the program list.
2. Click **CLOCK** until it reads **1 MHz**. From the default 1 kHz, that is four clicks.
3. Choose **Build & Run (F5)**. The Breadboard tab displays the title.
4. Tap **N**, or click the board's **NMI** button, to start. Tap again to switch lanes.
5. After a crash, tap **N** to retry. Your best score lasts until reset or a fresh build.

Keep the Breadboard tab active for keyboard controls. **R** resets everything,
including your best score. **F8** pauses/continues. IRQ / I is not a game control.
Changing the CPU clock changes game speed; 1 MHz is the intended rate.

Every wave has a star in one lane and an asteroid in the other. Two empty
columns separate waves, so switching lanes always has room. The ship occupies
the leftmost column. Collecting a star removes that wave after it reaches you.

## Build and verify from the terminal

From the repository root:

```sh
./VASM/vasm6502_oldstyle -Fbin -dotdir -wdc02 -o build/star_dodge.bin VASM/star_dodge.asm
cmake -S . -B build
cmake --build build --target StarDodgeSmoke -j2
./build/SystemTest/StarDodgeSmoke build/star_dodge.bin
```

The integration check executes the actual assembled firmware on the emulator.
It checks LCD pixels (including custom sprites), start/lane controls, both
hazard lanes, collecting 20 stars, decimal carry, increasing speed, collision,
retry and best-score retention, input counter rollover, score saturation, and
reset. It runs without a window or wall-clock delays.

## Firmware details

VIA Timer 1 generates a 20 Hz IRQ at 1 MHz. A movement consumes six timer ticks
initially, decreasing to three. Between events the CPU uses WAI; there is no
software frame-delay loop. LCD transfers poll the controller's busy flag.
Only the foreground accesses the LCD; IRQ acknowledges Timer 1 and counts
ticks, while NMI queues button presses. A small LFSR supplies wave placement;
the time spent at the title screen affects its starting state.

The firmware uses W65C02 instructions such as WAI, STZ, and BRA. It runs on the
existing emulator hardware with no host-side game logic or extra peripherals.
The source includes named RAM locations and comments for debugging. Useful
breakpoint locations include `switch_lane`, `move_course`, `add_point`, and
`crash` (click the first instruction below the label).

The first 64 bytes of RAM belong to the game. `$00` is the mode (title=0,
playing=1, crashed=2), `$01` is the lane, `$02`–`$04` are score digits and
`$05`–`$07` are best-score digits, ones first. Course cells occupy `$20`–`$2B`
and `$30`–`$3B`. `$11` counts completed gameplay redraws.
