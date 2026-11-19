  [![Build Linux](https://github.com/fly-lang/fly/actions/workflows/build-linux.yml/badge.svg)](https://github.com/fly-lang/fly/actions/workflows/build-linux.yml)
  [![Build Windows](https://github.com/fly-lang/fly/actions/workflows/build-windows.yml/badge.svg)](https://github.com/fly-lang/fly/actions/workflows/build-windows.yml)
  ![GitHub issues](https://img.shields.io/github/issues-raw/fly-lang/fly?color=blue)
  ![GitHub closed issues](https://img.shields.io/github/issues-closed/fly-lang/fly?color=blue)
  <br />
<p align="center">
  <a href="https://github.com/fly-lang/fly">
    <img src="https://github.com/fly-lang/graphics/blob/main/logo/fly_logo_300.png?raw=true" alt="Logo" width="300" height="300">
  </a>

  <h3 align="center">Fly Programming Language</h3>

  <p align="center">
    Simple - Reliable - Safe - Powerful
    <br />
    <br />
    <a href="https://flylang.org">View Website</a>
    ·
    <a href="https://github.com/fly-lang/fly/issues">Report Bug / Request Feature</a>
    ·
    <a href="https://github.com/fly-lang/fly/discussions">Open a Discussion</a>
  </p>
</p>

## About the Project
Fly is a compiled, high-level, general-purpose programming language built on LLVM, with
particular attention to simplicity, reliability, safety and power.

Fly is **self-hosted**: the compiler, the runtime and the standard library in this branch
are written in Fly.

| Directory | Contents |
|---|---|
| `compiler/` | the compiler and the `fly` driver (`lib/`) and their suites (`test/`) |
| `runtime/` | the per-platform runtime layer (`fly.runtime`) |
| `std/` | the standard library (`fly.*`) and its suites |
| `tools/` | `fly-lsp` (language server) and `fly-registry` (package registry) |
| `ci/` | the staged bootstrap and every test script, for Linux and Windows |

There is no garbage collector: structs live on the stack, strings are released
automatically, class instances are released explicitly.

### Built with
- [LLVM](https://llvm.org/) 20.1.8, from the project's own fork
  ([fly-lang/llvm-project](https://github.com/fly-lang/llvm-project)), downloaded by the
  bootstrap — no system LLVM, no package manager.
- A **seed compiler**: the C++ implementation of Fly kept on the `release/0.13.X` branch.
  It is downloaded as a pinned binary and used once, to compile this tree for the first
  time.

## Getting Started

### Install a release
Download the archive for your platform from the
[releases page](https://github.com/fly-lang/fly/releases) and unpack it anywhere. It is
self-contained: `bin/` holds `fly`, `fly-lsp`, `fly-registry`, the linker and `lldb`;
`lib/` holds the runtime, the standard library and its headers. Put `bin/` on your `PATH`.

```bash
mkdir hello && cd hello
cat > main.fly <<'EOF'
import fly.os.io

void main() {
    io.printLn("Hello, World!")
}
EOF
fly -o hello
./hello
```

`fly` compiles a **directory**, not a file: it discovers the source that declares `main()`
and pulls the rest in through the imports.

### Build from source

Fly builds itself in three stages:

| Stage | What happens |
|---|---|
| 0 | downloads the pinned seed compiler and the fork LLVM into `build/` |
| 1 | the seed compiles runtime, std and compiler from this tree and links the first `fly`; the suites then run under it |
| 2 | that `fly` recompiles the runtime and the compiler, builds the tools and runs every guard; `build/stage2` is what a release ships |

The seed version is pinned in `ci/linux/stage0.sh` and `ci/windows/stage0.ps1`.

#### Linux

Requires `gcc` and `libc6-dev` (for the C start-up objects), `ar` and `objcopy`.

```bash
git clone --branch release/0.14.X https://github.com/fly-lang/fly.git
cd fly
. ./ci/linux/stage0.sh        # sourced: it exports PATH and the library paths
./ci/linux/stage1.sh
./ci/linux/stage2.sh
```

No system LLVM and no `sudo` are needed: everything LLVM-related comes from the
fork's archive in `build/llvm`, and a release carries the compiler-rt builtins
it links into every program.

#### Windows

Requires **PowerShell 7** (`pwsh`) — the scripts do not run under Windows PowerShell 5.1.
No Visual Studio is needed, to build Fly or to use it: programs link against a bundled
llvm-mingw / UCRT sysroot.

```powershell
git clone --branch release/0.14.X --config core.autocrlf=false https://github.com/fly-lang/fly.git
cd fly
. .\ci\windows\stage0.ps1
.\ci\windows\stage1.ps1
.\ci\windows\stage2.ps1
```

The result is `build/stage2/bin/fly` (`fly.exe` on Windows).

#### Useful switches

| Environment variable | Effect |
|---|---|
| `FLY_BUNDLE_LLVM=1` | make each stage self-contained: LLVM, the linker and `lldb` next to `fly` (what CI and releases use) |
| `FLY_STAGE1_TESTS=0` | skip the stage 1 suites for a faster local build |
| `FLY_DEBUG_SYMBOLS=1` | DWARF in the stage 1 build |
| `STAGE=1` / `STAGE=2` | select the stage for a single `ci/*/build_*` or `ci/*/test_*` script |

### Run the tests

Every script takes the compiler of the stage named by `STAGE` (default: the one it belongs to).

```bash
STAGE=2 ./ci/linux/test_runtime.sh     # the runtime layer
STAGE=2 ./ci/linux/test_std.sh         # the standard library
STAGE=2 ./ci/linux/test_compiler.sh    # the compiler and the driver
./ci/linux/test_sema.sh                # programs the compiler must reject
./ci/linux/test_abi.sh                 # seed / self-host calling convention
./ci/linux/test_leaks.sh               # ownership and scope clean-up
```

The Windows scripts have the same names under `ci\windows\` with a `.ps1` extension.

## Documentation
The reference documentation is published at [flylang.org](https://flylang.org): tutorial,
language reference, standard library API, command line, testing, how Fly builds itself,
what the compiler enforces today, and the roadmap.

## Additional Info
- For how to contribute see `CONTRIBUTING.md`
- For know Authors see `AUTHORS.md`
- See `LICENSE`

## Contact
BlueSky: [@flylang.org](https://bsky.app/profile/flylang.org)

Email: [dev@flylang.org](mailto:dev@flylang.org)

Website: [flylang.org](https://flylang.org)
 

