# -----------------------------------------------------------------------------
# test_lsp.ps1 - end-to-end smoke test of fly-lsp over a scripted stdio session.
#
# Drives build\stage$STAGE\bin\fly-lsp.exe through a real LSP conversation and
# asserts on the raw framed output:
#   1. initialize        -> capabilities + serverInfo, id echoed
#   2. didOpen broken    -> publishDiagnostics carrying the REAL sema error
#   3. didOpen clean     -> publishDiagnostics with an empty list
#   4. didClose broken   -> publishDiagnostics clearing the file
#   5. unknown request   -> error -32601 (a client promise must never hang)
#   6. shutdown/exit     -> null result, process exits 0
#
# The transport is exercised for real: frames are composed as exact bytes and
# the output is checked for \r\n\r\n terminators - the thing a text-mode fd
# silently corrupts on Windows, which is the platform this file guards.
# -----------------------------------------------------------------------------
$ErrorActionPreference = 'Stop'
Set-Location (Resolve-Path (Join-Path $PSScriptRoot '..\..'))

$STAGE = if ($env:STAGE) { $env:STAGE } else { '2' }
$EXE = "build/stage$STAGE/bin/fly-lsp.exe"
if (-not (Test-Path $EXE)) { Write-Host "error: '$EXE' not found - run build_lsp.ps1 first."; exit 1 }

$D = "build/stage$STAGE/lsp_test"
New-Item -ItemType Directory -Force $D | Out-Null
$DirAbs = (Resolve-Path $D).Path

# Two fixtures: one with a genuine sema error, one clean.
@"
import fly.str

void main() {
    int n = fly.str.len("abc")
    undefinedFunction(n)
}
"@ | Set-Content -NoNewline -Encoding ascii "$D/broken.fly"
@"
import fly.str

void main() {
    int n = fly.str.len("abc")
}
"@ | Set-Content -NoNewline -Encoding ascii "$D/clean.fly"
# Navigation fixture: a call, a local, a class — every nav feature has a target.
# The class sits AFTER main so the earlier assertions' line numbers stay fixed.
@"
import fly.str

int square(const int n) {
    out = n * n
}

void main() {
    int base = 3
    int result = square(base)
    int len = fly.str.len("abc")
    Point p = new Point()
    int px = p.getX()
    int py = p.x
}

class Point {
    int x
    public Point() { this.x = 0 }
    public int getX() { out = this.x }
}
"@ | Set-Content -NoNewline -Encoding ascii "$D/navfix.fly"

function Frame([string]$json) {
    $body = [System.Text.Encoding]::UTF8.GetBytes($json)
    $hdr  = [System.Text.Encoding]::ASCII.GetBytes("Content-Length: $($body.Length)`r`n`r`n")
    return $hdr + $body
}
$brokenUri = "file:///" + ($DirAbs.Replace([char]92, [char]47)) + "/broken.fly"
$cleanUri  = "file:///" + ($DirAbs.Replace([char]92, [char]47)) + "/clean.fly"
$bytes = @()
$bytes += Frame('{"jsonrpc":"2.0","id":1,"method":"initialize","params":{}}')
$bytes += Frame('{"jsonrpc":"2.0","method":"initialized","params":{}}')
$bytes += Frame('{"jsonrpc":"2.0","method":"textDocument/didOpen","params":{"textDocument":{"uri":"' + $brokenUri + '"}}}')
$bytes += Frame('{"jsonrpc":"2.0","method":"textDocument/didOpen","params":{"textDocument":{"uri":"' + $cleanUri + '"}}}')
# didSave with UNCHANGED bytes: the server must coalesce (skip the recompile and
# publish nothing), so the expected frame count below does NOT grow by this line.
$bytes += Frame('{"jsonrpc":"2.0","method":"textDocument/didSave","params":{"textDocument":{"uri":"' + $cleanUri + '"}}}')
$bytes += Frame('{"jsonrpc":"2.0","method":"textDocument/didClose","params":{"textDocument":{"uri":"' + $brokenUri + '"}}}')
$bytes += Frame('{"jsonrpc":"2.0","id":9,"method":"no/suchMethod","params":{}}')
# ── navigation over navfix.fly ────────────────────────────────────────────────
$navUri = "file:///" + ($DirAbs.Replace([char]92, [char]47)) + "/navfix.fly"
$ntd = '"textDocument":{"uri":"' + $navUri + '"}'
$bytes += Frame('{"jsonrpc":"2.0","method":"textDocument/didOpen","params":{' + $ntd + '}}')
$bytes += Frame('{"jsonrpc":"2.0","id":20,"method":"textDocument/definition","params":{' + $ntd + ',"position":{"line":8,"character":17}}}')
$bytes += Frame('{"jsonrpc":"2.0","id":21,"method":"textDocument/hover","params":{' + $ntd + ',"position":{"line":8,"character":17}}}')
$bytes += Frame('{"jsonrpc":"2.0","id":22,"method":"textDocument/references","params":{' + $ntd + ',"position":{"line":7,"character":8},"context":{"includeDeclaration":false}}}')
$bytes += Frame('{"jsonrpc":"2.0","id":23,"method":"textDocument/documentSymbol","params":{' + $ntd + '}}')
$bytes += Frame('{"jsonrpc":"2.0","id":24,"method":"textDocument/foldingRange","params":{' + $ntd + '}}')
$bytes += Frame('{"jsonrpc":"2.0","id":25,"method":"textDocument/completion","params":{' + $ntd + ',"position":{"line":9,"character":4}}}')
$bytes += Frame('{"jsonrpc":"2.0","id":26,"method":"textDocument/signatureHelp","params":{' + $ntd + ',"position":{"line":8,"character":24}}}')
$bytes += Frame('{"jsonrpc":"2.0","id":27,"method":"textDocument/semanticTokens/full","params":{' + $ntd + '}}')
$bytes += Frame('{"jsonrpc":"2.0","id":28,"method":"workspace/symbol","params":{"query":"square"}}')
$bytes += Frame('{"jsonrpc":"2.0","id":29,"method":"textDocument/typeDefinition","params":{' + $ntd + ',"position":{"line":10,"character":10}}}')
$bytes += Frame('{"jsonrpc":"2.0","id":30,"method":"textDocument/inlayHint","params":{' + $ntd + ',"range":{"start":{"line":0,"character":0},"end":{"line":50,"character":0}}}}')
$bytes += Frame('{"jsonrpc":"2.0","id":31,"method":"textDocument/implementation","params":{' + $ntd + ',"position":{"line":8,"character":17}}}')
# receiver typing: a method call and a field access
$bytes += Frame('{"jsonrpc":"2.0","id":40,"method":"textDocument/definition","params":{' + $ntd + ',"position":{"line":11,"character":15}}}')
$bytes += Frame('{"jsonrpc":"2.0","id":41,"method":"textDocument/hover","params":{' + $ntd + ',"position":{"line":11,"character":15}}}')
$bytes += Frame('{"jsonrpc":"2.0","id":42,"method":"textDocument/definition","params":{' + $ntd + ',"position":{"line":12,"character":15}}}')
$bytes += Frame('{"jsonrpc":"2.0","id":43,"method":"textDocument/hover","params":{' + $ntd + ',"position":{"line":12,"character":15}}}')
$bytes += Frame('{"jsonrpc":"2.0","id":2,"method":"shutdown"}')
$bytes += Frame('{"jsonrpc":"2.0","method":"exit"}')

$psi = New-Object System.Diagnostics.ProcessStartInfo
$psi.FileName = (Resolve-Path $EXE).Path
$psi.RedirectStandardInput = $true
$psi.RedirectStandardOutput = $true
$psi.UseShellExecute = $false
$p = [System.Diagnostics.Process]::Start($psi)
$p.StandardInput.BaseStream.Write($bytes, 0, $bytes.Length)
$p.StandardInput.BaseStream.Flush()
$p.StandardInput.Close()
$outStream = New-Object System.IO.MemoryStream
$p.StandardOutput.BaseStream.CopyTo($outStream)
if (-not $p.WaitForExit(120000)) { $p.Kill(); Write-Host 'FAIL: fly-lsp did not exit (transport hang?)'; exit 1 }
$outBytes = $outStream.ToArray()
$outText  = [System.Text.Encoding]::UTF8.GetString($outBytes)

$failed = 0
function Check([bool]$cond, [string]$what) {
    if ($cond) { Write-Host "  ok    $what" }
    else       { Write-Host "  FAIL  $what"; $script:failed = 1 }
}

Check ($p.ExitCode -eq 0) 'process exited 0'
# Framing: every response terminated by real \r\n\r\n (6 messages expected).
$frames = 0
for ($i = 0; $i -lt $outBytes.Length - 3; $i++) {
    if ($outBytes[$i] -eq 13 -and $outBytes[$i+1] -eq 10 -and $outBytes[$i+2] -eq 13 -and $outBytes[$i+3] -eq 10) { $frames++ }
}
# initialize + 4 publishDiagnostics + -32601 + 16 nav responses + shutdown = 23.
# The 4 publishes: didOpen broken (its own diagnostics), didOpen clean (empty
# list), didClose broken (the clear), didOpen nav (empty). The didSave with
# unchanged bytes publishes NOTHING (coalesced) — a growth here means either the
# coalescing broke or a compile spilled diagnostics onto unrelated files again.
Check ($frames -eq 23) "23 CRLF-framed messages (got $frames)"
Check ($outText.Contains('"id":1') -and $outText.Contains('"textDocumentSync":{') -and $outText.Contains('"save":true')) 'initialize answered with sync-object capabilities (save enabled)'
Check ($outText.Contains('"serverInfo"')) 'serverInfo present'
Check ($outText.Contains("undefinedFunction")) 'sema diagnostic published for the broken file'
Check ($outText.Contains($cleanUri + '","diagnostics":[]')) 'clean file published an empty list'
Check (($outText -split [regex]::Escape($brokenUri)).Count -ge 3) 'broken file published twice (diagnostics, then the didClose clear)'
Check ($outText.Contains('-32601')) 'unknown request answered -32601'
Check ($outText.Contains('"id":2,"result":null')) 'shutdown answered null'
# ── navigation ────────────────────────────────────────────────────────────────
$def = ($outText -split 'Content-Length: \d+' | Where-Object { $_ -match '"id":20' })
# character 4 (0-based) = column 5 = the NAME `square` in "int square(...)".
# It used to be character 0 — the return type — because a declaration node's
# location is where the declaration starts; nameLoc points at the identifier.
Check ([bool]($def -match '"line":2,"character":4')) 'definition of square() lands on the NAME, not the return type'
$hov = ($outText -split 'Content-Length: \d+' | Where-Object { $_ -match '"id":21' })
Check ([bool]($hov -match 'int square\(int n')) 'hover shows the resolved signature'
$refs = ($outText -split 'Content-Length: \d+' | Where-Object { $_ -match '"id":22' })
Check ([bool]($refs -match '"line":7,"character":8') -and [bool]($refs -match '"line":8,"character":24')) 'references of base find declaration and use, name-exact'
$ds = ($outText -split 'Content-Length: \d+' | Where-Object { $_ -match '"id":23' })
Check ([bool]($ds -match '"name":"square","kind":12')) 'documentSymbol lists square as a function'
$fr = ($outText -split 'Content-Length: \d+' | Where-Object { $_ -match '"id":24' })
Check ([bool]($fr -match '"startLine":2,"endLine":3')) 'foldingRange folds the square body'
$cmp = ($outText -split 'Content-Length: \d+' | Where-Object { $_ -match '"id":25' })
Check ([bool]($cmp -match '"label":"base"') -and [bool]($cmp -match '"label":"square"')) 'completion offers locals and functions'
$sh2 = ($outText -split 'Content-Length: \d+' | Where-Object { $_ -match '"id":26' })
Check ([bool]($sh2 -match 'int square\(int n') -and [bool]($sh2 -match '"activeParameter":0')) 'signatureHelp shows the call signature'
$st = ($outText -split 'Content-Length: \d+' | Where-Object { $_ -match '"id":27' })
Check ([bool]($st -match '"data":\[\d')) 'semanticTokens returns a non-empty data array'
$ws = ($outText -split 'Content-Length: \d+' | Where-Object { $_ -match '"id":28' })
Check ([bool]($ws -match '"name":"square"')) 'workspace/symbol finds square'
$td2 = ($outText -split 'Content-Length: \d+' | Where-Object { $_ -match '"id":29' })
Check ([bool]($td2 -match '"line":15')) 'typeDefinition of a Point variable lands on class Point'
$ih = ($outText -split 'Content-Length: \d+' | Where-Object { $_ -match '"id":30' })
Check ([bool]($ih -match '"label":"n:"') -and [bool]($ih -match '"line":8,"character":24')) 'inlayHint names the square() argument'
$impl = ($outText -split 'Content-Length: \d+' | Where-Object { $_ -match '"id":31' })
Check ([bool]($impl -match '"line":8,"character":17')) 'implementation returns the call site'
$mdef = ($outText -split 'Content-Length: \d+' | Where-Object { $_ -match '"id":40' })
Check ([bool]($mdef -match '"line":18')) 'definition of p.getX() lands on the method (receiver typed)'
$mhov = ($outText -split 'Content-Length: \d+' | Where-Object { $_ -match '"id":41' })
Check ([bool]($mhov -match 'int getX\(')) 'hover on a method shows its signature'
$fdef = ($outText -split 'Content-Length: \d+' | Where-Object { $_ -match '"id":42' })
Check ([bool]($fdef -match '"line":16')) 'definition of p.x lands on the field'
$fhov = ($outText -split 'Content-Length: \d+' | Where-Object { $_ -match '"id":43' })
Check ([bool]($fhov -match '\(field\) int x')) 'hover on a field shows (field) type name'

if ($failed -ne 0) { Write-Host 'test_lsp: FAILED'; exit 1 }
Write-Host 'test_lsp: all checks passed'
exit 0
