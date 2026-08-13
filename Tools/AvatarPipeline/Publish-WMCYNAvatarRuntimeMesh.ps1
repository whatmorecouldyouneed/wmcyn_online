param(
    [Parameter(Mandatory = $true)]
    [string]$JobId,
    [Parameter(Mandatory = $true)]
    [string]$QuestSkeletalMeshPath,
    [string]$QuestHeadSkeletalMeshPath,
    [string]$PCVRSkeletalMeshPath,
    [string]$PCVRHeadSkeletalMeshPath,
    [string]$QuestAnimClassPath = "/Game/WMCYN/AvatarPipeline/Runtime/WMCYNAvatar_52b64e54/ABP_WMCYN_MetaHuman_Presentation.ABP_WMCYN_MetaHuman_Presentation_C",
    [string]$PCVRAnimClassPath = "/Game/WMCYN/AvatarPipeline/Runtime/WMCYNAvatar_52b64e54/ABP_WMCYN_MetaHuman_Presentation.ABP_WMCYN_MetaHuman_Presentation_C",
    [string]$QuestAssetRoot,
    [string]$PCVRAssetRoot,
    [string]$BackendRepoPath = "C:\Users\jvred\Documents\WMCYN\wmcyn-backend-infra",
    [string]$ProjectId = "wmcyn-online-mobile"
)

$ErrorActionPreference = "Stop"

function Get-AssetRootFromObjectPath {
    param([string]$ObjectPath)

    $trimmed = $ObjectPath.Trim()
    if (-not $trimmed) {
        return ""
    }

    $beforeObject = $trimmed.Split(".")[0]
    $lastSlash = $beforeObject.LastIndexOf("/")
    if ($lastSlash -lt 0) {
        return $beforeObject
    }
    return $beforeObject.Substring(0, $lastSlash)
}

function Resolve-ProjectAssetFile {
    param(
        [string]$ObjectPath,
        [string]$RepoRoot
    )

    $trimmed = $ObjectPath.Trim()
    if (-not $trimmed.StartsWith("/Game/")) {
        return $null
    }

    $packagePath = $trimmed.Split(".")[0]
    $relativeContentPath = $packagePath.Substring("/Game/".Length).Replace("/", "\") + ".uasset"
    return Join-Path $RepoRoot "Content\$relativeContentPath"
}

function Assert-ProjectAssetExists {
    param(
        [string]$ObjectPath,
        [string]$RepoRoot,
        [string]$Label
    )

    $assetFile = Resolve-ProjectAssetFile -ObjectPath $ObjectPath -RepoRoot $RepoRoot
    if (-not $assetFile) {
        return
    }

    if (-not (Test-Path -LiteralPath $assetFile)) {
        throw "$Label does not exist in project content: $ObjectPath (expected file: $assetFile)"
    }
}

if (-not $PCVRSkeletalMeshPath) {
    $PCVRSkeletalMeshPath = $QuestSkeletalMeshPath
}

if (-not $QuestAssetRoot) {
    $QuestAssetRoot = Get-AssetRootFromObjectPath -ObjectPath $QuestSkeletalMeshPath
}

if (-not $PCVRAssetRoot) {
    $PCVRAssetRoot = Get-AssetRootFromObjectPath -ObjectPath $PCVRSkeletalMeshPath
}

$publishTool = Join-Path $BackendRepoPath "functions\tools\publish-avatar-runtime-mesh.mjs"
if (-not (Test-Path -LiteralPath $publishTool)) {
    throw "Publish tool not found: $publishTool"
}

$repoRoot = Split-Path -Parent (Split-Path -Parent $PSScriptRoot)
Assert-ProjectAssetExists -ObjectPath $QuestSkeletalMeshPath -RepoRoot $repoRoot -Label "Quest skeletal mesh"
Assert-ProjectAssetExists -ObjectPath $PCVRSkeletalMeshPath -RepoRoot $repoRoot -Label "PCVR skeletal mesh"
if ($QuestHeadSkeletalMeshPath) {
    Assert-ProjectAssetExists -ObjectPath $QuestHeadSkeletalMeshPath -RepoRoot $repoRoot -Label "Quest head skeletal mesh"
}
if ($PCVRHeadSkeletalMeshPath) {
    Assert-ProjectAssetExists -ObjectPath $PCVRHeadSkeletalMeshPath -RepoRoot $repoRoot -Label "PCVR head skeletal mesh"
}
Assert-ProjectAssetExists -ObjectPath $QuestAnimClassPath -RepoRoot $repoRoot -Label "Quest anim class"
Assert-ProjectAssetExists -ObjectPath $PCVRAnimClassPath -RepoRoot $repoRoot -Label "PCVR anim class"

$arguments = @(
    $publishTool,
    "--project", $ProjectId,
    "--job-id", $JobId,
    "--quest-asset-root", $QuestAssetRoot,
    "--quest-skeletal-mesh-path", $QuestSkeletalMeshPath,
    "--quest-anim-class-path", $QuestAnimClassPath,
    "--pcvr-asset-root", $PCVRAssetRoot,
    "--pcvr-skeletal-mesh-path", $PCVRSkeletalMeshPath,
    "--pcvr-anim-class-path", $PCVRAnimClassPath
)

if ($QuestHeadSkeletalMeshPath) {
    $arguments += @("--quest-head-skeletal-mesh-path", $QuestHeadSkeletalMeshPath)
}
if ($PCVRHeadSkeletalMeshPath) {
    $arguments += @("--pcvr-head-skeletal-mesh-path", $PCVRHeadSkeletalMeshPath)
}

Push-Location $BackendRepoPath
try {
    node @arguments
}
finally {
    Pop-Location
}
