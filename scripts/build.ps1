#Requires -Version 5.1
<#
.SYNOPSIS
    Configure, build, and test the Helsinki project.

.DESCRIPTION
    Uses Ninja + MSVC (via vcvars64.bat). On first run or after -Rebuild,
    runs CMake configure. By default performs an incremental build and runs CTest.

.PARAMETER Rebuild
    Delete the build directory, reconfigure from scratch, build, and test.

.PARAMETER Clean
    Delete the build directory only. Does not build or test unless combined
    with a normal invocation (no -CleanOnly).

.PARAMETER CleanOnly
    Delete the build directory and exit.

.PARAMETER Configure
    Force CMake configure even if the build directory already exists.

.PARAMETER NoTest
    Skip running tests after a successful build.

.PARAMETER BuildType
    CMake build type (Debug or Release). Default: Debug.

.EXAMPLE
    .\scripts\build.ps1
    Incremental build and run tests.

.EXAMPLE
    .\scripts\build.ps1 -Rebuild
    Clean rebuild from scratch and run tests.

.EXAMPLE
    .\scripts\build.ps1 -CleanOnly
    Remove the build directory only.

.EXAMPLE
    .\scripts\build.ps1 -NoTest
    Incremental build without running tests.
#>
[CmdletBinding()]
param(
    [ValidateSet('Debug', 'Release')]
    [string] $BuildType = 'Debug',

    [switch] $Rebuild,
    [switch] $Clean,
    [switch] $CleanOnly,
    [switch] $Configure,
    [switch] $NoTest
)

Set-StrictMode -Version Latest
$ErrorActionPreference = 'Stop'

$RepoRoot = Resolve-Path (Join-Path $PSScriptRoot '..')
$BuildDir = Join-Path $RepoRoot 'build'

function Get-VcVars64Path {
    $vswhere = Join-Path ${env:ProgramFiles(x86)} 'Microsoft Visual Studio\Installer\vswhere.exe'
    if (-not (Test-Path $vswhere)) {
        throw "vswhere not found. Install Visual Studio with the C++ workload."
    }

    $installationPath = & $vswhere `
        -latest `
        -products * `
        -requires Microsoft.VisualStudio.Component.VC.Tools.x86.x64 `
        -property installationPath

    if (-not $installationPath) {
        throw "No Visual Studio installation with C++ tools found."
    }

    $vcvars = Join-Path $installationPath 'VC\Auxiliary\Build\vcvars64.bat'
    if (-not (Test-Path $vcvars)) {
        throw "vcvars64.bat not found at: $vcvars"
    }

    return $vcvars
}

function Invoke-InMsvcEnvironment {
    param(
        [Parameter(Mandatory)]
        [string] $VcVars64,

        [Parameter(Mandatory)]
        [string] $Command,

        [Parameter(Mandatory)]
        [string] $WorkingDirectory
    )

    $escapedVcVars = $VcVars64.Replace('"', '""')
    $fullCommand = "call `"$escapedVcVars`" >nul && $Command"

    Push-Location $WorkingDirectory
    try {
        cmd /c $fullCommand
        if ($LASTEXITCODE -ne 0) {
            throw "Command failed with exit code $LASTEXITCODE`: $Command"
        }
    }
    finally {
        Pop-Location
    }
}

function Remove-BuildDirectory {
    if (Test-Path $BuildDir) {
        Write-Host "Removing build directory: $BuildDir"
        Remove-Item -LiteralPath $BuildDir -Recurse -Force
    }
    else {
        Write-Host "Build directory does not exist: $BuildDir"
    }
}

function Test-NeedsConfigure {
    $buildFile = Join-Path $BuildDir 'build.ninja'
    return $Configure -or $Rebuild -or -not (Test-Path $buildFile)
}

if ($CleanOnly) {
    Remove-BuildDirectory
    Write-Host "Clean complete."
    exit 0
}

if ($Rebuild) {
    $Clean = $true
}

if ($Clean) {
    Remove-BuildDirectory
}

$vcvars = Get-VcVars64Path

if (Test-NeedsConfigure) {
    Write-Host "Configuring ($BuildType)..."
    $configureCommand = "cmake -B build -G Ninja -DCMAKE_BUILD_TYPE=$BuildType"
    Invoke-InMsvcEnvironment -VcVars64 $vcvars -Command $configureCommand -WorkingDirectory $RepoRoot
}
else {
    Write-Host "Skipping configure (existing build directory)."
}

Write-Host "Building ($BuildType)..."
$buildCommand = 'cmake --build build'
Invoke-InMsvcEnvironment -VcVars64 $vcvars -Command $buildCommand -WorkingDirectory $RepoRoot

if (-not $NoTest) {
    Write-Host 'Running tests...'
    $testCommand = "ctest -C $BuildType --output-on-failure --test-dir build"
    Invoke-InMsvcEnvironment -VcVars64 $vcvars -Command $testCommand -WorkingDirectory $RepoRoot
}
else {
    Write-Host 'Skipping tests (-NoTest).'
}

Write-Host 'Done.'
