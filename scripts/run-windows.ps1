param(
    [ValidateSet("Debug", "Release")]
    [string]$Config = "Debug",
    [ValidateSet("auto", "msvc2019", "msvc2022")]
    [string]$Toolchain = "auto",
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

    throw "CMake was not found. Install CMake, or install Visual Studio 2019/2022 with Desktop development with C++."
}

function Get-VswherePath {
    $vswhere = "C:\Program Files (x86)\Microsoft Visual Studio\Installer\vswhere.exe"
    if (Test-Path $vswhere) {
        return $vswhere
    }

    return $null
}

function Test-VisualStudioToolchain {
    param(
        [Parameter(Mandatory = $true)]
        [ValidateSet("msvc2019", "msvc2022")]
        [string]$Toolchain
    )

    $vswhere = Get-VswherePath
    if (-not $vswhere) {
        return $false
    }

    $versionRange = if ($Toolchain -eq "msvc2019") { "[16.0,17.0)" } else { "[17.0,18.0)" }
    $vsInstallPath = & $vswhere -latest -products * -version $versionRange -requires Microsoft.VisualStudio.Component.VC.Tools.x86.x64 -property installationPath
    return -not [string]::IsNullOrWhiteSpace($vsInstallPath)
}

function Get-PreferredToolchains {
    param(
        [Parameter(Mandatory = $true)]
        [string]$RequestedToolchain
    )

    if ($RequestedToolchain -ne "auto") {
        return @($RequestedToolchain)
    }

    $toolchains = @()
    if (Test-VisualStudioToolchain -Toolchain "msvc2022") {
        $toolchains += "msvc2022"
    }
    if (Test-VisualStudioToolchain -Toolchain "msvc2019") {
        $toolchains += "msvc2019"
    }

    if ($toolchains.Count -eq 0) {
        return @("msvc2022", "msvc2019")
    }

    return $toolchains
}

function Resolve-QtRoot {
    param(
        [Parameter(Mandatory = $true)]
        [string]$RequestedToolchain
    )

    foreach ($envVar in @("QT_ROOT", "QTDIR")) {
        $value = [Environment]::GetEnvironmentVariable($envVar)
        if ($value -and (Test-Path $value)) {
            return (Resolve-Path $value).Path
        }
    }

    if (Test-Path "C:\Qt") {
        $kitMap = @{
            "msvc2019" = "msvc2019_64"
            "msvc2022" = "msvc2022_64"
        }
        $kitCandidates = @()
        foreach ($toolchainName in Get-PreferredToolchains -RequestedToolchain $RequestedToolchain) {
            $kitCandidates += $kitMap[$toolchainName]
        }

        $qtRoot = Get-ChildItem "C:\Qt" -Directory |
            Sort-Object {
                try {
                    [Version]$_.Name
                } catch {
                    [Version]"0.0"
                }
            } -Descending |
            ForEach-Object {
                foreach ($kit in $kitCandidates) {
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

    throw "Qt 6 for MSVC was not found. Install a Qt 6 MSVC kit such as C:\Qt\<version>\msvc2022_64 or C:\Qt\<version>\msvc2019_64, or set QT_ROOT to that folder."
}

function Resolve-MsvcConfig {
    param(
        [Parameter(Mandatory = $true)]
        [string]$QtRoot,
        [Parameter(Mandatory = $true)]
        [string]$RequestedToolchain
    )

    $qtKit = Split-Path $QtRoot -Leaf
    $mapping = @{
        "msvc2019_64" = [pscustomobject]@{
            Toolchain = "msvc2019"
            ConfigurePreset = "windows-msvc2019"
            BuildDir = "windows-msvc2019"
            Generator = "Visual Studio 16 2019"
            VswhereRange = "[16.0,17.0)"
        }
        "msvc2022_64" = [pscustomobject]@{
            Toolchain = "msvc2022"
            ConfigurePreset = "windows-msvc2022"
            BuildDir = "windows-msvc2022"
            Generator = "Visual Studio 17 2022"
            VswhereRange = "[17.0,18.0)"
        }
    }

    if (-not $mapping.ContainsKey($qtKit)) {
        throw "Unsupported Qt kit '$qtKit'. Expected msvc2019_64 or msvc2022_64."
    }

    $config = $mapping[$qtKit]
    if ($RequestedToolchain -ne "auto" -and $RequestedToolchain -ne $config.Toolchain) {
        throw "QT_ROOT points to '$qtKit', but -Toolchain $RequestedToolchain was requested. Make them match."
    }

    return $config
}

function Assert-VisualStudioAvailable {
    param(
        [Parameter(Mandatory = $true)]
        [pscustomobject]$MsvcConfig
    )

    $vswhere = Get-VswherePath
    if (-not $vswhere) {
        throw "Visual Studio Installer metadata was not found. Install $($MsvcConfig.Generator) with Desktop development with C++."
    }

    $vsInstallPath = & $vswhere -latest -products * -version $MsvcConfig.VswhereRange -requires Microsoft.VisualStudio.Component.VC.Tools.x86.x64 -property installationPath
    if (-not $vsInstallPath) {
        throw "$($MsvcConfig.Generator) with Desktop development with C++ was not found. Install the matching Visual Studio C++ toolchain for $($MsvcConfig.Toolchain)."
    }
}

$repoRoot = Split-Path -Parent $PSScriptRoot
$cmakeExe = Resolve-CMake
$qtRoot = Resolve-QtRoot -RequestedToolchain $Toolchain
$msvcConfig = Resolve-MsvcConfig -QtRoot $qtRoot -RequestedToolchain $Toolchain
$buildPreset = if ($Config -eq "Release") { "$($msvcConfig.ConfigurePreset)-release" } else { "$($msvcConfig.ConfigurePreset)-debug" }
$exePath = Join-Path $repoRoot "build\$($msvcConfig.BuildDir)\$Config\ParticleLife.exe"

Assert-VisualStudioAvailable -MsvcConfig $msvcConfig

Write-Host "Using CMake from $cmakeExe"
Write-Host "Using Qt from $qtRoot"
Write-Host "Using toolchain $($msvcConfig.Toolchain)"
& $cmakeExe --preset $msvcConfig.ConfigurePreset "-DCMAKE_PREFIX_PATH=$qtRoot"
& $cmakeExe --build --preset $buildPreset

if (-not (Test-Path $exePath)) {
    throw "Build completed, but '$exePath' was not produced."
}

if (-not $SkipLaunch) {
    Start-Process $exePath
}
