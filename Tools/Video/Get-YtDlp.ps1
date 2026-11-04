# Fetches the pinned yt-dlp release for PCVR and refuses anything but the expected file.
# Packaging calls this; run it by hand to resolve links in a local editor session.
param([string]$Destination = (Join-Path $PSScriptRoot '../../ThirdParty/yt-dlp'))
$ErrorActionPreference = 'Stop'
Import-Module Microsoft.PowerShell.Utility -ErrorAction Stop
. "$PSScriptRoot/../Build/Common.ps1"

$version = '2026.10.26'
$sha256 = 'F5389D3698ECDE4EF06F4E6B1CE0294FE9AC1396228C8990B8D3416D6D177A7D'
$tool = Join-Path $Destination 'yt-dlp.exe'
New-Item -ItemType Directory -Force -Path $Destination | Out-Null
if (-not (Test-Path $tool) -or (Get-Sha256Hex $tool) -ne $sha256) {
    [Net.ServicePointManager]::SecurityProtocol = [Net.SecurityProtocolType]::Tls12
    (New-Object Net.WebClient).DownloadFile("https://github.com/yt-dlp/yt-dlp/releases/download/$version/yt-dlp.exe", $tool)
}
if ((Get-Sha256Hex $tool) -ne $sha256) {
    Remove-Item $tool -Force
    throw "yt-dlp $version failed its checksum."
}
Write-Output $tool
