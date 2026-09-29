W65C02 STUDIO - WINDOWS 11 (64-BIT)

1. Extract the entire ZIP to a folder you can write to, such as Documents.
2. Open that folder and double-click W65C02-Studio.exe.

Keep the assets, tools, and programs folders beside the executable.
No CLion, compiler, Python, Cygwin, or separate firmware download is needed.
You can move the complete folder or create a shortcut to the executable.
This is a portable app; there is no installer or uninstaller.

The Windows 11 title bar has a dark background, teal active border, CPU icon,
and native minimize, maximize/restore, close, resize, and snap controls.

ASSEMBLY
Select hello_world.asm and click Build & Run (F5).
F7 builds with debugging; click the editor gutter to set a breakpoint.
F8 pauses/continues and F10 steps. Your sources are saved in programs/.
Generated ROMs and assembler listings are written to builds/.

BASIC AND WOZMON
Open Terminal, then click Boot BASIC or Boot WozMon.
Wait for BASIC's OK prompt, then type PRINT 2+2 and press Enter.
Paste examples/basic/lcd.bas into BASIC to write to the LCD; switch to
Breadboard after OK appears to view the result. PRINT normally uses serial.
See docs/Serial-and-BASIC.md and docs/IDE.md for the full instructions.

BACKUP
Keep a copy of programs/ before replacing the app with a newer version.
BASIC LOAD/SAVE are not implemented; keep BASIC programs in text files.

This build is not code-signed. See THIRD-PARTY.md for bundled components.
