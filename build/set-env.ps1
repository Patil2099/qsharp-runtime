# Copyright (c) Microsoft Corporation. All rights reserved.
# Licensed under the MIT License.

$ErrorActionPreference = 'Stop'

Write-Host "Setting up build environment variables"

If ($Env:BUILD_BUILDNUMBER -eq $null) { $Env:BUILD_BUILDNUMBER ="0.0.1.0" }
If ($Env:BUILD_CONFIGURATION -eq $null) { $Env:BUILD_CONFIGURATION ="Debug" }
If ($Env:BUILD_VERBOSITY -eq $null) { $Env:BUILD_VERBOSITY ="m" }
If ($Env:ASSEMBLY_VERSION -eq $null) { $Env:ASSEMBLY_VERSION ="$Env:BUILD_BUILDNUMBER" }
If ($Env:NUGET_VERSION -eq $null) { $Env:NUGET_VERSION ="$Env:ASSEMBLY_VERSION-alpha" }

If (($Env:ENABLE_NATIVE -ne "false") -and ($Env:NATIVE_SIMULATOR -eq $null) ) {
    $Env:NATIVE_SIMULATOR = (Join-Path $PSScriptRoot "..\src\Simulation\Native\build\drop")
}

if ($Env:ENABLE_QIRRUNTIME -ne "false" -and $Env:QIR_DROPS -eq $null) {
    $Env:QIR_DROPS = (Join-Path $PSScriptRoot "../src/Qir/drops")
}

If ($Env:DROPS_DIR -eq $null) { $Env:DROPS_DIR =  [IO.Path]::GetFullPath((Join-Path $PSScriptRoot "..\drops")) }

If ($Env:NUGET_OUTDIR -eq $null) { $Env:NUGET_OUTDIR =  (Join-Path $Env:DROPS_DIR "nugets") }
If (-not (Test-Path -Path $Env:NUGET_OUTDIR)) { [IO.Directory]::CreateDirectory($Env:NUGET_OUTDIR) }

If ($Env:DOCS_OUTDIR -eq $null) { $Env:DOCS_OUTDIR =  (Join-Path $Env:DROPS_DIR "docs") }
If (-not (Test-Path -Path $Env:DOCS_OUTDIR)) { [IO.Directory]::CreateDirectory($Env:DOCS_OUTDIR) }

