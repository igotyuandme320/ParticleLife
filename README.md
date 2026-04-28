# ParticleLife

`ParticleLife` is a small Qt 6 desktop app that simulates particle-life style interactions with a live control panel.

## Requirements

### macOS

- Xcode Command Line Tools
- CMake 3.20+
- Qt 6 (`qt` or `qtbase`)

### Windows

- Visual Studio 2019 or 2022 with `Desktop development with C++`
- Qt 6 `msvc2019_64` or `msvc2022_64` installed to the default `C:\Qt\...` location, or `QT_ROOT` / `QTDIR` pointing at that kit
- VSCode with the recommended extensions in `.vscode/extensions.json` if you want the editor workflow

## Quick Start

### macOS

```bash
cmake -S . -B build
cmake --build build
open build/ParticleLife.app
```

### Windows

After `git clone`, the fastest path is:

```powershell
powershell -ExecutionPolicy Bypass -File .\scripts\run-windows.ps1
```

If you want to force the Visual Studio 2019 toolchain explicitly:

```powershell
powershell -ExecutionPolicy Bypass -File .\scripts\run-windows.ps1 -Toolchain msvc2019
```

That script:

- locates CMake from `PATH`, a normal CMake install, or Visual Studio's bundled CMake
- locates Qt from `QT_ROOT`, `QTDIR`, or `C:\Qt\*\msvc2019_64` / `msvc2022_64`
- matches the CMake preset to the Qt kit (`msvc2019_64` or `msvc2022_64`)
- builds the app
- launches `ParticleLife.exe`

The Windows build also runs `windeployqt` after each build, so the generated `.exe` can be started from the build folder without manually fixing `PATH`.

## VSCode On Windows

1. Open the cloned folder in VSCode.
2. Install the recommended extensions when prompted.
3. Run `CMake: Select Configure Preset`.
4. Pick `Windows MSVC 2019` for `msvc2019_64`, or `Windows MSVC 2022` for `msvc2022_64`.
5. Run `CMake: Build`.
6. Run `CMake: Run Without Debugging` or `CMake: Debug`.

If Qt is installed somewhere other than `C:\Qt\...`, set one of these before opening VSCode:

```powershell
$env:QT_ROOT = "D:\Qt\6.2.1\msvc2019_64"
```

## Manual Windows CLI

```powershell
cmake --preset windows-msvc2019 -DCMAKE_PREFIX_PATH=C:\Qt\6.2.1\msvc2019_64
cmake --build --preset windows-msvc2019-release
.\build\windows-msvc2019\Release\ParticleLife.exe
```
