param([switch]$Seed)
$ErrorActionPreference = 'Stop'
$content = Join-Path $PSScriptRoot '../../Content/WMCYN/BowlingPrivate'
$cache = Join-Path $env:ProgramData 'WMCYN/PrivateCookAssets'
foreach ($manifest in @('assets.sha256','fab.sha256')) {
if ($manifest -eq 'fab.sha256') { $content = Join-Path $PSScriptRoot '../../Content' }
foreach ($line in Get-Content (Join-Path $PSScriptRoot $manifest)) {
    if ($line -notmatch '^((?:[A-Za-z0-9_-]+/)*[A-Za-z0-9_-]+\.uasset) ([0-9A-F]{64})$') { throw 'Invalid private asset manifest' }
    $name, $hash = $Matches[1], $Matches[2]
    $asset = Join-Path $content $name
    $blob = Join-Path $cache $hash
    $src, $dst = if ($Seed) { $asset, $blob } else { $blob, $asset }
    if ((Test-Path -LiteralPath $dst) -and (Get-FileHash -LiteralPath $dst).Hash -eq $hash) { continue }
    if (!(Test-Path -LiteralPath $src) -or (Get-FileHash -LiteralPath $src).Hash -ne $hash) {
        throw "Missing matching private bowling content: $name. Seed this runner before cooking."
    }
    if (Test-Path -LiteralPath $dst) { throw "Refusing to overwrite mismatched local asset: $dst" }
    New-Item -ItemType Directory -Force (Split-Path $dst) | Out-Null
    Copy-Item -LiteralPath $src -Destination $dst
}
}
