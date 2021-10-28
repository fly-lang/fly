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

#include "llvm/ADT/Triple.h"

namespace fly {

    class Triple;

    class ToolChain {

        const llvm::Triple &T;

    public:
        ToolChain(const llvm::Triple &T);

        bool Link(llvm::StringRef File);
    };
}

#endif //FLY_TOOLCHAIN_H
