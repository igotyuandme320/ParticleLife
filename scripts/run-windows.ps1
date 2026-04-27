param(
    [ValidateSet("Debug", "Release")]
    [string]$Config = "Debug",
    [switch]$SkipLaunch
)

$ErrorActionPreference = "Stop"
Set-StrictMode -Version Latest

function Resolve-CMake {
    $command = Get-Command cmake -ErrorAction SilentlyContinue
    if ($command) {
        return $command.Source
    }

    foreach ($candidate in @(
        "C:\Program Files\CMake\bin\cmake.exe",
        "C:\Program Files (x86)\CMake\bin\cmake.exe"
    )) {
        if (Test-Path $candidate) {
            return $candidate
        }
    }

    $vswhere = "C:\Program Files (x86)\Microsoft Visual Studio\Installer\vswhere.exe"
    if (Test-Path $vswhere) {
        $vsInstallPath = & $vswhere -latest -products * -requires Microsoft.VisualStudio.Component.VC.Tools.x86.x64 -property installationPath
        if ($vsInstallPath) {
            foreach ($relativePath in @(
                "Common7\IDE\CommonExtensions\Microsoft\CMake\CMake\bin\cmake.exe",
                "Common7\IDE\CommonExtensions\Microsoft\CMake\Ninja\cmake.exe"
            )) {
                $candidate = Join-Path $vsInstallPath.Trim() $relativePath
                if (Test-Path $candidate) {
                    return $candidate
                }
            }
        }
    }

    throw "CMake was not found. Install CMake, or install Visual Studio 2022 with Desktop development with C++."
}

function Resolve-QtRoot {
    foreach ($envVar in @("QT_ROOT", "QTDIR")) {
        $value = [Environment]::GetEnvironmentVariable($envVar)
        if ($value -and (Test-Path $value)) {
            return (Resolve-Path $value).Path
        }
    }

    if (Test-Path "C:\Qt") {
        $qtRoot = Get-ChildItem "C:\Qt" -Directory |
            Sort-Object {
                try {
                    [Version]$_.Name
                } catch {
                    [Version]"0.0"
                }
            } -Descending |
            ForEach-Object {
                foreach ($kit in @("msvc2022_64", "msvc2019_64")) {
                    $candidate = Join-Path $_.FullName $kit
                    if (Test-Path $candidate) {
                        return $candidate
                    }
                }
            } |
            Select-Object -First 1

        if ($qtRoot) {
            return $qtRoot
        }
    }

    throw "Qt 6 for MSVC was not found. Install Qt 6 msvc2022_64 to C:\Qt or set QT_ROOT."
}

$repoRoot = Split-Path -Parent $PSScriptRoot
$cmakeExe = Resolve-CMake
$qtRoot = Resolve-QtRoot
$buildPreset = if ($Config -eq "Release") { "windows-release" } else { "windows-debug" }
$exePath = Join-Path $repoRoot "build\windows-msvc\$Config\ParticleLife.exe"

Write-Host "Using CMake from $cmakeExe"
Write-Host "Using Qt from $qtRoot"
& $cmakeExe --preset windows-msvc "-DCMAKE_PREFIX_PATH=$qtRoot"
& $cmakeExe --build --preset $buildPreset

if (-not (Test-Path $exePath)) {
    throw "Build completed, but '$exePath' was not produced."
}

if (-not $SkipLaunch) {
    Start-Process $exePath
}
