param([Parameter(Mandatory=$true)][ValidatePattern('^[0-9a-f]{40}$')][string]$Commit)
$ErrorActionPreference = 'Stop'
Set-Location (Join-Path $PSScriptRoot '../..')
function Assert-Tip {
    if ((gh api repos/whatmorecouldyouneed/wmcyn_online/commits/main --jq .sha) -ne $Commit) { throw 'Release no longer matches main' }
}
function Wait-Run($workflow, $since) {
    $deadline = (Get-Date).AddHours(6)
    do {
        $runs = gh run list -w $workflow --branch main --commit $Commit --limit 8 --json databaseId,status,conclusion,createdAt,url | ConvertFrom-Json
        $run = $runs | Where-Object { ([datetimeoffset]$_.createdAt) -ge $since } | Select-Object -First 1
        if ($run.status -eq 'completed') {
            if ($run.conclusion -ne 'success') { throw "$workflow $($run.conclusion): $($run.url)" }
            Write-Host "Verified $($run.url)"
            return $run.databaseId
        }
        Start-Sleep 30
    } while ((Get-Date) -lt $deadline)
    throw "Timed out waiting for $workflow"
}
Assert-Tip
$client = Wait-Run 'build-clients.yml' ([datetimeoffset]::MinValue)
Assert-Tip
$start = [datetimeoffset]::UtcNow.AddSeconds(-2)
gh workflow run build-server.yml --ref main -f deploy=true
if ($LASTEXITCODE) { throw 'Server dispatch failed' }
$null = Wait-Run 'build-server.yml' $start
Assert-Tip
$start = [datetimeoffset]::UtcNow.AddSeconds(-2)
gh workflow run publish-quest-alpha.yml --ref main -f "run_id=$client" -f "commit=$Commit"
if ($LASTEXITCODE) { throw 'Meta publish dispatch failed' }
$null = Wait-Run 'publish-quest-alpha.yml' $start
Write-Host 'Matched release and Meta ALPHA upload completed.'
