# Building Arcane Elemental Visuals

Complete editable source for both shipped DLLs is in this directory.
No base DLL or binary patch input is required. No game process is started.

Requirements: Windows x64, Python 3.10+ x64, MSVC C++ x64 Build Tools.
Reviewed toolchain: MSVC 14.51.36231 (Visual Studio Build Tools 2026).
No Python package dependencies are needed for the DLL build or isolated tests.

Run from the mod directory:

```powershell
python source/build_release.py --require-reviewed-hashes
python source/build_release.py --test
```

MSVC is detected with vswhere. Alternatively pass `--msvc-bin` followed by
the absolute path of MSVC's `bin/Hostx64/x64` directory.
`--component fireball` or `--component companion` selects one DLL.
`--output PATH` selects the output root (default: `build/from-source`).
For modified source, omit `--require-reviewed-hashes`; changes should differ.

The builder generates kernel32.lib and ntdll.lib from the supplied .def files,
then compiles the C sources without CRT or C++ headers. Runtime Ogre, PU and
KenshiLib APIs are resolved from the loaded modules. Do not use this setup to
build KenshiLib itself, which has different toolchain requirements.
The game and runtime dependency DLLs are not bundled here.

With the reviewed toolchain, both outputs are byte-for-byte identical to the
accepted local runtime. Only the COFF and PE debug-directory build timestamps
are normalized to the reviewed release values. No instructions, resource
constants or other payload bytes are patched after compilation.

| DLL | SHA256 |
| --- | --- |
| ElementalVisuals-v0.23-HolyFire-3x.dll | 4d61fb3aa3d94b74fe0be01d2a7d22ec48aa8cbeb9865abe8f58bb2a3bc961cc |
| AEV_EarthStorm-v4.dll | bb9ff6c42b513d658ff2dc8a8cf8195b4202c5ee2badec5ae17d89016ea610c9 |

The fireball sources were recovered from archived experimental source on
2026-11-01. The rejected GPU-transform suppression and chamber-emitter changes
were removed; the accepted 16-trail cap, visibility/reuse handling, Holy-fire
template and 4600 ms emission were restored. Reconstruction was verified
against every byte of the accepted DLL, not only its code section.

The isolated tests cover bounded allocation, unrelated objects, flight-material
restoration, hit effects, instance reuse, node cleanup, terrain scaling and
the companion effects. Test exports are excluded from release DLLs.

To install a modified release after testing, fully exit Kenshi, back up the
mod folder and copy release DLLs to the relative paths in RE_Kenshi.json.
Never install a test build. Disable the add-on's .mod and fully restart to
remove it. The add-on stores no spell progression in saves.

Source map:

- ElementalVisuals.c: guarded resource-directory redirect and entry point.
- MissileEmbers.c: accepted fireball material, flight embers and hit effect.
- ElementalCompanion.c: lifecycle tracking, engine guards and Earth dust.
- EarthRock.inl: branched rock proxy and terrain-pivot scaling.
- FireVortex.inl: fire-vortex attachment and cleanup.
- MissileFlame.inl: horizontal flame following the flight-ember source.
- *Tests.inl / AEV_TEST blocks: isolated checks, excluded from release DLLs.

Code: GPL-3.0-only. See COPYING, LICENSE.txt and MIT-ORIGINAL-NOTICE.txt.
Asset origins and permissions are listed in ../CREDITS.txt.
