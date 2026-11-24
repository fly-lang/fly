# -----------------------------------------------------------------------------
# gen_package.ps1 - generate build\gen\package.fly from the root Manifest.fly.
#
# The root Manifest.fly is the single source of truth for Fly's version (0.15.2).
# This writes the `fly.meta.package` module - name(), version(), description() -
# the same module `fly build` gives every executable it builds. The compiler
# reads its version from it (compiler/lib/basic/Version.fly: package.version()),
# so every compile of the compiler sources adds `--src-dir build/gen`: a plain
# source, which the seed compiles too. Once the tree is built with `fly build`
# (release/0.16.X) the Builder writes this module and the script goes away.
#
# The file is rewritten only when its content changes, so its timestamp - and
# the incremental fingerprints - stay put across builds. Prints the version.
# Mirrors ci/linux/gen_package.sh and the Builder (same bytes: LF line ends).
# The fields must not hold a double quote or a backslash: the seed reads
# literals raw.
# -----------------------------------------------------------------------------
$ErrorActionPreference = 'Stop'
Set-Location (Resolve-Path (Join-Path $PSScriptRoot '..\..'))

function Get-Field([string]$name) {
    $m = Select-String -Path 'Manifest.fly' -Pattern "^\s*string\s+$name\s*=\s*`"([^`"]*)`"" | Select-Object -First 1
    if ($m) { return $m.Matches[0].Groups[1].Value }
    return ''
}
$name = Get-Field 'name'
$version = Get-Field 'version'
$desc = Get-Field 'description'
if ($version -notmatch '^[0-9]+\.[0-9]+\.[0-9]+$') { throw 'Manifest.fly has no `string version = "x.y.z"` line.' }
if (("$name$desc").Contains([string][char]92)) { throw 'a Manifest.fly name/description holds a backslash.' }

New-Item -ItemType Directory -Force 'build/gen' | Out-Null
$out = Join-Path (Resolve-Path 'build/gen').Path 'package.fly'
$lines = @(
    'namespace fly.meta.package',
    '',
    '// GENERATED from Manifest.fly - do not edit.',
    'public string name() {',
    "    return `"$name`"",
    '}',
    'public string version() {',
    "    return `"$version`"",
    '}',
    'public string description() {',
    "    return `"$desc`"",
    '}'
)
$text = ($lines -join "`n") + "`n"
$old = if (Test-Path $out) { [IO.File]::ReadAllText($out) } else { $null }
if ($old -ne $text) {
    [IO.File]::WriteAllText($out, $text, (New-Object Text.UTF8Encoding($false)))
}
Write-Output $version
