# Shared by Build-Clients.ps1 and Publish-QuestAlpha.ps1. Both must produce the
# same hash format or publication rejects artifacts it should accept.

# Get-FileHash lives in Microsoft.PowerShell.Utility and failed to autoload
# inside the Actions step, seven minutes into a build. Hash through .NET so it
# cannot depend on module discovery. Uppercase hex matches Get-FileHash output.
function Get-Sha256Hex {
    param([Parameter(Mandatory)][string]$Path)
    $sha = [System.Security.Cryptography.SHA256]::Create()
    try {
        $stream = [IO.File]::OpenRead($Path)
        try {
            [BitConverter]::ToString($sha.ComputeHash($stream)).Replace('-', '')
        } finally { $stream.Dispose() }
    } finally { $sha.Dispose() }
}
