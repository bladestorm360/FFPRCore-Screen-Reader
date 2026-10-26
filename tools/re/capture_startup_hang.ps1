param([Parameter(Mandatory=$true)][int]$TargetProcessId,[Parameter(Mandatory=$true)][string]$OutputDirectory)
$ErrorActionPreference='Stop'
# Explicit user approval is required before running this diagnostic. It never
# launches a game, injects code, writes target memory or sends input. Native
# minidump stack/module data belongs outside every Git checkout.
$gameProcess=Get-Process -Id $TargetProcessId
$expected='D:\Games\SteamLibrary\steamapps\common\FINAL FANTASY III PR\FINAL FANTASY III.exe'
if ($gameProcess.Path -ne $expected) { throw 'Refusing a process other than the approved FFIII game.' }
$targetRoot=[IO.Path]::GetFullPath($OutputDirectory)
$allowed=[IO.Path]::GetFullPath((Join-Path $PSScriptRoot '..\..\..\tools\startup_hang_20261025'))
if (-not $targetRoot.StartsWith($allowed+[IO.Path]::DirectorySeparatorChar,[StringComparison]::OrdinalIgnoreCase)) {throw 'Diagnostic output must remain in the scoped external analysis directory.'}
$null=New-Item -ItemType Directory -Path $targetRoot -Force
$gitRoot=git -C $targetRoot rev-parse --show-toplevel 2>$null
if ($LASTEXITCODE -eq 0) {throw 'Refusing game-memory output inside a Git checkout.'}
Add-Type -TypeDefinition @'
using System;
using System.Runtime.InteropServices;
public static class FF3HangSnapshot {
 [DllImport("kernel32.dll",SetLastError=true)] public static extern IntPtr OpenProcess(uint access,bool inherit,int id);
 [DllImport("kernel32.dll",SetLastError=true)] public static extern bool CloseHandle(IntPtr handle);
 [DllImport("dbghelp.dll",SetLastError=true)] public static extern bool MiniDumpWriteDump(IntPtr process,int id,IntPtr file,uint type,IntPtr exception,IntPtr streams,IntPtr callback);
}
'@
$path=Join-Path $targetRoot ('ff3_'+$TargetProcessId+'.dmp')
$handle=[FF3HangSnapshot]::OpenProcess(0x0410,$false,$TargetProcessId)
if ($handle -eq [IntPtr]::Zero) {throw ('OpenProcess failed: '+[Runtime.InteropServices.Marshal]::GetLastWin32Error())}
$file=$null
try {
 $file=[IO.File]::Open($path,[IO.FileMode]::CreateNew,[IO.FileAccess]::ReadWrite,[IO.FileShare]::Read)
 # Normal stack/module snapshot plus thread metadata; no full heap or handles.
 if (-not [FF3HangSnapshot]::MiniDumpWriteDump($handle,$TargetProcessId,$file.SafeFileHandle.DangerousGetHandle(),0x1000,[IntPtr]::Zero,[IntPtr]::Zero,[IntPtr]::Zero)) {throw ('MiniDumpWriteDump failed: '+[Runtime.InteropServices.Marshal]::GetLastWin32Error())}
} finally {
 if ($file) {$file.Dispose()}
 [void][FF3HangSnapshot]::CloseHandle($handle)
}
$moduleRows=@($gameProcess.Modules | Select-Object ModuleName,FileName,@{n='base';e={$_.BaseAddress.ToInt64()}},ModuleMemorySize)
$moduleRows | ConvertTo-Json -Depth 4 | Set-Content -LiteralPath (Join-Path $targetRoot 'modules.json') -Encoding utf8
$summary=[pscustomobject]@{process_id=$TargetProcessId;process_path=$gameProcess.Path;started=$gameProcess.StartTime.ToUniversalTime().ToString('o');captured=[DateTime]::UtcNow.ToString('o');dump=$path;bytes=(Get-Item -LiteralPath $path).Length;sha256=(Get-FileHash -LiteralPath $path -Algorithm SHA256).Hash.ToLowerInvariant();explicit_user_approval=$true;target_memory_written=$false;game_launched=$false;game_input_sent=$false}
$summary | ConvertTo-Json -Depth 4 | Set-Content -LiteralPath (Join-Path $targetRoot 'capture.json') -Encoding utf8
$summary | ConvertTo-Json -Depth 3
exit 0
