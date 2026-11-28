param([switch]$Tests)
$ErrorActionPreference = 'Stop'
$root = Split-Path -Parent $PSScriptRoot
$output = Join-Path $root 'out'
$compiler = Join-Path $env:WINDIR 'Microsoft.NET/Framework64/v4.0.30319/csc.exe'
if (!(Test-Path -LiteralPath $compiler -PathType Leaf)) { throw 'The Windows .NET Framework x64 C# compiler is required.' }
New-Item -ItemType Directory -Force -Path $output | Out-Null
$references = @('System.Windows.Forms.dll','System.Drawing.dll','System.Web.Extensions.dll','System.IO.Compression.dll')
$arguments = @('/nologo', '/platform:x64', '/codepage:65001', '/optimize+', '/warnaserror+', ('/win32manifest:' + (Join-Path $PSScriptRoot 'setup.manifest'))) +
    @($references | ForEach-Object { '/reference:' + $_ }) +
    @('prism.lock.json','openal.lock.json' | ForEach-Object { '/resource:' + (Join-Path $root $_) + ',' + $_ }) +
    @(Get-ChildItem -LiteralPath $PSScriptRoot -Filter '*.cs' -File | ForEach-Object FullName)
$target = Join-Path $output 'InstallFFPRScreenReader.exe'
& $compiler '/target:winexe' ('/out:' + $target) @arguments
if ($LASTEXITCODE -ne 0) { throw 'The FFPR setup build failed.' }
Write-Output "Built $target"
if ($Tests) {
    $checks = Join-Path $output 'checks'
    New-Item -ItemType Directory -Force -Path $checks | Out-Null
    $harness = Join-Path $checks 'ffpr_installer_check.exe'
    & $compiler '/target:exe' '/main:FFPR.Setup.InstallerChecks' ('/out:' + $harness) @arguments (Join-Path $PSScriptRoot 'tests/InstallerChecks.cs')
    if ($LASTEXITCODE -ne 0) { throw 'The FFPR setup fixture build failed.' }
}
