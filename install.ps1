<#
.SYNOPSIS
Installs and builds mdcompiler tools on Windows.
#>
[CmdletBinding()]
param(
    [switch]$SkipDependencies,
    [string]$Prefix = (Join-Path $PSScriptRoot "dist")
)

$ErrorActionPreference = "Stop"
Set-Location $PSScriptRoot

if (-not $SkipDependencies) {
    foreach ($package in @(
        "Kitware.CMake",
        "Ninja-build.Ninja",
        "LLVM.LLVM",
        "RProject.R"
    )) {
        Write-Host "Checking $package..."
        winget install --id $package --exact --accept-package-agreements --accept-source-agreements
    }
}

$cmake = Get-Command cmake -ErrorAction SilentlyContinue
if (-not $cmake) {
    throw "CMake bulunamadi. Kurulumdan sonra yeni bir terminal acin."
}

$ninja = Get-Command ninja -ErrorAction SilentlyContinue
$generator = if ($ninja) { "Ninja" } else { "NMake Makefiles" }

& cmake -S $PSScriptRoot -B (Join-Path $PSScriptRoot "build") "-G$generator" "-DCMAKE_BUILD_TYPE=Release"
& cmake --build (Join-Path $PSScriptRoot "build") --parallel
& cmake --install (Join-Path $PSScriptRoot "build") --prefix $Prefix

Write-Host "Kurulum tamamlandi: $Prefix"
