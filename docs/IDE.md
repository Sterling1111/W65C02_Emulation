# W65C02 Studio

A small desktop IDE for this emulator. It uses the existing SFML window and
single W65C02 execution worker. The Editor and Breadboard tabs share the same
application; assembly runs on a background task so builds do not freeze input
or the currently running program.

## Start and run a program

```sh
cmake -S . -B build
cmake --build build --target main -j2
./build/SystemLib/main
```

Select an `.asm` program on the left. **Build & Run / F5** saves the selected
buffer, assembles a snapshot with vasm's `-Fbin -dotdir -wdc02` options, validates
the resulting ROM, and starts it on the Breadboard tab. **Build / F6** saves
and assembles but leaves the currently loaded/running program alone.

The build panel shows assembler diagnostics. The editor jumps to the first
reported error line and marks it red. A failed build never loads an older
binary as though it were the new result. ROMs must cover 8000–FFFF (32768 bytes)
and contain a reset vector into ROM. Builds time out after ten seconds.

A successful **Build & Run** starts a fresh program: RAM is initialized, the LCD
is powered on, and the CPU and VIA reset. The physical/keyboard **Reset** control
resets only the CPU and VIA, as before, and retains LCD power and RAM. **Stop**
stops the CPU worker. **R** in the Board tab restarts the loaded ROM by reset.
The CPU keeps running while you edit, unless explicitly stopped.

The **CLOCK** button cycles 1 kHz, 10 kHz, 100 kHz, 500 kHz, 1 MHz, 2 MHz, 10 MHz,
and 50 MHz. It changes the requested runtime rate without resetting the program.
The startup default remains `cpuMHz` in `SystemLib/main.cpp`, using `KHZ`/`MHZ`.
Firmware delays scale with this clock; high requested rates depend on host
performance. The setting is session-only, not written back to main.cpp.

## Step through a program

1. Select your program and choose **Build & Debug (F7)**. It saves and assembles
   the source, loads the ROM, and pauses at the reset vector before executing
   any instruction.
2. Click **Step (F10)** to execute one CPU instruction. The CPU remains paused
   afterward. The amber source line and gutter arrow indicate the next instruction; the
   register strip shows PC, A, X, Y, SP, and processor flags P in hexadecimal.
   The status bar reports the executed instruction and its cycle count.
3. Use **Continue / Pause (F8)** to resume from the current registers and memory
   or pause a running program. **Restart paused (F9)** resets the CPU and VIA
   and leaves the CPU at the reset vector. RAM and LCD contents are retained by
   this reset; Build & Debug starts a fresh board like Build & Run.

Step works in either tab and on the currently loaded ROM. If the CPU is
running, Step stops its worker first, executes one instruction, and leaves it
paused. JSR enters the subroutine; RTS and branches follow their normal targets.
Holding F10 does not auto-repeat. The board's PAUSED state is distinct from a
CPU halted by STP.

Stepping, pausing, and restarting paused preserve the current tab. They update
the loaded program's source position in the background, so opening the Editor
shows the highlighted instruction in view. Breakpoint hits open the Editor.
The header shows the address and line number. The arrow remains on WAI while waiting and on STP
while halted, labeled WAITING AT or HALTED AT, rather than moving to an
instruction that cannot execute yet.

Source positions come from vasm's listing for the exact loaded build. The
highlight is shown only when the selected main source buffer still matches
that build. Editing it hides the marker until it matches again or is rebuilt.
Included-file and dynamically generated RAM code can still be stepped by
address, but are not mapped into this editor. Step-over/step-out commands are not implemented.

The VIA and LCD advance by the cycles consumed by each instruction. Paused
wall-clock time does not advance the board. If WAI is waiting without an
interrupt, Step advances one idle PHI2 clock and reports that no instruction
ran; Continue lets peripheral clocks run normally. With an interrupt pending,
the CPU's interrupt entry and one handler instruction occur in that step.
STP requires Reset or Restart paused; additional Step presses do not execute
instructions or advance clocks while STP is halted.

## Breakpoints

Click the line-number gutter beside an instruction to toggle a breakpoint, or
press **Ctrl+B** on the current line. **Ctrl+Shift+B** clears all breakpoints
in the selected file. Markers remain associated with that file while switching
programs and tabs during this session; they are not saved across app restarts.

A **solid red dot** means the breakpoint is bound to the loaded ROM. A **hollow
red dot** is pending: build and load the matching source with **F5** or **F7**.
Choose an instruction or its label. A label-only line binds to the next
encoded line, across adjacent labels, comments, and blank lines. Binding does
not cross an intervening directive or include. Blank/comment-only lines remain
pending. Breakpoints use the exact addresses from vasm's
listing, including each emitted address for a source line.

Execution pauses before the instruction, switches to the Editor, and highlights
the next instruction. **F10** executes it and stays paused. **F8** continues past
that occurrence of the breakpoint; a loop will stop there again on its next
visit. Breakpoints also work at interrupt-handler entry, after interrupt entry
has completed and before the first handler opcode. Peripheral clocks freeze
while paused. Restart/reset rearms all breakpoints, including the first opcode.

Editing the loaded source makes its breakpoints pending and disables their old
addresses until the buffer matches again or you build and load it. Markers are
attached to line numbers; after inserting or deleting lines, review their
positions before rebuilding. Other files' breakpoints become active when those
programs are loaded. Building with F6 alone does not load a new ROM.

## Editing

| Control | Action |
| --- | --- |
| Click a program | Open it; other open buffers retain their edits. |
| + or Ctrl+N | Create a named `.asm` program with a reset/vector template. |
| Ctrl+S / Save | Save the selected source to the program directory. |
| Ctrl+Z / Ctrl+Y / Ctrl+Shift+Z | Undo / redo. |
| Ctrl+A / C / X / V | Select all / copy / cut / paste. |
| Shift+arrows / mouse drag | Select source text. |
| Ctrl+Left / Right | Move by word. |
| Home / End; Ctrl+Home / End | Line start/end; document start/end. |
| Page Up / Down; mouse wheel | Navigate or scroll vertically. |
| Shift+wheel / horizontal wheel | Scroll long lines horizontally. |
| Tab / Shift+Tab | Indent / outdent (four spaces), including selected lines. |
| Enter | New line, retaining the current indentation. |
| Ctrl+F; Enter; Esc | Find text; next match with wrapping; close search. |
| Ctrl+G | Go to a line. |
| Gutter click / Ctrl+B | Toggle a breakpoint. |
| Ctrl+Shift+B | Clear breakpoints in the selected file. |
| Ctrl+Tab | Switch Editor / Breadboard. |
| Refresh | Rescan the list of files in the workspace. |
| Reload | Confirm replacement of the selected buffer with its disk contents. |

The editor is intended for small ASCII assembly sources. Text input and paste
support ASCII; tabs and CRLF/CR line endings are normalized to spaces/LF.
Buffers are limited to 1 MiB. Undo retains up to 100 snapshots within an 8 MiB
history budget per file. Syntax highlighting is visual; vasm remains the
syntax authority. Find is case-sensitive. Basic instruction debugging is described above.

A dot beside a filename marks an unsaved buffer. Closing with unsaved buffers
offers **Save all & close**, **Discard**, or **Cancel**. Save writes a temporary
file and replaces the destination only after the write succeeds. If a file
was changed or removed outside the IDE, saving reports a conflict instead of
overwriting it. Copy needed edits before choosing Reload in that case.

## Program and build locations

The default program directory is the checkout's `VASM/`, independent of the
launch working directory. Only regular `.asm` files with simple names appear;
new names use letters, digits, underscores, hyphens, and dots. Symlinks and
paths outside that directory are not editable through the picker.

Each build uses a distinct directory under `build/ide/`, retaining its exact
source snapshot, assembler log/listing, and generated ROM. Relative `.include` files
are resolved against the program directory. Included files are not separate
editor tabs in this simple version. These generated build directories may be
deleted when no build is using them.

For a separate workspace or automated testing:

```sh
./build/SystemLib/main --programs /path/to/programs --build-dir /path/to/builds
```

The legacy `a.out` workflow still works: if a valid `a.out` exists in the launch
directory, it is loaded and can be started with **R** in the Breadboard tab.
Build & Run does not overwrite that file; it uses its own generated ROM.

The bundled assembler is selected for Windows or Linux at configure time.
Linux's bundled executable is x86-64. Source mapping supports both the legacy listing from the bundled Windows
vasm 1.8g and the newer Linux listing. Earlier revisions parsed only the newer
format, so Windows builds could assemble successfully while breakpoints stayed
pending and the execution highlight was missing. Rebuild the `main` target after
updating the source. Full window validation has been performed on Linux/WSL. Label fonts come from the bundled SFML resources; the monospace
font and its license are in `assets/fonts/`. No system font or new GUI toolkit
installation is needed. Source/build/asset paths are set by CMake for this
checkout; this is not a standalone relocatable installer.

## Verification

`SystemTest/IdeTests.cpp` covers editing/undo, selection, indentation, line
normalization, workspace listing and saves, external-file conflicts, path
boundaries, real vasm builds, workspace includes, error-line reporting,
stale-build separation, ROM validation and execution, and the new template.
Instruction-debugging tests also check paused reset, exact step boundaries,
subroutine/branch behavior, preserved state on Continue, WAI/IRQ handling, and
peripheral clocks. Breakpoint tests cover stopping before side effects, repeated
loop hits, reset rearming, stepping/continuing, live changes, IRQ entry, and
frozen clocks. The full regression suite includes these tests. Window checks also exercise
editing/saving, both tabs, build/run with live LCD output, failed-build display,
new files, and the unsaved-change dialog using temporary source copies.
