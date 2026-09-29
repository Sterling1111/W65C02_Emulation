#!/usr/bin/env python3
"""Rebuild the pinned Ben Eater WozMon/BIOS/MSBASIC ROM with cc65."""
import argparse
import hashlib
from pathlib import Path
import subprocess
import tempfile

root = Path(__file__).resolve().parent
parser = argparse.ArgumentParser(description=__doc__)
parser.add_argument('--ca65', default='ca65')
parser.add_argument('--ld65', default='ld65')
parser.add_argument('--include-dir', help='cc65 asminc directory, for unpacked toolchains')
parser.add_argument('--output', type=Path, default=root / 'roms' / 'wozmon-basic.bin')
args = parser.parse_args()
output = args.output.resolve()
output.parent.mkdir(parents=True, exist_ok=True)
with tempfile.TemporaryDirectory(prefix='w65c02-firmware-') as temporary:
    obj = str(Path(temporary) / 'eater.o')
    command = [args.ca65, '-D', 'eater', 'msbasic.s', '-o', obj]
    if args.include_dir:
        command += ['-I', args.include_dir]
    subprocess.run(command, cwd=root / 'msbasic', check=True)
    subprocess.run([args.ld65, '-C', 'eater.cfg', obj, '-o', str(output)],
                   cwd=root / 'msbasic', check=True)
rom = output.read_bytes()
if len(rom) != 32768:
    raise SystemExit('Expected a 32768-byte ROM')
print(f'{output}\nSHA256 {hashlib.sha256(rom).hexdigest()}')
