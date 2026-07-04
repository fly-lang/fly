//===- lld_driver.cpp — minimal bundled linker (`fly-lld`) ----------------===//
//
// A tiny LLD front-end linked into a standalone `fly-lld` so the released
// self-host toolchain ships its own linker. The LLD flavor is selected at
// compile time by target OS — ELF on Linux, Mach-O on macOS, COFF on Windows —
// so ONE driver source builds the right linker on each platform. `fly-lld` is
// invoked as a subprocess by ToolChain.fly (which prefers <exe_dir>/fly-lld over
// the host PATH); the driver ignores argv[0], so the binary name is free.
//
// Built by the per-platform build script (FLY_BUNDLE_LLVM=1) against liblld's
// static archives + a shared libLLVM (dynamic), so LLVM is not duplicated.
//===----------------------------------------------------------------------===//

#include "lld/Common/Driver.h"
#include "llvm/Support/InitLLVM.h"
#include "llvm/Support/raw_ostream.h"

#if defined(_WIN32)
LLD_HAS_DRIVER(coff)
#  define FLY_LLD_LINK lld::coff::link
#elif defined(__APPLE__)
LLD_HAS_DRIVER(macho)
#  define FLY_LLD_LINK lld::macho::link
#else
LLD_HAS_DRIVER(elf)
#  define FLY_LLD_LINK lld::elf::link
#endif

int main(int argc, char **argv) {
  llvm::InitLLVM X(argc, argv);
  llvm::ArrayRef<const char *> args(argv, argv + argc);
  return FLY_LLD_LINK(args, llvm::outs(), llvm::errs(), false, false) ? 0 : 1;
}
