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

#include <Frontend/InputFile.h>
#include "llvm/ADT/Triple.h"

namespace fly {

    class Triple;

    class ToolChain {

        const llvm::Triple &T;

    public:
        ToolChain(const llvm::Triple &T);

        bool BuildLib();

        bool Link(const llvm::SmallVector<std::string, 4> &ObjFiles, llvm::StringRef OutFile);

        bool LinkWindows(const llvm::SmallVector<std::string, 4> &ObjFiles, llvm::StringRef OutFile,
                   SmallVector<const char *, 4> &Args);

        bool LinkDarwin(const llvm::SmallVector<std::string, 4> &ObjFiles, llvm::StringRef OutFile,
                    SmallVector<const char *, 4> &Args);

        bool LinkLinux(const llvm::SmallVector<std::string, 4> &ObjFiles, llvm::StringRef OutFile,
                     SmallVector<const char *, 4> &Args);
    };
}

#endif //FLY_TOOLCHAIN_H
