#!/usr/bin/env bash
# -----------------------------------------------------------------------------
# test_registry.sh - end-to-end test of fly-registry over real HTTP.
# Mirror of ci/windows/test_registry.ps1; see that file for the walkthrough.
#
# One server run per request (--once); the storage directory carries state, so
# publish -> list -> download is a real sequence rather than isolated checks.
# -----------------------------------------------------------------------------
set -u
cd "$(dirname "$0")/../.."

STAGE="${STAGE:-2}"
EXE="build/stage$STAGE/bin/fly-registry"
[ -x "$EXE" ] || { echo "error: '$EXE' not found - run build_registry.sh first."; exit 1; }
EXE="$(cd "$(dirname "$EXE")" && pwd)/$(basename "$EXE")"

FIX="std/test/fixtures/regpkg.tar.gz"
[ -f "$FIX" ] || { echo "error: fixture '$FIX' not found."; exit 1; }
FIXABS="$(cd "$(dirname "$FIX")" && pwd)/$(basename "$FIX")"

D="build/stage$STAGE/registry_test"
rm -rf "$D"; mkdir -p "$D"
STORAGE="$(cd "$D" && pwd)"
TOKEN="secret-token"
TMP="$D/tmp"; mkdir -p "$TMP"

failed=0
check() { if [ "$1" -eq 1 ]; then echo "  ok    $2"; else echo "  FAIL  $2"; failed=1; fi; }

# serve_one <request-file> <response-file> — run the server for a single
# request and capture the raw response bytes.
serve_one() {
    local reqf="$1" respf="$2"
    local outf="$TMP/server.out"
    rm -f "$outf"
    "$EXE" --storage "$STORAGE" --host 127.0.0.1 --port 0 --token "$TOKEN" --once > "$outf" 2>&1 &
    local pid=$!
    local port=0 tries=0
    while [ "$port" -eq 0 ] && [ "$tries" -lt 200 ]; do
        port=$(grep -oE 'ready[[:space:]]*:[[:space:]]*port[[:space:]]+[0-9]+' "$outf" 2>/dev/null | grep -oE '[0-9]+$' || echo 0)
        [ -z "$port" ] && port=0
        [ "$port" -eq 0 ] && sleep 0.05
        tries=$((tries+1))
    done
    if [ "$port" -eq 0 ]; then kill "$pid" 2>/dev/null; echo "  FAIL  server never reported a port"; failed=1; return 1; fi
    # nc talks raw bytes both ways; -q1 closes after EOF on stdin.
    nc -q1 127.0.0.1 "$port" < "$reqf" > "$respf"
    wait "$pid" 2>/dev/null
    return 0
}

# build_request <method> <path> <auth|-> <bodyfile|-> <outfile>
build_request() {
    local method="$1" path="$2" auth="$3" bodyf="$4" outf="$5"
    { printf '%s %s HTTP/1.1\r\n' "$method" "$path"
      printf 'Host: 127.0.0.1\r\n'
      printf 'Connection: close\r\n'
      [ "$auth" != "-" ] && printf 'Authorization: Bearer %s\r\n' "$auth"
      if [ "$bodyf" != "-" ]; then printf 'Content-Length: %d\r\n' "$(wc -c < "$bodyf")"; fi
      printf '\r\n'
      [ "$bodyf" != "-" ] && cat "$bodyf"
    } > "$outf"
}

# body_of <response-file> <out-body-file> — the body, taken as the LAST
# Content-Length bytes. Splitting on the blank line instead would need a
# binary-safe search for "\r\n\r\n", and `grep -abo` cannot do that: it treats
# the newlines inside the pattern as line separators and reports offsets that
# land mid-header. The length the server itself declared is exact and needs no
# scanning.
body_of() {
    local cl
    # -a: the response body is binary, and without it grep prints
    # "binary file matches" instead of the header line.
    cl=$(tr -d '\r' < "$1" | grep -a -m1 -i '^Content-Length:' | sed 's/[^0-9]//g')
    if [ -z "$cl" ] || [ "$cl" -eq 0 ]; then : > "$2"; return; fi
    tail -c "$cl" "$1" > "$2"
}

REQ="$TMP/req.bin"; RESP="$TMP/resp.bin"; BODY="$TMP/body.bin"

# 1. list versions on an empty registry
build_request GET /v1/mypkg - - "$REQ"; serve_one "$REQ" "$RESP"
check "$(head -c 32 "$RESP" | grep -q '404' && echo 1 || echo 0)" "GET an unknown package answers 404"

# 2. publish WITHOUT the token
build_request POST /v1/mypkg/1.0.0 - "$FIXABS" "$REQ"; serve_one "$REQ" "$RESP"
check "$(head -c 32 "$RESP" | grep -q '401' && echo 1 || echo 0)" "POST without a Bearer token answers 401"

# 3. publish WITH the token
build_request POST /v1/mypkg/1.0.0 "$TOKEN" "$FIXABS" "$REQ"; serve_one "$REQ" "$RESP"
check "$(head -c 32 "$RESP" | grep -q '201' && echo 1 || echo 0)" "POST with the Bearer token answers 201"
check "$([ -f "$STORAGE/mypkg/1.0.0.tar.gz" ] && echo 1 || echo 0)" "the tarball is stored under <storage>/<name>/<version>.tar.gz"
check "$([ -f "$STORAGE/mypkg/1.0.0/Manifest.fly" ] && echo 1 || echo 0)" "Manifest.fly was extracted in-process at publish"

# 4. list versions
build_request GET /v1/mypkg - - "$REQ"; serve_one "$REQ" "$RESP"; body_of "$RESP" "$BODY"
check "$([ "$(cat "$BODY")" = '["1.0.0"]' ] && echo 1 || echo 0)" "GET /v1/mypkg lists the version (got $(cat "$BODY"))"

# 5. metadata
build_request GET /v1/mypkg/1.0.0 - - "$REQ"; serve_one "$REQ" "$RESP"; body_of "$RESP" "$BODY"
check "$(grep -q 'name = "mypkg"' "$BODY" && echo 1 || echo 0)" "GET metadata returns the Manifest.fly content"

# 6. download - byte-exact round trip of a BINARY payload
build_request GET /v1/mypkg/1.0.0/download - - "$REQ"; serve_one "$REQ" "$RESP"; body_of "$RESP" "$BODY"
check "$(cmp -s "$BODY" "$FIXABS" && echo 1 || echo 0)" "download returns the tarball byte-for-byte ($(wc -c < "$BODY")/$(wc -c < "$FIXABS") bytes)"

# 7. search
build_request GET '/v1/search?q=my' - - "$REQ"; serve_one "$REQ" "$RESP"; body_of "$RESP" "$BODY"
check "$([ "$(cat "$BODY")" = '["mypkg"]' ] && echo 1 || echo 0)" "search finds the package (got $(cat "$BODY"))"

# 8. path traversal
build_request GET '/v1/..%2f..%2fetc' - - "$REQ"; serve_one "$REQ" "$RESP"
check "$(head -c 32 "$RESP" | grep -q '400' && echo 1 || echo 0)" "a path-traversal name is rejected with 400"

# 9. a non-gzip body is refused rather than stored
printf 'this is not a gzip archive at all' > "$TMP/junk.bin"
build_request POST /v1/other/9.9.9 "$TOKEN" "$TMP/junk.bin" "$REQ"; serve_one "$REQ" "$RESP"
check "$(head -c 32 "$RESP" | grep -q '400' && echo 1 || echo 0)" "a non-gzip publish body is rejected with 400"
check "$([ ! -f "$STORAGE/other/9.9.9.tar.gz" ] && echo 1 || echo 0)" "the rejected upload left nothing on disk"

if [ "$failed" -ne 0 ]; then echo "test_registry: FAILED"; exit 1; fi
echo "test_registry: all checks passed"
exit 0
