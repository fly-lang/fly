#!/usr/bin/env bash
# -----------------------------------------------------------------------------
# test_lsp.sh - end-to-end smoke test of fly-lsp over a scripted stdio session.
# Mirror of ci/windows/test_lsp.ps1; see that file for the session walkthrough.
# -----------------------------------------------------------------------------
set -u
cd "$(dirname "$0")/../.."

STAGE="${STAGE:-2}"
EXE="build/stage$STAGE/bin/fly-lsp"
[ -x "$EXE" ] || { echo "error: '$EXE' not found - run build_lsp.sh first."; exit 1; }

D="build/stage$STAGE/lsp_test"
mkdir -p "$D"
DIRABS="$(cd "$D" && pwd)"

cat > "$D/broken.fly" << 'EOF'
import fly.str

void main() {
    int n = fly.str.len("abc")
    undefinedFunction(n)
}
EOF
cat > "$D/clean.fly" << 'EOF'
import fly.str

void main() {
    int n = fly.str.len("abc")
}
EOF
# Navigation fixture: a call, a local, a class — every nav feature has a target.
# The class sits AFTER main so the earlier assertions' line numbers stay fixed.
cat > "$D/navfix.fly" << 'EOF'
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
EOF

BROKEN_URI="file://$DIRABS/broken.fly"
CLEAN_URI="file://$DIRABS/clean.fly"

# frame <json> — emit "Content-Length: N\r\n\r\n<json>" with REAL CR bytes.
frame() {
    local body="$1"
    printf 'Content-Length: %d\r\n\r\n%s' "${#body}" "$body"
}

SESSION="$D/session.bin"
{
    frame '{"jsonrpc":"2.0","id":1,"method":"initialize","params":{}}'
    frame '{"jsonrpc":"2.0","method":"initialized","params":{}}'
    frame '{"jsonrpc":"2.0","method":"textDocument/didOpen","params":{"textDocument":{"uri":"'"$BROKEN_URI"'"}}}'
    frame '{"jsonrpc":"2.0","method":"textDocument/didOpen","params":{"textDocument":{"uri":"'"$CLEAN_URI"'"}}}'
    # didSave with UNCHANGED bytes: coalesced — no recompile, no publish, so the
    # expected frame count below does NOT grow by this line.
    frame '{"jsonrpc":"2.0","method":"textDocument/didSave","params":{"textDocument":{"uri":"'"$CLEAN_URI"'"}}}'
    frame '{"jsonrpc":"2.0","method":"textDocument/didClose","params":{"textDocument":{"uri":"'"$BROKEN_URI"'"}}}'
    frame '{"jsonrpc":"2.0","id":9,"method":"no/suchMethod","params":{}}'
    # navigation over navfix.fly
    NAV_URI="file://$DIRABS/navfix.fly"
    NTD='"textDocument":{"uri":"'"$NAV_URI"'"}'
    frame '{"jsonrpc":"2.0","method":"textDocument/didOpen","params":{'"$NTD"'}}'
    frame '{"jsonrpc":"2.0","id":20,"method":"textDocument/definition","params":{'"$NTD"',"position":{"line":8,"character":17}}}'
    frame '{"jsonrpc":"2.0","id":21,"method":"textDocument/hover","params":{'"$NTD"',"position":{"line":8,"character":17}}}'
    frame '{"jsonrpc":"2.0","id":22,"method":"textDocument/references","params":{'"$NTD"',"position":{"line":7,"character":8},"context":{"includeDeclaration":false}}}'
    frame '{"jsonrpc":"2.0","id":23,"method":"textDocument/documentSymbol","params":{'"$NTD"'}}'
    frame '{"jsonrpc":"2.0","id":24,"method":"textDocument/foldingRange","params":{'"$NTD"'}}'
    frame '{"jsonrpc":"2.0","id":25,"method":"textDocument/completion","params":{'"$NTD"',"position":{"line":9,"character":4}}}'
    frame '{"jsonrpc":"2.0","id":26,"method":"textDocument/signatureHelp","params":{'"$NTD"',"position":{"line":8,"character":24}}}'
    frame '{"jsonrpc":"2.0","id":27,"method":"textDocument/semanticTokens/full","params":{'"$NTD"'}}'
    frame '{"jsonrpc":"2.0","id":28,"method":"workspace/symbol","params":{"query":"square"}}'
    frame '{"jsonrpc":"2.0","id":29,"method":"textDocument/typeDefinition","params":{'"$NTD"',"position":{"line":10,"character":10}}}'
    frame '{"jsonrpc":"2.0","id":30,"method":"textDocument/inlayHint","params":{'"$NTD"',"range":{"start":{"line":0,"character":0},"end":{"line":50,"character":0}}}}'
    frame '{"jsonrpc":"2.0","id":31,"method":"textDocument/implementation","params":{'"$NTD"',"position":{"line":8,"character":17}}}'
    # receiver typing: a method call and a field access
    frame '{"jsonrpc":"2.0","id":40,"method":"textDocument/definition","params":{'"$NTD"',"position":{"line":11,"character":15}}}'
    frame '{"jsonrpc":"2.0","id":41,"method":"textDocument/hover","params":{'"$NTD"',"position":{"line":11,"character":15}}}'
    frame '{"jsonrpc":"2.0","id":42,"method":"textDocument/definition","params":{'"$NTD"',"position":{"line":12,"character":15}}}'
    frame '{"jsonrpc":"2.0","id":43,"method":"textDocument/hover","params":{'"$NTD"',"position":{"line":12,"character":15}}}'
    frame '{"jsonrpc":"2.0","id":2,"method":"shutdown"}'
    frame '{"jsonrpc":"2.0","method":"exit"}'
} > "$SESSION"

OUT="$D/out.bin"
timeout 120 "$EXE" < "$SESSION" > "$OUT"
RC=$?

failed=0
check() {  # check <0|1> <label>
    if [ "$1" -eq 1 ]; then echo "  ok    $2"; else echo "  FAIL  $2"; failed=1; fi
}

check "$([ $RC -eq 0 ] && echo 1 || echo 0)" "process exited 0"
# initialize + 4 publishDiagnostics + -32601 + 16 nav responses + shutdown = 23.
FRAMES=$(grep -c $'Content-Length' "$OUT")
check "$([ "$FRAMES" -eq 23 ] && echo 1 || echo 0)" "23 framed messages (got $FRAMES)"
CRLF=$(od -An -tx1 "$OUT" | tr -d ' \n' | grep -o '0d0a0d0a' | wc -l)
check "$([ "$CRLF" -eq 23 ] && echo 1 || echo 0)" "frame terminators are real CRLFCRLF (got $CRLF)"
check "$(grep -q '"id":1' "$OUT" && grep -q '"textDocumentSync":{' "$OUT" && grep -q '"save":true' "$OUT" && echo 1 || echo 0)" "initialize answered with sync-object capabilities (save enabled)"
check "$(grep -q '"serverInfo"' "$OUT" && echo 1 || echo 0)" "serverInfo present"
check "$(grep -q 'undefinedFunction' "$OUT" && echo 1 || echo 0)" "sema diagnostic published for the broken file"
check "$(grep -q "$CLEAN_URI\",\"diagnostics\":\[\]" "$OUT" && echo 1 || echo 0)" "clean file published an empty list"
BOPEN=$(grep -o "$BROKEN_URI" "$OUT" | wc -l)
check "$([ "$BOPEN" -ge 2 ] && echo 1 || echo 0)" "broken file published twice (diagnostics, then the didClose clear)"
check "$(grep -q '\-32601' "$OUT" && echo 1 || echo 0)" "unknown request answered -32601"
check "$(grep -q '"id":2,"result":null' "$OUT" && echo 1 || echo 0)" "shutdown answered null"
# navigation — each response carries its id, so a per-id grep pins the payload.
# character 4 (0-based) = column 5 = the NAME `square` in "int square(...)".
# It used to be character 0 — the return type — because a declaration node's
# location is where the declaration starts; nameLoc points at the identifier.
check "$(grep -q '"id":20,"result":{"uri".*"line":2,"character":4' "$OUT" && echo 1 || echo 0)" "definition of square() lands on the NAME, not the return type"
check "$(grep -q 'int square(int n' "$OUT" && echo 1 || echo 0)" "hover shows the resolved signature"
check "$(grep -q '"id":22,"result":\[.*"line":7,"character":8' "$OUT" && grep -q '"id":22,"result":\[.*"line":8,"character":24' "$OUT" && echo 1 || echo 0)" "references of base find declaration and use"
check "$(grep -q '"name":"square","kind":12' "$OUT" && echo 1 || echo 0)" "documentSymbol lists square as a function"
check "$(grep -q '"startLine":2,"endLine":3' "$OUT" && echo 1 || echo 0)" "foldingRange folds the square body"
check "$(grep -q '"label":"base"' "$OUT" && echo 1 || echo 0)" "completion offers locals"
check "$(grep -q '"id":26,"result":{"signatures".*"activeParameter":0' "$OUT" && echo 1 || echo 0)" "signatureHelp shows the call signature"
check "$(grep -q '"id":27,"result":{"data":\[[0-9]' "$OUT" && echo 1 || echo 0)" "semanticTokens returns a non-empty data array"
check "$(grep -q '"id":28,"result":\[{"name":"square"' "$OUT" && echo 1 || echo 0)" "workspace/symbol finds square"
check "$(grep -q '"id":29,"result":{"uri".*"line":15' "$OUT" && echo 1 || echo 0)" "typeDefinition of a Point variable lands on class Point"
check "$(grep -q '"id":30,"result":\[.*"label":"n:"' "$OUT" && echo 1 || echo 0)" "inlayHint names the square() argument"
check "$(grep -q '"id":31,"result":\[.*"line":8,"character":17' "$OUT" && echo 1 || echo 0)" "implementation returns the call site"
check "$(grep -q '"id":40,"result":{"uri".*"line":18' "$OUT" && echo 1 || echo 0)" "definition of p.getX() lands on the method (receiver typed)"
check "$(grep -q '"id":41,"result".*int getX(' "$OUT" && echo 1 || echo 0)" "hover on a method shows its signature"
check "$(grep -q '"id":42,"result":{"uri".*"line":16' "$OUT" && echo 1 || echo 0)" "definition of p.x lands on the field"
check "$(grep -q '(field) int x' "$OUT" && echo 1 || echo 0)" "hover on a field shows (field) type name"

if [ "$failed" -ne 0 ]; then echo "test_lsp: FAILED"; exit 1; fi
echo "test_lsp: all checks passed"
exit 0
