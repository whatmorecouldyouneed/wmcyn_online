$ErrorActionPreference = 'Stop'
Import-Module Microsoft.PowerShell.Utility -ErrorAction Stop
. "$PSScriptRoot/Common.ps1"
$root = (Resolve-Path "$PSScriptRoot/../..").Path
$output = "$root/Saved/FounderRelease"
$manifest = Get-Content "$output/build-manifest.json" -Raw | ConvertFrom-Json
if ($manifest.commit -ne $env:GITHUB_SHA) { throw 'Manifest commit mismatch.' }
foreach ($file in $manifest.files) {
    if ((Get-Sha256Hex (Join-Path $output $file.path)) -ne $file.sha256) { throw 'Artifact changed after verification.' }
}
$apk = @(Get-ChildItem "$output/Android" -Recurse -Filter '*.apk')
$obb = @(Get-ChildItem "$output/Android" -Recurse -Filter '*.obb')
if ($apk.Count -ne 1 -or $obb.Count -gt 1) { throw 'Unsupported APK/expansion layout; review before upload.' }
if (-not $env:META_APP_ID -or -not $env:META_APP_SECRET) { throw 'Meta upload credentials missing.' }
$cli = 'C:/Tools/ovr-platform-util.exe'
if (-not (Test-Path $cli)) {
    $installed = Get-ChildItem "$env:LOCALAPPDATA/Microsoft/WinGet/Packages/Meta.OculusPlatformUtility_*" -Filter 'ovr-platform-util.exe' -Recurse -ErrorAction SilentlyContinue | Select-Object -First 1
    if ($installed) { $cli = $installed.FullName }
}
if (-not (Test-Path $cli)) { throw 'Meta platform utility missing.' }
$args = @('upload-quest-build','--app-id',$env:META_APP_ID,'--app-secret',$env:META_APP_SECRET,'--apk',$apk[0].FullName,'--channel','ALPHA','--age-group','TEENS_AND_ADULTS','--notes',"Commit $($manifest.commit) / $($manifest.configuration) / $($manifest.versionCode)")
if ($obb.Count -eq 1) { $args += @('--obb',$obb[0].FullName) }
& $cli @args
if ($LASTEXITCODE -ne 0) { throw 'Meta upload failed; do not announce availability.' }
