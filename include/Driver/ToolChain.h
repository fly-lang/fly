//===--------------------------------------------------------------------------------------------------------------===//
// include/Driver/ToolChain.h - ToolChain
//
// Part of the Fly Project https://flylang.org
// Under the Apache License v2.0 see LICENSE for details.
// Thank you to LLVM Project https://llvm.org/
//
//===--------------------------------------------------------------------------------------------------------------===//

#ifndef FLY_TOOLCHAIN_H
#define FLY_TOOLCHAIN_H

#include "Frontend/InputFile.h"
#include "llvm/ADT/Triple.h"

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

        bool LinkDarwin(const llvm::SmallVector<std::string, 4> &InFiles, const std::string &OutFile);

        bool LinkLinux(const llvm::SmallVector<std::string, 4> &InFiles, const std::string &OutFile);
        bool getPIE();
        bool isArmBigEndian();
        const char *getLDMOption();
        std::string GetFilePath(llvm::Twine Name, SmallVector<std::string, 16> &PathList) const;
        llvm::vfs::FileSystem &getVFS() const;
        std::string getCompilerRT(const char *string, SmallVector<std::string, 16> &PathList);
        std::string getMultiarch() const;
        std::string getOSLibDir();

        SmallVector<std::string, 16> CreatePathList();
    };
}

#endif //FLY_TOOLCHAIN_H
