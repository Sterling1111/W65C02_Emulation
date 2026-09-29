# Windows 11 portable release

The release is an x86-64 portable ZIP. Extract the complete folder and run
`W65C02-Studio.exe`. Assets and tools resolve relative to the executable;
assembly sources live in `programs/` and generated ROMs in `builds/`. Launching
from a shortcut or another working directory works. Use a writable folder.

## Build

Use MinGW-w64 GCC 13.1 (the toolchain bundled with CLion was used for this
release) and CMake. Run from a Windows developer shell with the compiler and
CMake on PATH. Build on a local Windows drive: MinGW binutils cannot reliably
write object files or link static archives through a WSL UNC share.

```powershell
cmake -S . -B build-windows -G "MinGW Makefiles" -DCMAKE_BUILD_TYPE=Release -DW65C02_PORTABLE=ON -DW65C02_BUILD_TESTS=OFF -DW65C02_BUILD_BENCHMARKS=OFF
cmake --build build-windows --target main -j 4
```

By default the project uses the checked-in SFML 2.5.1 static MinGW libraries.
`-DSFML_DIR=...` can select another compatible SFML 2 distribution. Tests and
benchmarks remain enabled by default for ordinary development builds.

The release assembler was built from the unmodified official archive:
http://sun.hasenbraten.de/vasm/release/vasm.tar.gz

Source archive SHA-256 for this release:
`c84b2de1cbb87831795fe64a85c5d9a7002a766e3a7c30b0a2d7d5e99d878f49`.
Extract it, then run in its `vasm` directory:

```powershell
mingw32-make CPU=6502 SYNTAX=oldstyle TARGETEXTENSION=.exe "CFLAGS=-c -O2 -std=c90 -DOUTBIN" "LDFLAGS=-static -lm" vasm6502_oldstyle.exe -j4
```

No source edits are needed. This builds binary output and the 6502/oldstyle
backend without a Cygwin runtime. Keep the original archive for packaging.

```powershell
python packaging/package_windows.py --build-dir build-windows --assembler C:/path/to/vasm/vasm6502_oldstyle.exe --vasm-source C:/path/to/vasm.tar.gz --mingw-dir C:/path/to/mingw
```

The script produces a ZIP and SHA-256 file in `dist/`. The archive includes
per-file hashes, examples, firmware sources/provenance, and dependency notices.

## Native window styling

`SystemLib/WindowTheme.cpp` uses the documented Windows 11
[DWM window attributes](https://learn.microsoft.com/windows/win32/api/dwmapi/ne-dwmapi-dwmwindowattribute)
for a dark caption, light text, teal active border, and rounded corners. The
system still owns the caption buttons, resizing, snapping and system menu.
The embedded manifest requests ordinary user privileges and DPI awareness.
Linux retains its window manager's decorations and the CPU icon.

## Release validation

The packaged executable was extracted into a Windows folder containing spaces
and launched with `C:\Windows` as its working directory and only Windows system
folders on PATH. On Windows 11 (build 26200.9457), it successfully:

- applied the dark title bar, CPU icon and teal border (visually checked);
- minimized, maximized, restored and closed through native window controls;
- assembled `hello_world.asm` using the packaged assembler and emitted a 32 KiB
  ROM plus the source listing used by the debugger;
- booted the bundled WozMon/Microsoft BASIC ROM and evaluated `PRINT 2+2` as `4`.

PE import inspection found only Windows system DLL dependencies in both the
app and the assembler. No separate SFML, GCC, Cygwin, or Visual C++ runtime DLLs
are needed. This release is unsigned.

The optional `packaging/smoke_windows.ps1` script reproduces the native UI check
against an extracted package. It opens and closes its own app instance, writes
build outputs in that package, and restores the clipboard after checking serial
output. Run it in an interactive Windows desktop session:

```powershell
powershell -NoProfile -STA -File packaging/smoke_windows.ps1 -Package C:/path/to/W65C02-Studio-Windows-11-x64 -Results C:/path/to/existing/results-folder
```

The Linux regression suite also passed all 444 cases, including all 13 original
physical-board trace comparisons. Captures and CPU implementation were unchanged.
