# Ben Eater WozMon, BIOS and Microsoft BASIC

`roms/wozmon-basic.bin` is the 32 KiB ROM built **without source changes** from
[Ben Eater's msbasic repository](https://github.com/beneater/msbasic) at commit
`c21542e724b3da45ba3790405c2cf85e77bc1ad4`. The pinned source is in `msbasic/`.
It includes Ben's IRQ-buffered serial BIOS and VIA PA0 flow control, WozMon at
`$FE00`, and Microsoft BASIC's cold-start entry at `$8000`.

At reset the ROM enters WozMon. `8000R` enters BASIC; press Enter at the memory
size question to let BASIC detect RAM, then enter `80` at the terminal-width
question. The **Boot BASIC** button answers both prompts automatically. The emulator provides 16 KiB of RAM.
The serial setup is 19200 baud, 8 data bits, no parity, one stop bit. Use a
1 MHz CPU clock for the firmware's fixed transmit-delay loops.

The app includes the ROM, so a normal C++ build on Windows or Linux requires
no additional assembly tools or network downloads for this firmware.
To reproduce it, install the [cc65 toolchain](https://cc65.github.io/) and run:

```sh
python3 firmware/build.py
```

`--ca65`, `--ld65`, and `--include-dir` support an unpacked toolchain.
The build script prints a SHA-256 digest. The bundled ROM's SHA-256 is
`05f08c51e08a0427a5ab1d6af250753e3041c9e5dc68b846745929e9aa14f7ac`.
 The vendored upstream README credits
Michael Steil, Martin Hoffmann-Vetter, Bob Sander-Cederlof, Tom Greene, and Joe
Zbicak and specifies the **2-clause BSD license** for the source tree. Ben Eater's
adaptation and videos are credited in that README. WozMon originated with Steve
Wozniak. Preserve these credits with redistributed source and firmware.

This is the revision from Ben's **RS232 flow control** video. It uses the serial
expansion without requiring the later video's LCD rewiring or sound hardware.
Decimal-mode CPU arithmetic remains intentionally outside this project's scope.
