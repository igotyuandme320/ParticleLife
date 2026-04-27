# ParticleLife

`ParticleLife` is a small Qt 6 desktop app that simulates particle-life style interactions with a live control panel.

## Requirements

### macOS

- Xcode Command Line Tools
- CMake 3.20+
- Qt 6 (`qt` or `qtbase`)

### Windows

- Visual Studio 2022 with `Desktop development with C++`
- CMake 3.20+
- Qt 6 `msvc2022_64` installed to the default `C:\Qt\...` location, or `QT_ROOT` / `QTDIR` pointing at that kit
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

That script:

- locates Qt from `QT_ROOT`, `QTDIR`, or `C:\Qt\*\msvc2022_64`
- configures with the `windows-msvc` preset
- builds the app
- launches `ParticleLife.exe`

The Windows build also runs `windeployqt` after each build, so the generated `.exe` can be started from the build folder without manually fixing `PATH`.

## VSCode On Windows

1. Open the cloned folder in VSCode.
2. Install the recommended extensions when prompted.
3. Run `CMake: Select Configure Preset`.
4. Pick `Windows MSVC 2022`.
5. Run `CMake: Build`.
6. Run `CMake: Run Without Debugging` or `CMake: Debug`.

If Qt is installed somewhere other than `C:\Qt\...`, set one of these before opening VSCode:

```powershell
$env:QT_ROOT = "D:\Qt\6.8.3\msvc2022_64"
```

## Manual Windows CLI

```powershell
cmake --preset windows-msvc
cmake --build --preset windows-release
.\build\windows-msvc\Release\ParticleLife.exe
```
