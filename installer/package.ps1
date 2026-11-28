$ErrorActionPreference = 'Stop'
$root = Split-Path -Parent $PSScriptRoot
$exe = Join-Path $root 'out/InstallFFPRScreenReader.exe'
$guide = Join-Path $PSScriptRoot 'Install.txt'
if (!(Test-Path -LiteralPath $exe -PathType Leaf)) { throw 'Build the setup executable first.' }
$releases = Join-Path $root 'releases'
New-Item -ItemType Directory -Force -Path $releases | Out-Null
$archive = Join-Path $releases 'FFPR-Screen-Reader-Setup.zip'
if (Test-Path -LiteralPath $archive) { throw 'The setup ZIP already exists. Preserve it before rebuilding.' }
Compress-Archive -LiteralPath @($exe, $guide) -DestinationPath $archive -CompressionLevel Optimal
$expected = @{'InstallFFPRScreenReader.exe' = (Get-FileHash -LiteralPath $exe).Hash; 'Install.txt' = (Get-FileHash -LiteralPath $guide).Hash}
$zip = [IO.Compression.ZipFile]::OpenRead($archive)
try {
    if ($zip.Entries.Count -ne $expected.Count) { throw 'Wrong setup package inventory.' }
    foreach ($item in $zip.Entries) {
        $name = $item.FullName
        if (!$expected.ContainsKey($name)) { throw "Unexpected setup archive entry $name." }
        $stream = $item.Open()
        try { $hash = (Get-FileHash -InputStream $stream).Hash }
        finally { $stream.Dispose() }
        if ($hash -ne $expected[$name]) { throw "Setup archive hash mismatch for $name." }
        $expected.Remove($name)
    }
} finally { $zip.Dispose() }
Get-FileHash -LiteralPath $archive
