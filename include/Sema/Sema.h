//===--------------------------------------------------------------------------------------------------------------===//
// include/Sema/Sema.h - Main Parser
//
// Part of the Fly Project https://flylang.org
// Under the Apache License v2.0 see LICENSE for details.
// Thank you to LLVM Project https://llvm.org/
//
//===--------------------------------------------------------------------------------------------------------------===//

#ifndef FLY_SEMA_H
#define FLY_SEMA_H

#include "Resolver.h"
#include <llvm/ADT/SmallVector.h>

namespace llvm {
    class StringRef;
}

namespace fly {

    class DiagnosticsEngine;

    class Sema {

        DiagnosticsEngine &Diags;

        SemaBuilder *Builder;

    public:

        explicit Sema(DiagnosticsEngine& diags);

        ~Sema() = default;

        template <typename... Mods>
        llvm::SmallVector<SemaModule *, 8> Resolve(Mods... Modules) {
            llvm::SmallVector<ASTModule *, 8> ASTs;
            ASTs.reserve(sizeof...(Modules));

            // Expand variadic pack into the vector
            (ASTs.push_back(Modules), ...);

            return Resolve(ASTs);
        }

        llvm::SmallVector<SemaModule *, 8> Resolve(llvm::SmallVector<ASTModule *, 8> &Modules);

    };

}  // end namespace fly

#endif // FLY_SEMA_H