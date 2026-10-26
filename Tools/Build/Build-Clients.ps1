param(
    [ValidateSet('Both','Quest','PCVR')][string]$Target = 'Both',
    [ValidateSet('Development','Shipping')][string]$Configuration = 'Development',
    [bool]$PublishRequested = $false
)
$ErrorActionPreference = 'Stop'
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
}
$platforms = if ($Target -eq 'Both') { @('Win64','Android') } elseif ($Target -eq 'Quest') { @('Android') } else { @('Win64') }
foreach ($platform in $platforms) {
    $args = @('BuildCookRun', "-project=$project", "-platform=$platform", "-clientconfig=$Configuration", '-build','-cook','-stage','-pak','-package','-archive',"-archivedirectory=$output/$platform",'-unattended','-noP4','-utf8output')
    if ($platform -eq 'Android') { $args += '-cookflavor=ASTC' }
    & $uat @args
    if ($LASTEXITCODE -ne 0) { throw "$platform packaging failed ($LASTEXITCODE)." }
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
}
$hashes = @(Get-ChildItem $output -Recurse -File | ForEach-Object {
    @{ path = $_.FullName.Substring($output.Length + 1); sha256 = (Get-FileHash $_.FullName -Algorithm SHA256).Hash; bytes = $_.Length }
})
@{ commit=$sha; configuration=$Configuration; target=$Target; versionCode=$version; files=$hashes; hardwareAcceptance='pending'; serverCompatibility='requires verification' } |
    ConvertTo-Json -Depth 6 | Set-Content "$output/build-manifest.json"
