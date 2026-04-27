param(
    [ValidateSet("Debug", "Release")]
    [string]$Config = "Debug",
    [switch]$SkipLaunch
)

$ErrorActionPreference = "Stop"
Set-StrictMode -Version Latest

function Require-Command {
    param(
        [Parameter(Mandatory = $true)]
        [string]$Name
    )

    if (-not (Get-Command $Name -ErrorAction SilentlyContinue)) {
        throw "Required command '$Name' was not found in PATH."
    }
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

Require-Command -Name "cmake"

$repoRoot = Split-Path -Parent $PSScriptRoot
$qtRoot = Resolve-QtRoot
$buildPreset = if ($Config -eq "Release") { "windows-release" } else { "windows-debug" }
$exePath = Join-Path $repoRoot "build\windows-msvc\$Config\ParticleLife.exe"

Write-Host "Using Qt from $qtRoot"
cmake --preset windows-msvc "-DCMAKE_PREFIX_PATH=$qtRoot"
cmake --build --preset $buildPreset

if (-not (Test-Path $exePath)) {
    throw "Build completed, but '$exePath' was not produced."
}

if (-not $SkipLaunch) {
    Start-Process $exePath
}
