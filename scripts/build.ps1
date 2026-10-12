#Requires -Version 5.1
<#
.SYNOPSIS
    Configure, build, and test the Helsinki project.

.DESCRIPTION
    Uses Ninja + MSVC (via vcvars64.bat). On first run or after -Rebuild,
    runs CMake configure. By default performs an incremental build and runs CTest.
    Success prints "Build passed" and "Test passed". Command output is shown
    only when a step fails.

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
        [string] $WorkingDirectory,

        [Parameter(Mandatory)]
        [string] $FailedLabel
    )

    $escapedVcVars = $VcVars64.Replace('"', '""')
    $logFile = [System.IO.Path]::GetTempFileName()
    $escapedLog = $logFile.Replace('"', '""')
    $fullCommand = "call `"$escapedVcVars`" >nul 2>&1 && ($Command) > `"$escapedLog`" 2>&1"

    Push-Location $WorkingDirectory
    try {
        cmd /c $fullCommand
        $exitCode = $LASTEXITCODE
        if ($exitCode -ne 0) {
            if (Test-Path -LiteralPath $logFile) {
                Get-Content -LiteralPath $logFile -Raw | Write-Host
            }
            Write-Host $FailedLabel
            exit $exitCode
        }
    }
    finally {
        Pop-Location
        if (Test-Path -LiteralPath $logFile) {
            Remove-Item -LiteralPath $logFile -Force
        }
    }
}

function Remove-BuildDirectory {
    if (Test-Path $BuildDir) {
        Remove-Item -LiteralPath $BuildDir -Recurse -Force
    }
}

function Test-NeedsConfigure {
    $buildFile = Join-Path $BuildDir 'build.ninja'
    return $Configure -or $Rebuild -or -not (Test-Path $buildFile)
}

if ($CleanOnly) {
    Remove-BuildDirectory
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
    $configureCommand = "cmake -B build -G Ninja -DCMAKE_BUILD_TYPE=$BuildType"
    Invoke-InMsvcEnvironment -VcVars64 $vcvars -Command $configureCommand -WorkingDirectory $RepoRoot -FailedLabel 'Configure failed'
}

$buildCommand = 'cmake --build build'
Invoke-InMsvcEnvironment -VcVars64 $vcvars -Command $buildCommand -WorkingDirectory $RepoRoot -FailedLabel 'Build failed'
Write-Host 'Build passed'

if (-not $NoTest) {
    $testCommand = "ctest -C $BuildType --output-on-failure --test-dir build"
    Invoke-InMsvcEnvironment -VcVars64 $vcvars -Command $testCommand -WorkingDirectory $RepoRoot -FailedLabel 'Test failed'
    Write-Host 'Test passed'
}
