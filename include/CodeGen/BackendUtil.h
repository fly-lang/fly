//===--------------------------------------------------------------------------------------------------------------===//
// include/CodeGen/BackendUtil.h - Code Generator Utils
//
// Part of the Fly Project https://flylang.org
// Under the Apache License v2.0 see LICENSE for details.
// Thank you to LLVM Project https://llvm.org/
//
//===--------------------------------------------------------------------------------------------------------------===//

#ifndef FLY_BACKENDUTIL_H
#define FLY_BACKENDUTIL_H

namespace fly {

    enum BackendAction {
        Backend_EmitAssembly,  ///< Emit native assembly files
        Backend_EmitBC,        ///< Emit LLVM bitcode files
        Backend_EmitLL,        ///< Emit human-readable LLVM assembly
        Backend_EmitNothing,   ///< Don't emit anything (benchmarking mode)
        Backend_EmitObj        ///< Emit native object files
    };

    class BackendUtil {

    };
}

#endif //FLY_BACKENDUTIL_H
