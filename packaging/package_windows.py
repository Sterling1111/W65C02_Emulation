#!/usr/bin/env python3
"""Bundle a portable MinGW release; run after building main with W65C02_PORTABLE=ON."""
import argparse
import hashlib
import json
from pathlib import Path
import shutil
import tempfile
import zipfile

ROOT = Path(__file__).resolve().parents[1]
NAME = 'W65C02-Studio-Windows-11-x64'


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument('--build-dir', type=Path, required=True)
    parser.add_argument('--assembler', type=Path, required=True,
                        help='Native, statically linked vasm6502_oldstyle.exe')
    parser.add_argument('--vasm-source', type=Path, required=True,
                        help='Unmodified upstream vasm.tar.gz corresponding to the assembler')
    parser.add_argument('--mingw-dir', type=Path, required=True,
                        help='MinGW root containing licenses/')
    parser.add_argument('--output-dir', type=Path, default=ROOT / 'dist')
    args = parser.parse_args()
    cache = (args.build_dir / 'CMakeCache.txt').read_text()
    if 'W65C02_PORTABLE:BOOL=ON' not in cache:
        parser.error('Configure with -DW65C02_PORTABLE=ON before packaging')
    args.output_dir.mkdir(parents=True, exist_ok=True)
    with tempfile.TemporaryDirectory(prefix='w65c02-package-') as temporary:
        dest = Path(temporary) / NAME
        dest.mkdir()

        def copy(source, relative):
            target = dest / relative
            target.parent.mkdir(parents=True, exist_ok=True)
            shutil.copy2(source, target)

        copy(args.build_dir / 'SystemLib/W65C02-Studio.exe', 'W65C02-Studio.exe')
        for name in ['sansation.ttf', 'DejaVuSansMono.ttf', 'wozmon-basic.bin']:
            copy(args.build_dir / 'SystemLib/assets' / name, 'assets/' + name)
        copy(ROOT / 'assets/icons/cpu.ico', 'assets/cpu.ico')
        copy(args.assembler, 'tools/vasm6502_oldstyle.exe')
        copy(args.vasm_source, 'third-party/vasm-source.tar.gz')
        for source in sorted((ROOT / 'VASM').glob('*.asm')):
            copy(source, 'programs/' + source.name)
        shutil.copytree(ROOT / 'examples', dest / 'examples')
        shutil.copytree(ROOT / 'docs', dest / 'docs')
        shutil.copytree(ROOT / 'firmware', dest / 'firmware',
                        ignore=shutil.ignore_patterns('__pycache__', '*.pyc'))
        shutil.copytree(ROOT / 'packaging/licenses', dest / 'licenses')
        copy(ROOT / 'SFML/license.md', 'licenses/SFML.md')
        copy(ROOT / 'assets/fonts/LICENSE-DejaVu.txt', 'licenses/DejaVu.txt')
        copy(ROOT / 'assets/fonts/LICENSE-Sansation.txt', 'licenses/Sansation.txt')
        for name in ['gcc/COPYING3', 'gcc/COPYING.RUNTIME',
                     'crt/COPYING.MinGW-w64.txt', 'crt/COPYING.MinGW-w64-runtime.txt',
                     'winpthreads/COPYING', 'winpthreads/COPYING.winpthreads']:
            copy(args.mingw_dir / 'licenses' / name, 'licenses/MinGW/' + name)
        copy(ROOT / 'packaging/README-Windows.txt', 'READ-ME-FIRST.txt')
        copy(ROOT / 'packaging/THIRD-PARTY.md', 'THIRD-PARTY.md')
        manifest = {p.relative_to(dest).as_posix(): hashlib.sha256(p.read_bytes()).hexdigest()
                    for p in sorted(dest.rglob('*')) if p.is_file()}
        (dest / 'SHA256SUMS.json').write_text(json.dumps(manifest, indent=2) + '\n')
        archive = args.output_dir / (NAME + '.zip')
        with zipfile.ZipFile(archive, 'w', zipfile.ZIP_DEFLATED, compresslevel=9) as output:
            for source in sorted(dest.rglob('*')):
                if source.is_file():
                    output.write(source, source.relative_to(dest.parent).as_posix())
    digest = hashlib.sha256(archive.read_bytes()).hexdigest()
    archive.with_suffix('.zip.sha256').write_text(digest + '  ' + archive.name + '\n')
    print(archive)
    print('SHA256:', digest)
    print('Size:', archive.stat().st_size, 'bytes')


if __name__ == '__main__':
    main()
