# SPDX-License-Identifier: GPL-3.0-only
"""Build either DLL from source. No game installation or base DLL is needed."""
from pathlib import Path
import argparse
import ctypes
import hashlib
import json
import os
import struct
import subprocess
import sys

SOURCE = Path(__file__).resolve().parent
ROOT = SOURCE.parent
EXPECTED = {
    'fireball': '4d61fb3aa3d94b74fe0be01d2a7d22ec48aa8cbeb9865abe8f58bb2a3bc961cc',
    'companion': 'bb9ff6c42b513d658ff2dc8a8cf8195b4202c5ee2badec5ae17d89016ea610c9',
}


def toolchain(explicit=None):
    if explicit:
        candidate = Path(explicit).resolve()
    else:
        program_files = Path(os.environ.get('ProgramFiles(x86)', 'C:/Program Files (x86)'))
        vswhere = program_files / 'Microsoft Visual Studio/Installer/vswhere.exe'
        if not vswhere.exists():
            raise SystemExit('Install Visual Studio Build Tools with x64 C++ tools, or pass --msvc-bin.')
        installs = json.loads(subprocess.check_output([
            str(vswhere), '-all', '-prerelease', '-products', '*', '-format', 'json', '-utf8',
        ], encoding='utf-8-sig'))
        available = []
        for install in installs:
            tools = Path(install['installationPath']) / 'VC/Tools/MSVC'
            if not tools.is_dir():
                continue
            for version in tools.iterdir():
                binary = version / 'bin/Hostx64/x64'
                if all((binary / name).is_file() for name in ['cl.exe', 'lib.exe', 'link.exe']):
                    available.append((tuple(int(v) for v in version.name.split('.')), binary))
        if not available:
            raise SystemExit('Visual Studio x64 C++ tools were not found; use --msvc-bin.')
        candidate = max(available, key=lambda item: item[0])[1]
    for name in ['cl.exe', 'lib.exe', 'link.exe']:
        if not (candidate / name).is_file():
            raise SystemExit(f'Missing MSVC tool: {candidate / name}')
    return candidate


def run(arguments):
    result = subprocess.run(arguments, stdout=subprocess.PIPE, stderr=subprocess.STDOUT)
    # Localized cl.exe output is normally encoded in the Windows ANSI code page.
    try:
        message = result.stdout.decode('utf-8')
    except UnicodeDecodeError:
        message = result.stdout.decode('mbcs', errors='replace')
    print(message, end='')
    result.check_returncode()


def normalize_timestamps(path, timestamp):
    """Normalize only COFF and IMAGE_DEBUG_DIRECTORY timestamps, never code/data."""
    data = bytearray(path.read_bytes())
    pe = struct.unpack_from('<I', data, 0x3c)[0]
    if data[:2] != b'MZ' or data[pe:pe + 4] != b'PE\0\0':
        raise ValueError('Not a PE image')
    optional = pe + 24
    if struct.unpack_from('<H', data, optional)[0] != 0x20b:
        raise ValueError('An x64 PE32+ build is required')
    count = struct.unpack_from('<H', data, pe + 6)[0]
    optional_size = struct.unpack_from('<H', data, pe + 20)[0]
    sections = optional + optional_size

    def offset(rva):
        for i in range(count):
            header = sections + i * 40
            virtual_size, virtual, raw_size, raw = struct.unpack_from('<IIII', data, header + 8)
            if virtual <= rva < virtual + max(virtual_size, raw_size):
                return raw + rva - virtual
        raise ValueError(f'RVA outside sections: {rva:#x}')

    struct.pack_into('<I', data, pe + 8, timestamp)
    debug_rva, debug_size = struct.unpack_from('<II', data, optional + 112 + 6 * 8)
    if debug_rva:
        start = offset(debug_rva)
        for i in range(debug_size // 28):
            struct.pack_into('<I', data, start + i * 28 + 4, timestamp)
    path.write_bytes(data)


def build(which, vc, output, tests=False):
    dest = output / (which + ('-test' if tests else '-release'))
    dest.mkdir(parents=True, exist_ok=True)
    libs = []
    for name in ['kernel32', 'ntdll']:
        lib = dest / (name + '.lib')
        run([str(vc / 'lib.exe'), '/nologo', '/def:' + str(SOURCE / (name + '.def')),
             '/machine:x64', '/out:' + str(lib)])
        libs.append(str(lib))
    if which == 'fireball':
        files = ['ElementalVisuals.c', 'MissileEmbers.c']
        definitions = 'ElementalVisuals.def'
        # Keep this internal PE name: the accepted binary was renamed after linking.
        internal_name = 'ElementalVisuals.dll'
        published_name = 'ElementalVisuals-v0.23-HolyFire-3x.dll'
        timestamp = 0x6ae644a0
        flags = ['/DAEV_TEST'] if tests else []
        checks = ['runEmberTests']
    else:
        files = ['ElementalCompanion.c']
        definitions = 'ElementalCompanion.def'
        internal_name = 'AEV_EarthStorm-test.dll' if tests else 'AEV_EarthStorm-v4.dll'
        published_name = internal_name
        timestamp = 0x6ae70d59
        flags = ['/W4', '/wd4152'] + (['/DAEV_EARTH_TEST'] if tests else [])
        checks = ['runEarthTests', 'runGroundTests', 'runStormTests', 'runRockTests', 'runMissileFlameTests']
    target = dest / internal_name
    run([str(vc / 'cl.exe'), '/nologo', *flags, '/LD', '/O2', '/GS-', '/Oi', '/Zl',
         *(str(SOURCE / name) for name in files), '/Fo' + str(dest) + '/', '/link',
         '/NODEFAULTLIB', '/ENTRY:DllMain', '/MACHINE:X64', '/DYNAMICBASE', '/NXCOMPAT',
         '/DEF:' + str(SOURCE / definitions), '/IMPLIB:' + str(dest / 'addon.lib'),
         '/OUT:' + str(target), *libs])
    normalize_timestamps(target, timestamp)
    if published_name != internal_name:
        (dest / published_name).write_bytes(target.read_bytes())
    digest = hashlib.sha256(target.read_bytes()).hexdigest()
    if tests:
        library = ctypes.WinDLL(str(target))
        for symbol in checks:
            check = getattr(library, symbol)
            check.restype = ctypes.c_int
            result = check()
            if result:
                raise SystemExit(f'FAIL {symbol}: case {result}')
            print('PASS', symbol)
    print(f'BUILT {which}: {dest / published_name}\nSHA256 {digest}')
    return {'component': which, 'tests': tests, 'path': str(dest / published_name),
            'sha256': digest, 'matches_reviewed_release': not tests and digest == EXPECTED[which]}


def main():
    if hasattr(sys.stdout, 'reconfigure'):
        sys.stdout.reconfigure(encoding='utf-8', errors='replace')
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument('--component', choices=['all', 'fireball', 'companion'], default='all')
    parser.add_argument('--test', action='store_true', help='Run isolated lifecycle tests; never starts Kenshi.')
    parser.add_argument('--msvc-bin', help='MSVC bin/Hostx64/x64 directory; otherwise use vswhere.')
    parser.add_argument('--output', type=Path, default=ROOT / 'build/from-source')
    parser.add_argument('--require-reviewed-hashes', action='store_true')
    args = parser.parse_args()
    if args.test and args.require_reviewed_hashes:
        parser.error('Test DLLs deliberately differ from release DLLs.')
    vc = toolchain(args.msvc_bin)
    print('MSVC:', vc)
    components = ['fireball', 'companion'] if args.component == 'all' else [args.component]
    report = [build(name, vc, args.output.resolve(), args.test) for name in components]
    args.output.mkdir(parents=True, exist_ok=True)
    (args.output / ('test-results.json' if args.test else 'build-results.json')).write_text(
        json.dumps(report, indent=2) + '\n', encoding='utf-8')
    if args.require_reviewed_hashes and not all(item['matches_reviewed_release'] for item in report):
        raise SystemExit('Built successfully, but the reviewed hash differs. Check the documented MSVC version.')


if __name__ == '__main__':
    main()
