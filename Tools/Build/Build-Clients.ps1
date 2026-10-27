param(
    [ValidateSet('Both','Quest','PCVR')][string]$Target = 'Both',
    [ValidateSet('Development','Shipping')][string]$Configuration = 'Development',
    [bool]$PublishRequested = $false
)
$ErrorActionPreference = 'Stop'
# Fail in the first second if Utility cannot load, not seven minutes in.
Import-Module Microsoft.PowerShell.Utility -ErrorAction Stop
. "$PSScriptRoot/Common.ps1"
$root = (Resolve-Path "$PSScriptRoot/../..").Path
if (-not $env:GITHUB_WORKSPACE -or -not $root.StartsWith([IO.Path]::GetFullPath($env:GITHUB_WORKSPACE), [StringComparison]::OrdinalIgnoreCase)) {
    throw 'Run in the dedicated Actions checkout, not the interactive project.'
}
if ($PublishRequested -and $Target -eq 'PCVR') { throw 'ALPHA publication requires Quest.' }
$project = Join-Path $root 'wmcyn_online.uproject'
$uat = 'C:/Program Files/Epic Games/UE_5.8/Engine/Build/BatchFiles/RunUAT.bat'
if (-not (Test-Path $uat)) { throw 'Matching Launcher UE 5.8 is missing.' }
$sha = (& git -C $root rev-parse HEAD).Trim()
if ($LASTEXITCODE -ne 0 -or $sha -ne $env:GITHUB_SHA) { throw 'Checkout does not match requested commit.' }
& git -C $root lfs fsck
if ($LASTEXITCODE -ne 0) { throw 'Git LFS validation failed.' }
$output = Join-Path $root 'Saved/FounderRelease'
if (Test-Path $output) { throw 'Release output already exists; use a fresh checkout.' }
New-Item -ItemType Directory -Path $output -Force | Out-Null
foreach ($key in 'ANDROID_HOME','JAVA_HOME','NDKROOT','NDK_ROOT') {
    $value = [Environment]::GetEnvironmentVariable($key, 'User')
    if ($value) { [Environment]::SetEnvironmentVariable($key, $value, 'Process') }
}
$version = [DateTimeOffset]::UtcNow.ToUnixTimeSeconds()
# Quest-only validation facts; stay null for PCVR builds.
$apkBytes = $null; $sizeState = $null; $fingerprint = $null; $pcvrArchive = $null; $obbBytes = $null; $obbName = $null
if ($version -gt 2100000000) { throw 'Android version allocation requires migration.' }
if ($Target -ne 'PCVR') {
    foreach ($path in @($env:WMCYN_SIGNING_INI, $env:WMCYN_KEYSTORE)) {
        if (-not $path -or -not (Test-Path -LiteralPath $path)) { throw 'Configure runner signing file paths first.' }
    }
    if (-not $env:WMCYN_CERT_SHA256) { throw 'Pin the existing Meta signing certificate SHA256 first.' }
    New-Item -ItemType Directory -Path "$root/Config/Android", "$root/Build/Android" -Force | Out-Null
    Copy-Item -LiteralPath $env:WMCYN_KEYSTORE -Destination "$root/Build/Android/wmcyn-release.keystore"
    $ini = Get-Content -LiteralPath $env:WMCYN_SIGNING_INI -Raw
    if ($ini -notmatch '(?m)^StoreVersion=') { throw 'Signing override must contain StoreVersion.' }
    if ($ini -notmatch '(?m)^KeyStore=wmcyn-release\.keystore\s*$') { throw 'Signing override must reference wmcyn-release.keystore.' }
    $ini = $ini -replace '(?m)^StoreVersion=.*', "StoreVersion=$version"
    [IO.File]::WriteAllText("$root/Config/Android/AndroidEngine.ini", $ini)
    # AndroidFileServer is a USB file-serving dev tool CI never uses. Leaving it
    # enabled breaks Shipping: AndroidFileServer_UPL.xml forces bCompileAFSProject
    # back on when Configuration=Shipping and bIncludeInShipping=False, then runs a
    # bare "gradlew.bat" in AFSProject, which has no wrapper. Fails after the cook.
    $afs = @('', '[/Script/AndroidFileServerEditor.AndroidFileServerRuntimeSettings]', 'bEnablePlugin=False', '') -join "`r`n"
    [IO.File]::AppendAllText("$root/Config/Android/AndroidEngine.ini", $afs)
}
$platforms = if ($Target -eq 'Both') { @('Win64','Android') } elseif ($Target -eq 'Quest') { @('Android') } else { @('Win64') }
foreach ($platform in $platforms) {
    $args = @('BuildCookRun', "-project=$project", "-platform=$platform", "-clientconfig=$Configuration", '-build','-cook','-stage','-pak','-package','-archive',"-archivedirectory=$output/$platform",'-unattended','-noP4','-utf8output')
    if ($platform -eq 'Android') { $args += '-cookflavor=ASTC' }
    & $uat @args
    if ($LASTEXITCODE -ne 0) { throw "$platform packaging failed ($LASTEXITCODE)." }
}
if ($Target -ne 'Quest') {
    # Spec deliverable is a single packaged ZIP, not a loose staged tree.
    $pcvrDir = Join-Path $output 'Win64'
    if (-not (Test-Path $pcvrDir)) { throw 'PCVR archive directory is missing.' }
    $pcvrArchive = "wmcyn_online-PCVR-$Configuration-$($sha.Substring(0,12)).zip"
    Add-Type -AssemblyName System.IO.Compression.FileSystem
    [IO.Compression.ZipFile]::CreateFromDirectory($pcvrDir, (Join-Path $output $pcvrArchive), [IO.Compression.CompressionLevel]::Optimal, $false)
    $zipItem = Get-Item (Join-Path $output $pcvrArchive) -ErrorAction SilentlyContinue
    if (-not $zipItem -or $zipItem.Length -eq 0) { throw 'PCVR archive creation failed.' }
    Remove-Item $pcvrDir -Recurse -Force
}
if ($Target -ne 'PCVR') {
    $apk = @(Get-ChildItem "$output/Android" -Recurse -Filter '*.apk')
    if ($apk.Count -ne 1) { throw 'Expected exactly one Quest APK.' }
    $signer = "$env:LOCALAPPDATA/Android/Sdk/build-tools/36.0.0/apksigner.bat"
    # Development cooks can select Unreal's debug signing configuration.
    # Re-sign explicitly without placing passwords in process arguments/logs.
    $signing = @{}
    Get-Content -LiteralPath $env:WMCYN_SIGNING_INI | ForEach-Object {
        if ($_ -match '^([^;\[=]+)=(.*)$') { $signing[$matches[1].Trim()] = $matches[2].Trim() }
    }
    $env:WMCYN_STORE_PASSWORD = $signing['KeyStorePassword']
    $env:WMCYN_KEY_PASSWORD = $signing['KeyPassword']
    if (-not $env:WMCYN_KEY_PASSWORD -or $env:WMCYN_KEY_PASSWORD -eq '_sameaskeystore_') { $env:WMCYN_KEY_PASSWORD = $env:WMCYN_STORE_PASSWORD }
    if (-not $env:WMCYN_STORE_PASSWORD -or -not $signing['KeyAlias']) { throw 'Incomplete signing configuration.' }
    try {
        & $signer sign --ks "$root/Build/Android/wmcyn-release.keystore" --ks-key-alias $signing['KeyAlias'] --ks-pass env:WMCYN_STORE_PASSWORD --key-pass env:WMCYN_KEY_PASSWORD --v1-signing-enabled false --v2-signing-enabled true --v3-signing-enabled false --v4-signing-enabled false $apk[0].FullName
        if ($LASTEXITCODE -ne 0) { throw 'APK release signing failed.' }
    } finally {
        Remove-Item Env:WMCYN_STORE_PASSWORD, Env:WMCYN_KEY_PASSWORD -ErrorAction SilentlyContinue
        $signing.Clear()
    }
    $report = & $signer verify --verbose --print-certs $apk[0].FullName 2>&1
    if ($LASTEXITCODE -ne 0) { throw 'APK signature verification failed.' }
    $text = $report -join "`n"
    if ($text -match 'CN=Android Debug' -or $text -notmatch 'v2 scheme .*: true') { throw 'Release certificate and v2 signing required.' }
    $fingerprint = [regex]::Match($text, 'certificate SHA-256 digest: ([a-fA-F0-9]+)').Groups[1].Value
    if ($fingerprint -ne ($env:WMCYN_CERT_SHA256 -replace ':','').Trim()) { throw 'APK certificate does not match pinned Meta certificate.' }
    $report | Set-Content "$output/signature.txt"

    # VRC.Quest.Packaging.5 caps the APK at 1 GB. Meta documents "1 GB" without
    # stating binary or decimal, so fail only above 1 GiB (certain rejection)
    # and flag the band between 1e9 and 1 GiB for a human decision on OBB.
    # Re-stat: the FileInfo was created before apksigner rewrote the APK, so
    # the cached Length understates it by the signature block.
    $apk[0].Refresh()
    $apkBytes = $apk[0].Length
    $limitBinary = 1073741824
    $limitDecimal = 1000000000
    if ($apkBytes -gt $limitBinary) {
        throw ("Quest APK is {0:N0} bytes, over Meta's 1 GB cap ({1:N0}). Move content to an OBB expansion file." -f $apkBytes, $limitBinary)
    }
    $sizeState = if ($apkBytes -gt $limitDecimal) { 'within-1GiB-but-over-decimal-1GB' } else { 'ok' }
    if ($sizeState -ne 'ok') {
        Write-Warning ("Quest APK is {0:N0} bytes: under 1 GiB by {1:N0} but over decimal 1 GB. Upload may be rejected; prefer an OBB." -f $apkBytes, ($limitBinary - $apkBytes))
    }

    # Data ships outside the APK, so an expansion file must exist and must carry
    # the exact name Meta expects: main.<versionCode>.<package>.obb. A mismatch
    # is rejected at upload, so fail here instead.
    $engineIni = Get-Content -LiteralPath "$root/Config/DefaultEngine.ini" -Raw
    $packageName = [regex]::Match($engineIni, '(?m)^PackageName=(.+)$').Groups[1].Value.Trim()
    $dataInsideApk = [regex]::Match($engineIni, '(?m)^bPackageDataInsideApk=(\w+)$').Groups[1].Value
    $obb = @(Get-ChildItem "$output/Android" -Recurse -Filter '*.obb')
    if ($obb.Count -gt 1) { throw 'More than one expansion file; review the package layout before upload.' }
    if ($dataInsideApk -eq 'False' -and $obb.Count -eq 0) {
        throw 'Data is configured outside the APK but no expansion file was produced.'
    }
    if ($obb.Count -eq 1) {
        $obbBytes = $obb[0].Length
        $obbName = $obb[0].Name
        if ($obbBytes -gt 4294967296) {
            throw ("Expansion file is {0:N0} bytes, over Meta's 4 GB cap." -f $obbBytes)
        }
        $expectedObb = "main.$version.$packageName.obb"
        if ($obbName -ne $expectedObb) {
            throw "Expansion file is named $obbName but Meta requires $expectedObb."
        }
    }
}
# Protocol and build identifiers come from the shipped world contract, so the
# manifest cannot drift from what the client actually negotiates with the server.
$contractPath = "$root/Plugins/WMCYNRuntime/Source/WMCYNRuntime/Public/WMCYNFirstSignalWorldContract.h"
$contract = Get-Content -LiteralPath $contractPath -Raw
$protocolVersion = [regex]::Match($contract, 'ProtocolVersion\s*=\s*(\d+)').Groups[1].Value
$buildId = [regex]::Match($contract, 'DefaultBuildId\s*=\s*TEXT\("([^"]+)"\)').Groups[1].Value
$worldCapacity = [regex]::Match($contract, 'Capacity\s*=\s*(\d+)').Groups[1].Value
if (-not $protocolVersion -or -not $buildId) { throw 'Could not read protocol identifiers from the world contract.' }

$notes = @(
    "# WMCYN Online founder build",
    "",
    "Commit: $sha",
    "Configuration: $Configuration",
    "Target: $Target",
    "Android versionCode: $version",
    "World protocol version: $protocolVersion",
    "Default build id: $buildId",
    "World capacity: $worldCapacity",
    ""
)
if ($apkBytes) {
    $notes += @(
        "## Quest",
        "",
        "APK size: $apkBytes bytes ($sizeState)",
        "Signing certificate SHA-256: $fingerprint",
        "",
        $(if ($obbName) { "Sideload: see Expansion file below. The APK alone is not enough." } else { "Sideload:" }),
        "",
        $(if ($obbName) { "" } else { '    adb install -r <apk>' }),
        "",
        "The server must accept world protocol $protocolVersion. A client and server on",
        "different protocol versions will not connect.",
        ""
    )
}
if ($obbName) {
    # An expansion file is not installed by adb install. Pushing it to the wrong
    # path, or renaming it, gives a client that launches to an empty world.
    $notes += @(
        "## Expansion file",
        "",
        "Game data is in $obbName ($obbBytes bytes), not inside the APK.",
        "A channel install from Meta delivers it automatically. A sideload does not:",
        "",
        "    adb install -r <apk>",
        "    adb shell mkdir -p /sdcard/Android/obb/$packageName",
        "    adb push $obbName /sdcard/Android/obb/$packageName/",
        "",
        "Do not rename it. The APK looks for this exact filename and launches to an",
        "empty world without it.",
        ""
    )
}
if ($pcvrArchive) {
    $notes += @("## PCVR", "", "Unzip $pcvrArchive and run the packaged executable.", "")
}
$notes += @(
    "## Status",
    "",
    "Hardware acceptance: pending. Packaging success is not device, login, voice or",
    "multi-user validation. See build-manifest.json for per-file checksums.",
    ""
)
[IO.File]::WriteAllLines("$output/INSTALL.md", $notes)

$hashes = @(Get-ChildItem $output -Recurse -File | ForEach-Object {
    @{ path = $_.FullName.Substring($output.Length + 1); sha256 = (Get-Sha256Hex $_.FullName); bytes = $_.Length }
})
@{ commit=$sha; configuration=$Configuration; target=$Target; versionCode=$version; files=$hashes;
   questApkBytes=$apkBytes; questApkSizeState=$sizeState; certificateSha256=$fingerprint;
   protocolVersion=$protocolVersion; buildId=$buildId; worldCapacity=$worldCapacity; pcvrArchive=$pcvrArchive;
   questObbBytes=$obbBytes; questObbName=$obbName;
   hardwareAcceptance='pending'; serverCompatibility='requires verification' } |
    ConvertTo-Json -Depth 6 | Set-Content "$output/build-manifest.json"
