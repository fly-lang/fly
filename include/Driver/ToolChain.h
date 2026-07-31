//===--------------------------------------------------------------------------------------------------------------===//
// include/Driver/ToolChain.h - tool chain abstraction
//
// Part of the Fly Project https://flylang.org
// Under the Apache License v2.0 see LICENSE for details.
// Thank you to LLVM Project https://llvm.org/
//
//===--------------------------------------------------------------------------------------------------------------===//

#ifndef FLY_TOOLCHAIN_H
#define FLY_TOOLCHAIN_H

#include "Frontend/InputFile.h"
#include "llvm/TargetParser/Triple.h"

namespace fly {

    class Triple;

    class ToolChain {

        DiagnosticsEngine &Diag;

        const llvm::Triple &T;

        const CodeGenOptions &CodeGenOpts;

        IntrusiveRefCntPtr<llvm::vfs::FileSystem> VFS;

    public:
        ToolChain(DiagnosticsEngine &Diag, const llvm::Triple &T, CodeGenOptions &CodeGenOpts);

        bool BuildLib();

        bool BuildOutput(const llvm::SmallVector<std::string, 4> &InFiles, FrontendOptions &FrontendOpts);

        bool LinkWindows(const llvm::SmallVector<std::string, 4> &InFiles, const std::string &OutFile);
        bool getUniversalCRTLibraryPath(std::string &Path) const;
        bool getWindowsSDKLibraryPath(std::string &path) const;

        // windows-gnu / gnullvm: link against the bundled mingw/UCRT sysroot
        // (llvm-mingw) with ld.lld -m i386pep — no Visual Studio needed.
        bool LinkWindowsGNU(const llvm::SmallVector<std::string, 4> &InFiles, const std::string &OutFile);

        // Locate the bundled mingw/UCRT sysroot next to the runtime lib dir,
        // or "" when absent.
        std::string GetMingwSysrootDir() const;

        bool LinkDarwin(const llvm::SmallVector<std::string, 4> &InFiles, const std::string &OutFile);

        bool LinkLinux(const llvm::SmallVector<std::string, 4> &InFiles, const std::string &OutFile, FrontendOptions &FrontendOpts);
        bool getPIE();
        bool isArmBigEndian();
        const char *getLDMOption();
        std::string GetFilePath(llvm::Twine Name, SmallVector<std::string, 16> &PathList) const;
        llvm::vfs::FileSystem &getVFS() const;
        std::string getCompilerRT(const char *string, SmallVector<std::string, 16> &PathList);
        std::string getMultiarch() const;
        std::string getOSLibDir();

        SmallVector<std::string, 16> CreatePathList();

        // Returns the absolute path to libfly_runtime.a, or an empty string if
        // it cannot be located (e.g. cross-compilation without an installed runtime).
        std::string GetRuntimeLibPath() const;

        // Returns the absolute path to fly_std_lib.a (the compiled Fly standard
        // library), or an empty string if it cannot be found.
        std::string GetStdLibPath() const;

        // Returns the absolute path to the compiler-rt builtins archive
        // (libclang_rt.builtins-<arch>.a) for the current target, or an empty
        // string if it cannot be found.  Used in place of -lgcc.
        std::string GetCompilerRTBuiltinsPath() const;

        // Returns the absolute path to the compiler-rt builtins archive of the
        // BUNDLED llvm-mingw sysroot (probed next to RuntimeLibDir), or an empty
        // string. Needed on Windows to resolve the __atomic_* references of the
        // gnullvm-built 0.14 runtime when this toolchain acts as the seed.
        std::string GetMinGWBuiltinsPath() const;

        // Returns the absolute path to fly_tls_stub.a/.lib next to the runtime
        // archive, or an empty string. The seed always links the stub: it has no
        // --tls, and the 0.14 std references the tls_* primitives.
        std::string GetTlsStubPath() const;
    };
}

#endif //FLY_TOOLCHAIN_H
