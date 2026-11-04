param([Parameter(Mandatory)][string]$Tool, [hashtable]$Arguments = @{})
$uri = 'http://127.0.0.1:8000/mcp'
$headers = @{Accept = 'application/json, text/event-stream'}
$init = @{jsonrpc='2.0'; id=1; method='initialize'; params=@{
    protocolVersion='2024-11-05'; capabilities=@{};
    clientInfo=@{name='wmcyn-diagnostics'; version='1'}
}} | ConvertTo-Json -Depth 12
$response = Invoke-WebRequest -Uri $uri -Method Post -ContentType application/json -Headers $headers -Body $init
$headers['Mcp-Session-Id'] = [string]($response.Headers['Mcp-Session-Id'] | Select-Object -First 1)
$body = @{jsonrpc='2.0'; id=2; method='tools/call'; params=@{name=$Tool; arguments=$Arguments}} | ConvertTo-Json -Depth 30
$result = Invoke-RestMethod -Uri $uri -Method Post -ContentType application/json -Headers $headers -Body $body
if ($result -is [string]) {
    $data = ($result -split "`n" | Where-Object { $_.StartsWith('data: ') }) -replace '^data: ', ''
    $result = ($data -join "`n") | ConvertFrom-Json
}
if ($result.error) { throw ($result.error | ConvertTo-Json -Compress) }
$result.result.content | Where-Object type -eq text | ForEach-Object { $_.text }
