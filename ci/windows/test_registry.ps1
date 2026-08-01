# -----------------------------------------------------------------------------
# test_registry.ps1 - end-to-end test of fly-registry over real HTTP.
#
# Starts the server on an OS-chosen port (--port 0, the bound port is printed on
# stdout), then drives it with real requests over a real socket:
#   1. GET  /v1/mypkg              on an empty registry -> 404
#   2. POST /v1/mypkg/1.0.0        without a token      -> 401
#   3. POST /v1/mypkg/1.0.0        with the token       -> 201
#   4. GET  /v1/mypkg                                   -> ["1.0.0"]
#   5. GET  /v1/mypkg/1.0.0                             -> Manifest.fly content,
#                                                          extracted IN-PROCESS
#   6. GET  /v1/mypkg/1.0.0/download                    -> the tarball, byte-exact
#   7. GET  /v1/search?q=my                             -> ["mypkg"]
#   8. GET  /v1/..%2f..%2fetc                           -> 400 (path traversal)
#   9. POST of a non-gzip body                          -> 400
#
# The server serves ONE request per run (--once) so nothing is left listening;
# the storage directory carries state across the runs, which is what makes the
# publish→list→download sequence a real integration test rather than nine
# independent unit checks.
# -----------------------------------------------------------------------------
$ErrorActionPreference = 'Stop'
Set-Location (Resolve-Path (Join-Path $PSScriptRoot '..\..'))

$STAGE = if ($env:STAGE) { $env:STAGE } else { '2' }
$EXE = "build/stage$STAGE/bin/fly-registry.exe"
if (-not (Test-Path $EXE)) { Write-Host "error: '$EXE' not found - run build_registry.ps1 first."; exit 1 }
$EXE = (Resolve-Path $EXE).Path

$FIX = 'std/test/fixtures/regpkg.tar.gz'
if (-not (Test-Path $FIX)) { Write-Host "error: fixture '$FIX' not found."; exit 1 }
$tarball = [System.IO.File]::ReadAllBytes((Resolve-Path $FIX).Path)

$D = "build/stage$STAGE/registry_test"
Remove-Item $D -Recurse -Force -ErrorAction SilentlyContinue
New-Item -ItemType Directory -Force $D | Out-Null
$STORAGE = (Resolve-Path $D).Path
$TOKEN = 'secret-token'

$failed = 0
function Check([bool]$cond, [string]$what) {
    if ($cond) { Write-Host "  ok    $what" }
    else       { Write-Host "  FAIL  $what"; $script:failed = 1 }
}

# Serve exactly one request and return the raw response bytes.
function Invoke-Registry([byte[]]$RequestBytes) {
    $psi = New-Object System.Diagnostics.ProcessStartInfo
    $psi.FileName = $EXE
    $psi.Arguments = "--storage `"$STORAGE`" --host 127.0.0.1 --port 0 --token $TOKEN --once"
    $psi.RedirectStandardOutput = $true
    $psi.UseShellExecute = $false
    $p = [System.Diagnostics.Process]::Start($psi)

    # Wait for the "ready : port N" line, which is emitted after listen().
    $port = 0
    while ($true) {
        $line = $p.StandardOutput.ReadLine()
        if ($null -eq $line) { break }
        if ($line -match 'ready\s*:\s*port\s+(\d+)') { $port = [int]$Matches[1]; break }
    }
    if ($port -eq 0) { $p.Kill(); throw 'fly-registry never reported a port' }

    $client = New-Object System.Net.Sockets.TcpClient('127.0.0.1', $port)
    $ns = $client.GetStream()
    $ns.Write($RequestBytes, 0, $RequestBytes.Length)
    $ns.Flush()
    $ms = New-Object System.IO.MemoryStream
    $buf = New-Object byte[] 8192
    while ($true) {
        $n = $ns.Read($buf, 0, $buf.Length)
        if ($n -le 0) { break }
        $ms.Write($buf, 0, $n)
    }
    $client.Close()
    if (-not $p.WaitForExit(30000)) { $p.Kill(); throw 'fly-registry did not exit after --once' }
    return $ms.ToArray()
}

function Build-Request([string]$Method, [string]$Path, [hashtable]$Headers, [byte[]]$Body) {
    $head = "$Method $Path HTTP/1.1`r`nHost: 127.0.0.1`r`nConnection: close`r`n"
    if ($Headers) { foreach ($k in $Headers.Keys) { $head += "$k`: $($Headers[$k])`r`n" } }
    if ($Body) { $head += "Content-Length: $($Body.Length)`r`n" }
    $head += "`r`n"
    $bytes = [System.Text.Encoding]::ASCII.GetBytes($head)
    if ($Body) { $bytes += $Body }
    return $bytes
}

# Split a raw HTTP response into its status line and body bytes.
function Split-Response([byte[]]$Raw) {
    for ($i = 0; $i -lt $Raw.Length - 3; $i++) {
        if ($Raw[$i] -eq 13 -and $Raw[$i+1] -eq 10 -and $Raw[$i+2] -eq 13 -and $Raw[$i+3] -eq 10) {
            $head = [System.Text.Encoding]::ASCII.GetString($Raw, 0, $i)
            $bodyLen = $Raw.Length - ($i + 4)
            $body = New-Object byte[] $bodyLen
            [Array]::Copy($Raw, $i + 4, $body, 0, $bodyLen)
            return @{ Head = $head; Body = $body }
        }
    }
    return @{ Head = [System.Text.Encoding]::ASCII.GetString($Raw); Body = @() }
}

# 1. list versions on an empty registry
$r = Split-Response (Invoke-Registry (Build-Request 'GET' '/v1/mypkg' $null $null))
Check ($r.Head -match '404') 'GET an unknown package answers 404'

# 2. publish WITHOUT the token
$r = Split-Response (Invoke-Registry (Build-Request 'POST' '/v1/mypkg/1.0.0' $null $tarball))
Check ($r.Head -match '401') 'POST without a Bearer token answers 401'

# 3. publish WITH the token
$r = Split-Response (Invoke-Registry (Build-Request 'POST' '/v1/mypkg/1.0.0' @{ Authorization = "Bearer $TOKEN" } $tarball))
Check ($r.Head -match '201') 'POST with the Bearer token answers 201'
Check (Test-Path (Join-Path $STORAGE 'mypkg/1.0.0.tar.gz')) 'the tarball is stored under <storage>/<name>/<version>.tar.gz'
Check (Test-Path (Join-Path $STORAGE 'mypkg/1.0.0/Manifest.fly')) 'Manifest.fly was extracted in-process at publish'

# 4. list versions
$r = Split-Response (Invoke-Registry (Build-Request 'GET' '/v1/mypkg' $null $null))
$listBody = [System.Text.Encoding]::UTF8.GetString($r.Body)
Check ($r.Head -match '200' -and $listBody -eq '["1.0.0"]') "GET /v1/mypkg lists the version (got $listBody)"

# 5. metadata
$r = Split-Response (Invoke-Registry (Build-Request 'GET' '/v1/mypkg/1.0.0' $null $null))
$meta = [System.Text.Encoding]::UTF8.GetString($r.Body)
Check ($r.Head -match '200' -and $meta -match 'name\s*=\s*"mypkg"') 'GET metadata returns the Manifest.fly content'

# 6. download - byte-exact round trip of a BINARY payload
$r = Split-Response (Invoke-Registry (Build-Request 'GET' '/v1/mypkg/1.0.0/download' $null $null))
$same = ($r.Body.Length -eq $tarball.Length)
if ($same) { for ($i = 0; $i -lt $tarball.Length; $i++) { if ($r.Body[$i] -ne $tarball[$i]) { $same = $false; break } } }
Check ($r.Head -match '200' -and $same) "download returns the tarball byte-for-byte ($($r.Body.Length)/$($tarball.Length) bytes)"

# 7. search
$r = Split-Response (Invoke-Registry (Build-Request 'GET' '/v1/search?q=my' $null $null))
$searchBody = [System.Text.Encoding]::UTF8.GetString($r.Body)
Check ($r.Head -match '200' -and $searchBody -eq '["mypkg"]') "search finds the package (got $searchBody)"

# 8. path traversal must not escape the storage directory
$r = Split-Response (Invoke-Registry (Build-Request 'GET' '/v1/..%2f..%2fetc' $null $null))
Check ($r.Head -match '400') 'a path-traversal name is rejected with 400'

# 9. a non-gzip body is refused rather than stored
$junk = [System.Text.Encoding]::ASCII.GetBytes('this is not a gzip archive at all')
$r = Split-Response (Invoke-Registry (Build-Request 'POST' '/v1/other/9.9.9' @{ Authorization = "Bearer $TOKEN" } $junk))
Check ($r.Head -match '400') 'a non-gzip publish body is rejected with 400'
Check (-not (Test-Path (Join-Path $STORAGE 'other/9.9.9.tar.gz'))) 'the rejected upload left nothing on disk'

if ($failed -ne 0) { Write-Host 'test_registry: FAILED'; exit 1 }
Write-Host 'test_registry: all checks passed'
exit 0
