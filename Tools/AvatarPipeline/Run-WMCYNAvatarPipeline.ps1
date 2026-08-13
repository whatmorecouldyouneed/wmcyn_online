param(
    [string]$EngineRoot = "C:\UE58",
    [string]$ProjectPath = "C:\Users\jvred\OneDrive\Documents\Unreal Projects\wmcyn_online\wmcyn_online.uproject",
    [ValidateSet("identity", "character", "runtime", "full")]
    [string]$Phase = "full",
    [string]$TakeZipPath,
    [string]$FootagePath,
    [string]$IdentityName = "WMCYNAvatarIdentity",
    [string]$IdentityPath,
    [string]$CharacterName,
    [int]$NeutralFrame = 0,
    [int]$BodyIndex = 1,
    [string]$StoragePath = "/Game/WMCYN/AvatarPipeline",
    [string]$CharacterPackagePath = "/Game/MetaHumans/WMCYN",
    [string]$BuildPath = "/Game/MetaHumans/WMCYN",
    [string]$CommonPath = "/Game/MetaHumans/Common",
    [string]$RuntimePath = "/Game/WMCYN/AvatarPipeline/Runtime",
    [string]$RuntimeQuestPath,
    [string]$RuntimePCVRPath,
    [string]$AnimationClassPath,
    [ValidateSet("low", "medium", "high")]
    [string]$PipelineQuality = "medium",
    [switch]$SkipTextures,
    [switch]$SkipBuild,
    [switch]$Unattended
)

$ErrorActionPreference = "Stop"

$script:RepoRoot = Split-Path -Parent (Split-Path -Parent $PSScriptRoot)
$pythonScript = Join-Path $PSScriptRoot "wmcyn_metahuman_pipeline.py"
$editorPath = Join-Path $EngineRoot "Engine\Binaries\Win64\UnrealEditor.exe"
$savedInputsRoot = Join-Path $script:RepoRoot "Saved\AvatarPipeline\Inputs"
$logPath = Join-Path $script:RepoRoot "Saved\Logs\wmcyn_online.log"

function Get-ResultFilePath {
    param(
        [string]$IdentityName,
        [string]$Phase
    )

    $safeIdentityName = [Regex]::Replace($IdentityName.Trim(), "[^A-Za-z0-9_]+", "_").Trim("_")
    if (-not $safeIdentityName) {
        $safeIdentityName = "WMCYNAvatar"
    }

    $safePhase = [Regex]::Replace($Phase.Trim(), "[^A-Za-z0-9_]+", "_").Trim("_")
    if (-not $safePhase) {
        $safePhase = "result"
    }

    return Join-Path $script:RepoRoot "Saved\AvatarPipeline\Results\$safeIdentityName`_$safePhase.json"
}

function Update-ResultFileFailure {
    param(
        [string]$ResultFilePath,
        [string]$ErrorMessage
    )

    if (-not (Test-Path -LiteralPath $ResultFilePath)) {
        return
    }

    try {
        $json = Get-Content -LiteralPath $ResultFilePath -Raw | ConvertFrom-Json
        $json.success = $false
        if ($json.PSObject.Properties.Name -contains "error") {
            $json.error = $ErrorMessage
        } else {
            $json | Add-Member -NotePropertyName error -NotePropertyValue $ErrorMessage -Force
        }
        $json | ConvertTo-Json -Depth 20 | Set-Content -LiteralPath $ResultFilePath -Encoding utf8
    } catch {
        Write-Warning "Could not rewrite avatar pipeline result file '$ResultFilePath': $_"
    }
}

function Test-CharacterPhaseSucceeded {
    param(
        [string]$LogPath,
        [datetime]$StartedAt,
        [string]$ResultFilePath
    )

    if (-not (Test-Path -LiteralPath $LogPath)) {
        return
    }

    $logContent = Get-Content -LiteralPath $LogPath -Raw
    $failurePatterns = @(
        "Auto-Rigging of Face failed",
        "not ready for assembly: Character is not rigged",
        "User not logged in, please autorig",
        "Login failed - error code:",
        "Polling token grant failed"
    )

    $matchedFailure = $failurePatterns | Where-Object { $logContent -match [Regex]::Escape($_) } | Select-Object -First 1
    if (-not $matchedFailure) {
        return
    }

    $errorMessage = "Character phase failed verification: $matchedFailure"
    Update-ResultFileFailure -ResultFilePath $ResultFilePath -ErrorMessage $errorMessage
    throw $errorMessage
}

function Get-ExtractedFootagePath {
    param([string]$ZipPath)

    if (-not (Test-Path -LiteralPath $ZipPath)) {
        throw "Take zip not found: $ZipPath"
    }

    New-Item -ItemType Directory -Force -Path $savedInputsRoot | Out-Null

    $zipBaseName = [IO.Path]::GetFileNameWithoutExtension($ZipPath)
    $timestamp = Get-Date -Format "yyyyMMdd_HHmmss"
    $extractPath = Join-Path $savedInputsRoot "$zipBaseName`_$timestamp"
    New-Item -ItemType Directory -Force -Path $extractPath | Out-Null

    Add-Type -AssemblyName System.IO.Compression.FileSystem
    [IO.Compression.ZipFile]::ExtractToDirectory($ZipPath, $extractPath)

    $childDirectories = Get-ChildItem -LiteralPath $extractPath -Directory
    if ($childDirectories.Count -eq 1) {
        return $childDirectories[0].FullName
    }

    return $extractPath
}

if (-not (Test-Path -LiteralPath $pythonScript)) {
    throw "Pipeline script not found: $pythonScript"
}

if (-not (Test-Path -LiteralPath $editorPath)) {
    throw "UnrealEditor.exe not found: $editorPath"
}

if (-not (Test-Path -LiteralPath $ProjectPath)) {
    throw "Project path not found: $ProjectPath"
}

if ($Phase -in @("identity", "full")) {
    if (-not $FootagePath) {
        if (-not $TakeZipPath) {
            throw "Provide either -FootagePath or -TakeZipPath."
        }
        $FootagePath = Get-ExtractedFootagePath -ZipPath $TakeZipPath
    }

    if (-not (Test-Path -LiteralPath $FootagePath)) {
        throw "Footage path not found: $FootagePath"
    }
}

$env:WMCYN_AVATAR_PHASE = $Phase
if ($FootagePath) {
    $env:WMCYN_AVATAR_FOOTAGE_PATH = $FootagePath
} else {
    Remove-Item Env:WMCYN_AVATAR_FOOTAGE_PATH -ErrorAction SilentlyContinue
}
$env:WMCYN_AVATAR_STORAGE_PATH = $StoragePath
$env:WMCYN_AVATAR_IDENTITY_NAME = $IdentityName
$env:WMCYN_AVATAR_NEUTRAL_FRAME = [string]$NeutralFrame
$env:WMCYN_AVATAR_BODY_INDEX = [string]$BodyIndex
$env:WMCYN_AVATAR_CHARACTER_PACKAGE = $CharacterPackagePath
$env:WMCYN_AVATAR_BUILD_PATH = $BuildPath
$env:WMCYN_AVATAR_COMMON_PATH = $CommonPath
$env:WMCYN_AVATAR_RUNTIME_PATH = $RuntimePath
$env:WMCYN_AVATAR_PIPELINE_QUALITY = $PipelineQuality
$env:WMCYN_AVATAR_SKIP_TEXTURES = if ($SkipTextures) { "true" } else { "false" }
$env:WMCYN_AVATAR_SKIP_BUILD = if ($SkipBuild) { "true" } else { "false" }

if ($IdentityPath) {
    $env:WMCYN_AVATAR_IDENTITY_PATH = $IdentityPath
}

if ($CharacterName) {
    $env:WMCYN_AVATAR_CHARACTER_NAME = $CharacterName
}

if ($RuntimeQuestPath) {
    $env:WMCYN_AVATAR_RUNTIME_QUEST_PATH = $RuntimeQuestPath
}

if ($RuntimePCVRPath) {
    $env:WMCYN_AVATAR_RUNTIME_PCVR_PATH = $RuntimePCVRPath
}

if ($AnimationClassPath) {
    $env:WMCYN_AVATAR_ANIMATION_CLASS_PATH = $AnimationClassPath
}

$arguments = @(
    $ProjectPath,
    "-ExecutePythonScript=$pythonScript",
    "-log",
    "-stdout",
    "-FullStdOutLogOutput"
)

if ($Unattended) {
    $arguments += "-unattended"
}

Write-Host "WMCYN Avatar Pipeline"
Write-Host "  Phase:          $Phase"
Write-Host "  Footage path:   $FootagePath"
Write-Host "  Identity name:  $IdentityName"
Write-Host "  Character name: $CharacterName"
Write-Host "  Unreal:         $editorPath"
Write-Host ""

$startedAt = Get-Date
& $editorPath @arguments

if ($LASTEXITCODE -ne 0) {
    throw "UnrealEditor exited with code $LASTEXITCODE"
}

if ($Phase -in @("character", "full")) {
    $resultFilePath = Get-ResultFilePath -IdentityName $IdentityName -Phase "character"
    Test-CharacterPhaseSucceeded -LogPath $logPath -StartedAt $startedAt -ResultFilePath $resultFilePath
}
