//
// Created by marco on 2/24/21.
//

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
