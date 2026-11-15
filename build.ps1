<#
    Build and run the sorting algorithm test suite on Windows (PowerShell 5.1+).

    Examples:
        .\build.ps1
        .\build.ps1 -Clean
        .\build.ps1 -Configuration Debug
#>
param(
    [switch]$Clean,
    [ValidateSet('Debug', 'Release')]
    [string]$Configuration = 'Release'
)

$ErrorActionPreference = 'Stop'

$root = $PSScriptRoot
$buildDir = Join-Path $root 'build'
if ($Clean -and (Test-Path $buildDir)) {
    Remove-Item -Recurse -Force $buildDir
}
New-Item -ItemType Directory -Force -Path $buildDir | Out-Null

$compiler = if ($env:CC) { $env:CC } else { 'gcc' }
$optimization = if ($Configuration -eq 'Debug') { @('-O0', '-g') } else { @('-O2') }

$flags = @('-std=c11', '-Wall', '-Wextra', '-Wpedantic') + $optimization
$include = Join-Path $root 'include'
$sources = Get-ChildItem -Path (Join-Path $root 'src') -Filter '*.c' |
    Sort-Object Name |
    ForEach-Object { $_.FullName }
$testSource = Join-Path $root 'tests\test_sort.c'
$testBinary = Join-Path $buildDir 'test_sort.exe'

& $compiler @flags "-I$include" $testSource @sources '-o' $testBinary
if ($LASTEXITCODE -ne 0) {
    throw "Compilation failed with exit code $LASTEXITCODE."
}

& $testBinary
if ($LASTEXITCODE -ne 0) {
    throw "Tests failed with exit code $LASTEXITCODE."
}
