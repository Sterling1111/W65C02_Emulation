# Breadboard view

The IDE's **Breadboard** tab presents the complete breadboard computer, based on the
supplied hardware photo. It includes three breadboards, terminal holes and
power rails, the W65C02 CPU, W65C22 VIA, AT28C256 EEPROM, HM62256 SRAM,
74HC00 decode logic, oscillator cans, reset/interrupt buttons, capacitors,
resistors, power indicator, wire bundles, and the working 16x2 LCD.

Blue wires represent the address bus, teal the data connections, yellow the
control signals, red power, and dark wires ground. The layout is an
illustration of the board's components and connections, not a pin-accurate
netlist or an editable electrical simulator. The decoder, passive components,
power indicator, and spare oscillator are visual details; the existing `Bus`
address map and single `W65C02` CPU continue to define emulation behavior.
The 20 MHz spare oscillator does not supply the emulated clock.

## Controls and state

- Click **RESET**, or press **R**, to reset the existing CPU and VIA.
- Hold **IRQ**, or **I**, to assert the CPU's manual IRQ input.
- Press **NMI**, or **N**, to trigger its NMI input.
- Mouse release and loss of window focus release the manual interrupt inputs.
  Keyboard and mouse can both hold the same input; it releases when both let go.

The IDE debugger toolbar provides Step (F10), Continue/Pause (F8), and
Restart paused (F9). The board distinguishes PAUSED from the STP halt state.

The lower strip shows the CPU's PC/A/X/Y, VIA port A/B levels, IRQ line state,
execution state, and configured frequency. These are snapshots taken under
the existing CPU mutex. Port inspection uses non-acknowledging pin reads;
viewing the board does not clear VIA interrupt flags or clock the peripherals.

The active oscillator label follows the CPU clock, initially `cpuMHz` in
`SystemLib/main.cpp` and adjustable with the IDE's CLOCK button. **RUNNING**
and the displayed frequency describe CPU state and its requested clock; the
frequency label is not a measurement of achieved host performance.

## Rendering

`BreadboardView` composes the static board, a placed/scaled `LcdPanel`, a static
LCD wire overlay, and the small changing state readout. Artwork and wires are
cached in render textures. Only LCD dots, state text, and pressed-button
appearance are redrawn dynamically. The LCD pixel data still comes from the
HD44780U model; the drawing does not synthesize program output or memory.

The IDE fits the desktop at startup and preserves its aspect ratio when resized.
The board is fitted inside its tab. Mouse hit testing maps back into the same
logical coordinates.
CMake copies SFML's bundled Sansation font into `build/SystemLib/assets`, so
chip labels need no system font installation. The executable uses the font
path from its configured build directory.
