# Local distribution check only. No game launch, external upload, signing,
# security-setting changes or dependency installation. Uses installed Defender.
param([Parameter(Mandatory=$true)][string]$EvidenceDirectory,[switch]$BuiltPayload)
$ErrorActionPreference = 'Stop'
$repositoryRoot = (Resolve-Path -LiteralPath (Join-Path $PSScriptRoot '..')).Path
$evidenceRoot = [IO.Path]::GetFullPath($EvidenceDirectory)
if (!$evidenceRoot.StartsWith($repositoryRoot + [IO.Path]::DirectorySeparatorChar, [StringComparison]::OrdinalIgnoreCase)) {
    throw 'Evidence must stay inside the intended repository workspace.'
}
if (Test-Path -LiteralPath $evidenceRoot) { throw 'Preserve prior scan evidence; choose a new evidence directory.' }
$null = New-Item -ItemType Directory -Path $evidenceRoot
$payloadRoot = Join-Path $evidenceRoot 'payload'
$null = New-Item -ItemType Directory -Path $payloadRoot

# The same dependency bytes are installed in all six games. Select only the
# mod/runtime payload, never game code, preferences, saves or personal waypoints.
$receiptName = if ($BuiltPayload) { 'deploy_before.json' } else { 'deploy_after.json' }
$deploymentPath = Join-Path (Split-Path -Parent $evidenceRoot) $receiptName
$deployment = Get-Content -LiteralPath $deploymentPath -Raw | ConvertFrom-Json
$reference = $deployment.games.'2'
$relativePaths = @('hid.dll','SDL3.dll') + @($reference.protected.PSObject.Properties.Name | Where-Object {
    $_.StartsWith('.ffpr-deps\') -and $_ -notmatch '(\.lock$|prism-install\.json$)'
})
$manifest = @()
foreach ($relative in $relativePaths) {
    $source = Join-Path $reference.root $relative
    # Pre-install checks stage the canonical freshly built DLL and unchanged
    # installed dependencies. The receipt still verifies dependency consistency.
    if ($BuiltPayload -and $relative -eq 'hid.dll') { $source = Join-Path $repositoryRoot 'out\hid.dll' }
    $sourceHash = (Get-FileHash -LiteralPath $source -Algorithm SHA256).Hash.ToLowerInvariant()
    foreach ($game in $deployment.games.PSObject.Properties) {
        if ($BuiltPayload -and $relative -eq 'hid.dll') { continue }
        $installed = Join-Path $game.Value.root $relative
        if ((Get-FileHash -LiteralPath $installed -Algorithm SHA256).Hash.ToLowerInvariant() -ne $sourceHash) {
            throw "Installed payload mismatch: $($game.Name) / $relative"
        }
    }
    $target = Join-Path $payloadRoot $relative
    $null = New-Item -ItemType Directory -Path (Split-Path -Parent $target) -Force
    Copy-Item -LiteralPath $source -Destination $target
    if ((Get-FileHash -LiteralPath $target -Algorithm SHA256).Hash.ToLowerInvariant() -ne $sourceHash) {
        throw "Staging mismatch: $relative"
    }
    $manifest += [pscustomobject]@{path=$relative;sha256=$sourceHash;bytes=(Get-Item -LiteralPath $target).Length}
}
$manifest | ConvertTo-Json -Depth 5 | Set-Content -LiteralPath (Join-Path $evidenceRoot 'manifest.json') -Encoding utf8

$signatures = @($relativePaths | Where-Object { $_ -match '\.(dll|exe)$' } | ForEach-Object {
    $signature = Get-AuthenticodeSignature -LiteralPath (Join-Path $payloadRoot $_)
    [pscustomobject]@{path=$_;status=$signature.Status.ToString();message=$signature.StatusMessage;signer=$signature.SignerCertificate.Subject}
})
$signatures | ConvertTo-Json -Depth 5 | Set-Content -LiteralPath (Join-Path $evidenceRoot 'signatures.json') -Encoding utf8
$certificates = @(Get-ChildItem -LiteralPath Cert:\CurrentUser\My,Cert:\LocalMachine\My -CodeSigningCert | Select-Object Subject,Thumbprint,NotAfter)
ConvertTo-Json -InputObject $certificates -Depth 4 | Set-Content -LiteralPath (Join-Path $evidenceRoot 'signing_capability.json') -Encoding utf8

$scanner = Join-Path $env:ProgramFiles 'Windows Defender\MpCmdRun.exe'
$scannerSignature = Get-AuthenticodeSignature -LiteralPath $scanner
if ($scannerSignature.Status -ne 'Valid') { throw 'The installed scan utility does not have a valid platform signature.' }
$status = Get-CimInstance -Namespace root/Microsoft/Windows/Defender -ClassName MSFT_MpComputerStatus |
    Select-Object AMEngineVersion,AMProductVersion,AntivirusEnabled,RealTimeProtectionEnabled,AntivirusSignatureVersion,AntivirusSignatureLastUpdated,AntivirusSignatureAge,DefenderSignaturesOutOfDate
$status | ConvertTo-Json | Set-Content -LiteralPath (Join-Path $evidenceRoot 'scanner.json') -Encoding utf8
if (!$status.AntivirusEnabled -or !$status.RealTimeProtectionEnabled -or $status.DefenderSignaturesOutOfDate) {
    throw 'Record scanner readiness as a gap; never weaken protection or claim current coverage.'
}
$exclusionOutput = & $scanner -CheckExclusion -path $payloadRoot 2>&1
$exclusionExit = $LASTEXITCODE
$exclusionOutput | Out-String | Set-Content -LiteralPath (Join-Path $evidenceRoot 'exclusion_check.txt') -Encoding utf8
# Windows allows custom scans for this user but protects exclusion queries with
# admin ACLs. Retain that coverage limit; never elevate/change protection just
# to turn an unknown result into a clean-scan claim.
$exclusionText = $exclusionOutput | Out-String
$exclusionVerified = $exclusionText -match '(?i)not excluded'
if (!$exclusionVerified -and $exclusionText -notmatch '0x80070005') {
    throw 'Exclusion status needs investigation before interpreting scan coverage.'
}

$started = Get-Date
$scanOutput = & $scanner -Scan -ScanType 3 -File $payloadRoot 2>&1
$scanExit = $LASTEXITCODE
$finished = Get-Date
$scanOutput | Out-String | Set-Content -LiteralPath (Join-Path $evidenceRoot 'scan_output.txt') -Encoding utf8
# Exit0 can also mean successful remediation. Check matching detection events
# and every byte afterward before interpreting the scan as no detections.
$scanEventErrors = @()
$events = @(Get-WinEvent -FilterHashtable @{LogName='Microsoft-Windows-Windows Defender/Operational';StartTime=$started.AddSeconds(-1);Id=1000,1001,1116,1117} -ErrorAction SilentlyContinue -ErrorVariable +scanEventErrors |
    Where-Object { $_.TimeCreated -le $finished.AddSeconds(2) -and ($_.Id -in 1000,1001 -or $_.ToXml().Contains($payloadRoot)) } |
    Select-Object Id,TimeCreated,RecordId,Message)
ConvertTo-Json -InputObject $events -Depth 5 | Set-Content -LiteralPath (Join-Path $evidenceRoot 'scan_events.json') -Encoding utf8
$unchanged = $true
foreach ($file in $manifest) {
    $target = Join-Path $payloadRoot $file.path
    if (!(Test-Path -LiteralPath $target) -or (Get-FileHash -LiteralPath $target -Algorithm SHA256).Hash.ToLowerInvariant() -ne $file.sha256) { $unchanged=$false }
}
$detections = @($events | Where-Object { $_.Id -in 1116,1117 })
$result = [pscustomobject]@{
    started_utc=$started.ToUniversalTime().ToString('o');finished_utc=$finished.ToUniversalTime().ToString('o')
    scanner=$status;scanner_signature=$scannerSignature.Status.ToString();exclusion_exit=$exclusionExit;exclusions_verified=$exclusionVerified
    scan_exit=$scanExit;matching_detection_events=$detections.Count;payload_unchanged=$unchanged
    files=$manifest.Count;all_six_installed_hashes_match=(!$BuiltPayload);built_payload=[bool]$BuiltPayload;signatures=$signatures
    available_signing_certificates=$certificates.Count;game_launched=$false;external_upload=$false
    event_log_errors=@($scanEventErrors | ForEach-Object {$_.Exception.Message})
    scope=$(if ($BuiltPayload) {'Exact canonical built DLL and verified installed dependencies before deployment; no outer package or reputation test.'} else {'Exact installed extracted payload and dependencies; no outer installer/archive or download reputation test.'})
}
$result | ConvertTo-Json -Depth 7 | Set-Content -LiteralPath (Join-Path $evidenceRoot 'result.json') -Encoding utf8
if ($scanExit -ne 0 -or ($scanOutput | Out-String) -notmatch '(?i)found no threats' -or $detections.Count -ne 0 -or !$unchanged) { throw 'Scan requires investigation; do not call this payload clean or bypass security.' }
Write-Output "Defender reported no threats: $($manifest.Count) exact staged payload files; hashes unchanged. Exclusions verified: $exclusionVerified. Signing certificates: $($certificates.Count)."
